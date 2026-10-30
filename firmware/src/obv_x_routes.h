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

enum { OX_TO_ENV_AMT = 0, OX_TO_MORPH = 1 };

static inline uint32_t opera_x_route_slot(const uint8_t *v, uint32_t dest)
{
    return opera_x_expressive(v) ? (PATCH_BYTES - 2u + (dest != OX_TO_ENV_AMT))
                                : (PATCH_BYTES - 6u + (dest != OX_TO_ENV_AMT));
}
static inline uint8_t opera_x_route_get(const uint8_t *v, uint32_t dest)
{
    uint8_t raw;
    if (v[OX_ROUTE_TAG_AT] != OX_ROUTE_TAG) return 0u;
    raw = v[opera_x_route_slot(v, dest)];
    return raw <= 127u ? raw : 0u;
}
static inline void opera_x_route_set(uint8_t *v, uint32_t dest, uint8_t value)
{
    if (v[OX_ROUTE_TAG_AT] != OX_ROUTE_TAG) {
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

/* Filter envelope amount is a bipolar amount centered on 64 in the
 * existing engine; modulate its depth, NOT envelope ADSR times.
 * At knob 127: +/-24 control steps, about +/-36 semitones at full FENV.
 * At 0: exactly the original source value. LFO range: Q15. */
static inline int32_t opera_x_lfo_env_amount(int32_t base, int32_t lfo, uint8_t depth)
{
    int32_t change = (lfo * (int32_t)depth * 24) >> 22;
    int32_t amt = base + change;
    return amt < -64 ? -64 : amt > 63 ? 63 : amt;
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
