/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include "obv_params.h"
#include "siel_factory_convert.h"

int main(void) {
    patch_t p;
    unsigned i, j;
    assert(sizeof(siel_factory_packed) == 95u * 20u);
    for (i = 0; i < SIEL_FACTORY_PROGRAMS; ++i) {
        siel_factory_to_patch(&p, (uint8_t)i);
        for (j = 0; j < NPARAM; ++j)
            assert(p.v[j] <= PDEF[j].max);
        assert(p.v[NPARAM] == 'S');
        assert(p.v[NPARAM + 5u] == (uint8_t)('0' + i / 10u));
        assert(p.v[NPARAM + 6u] == (uint8_t)('0' + i % 10u));
        assert(p.v[P_FTYPE] == F_LP24);
        assert(p.v[P_CHO_MIX] == 0u && p.v[P_DLY_MIX] == 0u && p.v[P_REV_MIX] == 0u);
        /* v0.8.1 preview-only musical-patch safeguards.
         * Calibration programs 90..94 remain unchanged. */
        if (i < 90u) {
            assert(p.v[P_CUTOFF] >= 28u);
            if (p.v[P_O2_LVL])
                assert(p.v[P_O2_SEMI] >= 12u && p.v[P_O2_SEMI] <= 36u);
        }
        /* Original factory programs 00..89 must have an audible source. */
        if (i < 90u)
            assert(p.v[P_O1_LVL] || p.v[P_O2_LVL] || p.v[P_NOISE]);
    }
    siel_factory_to_patch(&p, 2u);
    assert(p.v[P_CUTOFF] == 76u);
    assert(p.v[P_RESO] == 35u);
    assert(p.v[P_FENV_AMT] == 64u);
    assert(p.v[P_O1_LVL] && p.v[P_O2_LVL]);
    siel_factory_to_patch(&p, 0u);
    assert(p.v[P_O1_WAVE] == W_SAW);
    assert(p.v[P_O2_WAVE] == W_OFF);
    assert(p.v[P_O1_LVL] == 127u && p.v[P_O2_LVL] == 0u);
    siel_factory_to_patch(&p, 91u); /* service: resonant VCF calibration */
    assert(p.v[P_CUTOFF] == 72u);
    assert(p.v[P_RESO] == 126u);
    siel_factory_to_patch(&p, 92u); /* service: 5.8s ADSR attack */
    assert(p.v[P_FA] == 126u && p.v[P_AA] == 126u);
    siel_factory_to_patch(&p, 93u); /* service: PWM calibration */
    assert(p.v[P_O1_PW] == 61u);
    siel_factory_to_patch(&p, 95u);
    assert(p.v[P_CUTOFF] == PDEF[P_CUTOFF].def);
    return 0;
}
