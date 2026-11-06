/* SPDX-License-Identifier: GPL-3.0-only
 * VG-1 click suppression regressions for the actual FM1 Trancegate core.
 * Compile: gcc -std=c11 -O2 -Wall -Wextra tests/vg1_gate_smoke.c -o /tmp/vg1_gate_smoke
 */
#include <stdio.h>
#include <stdint.h>
#include "../firmware/v1/v1_core.h"

#define REQUIRE(x, msg) do { if(!(x)) { fprintf(stderr,"FAIL: %s\n",msg);return 1;} }while(0)

int main(void) {
    const int32_t alpha=vg1_gate_contour_alpha(104u);
    int32_t intermediate=32767,output=32767,last=32767;
    uint32_t i,peak_step=0;
    REQUIRE(alpha>0 && alpha<32767,"valid contour coefficient");
    for(i=0;i<512u;i++) {
        intermediate=vg1_gate_smooth(intermediate,0,alpha);
        output=vg1_gate_smooth(output,intermediate,alpha);
        uint32_t jump=(uint32_t)(last>output?last-output:output-last);
        if(jump>peak_step)peak_step=jump;
        REQUIRE(output<=last,"release must be monotone");
        REQUIRE(output>=0,"release must stay nonnegative");
        last=output;
    }
    REQUIRE(output==0,"gate must fully close without fixed point DC leakage");
    REQUIRE(peak_step<500u,"gate must not have near-instantaneous click edges");
    for(i=0;i<512u;i++) {
        intermediate=vg1_gate_smooth(intermediate,32767,alpha);
        output=vg1_gate_smooth(output,intermediate,alpha);
        uint32_t jump=(uint32_t)(last>output?last-output:output-last);
        if(jump>peak_step)peak_step=jump;
        REQUIRE(output>=last,"attack must be monotone");
        REQUIRE(output<=32767,"attack must stay below unity");
        last=output;
    }
    REQUIRE(output==32767,"gate must fully open");
    REQUIRE(peak_step<500u,"attack must not click");
    REQUIRE(vg1_gate_contour_alpha(127)>alpha,
            "raising contour should sharpen gate");
    REQUIRE(vg1_gate_contour_alpha(0)<alpha,
            "lowering contour should soften gate");
    printf("PASS: stereo Trancegate contour envelope is pop-safe: 0-to-full transitions, max step=%u/32767\n",
           peak_step);
    return 0;
}
