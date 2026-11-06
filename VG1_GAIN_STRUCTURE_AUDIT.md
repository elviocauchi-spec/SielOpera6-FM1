# VG-1 Vanguard FM1 — gain-structure audit, 6 November 2026

## User-visible symptom

The online FM1 emulator still sounded distorted across factory
programs, including Heaven Lead, after the earlier CPU and headroom
repairs. Zero hard-clipped PCM samples **did not** prove that gain
structure was clean: earlier output stages could overdrive internally,
or the final soft-knee limiter could change the sound before PCM clip.

## Root causes fixed

### 1. Up to +6 dB of hidden stereo panning gain

After the polyphonic bus already applied safe gain, the old pan law
multiplied its most favored channel by up to **2x**:

```c
dryl = dryl * (254 - 2 * pan) / 127;
dryr = dryr * 2 * pan / 127;
```

With Vanguard's stereo SPREAD modulation, moving toward the left or
right could defeat all upstream headroom. It now uses per-channel
unity-ceiling balance factors. Center pan remains approximately the
same; extreme pan no longer gets an unwanted +6 dB.

### 2. The chord-gain envelope arrived too late

The original mixer target depended on note count but its output
approached that target during the first ~1 ms of audio. Starting
a big chord at once therefore delivered the first samples with
the prior unity gain; the final soft limiter was forced to suppress
a transient after it had already reached the effects.

When a newly keyed chord begins with **all amp envelopes at zero**,
the mixer now pre-arms its required headroom before any note produces
audio. When adding notes to an already audible chord it still uses the
smooth, click-resistant gain transition.

### 3. Loud programs and late reverb were not in the gain budget

A voice mix that was safe at master 55 might not be safe at master
93–97 plus delay/reverb wet feedback. The gain budget now reserves
progressively more room for loud programs with 3+ voices, for eighth
notes in dense chords, and for patches with high reverb mix whose
wet tails build up over a sustained chord.

Acid Runner's resonant filter/DRIVE also receives sufficient
pre-filter source headroom to avoid the flat-top portion of the
DSP's smooth drive shaper on high notes.

### 4. The previous tests did not measure everything

The audit now counts **filter state integrator clamps**, the
post-gain voice-bus clamp, stereo pan overloads, 16-bit delay and
reverb buffers, AMP DRIVE flat-top activation, the raw final output
before its safety limiter, and every use of the output softguard.

A new `tests/vg1_gain_matrix.c` explores:
- All eight factory presets
- Five root notes (MIDI 36, 48, 60, 72, 84)
- One, three, five and eight simultaneous notes
- Zero/full modulation settings (modwheel, aftertouch and pitch bend)
- MIDI velocity 127
- Extra sustained ~1-second feedback windows for five/eight-note
  chords in registers MIDI 48 and 72

That is **320 scenarios** in addition to the preexisting 256
simultaneous/progressive voice tests at velocity 100 and 127.

## Confirmed measurements

| Audit iteration | Flagged combinations | Description |
|---|---:|---|
| Initial, before stereo/mix corrections | 53 / 320 | Output limiter overuse; stereo pan exceeded internal headroom |
| After fixing pan amplification | 32 / 320 | Panner no longer boosts channels |
| After master-aware voice-bus headroom | 11 / 320 | Most hot-preset chord clipping eliminated |
| After additional eight-note headroom | 3 / 320 | Only Floor Shaker dense transient remained |
| After silent-chord pre-arming | 0 / 320 in short audit | Correct headroom from the first audio sample |
| Added sustained feedback tests | 2 / 320 | Saturn Pad late wet return still triggered softguard |
| After high-reverb wet-return budget | **0 / 320** | No monitored internal hard clipping or output softguard use |

The final audit output reports `flagged=0, filter=0, bus=0,
pan=0, fx=0, output_guard=0`. Full 128 simultaneous + 128
progressive factory headroom tests also pass, as do the byte-exact
reference-vs-fast DSP comparison, waveform distinctness tests, and
experimental FM1 firmware compilation.

## What this establishes / limitations

This establishes that the **new DSP renderer's measured gain stages**
do not hit any saturation/limiter in the tested stress scenarios, and
the program's sound is not being flattened by the end-of-chain guard
there. It does NOT prove that analog output, emulator software, USB
monitoring, audio-buffer underruns, oscillator aliasing or arbitrary
user-edited extreme patch settings are free of distortion.

The architecture and oscillator algorithms are independently written
Vanguard-compatible approximations, not an exact reFX binary or
the copyrighted preset bank.

## Required FM1 emulator test

After installing the newest successful VG-1 firmware artifact,
listen to Heaven Lead, Gated Motion and Saturn Pad with:
1. One held note at full velocity.
2. C4/E4/G4 then C5, with note-offs after a couple of seconds.
3. Six to eight notes; hold the chord long enough for delay/reverb to
   build up.
4. Monitor HELD, DSP and FAT readouts, and compare the generated WAVs
   for audibly identical synth tone.

The physical MASTER knob also affects the platform's final gain;
the on-screen DSP meter is **complete audio ISR load** and should be
kept well under the hard 100% audio deadline.

If sound still seems distorted while the gain audit and emulator
full-ISR DSP are clean, the next target is waveform anti-aliasing,
filter harmonic character or the emulator's audio/USB pipeline,
rather than repeatedly lowering all preset volumes.
