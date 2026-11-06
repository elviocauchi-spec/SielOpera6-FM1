#!/usr/bin/env python3
"""Regression checks for the audible PAD LAB demo on FM-1.

These deliberately assert the musical intent at source level. They complement
the host chord C unit tests and target compilation, not hardware listening.
"""
from pathlib import Path
import re

presets = Path("firmware/src/obv_patches.h").read_text()
seq = Path("firmware/src/obv_seq.h").read_text()
app = Path("firmware/src/obervave.c").read_text()

def sound(name):
    found = re.search(
        rf"static const uint8_t {re.escape(name)}\[\]\s*=\s*\{{(.*?)\}};",
        presets, re.S
    )
    assert found, f"preset {name} missing"
    pairs = re.findall(r"\b(P_[A-Z0-9_]+)\s*,\s*([A-Z_0-9]+)\s*[,\n]", found.group(1))
    return dict(pairs)

b = sound("PF_PAD_MISTFLOW")
assert b.get("P_ARP_MODE") == "A_OFF", "no independent arp melody"
assert b.get("P_MODE") == "M_POLY", "B is a single-voice-per-note accompaniment"
assert int(b["P_AA"]) >= 65, "avoid clicky attack"
assert int(b["P_AR"]) >= 85, "soft tail"
assert int(b["P_AS"]) >= 90, "not a pluck with zero sustain"
assert int(b["P_FENV_AMT"]) == 64, "neutral filter envelope"
# Gain/presence correction: the old preview started B at level 80, which
# yields only 0.64x (80/100)^2 mixer gain before the patch/filter losses.
# Raising to 112 yields 1.25x nominal without altering the master.
assert "#define PAD_DEMO_B_LEVEL 112u" in app, "audition B default must be clearly audible"
assert "pad_demo_b_level = PAD_DEMO_B_LEVEL;" in app, "audition must reset to stronger B level"
assert "ctl_mix_lvl[1] = (uint8_t)pad_demo_b_level;" in app, "LEVEL knob must control the actual mixer"
assert "if (pad_demo_active)" in app, "preview must remain separate from saved song state"
assert 110 <= int(b["P_VOLUME"]) <= 118, "backing patch must have enough gain without going full-scale"
assert 60 <= int(b["P_CUTOFF"]) <= 70, "a warm filter with audible note definition"
assert int(b["P_O1_LVL"]) >= 90 and int(b["P_O2_LVL"]) >= 60
assert 68 <= int(b["P_AA"]) <= 78, "audible but click-free onset"
assert b.get("P_DRIVE") == "0", "do not use distortion to solve a level issue"
assert "MIST FLOW" in presets
assert "static seqtrk_t pad_demo_track = {8u, D_4," in seq, "use quarter-note resolution"
pat = re.search(
    r"static const step_t pad_demo_notes\[8\]\s*=\s*\{(.*?)\};",
    seq, re.S
)
assert pat
entries = re.findall(r"\{\.note=\{(\d+)u,0,0,0\}", pat.group(1))
assert entries == ["60", "67", "64", "72"], f"fixed C-major chord tone pattern only, got {entries}"
assert pat.group(1).count("{0}") == 4, "four rests in 8-step demo"
vels = [int(n) for n in re.findall(r"\.vel=(\d+)u", pat.group(1))]
assert len(vels) == 4 and min(vels) >= 85 and max(vels) <= 110, "soft but audible notes"
gates = [int(n) for n in re.findall(r"\.gate=(\d+)u", pat.group(1))]
assert len(gates) == 4 and all(g >= 110 and g < 127 for g in gates), "sustained notes without ties"
assert "if (pad_demo_active && t != 1u)" in seq, "no legacy note sequences in demo"
assert "if (!pad_demo_active) seq_auto(" in seq, "no legacy automation in demo"
assert "pad_demo_active && t == 1u" in seq, "dedicated B demo sequence"
assert "static uint32_t pad_b_unarmed(uint32_t t)" in seq, "backing must be keyboard gated"
assert "if (pad_b_unarmed(t)) {" in seq, "last-key release must end sounding B notes"
assert "seq_release_t(t);" in seq, "sequenced notes must release through envelope"
assert "r->tie = 0;" in seq and "r->wait = NO_WAIT;" in seq, "cancel pending notes/ties"
assert "r->play = !pad_b_unarmed(t) &&" in seq, "no new notes can be triggered without a triad"
assert "if (!count) {" in Path("firmware/src/pad_chord.h").read_text(), "clear harmony when the last key lifts"
assert "ctl_click = 0u;" in app, "force metronome off on entering demo"
assert "ctl_click = !pad_demo_active &&" in app, "prevent metronome reactivation"
assert "ctl_seq_run = 1u;" in app, "start backing transport"
assert "patch_from_factory(&live_p[0], &FACT_PAD_LAB[0])" in app
assert "patch_from_factory(&live_p[1], &FACT_PAD_LAB[3])" in app
assert "live_p[0] = pad_demo_saved[0]" in app, "restore A when leaving"
assert "live_p[1] = pad_demo_saved[1]" in app, "restore B when leaving"
print("PAD LAB atmospheric preview checks: PASS (hardware listening still needed)")
