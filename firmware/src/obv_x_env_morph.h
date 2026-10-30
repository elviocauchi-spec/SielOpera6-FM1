/* SPDX-License-Identifier: GPL-3.0-only */
/* Opera X v0.18: musical filter-ADSR -> spectral/formant MORPH.
 *
 * Reuse the original P_ENV_O2 bipolar parameter when an EXPERIMENTAL
 * Opera X patch contains a spectral or vocal wavetable oscillator.
 * 64 is off; lower values open toward the darker 0 endpoint; higher
 * values open toward the brighter 127 endpoint. In this mode it no
 * longer modulates OSC 2 pitch. Vintage SIEL and all normal analog
 * patches retain EXACTLY their original ENV>O2 pitch functionality.
 *
 * No patch bytes or CC numbers added. Signed Q15 envelope, a few
 * fixed-point operations PER CONTROL BLOCK, no audio-loop cost.
 */
#ifndef OBV_X_ENV_MORPH_H
#define OBV_X_ENV_MORPH_H

static inline int opera_x_env_morph_enabled(const uint8_t *patch)
{
    if (!opera_x_expressive(patch)) return 0;
    return patch[P_O1_WAVE] == W_SPECTRAL ||
           patch[P_O1_WAVE] == W_VOX ||
           patch[P_O2_WAVE] == W_SPECTRAL ||
           patch[P_O2_WAVE] == W_VOX;
}

/* Move toward the selected end of the five-stage table according to
 * the filter envelope's evolving ADSR. Each end stays bounded and the
 * fully positive/negative route cannot push past 127/0.
 * The manual MORPH value is always the baseline when env=0 (and after
 * a filter release finishes), including during note-on with long ATTACK.
 * round-to-nearest avoids a coarse dead zone for small changes. */
static inline uint8_t opera_x_env_morph_apply(uint8_t center,
                                               int32_t env_q15,
                                               uint8_t route)
{
    int32_t c = center > 127u ? 127 : center;
    int32_t amount = (int32_t)route - 64;
    int32_t available = amount < 0 ? c : 127 - c;
    int32_t scaled, next;
    if (!amount || env_q15 <= 0 || !available) return (uint8_t)c;
    if (env_q15 > 32768) env_q15 = 32768;
    /* Signed parameter range is -64..+63, so the positive and
     * negative halves require different normalizers to reach both
     * endpoints EXACTLY. Products stay below 267 million (int32 safe).
     * Division is evaluated once per 16-frame control block only. */
    if (amount > 0)
        scaled = (env_q15 * amount * available + 63 * 16384) / (63 * 32768);
    else
        scaled = (env_q15 * -amount * available + 64 * 16384) >> 21;
    next = c + (amount < 0 ? -scaled : scaled);
    return (uint8_t)(next < 0 ? 0 : next > 127 ? 127 : next);
}
#endif /* OBV_X_ENV_MORPH_H */
