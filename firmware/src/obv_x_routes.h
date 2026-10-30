/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X v0.15 — editable LFO -> filter-envelope-amount and wave MORPH.
 *
 * PATCH_BYTES remains 80 and NPARAM remains 63; names still occupy 63..72.
 * 73 is an opt-in feature marker. The legacy 74..77 Opera X metadata and
 * 78..79 SIEL oscillator-footage marker MUST NOT move or be overwritten.
 *
 * For Opera X expressive patches (74==0xB9):
 *    78 = LFO -> envelope amount, 79 = LFO -> oscillator morph.
 * For original/SIEL patches (74 != 0xB9):
 *    74 = LFO -> envelope amount, 75 = LFO -> oscillator morph.
 *    78..79 stay untouched, including original SIEL footage 0xA6.
 *
 * A saved patch with no marker has both new routes DISABLED regardless of
 * what bytes 74..79 happen to contain. A first edit enables the marker and
 * zeroes just the two dedicated route bytes. All 0..127 values are legal.
 */
#ifndef OBV_X_ROUTES_H
#define OBV_X_ROUTES_H
#define OX_ROUTE_TAG_AT (PATCH_BYTES - 7u) /* 73 */
#define OX_ROUTE_TAG 0xC5u
#define OX_ENV_MORPH_TAG 0xC6u /* v0.18: new ENV>MORPH mode opt-in */

enum { OX_TO_ENV_AMT = 0, OX_TO_MORPH = 1 };

static inline uint32_t opera_x_route_slot(const uint8_t *v, uint32_t dest)
{
    return opera_x_expressive(v) ? (PATCH_BYTES - 2u + (dest != OX_TO_ENV_AMT))
                                : (PATCH_BYTES - 6u + (dest != OX_TO_ENV_AMT));
}
static inline uint8_t opera_x_route_get(const uint8_t *v, uint32_t dest)
{
    uint8_t raw;
    /* New v0.18 patches use 0xC6. Both tags share the exact same
     * LFO route storage so no user-saved LFO+ depth gets lost. */
    if (v[OX_ROUTE_TAG_AT] != OX_ROUTE_TAG &&
        v[OX_ROUTE_TAG_AT] != OX_ENV_MORPH_TAG) return 0u;
    raw = v[opera_x_route_slot(v, dest)];
    return raw <= 127u ? raw : 0u;
}
static inline void opera_x_route_set(uint8_t *v, uint32_t dest, uint8_t value)
{
    if (v[OX_ROUTE_TAG_AT] != OX_ROUTE_TAG &&
        v[OX_ROUTE_TAG_AT] != OX_ENV_MORPH_TAG) {
        uint32_t a = opera_x_route_slot(v, OX_TO_ENV_AMT);
        uint32_t b = opera_x_route_slot(v, OX_TO_MORPH);
        /* Validate where the patch stores its SIEL footage BEFORE
         * enabling or writing the new modulation marker. */
        v[a] = 0u;
        v[b] = 0u;
        v[OX_ROUTE_TAG_AT] = OX_ROUTE_TAG;
    }
    v[opera_x_route_slot(v, dest)] = value <= 127u ? value : 127u;
}

/* Filter-envelope intensity modulation that REMAINS audible in a pad.
 *
 * Original v0.15 applied a symmetric +/-24 amount swing, much of which
 * disappeared when a pad's filter was already near the top of its range.
 * v0.16 uses a deliberately asymmetric musical response:
 *   positive half-cycle: brighten by up to +12 ENV AMT steps;
 *   negative half-cycle: darken by up to -56 ENV AMT steps.
 *
 * Both are relative to the existing bipolar ENV AMT setting (center 64).
 * Depth 0 or LFO 0 is BIT-EXACTLY the original amount, preserving LFO delay
 * and legacy presets. The envelope (ADSR) still gates the entire effect:
 * when the filter envelope is at zero, this route does NOTHING, unlike
 * the separate direct FILTER cutoff LFO destination.
 *
 * All math is safe 32-bit and evaluated ONCE PER 16-FRAME CONTROL BLOCK.
 * Round to nearest to avoid a coarse low-end dead zone. Saturation avoids
 * filter-envelope amount wrapping or invalid negative parameter bytes.
 */
static inline int32_t opera_x_lfo_env_amount(int32_t base, int32_t lfo, uint8_t depth)
{
    int32_t delta, value;
    if (!depth || !lfo) return base;
    if (lfo > 32767) lfo = 32767;
    if (lfo < -32768) lfo = -32768;
    if (lfo >= 0)
        delta = ((lfo * (int32_t)depth * 12) + (1 << 21)) >> 22;
    else
        delta = -((((-lfo) * (int32_t)depth * 56) + (1 << 21)) >> 22);
    value = base + delta;
    return value < -64 ? -64 : value > 63 ? 63 : value;
}

/* Filter-envelope contribution is signed Q8 semitones. Centralizing
 * the equation lets host regressions exercise the SAME calculation used
 * by the realtime audio engine. 32768 * 64 * 390 < 2^31, so this is
 * safely computed with 32-bit multiplies and a final shift. */
static inline int32_t opera_x_env_cutoff_q8(int32_t env_q15, int32_t base,
                                           int32_t lfo, uint8_t depth)
{
    return (env_q15 * opera_x_lfo_env_amount(base, lfo, depth) * 390) >> 15;
}

/* A note's editable LFO can move the oscillator's spectral position.
 * At the manual endpoints (0/127), modulation tapers naturally to
 * zero: the knob always means the same exact spectrum at each end.
 * Never converts a non-wavetable oscillator into a wavetable.
 * Depth 127 gives up to +/-55 morph steps near center. */
static inline uint8_t opera_x_lfo_morph(uint8_t center, int32_t lfo, uint8_t depth)
{
    int32_t c = center > 127u ? 127 : center;
    int32_t room = c < 127 - c ? c : 127 - c;
    int32_t span = room * (int32_t)depth * 7 / (127 * 8);
    int32_t next = c + ((lfo * span) >> 15);
    return (uint8_t)(next < 0 ? 0 : next > 127 ? 127 : next);
}
#endif /* OBV_X_ROUTES_H */
