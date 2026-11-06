/* SPDX-License-Identifier: GPL-3.0-only
 * Diagnose hard saturation masked by FM1's soft output guard.
 * Exercises a wider pitch/modulation/stereo envelope than factory smoke.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define VG1_AUDIO_DIAGNOSTICS 1
#include "../firmware/v1/vg1_factory.h"
static v1_engine_t engine;
static int32_t samples[512];
static const uint8_t intervals[8]={0,4,7,12,14,16,17,19};
static uint32_t run(uint32_t frames) {
    uint32_t peak=0;
    while(frames) {
        uint32_t n=frames>256u?256u:frames;
        memset(samples,0,sizeof samples);
        v1_engine_render(&engine,samples,n,128u);
        for(uint32_t i=0;i<2u*n;i++) {
            uint32_t x=samples[i]<0?(uint32_t)-samples[i]:(uint32_t)samples[i];
            if(x>peak)peak=x;
        }
        frames-=n;
    }
    return peak;
}
static uint32_t filter_overloads(void) {
    uint32_t c=0;
    for(uint32_t i=0;i<V1_VOICES;i++) {
        const v1_voice_t *v=&engine.voices[i];
        c+=v->f1.diag_state_clamps+v->f2.diag_state_clamps
          +v->f1_right.diag_state_clamps+v->f2_right.diag_state_clamps;
    }
    return c;
}
int main(void) {
    const uint8_t roots[]={36,48,60,72,84};
    const uint8_t count[]={1,3,5,8};
    uint32_t cases=0,bad=0,filter=0,bus=0,pan=0,fx=0,guard=0;
    for(uint32_t p=0;p<V1_PRESETS;p++)
    for(uint32_t k=0;k<sizeof roots;k++)
    for(uint32_t ci=0;ci<4;ci++)
    for(uint32_t mod=0;mod<2;mod++) {
        v1_patch_t patch=V1_FACTORY[p];
        v1_engine_init(&engine,&patch);
        engine.modwheel=mod?127:0;
        engine.aftertouch=mod?127:0;
        engine.bend14=mod?8191:0;
        for(uint32_t i=0;i<count[ci];i++)
            v1_hold(&engine,1,(uint8_t)(roots[k]+intervals[i]),127);
        uint32_t peak=run(V1_SR/6);
        uint32_t f=filter_overloads();
        uint32_t b=engine.diag_post_gain_clamp;
        uint32_t pn=engine.diag_pan_over;
        uint32_t x=engine.diag_delay_over+engine.diag_predelay_over+
                   engine.diag_reverb_over;
        uint32_t g=engine.diag_output_guard;
        if(f||b||pn||x||engine.diag_final_hot||engine.diag_osc_over||
           engine.diag_drive_plateau||peak>=32767) {
            printf("GAIN_MATRIX %-14s root=%u voices=%u mod=%u peak=%u filter=%u osc=%u driveplateau=%u bus=%u pan=%u fx=%u finalhot=%u guard=%u\n",
                   patch.name,roots[k],count[ci],mod,peak,f,engine.diag_osc_over,
                   engine.diag_drive_plateau,b,pn,x,engine.diag_final_hot,g);
            bad++;
        }
        filter+=f;bus+=b;pan+=pn;fx+=x;guard+=g;cases++;
    }
    printf("GAIN MATRIX TOTAL cases=%u flagged=%u filter=%u bus=%u pan=%u fx=%u output_guard=%u\n",
           cases,bad,filter,bus,pan,fx,guard);
    return 0; /* diagnostic mode until the signal chain is corrected */
}
