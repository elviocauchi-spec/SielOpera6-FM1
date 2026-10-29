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
