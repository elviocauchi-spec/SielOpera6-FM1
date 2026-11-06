# VG-1 FM1 — import genuine Vanguard 1 factory presets

## Goal

Replace the extra *invented* sound designs with genuine parameter data
from the original reFX Vanguard 1 factory bank (128 programs). Preserve
the existing experimental eight-preset bank until source conversion and
audio/headroom testing succeeds. This project is an independent DSP
recreation; importing parameter values is not a guarantee of identical
reFX oscillator and filter audio.

## Required source

Obtain a **bank exported from an original licensed Vanguard 1 plugin**
using the host's Save Bank / Export VST Bank function. Save as
`Vanguard-Factory.fxb`. An individual `*.fxp` is also usable to
calibrate the mapping, but a full bank is preferred.

A factory bank export must be performed BEFORE changing or replacing
original factory program slots if the goal is the factory bank.

Do not use a Vanguard 2 bank as a drop-in replacement: its synth
architecture and preset format are not identical to Vanguard 1.

## What the new importer can do now

```sh
python3 tools/vg1_fxb_inspect.py Vanguard-Factory.fxb \
  --json build/vanguard1-factory.json
```

- VST2 `FxBk` (regular bank): extracts all names and normalized
  parameter floats, preserving their exact values and bank order.
- VST2 `FxCk` (program): extracts name and parameters.
- VST2 `FPCh` or `FBCh` (opaque plugin-specific chunks): correctly
  identifies their sizes and SHA256 hashes and **refuses to invent
  parameters**. Exact Vanguard-specific chunk decoding will require
  examining a legitimately obtained sample bank.
- Rejects corrupt record sizes, nonfinite parameter values and banks
  containing programs from a different VST plugin.
- All four record types are exercised with synthetic data in
  `tests/vg1_fxb_inspect_test.py` and the GitHub VG-1 workflow.

**Important:** this is a source-bank inspector, not yet a complete
Vanguard 1 -> VG-1 parameter converter. VST2 `FxCk` files contain
parameter **indices and normalized floats**, not semantic control
names. The real Vanguard 1 parameter ordering/scales must be verified
against exported presets or host automation parameter names before
translation can be trusted.

## Required steps once genuine data is available

1. Inspect the source bank, record plugin ID, versions, program count
   and whether state is regular parameters or opaque chunks.
2. Map actual Vanguard 1 parameter indices or chunk fields to our
   explicit `vg1_reference.h` contract: OSC1–3 waveform/PW/semitone/
   fine/level, FAT/VCS/AGE, filter/resonance and tracking, ENV1/ENV2,
   LFO1–3, AMP/DRIVE/SPREAD, Trancegate patterns and speed, delay,
   reverb, arpeggiator and glide.
3. Convert the factory bank into `v1_patch_t` data and include
   converted presets in the firmware's compile-time bank. The existing
   encoder/MIDI Program Change selection already supports more than
   eight program indices; the bank-size constant and UI labeling must
   be updated to the actual exported program count.
4. Render and compare every converted program, apply the same strict
   zero-clipping and polyphony/ISR overload tests, and tune relative
   levels **without deleting the original source parameter records**.
5. Compile a new `vg1-fm1.fwsc` firmware and publish a separate WAV
   listening pack. Retain the original eight VG-1-authored patches as
   a separate optional bank rather than presenting them as reFX
   factory sounds.

## Not currently available

No original Vanguard 1 `.fxb`, `.fxp` or original plugin binary
has been supplied in this conversation or stored in the connected
project repository. No original factory sounds are therefore
converted yet. **Do not claim 128 reFX presets are included in the
current eight-preset VG-1 firmware.**
