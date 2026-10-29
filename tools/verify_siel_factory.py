#!/usr/bin/env python3
"""Check original SIEL factory data against its exact, verified SysEx format."""
from pathlib import Path
import hashlib
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "data/siel_factory_packed.hex"
HEADER = ROOT / "firmware/src/siel_factory_bank.h"
SYSEX_SHA256 = "92a2cdf1133601bb1c1d6dc458e2ed7f8abafdfae1070ade9c56daf3349fe2c6"
PACKED_SHA256 = "34764c94081470b018b3fa5d80767b104cfa0403e30347493a3faf26c8be9dc8"

def verify():
    lines = SOURCE.read_text(encoding="ascii").splitlines()
    assert len(lines) == 95, "95 factory records required"
    assert all(re.fullmatch(r"[0-9a-f]{40}", line) for line in lines)
    records = [bytes.fromhex(line) for line in lines]
    packed = b"".join(records)
    assert hashlib.sha256(packed).hexdigest() == PACKED_SHA256
    messages = []
    for program, record in enumerate(records):
        nibbles = bytes(n for b in record for n in (b & 15, b >> 4))
        assert len(nibbles) == 40 and max(nibbles) < 16
        messages.append(bytes([0xb0, 0x57, 0x7f, 0xf0, 0x21, 0x01, program]) + nibbles + b"\xf7")
    reconstructed = b"".join(messages)
    assert len(reconstructed) == 4560
    assert hashlib.sha256(reconstructed).hexdigest() == SYSEX_SHA256
    generated = re.findall(r"/\* \d\d \*/ \{([^}]+)\},", HEADER.read_text(encoding="ascii"))
    assert len(generated) == 95, "C header must contain 95 indexed entries"
    for program, row in enumerate(generated):
        assert bytes(int(v.strip(), 16) for v in row.split(",")) == records[program], program
    print("PASS: 95 factory programs, full bank checksum, exact C header bytes")

if __name__ == "__main__":
    verify()
