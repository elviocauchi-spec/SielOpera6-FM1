/* SPDX-License-Identifier: GPL-3.0-only
 * Measures every actual VG1 factory program through the FULL firmware DSP
 * including 3 oscillators, FAT, both filters, AMP DRIVE, polybus, effects.
 * Stress: one note, 3-note chord, 5-note cluster, 8-note chord.
 * Initially diagnostic-only: the output shows WHY presets sound overdriven.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#define VG1_AUDIO_DIAGNOSTICS 1
#include "../firmware/v1/vg1_factory.h"
static v1_engine_t e;
static int32_t block[256*2];
static const uint8_t offsets[8]={0,4,7,12,14,16,17,19};
static uint32_t render(uint32_t frames) {
    uint32_t peak=0;
    while(frames) {
        uint32_t n=frames>256?256:frames;
        memset(block,0,sizeof(block));
        v1_engine_render(&e,block,n,128);
        for(uint32_t i=0;i<n*2;i++) {
            uint32_t a=block[i]<0?(uint32_t)-block[i]:(uint32_t)block[i];
            if(a>peak)peak=a;
        }
        frames-=n;
    }
    return peak;
}
int main(void) {
    static const uint32_t chords[8]={1,2,3,4,5,6,7,8};
    static const uint8_t velocities[2]={100,127};
    uint32_t cases=0;
    for(uint32_t preset=0;preset<V1_PRESETS;preset++) {
        for(uint32_t vi=0;vi<2;vi++) {
            uint8_t velocity=velocities[vi];
            for(uint32_t k=0;k<8;k++) {
                uint32_t n=chords[k];
                uint8_t note=(preset==1||preset==4)?43:(preset==2||preset==7)?72:60;
                v1_engine_init(&e,&V1_FACTORY[preset]);
                for(uint32_t j=0;j<n;j++)
                    v1_hold(&e,1,(uint8_t)(note+offsets[j]),velocity);
                uint32_t peak=render(V1_SR);
                printf("HEADROOM %-14s velocity=%u notes=%u peak=%u raw_osc=%u stack_clip=%u predrive=%u drivehot=%u driveplateau=%u dry65k=%u busclip=%u panclip=%u delayclip=%u predelay=%u reverbclip=%u finalclip=%u\n",
                       V1_FACTORY[preset].name,velocity,n,peak,e.diag_osc_raw_over,e.diag_osc_over,
                       e.diag_pre_drive_over,e.diag_drive_over,e.diag_drive_plateau,
                       e.diag_poly_mix_over,e.diag_post_gain_clamp,
                       e.diag_pan_over,e.diag_delay_over,e.diag_predelay_over,
                       e.diag_reverb_over,e.diag_final_over);
                if(e.diag_osc_over||e.diag_drive_plateau||e.diag_post_gain_clamp||
                   e.diag_delay_over||e.diag_predelay_over||e.diag_reverb_over||
                   e.diag_final_over||peak>=32767||peak<100u) {
                    fprintf(stderr,
                        "FACTORY HEADROOM REGRESSION: %s, %u notes, velocity %u\n",
                        V1_FACTORY[preset].name,n,velocity);
                    return 1;
                }
                cases++;
            }
        }
    }
    printf("PASS: %u factory stress cases, 8 presets x 1..8 notes x 2 velocities; zero accidental internal hard saturation or DAC clipping\n",cases);
    return 0;
}
