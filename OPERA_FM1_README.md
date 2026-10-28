# OPERA-FM1 v0.6 — OberVave adaptation (source only)

Based on user-supplied OberVave 3.0.1 source. GPL-3.0-only, retain upstream notices.

Changes:
- Six active synthesis voices (NV=6; original UI voice-card displays may still assume eight).
- First six factory sounds replaced by DK600-inspired starting patches: OP BRASS, OP STRING, OP BASS, OP PAD, OP LEAD, OP KEYS.
- All six use two oscillators, LP24, and disable effects.

These are **approximations**, not verified SIEL Opera 6 circuit emulations. The SSM2044/SSM2056 models and authentic DK600 parameter mapping remain to be developed.

Host test: `./tests/run_host.sh` (requires GCC and Pillow).
Target build: `./build.sh` (requires JieLi PI32v2 toolchain and AC79 SDK, see upstream README).
No target `.fwsc` was built or verified. The included `docs/firmware/obervave-3.0.1.fwsc` is the **unaltered upstream firmware**, NOT OPERA-FM1. Do not flash it expecting the new sounds.
