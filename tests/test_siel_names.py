#!/usr/bin/env python3
"""Verify 00..89 SIEL label mapping and ten-character FM1 display names."""
import csv
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
with (root / "data/siel_factory_names.csv").open(newline="", encoding="utf-8") as f:
    rows = list(csv.DictReader(f))
assert len(rows) == 90
header = (root / "firmware/src/siel_factory_names.h").read_text(encoding="utf-8")
body = header.split("siel_factory_display_names[90] = {", 1)[1].split("};", 1)[0]
entries = re.findall(r'/\*\s*(\d{2})\s*\*/\s*"([^"]+)"', body)
assert len(entries) == 90
for i, (row, (num, name)) in enumerate(zip(rows, entries)):
    number = f"{i:02d}"
    assert row["program"] == num == number, (i, num)
    assert row["display_name"] == name, number
    assert 0 < len(name) <= 10 and name.isupper(), number
    assert row["original_label"], number
assert len({row["display_name"] for row in rows}) == 90
for num, label in {"00": "BRASS I", "09": "SYNTH BASS",
                   "41": "SUBDUET STRINGS", "63": "SYNTH - CORD",
                   "83": "LEAD SYNTH", "84": "METAL GUITAR",
                   "89": "FREE AREA"}.items():
    assert rows[int(num)]["original_label"] == label, num
print("SIEL Opera 6: all 90 factory program names and display mappings verified")
