/* SPDX-License-Identifier: GPL-3.0-only
 * PAD LAB: verify the new pad bank actually contains distinct synthesis
 * programs, rather than dozens of INIT slots or four cosmetic name changes.
 * These tests are structural; a final judgement of sound needs real FM-1 ears. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../firmware/src/obv_params.h"
#include "../firmware/src/obv_patches.h"

int main(void)
{
    patch_t p[NPAD_LAB];
    uint32_t i, j;
    const uint8_t expected_filters[] = {
        F_LP12, F_LP24, F_BP12, F_LP24, F_LP12,
        F_LP24, F_HP12, F_LP24, F_BP12, F_LP12
    };
    const uint8_t expected_octaves[] = {
        36u, 12u, 36u, 24u, 24u, 24u, 36u, 12u, 24u, 24u
    };
    assert(NPROG > NFACT + NVIBE + NPAD_LAB);
    assert(NPAD_LAB == 10u);
    for (i = 0; i < NPAD_LAB; i++) {
        patch_from_factory(&p[i], &FACT_PAD_LAB[i]);
        assert(p[i].v[P_MODE] == M_POLY); /* no 2+ cards per note */
        assert(p[i].v[P_ARP_MODE] == A_OFF); /* A stays live under B's arp */
        assert(p[i].v[P_O1_WAVE] != W_OFF);
        assert(p[i].v[P_AA] >= 50u); /* all are pads, no accidental clicks */
        assert(p[i].v[P_AR] >= 85u);
        assert(p[i].v[P_VOLUME] >= 98u);
        assert(p[i].v[P_FTYPE] == expected_filters[i]);
        assert(p[i].v[P_O2_SEMI] == expected_octaves[i]);
        assert(strcmp(FACT_PAD_LAB[i].name, "INIT") != 0);
        assert(strlen(FACT_PAD_LAB[i].name) <= PATCH_NAME);
        for (j = 0; j < i; j++) {
            assert(strcmp(FACT_PAD_LAB[i].name, FACT_PAD_LAB[j].name) != 0);
            assert(memcmp(p[i].v, p[j].v, NPARAM) != 0);
        }
    }
    /* First three should have distinctly different filter types and
     * complementary register/oscillator roles, not the old near duplicates. */
    assert(p[0].v[P_CUTOFF] >= 80u);
    assert(p[1].v[P_CUTOFF] <= 45u);
    assert(p[2].v[P_FTYPE] == F_BP12);
    assert(p[0].v[P_O1_WAVE] != p[1].v[P_O1_WAVE]);
    assert(p[2].v[P_O2_WAVE] != p[0].v[P_O2_WAVE]);

    puts("PAD LAB preset bank: ten distinct synth programs PASS");
    return 0;
}
