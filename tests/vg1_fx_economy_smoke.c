/* SPDX-License-Identifier: GPL-3.0-only
 * FM1 emergency economy path: when FAT has reached one layer/voice,
 * halve the four-comb reverb cost while preserving all eight note voices.
 * This test uses actual production DSP; timing is host-only, not AC79.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#define VG1_AUDIO_DIAGNOSTICS 1
#include "../firmware/v1/vg1_factory.h"
static v1_engine_t engine;
static int32_t pcm[512];
static const uint8_t notes[8]={60,64,67,72,74,76,77,79};
#define CHECK(cond,msg) do{if(!(cond)){fprintf(stderr,"FAIL: %s\n",msg);return 1;}}while(0)
static uint32_t render(uint32_t frames) {
    uint32_t peak=0;
    while(frames) {
        uint32_t n=frames>256?256:frames;
        memset(pcm,0,sizeof pcm);
        v1_engine_render(&engine,pcm,n,128);
        for(uint32_t i=0;i<2u*n;i++) {
            uint32_t a=pcm[i]<0?(uint32_t)-pcm[i]:(uint32_t)pcm[i];
            if(a>peak)peak=a;
        }
        frames-=n;
    }
    return peak;
}
int main(void) {
    const uint8_t presets[3]={0,3,5}; /* Heaven Lead, Gated Motion, Saturn Pad */
    for(uint32_t i=0;i<3;i++) {
        for(uint32_t eco=0;eco<2;eco++) {
            const v1_patch_t *p=&V1_FACTORY[presets[i]];
            v1_engine_init(&engine,p);
            for(uint32_t k=0;k<8;k++)v1_hold(&engine,1,notes[k],100);
            engine.runtime_reverb_economy=(uint8_t)eco;
            clock_t start=clock();
            uint32_t peak=render(V1_SR);
            clock_t end=clock();
            uint32_t active=0;
            for(uint32_t k=0;k<V1_VOICES;k++)
                if(engine.voices[k].active&&engine.voices[k].gate)active++;
            printf("FX ECONOMY %-14s mode=%s host_cpu_ms=%.2f peak=%u active=%u fade=%d clip=%u,%u,%u\n",
                   p->name,eco?"two-comb":"four-comb",
                   1000.0*(end-start)/CLOCKS_PER_SEC,peak,active,
                   engine.reverb_late_gain_q15,engine.diag_reverb_over,
                   engine.diag_predelay_over,engine.diag_final_over);
            CHECK(active==8,"economy must not suppress any of eight voices");
            CHECK(peak>100 && peak<32767,"economy audio must remain audible and unclipped");
            CHECK(!engine.diag_reverb_over&&!engine.diag_final_over,
                  "economy must not introduce internal saturation");
            CHECK(eco?engine.reverb_late_gain_q15==0:
                      engine.reverb_late_gain_q15==32767,
                  "late-comb fade must reach exact target without popping");
            /* Return to full quality after overload ends, in 2ms. */
            engine.runtime_reverb_economy=0;
            render(256);
            CHECK(engine.reverb_late_gain_q15==32767,
                  "economy must restore all four combs after the overload");
            for(uint32_t k=0;k<8;k++)v1_hold(&engine,0,notes[k],0);
            CHECK(engine.held_count==0,"all note-offs retained during economy");
            render(3*V1_SR);
        }
    }
    puts("PASS: 8 voices in full/economy reverb, no DAC saturation, all releases, reversible fade");
    return 0;
}
