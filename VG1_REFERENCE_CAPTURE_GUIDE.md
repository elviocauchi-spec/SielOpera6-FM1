# VG-1 FM1 — original reFX Vanguard 1.6 listening comparison

## What the comparison measures

The **VG1-Vanguard1-Audio-Reference** GitHub Actions artifact contains
39 real audio renders from the SAME C DSP used by the FM1 firmware:

- Eight VG-1 *original diagnostic presets* in `presets/` (three seconds each).
- All 31 Vanguard 1.x named oscillator modes in `oscillators/` (half a
  second each, isolated, with their recorded file names and waveform numbers).
- An automatically generated `audio_report.csv` containing RMS, peak, clipped
  sample counts, stereo correlation and PCM hashes for every WAV.

This checks whether VG-1 is functional. It does **not** establish that it
sounds indistinguishable from reFX Vanguard 1.6; that requires independently
recorded, legally obtained Vanguard audio.

## Capture the original plugin for an honest A/B test

Use a licensed copy of the original reFX Vanguard **1.6 or 1.6.1**,
not Vanguard 2. The original manual describes the 31 waveform entries
(including OFF), 13 filters, FAT/VCS, AMP, ENV, FX and modulation controls.

In a DAW at **44,100 Hz**, initialize a patch as follows, without borrowing
any sample or wavetable assets:

1. Select only **oscillator 1**. Switch OSC2 and OSC3 to OFF (`---`).
   OSC1 level approximately 120/127; semi 0; PWM midpoint.
2. Switch FAT off and set AGE to zero. Disable OSC/LFO modulation,
   both envelope modulation matrices, glide, arpeggiator and Trancegate.
3. Set FILTER to **low 6 dB**, cutoff close to maximum and resonance at
   minimum; disable key and velocity filter tracking.
4. Set ENV1 AMP to immediate attack, full sustain and short release,
   drive to zero, AMP pan center, AMP SPREAD zero, and normal output level.
5. Turn off DELAY and REVERB, output exciter, and other host/master FX.
   Avoid DAW limiters and automatic loudness normalization.
6. Render a MIDI C4 (note 60), velocity 106, starting at 0.0 seconds,
   held to 0.40 seconds; render 0.50 seconds total as **stereo 16-bit
   PCM WAV at 44,100 Hz**. Do not trim the start or move it to the first
   nonzero sample.
7. Repeat for the original Vanguard menu entries at least:
   `sawtooth`, `square`, `peak pwm`, `combed sqr`, `ajx saw3`,
   `c64 noise` and `white noise`. For full coverage, use all 31.

The comparability of amplitude depends on matching reFX's UI values,
host volume, DAW mixing and original envelope/phase behavior. A simple
difference in RMS is not conclusive evidence of an incorrect oscillator.

### Names to use

Copy each original recording into a folder mirroring the renderer's
`oscillators` directory and use its matching WAV filename, e.g.:

```text
original-vanguard/
  oscillators/
    01_SQUARE_C4.wav
    03_SAWTOOTH_C4.wav
    05_PEAK_PWM_C4.wav
    06_COMBED_SQR_C4.wav
    21_AJX_SAW3_C4.wav
    29_C64_NOISE_C4.wav
    31_WHITE_NOISE_C4.wav
```

Run:

```sh
python3 tools/vg1_audio_report.py build/vg1-audio \
  --references original-vanguard
```

It writes `build/vg1-audio/reference_comparison.csv`, showing:

- Peak and RMS level differences (dB)
- Normalized 16-frequency spectral similarity
- Eight-slice envelope similarity
- Stereo correlation difference

This is an **initial diagnostic**. A true sound-matching pass must additionally
compare phase-aligned waveforms, spectrum across octaves, filter sweep
responses, aliasing, resonance, stereo FAT VCS 2/6/10, tempo-synced
Trancegate timing, and delay/reverb transients.

## Next engineering goals

1. Compare the seven distinctive oscillator reference captures with the
   corresponding VG-1 samples and improve wave transfer curves.
2. Characterize all 13 filter modes, adjusting response, resonance,
   filtering nonlinearity and key-tracking calibration.
3. Match Vanguard's stereo FAT distribution and retrigger/AGE behavior.
4. Build the reference-derived sound bank, then verify hardware CPU safety
   with large chords and FAT 10.
5. Only then label firmware **sound-matched**; passing build/test checks alone
   is insufficient.

This repository contains original implementation and original diagnostic
programs. Do not include proprietary reFX DLLs, wavetable files or original
factory preset banks in the firmware.
