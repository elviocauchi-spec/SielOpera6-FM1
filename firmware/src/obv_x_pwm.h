/* SPDX-License-Identifier: GPL-3.0-only */
/* Opera X v0.17: audible LFO PWM and wide-oscillator movement.
 *
 * The legacy OberVave PWM depth uses d*d/127. At knob 43 this is
 * only 14/127, causing a barely perceptible duty-cycle shift. Original
 * SIEL and unmarked patches MUST retain this exact response.
 *
 * Opt-in Opera X patches instead use a linear depth: at 43, a pulse
 * moves by +/-8.5% of its period, not +/-2.8%. At full depth, it moves
 * by +/-25%. The original anti-aliased pulse oscillator is unchanged.
 *
 * SUP SAW has no pulse width to change. When PWM is routed to it,
 * treat the same LFO as stereo detune-width modulation instead:
 * at full depth the spread travels from 0 to its maximum setting,
 * while the center oscillator remains perfectly in tune.
 *
 * Runs once per 16-sample control block per active voice. The side
 * oscillator count, sample loop and 80-byte patch format stay intact.
 */
#ifndef OBV_X_PWM_H
#define OBV_X_PWM_H

/* A negative lfo*depth occurs every half-cycle. The old oscillator
 * used signed << 8, which is undefined in C for negative values.
 * Equivalent 64-bit multiplication keeps 0..127 SIEL PWM bit-exact. */
static inline uint32_t opera_x_pwm_phase(uint32_t base, int32_t mod)
{
    int64_t p = (int64_t)base + (int64_t)mod * 256;
    if (p < (1LL << 26)) p = 1LL << 26;
    if (p > (int64_t)0xFFFFFFFFu - (1LL << 26))
        p = (int64_t)0xFFFFFFFFu - (1LL << 26);
    return (uint32_t)p;
}

static inline uint32_t opera_x_pwm_depth(uint8_t dial, int expressive)
{
    return expressive ? (uint32_t)dial : ((uint32_t)dial * dial) >> 7;
}
static inline int opera_x_pwm_is_pulse(uint32_t wave)
{
    return wave == W_PLS || wave == W_BOTH || wave == W_SUPERPULSE;
}
static inline int opera_x_pwm_is_wide_saw(uint32_t wave)
{
    return wave == W_SUPERSAW;
}
static inline uint32_t opera_x_pwm_superwidth(uint32_t base, int32_t lfo, uint8_t depth)
{
    int32_t b = (int32_t)(base > 127u ? 127u : base);
    int32_t d = (int32_t)(depth > 127u ? 127u : depth);
    int32_t peak = lfo < -32768 ? -32768 : lfo > 32767 ? 32767 : lfo;
    int32_t range, adjusted;
    if (!d || !peak) return (uint32_t)b;
    if (peak >= 0) {
        range = ((127 - b) * d + 63) / 127;
        adjusted = b + ((peak * range + 16384) >> 15);
    } else {
        range = (b * d + 63) / 127;
        adjusted = b - (((-peak) * range + 16384) >> 15);
    }
    return (uint32_t)(adjusted < 0 ? 0 : adjusted > 127 ? 127 : adjusted);
}
#endif /* OBV_X_PWM_H */
