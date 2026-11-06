#!/usr/bin/env python3
"""Synthetic VST2 fixtures prove importer's byte order/lengths and fail-closed behavior."""
import importlib.util
from pathlib import Path
import struct
import tempfile

module = Path(__file__).resolve().parents[1] / "tools" / "vg1_fxb_inspect.py"
spec = importlib.util.spec_from_file_location("fxbinspect", module)
assert spec and spec.loader
importer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(importer)

def word(x): return struct.pack(">I", x)

def record(kind, body, count, id=b"VG1!"):
    raw = kind + word(1) + id + word(0x10601) + word(count) + body
    return b"CcnK" + word(len(raw)) + raw

def fxp(name, params, id=b"VG1!"):
    name = name.encode("ascii")[:27].ljust(28,b"\0")
    return record(b"FxCk", name+b"".join(struct.pack(">f",f) for f in params),len(params),id)

def test():
    prg=fxp("Original One",(0.0,0.25,1.0))
    v=importer.parse(prg)
    assert v["kind"]=="FxCk" and v["name"]=="Original One"
    assert v["parameters"]==[0.0,0.25,1.0] and v["normalized_0_to_1"]
    prg2=fxp("Original Two",(0.6,0.3))
    bank=record(b"FxBk",b"\x00"*128+prg+prg2,2)
    b=importer.parse(bank)
    assert b["kind"]=="FxBk" and len(b["programs"])==2
    assert b["programs"][1]["name"]=="Original Two"
    chunk=record(b"FPCh",b"Opaque".ljust(28,b"\x00")+word(6)+b"ABCDEF",1)
    assert importer.parse(chunk)["opaque_chunk_bytes"]==6
    opaque_bank=record(b"FBCh",b"\x00"*128+word(4)+b"DATA",2)
    assert importer.parse(opaque_bank)["conversion_status"]=="needs_Vanguard_specific_chunk_decoder"
    corrupted=bytearray(bank)
    corrupted[4:8]=word(0xffffffff)
    for baddata in (bytes(corrupted),bank[:-2],b"ZZZZ"+bank[4:]):
        try: importer.parse(baddata)
        except importer.PresetFormatError: pass
        else: raise AssertionError("Malformed VST2 record was accepted")
    other=fxp("Wrong ID",(.5,),b"NOPE")
    badbank=record(b"FxBk",b"\0"*128+prg+other,2)
    try: importer.parse(badbank)
    except importer.PresetFormatError: pass
    else: raise AssertionError("Mismatched plugin IDs were accepted")
    print("PASS: VST2 FxCk/FxBk/FPCh/FBCh parsing, 2 original names/values, malformed and cross-plugin rejection")

if __name__ == "__main__":
    test()
