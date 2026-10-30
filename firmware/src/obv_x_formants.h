/* SPDX-License-Identifier: GPL-3.0-only */
/* Opera X v0.14: two absolute-frequency vowel formants.
 *
 * The v0.13 VOX wavetables change harmonic amplitudes at fixed harmonic
 * numbers. That makes a vowel change pitch with the fundamental, which is
 * why the sound was only vaguely voice-like. Actual vowel identity depends
 * largely on resonances at absolute frequencies (F1 and F2), which should
 * NOT move up an octave every time the played note does.
 *
 * Two musical, deliberately approximate voiced formants now follow the
 * 5-stage OO > OH > AH > EH > EE MORPH path. The existing VOX table remains
 * the excitation source. One pair of SVF coefficients is shared by ALL
 * voices playing the same part, recalculated once every 16 frames, with
 * only two new filter states per VOX voice (16 bytes; ~96 bytes at NV=6).
 *
 * Positions are approximate vowel formants, not language-specific measured
 * speech or recorded voices. Both filters are 12 dB SVF band-pass sections.
 * The formant stage is opt-in for the VOX oscillator only; all SIEL and
 * other Opera X sounds retain the old filter/audio behavior exactly.
 */
#ifndef OBV_X_FORMANTS_H
#define OBV_X_FORMANTS_H

static const uint16_t opera_x_vox_f1_q8[5] = {16650u, 17764u, 20201u, 19184u, 15967u};
static const uint16_t opera_x_vox_f2_q8[5] = {20836u, 21519u, 22292u, 23654u, 24994u};
/* Nominal Hz: F1 = 350/450/780/620/300 ; F2 = 900/1050/1250/1700/2300.
 * Stored as MIDI semitone number Q8, the native svf_coef cutoff unit. */

static inline int opera_x_vox_active(const uint8_t *v)
{
    return v[P_O1_WAVE] == W_VOX || v[P_O2_WAVE] == W_VOX;
}

static inline uint8_t opera_x_vox_position(const uint8_t *v,
                                           int32_t motion_a, int32_t motion_b)
{
    uint8_t base = v[v[P_O1_WAVE] == W_VOX ? P_O1_PW : P_O2_PW];
    uint8_t depth = opera_x_expressive(v) ? v[OX_TONE] : 0u;
    int32_t timbre = v[P_O1_WAVE] == W_VOX ? motion_a : -motion_a;
    return opera_x_wt_morph(base, timbre, motion_b, depth);
}

static inline uint16_t opera_x_formant_interp(const uint16_t *anchors, uint8_t morph)
{
    uint32_t step = opera_x_wt_stage(morph);
    uint32_t t = opera_x_wt_blend(morph);
    int32_t a = anchors[step], delta = (int32_t)anchors[step+1u] - a;
    return (uint16_t)(a + ((delta * (int32_t)t) >> 7));
}

static inline void opera_x_formant_targets(uint8_t morph, uint16_t *f1, uint16_t *f2)
{
    *f1 = opera_x_formant_interp(opera_x_vox_f1_q8, morph);
    *f2 = opera_x_formant_interp(opera_x_vox_f2_q8, morph);
}

/* A damping coefficient k of 2458 corresponds to roughly Q=1.7.
 * Moderate Q preserves keyboard playability and avoids narrow, whistling
 * peaks that would become harsh with many simultaneous notes. */
static inline void opera_x_formant_coeffs(uint8_t morph,
                                         svf_coef_t *c1, svf_coef_t *c2)
{
    uint16_t f1, f2;
    opera_x_formant_targets(morph, &f1, &f2);
    svf_coef(c1, f1, 2458);
    svf_coef(c2, f2, 2458);
}

/* Run only for the VOX sound, after the patch's main filter. A small dry
 * component keeps attacks intelligible; the two resonances reinforce vowel
 * shapes while the source's own harmonic MORPH remains audible.
 *
 * Filters run in *mono* before stereo-width is applied. Thus the texture
 * cannot randomly pan different notes as the F1/F2 peaks change.
 */
static inline int32_t opera_x_formant_render(int32_t y,
                                             const svf_coef_t *c1, const svf_coef_t *c2,
                                             svf_state_t *st1, svf_state_t *st2)
{
    int32_t in = clampi(y >> 2, -16000, 16000);
    int32_t l1,b1,h1,l2,b2,h2,wet;
    svf_step(c1, st1, in, &l1, &b1, &h1);
    svf_step(c2, st2, in, &l2, &b2, &h2);
    (void)l1; (void)h1; (void)l2; (void)h2;
    wet = (clampi(b1, -24000, 24000) + clampi(b2, -24000, 24000)) * 3 / 2;
    return clampi((y >> 2) + wet, -65000, 65000);
}
#endif /* OBV_X_FORMANTS_H */
