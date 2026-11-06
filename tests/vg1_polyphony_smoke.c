/* SPDX-License-Identifier: GPL-3.0-only
 * Polyphonic FAT load / headroom regression. Host runtime is indicative only:
 * the AC79 clock speed and emulator timing require real device validation.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define VG1_AUDIO_DIAGNOSTICS 1
#include "../firmware/v1/vg1_factory.h"
static v1_engine_t engine;
static int32_t samples[256u*2u];
#define CHECK(a,msg) do { if(!(a)) {fprintf(stderr,"FAIL: %s\n",msg); return 1;} } while(0)
static uint32_t render_frames(uint32_t frames) {
    uint32_t peak=0;
    while(frames) {
        uint32_t n=frames>256?256:frames;
        memset(samples,0,sizeof(samples));
        v1_engine_render(&engine,samples,n,128u);
        for(uint32_t i=0;i<n*2u;i++) {
            uint32_t a=(uint32_t)(samples[i]<0?-samples[i]:samples[i]);
            if(a>peak)peak=a;
        }
        frames-=n;
    }
    return peak;
}
int main(void) {
    const v1_patch_t *p=&V1_FACTORY[3]; /* Gated Motion */
    CHECK(vg1_fat_render_count(p,1)==4,"one-note original FAT = 4");
    CHECK(vg1_fat_render_count(p,2)==4,"two-note original FAT = 4");
    CHECK(vg1_fat_render_count(p,3)==3,"three-note FAT adaptive = 3");
    CHECK(vg1_fat_render_count(p,4)==2,"four-note stereo FAT adaptive = 2");
    CHECK(vg1_fat_render_count(p,6)==1,"six-note polyphony never exceeds budget");
    /* 8-note polyphonic input: C4 E4 G4 C5 D5 E5 F5 G5. */
    const uint8_t notes[8]={60,64,67,72,74,76,77,79};
    for(uint32_t n=1;n<=8;n++) {
        uint32_t active=0;
        v1_engine_init(&engine,p);
        for(uint32_t i=0;i<n;i++)v1_hold(&engine,1,notes[i],100);
        for(uint32_t i=0;i<V1_VOICES;i++)
            if(engine.voices[i].active&&engine.voices[i].gate)active++;
        CHECK(active==n,"fourth key must actually trigger a fourth voice");
        clock_t t0=clock();
        uint32_t peak=render_frames(V1_SR);
        clock_t t1=clock();
        printf("GATED MOTION %u-note chord: FAT/note=%u, host_ms=%.1f, peak=%u, osc=%u, drive=%u, MIX16=%u, MIX65K=%u, pan=%u, delay=%u, predelay=%u, reverb=%u, final=%u\n",
               n,vg1_fat_render_count(p,n),1000.0*(t1-t0)/CLOCKS_PER_SEC,
               peak,engine.diag_osc_over,engine.diag_drive_over,
               engine.diag_poly_mix_16_over,engine.diag_poly_mix_over,
               engine.diag_pan_over,engine.diag_delay_over,
               engine.diag_predelay_over,engine.diag_reverb_over,
               engine.diag_final_over);
        CHECK(peak>300u,"polyphony audible");
        /* Stage metrics are diagnostic initially: do not gate CI until the
           correction is complete and high-density chords are clean. */
        for(uint32_t i=0;i<n;i++)v1_hold(&engine,0,notes[i],0);
        CHECK(engine.held_count==0,"polyphony release must remove all held notes");
        render_frames(3*V1_SR);
        for(uint32_t i=0;i<V1_VOICES;i++)
            CHECK(!engine.voices[i].active,"polyphonic voice must fully release");
    }
    puts("PASS: 1..8-note Gated Motion render and voice release; overload counters logged");
    return 0;
}
