# VG-1 Gated Motion — crunch diagnosis and revised voicing

The user heard residual crunch after the first no-clipping repair. The cause
was not hard-clipping at the DAC: the original Trancegate had almost
instantaneous edges (a seven-sample one-pole fade at CONTOUR 104) and the
PWM oscillator dominated the more restrained AJX Saw3 oscillator.

## Changes on the Vanguard reconstruction branch

- The real FM1 DSP Trancegate now runs **two cascaded, smoothly shaped
  one-pole amplitude envelopes**, separately for L and R, using fixed-point
  arithmetic and one contour coefficient per audio block. It keeps original
  independent 16-step L/R masks and original tempo-sync.
- Gated Motion now uses a modestly less prominent PWM oscillator, zero
  accidental drive gain, a slightly calmer cutoff/resonance and contour.
  It preserves all three Vanguard oscillators, stereo FAT/VCS four,
  the original gate patterns, cross delay and reverb.
- The host regression `tests/vg1_gate_smoke.c` tests edge-step size,
  monotonic opening/closing, the endpoints and contour mapping.
- The normal `tests/vg1_render.c` stereo WAV build now includes an
  **identical-chord comparison pack** in `diagnostics/`.

## Listen to these WAVs in order

All versions play the same C major chord, with the same release and
3-second WAV length. The file `09_PREVIOUS_BRIGHT_TONE.wav` isolates the
older bright PWM/filter settings while using the **same new gate smoothing**.

1. `diagnostics/01_ORIGINAL_PRESET.wav` — current repaired, softer preset
2. `diagnostics/09_PREVIOUS_BRIGHT_TONE.wav` — old brighter settings for A/B
3. `diagnostics/02_GATE_ON_NO_FX.wav` — gate and oscillators; no delay or reverb
4. `diagnostics/03_NO_GATE_NO_FX.wav` — oscillators and filter only. Output
   gain is lowered for safe headroom, so adjust listening volume manually.
5. `diagnostics/04_NO_FAT_GATE_ONLY.wav` — same gate, but no FAT VCS layers
6. `diagnostics/05_AJX_SAW3_ONLY.wav` — isolated AJX oscillator
7. `diagnostics/06_PWM_PULSE_ONLY.wav` — isolated PWM oscillator
8. `diagnostics/07_SAW_ONLY.wav` — isolated third saw oscillator
9. `diagnostics/08_GATE_WITHOUT_DRIVE.wav` — isolate effect of AMP drive

If 03 is clean but 02 is rough, inspect **the gate shaping**. If 02 is
clean but 01 is rough, inspect **delay/reverb feedback/tails**. If 06
alone sounds harsh, inspect **PWM wave bandlimiting and modulation**.
If 04 is clean but 02 is rough, check **FAT detuning/stereo phase**.

These experiments compare VG-1 against itself; they are **not** evidence
of reFX Vanguard waveform accuracy. Continue original Vanguard 1.6
audio reference matching separately.

## Quality gates

For Gated Motion the audio render pipeline fails if it detects internal
oscillator/mixer, pre-drive, post-drive, delay, reverb or output overflow.
The `tools/vg1_audio_report.py` stage also requires no silent waveforms,
no clipped PCM in any WAV, and distinct oscillator/preset signals.

Tests cannot conclusively rule out aliasing, oversharp transients or
device-specific buffer underruns. Real FM1 listening and CPU profiling
remain outstanding.
