/* SPDX-License-Identifier: GPL-3.0-only
 * Validates every imported ProSounds FreeSet 1 approximation independently
 * of the rigorously calibrated eight original VG-1 programs.
 * VST2 parameters are a provisional mapping; ARP/GATE switches verified.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define VG1_AUDIO_DIAGNOSTICS 1
#include "../firmware/v1/vg1_factory.h"
static v1_engine_t e;
static int32_t buf[512];
#define CHECK(cond,msg) do {if(!(cond)){fprintf(stderr,"IMPORT FAIL: preset=%u (%s): %s\n",index,V1_FACTORY[index].name,msg); return 1;}} while(0)
static uint32_t render(uint32_t frames) {
    uint32_t peak=0;
    while(frames) {
        uint32_t n=frames>256u?256u:frames;
        memset(buf,0,sizeof buf);
        v1_engine_render(&e,buf,n,128u);
        for(uint32_t i=0;i<2u*n;i++) {
            uint32_t amp=buf[i]<0?(uint32_t)-buf[i]:(uint32_t)buf[i];
            if(amp>peak)peak=amp;
        }
        frames-=n;
    }
    return peak;
}
int main(void) {
    uint32_t distinct=0,maximum=0;
    for(uint32_t index=VG1_AUTHORED_PRESETS;index<V1_PRESETS;index++) {
        const v1_patch_t *p=&V1_FACTORY[index];
        CHECK(p->source_bank==1,"import marker missing");
        CHECK(p->name[0]!=0,"empty import program name");
        if(index>VG1_AUTHORED_PRESETS &&
           strcmp(p->name,V1_FACTORY[index-1].name)!=0)distinct++;
        if(index>=VG1_AUTHORED_PRESETS+46u && index<VG1_AUTHORED_PRESETS+90u)
            CHECK(p->vg_gate.enabled,"source Gated section lost gate enable");
        if(index>=VG1_AUTHORED_PRESETS+91u)
            CHECK(p->vg_arp.on,"source Arpeggiated section lost ARP enable");
        /* Exclude clipping and silence for 1,3,5-note playing in each patch;
           keep host CPU cost bounded so CI runs on every commit. */
        for(uint32_t notes=1u;notes<=5u;notes+=2u) {
            static const uint8_t offsets[5]={0,4,7,12,16};
            v1_engine_init(&e,p);
            for(uint32_t k=0;k<notes;k++)
                v1_hold(&e,1,(uint8_t)(48u+offsets[k]),100);
            uint32_t peak=render(V1_SR/2u);
            if(peak>maximum)maximum=peak;
            CHECK(peak>10u,"imported preset silent");
            CHECK(peak<32767u,"imported preset hit full-scale digital hard clip");
            CHECK(!e.diag_final_over,"imported preset exceeded final DAC");
            for(uint32_t k=0;k<notes;k++)
                v1_hold(&e,0,(uint8_t)(48u+offsets[k]),0);
            CHECK(e.held_count==0u,"note-off held-key leak");
        }
    }
    printf("PASS: %u imported program slots, %u consecutive distinct names, %u audio cases; max PCM=%u\n",
           VG1_PROSOUNDS_PRESETS,distinct,VG1_PROSOUNDS_PRESETS*3u,maximum);
    return 0;
}
