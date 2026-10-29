/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X: musical oscillator extensions. Pure fixed point, no FPU, no runtime
 * table generation, and no new parameters in the original Opera 6 bank.
 *
 * SINE: 256-entry interpolated wavetable (linear, 24-bit phase fraction).
 * TRI: continuous triangle, low aliasing because harmonics decay as 1/n^2.
 * SUP SAW / SUP PLS: three independent polyBLEP oscillators with a shared
 * root pitch and two gently detuned side oscillators. PW knob = width.
 *
 * This is intentionally optional per oscillator. Classic waves 0..3 still
 * run through the original exact oscillator path.
 */
#ifndef OBV_X_WAVES_H
#define OBV_X_WAVES_H

static const int16_t opera_x_sine_tab[257] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739,
    9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530,
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594, 23170, 23731, 24279, 24811,
    25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956,
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521,
    32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285,
    32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571, 30273, 29956, 29621, 29268,
    28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731,
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151,
    15446, 14732, 14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179,
    6393, 5602, 4808, 4011, 3212, 2410, 1608, 804, 0, -804, -1608, -2410,
    -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793,
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159,
    -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
    -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113,
    -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580,
    -31356, -31113, -30852, -30571, -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
    -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403,
    -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011,
    -3212, -2410, -1608, -804, 0
};

/* 256 lookup samples + interpolation; avoids the obvious stair-step grit
 * in soft pads. Q15 signed output, with 32-bit-safe arithmetic. */
static inline int32_t opera_x_sine(uint32_t phase)
{
    uint32_t i = phase >> 24;
    int32_t a = opera_x_sine_tab[i], diff = opera_x_sine_tab[i + 1u] - a;
    return a + (int32_t)((diff * (int32_t)(phase & 0x00ffffffu)) >> 24);
}
static inline int32_t opera_x_triangle(uint32_t phase)
{
    if (phase & 0x80000000u)
        return 32767 - (int32_t)((phase - 0x80000000u) >> 15);
    return (int32_t)(phase >> 15) - 32768;
}

/* One 64-bit multiplication per control block, never in the sample loop.
 * Maximum spread approx +/-0.78% (13.4 cents) at width 127.
 * Pitch at zero width stays exactly in tune. */
static inline void opera_x_detuned_incs(uint32_t inc, uint32_t width, uint32_t *lo, uint32_t *hi)
{
    uint32_t delta = (uint32_t)(((uint64_t)inc * (width & 127u)) >> 14);
    *lo = inc > delta ? inc - delta : 1u;
    *hi = inc + delta < inc ? 0xffffffffu : inc + delta;
}

/* After a note begins, use two phase-offset side oscillators to get
 * immediate width even before the detuned beating develops. */
static inline void opera_x_init_aux(osc_t *l, osc_t *r, uint32_t phase)
{
    l->ph = phase + 0x15555555u;
    r->ph = phase - 0x15555555u;
    l->prev = 0;
    r->prev = 0;
}

/* Return the same approximate amplitude as the classic oscillator so
 * existing filter/drive compensation and limiter continue to work. */
static inline int32_t opera_x_osc_sample(osc_t *main, osc_t *side_l, osc_t *side_r,
                                        uint32_t inc, uint32_t inc_l, uint32_t inc_r,
                                        uint32_t pw, uint32_t wave, int32_t reset_t)
{
    if (wave == W_SINE || wave == W_TRI) {
        int32_t out = main->prev;
        main->ph = reset_t < 0 ? main->ph + inc : (inc >> 15) * (uint32_t)reset_t;
        main->prev = wave == W_SINE ? opera_x_sine(main->ph) : opera_x_triangle(main->ph);
        return out;
    }
    if (wave == W_SUPERSAW || wave == W_SUPERPULSE) {
        uint32_t base_wave = wave == W_SUPERSAW ? OSC_SAW : OSC_PLS;
        int32_t hard_wrap = -65536; /* saw AND pulse have one falling discontinuity */
        int32_t center = osc_step(main, inc, inc >> 15, pw, base_wave, hard_wrap, reset_t);
        int32_t left = osc_step(side_l, inc_l, inc_l >> 15, pw, base_wave, hard_wrap, reset_t);
        int32_t right = osc_step(side_r, inc_r, inc_r >> 15, pw, base_wave, hard_wrap, reset_t);
        return (center * 2 + left + right) >> 2;
    }
    return osc_step(main, inc, inc >> 15, pw, wave,
                    (wave & OSC_SAW ? -65536 : 0) + (wave & OSC_PLS ? -65536 : 0),
                    reset_t);
}
#endif /* OBV_X_WAVES_H */
