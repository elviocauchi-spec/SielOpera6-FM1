#!/usr/bin/env python3
"""Compare host CPU times and bit-identical checksums; not a Jieli clock emulation."""
import re
import statistics
import sys
from pathlib import Path

ROW = re.compile(r"VG1 BENCH (?P<name>.+?)\s+voices=(?P<voices>\d+) "
                 r"FAT=\d+ audio_sec=\d+ host_cpu_ms=(?P<ms>[\d.]+) "
                 r"checksum=(?P<checksum>\d+)")

def load(path):
    data = {}
    for m in ROW.finditer(Path(path).read_text()):
        name = m["name"].strip()
        key = (name, int(m["voices"]))
        data[key] = (float(m["ms"]), m["checksum"])
    return data

def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: vg1_cpu_benchmark_report.py old.log new.log")
    old, new = load(sys.argv[1]), load(sys.argv[2])
    if not old or old.keys() != new.keys():
        raise RuntimeError("Missing comparable CPU timing rows")
    ratios = []
    for key in sorted(old):
        before, old_checksum = old[key]
        after, new_checksum = new[key]
        if old_checksum != new_checksum:
            raise RuntimeError(f"PCM benchmark checksum differs: {key}")
        ratio = before/after if after else float("inf")
        ratios.append(ratio)
        print(f"{key[0]:14s} {key[1]} voices: original {before:.2f} ms CPU, "
              f"optimized {after:.2f} ms, ratio {ratio:.2f}x")
    print(f"Host median relative speedup: {statistics.median(ratios):.2f}x "
          "(not a prediction of FM1 CPU load or realtime safety)")
    print("PASS: bench audio checksums identical for every chord")

if __name__ == "__main__":
    main()
