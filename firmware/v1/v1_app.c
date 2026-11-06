/* SPDX-License-Identifier: GPL-3.0-only
 * V-1 FM1 application. Independent virtual-analog monosynth/polysynth UI.
 * The platform provides ONLY board startup, audio, physical I/O and firmware recovery.
 */
#define APP_NAME "V-1 FM1"
static void app_idle(void);
#define LCD_IDLE() app_idle()
#include "../src/platform.c"
#include "v1_core.h"

#define V1_KEY0 53u                /* FM1 physical key 0: F3 */
#define V1_BPM_MIN 40u
#define V1_BPM_MAX 240u
#define V1_PRESETS 8u
#define O(w,l,p,d,s,f) {w,l,p,d,s,f}
#define ADS(a,d,s,r) {a,d,s,r}
static const v1_patch_t V1_FACTORY[V1_PRESETS] = {
    { .name="HEAVEN LEAD",
      .osc={O(V1_SUPERSAW,123,64,80,0,0),O(V1_SAW,88,64,0,0,7),O(V1_SAW,75,64,0,-12,-3)},
      .amp=ADS(1,25,116,40),.filter_env=ADS(1,35,85,35),
      .cutoff=99,.resonance=18,.filter_type=V1_LP12,.filter_env_depth=10,
      .lfo_rate={4,5,3},.lfo_pitch=0,.lfo_filter=0,.drive=12,
      .chorus=68,.delay=43,.delay_feedback=45,.reverb=22,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=95,.arp_octaves=1,
      .trance_mask=0xffff,.trance_depth=127,.master=91
    },
    { .name="FLOOR SHAKER",
      .osc={O(V1_SAW,125,64,0,0,0),O(V1_PULSE,90,58,0,0,0),O(V1_SINE,106,64,0,-12,0)},
      .amp=ADS(0,18,91,15),.filter_env=ADS(0,18,0,15),
      .cutoff=42,.resonance=40,.filter_type=V1_LP24,.filter_env_depth=49,
      .lfo_rate={0,0,0},.drive=56,
      .chorus=0,.delay=0,.delay_feedback=35,.reverb=0,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=96,.arp_octaves=1,
      .trance_mask=0xffff,.trance_depth=127,.master=93
    },
    { .name="CRYSTAL PLUCK",
      .osc={O(V1_PULSE,112,52,0,0,0),O(V1_SAW,75,64,0,0,7),O(V1_SINE,85,64,0,12,0)},
      .amp=ADS(0,29,0,25),.filter_env=ADS(0,27,0,20),
      .cutoff=57,.resonance=35,.filter_type=V1_LP12,.filter_env_depth=58,
      .lfo_rate={8,4,5},.drive=4,
      .chorus=13,.delay=78,.delay_feedback=46,.reverb=35,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=69,.arp_octaves=1,
      .trance_mask=0xffff,.trance_depth=127,.master=97
    },
    { .name="GATED MOTION",
      .osc={O(V1_SUPERSAW,108,64,65,0,0),O(V1_PULSE,99,74,0,0,4),O(V1_SAW,65,64,0,-12,0)},
      .amp=ADS(28,65,110,70),.filter_env=ADS(22,56,70,68),
      .cutoff=68,.resonance=23,.filter_type=V1_LP12,.filter_env_depth=20,
      .lfo_rate={12,6,2},.lfo_filter=25,.drive=4,
      .chorus=52,.delay=42,.delay_feedback=43,.reverb=52,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=80,.arp_octaves=1,
      .trance_mask=0xbaba,.trance_enabled=1,.trance_rate=1,.trance_depth=127,.master=91
    },
    { .name="ACID RUNNER",
      .osc={O(V1_SAW,125,64,0,0,0),O(V1_PULSE,72,26,0,0,0),O(V1_OFF,0,64,0,0,0)},
      .amp=ADS(0,20,75,11),.filter_env=ADS(0,19,0,13),
      .cutoff=47,.resonance=96,.filter_type=V1_LP24,.filter_env_depth=45,
      .lfo_rate={15,0,0},.drive=48,
      .chorus=0,.delay=16,.delay_feedback=27,.reverb=6,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=95,.arp_octaves=1,
      .trance_mask=0xffff,.trance_depth=127,.master=96
    },
    { .name="SATURN PAD",
      .osc={O(V1_SUPERSAW,97,64,49,0,0),O(V1_TRI,74,64,0,0,0),O(V1_SINE,47,64,0,-12,0)},
      .amp=ADS(73,89,102,100),.filter_env=ADS(85,90,78,80),
      .cutoff=54,.resonance=13,.filter_type=V1_LP12,.filter_env_depth=22,
      .lfo_rate={2,2,3},.lfo_filter=36,.lfo_amp=10,
      .drive=0,.chorus=100,.delay=12,.delay_feedback=26,.reverb=85,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=115,.arp_octaves=1,
      .trance_mask=0xffff,.trance_depth=127,.master=99
    },
    { .name="NIGHT ARPEG",
      .osc={O(V1_SAW,117,64,0,0,0),O(V1_SINE,64,64,0,12,0),O(V1_PULSE,82,48,0,0,7)},
      .amp=ADS(0,25,0,25),.filter_env=ADS(0,36,0,30),
      .cutoff=67,.resonance=32,.filter_type=V1_LP12,.filter_env_depth=43,
      .lfo_rate={2,11,4},.lfo_filter=10,
      .drive=11,.chorus=27,.delay=66,.delay_feedback=53,.reverb=34,
      .arp_mode=V1_ARP_UP,.arp_rate=1,.arp_gate=67,.arp_octaves=2,
      .trance_mask=0xffff,.trance_depth=127,.master=104
    },
    { .name="LASER SHOT",
      .osc={O(V1_PULSE,120,82,0,0,0),O(V1_SAW,90,64,0,12,0),O(V1_NOISE,35,64,0,0,0)},
      .amp=ADS(0,21,0,21),.filter_env=ADS(0,18,0,16),
      .cutoff=96,.resonance=49,.filter_type=V1_HP12,.filter_env_depth=-44,
      .lfo_rate={45,20,0},.lfo_amp=10,
      .drive=75,.chorus=12,.delay=36,.delay_feedback=34,.reverb=40,
      .arp_mode=V1_ARP_OFF,.arp_rate=1,.arp_gate=80,.arp_octaves=1,
      .trance_mask=0xffff,.trance_depth=127,.master=88
    },
};
#undef O
#undef ADS

static v1_engine_t v1 __attribute__((section(".pool")));
static volatile uint32_t req_program;
static uint32_t got_program, page, octave, bpm=128, panel_last, keys_last;
static uint32_t page_changed;
enum { UI_PLAY,UI_OSC1,UI_OSC2,UI_OSC3,UI_FILTER,UI_AMP,UI_FENV,UI_MOD,
       UI_FX,UI_GATE,UI_GATE_STEPS,UI_ARP,UI_MASTER,UI_PAGES };
static const char *const UI_TITLES[UI_PAGES] = {
    "PERFORM","OSCILLATOR 1","OSCILLATOR 2","OSCILLATOR 3","FILTER",
    "AMP ENVELOPE","FILTER ENVELOPE","MODULATION","EFFECTS",
    "TRANCEGATE","16-STEP GATE","ARPEGGIATOR","MASTER"
};
static const char *const UI_LABELS[UI_PAGES][4] = {
    {"CUTOFF","RESO","CHORUS","GATE"},
    {"WAVE","LEVEL","DETUNE","WIDTH"},
    {"WAVE","LEVEL","SEMI","WIDTH"},
    {"WAVE","LEVEL","SEMI","FINE"},
    {"CUTOFF","RESO","TYPE","ENV AMT"},
    {"ATTACK","DECAY","SUSTAIN","RELEASE"},
    {"ATTACK","DECAY","SUSTAIN","RELEASE"},
    {"LFO RATE","PITCH","FILTER","AMP"},
    {"CHORUS","DELAY","REVERB","DRIVE"},
    {"ON/OFF","RATE","DEPTH","BPM"},
    {"STEPS 1-4","STEPS 5-8","STEPS 9-12","STEPS 13-16"},
    {"MODE","RATE","GATE","OCTAVES"},
    {"LEVEL","FEEDBACK","LFO 2","LFO 3"}
};
static const char *const WAVE_NAMES[]={"OFF","SAW","PULSE","TRI","SINE","SUPERSAW","NOISE"};
static const char *const FILTER_NAMES[]={"LP12","HP12","BP12","NOTCH","LP24","HP24","BP24"};
static const char *const ARP_NAMES[]={"OFF","UP","DOWN","UP-DOWN","RANDOM","PLAYED"};
static const char *const RATE_NAMES[]={"1/8","1/16","1/32"};
static const char *const STEP_HEX[]={"0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};

typedef struct { uint8_t on,note,vel; } v1_note_msg_t;
#define V1_Q 64u
static v1_note_msg_t msgs[V1_Q];
static volatile uint32_t qw,qr;
static void v1_queue(uint32_t on,uint32_t note,uint32_t vel) {
    uint32_t i=qw;
    if(i-qr>=V1_Q)return;
    msgs[i&(V1_Q-1u)].on=(uint8_t)on;
    msgs[i&(V1_Q-1u)].note=(uint8_t)note;
    msgs[i&(V1_Q-1u)].vel=(uint8_t)vel;
    __asm__ volatile("" ::: "memory");
    qw=i+1;
}
static void app_audio(int32_t *out,uint32_t n) {
    if(req_program!=got_program) {
        got_program=req_program%V1_PRESETS;
        v1_panic(&v1);
        v1.patch=V1_FACTORY[got_program];
        v1.gate_phase=v1.gate_step=v1.arp_phase=v1.arp_pos=0;
        v1.gate_smooth=32767;
    }
    while(qr!=qw) {
        v1_note_msg_t m=msgs[qr&(V1_Q-1u)];
        qr++;
        v1_hold(&v1,m.on,m.note,m.vel);
    }
    v1_engine_render(&v1,out,n,bpm);
}
static void app_idle(void) { platform_poll(); }
static uint32_t v1_bound(int32_t v,int32_t max) {
    if(v<0)return 0;
    if(v>max)return (uint32_t)max;
    return (uint32_t)v;
}
static void v1_param_change(uint32_t pg,uint32_t k,int32_t n) {
    v1_patch_t *p=&v1.patch;
    v1_osc_patch_t *o;
    v1_adsr_patch_t *a;
    uint8_t *field=0;
    int32_t max=127,now=0,next;
    if(pg==UI_OSC1||pg==UI_OSC2||pg==UI_OSC3) {
        o=&p->osc[pg-UI_OSC1];
        if(k==0){field=&o->wave;max=V1_NOISE;}
        else if(k==1)field=&o->level;
        else if(k==2 && pg==UI_OSC1)field=&o->detune;
        else if(k==2){int32_t s=(int32_t)o->semi+n; o->semi=(int8_t)v1_clip(s,-24,24);return;}
        else if(k==3 && pg==UI_OSC3){o->fine=(int8_t)v1_clip((int32_t)o->fine+n,-50,50);return;}
        else field=&o->pw;
    } else if(pg==UI_AMP||pg==UI_FENV) {
        a=pg==UI_AMP?&p->amp:&p->filter_env;
        field=k==0?&a->attack:k==1?&a->decay:k==2?&a->sustain:&a->release;
    } else if(pg==UI_FILTER) {
        if(k==0)field=&p->cutoff;
        if(k==1)field=&p->resonance;
        if(k==2){field=&p->filter_type;max=V1_BP24;}
        if(k==3){p->filter_env_depth=(int8_t)v1_clip((int32_t)p->filter_env_depth+n,-64,64);return;}
    } else if(pg==UI_MOD) {
        field=k==0?&p->lfo_rate[0]:k==1?&p->lfo_pitch:k==2?&p->lfo_filter:&p->lfo_amp;
    } else if(pg==UI_FX) {
        field=k==0?&p->chorus:k==1?&p->delay:k==2?&p->reverb:&p->drive;
    } else if(pg==UI_GATE) {
        if(k==0){field=&p->trance_enabled;max=1;}
        if(k==1){field=&p->trance_rate;max=2;}
        if(k==2)field=&p->trance_depth;
        if(k==3){bpm=v1_bound((int32_t)bpm+n,V1_BPM_MAX);if(bpm<V1_BPM_MIN)bpm=V1_BPM_MIN;return;}
    } else if(pg==UI_GATE_STEPS) {
        uint32_t shift=k*4u;
        int32_t x=(p->trance_mask>>shift)&15u;
        x=v1_clip(x+n,0,15);
        p->trance_mask=(uint16_t)((p->trance_mask&~(15u<<shift))|((uint32_t)x<<shift));
        return;
    } else if(pg==UI_ARP) {
        if(k==0){field=&p->arp_mode;max=V1_ARP_PLAYED;}
        if(k==1){field=&p->arp_rate;max=2;}
        if(k==2)field=&p->arp_gate;
        if(k==3){field=&p->arp_octaves;max=4;}
    } else if(pg==UI_MASTER) {
        field=k==0?&p->master:k==1?&p->delay_feedback:k==2?&p->lfo_rate[1]:&p->lfo_rate[2];
    } else {
        field=k==0?&p->cutoff:k==1?&p->resonance:k==2?&p->chorus:&p->trance_depth;
    }
    if(field){now=*field;next=(int32_t)v1_bound(now+n,max);*field=(uint8_t)next;}
}
static uint32_t v1_param_value(uint32_t pg,uint32_t k) {
    const v1_patch_t *p=&v1.patch;
    const v1_osc_patch_t *o;
    const v1_adsr_patch_t *a;
    if(pg>=UI_OSC1&&pg<=UI_OSC3){
        o=&p->osc[pg-UI_OSC1];
        return k==0?o->wave:k==1?o->level:k==2?(pg==UI_OSC1?o->detune:(uint32_t)((int32_t)o->semi+24)):pg==UI_OSC3?(uint32_t)((int32_t)o->fine+50):o->pw;
    }
    if(pg==UI_AMP||pg==UI_FENV){a=pg==UI_AMP?&p->amp:&p->filter_env;return k==0?a->attack:k==1?a->decay:k==2?a->sustain:a->release;}
    if(pg==UI_FILTER)return k==0?p->cutoff:k==1?p->resonance:k==2?p->filter_type:(uint32_t)((int32_t)p->filter_env_depth+64);
    if(pg==UI_MOD)return k==0?p->lfo_rate[0]:k==1?p->lfo_pitch:k==2?p->lfo_filter:p->lfo_amp;
    if(pg==UI_FX)return k==0?p->chorus:k==1?p->delay:k==2?p->reverb:p->drive;
    if(pg==UI_GATE)return k==0?p->trance_enabled:k==1?p->trance_rate:k==2?p->trance_depth:bpm;
    if(pg==UI_GATE_STEPS)return (p->trance_mask>>(k*4u))&15u;
    if(pg==UI_ARP)return k==0?p->arp_mode:k==1?p->arp_rate:k==2?p->arp_gate:p->arp_octaves;
    if(pg==UI_MASTER)return k==0?p->master:k==1?p->delay_feedback:k==2?p->lfo_rate[1]:p->lfo_rate[2];
    return k==0?p->cutoff:k==1?p->resonance:k==2?p->chorus:p->trance_depth;
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
    cv_text(9,4,&FONT_S,"V-1  /  FM1",high);
    cv_text(9,24,&FONT_S,p->name,RGB(236,235,242));
    cv_rect(8,44,224,1,rule);
    cv_text(9,50,&FONT_S,UI_TITLES[pg],warm);
    for(k=0;k<V1_VOICES;k++)if(v1.voices[k].active)voice++;
    v1_num(tmp,voice);cv_text(205,51,&FONT_S,tmp,high);
    for(k=0;k<4;k++) {
        uint32_t x=7u+k*59u,val=v1_param_value(pg,k);
        const char *value=0;
        if((pg>=UI_OSC1 && pg<=UI_OSC3)&&k==0)value=WAVE_NAMES[val%(V1_NOISE+1u)];
        else if(pg==UI_FILTER&&k==2)value=FILTER_NAMES[val%(V1_BP24+1u)];
        else if(pg==UI_ARP&&k==0)value=ARP_NAMES[val%(V1_ARP_PLAYED+1u)];
        else if((pg==UI_GATE&&k==1)||(pg==UI_ARP&&k==1))value=RATE_NAMES[val%3u];
        else if(pg==UI_GATE_STEPS)value=STEP_HEX[val&15u];
        else {v1_num(tmp,val);value=tmp;}
        cv_text((int32_t)x,73,&FONT_S,UI_LABELS[pg][k],RGB(125,145,169));
        cv_text((int32_t)x,91,&FONT_S,value,RGB(235,235,239));
    }
    cv_blit(0,0);
    cv_begin(240,112,bg);
    cv_rect(8,3,224,1,rule);
    cv_text(10,13,&FONT_S,"ORIGINAL 3 OSC ENGINE",high);
    cv_text(10,37,&FONT_S,p->arp_mode?"ARP ACTIVE":"ARP OFF",RGB(161,180,205));
    cv_text(10,61,&FONT_S,p->trance_enabled?"TRANCEGATE ON":"TRANCEGATE OFF",RGB(161,180,205));
    cv_text(10,85,&FONT_S,"BPM",warm);
    v1_num(bpmtext,bpm);cv_text(52,85,&FONT_S,bpmtext,RGB(230,235,246));
    cv_text(138,85,&FONT_S,"V-1  0.1",high);
    cv_blit(0,112);
}
static void v1_midi(void) {
    uint8_t st,d1,d2;
    while(midi_get(&st,&d1,&d2)) {
        uint32_t kind=st&0xf0u;
        if(kind==0x90u && d2)v1_queue(1,d1,d2);
        else if(kind==0x80u || (kind==0x90u&&!d2))v1_queue(0,d1,0);
        else if(kind==0xc0u)req_program=d1%V1_PRESETS;
        else if(kind==0xb0u && (d1==120u||d1==123u)) {
            uint32_t k;for(k=0;k<128;k++)v1_queue(0,k,0);
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
        uint32_t down=fm1_in.notes & ~keys_last;
        uint32_t up=keys_last & ~fm1_in.notes;
        for(k=0;k<27;k++) {
            uint32_t note=v1_bound((int32_t)V1_KEY0+(int32_t)k+((int32_t)octave-2)*12,127);
            if(down&(1u<<k)){v1_queue(1,note,102);midi_send(0x90u,note,102);}
            if(up&(1u<<k)){v1_queue(0,note,0);midi_send(0x80u,note,0);}
        }
        keys_last=fm1_in.notes;
        buttons=fm1_input_edges(&rel);
        if(buttons&(1u<<BTN_OCT_DOWN)){if(octave)octave--;}
        if(buttons&(1u<<BTN_OCT_UP)){if(octave<4u)octave++;}
        if(buttons&(1u<<BTN_HOME))page=UI_PLAY;
        if(buttons&(1u<<BTN_EDIT))page=page>=UI_OSC1&&page<UI_FILTER?page+1:page==UI_FILTER?UI_OSC1:UI_OSC1;
        if(buttons&(1u<<BTN_ENV))page=page==UI_AMP?UI_FENV:UI_AMP;
        if(buttons&(1u<<BTN_LFO))page=UI_MOD;
        if(buttons&(1u<<BTN_FX))page=UI_FX;
        if(buttons&(1u<<BTN_SEQ))page=page==UI_GATE?UI_GATE_STEPS:UI_GATE;
        if(buttons&(1u<<BTN_ARP))page=UI_ARP;
        if(buttons&(1u<<BTN_GLO))page=UI_MASTER;
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
