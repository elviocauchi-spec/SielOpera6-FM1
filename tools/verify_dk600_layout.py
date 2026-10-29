#!/usr/bin/env python3
"""Check v10 TMS7000 ROM evidence for factory byte layout; no CC mapping.

Usage: python3 tools/verify_dk600_layout.py /path/to/dk600v10.zip
Original user-supplied ROM binaries are not redistributed.
"""
import hashlib
import sys
import zipfile

SHA256 = {
    "dk600v10E.bin": "19da47e9afaa069c564e12ba059b9bc55fa52533442bf6680149dc168c89ef54",
    "dk600v10F.bin": "2c3a93fbcebc511d80bed720551960704e04846cbb3434661fd0dc9df284b551",
}
# E1B5: store analog slot B to C020+B and C034+B.
# E54A/E557/E565/E58D: mirror four digital registers to C044..C047.
# E7D5: copy 20 bytes from C047 downwards to patch storage.
# F6B2: send low nibble then high nibble from each packed byte.
WINDOWS = {
    0xe1b5: "abc0205814abc020",
    0xe54a: "122c8ba0018bc0308bc044",
    0xe557: "122d8ba0028bc0318bc045",
    0xe565: "122e8ba003b68a60002680038ce86eb68bc0328bc046",
    0xe58d: "74012ff58bc0338bc047",
    0xe63c: "aac01fabc033caf2",
    0xe7d5: "5214aac0339b2bdb2bcaf70a",
    0xe7f9: "9a2babc033db2bcaf78ee8780a",
    0xe806: "5c1488c0a62b48012b49002a52140a",
    0xf6b2: "9a2b230f8ef6f19a2bd700230f8ef6f1db2bcaec",
}

def main(path):
    with zipfile.ZipFile(path) as archive:
        rom = bytearray([0xff] * 65536)
        for filename, addr in [("dk600v10E.bin", 0xe000), ("dk600v10F.bin", 0xf000)]:
            data = archive.read(filename)
            assert len(data) == 4096
            assert hashlib.sha256(data).hexdigest() == SHA256[filename], filename
            rom[addr:addr + len(data)] = data
    for addr, hexcode in WINDOWS.items():
        expected = bytes.fromhex(hexcode)
        assert rom[addr:addr+len(expected)] == expected, f"ROM mismatch at {addr:04x}"
    print("PASS: v10 ROM confirms four digital bytes and 16 analog bytes in reverse storage order")
    print("UNRESOLVED: analog slot and digital bit mapping to named synthesis controls")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("Usage: python3 tools/verify_dk600_layout.py dk600v10.zip")
    main(sys.argv[1])
