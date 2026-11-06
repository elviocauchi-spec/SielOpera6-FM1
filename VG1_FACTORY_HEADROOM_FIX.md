# VG-1 FM1: all-preset unwanted distortion fix

## What the FM1 emulator user observed

Gated Motion's original problems were fixed, but the user subsequently
reported that **Heaven Lead and the other factory programs distorted too**,
including single-note playing. The polyphonic mixer correction did not
address hard clipping that was happening *within each individual voice*.

## Confirmed causes in the existing FM1 firmware DSP

- Very hot three-oscillator stacks were clipped at a fixed +/-50000
  **before the resonant filters**. These levels were not corrected by
  lowering the output MASTER or by the polyphonic voice bus.
- AMP DRIVE multiplied the filtered signal, then used a **hard
  16-bit clamp**, creating strong flat-topped digital harmonics even
  on intended clean/lead sounds.
- High filter resonance, e.g. Acid Runner, magnified that problem.
- Adding held notes could overload the sum after the filters and later
  the final stereo output. Some previously 'hot delay/reverb' reports
  were actually inactive effects buffers unnecessarily fed dry audio.
- Program switches could inherit reduced voice-bus gain from the
  previous patch.

The first full diagnostic tested **all 8 factory programs** from
1, 3, 5 and 8 notes. It recorded drive-stage clipping even on one-note
Heaven Lead, Floor Shaker, Crystal Pluck, Acid Runner, Night Arpeg
and Laser Shot. Several programs also hard-clipped the final output
on multi-note chords. The diagnostic uses the real firmware DSP.

## Fixes

1. **Pre-filter headroom.** Sum all oscillator modules in 32-bit first,
   and use a patch-dependent, fixed-point pre-filter stack gain before
   entering any clipping point. Driven patches receive headroom based on
   summed levels, AMP DRIVE and filter resonance. Clean Gated Motion
   and Saturn Pad retain their untrimmed oscillator stacks.
2. **AMP DRIVE shaping.** Replace the old digital hard-clip step with a
   continuous-slope, 32-bit quadratic soft drive. It leaves lower-level
   audio linear, produces progressively softer harmonics when DRIVE is
   raised, and avoids hard clipping into an unmodulated square shape.
   The DRIVE control remains operational and intentional coloration
   can still be heard at higher settings.
3. **Driven-chord headroom.** The previously implemented smoothed
   voice-bus gain now handles two-note and larger *driven* chords too.
   Full 32-bit voice sums are reduced before stereo pan, Trancegate
   and effects to prevent distortion being imprinted upstream of MASTER.
4. **Peak-only output protection.** The final stereo bus has a smooth
   quadratic knee above 28000 PCM units and a maximum of 32096,
   avoiding the former abrupt clamp at +/-32767 for the rare
   transient which escapes the upstream gain structure.
5. **OFF really means OFF.** Disable delay/reverb audio processing and
   clear their running buffer entries when those effects are off;
   they no longer waste FM1 CPU or report phantom overloads.
6. **Preset switching.** Reset the polyphonic mixer envelope on patch
   change rather than inheriting the previous program's attenuation.

## Automated validation

The `tests/vg1_all_presets_headroom.c` GitHub CI job now exercises:

- All eight unique Vanguard-architecture diagnostic presets
- All simultaneous note counts from 1 through 8
- Two MIDI velocities: 100 and 127
- A further round of progressively **adding** notes one by one
- 128 simultaneous + 128 progressive tests (256 cases total)

The tests fail for oscillator stack clipping after its trim, true
AMP DRIVE plateau saturation, post-gain bus clipping, delay/predelay/
reverb buffer saturation or output hard clipping. The host smoke
tests and the full 44.1kHz WAV audit still run separately.

The audio artifact includes `drive_comparisons/` WAVs for all six
factory patches with AMP DRIVE enabled; each has SOLO and CHORD
versions with DRIVE ON and DRIVE OFF. The DRIVE OFF versions retain
approximately the same *pre-filter* gain, isolating coloration more
fairly than simply toggling DRIVE while changing source headroom.

## Still to verify on the online emulator

The fixed-point DSP and output samples pass host tests. The GitHub
pipeline builds experimental FM1 firmware, but this does not reproduce
FM1 hardware buffer timing, analog gain, DAC behavior, emulator-specific
artifacts or the original reFX Vanguard 1.6 DSP.

Install the newest firmware, load **HEAVEN LEAD** and hold one note,
then 3–4 and 6–8 notes, monitoring the DSP readout. Compare solo/chord
DRIVE ON/OFF WAVs to identify intended DRIVE coloration. Repeat for
FLOOR SHAKER, CRYSTAL PLUCK and ACID RUNNER. User can also turn the
AMP page's DRIVE parameter to zero to check a fully undriven patch.

This is a correction to *our original Vanguard-compatible engine*,
not an assertion of exact reFX factory preset or timbre identity.
