#!/usr/bin/env python3
"""Vanguard 1 FM1 offline audio QC; stdlib only.

Usage:
  python3 tools/vg1_audio_report.py build/vg1-audio
  python3 tools/vg1_audio_report.py build/vg1-audio --references ./my-vanguard-renders

The first call uses the *exact* VG-1 firmware DSP and reports measured output.
References are optional legally created captures from an installed reFX Vanguard.
No reference captures or presets are bundled or automatically downloaded.
Outputs audio_report.csv and reference_comparison.csv (when provided).
"""
from __future__ import annotations

import argparse
from array import array
import csv
import hashlib
import math
from pathlib import Path
import sys
import wave

SR = 44100
FREQS = (110, 164, 220, 330, 440, 660, 880, 1100,
         1320, 1760, 2200, 2640, 3300, 4400, 6600, 8800)


def load_wav(path: Path):
    with wave.open(str(path), "rb") as w:
        if w.getframerate() != SR or w.getnchannels() != 2 or w.getsampwidth() != 2:
            raise ValueError(f"{path}: expected stereo/16-bit/44100Hz WAV")
        raw = w.readframes(w.getnframes())
    samples = array("h")
    samples.frombytes(raw)
    if sys.byteorder != "little":
        samples.byteswap()
    left = samples[0::2]
    right = samples[1::2]
    return raw, left, right


def spectral_fingerprint(samples: array) -> tuple[float, ...]:
    # Goertzel energies, calibrated per fixed 4096-sample window. A spectral
    # fingerprint is NOT a substitute for a detailed FFT or listening test.
    offset = min(len(samples)//4, SR//5)
    count = min(4096, len(samples)-offset)
    if count <= 0:
        return (0.0,)*len(FREQS)
    block = samples[offset:offset+count]
    weights = []
    for hz in FREQS:
        harmonic_index = round(count*hz/SR)
        w = 2*math.pi*harmonic_index/count
        coeff = 2*math.cos(w)
        q0 = q1 = q2 = 0.0
        for value in block:
            q0 = value + coeff*q1 - q2
            q2, q1 = q1, q0
        power = max(0.0, q1*q1+q2*q2-coeff*q1*q2)
        weights.append(math.sqrt(power))
    magnitude = math.sqrt(sum(z*z for z in weights))
    return tuple(z/magnitude for z in weights) if magnitude else (0.0,)*len(FREQS)


def info(path: Path):
    raw, l, r = load_wav(path)
    n = len(l)
    if not n:
        raise ValueError(f"{path}: empty audio")
    sq_l = sum(int(x)*x for x in l)
    sq_r = sum(int(x)*x for x in r)
    avg = (sq_l+sq_r)/(2*n)
    rms = math.sqrt(avg)/32768
    peak = max(max(abs(x) for x in l), max(abs(x) for x in r))/32768
    corr_denom = math.sqrt(sq_l*sq_r)
    corr = sum(int(a)*b for a,b in zip(l,r))/corr_denom if corr_denom else 0
    clips = sum(1 for x in l if abs(x)>=32766)+sum(1 for x in r if abs(x)>=32766)
    zcr = sum((l[i]<0)!=(l[i-1]<0) for i in range(1,n))/max(n-1,1)
    signature = spectral_fingerprint(l)
    # Envelope differences also matter: avoid treating similar spectrum as
    # proof that two plucks/pads sound the same.
    envelope = []
    for k in range(8):
        a=k*n//8
        b=(k+1)*n//8
        energy=sum(int(x)*x for x in l[a:b])
        envelope.append(math.sqrt(energy/max(b-a,1))/32768)
    maximum = max(envelope) or 1
    envelope = [e/maximum for e in envelope]
    return {
        "file": path.name, "frames": n, "seconds": round(n/SR,3),
        "peak_dbfs": round(20*math.log10(max(peak,1e-12)),2),
        "rms_dbfs": round(20*math.log10(max(rms,1e-12)),2),
        "clip_samples": clips,
        "stereo_correlation": round(corr,4),
        "zero_crossings_per_sample": round(zcr,5),
        "sha256_pcm": hashlib.sha256(raw).hexdigest(),
        "fingerprint": signature,
        "envelope": envelope,
    }


def cosine(a,b):
    norm1 = math.sqrt(sum(x*x for x in a))
    norm2 = math.sqrt(sum(x*x for x in b))
    return sum(x*y for x,y in zip(a,b))/norm1/norm2 if norm1 and norm2 else 0


def report(folder: Path, references: Path|None):
    files = sorted(folder.rglob("*.wav"))
    if not files:
        raise ValueError(f"No WAV files found beneath {folder}")
    entries = []
    silence = []
    for path in files:
        data = info(path)
        data["group"] = path.parent.name
        data["file"] = str(path.relative_to(folder))
        entries.append(data)
        if data["rms_dbfs"] < -65:
            silence.append(data["file"])
    rows = [{k:v for k,v in e.items() if k not in ("fingerprint","envelope")}
            for e in entries]
    with (folder/"audio_report.csv").open("w",newline="",encoding="utf-8") as f:
        w=csv.DictWriter(f,fieldnames=list(rows[0]))
        w.writeheader()
        w.writerows(rows)
    print(f"VG-1 offline audio QC: {len(entries)} recordings; report={folder/'audio_report.csv'}")
    if silence:
        print("SILENT renders:",", ".join(silence))
    clipped = [e for e in entries if e["clip_samples"]]
    print(f"Hard-clipped renders: {len(clipped)}")
    for e in sorted(clipped,key=lambda v:v["clip_samples"],reverse=True)[:6]:
        print(f"  {e['file']} ({e['clip_samples']} clipped samples)")
    presets = [e for e in entries if e["group"]=="presets"]
    duplicates=[]
    for i,a in enumerate(presets):
        for b in presets[i+1:]:
            if a["sha256_pcm"]==b["sha256_pcm"]:
                duplicates.append((a["file"],b["file"]))
            spectral = cosine(a["fingerprint"],b["fingerprint"])
            env = cosine(a["envelope"],b["envelope"])
            if spectral>.9999 and env>.9999 and abs(a["rms_dbfs"]-b["rms_dbfs"])<.15:
                print(f"NEAR-IDENTICAL PRESET WARNING: {a['file']} vs {b['file']}")
    if references:
        comparisons=[]
        for e in entries:
            other=references/e["file"]
            if not other.is_file():
                continue
            ref=info(other)
            comparisons.append({
                "file":e["file"],"reference":str(other),
                "rms_delta_db":round(e["rms_dbfs"]-ref["rms_dbfs"],2),
                "peak_delta_db":round(e["peak_dbfs"]-ref["peak_dbfs"],2),
                "spectrum_similarity":round(cosine(e["fingerprint"],ref["fingerprint"]),4),
                "envelope_similarity":round(cosine(e["envelope"],ref["envelope"]),4),
                "VG1_stereo_corr":e["stereo_correlation"],
                "ref_stereo_corr":ref["stereo_correlation"],
            })
        if comparisons:
            with (folder/"reference_comparison.csv").open("w",newline="",encoding="utf-8") as f:
                w=csv.DictWriter(f,fieldnames=list(comparisons[0]))
                w.writeheader()
                w.writerows(comparisons)
        print(f"Original Vanguard reference matches: {len(comparisons)}")
        if not comparisons:
            print("No matching WAV names; references must mirror the render subfolders.")
    if duplicates or silence or clipped:
        if duplicates: print(f"Identical preset renders: {duplicates}")
        raise RuntimeError("Audio QC failed: silence, digital clipping, or byte-identical factory programs")
    print("PASS: all WAVs audible, zero clipped samples, and factory programs are distinct.")
    print("NOTE: Tests do not claim bit-accurate reFX matching. Use --references for real A/B metrics.")


if __name__=="__main__":
    parser=argparse.ArgumentParser()
    parser.add_argument("folder",type=Path)
    parser.add_argument("--references",type=Path)
    args=parser.parse_args()
    report(args.folder,args.references)
