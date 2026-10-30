/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X v0.12 spectral and formant morphing sounds. Slots 95..98 are the four formerly empty
 * INIT programs. Saved user programs still override these factory previews.
 * All original SIEL 00..94 packed programs, their names and conversions are
 * untouched. The effects are shared mixer returns, not patch-local sends.
 */
#ifndef OBV_X_PATCHES_H
#define OBV_X_PATCHES_H

/* These are explicit per-patch mixer DRIVE sends, not replacement master
 * effects settings. Users may subsequently adjust them in the MIXER.
 * Pads retain a gentle color, while lead and bass have considerably
 * more saturation available from the DRIVE page. */
static inline uint8_t opera_x_factory_drive_send(uint8_t index)
{
    static const uint8_t send[4] = {32u, 38u, 88u, 75u};
    return index < 4u ? send[index] : 0u;
}

static void opera_x_factory_patch(patch_t *p, uint8_t index)
{
    static const char *const names[4] = {
        "X AURORA", "X VOXSCAPE", "X HOTLEAD", "X DEEPBASS"
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
    case 0: /* AURORA: long evolving spectral pad, warm to glassy */
        p->v[P_O1_WAVE] = W_SPECTRAL;
        p->v[P_O1_PW] = 63u;           /* editable MORPH center, LFO scans endpoints */
        p->v[P_O1_LVL] = 115u;
        p->v[P_O2_WAVE] = W_SUPERSAW;  /* supporting stereo detune bloom */
        p->v[P_O2_PW] = 44u;
        p->v[P_O2_LVL] = 54u;
        p->v[P_O2_SEMI] = 24u;
        p->v[P_O2_FINE] = 66u;
        p->v[P_CUTOFF] = 88u;     /* v0.16: enough room for envelope-LFO sweeps */
        p->v[P_RESO] = 17u;
        p->v[P_FENV_AMT] = 83u;
        p->v[P_FTYPE] = F_LP12;        /* keep the upper harmonic morph audible */
        p->v[P_FA] = 80u; p->v[P_FD] = 90u;
        p->v[P_FS] = 103u; p->v[P_FR] = 111u;
        p->v[P_AA] = 86u; p->v[P_AD] = 94u;
        p->v[P_AS] = 119u; p->v[P_AR] = 111u;
        p->v[P_LFO_SHAPE] = L_TRI;
        p->v[P_LFO_RATE] = 48u;
        p->v[P_LFO_PITCH] = 13u;
        p->v[P_LFO_FLT] = 25u;
        p->v[P_LFO_DELAY] = 0u;
        p->v[P_VEL_FLT] = 65u;
        p->v[P_VEL_AMP] = 47u;
        p->v[P_SPREAD] = 80u;
        p->v[P_VOLUME] = 88u;
        p->v[OX_RATE] = 60u;   /* ~4-second filter LFO, ~2-second wavetable motion */
        p->v[OX_TONE] = 118u; /* scan nearly the full warm->glassy spectrum */
        p->v[OX_WIDTH] = 68u; /* centered notes; width within each chord voice */
        break;
    case 1: /* VOXSCAPE: formant-vowel movement instead of analog strings */
        p->v[P_O1_WAVE] = W_VOX;
        p->v[P_O1_PW] = 63u;           /* editable OO->AA formant MORPH */
        p->v[P_O1_LVL] = 127u;
        p->v[P_O2_WAVE] = W_SUPERPULSE;
        p->v[P_O2_PW] = 58u;
        p->v[P_O2_LVL] = 42u;
        p->v[P_O2_SEMI] = 36u;        /* upper-octave breath/harmonic sheen */
        p->v[P_O2_FINE] = 61u;
        p->v[P_NOISE] = 7u;           /* subtle breath */
        p->v[P_CUTOFF] = 93u;     /* v0.16: keep vowel peaks but avoid open-filter ceiling */
        p->v[P_RESO] = 11u;
        p->v[P_FTYPE] = F_LP12;
        p->v[P_FENV_AMT] = 70u;
        p->v[P_FA] = 72u; p->v[P_FD] = 87u;
        p->v[P_FS] = 106u; p->v[P_FR] = 102u;
        p->v[P_AA] = 78u; p->v[P_AD] = 92u;
        p->v[P_AS] = 116u; p->v[P_AR] = 102u;
        p->v[P_LFO_SHAPE] = L_TRI;
        p->v[P_LFO_RATE] = 43u;
        p->v[P_LFO_PITCH] = 11u;
        p->v[P_LFO_PWM] = 43u;
        p->v[P_LFO_PWM_TO] = TO_O2;
        p->v[P_LFO_FLT] = 12u;
        p->v[P_VEL_AMP] = 53u;
        p->v[P_VINTAGE] = 19u;
        p->v[P_SPREAD] = 73u;
        p->v[P_VOLUME] = 90u;
        p->v[OX_RATE] = 67u;  /* faster vocal transitions than AURORA */
        p->v[OX_TONE] = 124u;
        p->v[OX_WIDTH] = 56u;
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
        p->v[OX_RATE] = 69u;
        p->v[OX_TONE] = 19u;  /* keep lead punchy, not seasick */
        p->v[OX_WIDTH] = 37u;
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
        p->v[OX_RATE] = 55u;
        p->v[OX_TONE] = 0u; /* fundamental holds steady */
        p->v[OX_WIDTH] = 18u; /* keep deep bass predominantly mono */
        break;
    }
    /* The new LFO routes are opt-in and stored without moving the
     * 10-character name or any vintage parameter. Bass stays essentially
     * fixed; the evolving pads demonstrate the destinations audibly. */
    {
        static const uint8_t fenv_depth[4] = {63u, 52u, 18u, 0u};
        static const uint8_t morph_depth[4] = {66u, 78u, 0u, 0u};
        opera_x_route_set(p->v, OX_TO_ENV_AMT, fenv_depth[index]);
        opera_x_route_set(p->v, OX_TO_MORPH, morph_depth[index]);
    }
    for (i = 0u; i < PATCH_NAME && names[index][i]; ++i)
        p->v[NPARAM + i] = (uint8_t)names[index][i];
    for (; i < PATCH_NAME; ++i)
        p->v[NPARAM + i] = (uint8_t)' ';
}
#endif /* OBV_X_PATCHES_H */
