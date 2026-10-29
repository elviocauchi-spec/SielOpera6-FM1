# SIEL Opera 6 / DK600 factory bank — v0.8 development

**95 original programs preserved. ROM-proven storage partition now decoded; not yet playable FM1 presets.**

`data/siel_factory_packed.hex` contains the exact 95 original program records (00–94), 20 packed bytes each. `firmware/src/siel_factory_bank.h` preserves these bytes. Original SysEx SHA-256: `92a2cdf1133601bb1c1d6dc458e2ed7f8abafdfae1070ade9c56daf3349fe2c6`.

## New finding from user-supplied DK600 v10 TMS7000 ROM

The original 20-byte program format is **four digital-register bytes followed by 16 analog-control bytes in reverse scan order**. This is not an assumption based on the MIDI CC map:

- At E1B5..E1BA, a control value is written to both `C020+B` and `C034+B`; the E1xx scan traverses 16 values, B=0..15.
- E54A/E557/E565/E58D mirror R44, R45, R46, R47 to `C044`, `C045`, `C046`, `C047` respectively.
- At E7D5..E7E0, 20 bytes are copied from `C047` down to `C034` to the program storage; E7F9 performs the reverse load.
- At F6B2..F6C4, the MIDI dump transmits the low nibble, then high nibble of each packed byte.

Thus the original SysEx packed bytes map as follows:

| Packed byte | DK600 working RAM mirror | Meaning established |
| --- | --- | --- |
| 0 | C047 | Digital register R47 |
| 1 | C046 | Digital register R46 |
| 2 | C045 | Digital register R45 |
| 3 | C044 | Digital register R44 |
| 4..19 | C043..C034 | Analog slots 15..0, in reverse order |

The header `firmware/src/siel_factory_layout.h` provides `siel_factory_digital(program, index)` and `siel_factory_analog(program, slot)` accessors. These return original values only; they do not change the active patch engine.

To verify locally, run `python3 tools/verify_dk600_layout.py /path/to/dk600v10.zip` with the original user-provided ROM archive, and `cc -std=c11 -Wall -Wextra -Werror tests/test_siel_layout.c -o /tmp/sieltest && /tmp/sieltest`. CI tests all 95 programs without distributing ROM binaries.

## Remaining blocker for authentic playable sounds

The exact **names and polarity/scaling** of analog slots 0..15, and the individual bits of digital registers R44..R47, are not established by the storage routine. The supplied `DK600ccmap.pdf` documents MIDI controller numbers but **not** those hardware scan assignments. The five `calptches.syx` records duplicate programs 90..94 and do not provide independent single-parameter changes. Converting unlabelled bytes into FM1 `P_*` settings would fabricate factory sounds.

The shortest reliable next step is to trace the analog multiplexer input wiring and digital register bit assignments in the original SIEL schematics, or compare two genuine DK600 SysEx dumps where **one known front-panel control** was changed. Validate all 16 analog slots and digital flags, then implement and test the actual FM1 conversion. Do not call the existing source bank a playable factory preset set.

## v0.8 playable factory preview (29 October 2026)

**New:** `firmware/src/siel_factory_convert.h` converts all 95 original programs into editable FM1 patches. `obervave.c::prog_get()` now selects SIEL programs 00..94 (the five calibration/service programs 90..94 are marked `SVC`); 95..98 remain INIT. Saved user patches still take precedence. No changes were made to the working `main` / v0.7 branch. This is a **playable engineering preview**, not yet a validated reproduction of the original sound.

The original 1983 SIEL service schematic, p.11 (control panels) and p.16 (CPU DAC, 4051 analog mux/demux) gives the sixteen analog control-channel names. The ROM scan writes slot B to C034+B, B=0..15. Matching C1..C16 to scan slots gives:

| RAM slot | Analog channel | Physical control | FM1 preview parameter |
| --- | --- | --- | --- |
| 0 | C1 | LFO I/II depth | LFO pitch depth |
| 1 | C2 | LFO I/II speed | LFO rate (when pitch used) |
| 2 | C3 | LFO III depth | LFO PWM/filter depth |
| 3 | C4 | LFO III speed | LFO rate (when PWM/filter used) |
| 4 | C5 | DCO A pulse width | OSC1 PW |
| 5 | C6 | Noise level | Noise |
| 6 | C7 | DCO B pulse width | OSC2 PW |
| 7 | C8 | DCO B fine detune | OSC2 fine (estimated neutral 121) |
| 8 | C9 | DCO B coarse detune | OSC2 semitone offset (estimated -7..0) |
| 9 | C10 | ADSR attack | Amp/filter attack |
| 10 | C11 | ADSR decay | Amp/filter decay |
| 11 | C12 | ADSR sustain | Amp/filter sustain |
| 12 | C13 | ADSR release | Amp/filter release |
| 13 | C14 | VCF ADSR amount | Filter envelope depth |
| 14 | C15 | VCF cutoff | Filter cutoff |
| 15 | C16 | VCF resonance | Filter resonance |

**Cross-check:** SIEL service program 91 has cutoff slot14=145 and resonance slot15=252, consistent with the service procedure's resonant sine-wave calibration. Program 92 has attack slot9=252, consistent with its 5.8-second attack calibration. These are cross-checks, not proof of the exact response curves.

The ROM's digital switch handlers E3CB..E50E cycle field masks in R44..R46. The preview interprets R44 as two oscillator octave selectors plus LFO III waveform and DCO A waveform, R45 as DCO B waveform, half-level and LFO destinations, R46 as dynamics, ADSR routing and key tracking. This interpretation is **provisional** pending controlled one-knob SysEx comparisons or complete electrical tracing; no unverified field is represented as ROM-proven.

**Important audible limitations:** FM1 has one shared LFO where the original has two independently adjustable LFO rates; FM1 has no independent oscillator-A octave parameter; the raw 8-bit analog voltages are mapped to 7-bit controls with linear estimates, and the SSM filter, envelope and PWM circuits have different curves. Velocity-controlled attack time, wheel destinations and mixed LFO III waveforms are not emulated. Effects are disabled for these presets. Some service programs are not musical sounds. The new preview is useful for listening and comparing, but **do not describe it as an authentic, bit-exact 95-patch emulation**.

The v0.8 CI workflow `.github/workflows/opera-v08-preview.yml` compiles host tests for all 95 conversions and attempts to build `obervave.fwsc` for FM1. The artifact name explicitly contains `PREVIEW-NOT-VALIDATED`. Firmware should be tested with a recovery path and backed-up user patches before flashing.

Sources: [Original SIEL 1983 schematics](https://www.polynominal.com/siel-dk600/assets/files/Siel%20DK-600%20Schematic%20.pdf); [Tauntek DK600 MIDI CC list (user supplied DK600ccmap.pdf)](../data/siel_factory_packed.hex); [original DK600 review](https://www.muzines.co.uk/articles/siel-dk600/8028).
