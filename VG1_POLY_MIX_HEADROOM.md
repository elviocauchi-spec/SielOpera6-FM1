# VG-1 FM1 — correcting overdrive with dense chords

## Emulator symptom

After performance optimizations, the user reported that adding notes
still caused audible overdrive even when FM1 DSP utilization was not
at its deadline. This is a **separate signal-headroom problem**, not an
audio underrun.

## Verified saturation before the fix

The Gated Motion per-voice oscillator, filter and drive stages were
clean. However, voices were summed directly in 32-bit arithmetic and
then hard-clipped at +/-65535 **before** pan, Trancegate, delay and
reverb. The output master being low could not undo this internal
saturation. A one-second C4-based stress test found:

| Simultaneous notes | Raw dry-bus >65535 | Final-output clips |
|---|---:|---:|
| 3 | 0 | 0 |
| 4 | 0 | 0 |
| 5 | 11 | 7 |
| 6 | 23 | 4 |
| 7 | 177 | 27 |
| 8 | 291 | 18 |

These metrics are for the original bus at nominal FM1 patch level with
the polyphonic FAT CPU budget enabled. They do NOT imply that samples
were continuously clipped or that the FM1 DSP was overloaded.

## Fix

The voice summing code keeps the full 32-bit mix *until* it has
applied a smooth, polyphony-dependent gain, **before any hard-clip or
effects feed**. This prevents the 16-bit delay and reverb loops from
being driven by an already-clipped bus.

- 1–3 sounding voices: unity target gain (original clean voicing
  unchanged aside from fixed-point rounding).
- 4+ sounding voices: progressively greater mix headroom, with roughly
  a 1ms attack and slower release to avoid note-on clicks and volume
  pumping.
- Per-voice filters, oscillators, envelopes and the intended **AMP
  DRIVE** parameters are untouched. The polyphonic FAT CPU budget and
  Trancegate timing remain untouched.
- Emergency dry-bus clamp is still present for unusually loud,
  user-edited patches but no longer engages for the tested Gated Motion
  chords.

The one-to-eight-note host test now verifies zero post-gain bus clamp
events, zero delay/reverb buffer saturation and zero final digital
clipping for both all-at-once and **progressively added** notes. The
eight-note static test peaked at around 27,573 of 32,767 instead of
hitting 32,767.

The desktop WAV artifact includes:
- `diagnostics/10_C4_E4_G4_C5_FOUR_NOTES.wav`
- `diagnostics/11_FIVE_NOTES_NO_OVERDRIVE.wav`
- `diagnostics/12_EIGHT_NOTES_NO_OVERDRIVE.wav`

All WAVs render at 44.1kHz stereo with a two-second note hold,
one-second release/tail. A host regression is not a substitute for
listening through the web FM1 emulator or real FM1 hardware.

## What to check next in the emulator

Play Gated Motion with 3, 4, 5, 6 and then 8 held notes.
Confirm no distortion, *HELD* matches the count and *DSP* remains
below 80%. The effective FAT count may decrease as voices are added
to preserve CPU capacity.

If distortion remains even when the desktop 8-note WAV is clean,
compare the physical **MASTER** potentiometer position and MIDI CC7/CC11
levels, and record the emulator's **DSP, FAT, HELD** readouts.
Use that to distinguish any remaining platform gain-stage clipping
from DSP overload or missed MIDI events.

Other factory patches with intentionally high **AMP DRIVE** may still
saturate by design; this fix addresses accidental chord-summing
saturation specifically.
