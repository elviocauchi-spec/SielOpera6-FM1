/* SPDX-License-Identifier: GPL-3.0-only
 * V-1 FM1 application. Independent virtual-analog monosynth/polysynth UI.
 * The platform provides ONLY board startup, audio, physical I/O and firmware recovery.
 */
#define APP_NAME "VG-1 FM1"
static void app_idle(void);
#define LCD_IDLE() app_idle()
#include "../src/platform.c"
#include "v1_core.h"

#define V1_KEY0 53u                /* FM1 physical key 0: F3 */
#define V1_BPM_MIN 40u
#define V1_BPM_MAX 240u
#include "vg1_factory.h"
#include "vg1_key_router.h"

static v1_engine_t v1 __attribute__((section(".pool")));
static volatile uint32_t req_program;
static volatile uint32_t vg1_panic_requested;
static uint32_t got_program, page, octave, bpm=128, panel_last;
static vg1_key_router_t v1_keyboard;
static volatile uint32_t v1_dsp_pct; /* time in our DSP relative to the 256-frame audio deadline */
/* User-facing pages follow the original Vanguard 1.6 section names.
 * The small FM1 screen navigates the same controls with four encoders.
 * The oscillator/filter menus display their documented entries verbatim. */
enum { UI_PLAY,
       UI_OSC1,UI_OSC2,UI_OSC3,UI_OSC_FAT,
       UI_LFO1,UI_LFO2,UI_LFO3,
       UI_FILTER,UI_FILTER_TRACK,
       UI_ENV1,UI_ENV1_MOD,UI_ENV2,UI_ENV2_MOD,
       UI_AMP,UI_AMP_STEREO,
       UI_GATE,UI_GATE_STEPS_L,UI_GATE_STEPS_R,UI_GATE_RATE,
       UI_DELAY,UI_DELAY_MORE,
       UI_REVERB,UI_REVERB_MORE,
       UI_ARP,UI_ARP_MORE,
       UI_GLIDE,UI_PITCH,UI_VOICES,
       UI_PAGES };
static const char *const UI_TITLES[UI_PAGES] = {
    "VANGUARD 1","OSCILLATOR 1","OSCILLATOR 2","OSCILLATOR 3","OSC FAT/VCS",
    "LFO 1","LFO 2","LFO 3","FILTER","FILTER TRACK",
    "ENVELOPE 1","ENV 1 ROUTING","ENVELOPE 2","ENV 2 ROUTING",
    "AMPLIFIER","AMP STEREO",
    "TRANCEGATE","TRANCEGATE L","TRANCEGATE R","GATE TIMING",
    "DELAY","DELAY MODES","REVERB","REVERB SPACE",
    "ARPEGGIATOR","ARP OPTIONS","GLIDE","PITCH","VOICES"
};
static const char *const UI_LABELS[UI_PAGES][4] = {
    {"CUTOFF","RESO","FAT","BPM"},
    {"WAVE","LEVEL","SEMI","PWM"},
    {"WAVE","LEVEL","SEMI","PWM"},
    {"WAVE","LEVEL","SEMI","PWM"},
    {"RETRIG","FAT","VCS","AGE"},
    {"SPEED","DETUNE","PWM","CUTOFF"},
    {"SPEED","DETUNE","PWM","CUTOFF"},
    {"SPEED","DETUNE","PWM","CUTOFF"},
    {"TYPE","CUTOFF","RESO","KEYTRK"},
    {"KEYTRK","VELTRK","--","--"},
    {"ATTACK","DECAY","SUSTAIN","RELEASE"},
    {"LEVEL","PWM","CUTOFF","DETUNE"},
    {"ATTACK","DECAY","SUSTAIN","RELEASE"},
    {"RESO","PWM","CUTOFF","PITCH"},
    {"DRIVE","LEVEL","VELTRK","PAN"},
    {"SPREAD","--","--","--"},
    {"ON/OFF","STEREO","MIX","CONTOUR"},
    {"STEP","ON/OFF","ROTATE","ALL/CLR"},
    {"STEP","ON/OFF","ROTATE","ALL/CLR"},
    {"SPEED","TRIP/DOT","SYNC POS","BPM"},
    {"ON/OFF","TYPE","MIX","SPEED"},
    {"FEEDBACK","DAMP","TRIP/DOT","WIDEN MS"},
    {"ON/OFF","MIX","PRE-DLY","ROOMSIZE"},
    {"DAMP","WIDTH","--","--"},
    {"ON/OFF","MODE","SPEED","GATE"},
    {"OCTAVES","RETRIG","TRIP/DOT","--"},
    {"ON/OFF","SPEED","TRIP/DOT","--"},
    {"PB RANGE","VIB SPEED","VIB DEPTH","--"},
    {"VOICES","MASTER","--","--"}
};
static const char *const RATE_NAMES[]={"1/8","1/16","1/32"};
static uint32_t vg1_gate_cursor;
static const char *const STEP_HEX[]={"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};

typedef struct { uint8_t on,note,vel; } v1_note_msg_t;
#define V1_Q 64u
static v1_note_msg_t msgs[V1_Q];
static volatile uint32_t qw,qr;
static void v1_queue(uint32_t on,uint32_t note,uint32_t vel) {
    uint32_t i=qw;
    if(i-qr>=V1_Q) {
        /* Dropping a NOTE-OFF leaves the synth sustaining forever. Under
           abnormal MIDI bursts, fail safe by releasing all held voices
           instead of allowing an unbounded stuck note. */
        if(!on)vg1_panic_requested=1;
        return;
    }
    msgs[i&(V1_Q-1u)].on=(uint8_t)on;
    msgs[i&(V1_Q-1u)].note=(uint8_t)note;
    msgs[i&(V1_Q-1u)].vel=(uint8_t)vel;
    __asm__ volatile("" ::: "memory");
    qw=i+1;
}
static void app_audio(int32_t *out,uint32_t n) {
    uint32_t t0=fm1_ticks();
    if(vg1_panic_requested) {
        vg1_panic_requested=0;
        v1_panic(&v1);
        qr=qw; /* emergency all-sound-off: discard stale queued note-ons */
    }
    if(req_program!=got_program) {
        got_program=req_program%V1_PRESETS;
        v1_panic(&v1);
        v1.patch=V1_FACTORY[got_program];
        v1.gate_phase=v1.gate_step=v1.arp_phase=v1.arp_pos=0;
        v1.gate_smooth=v1.gate_smooth_right=32767;
        v1.gate_out_left=v1.gate_out_right=32767;
        v1.runtime_fat_limit=0; /* new program restores its configured FAT */
    }
    while(qr!=qw) {
        v1_note_msg_t m=msgs[qr&(V1_Q-1u)];
        qr++;
        v1_hold(&v1,m.on,m.note,m.vel);
    }
    v1_engine_render(&v1,out,n,bpm);
    {
        uint32_t usec=(fm1_ticks()-t0)/FM1_TICKS_PER_US;
        uint32_t deadline=(n*1000000u)/V1_SR;
        v1_dsp_pct=deadline ? (usec*100u)/deadline : 0;
        /* AC79 safety governor: never repeat a measured high-load block
           at the same FAT setting. It only drops one VCS layer at a time
           and never suppresses a held key. The conservative polyphonic
           baseline normally avoids needing this last-resort measure. */
        if(v1_dsp_pct>78u && v1.last_render_vcs>1u)
            v1.runtime_fat_limit=v1.last_render_vcs-1u;
        if(!v1.held_count) {
            uint32_t i,active=0;
            for(i=0;i<V1_VOICES;i++)
                active|=v1.voices[i].active;
            if(!active)v1.runtime_fat_limit=0;
        }
    }
}
static void app_idle(void) { platform_poll(); }
static uint32_t v1_bound(int32_t v,int32_t max) {
    if(v<0)return 0;
    if(v>max)return (uint32_t)max;
    return (uint32_t)v;
}
static void vg1_adj_u8(uint8_t *p,int32_t delta,int32_t lo,int32_t hi)
{ *p=(uint8_t)v1_clip((int32_t)*p+delta,lo,hi); }
static void vg1_adj_i8(int8_t *p,int32_t delta,int32_t lo,int32_t hi)
{ *p=(int8_t)v1_clip((int32_t)*p+delta,lo,hi); }
static void vg1_adj_rate(vg1_rate_t *r,uint32_t k,int32_t d) {
    if(k==0) {
        uint32_t den=r->denominator?r->denominator:16u,ix=0;
        while(den>1u&&ix<6u){den>>=1u;ix++;}
        ix=(uint32_t)v1_clip((int32_t)ix+d,0,6);
        r->denominator=(uint8_t)(1u<<ix);
    } else vg1_adj_u8(&r->variant,d,0,2);
}
static int32_t vg1_get_signed(uint32_t pg,uint32_t k) {
    const v1_patch_t *p=&v1.patch;
    if(pg==UI_OSC1||pg==UI_OSC2||pg==UI_OSC3)
        return k==2 ? p->osc[pg-UI_OSC1].semi : 0;
    if(pg==UI_LFO1||pg==UI_LFO2||pg==UI_LFO3){
        const vg1_lfo_patch_t *l=&p->vg_lfo[pg-UI_LFO1];
        return k==1?l->detune:k==2?l->pwm:k==3?l->cutoff:0;
    }
    if(pg==UI_FILTER||pg==UI_FILTER_TRACK)
        return k==3||k==0 ? p->vg_keytrack.keytrk : k==1?p->vg_keytrack.veltrk:0;
    if(pg==UI_ENV1_MOD){
        return k==0?p->vg_env1.level:k==1?p->vg_env1.pwm:k==2?p->vg_env1.cutoff:p->vg_env1.detune;
    }
    if(pg==UI_ENV2_MOD){
        return k==0?p->vg_env2.resonance:k==1?p->vg_env2.pwm:k==2?p->vg_env2.cutoff:p->vg_env2.pitch;
    }
    if(pg==UI_AMP&&k==2)return p->vg_amp.veltrk;
    return 0;
}
static uint32_t vg1_is_signed(uint32_t pg,uint32_t k) {
    return ((pg>=UI_OSC1&&pg<=UI_OSC3&&k==2)||
           (pg>=UI_LFO1&&pg<=UI_LFO3&&k>0)||
           (pg==UI_FILTER&&k==3)||
           (pg==UI_FILTER_TRACK&&k<2)||
           (pg==UI_ENV1_MOD)||(pg==UI_ENV2_MOD)||
           (pg==UI_AMP&&k==2));
}
static void v1_param_change(uint32_t pg,uint32_t k,int32_t n) {
    v1_patch_t *p=&v1.patch;
    v1_osc_patch_t *o;
    vg1_lfo_patch_t *l;
    v1_adsr_patch_t *a;
    uint16_t *mask;
    if(pg>=UI_OSC1&&pg<=UI_OSC3){
        o=&p->osc[pg-UI_OSC1];
        if(k==0)vg1_adj_u8(&o->wave,n,0,VG1_N_WAVES-1);
        if(k==1)vg1_adj_u8(&o->level,n,0,127);
        if(k==2)vg1_adj_i8(&o->semi,n,-36,36);
        if(k==3)vg1_adj_u8(&o->pw,n,0,127);
    } else if(pg==UI_OSC_FAT) {
        if(k==0)vg1_adj_u8(&p->vg_osc.retrig,n,0,1);
        if(k==1)vg1_adj_u8(&p->vg_osc.fat,n,0,127);
        if(k==2)vg1_adj_u8(&p->vg_osc.vcs,n,2,10);
        if(k==3)vg1_adj_u8(&p->vg_osc.age,n,0,127);
    } else if(pg>=UI_LFO1&&pg<=UI_LFO3) {
        l=&p->vg_lfo[pg-UI_LFO1];
        if(k==0)vg1_adj_u8(&l->speed,n,0,60);
        if(k==1)vg1_adj_i8(&l->detune,n,-127,127);
        if(k==2)vg1_adj_i8(&l->pwm,n,-127,127);
        if(k==3)vg1_adj_i8(&l->cutoff,n,-127,127);
    } else if(pg==UI_FILTER) {
        if(k==0)vg1_adj_u8(&p->filter_type,n,0,VG1_N_FILTERS-1);
        if(k==1)vg1_adj_u8(&p->cutoff,n,0,127);
        if(k==2)vg1_adj_u8(&p->resonance,n,0,127);
        if(k==3)vg1_adj_i8(&p->vg_keytrack.keytrk,n,-127,127);
    } else if(pg==UI_FILTER_TRACK){
        if(k==0)vg1_adj_i8(&p->vg_keytrack.keytrk,n,-127,127);
        if(k==1)vg1_adj_i8(&p->vg_keytrack.veltrk,n,-127,127);
    } else if(pg==UI_ENV1||pg==UI_ENV2) {
        a=pg==UI_ENV1?&p->amp:&p->filter_env;
        uint8_t *v=k==0?&a->attack:k==1?&a->decay:k==2?&a->sustain:&a->release;
        vg1_adj_u8(v,n,0,127);
    } else if(pg==UI_ENV1_MOD) {
        int8_t *v=k==0?&p->vg_env1.level:k==1?&p->vg_env1.pwm:k==2?&p->vg_env1.cutoff:&p->vg_env1.detune;
        vg1_adj_i8(v,n,k==3?-100:-127,k==3?100:127);
    } else if(pg==UI_ENV2_MOD) {
        int8_t *v=k==0?&p->vg_env2.resonance:k==1?&p->vg_env2.pwm:k==2?&p->vg_env2.cutoff:&p->vg_env2.pitch;
        vg1_adj_i8(v,n,k==3?-48:-127,k==3?48:127);
    } else if(pg==UI_AMP){
        if(k==0)vg1_adj_u8(&p->vg_amp.drive,n,0,127);
        if(k==1)vg1_adj_u8(&p->vg_amp.level,n,0,127);
        if(k==2)vg1_adj_i8(&p->vg_amp.veltrk,n,-127,127);
        if(k==3)vg1_adj_u8(&p->vg_amp.pan,n,0,127);
    } else if(pg==UI_AMP_STEREO){
        if(k==0)vg1_adj_u8(&p->vg_amp.spread,n,0,127);
    } else if(pg==UI_GATE) {
        if(k==0)vg1_adj_u8(&p->vg_gate.enabled,n,0,1);
        if(k==1)vg1_adj_u8(&p->vg_gate.stereo,n,0,1);
        if(k==2)vg1_adj_u8(&p->vg_gate.mix,n,0,127);
        if(k==3)vg1_adj_u8(&p->vg_gate.contour,n,0,127);
    } else if(pg==UI_GATE_STEPS_L||pg==UI_GATE_STEPS_R) {
        mask=pg==UI_GATE_STEPS_L?&p->vg_gate.left:&p->vg_gate.right;
        if(k==0)vg1_gate_cursor=(uint32_t)v1_clip((int32_t)vg1_gate_cursor+n,0,15);
        if(k==1&&n)*mask^=(uint16_t)(1u<<vg1_gate_cursor);
        if(k==2&&n)*mask=(uint16_t)(n>0?(*mask<<1)|(*mask>>15):(*mask>>1)|(*mask<<15));
        if(k==3&&n)*mask=n>0?0xffffu:0;
    } else if(pg==UI_GATE_RATE) {
        if(k==0)vg1_adj_rate(&p->vg_gate.speed,0,n);
        if(k==1)vg1_adj_rate(&p->vg_gate.speed,1,n);
        if(k==2)vg1_adj_u8(&p->vg_gate.sync_to_song_position,n,0,1);
        if(k==3) {bpm=(uint32_t)v1_clip((int32_t)bpm+n,V1_BPM_MIN,V1_BPM_MAX);}
    } else if(pg==UI_DELAY){
        if(k==0)vg1_adj_u8(&p->vg_delay.on,n,0,1);
        if(k==1)vg1_adj_u8(&p->vg_delay.type,n,0,3);
        if(k==2)vg1_adj_u8(&p->vg_delay.mix,n,0,127);
        if(k==3)vg1_adj_rate(&p->vg_delay.speed,0,n);
    } else if(pg==UI_DELAY_MORE) {
        if(k==0)vg1_adj_u8(&p->vg_delay.feedback,n,0,120);
        if(k==1)vg1_adj_u8(&p->vg_delay.damp,n,0,127);
        if(k==2)vg1_adj_rate(&p->vg_delay.speed,1,n);
        if(k==3)vg1_adj_u8(&p->vg_delay.widen_ms,n,1,127);
    } else if(pg==UI_REVERB){
        if(k==0)vg1_adj_u8(&p->vg_reverb.on,n,0,1);
        if(k==1)vg1_adj_u8(&p->vg_reverb.mix,n,0,127);
        if(k==2)vg1_adj_u8(&p->vg_reverb.predelay,n,0,127);
        if(k==3)vg1_adj_u8(&p->vg_reverb.room_size,n,0,127);
    } else if(pg==UI_REVERB_MORE){
        if(k==0)vg1_adj_u8(&p->vg_reverb.damp,n,0,127);
        if(k==1)vg1_adj_u8(&p->vg_reverb.width,n,0,127);
    } else if(pg==UI_ARP) {
        if(k==0)vg1_adj_u8(&p->vg_arp.on,n,0,1);
        if(k==1)vg1_adj_u8(&p->vg_arp.mode,n,0,4);
        if(k==2)vg1_adj_rate(&p->vg_arp.speed,0,n);
        if(k==3)vg1_adj_u8(&p->vg_arp.gate,n,0,127);
    } else if(pg==UI_ARP_MORE) {
        if(k==0)vg1_adj_u8(&p->vg_arp.octaves,n,1,4);
        if(k==1)vg1_adj_u8(&p->vg_arp.retrig,n,0,1);
        if(k==2)vg1_adj_rate(&p->vg_arp.speed,1,n);
    } else if(pg==UI_GLIDE){
        if(k==0)vg1_adj_u8(&p->vg_glide.on,n,0,1);
        if(k==1) {
            /* Original Vanguard permits INSTANT + 1/1 through 1/64. */
            uint32_t d=p->vg_glide.speed.denominator;
            int32_t index=0;
            if(d) {index=1;while(d>1u&&index<7){d>>=1u;index++;}}
            index=v1_clip(index+n,0,7);
            p->vg_glide.speed.denominator=(uint8_t)(index?1u<<(index-1):0u);
        }
        if(k==2)vg1_adj_rate(&p->vg_glide.speed,1,n);
    } else if(pg==UI_PITCH) {
        /* Vanguard's pitchbend/aftertouch parameters are separate from LFO1 */
        if(k==0)vg1_adj_u8(&p->bend_range,n,0,24);
        if(k==1)vg1_adj_u8(&p->vib_speed,n,0,127);
        if(k==2)vg1_adj_u8(&p->vib_depth,n,0,127);
    } else if(pg==UI_VOICES){
        if(k==0) {
            uint32_t old=p->voice_limit?p->voice_limit:8u;
            p->voice_limit=(uint8_t)v1_clip((int32_t)old+n,1,V1_VOICES);
        }
        if(k==1)vg1_adj_u8(&p->master,n,0,127);
    } else if(pg==UI_PLAY) {
        if(k==0)vg1_adj_u8(&p->cutoff,n,0,127);
        if(k==1)vg1_adj_u8(&p->resonance,n,0,127);
        if(k==2)vg1_adj_u8(&p->vg_osc.fat,n,0,127);
        if(k==3)bpm=(uint32_t)v1_clip((int32_t)bpm+n,V1_BPM_MIN,V1_BPM_MAX);
    }
}
static uint32_t v1_param_value(uint32_t pg,uint32_t k) {
    const v1_patch_t *p=&v1.patch;
    if(pg>=UI_OSC1&&pg<=UI_OSC3) {
        const v1_osc_patch_t *o=&p->osc[pg-UI_OSC1];
        return k==0?o->wave:k==1?o->level:k==2?(uint32_t)((int32_t)o->semi+36):o->pw;
    }
    if(pg==UI_OSC_FAT)return k==0?p->vg_osc.retrig:k==1?p->vg_osc.fat:k==2?p->vg_osc.vcs:p->vg_osc.age;
    if(pg>=UI_LFO1&&pg<=UI_LFO3){
        const vg1_lfo_patch_t *l=&p->vg_lfo[pg-UI_LFO1];
        return k==0?l->speed:k==1?l->detune+127u:k==2?l->pwm+127u:l->cutoff+127u;
    }
    if(pg==UI_FILTER)return k==0?p->filter_type:k==1?p->cutoff:k==2?p->resonance:p->vg_keytrack.keytrk+127u;
    if(pg==UI_FILTER_TRACK)return k==0?p->vg_keytrack.keytrk+127u:k==1?p->vg_keytrack.veltrk+127u:0;
    if(pg==UI_ENV1||pg==UI_ENV2){
        const v1_adsr_patch_t *a=pg==UI_ENV1?&p->amp:&p->filter_env;
        return k==0?a->attack:k==1?a->decay:k==2?a->sustain:a->release;
    }
    if(pg==UI_ENV1_MOD)return (uint32_t)(k==0?p->vg_env1.level+127:k==1?p->vg_env1.pwm+127:k==2?p->vg_env1.cutoff+127:p->vg_env1.detune+100);
    if(pg==UI_ENV2_MOD)return (uint32_t)(k==0?p->vg_env2.resonance+127:k==1?p->vg_env2.pwm+127:k==2?p->vg_env2.cutoff+127:p->vg_env2.pitch+48);
    if(pg==UI_AMP)return k==0?p->vg_amp.drive:k==1?p->vg_amp.level:k==2?p->vg_amp.veltrk+127u:p->vg_amp.pan;
    if(pg==UI_AMP_STEREO)return k==0?p->vg_amp.spread:0;
    if(pg==UI_GATE)return k==0?p->vg_gate.enabled:k==1?p->vg_gate.stereo:k==2?p->vg_gate.mix:p->vg_gate.contour;
    if(pg==UI_GATE_STEPS_L||pg==UI_GATE_STEPS_R){
        uint16_t m=pg==UI_GATE_STEPS_L?p->vg_gate.left:p->vg_gate.right;
        return k==0?vg1_gate_cursor+1u:k==1?((m>>vg1_gate_cursor)&1u):0u;
    }
    if(pg==UI_GATE_RATE)return k==0?p->vg_gate.speed.denominator:k==1?p->vg_gate.speed.variant:k==2?p->vg_gate.sync_to_song_position:bpm;
    if(pg==UI_DELAY)return k==0?p->vg_delay.on:k==1?p->vg_delay.type:k==2?p->vg_delay.mix:p->vg_delay.speed.denominator;
    if(pg==UI_DELAY_MORE)return k==0?p->vg_delay.feedback:k==1?p->vg_delay.damp:k==2?p->vg_delay.speed.variant:p->vg_delay.widen_ms;
    if(pg==UI_REVERB)return k==0?p->vg_reverb.on:k==1?p->vg_reverb.mix:k==2?p->vg_reverb.predelay:p->vg_reverb.room_size;
    if(pg==UI_REVERB_MORE)return k==0?p->vg_reverb.damp:k==1?p->vg_reverb.width:0;
    if(pg==UI_ARP)return k==0?p->vg_arp.on:k==1?p->vg_arp.mode:k==2?p->vg_arp.speed.denominator:p->vg_arp.gate;
    if(pg==UI_ARP_MORE)return k==0?p->vg_arp.octaves:k==1?p->vg_arp.retrig:k==2?p->vg_arp.speed.variant:0;
    if(pg==UI_GLIDE)return k==0?p->vg_glide.on:k==1?p->vg_glide.speed.denominator:k==2?p->vg_glide.speed.variant:0;
    if(pg==UI_PITCH)return k==0?p->bend_range:k==1?p->vib_speed:k==2?p->vib_depth:0;
    if(pg==UI_VOICES)return k==0?(p->voice_limit?p->voice_limit:8u):k==1?p->master:0;
    return k==0?p->cutoff:k==1?p->resonance:k==2?p->vg_osc.fat:bpm;
}
static void v1_num(char *p,uint32_t v) {
    char t[12];uint32_t i=0,j=0;
    if(v==0){p[0]='0';p[1]=0;return;}
    while(v && i<11){t[i++]=(char)('0'+v%10u);v/=10u;}
    while(i)p[j++]=t[--i];
    p[j]=0;
}
static void v1_panel_draw(void) {
    uint32_t k,pg=page,voice=0;
    char tmp[12], bpmtext[12];
    const v1_patch_t *p=&v1.patch;
    uint16_t bg=RGB(8,10,22), rule=RGB(58,73,105), high=RGB(67,216,223), warm=RGB(244,193,105);
    cv_begin(240,112,bg);
    cv_text(9,4,&FONT_S,"VG-1  /  FM1",high);
    cv_text(9,24,&FONT_S,p->name,RGB(236,235,242));
    cv_rect(8,44,224,1,rule);
    cv_text(9,50,&FONT_S,UI_TITLES[pg],warm);
    for(k=0;k<V1_VOICES;k++)if(v1.voices[k].active)voice++;
    v1_num(tmp,voice);cv_text(205,51,&FONT_S,tmp,high);
    for(k=0;k<4;k++) {
        uint32_t x=7u+k*59u,val=v1_param_value(pg,k);
        const char *value=0;
        if((pg>=UI_OSC1 && pg<=UI_OSC3)&&k==0) {
            v1_num(tmp,val);value=tmp; /* full wave name shown above controls */
        }
        else if(pg==UI_FILTER&&k==0) {
            v1_num(tmp,val);value=tmp; /* mode number, full label in status area */
        }
        else if(pg==UI_GLIDE&&k==1&&!val)value="INSTANT";
        else if(pg==UI_ARP&&k==1)value=vg1_arp_name[val%VG1_ARP_N_MODES];
        else if(pg==UI_DELAY&&k==1)value=vg1_delay_name[val%VG1_N_DELAY_MODES];
        else if(pg==UI_GATE_STEPS_L||pg==UI_GATE_STEPS_R){v1_num(tmp,val);value=tmp;}
        else {v1_num(tmp,val);value=tmp;}
        cv_text((int32_t)x,73,&FONT_S,UI_LABELS[pg][k],RGB(125,145,169));
        cv_text((int32_t)x,91,&FONT_S,value,RGB(235,235,239));
    }
    cv_blit(0,0);
    cv_begin(240,112,bg);
    cv_rect(8,3,224,1,rule);
    if(pg>=UI_OSC1&&pg<=UI_OSC3)
        cv_text(8,13,&FONT_S,vg1_wave_name[p->osc[pg-UI_OSC1].wave%VG1_N_WAVES],high);
    else if(pg==UI_FILTER)
        cv_text(8,13,&FONT_S,vg1_filter_name[p->filter_type%VG1_N_FILTERS],high);
    else
        cv_text(10,13,&FONT_S,"VANGUARD 1 ARCHITECTURE",high);
    cv_text(10,37,&FONT_S,v1_arp_mode(p)?"ARP ACTIVE":"ARP OFF",RGB(161,180,205));
    /* Held keys distinguishes a missed key-up (HELD stays nonzero) from
       the intended ADSR/FX release tail (HELD=0, VOICES fades to 0). */
    cv_text(143,37,&FONT_S,"HELD",warm);
    v1_num(tmp,v1.held_count);cv_text(207,37,&FONT_S,tmp,high);
    cv_text(10,61,&FONT_S,p->vg_gate.enabled?"TRANCEGATE ON":"TRANCEGATE OFF",RGB(161,180,205));
    cv_text(160,61,&FONT_S,"FAT",warm);
    v1_num(tmp,v1.last_render_vcs);
    cv_text(209,61,&FONT_S,tmp,high);
    cv_text(10,85,&FONT_S,"BPM",warm);
    v1_num(bpmtext,bpm);cv_text(52,85,&FONT_S,bpmtext,RGB(230,235,246));
    cv_text(133,85,&FONT_S,"DSP",high);
    v1_num(tmp,v1_dsp_pct);
    cv_text(178,85,&FONT_S,tmp,v1_dsp_pct>80?RGB(248,85,93):high);
    cv_blit(0,112);
}
static void v1_midi(void) {
    uint8_t st,d1,d2;
    while(midi_get(&st,&d1,&d2)) {
        uint32_t kind=st&0xf0u;
        if(kind==0x90u && d2)v1_queue(1,d1,d2);
        else if(kind==0x80u || (kind==0x90u&&!d2))v1_queue(0,d1,0);
        else if(kind==0xc0u)req_program=d1%V1_PRESETS;
        else if(kind==0xe0u)v1.bend14=(int32_t)(((uint32_t)d2<<7)|d1)-8192;
        else if(kind==0xd0u)v1.aftertouch=d1;
        else if(kind==0xb0u){
            /* CC assignments taken from Vanguard 1 manual appendix B.
               Volume and expression are independent of patch AMP LEVEL. */
            switch(d1) {
                case 1:  v1.modwheel=d2; break;                    /* filter cutoff addition */
                case 7:  v1.midi_volume=d2; break;
                case 8:  v1.patch.vg_amp.spread=d2; break;
                case 10: v1.patch.vg_amp.pan=d2; break;
                case 11: v1.midi_expression=d2; break;
                case 21: v1.patch.vg_gate.contour=d2; break;
                case 22: v1.patch.vib_speed=d2; break;
                case 23: v1.patch.vib_depth=d2; break;
                case 71: v1.patch.resonance=d2; break;
                case 74: v1.patch.cutoff=d2; break;
                case 120: case 123: vg1_panic_requested=1; break;
                default: break;
            }
        }
    }
}
static void app_main(void) {
    uint32_t k,rel,buttons,now;
    int32_t e;
    v1_engine_init(&v1,&V1_FACTORY[0]);
    req_program=got_program=page=0;
    bpm=128;
    octave=2u; /* software octave centre = 2; transpose range -24..+24 */
    v1_panel_draw();panel_last=fm1_ms;
    for(;;) {
        platform_poll();
        v1_midi();
        /* Octave changes apply ONLY to new key-down events. Note-offs must
           match their stored original pitches, including the USB-MIDI out. */
        vg1_key_event_t note_events[VG1_KEY_COUNT];
        uint32_t count=vg1_route_keys(&v1_keyboard,fm1_in.notes,octave,note_events);
        for(k=0;k<count;k++) {
            uint32_t note=note_events[k].note;
            if(note_events[k].on) {
                v1_queue(1,note,102);midi_send(0x90u,note,102);
            } else {
                v1_queue(0,note,0);midi_send(0x80u,note,0);
            }
        }
        buttons=fm1_input_edges(&rel);
        if(buttons&(1u<<BTN_OCT_DOWN)){if(octave)octave--;}
        if(buttons&(1u<<BTN_OCT_UP)){if(octave<4u)octave++;}
        if(buttons&(1u<<BTN_HOME))page=UI_PLAY;
        if(buttons&(1u<<BTN_EDIT))page=page>=UI_OSC1&&page<UI_OSC_FAT?page+1:UI_OSC1;
        if(buttons&(1u<<BTN_ENV))page=page>=UI_ENV1&&page<=UI_ENV2_MOD?UI_ENV1+(page-UI_ENV1+1u)%4u:UI_ENV1;
        if(buttons&(1u<<BTN_LFO))page=page>=UI_LFO1&&page<=UI_LFO3?UI_LFO1+(page-UI_LFO1+1u)%3u:UI_LFO1;
        if(buttons&(1u<<BTN_FX))page=page>=UI_GATE&&page<=UI_REVERB_MORE?UI_GATE+(page-UI_GATE+1u)%8u:UI_GATE;
        if(buttons&(1u<<BTN_SEQ))page=page==UI_GATE_STEPS_L?UI_GATE_STEPS_R:UI_GATE_STEPS_L;
        if(buttons&(1u<<BTN_ARP))page=page==UI_ARP?UI_ARP_MORE:UI_ARP;
        if(buttons&(1u<<BTN_GLO))page=UI_VOICES;
        e=fm1_enc_take(ENC_SELECT);
        if(e)page=v1_bound((int32_t)page+(e>0?1:-1),UI_PAGES-1);
        e=fm1_enc_take(ENC_PRESETS);
        if(e)req_program=v1_bound((int32_t)req_program+(e>0?1:-1),V1_PRESETS-1);
        e=fm1_enc_take(ENC_ALGORITHM);
        if(e){bpm=v1_bound((int32_t)bpm+e,V1_BPM_MAX);if(bpm<V1_BPM_MIN)bpm=V1_BPM_MIN;}
        for(k=0;k<4;k++){
            e=fm1_enc_take(ENC_KNOB1+k);
            if(e)v1_param_change(page,k,e);
        }
        now=fm1_ms;
        if((uint32_t)(now-panel_last)>=160u){
            panel_last=now;
            v1_panel_draw();
        }
    }
}
