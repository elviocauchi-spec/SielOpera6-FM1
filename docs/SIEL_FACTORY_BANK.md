# SIEL Opera 6 / DK600 factory bank — development v0.8

**Preserved original data; not yet playable FM1 patches.**

The source file `data/siel_factory_packed.hex` contains 95 original program records (00–94), each 20 packed bytes, reconstructed from the user's verified DK600 factory SysEx. The nibble order follows the original MIDI transmission (low nibble, then high nibble). The generated `firmware/src/siel_factory_bank.h` provides those 95 packed records and an indexed nibble reader.

`python3 tools/verify_siel_factory.py` checks exact data checksums and every C table byte. The independent GitHub Actions workflow also checks the bank.

**Critical limitation:** The MIDI CC map describes control numbers, not the locations of those controls in the original 20-byte patch records. The original storage-to-parameter mapping must be established before connecting these values to the FM1 synthesizer's native `P_*` controls. Until then, the factory bank is authentic source data, **not** an audible factory-preset emulation.

Original SysEx SHA-256: `92a2cdf1133601bb1c1d6dc458e2ed7f8abafdfae1070ade9c56daf3349fe2c6`.
