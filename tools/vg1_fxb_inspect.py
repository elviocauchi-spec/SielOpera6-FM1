#!/usr/bin/env python3
"""Inspect original Vanguard VST2 .fxp/.fxb exports without guessing parameters.

Supports the documented VST2 CcnK binary formats:
  FxCk: per-program float parameters   FPCh: opaque plugin program chunk
  FxBk: bank of FxCk/FPCh programs      FBCh: opaque plugin bank chunk

For regular parameter banks, export all numeric settings exactly to JSON
so that the ORIGINAL plugin's parameter index map can be calibrated.
Opaque plugin chunks are retained as hex digests and deliberately NOT
invented as equivalent VG-1 patch values. Only work with legitimately
obtained presets you are authorized to convert.

Usage:
  python3 tools/vg1_fxb_inspect.py path/to/Vanguard.fxb --json build/vanguard-bank.json
"""
from __future__ import annotations
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct

MAX_FILE_BYTES = 64 * 1024 * 1024
MAX_PROGRAMS = 16384
MAX_PARAMS = 32768
MAGIC = b"CcnK"
KINDS = (b"FxCk", b"FPCh", b"FxBk", b"FBCh")


class PresetFormatError(ValueError):
    pass


def uint32(buf: bytes, pos: int) -> int:
    if pos + 4 > len(buf):
        raise PresetFormatError(f"truncated integer at offset {pos}")
    return struct.unpack_from(">I", buf, pos)[0]


def parse(data: bytes, offset: int = 0, depth: int = 0):
    if depth > 2:
        raise PresetFormatError("nested FXB/FXP depth exceeded")
    if len(data) - offset < 28 or data[offset:offset+4] != MAGIC:
        raise PresetFormatError(f"not a VST2 CcnK preset/bank at byte {offset}")
    length = uint32(data, offset+4)
    end = offset + 8 + length
    if end > len(data) or end <= offset+28:
        raise PresetFormatError(f"invalid VST2 record length at {offset}: {length}")
    kind = data[offset+8:offset+12]
    if kind not in KINDS:
        raise PresetFormatError(f"unsupported FX magic {kind!r} at byte {offset}")
    version = uint32(data, offset+12)
    plugin_id = data[offset+16:offset+20].decode("latin1")
    plugin_version = uint32(data, offset+20)
    count = uint32(data, offset+24)
    result = {
        "kind": kind.decode("ascii"),
        "format_version": version,
        "plugin_id": plugin_id,
        "plugin_version": plugin_version,
        "count": count,
        "offset": offset,
        "length": end-offset
    }
    if kind in (b"FxCk", b"FPCh"):
        if offset + 56 > end:
            raise PresetFormatError("FXP program name truncated")
        result["name"] = data[offset+28:offset+56].split(b"\x00",1)[0].decode("latin1").strip()
        pos = offset + 56
        if kind == b"FxCk":
            if count > MAX_PARAMS or pos + 4*count > end:
                raise PresetFormatError("FXP parameter count exceeds record")
            values = struct.unpack_from(">" + "f"*count, data, pos) if count else ()
            if not all(math.isfinite(v) for v in values):
                raise PresetFormatError("FXP contains non-finite float parameters")
            result["parameters"] = [float(x) for x in values]
            result["normalized_0_to_1"] = all(0 <= x <= 1 for x in values)
        else:
            chunk_size = uint32(data, pos)
            if chunk_size > end-(pos+4):
                raise PresetFormatError("truncated opaque FXP chunk")
            chunk = data[pos+4:pos+4+chunk_size]
            result["opaque_chunk_bytes"] = chunk_size
            result["opaque_chunk_sha256"] = hashlib.sha256(chunk).hexdigest()
            result["conversion_status"] = "needs_Vanguard_specific_chunk_decoder"
    else:
        if count > MAX_PROGRAMS:
            raise PresetFormatError(f"invalid bank program count {count}")
        pos = offset + 156
        if pos > end:
            raise PresetFormatError("FXB bank header truncated")
        if kind == b"FxBk":
            programs = []
            for i in range(count):
                p = parse(data, pos, depth+1)
                if p["kind"] not in ("FxCk", "FPCh"):
                    raise PresetFormatError(f"bank program {i} is not a VST2 program")
                if p["plugin_id"] != plugin_id:
                    raise PresetFormatError(f"bank program {i} belongs to different plugin")
                programs.append(p)
                pos += p["length"]
            result["programs"] = programs
        else:
            chunk_size = uint32(data, pos)
            if chunk_size > end-(pos+4):
                raise PresetFormatError("truncated opaque FXB bank chunk")
            chunk = data[pos+4:pos+4+chunk_size]
            result["opaque_chunk_bytes"] = chunk_size
            result["opaque_chunk_sha256"] = hashlib.sha256(chunk).hexdigest()
            result["conversion_status"] = "needs_Vanguard_specific_chunk_decoder"
    return result


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("file", type=Path, help="Original VST2 Vanguard FXB or FXP")
    ap.add_argument("--json", type=Path, help="Path for extracted parameter records")
    args = ap.parse_args()
    if not args.file.is_file():
        ap.error("preset file does not exist")
    data = args.file.read_bytes()
    if len(data) > MAX_FILE_BYTES:
        ap.error("preset bank too large")
    bank = parse(data)
    if bank["length"] != len(data):
        raise PresetFormatError("file contains unrecognized trailing bytes")
    print(f"VST {bank['kind']}: ID={bank['plugin_id']!r}, plugin_version={bank['plugin_version']}, count={bank['count']}")
    for i, p in enumerate(bank.get("programs", [bank])):
        if p["kind"] == "FxCk":
            print(f"  {i+1:03d} {p['name'][:27]:27} parameters={p['count']}"
                  f" normalized={p['normalized_0_to_1']}")
        else:
            print(f"  {i+1:03d} {p.get('name','<whole-bank>')[:27]:27}"
                  f" opaque_bytes={p['opaque_chunk_bytes']} — decoder required")
    if args.json:
        args.json.parent.mkdir(parents=True,exist_ok=True)
        args.json.write_text(json.dumps(bank,indent=2,ensure_ascii=False)+"\n",encoding="utf8")
        print(f"Extracted source parameters to {args.json}")
    print("Note: parsing source is not conversion; no unverified parameter map is assumed.")


if __name__=="__main__":
    main()
