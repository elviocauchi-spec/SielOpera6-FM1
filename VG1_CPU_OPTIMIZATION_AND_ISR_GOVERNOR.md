# VG-1 FM1 — CPU optimization and audio-ISR deadline protection

## Motivation

The online FM1 emulator previously showed DSP ~87% at three notes and
~107% with a fourth, with digital artifacts that did not occur in the
desktop WAVs. A later firmware fixed clipping in the oscillator and
polyphonic gain stages, but some CPU-related artifacts remained.

The FM1's 256-frame 44.1kHz half-buffer callback has an approximately
5.8ms audio deadline. The old app's **DSP** display measured only the
synth render, omitting the following physical MASTER scaling, USB tap,
sample conversion, DMA writes and IRQ bookkeeping. This could underreport
the time spent in the actual audio interrupt.

## Performance changes

These changes are on the separate `feature/vanguard1-recreation`
branch only, not the Siel Opera 6 release branch.

- ADSR attack/decay/sample rates are calculated **once per audio block**,
  instead of per voice per sample; the old per-sample envelope remains
  available to validate exact matching PCM.
- Filter cutoff and envelope/LFO routing for the filter is computed only
  on the existing **32-sample control ticks** rather than all 44,100
  samples/sec per note.
- Vibrato sine is evaluated only on pitch update samples.
- The waveform evaluator no longer computes an unused bandlimited
  sawtooth for pulse, sine, FM and noise oscillator modes.
- The common one-, two-, three- and four-layer FAT oscillator cases use
  dedicated short loops and exact fixed-count stereo averaging. All
  other FAT sizes still work.
- The renderer scans only the currently configured or still-active
  voice slots instead of checking all sixteen slots on every sample.
  The monophonic glide and 8-voice default remain intact.
- Three oscillator PWM LFO terms are shared across the voices at
  audio rate. Reverb tap positions/room size, predelay, delay feedback,
  damping, width and wet mix are cached per 256-frame block.
- In the VG1-only platform build, the renderer now **overwrites every
  stereo sample** instead of adding to a pre-cleared DMA buffer. The
  I2S IRQ can skip all 512 initial clearing stores and the synth avoids
  512 unnecessary additive reads.
- The DSP meter now covers the **entire ALNK audio ISR**. At more than
  **72%** complete ISR load, the FAT governor reduces the number of
  layered oscillators, preserving all individual note voices.
- If the full ISR is still above **76%** when FAT is already at one layer
  per voice and at least four notes are held, an emergency second tier
  fades the two late reverb combs out over ~0.7ms, leaving the first
  **two stereo combs active**. This sacrifices some reverb density rather
  than dropping musical notes or overrunning the audio deadline.
  Full quality is restored when all voices finish or a patch is changed.

## Regression tests

GitHub Actions now builds the actual FM1 firmware, runs 256 simultaneous
and progressively added multi-note factory headroom cases, renders 75
reference WAV recordings and compares **every WAV byte** with the
original per-sample envelope, variable FAT averaging and full voice
slot scan reference. The benchmark additionally checks checksums of
the same 256-frame DSP calls, with optimized I2S overwrite enabled.

The eight-voice emergency-reverb test verifies four-comb versus
two-comb mode for Heaven Lead, Gated Motion and Saturn Pad, checking
all eight notes remain active, the reverb fade reverses and no internal
reverb or final DAC saturation appears.

Host CPU timings (NOT an AC79 speed prediction) have varied between
GitHub runners. A recent run measured a median speedup of about
**1.06x** for the benchmarked 2/4/6/8-note workloads. Two- and four-note
passages generally improve by ~4–10%, while some six-to-eight-note
host benchmarks are near parity or a few percent slower. The true
embedded benefit of avoiding per-sample division and I2S buffer
clearing must be verified on the online emulator/real AC79 hardware.

## FM1 emulator acceptance

1. Install the latest experimental VG1 firmware from the successful
   workflow on the feature branch.
2. Test HEAVEN LEAD and GATED MOTION with 1, 3, 4, 6 and 8 held notes.
   Use C4 E4 G4 C5 for a four-note reference; play additional D5 F5
   G5 for the dense test.
3. Record **DSP** (now whole audio interrupt) and **FAT** (actual
   oscillator layers rendered) at each chord size.
4. Desired sustained whole-ISR DSP is below about **80%**. Crackling,
   underruns or a reading above ~90% means further hardware-targeted
   optimization is needed. This is NOT a promise of realtime success.
5. Check that none of the notes hangs after release (HELD=0) and compare
   with the WAV audio reference. At high load the reverb may become
   slightly less dense, which is the safety tradeoff.

The implementation is a new DSP approximation of Vanguard 1's
architecture; do not claim a bit-perfect reFX emulator.
