/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X v0.10 revised musical sounds. Slots 95..98 are the four formerly empty
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
    /* v0.10: independent slow tone modulation, real stereo wide-oscillator
     * component. Unmarked v0.9 and legacy SIEL programs are untouched. */
    p->v[OX_MAGIC] = OX_TAG;
    p->v[OX_RATE] = 36u;
    p->v[OX_TONE] = 46u;
    p->v[OX_WIDTH] = 48u;

    switch (index) {
    case 0: /* Dreamy, gently animated wide pad */
        p->v[P_O1_WAVE] = W_SUPERSAW;
        p->v[P_O1_PW] = 73u;          /* wider beating SUP SAW */
        p->v[P_O1_LVL] = 118u;
        p->v[P_O2_WAVE] = W_TRI;
        p->v[P_O2_LVL] = 77u;
        p->v[P_O2_SEMI] = 24u;
        p->v[P_O2_FINE] = 66u;
        p->v[P_CUTOFF] = 84u;
        p->v[P_RESO] = 27u;
        p->v[P_FENV_AMT] = 91u;
        p->v[P_FA] = 86u; p->v[P_FD] = 92u;
        p->v[P_FS] = 98u; p->v[P_FR] = 113u;
        p->v[P_AA] = 77u; p->v[P_AD] = 81u;
        p->v[P_AS] = 115u; p->v[P_AR] = 108u;
        p->v[P_LFO_SHAPE] = L_TRI;
        p->v[P_LFO_RATE] = 39u;
        p->v[P_LFO_PITCH] = 4u;  /* vibrato from legacy LFO, tone motion is independent */
        p->v[P_LFO_FLT] = 12u;
        p->v[P_VEL_AMP] = 55u;
        p->v[P_SPREAD] = 105u;
        p->v[P_VOLUME] = 89u;
        p->v[OX_RATE] = 25u;  /* slow arc */
        p->v[OX_TONE] = 72u;
        p->v[OX_WIDTH] = 114u;
        break;
    case 1: /* Soft, moving synthetic strings: one wide PWM source */
        p->v[P_O1_WAVE] = W_SUPERPULSE;
        p->v[P_O1_PW] = 62u;          /* pulse width and detune */
        p->v[P_O1_LVL] = 117u;
        p->v[P_O2_WAVE] = W_TRI;
        p->v[P_O2_LVL] = 75u;
        p->v[P_O2_SEMI] = 36u;
        p->v[P_O2_FINE] = 62u;
        p->v[P_CUTOFF] = 92u;
        p->v[P_RESO] = 16u;
        p->v[P_FENV_AMT] = 81u;
        p->v[P_FA] = 68u; p->v[P_FD] = 89u;
        p->v[P_FS] = 93u; p->v[P_FR] = 94u;
        p->v[P_AA] = 69u; p->v[P_AD] = 76u;
        p->v[P_AS] = 115u; p->v[P_AR] = 88u;
        p->v[P_LFO_SHAPE] = L_TRI;
        p->v[P_LFO_RATE] = 50u;
        p->v[P_LFO_PWM] = 39u;
        p->v[P_LFO_PWM_TO] = TO_O1;
        p->v[P_VINTAGE] = 35u;
        p->v[P_SPREAD] = 98u;
        p->v[OX_RATE] = 40u;
        p->v[OX_TONE] = 50u;
        p->v[OX_WIDTH] = 100u;
        break;
    case 2: /* Expressive mono lead with real-time filter bite and glide */
        p->v[P_O1_WAVE] = W_SUPERSAW;
        p->v[P_O1_PW] = 83u;
        p->v[P_O1_LVL] = 120u;
        p->v[P_O2_WAVE] = W_SAW;
        p->v[P_O2_LVL] = 88u;
        p->v[P_O2_SEMI] = 12u;
        p->v[P_O2_FINE] = 70u;
        p->v[P_CUTOFF] = 89u;
        p->v[P_RESO] = 33u;
        p->v[P_FENV_AMT] = 117u;
        p->v[P_FA] = 7u; p->v[P_FD] = 52u;
        p->v[P_FS] = 58u; p->v[P_FR] = 35u;
        p->v[P_AA] = 5u; p->v[P_AD] = 70u;
        p->v[P_AS] = 116u; p->v[P_AR] = 38u;
        p->v[P_FTYPE] = F_LP12;
        p->v[P_DRIVE] = 27u;
        p->v[P_MODE] = M_MONO;
        p->v[P_GLIDE] = 41u;
        p->v[P_VEL_FLT] = 82u;
        p->v[P_VINTAGE] = 15u;
        p->v[P_VOLUME] = 76u;
        p->v[OX_RATE] = 66u;
        p->v[OX_TONE] = 12u;  /* keeps the lead focused */
        p->v[OX_WIDTH] = 51u;
        break;
    default: /* Deep, controlled bass: clean sine underneath a buzzy pulse */
        p->v[P_O1_WAVE] = W_SINE;
        p->v[P_O1_LVL] = 120u;
        p->v[P_O2_WAVE] = W_SUPERPULSE;
        p->v[P_O2_PW] = 45u;
        p->v[P_O2_LVL] = 86u;
        p->v[P_O2_SEMI] = 12u;
        p->v[P_CUTOFF] = 76u;
        p->v[P_RESO] = 19u;
        p->v[P_FENV_AMT] = 103u;
        p->v[P_FA] = 4u; p->v[P_FD] = 48u;
        p->v[P_FS] = 58u; p->v[P_FR] = 31u;
        p->v[P_AA] = 3u; p->v[P_AD] = 47u;
        p->v[P_AS] = 101u; p->v[P_AR] = 27u;
        p->v[P_DRIVE] = 23u;
        p->v[P_KEYTRK] = 64u;
        p->v[P_MODE] = M_MONO;
        p->v[P_SPREAD] = 20u;
        p->v[P_VINTAGE] = 11u;
        p->v[P_VOLUME] = 83u;
        p->v[OX_RATE] = 28u;
        p->v[OX_TONE] = 0u; /* fundamental holds steady */
        p->v[OX_WIDTH] = 23u; /* keep deep bass predominantly mono */
        break;
    }
    for (i = 0u; i < PATCH_NAME && names[index][i]; ++i)
        p->v[NPARAM + i] = (uint8_t)names[index][i];
    for (; i < PATCH_NAME; ++i)
        p->v[NPARAM + i] = (uint8_t)' ';
}
#endif /* OBV_X_PATCHES_H */
