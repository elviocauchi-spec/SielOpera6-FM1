/* SPDX-License-Identifier: GPL-3.0-only */
/* FM1 FX ROUTING integration: exercise the real delay and Dattorro tank.
 * The original serial modes must NOT change the legacy parallel sound,
 * must preserve DRIVE type in the stored settings byte, and must route
 * wet audio in the requested direction (not merely rename an option).
 */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#define FS 44100u
#define CTL 16u
#define HALF_FRAMES 256u
#include "obv_dsp.h"
#include "obv_params.h"
#include "obv_x_drive.h"
#include "obv_x_fx_routing.h"
static volatile uint32_t ui_out_peak;
#include "obv_fx.h"

static void reset_fx(void)
{
    fx_used = 0;
    memset(fx_mem, 0, sizeof fx_mem);
    cho_ph = cho_inc = rv_ph1 = rv_ph2 = 0u;
    dly_lpl = dly_lpr = rv_lp_in = rv_lpl = rv_lpr = rv_yl = rv_yr = 0;
    dly_active = rev_active = 0u;
#if REV_DIV == 2u
    rv_hold = 0;
    fx_odd = 0u;
#endif
    fx_init();
    assert(fx_used == FX_MEM_N);
}

static uint64_t run_impulse(uint8_t mode, int delay_input, int reverb_input,
                            uint64_t *weighted)
{
    patch_t p;
    uint32_t block, i;
    uint64_t energy = 0u, sig = 0u;
    int32_t out[HALF_FRAMES * 2u];
    int32_t cs[HALF_FRAMES], ds[HALF_FRAMES], rs[HALF_FRAMES];
    patch_defaults(&p);
    p.v[P_CHO_MIX] = 0u;
    p.v[P_DLY_TIME] = DD_16;
    p.v[P_DLY_FB] = 48u;
    p.v[P_DLY_TONE] = 90u;
    p.v[P_DLY_MIX] = 127u;
    p.v[P_REV_SIZE] = 75u;
    p.v[P_REV_DAMP] = 40u;
    p.v[P_REV_PRE] = 0u;
    p.v[P_REV_MIX] = 127u;

    reset_fx();
    ctl_dv_type = opera_x_fx_with_route(DV_DIST, mode);
    fx_block(&p, 800u); /* short but musical tempo-synced delay */
    /* At least 0.4s of audio. The reverb has long allpass delays;
     * this is long enough for impulse reflections to appear. */
    for (block = 0u; block < 110u; ++block) {
        memset(out, 0, sizeof out);
        memset(cs, 0, sizeof cs);
        memset(ds, 0, sizeof ds);
        memset(rs, 0, sizeof rs);
        if (block == 0u) {
            if (delay_input) ds[0] = 21000;
            if (reverb_input) rs[0] = 21000;
        }
        fx_process(out, cs, ds, rs, HALF_FRAMES);
        for (i = 0u; i < 2u * HALF_FRAMES; ++i) {
            int32_t y = out[i];
            uint32_t a = (uint32_t)(y < 0 ? -y : y);
            energy += a;
            /* A deterministic weighted checksum detects a different
             * effect tail even when total RMS happens to match. */
            sig += (uint64_t)a * ((block * HALF_FRAMES * 2u + i) % 127u + 1u);
            assert(y >= -32767 && y <= 32767);
        }
    }
    *weighted = sig;
    return energy;
}

int main(void)
{
    uint8_t raw, route, drive;
    uint64_t ep, er, e1, e2, sp, sr, s1, s2;
    assert(opera_x_fx_route(0u) == OX_FX_PARALLEL);
    assert(opera_x_fx_route(255u) == OX_FX_PARALLEL); /* invalid -> safe */
    assert(opera_x_fx_chain_feed(1000, 2500, 1500, 0) == 1000);
    assert(opera_x_fx_chain_feed(1000, 2500, 1500, 128) == 2000);
    assert(opera_x_fx_chain_feed(32760, 32767, 32767, 180) == 32767);
    assert(opera_x_fx_chain_feed(-32760, -32767, -32767, 180) == -32767);
    assert(opera_x_fx_chain_feed(0, 2000, -2000, 180) == 0);
    /* Verify all mode/DRIVE combinations round-trip through the
     * original uint8 settings/pattern field, no structure changes. */
    for (route = 0u; route < OX_FX_ROUTE_COUNT; ++route)
        for (drive = 0u; drive < NDV; ++drive) {
            raw = opera_x_fx_with_route(drive, route);
            assert(opera_x_fx_route(raw) == route);
            assert(opera_x_fx_drive_type(raw) == drive);
            raw = opera_x_fx_with_drive(raw, (uint8_t)((drive + 1u) % NDV));
            assert(opera_x_fx_route(raw) == route);
            assert(opera_x_fx_drive_type(raw) == (drive + 1u) % NDV);
        }

    ep = run_impulse(OX_FX_PARALLEL, 1, 0, &sp);
    e1 = run_impulse(OX_FX_DELAY_REVERB, 1, 0, &s1);
    e2 = run_impulse(OX_FX_REVERB_DELAY, 1, 0, &s2);
    /* With ONLY a delay send, DLY>REV produces a reverb tail;
     * REV>DLY must be bit-identical to original delay only. */
    assert(ep > 1000u);
    assert(s1 != sp && e1 != ep);
    assert(s2 == sp && e2 == ep);

    er = run_impulse(OX_FX_PARALLEL, 0, 1, &sr);
    e1 = run_impulse(OX_FX_DELAY_REVERB, 0, 1, &s1);
    e2 = run_impulse(OX_FX_REVERB_DELAY, 0, 1, &s2);
    /* With ONLY a reverb send, REV>DLY makes echoed reverb tails;
     * DLY>REV stays identical to the original reverb-only sound. */
    assert(er > 1000u);
    assert(s1 == sr && e1 == er);
    assert(s2 != sr && e2 != er);

    printf("OPERA X FX ROUTE: parallel stable, DLY>REV/REV>DLY wet cascades and saved DRIVE fields pass\n");
    return 0;
}
