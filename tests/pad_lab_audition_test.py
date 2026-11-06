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
preview = app[app.index("static void pad_demo_apply_mix(void)"):app.index("static void pad_demo_enter(void)")]
assert "ctl_mix_lvl[0]" not in preview, "preview mixer must not reset A level"
assert "ctl_mix_pan[0]" not in preview, "preview mixer must not reset A pan"
assert "ctl_mix_cho[0]" not in preview, "preview mixer must not reset A sends"
assert "ctl_mix_lvl[1] = (uint8_t)pad_demo_b_level;" in preview
level = app[app.index("else if (k == 1u) {\n                    if (pad_demo_active)"):app.index("else if (k == 2u) { if (pad_demo_active)")]
assert "ctl_mix_lvl[1] = (uint8_t)pad_demo_b_level;" in level, "B knob modifies only B"
assert "pad_demo_apply_mix();" not in level, "B knob must not reinitialize other mixer channels"
assert "ctl_mix_lvl[0]" not in level
presets_block = app[app.index("if (pad_demo_active && (e = fm1_enc_take(ENC_PRESETS))"):app.index("} else if ((e = fm1_enc_take(ENC_PRESETS)) != 0 && save_armed)")]
assert "prog_get(pad_demo_a_prog, &live_p[0]);" in presets_block
assert "pad_demo_exit()" not in presets_block, "browse A while B continues arpeggiating"
assert "trk_load(" not in presets_block, "preset previews must not overwrite saved-song selection"

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
assert "pad_keys_held()" in seq, "gating must use the physical notes on A"
assert "pad_keys_pick(sr[t].step / 2u, &played)" in seq, "arp pitches must be sourced from actual held notes"
assert "pad_keys_note_held(n)" in seq, "release notes individually when released on keyboard"
assert "pad_chord_pitch" not in seq, "NO chord guessing or scale snapping"
assert "pad_keys_event(on, note)" in app, "keyboard should publish held notes"
assert "#include \"pad_keys.h\"" in app
assert "pad_chord_event" not in app, "NO legacy chord-harmonizer event path"
assert "if (pad_b_unarmed(t)) {" in seq, "last-key release must end sounding B notes"
assert "seq_release_t(t);" in seq, "sequenced notes must release through envelope"
assert "r->tie = 0;" in seq and "r->wait = NO_WAIT;" in seq, "cancel pending notes/ties"
assert "r->play = !pad_b_unarmed(t) &&" in seq, "no new notes can be triggered without a triad"
assert "pad_keys_slot = next;" in Path("firmware/src/pad_keys.h").read_text(), "publish exact held-key snapshots"
assert "ctl_click = 0u;" in app, "force metronome off on entering demo"
assert "ctl_click = !pad_demo_active &&" in app, "prevent metronome reactivation"
assert "ctl_seq_run = 1u;" in app, "start backing transport"
assert "patch_from_factory(&live_p[0], &FACT_PAD_LAB[0])" in app
assert "patch_from_factory(&live_p[1], &FACT_PAD_LAB[3])" in app
assert "live_p[0] = pad_demo_saved[0]" in app, "restore A when leaving"
assert "live_p[1] = pad_demo_saved[1]" in app, "restore B when leaving"
print("PAD LAB atmospheric preview checks: PASS (hardware listening still needed)")
