/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X first playable sounds. Slots 95..98 are the four formerly empty
 * INIT programs. Saved user programs still override these factory previews.
 * All original SIEL 00..94 packed programs, their names and conversions are
 * untouched. The effects are shared mixer returns, not patch-local sends.
 */
#ifndef OBV_X_PATCHES_H
#define OBV_X_PATCHES_H

static void opera_x_factory_patch(patch_t *p, uint8_t index)
{
    static const char *const names[4] = {
        "X CLOUDPAD", "X SILKSTR", "X HOTLEAD", "X DEEPBASS"
    };
    unsigned i;
    patch_defaults(p);
    if (index >= 4u) return;

    /* Shared baseline: useful velocity response, modest analog variation.
     * Headroom remains for the master mixer and the chorus/reverb returns. */
    p->v[P_O2_FINE] = 64u;
    p->v[P_KEYTRK] = 70u;
    p->v[P_VEL_AMP] = 65u;
    p->v[P_FTYPE] = F_LP24;
    p->v[P_VINTAGE] = 23u;
    p->v[P_SPREAD] = 70u;
    p->v[P_VOLUME] = 92u;

    switch (index) {
    case 0: /* Dreamy, gently animated wide pad */
        p->v[P_O1_WAVE] = W_SUPERSAW;
        p->v[P_O1_PW] = 48u;          /* SUP SAW: detune width */
        p->v[P_O1_LVL] = 102u;
        p->v[P_O2_WAVE] = W_TRI;
        p->v[P_O2_LVL] = 72u;
        p->v[P_O2_SEMI] = 24u;
        p->v[P_O2_FINE] = 66u;
        p->v[P_CUTOFF] = 86u;
        p->v[P_RESO] = 18u;
        p->v[P_FENV_AMT] = 75u;
        p->v[P_FA] = 74u; p->v[P_FD] = 81u;
        p->v[P_FS] = 101u; p->v[P_FR] = 104u;
        p->v[P_AA] = 74u; p->v[P_AD] = 76u;
        p->v[P_AS] = 115u; p->v[P_AR] = 100u;
        p->v[P_LFO_SHAPE] = L_TRI;
        p->v[P_LFO_RATE] = 39u;
        p->v[P_LFO_FLT] = 14u;
        p->v[P_VEL_AMP] = 55u;
        p->v[P_SPREAD] = 105u;
        break;
    case 1: /* Soft, moving synthetic strings: one wide PWM source */
        p->v[P_O1_WAVE] = W_SUPERPULSE;
        p->v[P_O1_PW] = 53u;          /* pulse width and detune */
        p->v[P_O1_LVL] = 112u;
        p->v[P_O2_WAVE] = W_TRI;
        p->v[P_O2_LVL] = 57u;
        p->v[P_O2_SEMI] = 36u;
        p->v[P_O2_FINE] = 62u;
        p->v[P_CUTOFF] = 88u;
        p->v[P_RESO] = 16u;
        p->v[P_FENV_AMT] = 81u;
        p->v[P_FA] = 64u; p->v[P_FD] = 82u;
        p->v[P_FS] = 93u; p->v[P_FR] = 94u;
        p->v[P_AA] = 63u; p->v[P_AD] = 67u;
        p->v[P_AS] = 115u; p->v[P_AR] = 88u;
        p->v[P_LFO_SHAPE] = L_TRI;
        p->v[P_LFO_RATE] = 50u;
        p->v[P_LFO_PWM] = 26u;
        p->v[P_LFO_PWM_TO] = TO_O1;
        p->v[P_VINTAGE] = 35u;
        p->v[P_SPREAD] = 90u;
        break;
    case 2: /* Expressive mono lead with real-time filter bite and glide */
        p->v[P_O1_WAVE] = W_SUPERSAW;
        p->v[P_O1_PW] = 66u;
        p->v[P_O1_LVL] = 120u;
        p->v[P_O2_WAVE] = W_SAW;
        p->v[P_O2_LVL] = 73u;
        p->v[P_O2_SEMI] = 12u;
        p->v[P_O2_FINE] = 70u;
        p->v[P_CUTOFF] = 83u;
        p->v[P_RESO] = 33u;
        p->v[P_FENV_AMT] = 108u;
        p->v[P_FA] = 7u; p->v[P_FD] = 52u;
        p->v[P_FS] = 58u; p->v[P_FR] = 35u;
        p->v[P_AA] = 5u; p->v[P_AD] = 70u;
        p->v[P_AS] = 116u; p->v[P_AR] = 38u;
        p->v[P_FTYPE] = F_LP12;
        p->v[P_DRIVE] = 17u;
        p->v[P_MODE] = M_MONO;
        p->v[P_GLIDE] = 41u;
        p->v[P_VEL_FLT] = 82u;
        p->v[P_VINTAGE] = 15u;
        p->v[P_VOLUME] = 78u;
        break;
    default: /* Deep, controlled bass: clean sine underneath a buzzy pulse */
        p->v[P_O1_WAVE] = W_SINE;
        p->v[P_O1_LVL] = 120u;
        p->v[P_O2_WAVE] = W_SUPERPULSE;
        p->v[P_O2_PW] = 36u;
        p->v[P_O2_LVL] = 69u;
        p->v[P_O2_SEMI] = 12u;
        p->v[P_CUTOFF] = 71u;
        p->v[P_RESO] = 19u;
        p->v[P_FENV_AMT] = 103u;
        p->v[P_FA] = 4u; p->v[P_FD] = 48u;
        p->v[P_FS] = 58u; p->v[P_FR] = 31u;
        p->v[P_AA] = 3u; p->v[P_AD] = 47u;
        p->v[P_AS] = 101u; p->v[P_AR] = 27u;
        p->v[P_DRIVE] = 16u;
        p->v[P_KEYTRK] = 64u;
        p->v[P_MODE] = M_MONO;
        p->v[P_SPREAD] = 20u;
        p->v[P_VINTAGE] = 11u;
        p->v[P_VOLUME] = 87u;
        break;
    }
    for (i = 0u; i < PATCH_NAME && names[index][i]; ++i)
        p->v[NPARAM + i] = (uint8_t)names[index][i];
    for (; i < PATCH_NAME; ++i)
        p->v[NPARAM + i] = (uint8_t)' ';
}
#endif /* OBV_X_PATCHES_H */
