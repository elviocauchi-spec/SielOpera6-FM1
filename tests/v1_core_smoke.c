/* SPDX-License-Identifier: GPL-3.0-only
 * Host smoke tests for the standalone V-1 sound engine.
 * gcc -std=c11 -O2 -Wall -Wextra -o /tmp/v1_smoke tests/v1_core_smoke.c
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../firmware/v1/v1_core.h"
static v1_engine_t engine;
static int32_t buffer[2048u*2u];

static uint64_t render_energy(uint32_t n) {
    uint64_t sum=0;
    uint32_t i;
    memset(buffer,0,sizeof(buffer));
    v1_engine_render(&engine,buffer,n,128);
    for(i=0;i<n*2u;i++){
        int64_t x=buffer[i];
        sum+=(uint64_t)(x*x);
    }
    return sum;
}
static v1_patch_t p_init(void) {
    v1_patch_t p;
    memset(&p,0,sizeof(p));
    p.osc[0].wave=V1_SAW;p.osc[0].level=120;p.osc[0].pw=64;
    p.osc[1].wave=V1_PULSE;p.osc[1].level=90;p.osc[1].pw=64;
    p.osc[2].wave=V1_SINE;p.osc[2].level=70;p.osc[2].semi=-12;
    p.amp.attack=0;p.amp.decay=20;p.amp.sustain=100;p.amp.release=8;
    p.filter_env.attack=0;p.filter_env.decay=32;p.filter_env.sustain=0;p.filter_env.release=8;
    p.cutoff=80;p.resonance=20;p.filter_type=V1_LP12;p.filter_env_depth=20;
    p.trance_mask=0xffff;p.trance_depth=127;
    p.master=95;return p;
}
int main(void) {
    v1_patch_t p=p_init();
    uint64_t a,b,low,high;
    unsigned i,active=0;
    v1_engine_init(&engine,&p);
    v1_hold(&engine,1,60,100);
    a=render_energy(2048);
    if(a<1000000u){fprintf(stderr,"silence on main patch: %llu\n",(unsigned long long)a);return 1;}
    v1_hold(&engine,0,60,0);
    for(i=0;i<60;i++)render_energy(2048);
    for(i=0;i<V1_VOICES;i++)active+=engine.voices[i].active;
    if(active){fprintf(stderr,"stuck notes: %u\n",active);return 2;}
    p.osc[0].wave=V1_SUPERSAW;p.osc[0].detune=90;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    b=render_energy(2048);
    if(b<1000000u||a==b){fprintf(stderr,"supersaw failed: %llu\n",(unsigned long long)b);return 3;}
    p.trance_enabled=1;p.trance_mask=0;p.trance_depth=127;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    render_energy(2048);low=render_energy(2048);
    p.trance_enabled=0;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    render_energy(2048);high=render_energy(2048);
    if(low*10u>high){fprintf(stderr,"gate ineffective: %llu vs %llu\n",
        (unsigned long long)low,(unsigned long long)high);return 4;}
    p.arp_mode=V1_ARP_UP;p.arp_rate=2;p.arp_octaves=2;p.arp_gate=120;
    v1_engine_init(&engine,&p);
    v1_hold(&engine,1,60,100);v1_hold(&engine,1,64,100);v1_hold(&engine,1,67,100);
    if(engine.voices[0].active){fprintf(stderr,"arp started prematurely\n");return 5;}
    a=render_energy(2048);
    if(a<1000000u||!engine.arp_active){fprintf(stderr,"arp silent\n");return 6;}
    v1_hold(&engine,0,60,0);v1_hold(&engine,0,64,0);v1_hold(&engine,0,67,0);
    v1_panic(&engine);
    if(engine.held_count||engine.arp_active){fprintf(stderr,"arp held note stuck\n");return 7;}
    printf("V-1 core smoke PASS: voice lifecycle, supersaw, 16-step gate, chord-note arp\n");
    return 0;
}
