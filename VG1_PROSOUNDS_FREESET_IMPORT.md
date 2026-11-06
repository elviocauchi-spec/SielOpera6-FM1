# ProSounds Vanguard FreeSet 1 — imported into VG-1 FM1

**Source:** user-provided `ProSounds_Vanguard_FreeSet_1.fxb`
**SHA-256:** `a47557abeb228077f2d659fd38e0dc6e6a76ae328219272e33d1a9077a34b3c6`
**Source format:** VST2 `CcnK/FxBk`, plugin ID `VGRD`, 128 program records `FxCk` containing 92 big-endian floating-point parameters each.

This is an external **ProSounds** sound bank, **not** the genuine original reFX factory bank.

## Firmware bank layout

- Program slots 1–8: the eight previously calibrated original VG-1-authored presets (unchanged).
- Program slots 9–136: all 128 ProSounds FreeSet VST2 program slots in exactly their original order.
- ProSounds internal source slots 0–45: normal programs / source section labels.
- ProSounds internal source slots 46–89: Gated section.
- ProSounds internal source slots 90–127: Arpeggiated section.
- Three category-divider records remain in the list so the source bank's position numbers stay aligned.

The FM1 display shows `VG1 1..8` or `PS 1..128` with the sound name. The preset encoder browses the whole 136-slot bank. From a MIDI controller, send CC0=0 followed by Program Change 0–7 for the VG-1-authored bank, or CC0=1 then Program Change 0–127 for all 128 ProSounds slots.

## Parameter mapping accuracy

The input is a real and usable Vanguard 1 VST2 bank, and we have extracted the complete names and all source slot positions. However, VST2 regular FXB data stores **92 anonymous normalized floats per program**, not their semantic control names. The actual reFX Vanguard 1 VST2 parameter-index contract has **not** been established in this repository.

Comparisons of duplicate named source programs across the three bank sections identify **parameter index 45 as ARP enable** and **index 71 as Trancegate enable** with high confidence. These enable states are preserved directly.

Other values in `vg1_prosounds_generated.inc` are a **provisional sound-design translation** using bounded values derived from the source floats. This is a set of newly playable approximations **not guaranteed to be a faithful reproduction of each ProSounds patch**.

The bank has seven anomalous raw floating-point values far outside normalized 0..1 at source parameter index 65 (three different programs repeated in multiple sections). The provisional mapper ignores that field rather than interpreting out-of-range values as real synth controls. The original uploaded FXB remains the source of truth.

## Safety and build tests

The VG-1 CI exercises:
- 128 imported programs, one/three/five-note render and correct note releases: 384 cases
- Separate exact authored-program 128 simultaneous + 128 progressive test suite
- 320 extended original-program full-gain stress scenarios
- 203 offline 44.1 kHz stereo WAV renders, with no digitally clipped samples or fully silent files
- Byte-exact optimized/reference DSP audio comparisons
- FM1 firmware build using the target AC79 SDK

**Status:** the experimental 136-slot firmware and WAV artifact were both built by GitHub Actions run `42280493162`.

## Future work to obtain sound-for-sound Vanguard compatibility

Use an original installed Vanguard 1 VST host to export plugin automation/parameter-name + value descriptions for a handful of known source presets, or acquire a documented version-specific VST parameter index map. Only then replace the provisional index assignments with verified control mappings. Compare audio rendered in Vanguard 1 vs VG-1 for the same MIDI notes to calibrate oscillator/filter/effect DSP response.

Do not relabel these programs as `Original reFX Factory` or claim bit-identical conversion.
