/* SPDX-License-Identifier: GPL-3.0-only */
/* Opera X: gain staging for the parallel drive send.
 * Previous DSP used a "return gain" around 4000 but divided by 32768:
 * the supposed full-level wet return was actually only 12% of full scale.
 * Use Q15 consistently, and keep L/R wet processing independent.
 */
#ifndef OBV_X_DRIVE_H
#define OBV_X_DRIVE_H

static inline int32_t opera_x_drive_return_gain(uint8_t wet)
{
    return (int32_t)((uint32_t)wet * 25500u / 127u); /* Q15: max ~78% */
}

static inline int32_t opera_x_drive_pre_gain(uint8_t amount)
{
    return 256 + (int32_t)(24u * 256u * (uint32_t)amount / 127u);
}

/* Saturating waveshapers, independent of the audio mix and wet/dry gains.
 * ty=3 (CRUSH) keeps its existing shared hold clock elsewhere. */
static inline int32_t opera_x_drive_shape(int32_t x, uint32_t ty)
{
    switch (ty) {
    case 0u: return softclip(x); /* overdrive */
    case 1u: return x > 0 ? softclip(x * 8 / 5) : (softclip(x * 7 / 10) * 4) / 5;
    case 2u: return clampi(x * 40, -32767, 32767); /* fuzz */
    default: return softclip(x);
    }
}
#endif /* OBV_X_DRIVE_H */
