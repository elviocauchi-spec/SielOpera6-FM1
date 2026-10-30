/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X synthesis regression: sound waves, detune, factory preview slots.
 * This runs on the host in CI before a flashable firmware is ever published.
 */
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "obv_dsp.h"
#include "obv_params.h"
#include "obv_x_waves.h"
#include "obv_x_motion.h"
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
        "X CLOUDPAD", "X SILKSTR", "X HOTLEAD", "X DEEPBASS"
    };

    /* Original Opera 6 ROM wave codes remain bit-for-bit unchanged. */
    assert(W_OFF == 0 && W_SAW == 1 && W_PLS == 2 && W_BOTH == 3);
    assert(W_SINE == 4 && W_TRI == 5 && W_SUPERSAW == 6 && W_SUPERPULSE == 7);
    assert(PDEF[P_O1_WAVE].max == 7 && PDEF[P_O2_WAVE].max == 7);

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

    /* Each free slot has a meaningful and playable musical sound. */
    for (i = 0; i < 4u; ++i) {
        opera_x_factory_patch(&patch, (uint8_t)i);
        for (j = 0; j < NPARAM; ++j)
            assert(patch.v[j] <= PDEF[j].max);
        for (j = 0; j < PATCH_NAME; ++j) {
            uint8_t want = expected[i][j] ? (uint8_t)expected[i][j] : (uint8_t)' ';
            assert(patch.v[NPARAM + j] == want);
        }
        assert(patch.v[P_O1_LVL] != 0u || patch.v[P_O2_LVL] != 0u);
        assert(patch.v[PATCH_BYTES - 2u] != 0xA6u); /* no SIEL footage marker */
        assert(opera_x_expressive(patch.v));
        assert(patch.v[OX_WIDTH] <= 127u && patch.v[OX_TONE] <= 127u);
        assert(patch.v[OX_RATE] <= 127u);
        assert(patch.v[OX_MAGIC] == OX_TAG);
    }
    opera_x_factory_patch(&patch, 0u);
    assert(patch.v[P_O1_WAVE] == W_SUPERSAW);
    opera_x_factory_patch(&patch, 1u);
    assert(patch.v[P_O1_WAVE] == W_SUPERPULSE);
    opera_x_factory_patch(&patch, 2u);
    assert(patch.v[P_MODE] == M_MONO);
    opera_x_factory_patch(&patch, 3u);
    assert(patch.v[P_O1_WAVE] == W_SINE);
    puts("OPERA X: extended waves and four playable factory sounds pass");
    return 0;
}
