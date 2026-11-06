# VG-1 FM1 — reFX Vanguard 1.x recreation project

> **Engineering status: experimental.** This is a feature-by-feature recreation
> of the *original* reFX Vanguard (1.6-era specification), **not Vanguard 2,
> Opera 6 or a loosely inspired trance synth**. This is a new implementation,
> not reFX software running on the FM1. The current filters/waves have not yet
> been acoustically calibrated against the reference plugin.

Reference: *The Vanguard Manual* 1.6.0 (reFX, 26 December 2006); original
Vanguard 1.0 manual; and 2007 v1.6/v1.6.1 release notes.

**Work branch:** `feature/vanguard1-recreation`  
**DSP:** `firmware/v1/v1_core.h`, `vg1_wave_dsp.h`,
`vg1_filter_dsp.h`, `vg1_reference.h`  
**Application:** `firmware/v1/v1_app.c`  
**Regression tests:** `tests/v1_core_smoke.c`.

## Vanguard 1 implementation matrix

| Vanguard 1 section | Work already present | What still requires validation / implementation |
|---|---|---|
| OSC | 3 independent oscillators, original **31 waveform menu entries**, tunings, pulsewidth modulation, retrig, AGE | Waveform transfer functions are *original approximations*, not matched to reFX; high-register anti-aliasing test |
| FAT + VCS | Stereo detune with up to **10 layered component voices**; oscillator audio rendered separately L/R | Match exact detune law and phase/stereo behavior to original |
| FILTER | Original **13 modes** with individual LP, HP, BP, notch, combined and formant/waveshaper paths | Response curves, resonance, formants, saturation and high-frequency performance need spectral A/B calibration |
| LFO 1/2/3 | Three triangle LFOs with per-oscillator detune/PWM and common filter cutoff routing | Match rate curves and retrigger behavior |
| ENV 1/2 | Two ADSRs; ENV1 level/PWM/cutoff/detune; ENV2 resonance/PWM/cutoff/pitch | Time curves and control response need calibration |
| AMP | DRIVE, level, VELTRK, stereo pan and SPREAD auto-pan | Calibrate nonlinear drive and panning law |
| TRANCEGATE | Two **separate L/R 16-step on/off patterns**, contour, speed/divisions, mix | MIDI Song Position sync control currently displayed but not connected to external transport |
| DELAY | Original mono/stereo/cross/widen types, feedback, damping, speed including triplet/dotted and Widen milliseconds | A/B sonic characterization against reference |
| REVERB | Serial after delay; predelay, room size, damp, width, stereo output | Reverb character still an approximation |
| ARP | Original 5 modes: UP/DOWN/ALTER/ORDERED/RANDOM; played-note order and octave range | Full retrigger/timing tests against real keyboard and external MIDI clock |
| GLIDE | Original **mono-legato** behavior, hold-note priority and INSTANT / tempo subdivisions | Hardware note transitions and release timing |
| PITCH | ±0–24 semitone pitchbend; channel aftertouch controls vibrato | Calibrate vibrato depth/rate curves |
| VOICES | 16 allocated slots, 8 voice safe initial setting; variable per-patch | Original software offers 1–32; hardware capacity remains to be benchmarked |
| MIDI | Notes, program changes, pitchbend, aftertouch, CC1/7/8/10/11/21/22/23/71/74, All Sound Off | External MIDI tempo clock and full synchronization |
| Presets | Eight original diagnostic sound designs using Vanguard controls | Professional A/B-voiced bank, user saving, import/export after licensing checks |
| UI | Vanguard-specific pages for OSC 1–3, FAT/VCS, LFO 1–3, FILTER, ENV routing, AMP, TRANCEGATE, DELAY, REVERB, ARP, GLIDE, PITCH, VOICES | Refine compact navigation on the 240×240 display (not copied reFX trade dress) |

### Important engineering constraints

The M-VAVE FM1 has very different CPU resources from a desktop VST host.
Compatibility does **not** mean falsely claiming all 32 voices or exact reFX
DSP if doing so would cause under-runs. The audio backend is fixed-point
44.1 kHz, 256-frame double-buffered I2S. The on-screen DSP meter measures
work relative to the buffer deadline. **A successful CI build does not prove
that the physical FM1 is glitch-free.**

### Current automated validation

The independent GitHub workflow builds the new DSP on host Linux, runs
Vanguard-focused regression tests, then compiles an FM1 firmware package.
Tests cover 31 waveform dispatches, 13 filter modes, stereo FAT versus mono,
VELTRK zero/positive, hard panning, stereo Trancegate, ordered held-note
arpeggiation, note release, monophonic glide and MIDI output gain.

A waveform passed by the test means it is non-silent and correctly dispatched,
not that it reproduces the proprietary original oscillator precisely.

### Build instructions

From this branch, on a suitable build environment with the pinned
JieLi AC79NN SDK and toolchain:

```sh
V1_FM1=1 BOARD=fm1 sh build.sh
```

Output: `build/v1-fm1.fwsc`. The standalone Vanguard build reuses only
the GPL-licensed board platform, display, MIDI, flash recovery and packaging.
It does not link the Opera 6 synthesis engine, sequencer, mixer or UI.

### Release gates before recommending firmware installation

1. Eight-key chord plus all OSC FAT/VCS settings, all three LFOs, filters and
   full FX chain: measure worst-case on-device audio ISR usage, not just average.
2. Verify note releases, voice stealing, patch changes, MIDI USB/TRS, pitchbend,
   pedal/CC handling, tempo behavior and absence of stuck notes.
3. Confirm stereo output, filtering, level and distortion on physical headphones
   and compare actual audio captures to legally sourced Vanguard reference.
4. Verify the firmware updater, rescue mode and rollback path on the exact FM1.
5. Implement persistent Vanguard patch storage with a separate flash
   compatibility namespace. The prototype currently edits settings **in RAM**.

**Do not install as a production firmware yet.** The build is an experimental
engineering prototype until these hardware gates have been verified.

### Sound-matching methodology

To recreate the *sound* rather than merely the feature names, capture
reference renders at 44.1 kHz for every original oscillator over C2–C6,
PWM 0/64/127 and several pitches/velocities; record filter sweeps/resonance,
FAT zero/mid/high with VCS 2/6/10, envelope transients, stereo Trancegate
timing, feedback repeats and isolated effects. Compare pitch, harmonic
spectra, stereo correlation, envelope dynamics, timing and perceived
character; iterate original DSP until the differences are acceptably small.
No proprietary binaries, preset banks, wavetable files or artwork are copied.
