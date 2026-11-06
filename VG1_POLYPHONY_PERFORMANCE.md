# VG-1 FM1 — four-note DSP overload mitigation (experimental)

## Reported issue

Testing the online FM1 emulator, GATED MOTION was at **87% DSP with
three simultaneous notes** and approximately **107% DSP with four**,
resulting in severe sound distortion. These are observed emulator values,
not CPU estimates. Real FM1 silicon has not yet been measured.

## Root cause and changes

GATED MOTION normally uses three Vanguard oscillator modules (AJX saw3,
pulse PWM and sawtooth), each rendered as four FAT/VCS stereo layers.
Its DSP cost rises sharply with voice count.

1. Pitch modulation originally recalculated all ten possible FAT layers
   for *each* oscillator every 32 samples, even when fewer were active.
   It now processes only layers selected for rendering, and skips silent
   oscillators entirely.
2. The second filter coefficient (and its two divisions) is now computed
   only for dual-filter modes such as X, M and formant. A standard LP12
   no longer pays this cost.
3. Shared parameters (MIDI modwheel, ARP mode, FAT count) are determined
   once per audio block rather than repeatedly in the per-sample loop;
   sine vibrato evaluation is bypassed when aftertouch depth is zero.
4. A conservative global FAT voice budget of ten oscillator layers
   prevents a large chord from multiplying the work beyond the safe
   baseline. The user-visible VCS setting remains the *ceiling*.
   For GATED MOTION with VCS=4 the allocation is:
   - 1–2 active voices: four layers each
   - 3 active voices: three layers each
   - 4 active voices: two layers each
   - 6 active voices: one layer each
   All notes still have their own filter and amplitude envelopes.
5. The FM1 app measures the audio callback against its actual 256-frame
   deadline. If the measured load exceeds **78%**, the next callback
   runs with a lower FAT ceiling. The ceiling does not increase during
   the phrase; it resets after the last voice finishes or at preset
   change. The display now shows effective **FAT** alongside **DSP**.
   This emergency governor is *reactive* and cannot guarantee the
   very first overloaded callback is glitch-free.
6. The no-clip audio report was rerun for *all eight* factory patches
   and all 31 oscillator types. SATURN PAD's master was reduced from
   76 to 70 to restore headroom after the FAT allocation changed.

## Required regression testing

The host test `tests/vg1_polyphony_smoke.c` checks:
- Exact FAT layer budget for one, two, three, four and six voices
- All four note-ons produce independent active voices
- Four-note and three-note C-major renders are audible, without
  hard-clipped output; note-off fully releases every voice
- Host render times for each chord (diagnostic only; host time is **not**
  a calibration to the FM1 CPU)

The WAV renderer emits
`diagnostics/10_C4_E4_G4_C5_FOUR_NOTES.wav`, a three-second
four-note reference: hold C4/E4/G4/C5 for two seconds at 128 BPM,
then release and listen to the effect decay.

## FM1 emulator acceptance procedure

1. Load the latest experimental firmware built from this branch.
2. Select **GATED MOTION**, play C4/E4/G4 for several seconds.
   Note DSP% and effective FAT on the screen.
3. Add C5 while holding the other three keys.
   Note peak DSP% and whether FAT drops automatically.
4. The steady DSP value should stay **below 80%** without clicks or
   missing notes. If it does not, report the four-note **DSP** and
   **FAT** readouts: further improvements are required.
5. Release all keys. HELD should read 0; voices should fade and all
   become inactive. Listen for the expected short effects tail.
6. Compare the online emulator audio with the included four-note WAV.

**Not certified for physical FM1 yet.** The governor avoids persistent
overload when reducing FAT is sufficient, but it is not a substitute for
actual AC79 load profiling or audio interrupt deadline testing.
