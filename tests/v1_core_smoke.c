/* SPDX-License-Identifier: GPL-3.0-only
 * Vanguard 1.x recreation: host DSP contract tests.
 * Every test here checks a documented Vanguard 1 parameter relationship,
 * not generic V-1 prototype synthesis.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../firmware/v1/v1_core.h"
static v1_engine_t engine;
static int32_t buffer[4096u*2u];

static uint64_t render_energy(uint32_t n,int ch) {
    uint64_t sum=0;uint32_t i;
    memset(buffer,0,n*2u*sizeof(buffer[0]));
    v1_engine_render(&engine,buffer,n,128);
    for(i=0;i<n;i++) {
        if(ch<0) {int64_t l=buffer[i*2],r=buffer[i*2+1];sum+=(uint64_t)(l*l+r*r);}
        else {int64_t x=buffer[i*2u+(uint32_t)ch];sum+=(uint64_t)(x*x);}
    }
    return sum;
}
static uint64_t render_stereo_difference(uint32_t n) {
    uint64_t sum=0;uint32_t i;
    memset(buffer,0,n*2u*sizeof(buffer[0]));
    v1_engine_render(&engine,buffer,n,128);
    for(i=0;i<n;i++) {int64_t d=(int64_t)buffer[i*2u]-buffer[i*2u+1u];sum+=(uint64_t)(d*d);}
    return sum;
}
static v1_patch_t init_patch(void) {
    v1_patch_t p;memset(&p,0,sizeof p);
    p.osc[0].wave=VG1_W_SAWTOOTH;p.osc[0].level=110;p.osc[0].pw=64;
    p.osc[1].wave=VG1_W_PULSE_PWM;p.osc[1].level=85;p.osc[1].pw=64;
    p.osc[2].wave=VG1_W_SINE;p.osc[2].level=65;p.osc[2].semi=-12;p.osc[2].pw=64;
    p.amp.attack=1;p.amp.decay=15;p.amp.sustain=100;p.amp.release=10;
    p.filter_env.attack=1;p.filter_env.decay=26;p.filter_env.sustain=0;p.filter_env.release=11;
    p.cutoff=87;p.resonance=19;p.filter_type=VG1_F_LOW12;
    p.vg_amp.level=103;p.vg_amp.pan=64;
    p.vg_gate.left=p.vg_gate.right=0xffffu;p.vg_gate.contour=104;
    p.vg_gate.mix=127;p.vg_gate.speed.denominator=16;
    p.vg_osc.vcs=2;p.voice_limit=8;p.master=90;
    return p;
}
#define VERIFY(test, why, code) do {if(!(test)){fprintf(stderr,"Vanguard FAIL: %s\n",why);return code;}}while(0)
int main(void) {
    v1_patch_t p=init_patch();
    uint64_t a,b,left,right,diff_mono,diff_fat,low,high;
    uint32_t i,active=0;
    VERIFY(VG1_N_WAVES==32 && VG1_N_FILTERS==13,"31 wave types + off; 13 filter types",1);
    v1_engine_init(&engine,&p);
    v1_hold(&engine,1,60,90);
    a=render_energy(4096,-1);
    VERIFY(a>1000000u,"standard Vanguard patch silent",2);
    v1_hold(&engine,0,60,0);
    for(i=0;i<28;i++)render_energy(4096,-1);
    for(i=0;i<V1_VOICES;i++)active+=engine.voices[i].active;
    VERIFY(!active,"note-off left hanging voice",3);

    /* Every named waveform must have a real DSP dispatch, not a duplicate
       static enum placeholder or a silent entry. */
    for(i=1;i<VG1_N_WAVES;i++) {
        p=init_patch();p.osc[0].wave=i;p.osc[1].level=p.osc[2].level=0;
        v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
        a=render_energy(2048,-1);
        if(a<10000u){fprintf(stderr,"Vanguard FAIL: waveform %u (%s) silent\n",i,vg1_wave_name[i]);return 4;}
    }
    for(i=0;i<VG1_N_FILTERS;i++) {
        p=init_patch();p.filter_type=i;
        v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
        a=render_energy(2048,-1);
        if(a<10000u){fprintf(stderr,"Vanguard FAIL: filter %u (%s) silent\n",i,vg1_filter_name[i]);return 5;}
    }
    /* Vanguard FAT creates separate detuned L/R audio, not just a centered
       supersaw. A monophonic baseline should have little stereo difference. */
    p=init_patch();p.vg_osc.fat=0;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    diff_mono=render_stereo_difference(2048);
    p.vg_osc.fat=85;p.vg_osc.vcs=6;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    diff_fat=render_stereo_difference(2048);
    VERIFY(diff_fat>diff_mono*3u+100000u,"FAT/VCS missing stereo detune",6);

    /* Unlike a generic synth, Vanguard VELTRK 0 disables sensitivity. */
    p=init_patch();p.vg_amp.veltrk=0;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,28);
    a=render_energy(2048,-1);
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,119);
    b=render_energy(2048,-1);
    VERIFY(a==b,"VELTRK=0 must make velocity irrelevant",7);
    p.vg_amp.veltrk=115;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,28);
    a=render_energy(2048,-1);
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,119);
    b=render_energy(2048,-1);
    VERIFY(b>a*2u,"Vanguard positive VELTRK must favor high velocity",8);

    /* AMP PAN at extreme must fully attenuate the opposite channel. */
    p=init_patch();p.vg_amp.pan=0;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    left=render_energy(2048,0);
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    right=render_energy(2048,1);
    VERIFY(left>10000u && right==0,"hard left AMP PAN",9);

    p=init_patch();p.vg_gate.enabled=1;p.vg_gate.stereo=1;
    p.vg_gate.left=0;p.vg_gate.right=0xffff;p.vg_gate.mix=127;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    left=render_energy(2048,0);
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    right=render_energy(2048,1);
    VERIFY(right>left*4u+10000u,"stereo 16-step Trancegate is mono",10);

    p=init_patch();p.vg_gate.enabled=1;p.vg_gate.left=p.vg_gate.right=0;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    render_energy(2048,-1);low=render_energy(2048,-1);
    p.vg_gate.enabled=0;
    v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    render_energy(2048,-1);high=render_energy(2048,-1);
    VERIFY(high>low*10u,"Vanguard trancegate switch/depth ineffective",11);

    p=init_patch();p.vg_arp.on=1;p.vg_arp.mode=VG1_ARP_ORDERED;
    p.vg_arp.octaves=2;p.vg_arp.speed.denominator=16;p.vg_arp.gate=120;
    v1_engine_init(&engine,&p);
    v1_hold(&engine,1,67,100);v1_hold(&engine,1,60,100);v1_hold(&engine,1,64,100);
    VERIFY(!engine.voices[0].active,"ARP on: held notes should not chord",12);
    a=render_energy(2048,-1);
    VERIFY(a>10000u && engine.arp_active && engine.arp_note==67,
           "ordered ARP must follow held note order",13);
    v1_hold(&engine,0,67,0);v1_hold(&engine,0,60,0);v1_hold(&engine,0,64,0);
    VERIFY(engine.held_count==0 && !engine.arp_active,"ARP stopped with no held notes",14);
    /* Vanguard GLIDE switches to single-note legato, moving smoothly between
       held notes at the tempo-synced rate. There must not be a new poly voice. */
    p=init_patch();p.vg_glide.on=1;p.vg_glide.speed.denominator=16;
    v1_engine_init(&engine,&p);
    v1_hold(&engine,1,60,100);
    render_energy(2048,-1);
    v1_hold(&engine,1,64,100);
    VERIFY(engine.voices[0].note==64 &&
           engine.glide_current_q16==(60<<16) &&
           engine.glide_target_q16==(64<<16),"GLIDE new legato note",15);
    render_energy(2048,-1);
    VERIFY(engine.glide_current_q16>(60<<16) &&
           engine.glide_current_q16<(64<<16),"GLIDE must interpolate pitch",16);
    for(i=1;i<V1_VOICES;i++)active+=engine.voices[i].active;
    VERIFY(!active,"GLIDE must be monophonic",17);
    v1_hold(&engine,0,64,0);
    VERIFY(engine.voices[0].note==60,"GLIDE must return to held note",18);
    v1_hold(&engine,0,60,0);

    /* MIDI CC7 + CC11 are separate multipliers, not changes to AMP LEVEL. */
    p=init_patch();v1_engine_init(&engine,&p);v1_hold(&engine,1,60,100);
    a=render_energy(2048,-1);
    v1_engine_init(&engine,&p);engine.midi_volume=63;v1_hold(&engine,1,60,100);
    b=render_energy(2048,-1);
    VERIFY(a>b*2u,"MIDI CC7 output volume should attenuate",19);

    puts("Vanguard 1 DSP PASS: 31 waves, 13 filters, stereo FAT, VELTRK, PAN, Trancegate, ordered arp, glide, MIDI gain");
    return 0;
}
