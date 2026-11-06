> **Superseded.** This file documents the abandoned generic V-1 prototype.
> The active project is the **original reFX Vanguard 1.x recreation**:
> [VG1_VANGUARD_RECREATION.md](VG1_VANGUARD_RECREATION.md).
> Please use that document for the current architecture, validation and build.
>
# V-1 FM1 — independent trance synthesizer for M-VAVE FM1

**Development status:** V-1 0.1 experimental. Host DSP tests pass. The first firmware CI
build passed. Not yet measured or auditioned on a physical FM1. **Do not treat this
as a release.**

## Design philosophy

V-1 is an original **Vanguard-inspired performance synthesizer**, not a patch
set for Opera 6 and not an emulation of reFX's executable. It uses the FM1's
existing GPL-licensed board bootstrap only: codec/I2S, MIDI USB/TRS, screen,
keyboard/encoders, hardware protection, update recovery and packaging.

The signal path, oscillator allocation, envelopes, filters, effects,
Trancegate, arpeggiator, presets, patch structure and panel are V-1's own.
The Opera 6 four-track sequencer, mixer, synthesis engine and UI are not linked
into the V-1 executable.

Source layout:

- `firmware/v1/v1_core.h`: standalone fixed-point DSP, voice allocation,
  three oscillators, five-saw supersaw, filters, modulation, envelopes, gate,
  time-based effects and held-note arpeggiator.
- `firmware/v1/v1_app.c`: original V-1 FM1 control/navigation and starter
  preset bank, without Opera 6's multi-track structure.
- `tests/v1_core_smoke.c`: host DSP regression tests.
- `tools/build.py`: separate `V1_FM1=1` switch; unchanged Opera build
  when absent.
- `.github/workflows/v1-fm1.yml`: independent V-1 tests and firmware target.

## 0.1 feature inventory

| Component | Implemented now | Later |
|---|---|---|
| Oscillators | Three independent per note, saw/pulse/triangle/sine/noise/five-saw supersaw | Further digital/wavetable types, sync and ring modulation |
| Polyphony | 16 allocated voice slots; 8 active by default; control allows 4–16 | Hardware profiling to determine a glitch-free active limit and adaptive unison |
| Filters | LP12, HP12, BP12, notch, LP24, HP24, BP24 | More character modes; aim for 13 diverse choices |
| Envelopes | Independent amp and filter ADSR | More envelope routing |
| LFO | Three running LFOs, independent pitch/filter/amp destinations | Additional shapes, sync, fades |
| Arpeggiator | Up/down/up-down/random/pressed-order; only held notes; octave spread | Additional latch/swing/timing options |
| Trancegate | 16 editable binary steps; 1/8, 1/16, 1/32 rates, depth and smoothing | Gate-level steps, swing and tempo sync to MIDI |
| FX | Chorus, tempo-derived stereo delay, reverb, drive | Richer EQ and stereo effects |
| Presets | Eight original, deliberately different sounds | Larger professionally voiced bank, user storage |
| UI | Standalone V-1 oscillator/filter/FX/gate/arp pages | Polished performance screen and patch browser |
| MIDI | Local keyboard and USB/TRS MIDI notes, basic program changes | Full MIDI clock, sustain, CC learn, pitch bend |

Sound design starts with **HEAVEN LEAD, FLOOR SHAKER, CRYSTAL PLUCK,
GATED MOTION, ACID RUNNER, SATURN PAD, NIGHT ARPEG, LASER SHOT**.

These are V-1 patches, not copied reFX Vanguard preset data.

## Build

On Linux x86_64 with the JieLi toolchain, pinned AC79 SDK and Pillow:

```sh
BOARD=fm1 V1_FM1=1 sh build.sh
```

Expected output: `build/v1-fm1.fwsc`.

Or use the separate GitHub Actions workflow **V-1 FM1 independent synth
validation** and download its **V-1-FM1-experimental** artifact.

The build still carries the FM1 update identity required for the physical
hardware. Branch isolation does not create a physically separate flash slot.

## Hardware release gates

Before recommending installation:

1. Measure audio ISR time at 44.1 kHz against its 256-frame deadline with
   eight and sixteen notes, three oscillators, active supersaw and effects. Target under
   80% worst-case CPU allocation, leaving safety headroom.
2. Monitor for lost MIDI events, clicks, clipped peaks, stuck notes, runaway
   filters, and gate/ARP discontinuities.
3. Verify the on-screen DSP load gauge, encoder assignments, octave/key mapping, screen refresh and USB/TRS
   MIDI against a physical FM1.
4. Validate recovery/update route and firmware rollback.
5. Finish independent flash settings namespace, user-patch persistence,
   modulation and factory preset quality. Currently preset editing is volatile.

**Intellectual property:** The original reFX Vanguard sound aesthetic and
feature concepts are reference points. Do not distribute its proprietary
code, protected preset data, artwork or trade dress.
