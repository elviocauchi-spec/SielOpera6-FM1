#!/usr/bin/env python3
"""Bit-for-bit comparison of original vs cached-envelope V-1 FM1 DSP.

The two programs compile the SAME firmware source with VG1_LEGACY_ENVELOPES
switched on or off. Check ALL 75 diagnostic WAV recordings, not just peaks.
The script intentionally needs only the Python standard library.
"""
from pathlib import Path
import hashlib
import sys

def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: vg1_compare_audio.py <legacy-wavs> <optimized-wavs>")
    left, right = (Path(p) for p in sys.argv[1:])
    files = sorted(p.relative_to(left) for p in left.rglob("*.wav"))
    other = sorted(p.relative_to(right) for p in right.rglob("*.wav"))
    if not files or files != other:
        raise RuntimeError(f"audio inventory mismatch: legacy={len(files)}, optimized={len(other)}")
    differences = []
    for rel in files:
        a = (left / rel).read_bytes()
        b = (right / rel).read_bytes()
        if hashlib.sha256(a).digest() != hashlib.sha256(b).digest():
            differing = sum(x != y for x, y in zip(a, b)) + abs(len(a) - len(b))
            differences.append((str(rel), differing))
    print(f"VG1 legacy vs optimized: compared {len(files)} complete 44.1kHz stereo WAVs")
    if differences:
        for f, count in differences[:20]:
            print(f"  DIFFERENT {f} ({count} differing bytes)")
        raise RuntimeError(f"{len(differences)} of {len(files)} WAVs differ from previous DSP")
    print("PASS: every PCM WAV is bit-for-bit identical to the pre-optimization envelope DSP.")

if __name__ == "__main__":
    main()
