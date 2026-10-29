/* SPDX-License-Identifier: GPL-3.0-only */
/* SIEL Opera 6/DK600 factory-bank -> FM1 PLAYABLE PREVIEW (v0.8).
 *
 * Proven: 95 original packed programs, RAM order from v10 TMS7000 ROM.
 * Mapped: analog C1..C16 names from October 1983 SIEL service schematic,
 *         with RAM slot order corroborated by service programs 91/92.
 * Inferred: some digital fields by ROM's front-panel switch state machines
 *           E3CB..E50E and program statistics.
 * Approximate: voltage curves, oscillator A octave (FM1 lacks O1 transpose),
 *              dual independent LFO rates, LFO III mixed waveform,
 *              dynamic attack-time modulation, analog filter/VCA character.
 * NOT a bit-exact emulation or a promise of original factory timbre.
 */
#ifndef SIEL_FACTORY_CONVERT_H
#define SIEL_FACTORY_CONVERT_H
#include "siel_factory_layout.h"

/* FM1 parameter definitions (obv_params.h) must be included before this file. */
static uint8_t siel_clamp127(int32_t x) {
    return (uint8_t)(x < 0 ? 0 : x > 127 ? 127 : x);
}
static uint8_t siel_midi8(uint8_t x) { return (uint8_t)(x >> 1); }
static uint8_t siel_route(uint8_t bits) {
    return bits == 1u ? TO_O1 : bits == 2u ? TO_O2 : TO_BOTH;
}
static void siel_factory_to_patch(patch_t *p, uint8_t program) {
    uint8_t r44, r45, r46, r47, afoot, bfoot, pitch_route, pwm_route, adsr_dest, dyn_dest;
    int32_t coarse, semis, fine;
    if (program >= SIEL_FACTORY_PROGRAMS) { patch_defaults(p); return; }
    patch_defaults(p);
    r47 = siel_factory_digital(program, 0u);
    r46 = siel_factory_digital(program, 1u);
    r45 = siel_factory_digital(program, 2u);
    r44 = siel_factory_digital(program, 3u);
    (void)r47; /* pitch/mod wheel destination cannot be independently represented in FM1 */

    /* Analog scan C1..C16, original 8-bit values (low/hi SysEx nibbles). */
    p->v[P_O1_PW] = siel_midi8(siel_factory_analog(program, 4u)); /* C5: PWM A */
    p->v[P_NOISE] = siel_midi8(siel_factory_analog(program, 5u)); /* C6: noise */
    p->v[P_O2_PW] = siel_midi8(siel_factory_analog(program, 6u)); /* C7: PWM B */
    p->v[P_CUTOFF] = siel_midi8(siel_factory_analog(program, 14u)); /* C15 */
    /* Preview-only correction: FM1's digital LP24 response is not the SIEL
     * analog SSM filter response. Very low converted cutoffs muted numerous
     * musical patches, so use a conservative audible minimum. Never alter
     * the original SIEL data or hardware calibration programs 90..94. */
    if (program < 90u && p->v[P_CUTOFF] < 28u)
        p->v[P_CUTOFF] = 28u;
    p->v[P_RESO] = siel_midi8(siel_factory_analog(program, 15u));   /* C16 */
    p->v[P_FA] = p->v[P_AA] = siel_midi8(siel_factory_analog(program, 9u));  /* C10 */
    p->v[P_FD] = p->v[P_AD] = siel_midi8(siel_factory_analog(program, 10u)); /* C11 */
    p->v[P_FS] = p->v[P_AS] = siel_midi8(siel_factory_analog(program, 11u)); /* C12 */
    p->v[P_FR] = p->v[P_AR] = siel_midi8(siel_factory_analog(program, 12u)); /* C13 */

    /* ROM switch-state groups. R44[7:6] is 3-way A wave; R45[1:0] B wave. */
    p->v[P_O1_WAVE] = (uint8_t)((r44 >> 6) & 3u);
    p->v[P_O2_WAVE] = (uint8_t)(r45 & 3u);
    p->v[P_O1_LVL] = p->v[P_O1_WAVE] ? 127u : 0u;
    p->v[P_O2_LVL] = p->v[P_O2_WAVE] ? ((r45 & 4u) ? 64u : 127u) : 0u;
    /* Octave A is the FM1 reference; preserve relative B/A octave spacing.
     * Original coarse detune goes downward by ~a fifth, not +/- 24 semitones.
     * Scaling is an engineering approximation, not ROM-proven. */
    afoot = (uint8_t)(r44 & 3u);
    bfoot = (uint8_t)((r44 >> 2) & 3u);
    if (!afoot) afoot = 2u;
    if (!bfoot) bfoot = 2u;
    coarse = ((int32_t)255 - siel_factory_analog(program, 8u) + 18) / 36;
    semis = 24 + 12 * ((int32_t)bfoot - afoot) - coarse;
    /* Preview-only guard against extreme octave intervals. A zero or
     * four-octave separation made some programs seem sub-bass or unrelated
     * to their source. Final tuning requires an accurate Opera 6 mapping. */
    if (program < 90u && p->v[P_O2_LVL]) {
        if (semis < 12) semis = 12;
        if (semis > 36) semis = 36;
    }
    p->v[P_O2_SEMI] = (uint8_t)(semis < 0 ? 0 : semis > 48 ? 48 : semis);
    fine = 64 + ((int32_t)siel_factory_analog(program, 7u) - 121) / 2;
    p->v[P_O2_FINE] = siel_clamp127(fine);

    adsr_dest = (uint8_t)((r46 >> 2) & 3u); /* neither/VCF/VCA/both */
    dyn_dest = (uint8_t)(r46 & 3u);         /* neither/level/attack/both */
    p->v[P_FENV_AMT] = (adsr_dest & 1u)
        ? (uint8_t)(64u + (siel_factory_analog(program, 13u) >> 2)) : 64u;
    if (!(adsr_dest & 2u)) { /* No original VCA envelope: simple note gate. */
        p->v[P_AA] = 0u; p->v[P_AD] = 0u; p->v[P_AS] = 127u; p->v[P_AR] = 0u;
    }
    p->v[P_KEYTRK] = (r46 & 0x10u) ? 127u : 0u;
    p->v[P_VEL_AMP] = ((dyn_dest & 1u) && (adsr_dest & 2u)) ? 100u : 0u;
    p->v[P_VEL_FLT] = ((dyn_dest & 1u) && (adsr_dest & 1u)) ? 100u : 0u;

    pitch_route = (uint8_t)((r45 >> 3) & 3u); /* LFO I/II -> DCO A/B */
    pwm_route = (uint8_t)((r45 >> 5) & 3u);   /* LFO III -> PWM A/B */
    p->v[P_LFO_PITCH] = pitch_route ? siel_midi8(siel_factory_analog(program, 0u)) : 0u;
    p->v[P_LFO_PWM] = pwm_route ? siel_midi8(siel_factory_analog(program, 2u)) : 0u;
    p->v[P_LFO_FLT] = (r45 & 0x80u) ? siel_midi8(siel_factory_analog(program, 2u)) : 0u;
    p->v[P_LFO_PITCH_TO] = pitch_route ? siel_route(pitch_route) : TO_BOTH;
    p->v[P_LFO_PWM_TO] = pwm_route ? siel_route(pwm_route) : TO_BOTH;
    /* FM1 has one LFO; select the relevant original rate, preferring pitch. */
    p->v[P_LFO_RATE] = siel_midi8(siel_factory_analog(program,
        pitch_route ? 1u : ((pwm_route || (r45 & 0x80u)) ? 3u : 1u)));
    p->v[P_LFO_SHAPE] = (((r44 >> 4) & 3u) == 2u) ? L_SQR : L_TRI;

    p->v[P_MODE] = M_POLY;
    p->v[P_FTYPE] = F_LP24;
    p->v[P_VINTAGE] = 0u;
    p->v[P_SPREAD] = 0u;
    p->v[P_DRIVE] = 0u;
    /* Diagnostic rescue for the THIRD displayed factory slot, SIEL 02.
     * Reported inaudible in the emulator. The approximate conversion puts
     * a resonant 24dB low-pass on a quiet setting (cutoff 32, resonance 106).
     * Hold the same source waveforms and envelopes, but open the filter
     * and reduce resonance enough to test whether the filter causes silence.
     * NOT an authentic SIEL calibration; original 20 bytes are unchanged. */
    if (program == 2u) {
        p->v[P_CUTOFF] = 76u;
        p->v[P_RESO] = 35u;
        p->v[P_FENV_AMT] = 64u;
    }
    p->v[P_CHO_MIX] = 0u;
    p->v[P_DLY_MIX] = 0u;
    p->v[P_REV_MIX] = 0u;
    p->v[NPARAM + 0u] = 'S';
    p->v[NPARAM + 1u] = program >= 90u ? 'V' : 'I';
    p->v[NPARAM + 2u] = program >= 90u ? 'C' : 'E';
    p->v[NPARAM + 3u] = program >= 90u ? ' ' : 'L';
    p->v[NPARAM + 4u] = ' ';
    p->v[NPARAM + 5u] = (uint8_t)('0' + program / 10u);
    p->v[NPARAM + 6u] = (uint8_t)('0' + program % 10u);
}
#endif /* SIEL_FACTORY_CONVERT_H */
