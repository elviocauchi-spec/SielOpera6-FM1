/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X synthesis regression: sound waves, detune, factory preview slots.
 * This runs on the host in CI before a flashable firmware is ever published.
 */
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "obv_dsp.h"
#include "obv_params.h"
#include "obv_x_waves.h"
#include "obv_x_tables.h"
#include "obv_x_motion.h"
#include "obv_x_formants.h"
#include "obv_x_routes.h"
#include "obv_x_drive.h"
#include "obv_x_patches.h"

static int64_t test_osc(uint32_t wave)
{
    osc_t main = {0}, left = {0}, right = {0};
    uint32_t lo, hi, i, inc = 0x01777777u;
    int64_t energy = 0;
    opera_x_init_aux(&left, &right, main.ph);
    opera_x_detuned_incs(inc, 60u, &lo, &hi);
    for (i = 0; i < 2048u; ++i) {
        int32_t s = opera_x_osc_sample(&main, &left, &right, inc,
                                      lo, hi, 0x80000000u, wave, -1);
        assert(s >= -65536 && s <= 65536);
        energy += s < 0 ? -(int64_t)s : (int64_t)s;
    }
    return energy;
}

int main(void)
{
    patch_t patch;
    unsigned i, j;
    static const char *const expected[4] = {
        "X AURORA", "X VOXSCAPE", "X HOTLEAD", "X DEEPBASS"
    };

    /* Original Opera 6 ROM wave codes remain bit-for-bit unchanged. */
    assert(W_OFF == 0 && W_SAW == 1 && W_PLS == 2 && W_BOTH == 3);
    assert(W_SINE == 4 && W_TRI == 5 && W_SUPERSAW == 6 && W_SUPERPULSE == 7);
    assert(W_SPECTRAL == 8 && W_VOX == 9);
    assert(PDEF[P_O1_WAVE].max == 9 && PDEF[P_O2_WAVE].max == 9);

    /* Signed, linearly interpolated basic waves at exact quarter phases. */
    assert(opera_x_sine(0u) == 0);
    assert(opera_x_sine(0x40000000u) == 32767);
    assert(opera_x_sine(0x80000000u) == 0);
    assert(opera_x_sine(0xc0000000u) == -32767);
    assert(opera_x_triangle(0u) == -32768);
    assert(opera_x_triangle(0x40000000u) == 0);
    assert(opera_x_triangle(0x80000000u) == 32767);

    /* Width=0 means zero pitch detune; width 127 creates a symmetric spread. */
    {
        uint32_t lo, hi;
        opera_x_detuned_incs(0x02000000u, 0u, &lo, &hi);
        assert(lo == 0x02000000u && hi == 0x02000000u);
        opera_x_detuned_incs(0x02000000u, 127u, &lo, &hi);
        assert(lo < 0x02000000u && hi > 0x02000000u);
    }

    /* All three new timbres plus basic waves emit bounded audio. */
    assert(test_osc(W_SINE) > 1000000);
    assert(test_osc(W_TRI) > 1000000);
    assert(test_osc(W_SUPERSAW) > 1000000);
    assert(test_osc(W_SUPERPULSE) > 1000000);

    /* v0.10: independent slow filter motion is bounded, bipolar and not
     * confused with the legacy patch LFO controls. */
    assert(opera_x_motion(0u) == -32768);
    assert(opera_x_motion(0x40000000u) == 0);
    assert(opera_x_motion(0x80000000u) == 32767);
    assert(opera_x_filter_delta(0, 127u) == 0);
    assert(opera_x_filter_delta(32767, 0u) == 0);
    assert(opera_x_filter_delta(32767, 127u) > 2600);
    assert(opera_x_filter_delta(-32768, 127u) < -2600);
    assert(opera_x_motion_inc(30u) > 0u);
    assert(opera_x_width_gain(0u) == 0);
    assert(opera_x_width_gain(127u) == 1150);
    assert(opera_x_side_pair(12000, -9000, 127) > 10000);
    assert(opera_x_side_pair(12000, -9000, 0) == 0);
    {
        int32_t lpf = 0;
        unsigned k;
        for (k = 0; k < 100u; ++k)
            lpf = opera_x_side_smooth(lpf, 16000);
        assert(lpf > 15000 && lpf <= 16000);
        /* A marked patch produces different left/right output with nonzero
         * side content, while width 0 collapses precisely to mono. */
        assert((10000 + (lpf * opera_x_width_gain(90u) >> 12)) !=
               (10000 - (lpf * opera_x_width_gain(90u) >> 12)));
        assert((10000 + (lpf * opera_x_width_gain(0u) >> 12)) ==
               (10000 - (lpf * opera_x_width_gain(0u) >> 12)));
    }

    /* v0.11: FX DRIVE must have substantial, monotonic wet gain;
     * before this patch the full return was only 4000/32768. */
    assert(opera_x_drive_return_gain(0u) == 0);
    assert(opera_x_drive_return_gain(127u) == 25500);
    assert(opera_x_drive_return_gain(64u) > 12000);
    assert(opera_x_drive_pre_gain(127u) > opera_x_drive_pre_gain(10u));
    assert(opera_x_drive_shape(1000, 0u) > 700 && opera_x_drive_shape(1000, 0u) < 1200);
    assert(opera_x_drive_shape(1000, 2u) > opera_x_drive_shape(1000, 0u));
    assert(opera_x_drive_shape(-1000, 2u) < 0);

    /* v0.11: modulation must be clearly different within a few seconds
     * rather than taking 20 seconds for its first barely audible arc. */
    {
        uint32_t ph = 0u, n;
        int32_t start = opera_x_filter_delta(opera_x_motion(ph), 82u);
        int32_t timbre_before = opera_x_timbre_motion(ph);
        for (n = 0u; n < 5500u; ++n)
            ph += opera_x_motion_inc(59u); /* about two seconds of audio */
        assert(opera_x_filter_delta(opera_x_motion(ph), 82u) - start > 2000);
        /* Second timbre wave is not the same function as first. */
        assert(opera_x_timbre_motion(ph) != opera_x_motion(ph));
        assert(opera_x_timbre_motion(ph) != timbre_before);
        assert(opera_x_animated_detune(70u, 32767, 80u) > 70u);
        assert(opera_x_animated_detune(70u, -32768, 80u) < 70u);
        assert(opera_x_animated_level(96, 32767, 80u) > 96);
        assert(opera_x_animated_level(96, -32768, 80u) < 96);
    }
    patch_defaults(&patch);
    assert(opera_x_note_pan(patch.v, -45) == -45); /* legacy stereo unchanged */

    /* Each spectral family must change harmonic content as MORPH moves;
     * the two families must not be near-identical analog waveforms. */
    {
        uint32_t j;
        uint64_t distance0 = 0, distance1 = 0, between = 0;
        int64_t sum0 = 0, sum1 = 0;
        osc_t osc = {0};
        uint32_t inc = 0x01800000u; /* low-mid register, full 32 harmonics */
        for (j = 0; j < 256u; ++j) {
            uint32_t ph = j << 24;
            int32_t a0 = opera_x_wt_lookup(ph, inc, 0u, 0u);
            int32_t a1 = opera_x_wt_lookup(ph, inc, 127u, 0u);
            int32_t b0 = opera_x_wt_lookup(ph, inc, 0u, 1u);
            int32_t b1 = opera_x_wt_lookup(ph, inc, 127u, 1u);
            int32_t d0 = a0 - a1, d1 = b0 - b1, db = a0 - b0;
            assert(a0 >= -30000 && a0 <= 30000);
            assert(a1 >= -30000 && a1 <= 30000);
            assert(b0 >= -30000 && b0 <= 30000);
            assert(b1 >= -30000 && b1 <= 30000);
            distance0 += d0 < 0 ? -(int64_t)d0 : d0;
            distance1 += d1 < 0 ? -(int64_t)d1 : d1;
            between += db < 0 ? -(int64_t)db : db;
            sum0 += a0 < 0 ? -(int64_t)a0 : a0;
            sum1 += b0 < 0 ? -(int64_t)b0 : b0;
        }
        assert(distance0 / 256u > 4000u); /* warm -> glass is a real change */
        assert(distance1 / 256u > 3000u); /* rounded -> vowel is a real change */
        assert(between / 256u > 3000u);   /* spectral != vocal family */
        assert(sum0 > 1000000 && sum1 > 1000000);
        assert(opera_x_wt_band(0u) == 0u);
        assert(opera_x_wt_band(0xffffffffu) == 4u);
        /* v0.13: independent modulation must not pin MORPH to its edge.
         * Manual center 64 has controlled +/-43 travel; both endpoints
         * remain available by turning the knob to 0 or 127. */
        assert(opera_x_wt_morph(64u, 32767, 32767, 118u) > 100u);
        assert(opera_x_wt_morph(64u, 32767, 32767, 118u) < 114u);
        assert(opera_x_wt_morph(64u, -32768, -32768, 118u) > 15u);
        assert(opera_x_wt_morph(64u, -32768, -32768, 118u) < 28u);
        assert(opera_x_timbre_inc(60u) > opera_x_motion_inc(60u));
        for (j = 0; j < 512u; ++j) {
            int32_t y = opera_x_wt_step(&osc, inc, 80u, 1u, -1);
            assert(y >= -30000 && y <= 30000);
        }
    }

    /* v0.13 range regression: NO DEAD ZONES. Each quarter of the physical
     * MORPH knob contains its own distinct timbral transition. The previous
     * test checked endpoints only, missing the listener's 15%-only problem.
     * Test low/mid-band notes in both oscillator families. */
    {
        static const uint32_t pitch_inc[3] = {
            0x01800000u, 0x04000000u, 0x0a000000u
        };
        unsigned family, k, start, ph_index;
        assert(sizeof(opera_x_spectra) == 25700u);
        for (family = 0u; family < 2u; ++family) {
            for (k = 0u; k < 3u; ++k) {
                for (start = 0u; start < 127u; start += 16u) {
                    unsigned next = start + 16u <= 127u ? start + 16u : 127u;
                    uint64_t diff = 0;
                    for (ph_index = 0u; ph_index < 256u; ++ph_index) {
                        uint32_t phase = ph_index << 24;
                        int32_t a = opera_x_wt_lookup(phase, pitch_inc[k], start, family);
                        int32_t b = opera_x_wt_lookup(phase, pitch_inc[k], next, family);
                        int32_t d = a - b;
                        diff += d < 0 ? -(int64_t)d : d;
                    }
                    assert(diff / 256u > 900u);
                }
            }
        }
        assert(opera_x_wt_stage(0u) == 0u);
        assert(opera_x_wt_stage(32u) == 1u);
        assert(opera_x_wt_stage(64u) == 2u);
        assert(opera_x_wt_stage(96u) == 3u);
        assert(opera_x_wt_stage(127u) == 3u);
        assert(opera_x_wt_blend(0u) == 0u);
        assert(opera_x_wt_blend(127u) == 128u);
        /* Test that a FULL MORPH sweep stays monotonic in effective position
         * during either extreme modulation direction, without clipping
         * except at the deliberately selected physical endpoints. */
        for (k = 0u; k < 3u; ++k) {
            int32_t motion = k == 0u ? -32768 : k == 1u ? 0 : 32767;
            unsigned previous = 0;
            for (start = 0u; start <= 127u; ++start) {
                unsigned p = opera_x_wt_morph((uint8_t)start, motion, motion, 127u);
                if (start == 0u) assert(p == 0u);
                else assert(p >= previous);
                if (start == 127u) assert(p == 127u);
                if (start > 0u && start < 127u)
                    assert(p > 0u && p < 127u);
                if (motion == 0) assert(p == start);
                previous = p;
            }
        }
    }

    /* Also exercise the ACTUAL low-pass DSP: a waveform-distance test
     * alone can falsely pass when a synth filter masks all timbre change.
     * Use the 12 dB SVF at a typical bright-pad cutoff and compare every
     * 16-position MORPH interval after the initial filter settling time. */
    {
        unsigned family, start;
        svf_coef_t c;
        svf_coef(&c, 109 << 8, 6500);
        for (family = 0u; family < 2u; ++family) {
            for (start = 0u; start < 127u; start += 16u) {
                uint32_t ph = 0u, inc = 0x01800000u, n;
                unsigned next = start + 16u <= 127u ? start + 16u : 127u;
                svf_state_t a = {0}, b = {0};
                int64_t diff = 0;
                for (n = 0u; n < 1536u; ++n, ph += inc) {
                    int32_t al, ab, ah, bl, bb, bh;
                    int32_t v1 = opera_x_wt_lookup(ph, inc, start, family);
                    int32_t v2 = opera_x_wt_lookup(ph, inc, next, family);
                    svf_step(&c, &a, v1, &al, &ab, &ah);
                    svf_step(&c, &b, v2, &bl, &bb, &bh);
                    if (n >= 512u) {
                        int32_t d = al - bl;
                        diff += d < 0 ? -(int64_t)d : (int64_t)d;
                    }
                }
                assert(diff / 1024u > 850u);
            }
        }
    }

    /* v0.15: the two new editable routes must never collide with the
     * original name, factory ROM, or oscillator footage trailer. */
    {
        patch_t original, x;
        uint32_t j;
        patch_defaults(&original);
        original.v[PATCH_BYTES - 2u] = 0xA6u;
        original.v[PATCH_BYTES - 1u] = 3u;
        for (j = 0u; j < PATCH_NAME; ++j)
            original.v[NPARAM + j] = (uint8_t)("SIEL PATCH"[j]);
        assert(opera_x_route_get(original.v, OX_TO_ENV_AMT) == 0u);
        assert(opera_x_route_get(original.v, OX_TO_MORPH) == 0u);
        opera_x_route_set(original.v, OX_TO_ENV_AMT, 100u);
        opera_x_route_set(original.v, OX_TO_MORPH, 72u);
        assert(original.v[PATCH_BYTES - 2u] == 0xA6u);
        assert(original.v[PATCH_BYTES - 1u] == 3u);
        assert(opera_x_route_get(original.v, OX_TO_ENV_AMT) == 100u);
        assert(opera_x_route_get(original.v, OX_TO_MORPH) == 72u);
        for (j = 0u; j < PATCH_NAME; ++j)
            assert(original.v[NPARAM + j] == (uint8_t)("SIEL PATCH"[j]));

        patch_defaults(&x);
        x.v[OX_MAGIC] = OX_TAG;
        x.v[OX_RATE] = 55u; x.v[OX_TONE] = 85u; x.v[OX_WIDTH] = 62u;
        assert(opera_x_expressive(x.v));
        assert(opera_x_route_get(x.v, OX_TO_MORPH) == 0u);
        opera_x_route_set(x.v, OX_TO_ENV_AMT, 64u);
        opera_x_route_set(x.v, OX_TO_MORPH, 127u);
        assert(opera_x_route_get(x.v, OX_TO_ENV_AMT) == 64u);
        assert(opera_x_route_get(x.v, OX_TO_MORPH) == 127u);
        assert(opera_x_expressive(x.v)); /* v0.10 stereo/motion stays intact */
        assert(x.v[OX_RATE] == 55u && x.v[OX_TONE] == 85u && x.v[OX_WIDTH] == 62u);
        assert(x.v[PATCH_BYTES - 2u] == 64u && x.v[PATCH_BYTES - 1u] == 127u);
        /* v0.16 response is intentionally asymmetric: 0 = untouched,
         * positive phase = modest +12, negative phase = strong -56.
         * Old v0.15's +/-24 often ran into the high-cutoff ceiling. */
        assert(opera_x_lfo_env_amount(30, 0, 127u) == 30);
        assert(opera_x_lfo_env_amount(30, 32767, 0u) == 30);
        assert(opera_x_lfo_env_amount(30, -32768, 0u) == 30);
        assert(opera_x_lfo_env_amount(30, 32767, 127u) == 42);
        assert(opera_x_lfo_env_amount(30, -32768, 127u) == -26);
        assert(opera_x_lfo_env_amount(30, 32767, 64u) == 36);
        assert(opera_x_lfo_env_amount(30, -32768, 64u) == 2);
        assert(opera_x_lfo_env_amount(63, 32767, 127u) == 63);
        assert(opera_x_lfo_env_amount(-64, -32768, 127u) == -64);
        assert(opera_x_env_cutoff_q8(0, 30, -32768, 127u) == 0);
        assert(opera_x_env_cutoff_q8(32767, 30, 0, 127u) ==
               opera_x_env_cutoff_q8(32767, 30, -32768, 0u));
        assert(opera_x_lfo_morph(64u, 0, 127u) == 64u);
        assert(opera_x_lfo_morph(64u, 32767, 0u) == 64u);
        assert(opera_x_lfo_morph(64u, 32767, 127u) > 110u);
        assert(opera_x_lfo_morph(64u, -32768, 127u) < 15u);
        assert(opera_x_lfo_morph(0u, 32767, 127u) == 0u);
        assert(opera_x_lfo_morph(127u, -32768, 127u) == 127u);
        for (j = 0; j < 127u; ++j) {
            int32_t pos = opera_x_lfo_morph((uint8_t)j, 32767, 95u);
            assert(pos >= (int32_t)j && pos <= 127);
        }
    }

    /* v0.16: don't merely check control numbers: check audible cutoff
     * headroom and the actual low-pass output with a sustained pad signal.
     * With the v0.15 preset cutoff, the hi-LFO point was effectively
     * wide open and most of the ENV amount control did nothing. */
    {
        unsigned patch_num;
        for (patch_num = 0u; patch_num < 2u; ++patch_num) {
            patch_t pad;
            svf_coef_t low_coef, high_coef;
            svf_state_t low = {0}, high = {0};
            uint32_t ph = 0u, inc = 0x01800000u, sample;
            int32_t base_amt, env, center, cut_low, cut_high, diff;
            int64_t audible_difference = 0;
            opera_x_factory_patch(&pad, (uint8_t)patch_num);
            base_amt = (int32_t)pad.v[P_FENV_AMT] - 64;
            env = (int32_t)pad.v[P_FS] * 32767 / 127;
            center = ((int32_t)pad.v[P_CUTOFF] + 8) << 8;
            cut_low = center + opera_x_env_cutoff_q8(
                env, base_amt, -32768,
                opera_x_route_get(pad.v, OX_TO_ENV_AMT));
            cut_high = center + opera_x_env_cutoff_q8(
                env, base_amt, 32767,
                opera_x_route_get(pad.v, OX_TO_ENV_AMT));
            diff = cut_high - cut_low;
            assert(diff > 5000);       /* >19.5 semitones across LFO cycle */
            assert(cut_high < (136 << 8)); /* never cap at maximum filter Hz */
            assert(cut_low > (45 << 8));  /* don't disappear into sub-bass */
            svf_coef(&low_coef, cut_low, 7200);
            svf_coef(&high_coef, cut_high, 7200);
            for (sample = 0u; sample < 2048u; ++sample, ph += inc) {
                int32_t input = opera_x_wt_lookup(
                    ph, inc, 63u, patch_num == 1u ? 1u : 0u) >> 1;
                int32_t lp_a,bp_a,hp_a,lp_b,bp_b,hp_b;
                svf_step(&low_coef, &low, input, &lp_a, &bp_a, &hp_a);
                svf_step(&high_coef, &high, input, &lp_b, &bp_b, &hp_b);
                if (sample >= 512u) {
                    int32_t d = lp_a - lp_b;
                    audible_difference += d < 0 ? -(int64_t)d : d;
                }
            }
            assert(audible_difference / 1536u > 400u);
        }
    }

    /* v0.14: vowel resonances are specified at absolute Hz, not indexed
     * by MIDI note/octave; verify distinct and ordered interpolated peaks. */
    {
        unsigned j;
        svf_coef_t c1, c2;
        svf_state_t s1 = {0}, s2 = {0};
        int64_t energy = 0;
        const uint8_t centers[5] = {0u, 32u, 64u, 96u, 127u};
        const uint16_t *f1 = opera_x_vox_f1_q8, *f2 = opera_x_vox_f2_q8;
        for (j = 0u; j < 5u; ++j) {
            uint16_t a, b;
            opera_x_formant_targets(centers[j], &a, &b);
            assert(a == f1[j] && b == f2[j]);
            assert(a > 13000u && a < b && b < 30000u);
            opera_x_formant_coeffs(centers[j], &c1, &c2);
            assert(c1.a1 > 0 && c2.a1 > 0);
        }
        opera_x_formant_coeffs(64u, &c1, &c2);
        for (j = 0u; j < 2048u; ++j) {
            uint32_t ph = j * 0x02000000u;
            int32_t wave = opera_x_wt_lookup(ph, 0x02000000u, 64u, 1u);
            int32_t y = opera_x_formant_render(wave, &c1, &c2, &s1, &s2);
            assert(y >= -65000 && y <= 65000);
            energy += y < 0 ? -(int64_t)y : y;
        }
        assert(energy > 100000u);
    }

    /* Each free slot has a meaningful and playable musical sound. */
    for (i = 0; i < 4u; ++i) {
        opera_x_factory_patch(&patch, (uint8_t)i);
        for (j = 0; j < NPARAM; ++j)
            assert(patch.v[j] <= PDEF[j].max);
        for (j = 0; j < PATCH_NAME; ++j) {
            uint8_t want = j < strlen(expected[i]) ? (uint8_t)expected[i][j] : (uint8_t)' ';
            assert(patch.v[NPARAM + j] == want);
        }
        assert(patch.v[P_O1_LVL] != 0u || patch.v[P_O2_LVL] != 0u);
        assert(patch.v[PATCH_BYTES - 2u] != 0xA6u); /* no SIEL footage marker */
        assert(opera_x_expressive(patch.v));
        assert(patch.v[OX_WIDTH] <= 127u && patch.v[OX_TONE] <= 127u);
        assert(patch.v[OX_RATE] <= 127u);
        assert(patch.v[OX_MAGIC] == OX_TAG);
        assert(opera_x_factory_drive_send((uint8_t)i) > 0u);
        assert(patch.v[OX_ROUTE_TAG_AT] == OX_ROUTE_TAG);
        assert(patch.v[OX_MAGIC] == OX_TAG); /* no metadata alias */
        if (i < 2u) {
            assert(opera_x_route_get(patch.v, OX_TO_ENV_AMT) >= 25u);
            assert(opera_x_route_get(patch.v, OX_TO_MORPH) >= 60u);
        } else {
            assert(opera_x_route_get(patch.v, OX_TO_MORPH) == 0u);
        }
        if (i < 2u) {
            assert(opera_x_note_pan(patch.v, -60) == 0);
            assert(opera_x_note_pan(patch.v, 40) == 0);
            assert(patch.v[OX_RATE] >= 50u);
            assert(patch.v[OX_TONE] >= 110u);
        } else {
            assert(opera_x_note_pan(patch.v, 40) == 40); /* mono stays mono */
        }
    }
    opera_x_factory_patch(&patch, 0u);
    assert(patch.v[P_O1_WAVE] == W_SPECTRAL);
    assert(patch.v[P_O2_WAVE] == W_SUPERSAW);
    assert(patch.v[OX_TONE] >= 110u); /* near full-spectrum timbral movement */
    assert(patch.v[P_O1_PW] > 0u && patch.v[P_O1_PW] < 127u); /* editable MORPH */
    opera_x_factory_patch(&patch, 1u);
    assert(patch.v[P_O1_WAVE] == W_VOX);
    assert(patch.v[P_O2_WAVE] == W_SUPERPULSE);
    assert(patch.v[OX_TONE] >= 110u);
    opera_x_factory_patch(&patch, 2u);
    assert(patch.v[P_MODE] == M_MONO);
    opera_x_factory_patch(&patch, 3u);
    assert(patch.v[P_O1_WAVE] == W_SINE);
    puts("OPERA X: audible ENV AMT, dual LFO routes, formants, preserved footage and DRIVE pass");
    return 0;
}
