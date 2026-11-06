/* SPDX-License-Identifier: GPL-3.0-only
 * V-1 FM1: original trance-oriented synthesizer DSP.
 * Entirely new instrument and voice model; ONLY the FM1 HAL is reused.
 * No reFX binary, presets, samples, artwork or proprietary DSP.
 * The fixed-point audio path deliberately makes no libm calls.
 */
#ifndef V1_CORE_H
#define V1_CORE_H
#include <stdint.h>
#include "vg1_reference.h"
#define V1_SR 44100u
#define V1_VOICES 16u /* capacity, not hard-coded active polyphony */
#define V1_OSCS 3u
#define V1_SUPER 10u /* Vanguard FAT/VCS accepts 2..10 layered oscillators */
#define V1_OFF VG1_W_OFF
#define V1_SAW VG1_W_SAWTOOTH
#define V1_PULSE VG1_W_PULSE_PWM
#define V1_TRI VG1_W_TRI_SAW_MORPH
#define V1_SINE VG1_W_SINE
#define V1_SUPERSAW VG1_W_AJX_SAW3 /* transitional name for old prototype tests */
#define V1_NOISE VG1_W_WHITE_NOISE
#define V1_LP12 VG1_F_LOW12
#define V1_HP12 VG1_F_HIGH6
#define V1_BP12 VG1_F_X
#define V1_NOTCH VG1_F_NOTCH
#define V1_LP24 VG1_F_LOW24
#define V1_HP24 VG1_F_HIGH24
#define V1_BP24 VG1_F_BAND24
enum { V1_ARP_OFF, V1_ARP_UP, V1_ARP_DOWN, V1_ARP_UPDOWN, V1_ARP_RANDOM, V1_ARP_PLAYED };

typedef struct { uint8_t wave, level, pw, detune; int8_t semi, fine; } v1_osc_patch_t;
typedef struct { uint8_t attack, decay, sustain, release; } v1_adsr_patch_t;
typedef struct {
    char name[16];
    v1_osc_patch_t osc[3];
    v1_adsr_patch_t amp, filter_env;
    uint8_t cutoff, resonance, filter_type;
    int8_t filter_env_depth;
    uint8_t lfo_rate[3], lfo_pitch, lfo_filter, lfo_amp;
    uint8_t drive, chorus, delay, delay_feedback, reverb;
    uint8_t arp_mode, arp_rate, arp_gate, arp_octaves;
    uint16_t trance_mask; /* 16 genuinely editable steps; bit 0 is beat 1 */
    uint8_t trance_enabled, trance_rate, trance_depth;
    uint8_t master;
    uint8_t voice_limit; /* zero denotes safe 8-voice initial setting */
    /* Original reFX Vanguard 1.6 parameter architecture, distinct from
       prototype controls above. New UI and DSP use these VG1 modules. */
    vg1_osc_global_t vg_osc;
    vg1_lfo_patch_t vg_lfo[3];
    vg1_env1_mod_t vg_env1;
    vg1_env2_mod_t vg_env2;
    vg1_filter_tracking_t vg_keytrack;
    vg1_amp_patch_t vg_amp;
    vg1_trancegate_patch_t vg_gate;
    vg1_delay_patch_t vg_delay;
    vg1_reverb_patch_t vg_reverb;
    vg1_arpeggiator_patch_t vg_arp;
    vg1_glide_patch_t vg_glide;
    uint8_t bend_range,vib_speed,vib_depth;
} v1_patch_t;

typedef struct { uint32_t ph[V1_SUPER], inc[V1_SUPER], base_inc[V1_SUPER]; uint32_t rng; int32_t noise_prev; } v1_osc_state_t;
typedef struct { int32_t ic1, ic2; } v1_svf_state_t;
typedef struct { uint8_t stage; int32_t level, release_step; } v1_env_state_t;
typedef struct {
    v1_osc_state_t osc[3];
    v1_svf_state_t f1, f2, f1_right, f2_right;
    v1_env_state_t a, f;
    uint32_t age;
    int32_t drift;
    int32_t fa1,fa2,fa3,fk,fb1,fb2,fb3,bk,pole6,pole6_right; /* control-rate coefficients and 6dB pole */
    uint8_t note, velocity, gate, active;
} v1_voice_t;

typedef struct {
    v1_patch_t patch;
    v1_voice_t voices[V1_VOICES];
    uint32_t note_age, rng;
    uint32_t lfo_ph[3], frames, vibrato_ph, auto_pan_ph;
    int32_t lfo[3];
    volatile int32_t bend14;
    volatile uint8_t aftertouch;
    uint16_t gate_step;
    uint32_t gate_phase;
    int32_t gate_smooth,gate_smooth_right;
    uint8_t held[16], held_velocity[16], held_count, arp_pos;
    uint8_t arp_note, arp_active, arp_direction;
    uint32_t arp_phase;
    int16_t delay_mem[2][32768];
    uint32_t delay_pos;
    int32_t delay_lp[2],reverb_lp[4];
    int16_t reverb_mem[4][2048];
    uint32_t reverb_pos;
    uint32_t render_peak;
} v1_engine_t;

static const uint32_t v1_pitch_inc[128] = {
    796254u,    843601u,    893765u,    946911u,    1003217u,    1062871u,    1126073u,    1193033u,
    1263974u,    1339134u,    1418763u,    1503127u,    1592507u,    1687203u,    1787529u,    1893821u,
    2006434u,    2125742u,    2252146u,    2386065u,    2527948u,    2678268u,    2837526u,    3006254u,
    3185015u,    3374406u,    3575058u,    3787642u,    4012867u,    4251485u,    4504291u,    4772130u,
    5055896u,    5356535u,    5675051u,    6012507u,    6370030u,    6748811u,    7150117u,    7575285u,
    8025735u,    8502970u,    9008582u,    9544261u,    10111792u,    10713070u,    11350103u,    12025015u,
    12740059u,    13497623u,    14300233u,    15150569u,    16051469u,    17005939u,    18017165u,    19088521u,
    20223584u,    21426141u,    22700205u,    24050030u,    25480119u,    26995246u,    28600467u,    30301139u,
    32102938u,    34011878u,    36034330u,    38177043u,    40447168u,    42852281u,    45400411u,    48100060u,
    50960238u,    53990491u,    57200933u,    60602278u,    64205876u,    68023757u,    72068660u,    76354085u,
    80894335u,    85704563u,    90800821u,    96200119u,    101920476u,    107980983u,    114401866u,    121204555u,
    128411753u,    136047513u,    144137319u,    152708170u,    161788671u,    171409126u,    181601643u,    192400238u,
    203840952u,    215961966u,    228803732u,    242409110u,    256823506u,    272095026u,    288274639u,    305416341u,
    323577341u,    342818251u,    363203285u,    384800477u,    407681904u,    431923931u,    457607465u,    484818220u,
    513647012u,    544190053u,    576549277u,    610832681u,    647154683u,    685636503u,    726406571u,    769600953u,
    815363807u,    863847862u,    915214929u,    969636441u,    1027294024u,    1088380105u,    1153098554u,    1221665363u

};
static const int16_t v1_sin_q15[257] = {
    0, 804, 1608, 2410, 3212, 4011, 4808, 5602, 6393, 7179, 7962, 8739, 
    9512, 10278, 11039, 11793, 12539, 13279, 14010, 14732, 15446, 16151, 16846, 17530, 
    18204, 18868, 19519, 20159, 20787, 21403, 22005, 22594, 23170, 23731, 24279, 24811, 
    25329, 25832, 26319, 26790, 27245, 27683, 28105, 28510, 28898, 29268, 29621, 29956, 
    30273, 30571, 30852, 31113, 31356, 31580, 31785, 31971, 32137, 32285, 32412, 32521, 
    32609, 32678, 32728, 32757, 32767, 32757, 32728, 32678, 32609, 32521, 32412, 32285, 
    32137, 31971, 31785, 31580, 31356, 31113, 30852, 30571, 30273, 29956, 29621, 29268, 
    28898, 28510, 28105, 27683, 27245, 26790, 26319, 25832, 25329, 24811, 24279, 23731, 
    23170, 22594, 22005, 21403, 20787, 20159, 19519, 18868, 18204, 17530, 16846, 16151, 
    15446, 14732, 14010, 13279, 12539, 11793, 11039, 10278, 9512, 8739, 7962, 7179, 
    6393, 5602, 4808, 4011, 3212, 2410, 1608, 804, 0, -804, -1608, -2410, 
    -3212, -4011, -4808, -5602, -6393, -7179, -7962, -8739, -9512, -10278, -11039, -11793, 
    -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530, -18204, -18868, -19519, -20159, 
    -20787, -21403, -22005, -22594, -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790, 
    -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956, -30273, -30571, -30852, -31113, 
    -31356, -31580, -31785, -31971, -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757, 
    -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285, -32137, -31971, -31785, -31580, 
    -31356, -31113, -30852, -30571, -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683, 
    -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731, -23170, -22594, -22005, -21403, 
    -20787, -20159, -19519, -18868, -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279, 
    -12539, -11793, -11039, -10278, -9512, -8739, -7962, -7179, -6393, -5602, -4808, -4011, 
    -3212, -2410, -1608, -804, 0
};
static const int16_t v1_cut_g[128] = {
    10, 11, 11, 12, 12, 13, 14, 14, 15, 16, 16, 17, 
    18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 30, 
    31, 33, 34, 36, 38, 39, 41, 43, 45, 48, 50, 52, 
    55, 57, 60, 63, 66, 69, 72, 76, 79, 83, 87, 91, 
    96, 100, 105, 110, 115, 121, 126, 133, 139, 145, 152, 160, 
    167, 175, 184, 192, 202, 211, 221, 232, 243, 255, 267, 280, 
    293, 307, 322, 337, 353, 370, 388, 407, 426, 447, 468, 491, 
    514, 539, 565, 592, 621, 651, 683, 716, 751, 788, 826, 867, 
    909, 954, 1001, 1051, 1104, 1159, 1217, 1279, 1344, 1413, 1486, 1564, 
    1646, 1733, 1826, 1925, 2031, 2145, 2266, 2397, 2538, 2690, 2856, 3036, 
    3234, 3452, 3692, 3961, 4262, 4604, 4995, 5448
};

static inline int32_t v1_clip(int32_t x, int32_t lo, int32_t hi)
{ return x < lo ? lo : x > hi ? hi : x; }
static inline uint32_t v1_rand(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    *state = x ? x : 0x9e3779b9u;
    return *state;
}
static inline int32_t v1_sat16(int32_t x) { return v1_clip(x,-32767,32767); }

/* BLEP correction for one discontinuity, computed in fixed point.
 * Only the two samples adjacent to an edge require division. */
static inline int32_t v1_blep(uint32_t phase, uint32_t step) {
    uint32_t d = step >> 15;
    if (!d) return 0;
    if (phase < step) {
        int32_t t = v1_clip((int32_t)(phase / d),0,32768);
        return 32768 - 2*t + ((t*t)>>15);
    }
    if (phase > 0xffffffffu - step) {
        int32_t t = v1_clip((int32_t)((0xffffffffu - phase) / d),0,32768);
        return -32768 + 2*t - ((t*t)>>15);
    }
    return 0;
}
static inline int32_t v1_saw_sample(uint32_t phase,uint32_t step)
{ return ((int32_t)(phase >> 16) - 32768) + v1_blep(phase,step); }
static inline int32_t v1_sine_sample(uint32_t ph) {
    uint32_t idx = ph >> 24;
    uint32_t frac = (ph >> 16)&255u;
    int32_t a = v1_sin_q15[idx];
    return a + (((v1_sin_q15[idx+1]-a)*(int32_t)frac)>>8);
}
static inline int32_t v1_triangle_sample(uint32_t ph) {
    uint32_t t = ph >> 16;
    return (int32_t)(t < 32768 ? (t<<1) : ((65535u-t)<<1)) - 32768;
}
/* Piecewise semitone ratios implement full +/-48 semitone envelope-2 pitch
   range; the previous linear cents approximation clipped to one octave. */
static inline uint32_t v1_detune_inc(uint32_t inc, int32_t cents) {
    static const uint32_t ratio[12]={
        65536u,69433u,73561u,77936u,82570u,87481u,
        92682u,98193u,104032u,110219u,116771u,123715u
    };
    int32_t semis=cents/100, frac=cents-semis*100, oct=semis/12;
    int32_t step=semis-oct*12;
    uint64_t tmp;
    if(step<0) {step+=12;oct--;}
    tmp=((uint64_t)inc*ratio[step])>>16;
    if(oct>0)tmp<<=(oct>7?7:oct);
    if(oct<0)tmp>>=(-oct>7?7:-oct);
    tmp=(tmp*(uint32_t)v1_clip(65536+frac*38,32768,131072))>>16;
    return (uint32_t)(tmp>0x60000000u?0x60000000u:tmp);
}
static inline uint32_t v1_note_inc(int32_t note, int32_t cents) {
    uint32_t inc = v1_pitch_inc[v1_clip(note,0,127)];
    return v1_detune_inc(inc, cents);
}
#include "vg1_wave_dsp.h"
/* Each FAT/VCS component is rendered ONCE and goes to its own stereo
 * channel. This is essential to Vanguard's FAT stereo detune character:
 * summing all FAT oscillators to mono destroys the intended width. */
static inline void vg1_osc_stereo(v1_osc_state_t *o,const v1_osc_patch_t *p,
    uint32_t count,int32_t pwm_mod,int32_t *left,int32_t *right) {
    if(!p->level||p->wave==VG1_W_OFF)return;
    if(count<1u)count=1u;
    if(count>V1_SUPER)count=V1_SUPER;
    uint32_t pw=vg1_pw(p->pw,pwm_mod),i;
    int32_t sl=0,sr=0;
    for(i=0;i<count;i++) {
        uint32_t step=o->inc[i],phase=(o->ph[i]+=step);
        int32_t sample=vg1_wave_eval(o,phase,step,pw,p->wave);
        if(count==1u)sl=sr=sample;
        else if(i&1u)sr+=sample;
        else sl+=sample;
    }
    if(count>1u){sl/=(int32_t)((count+1u)/2u);sr/=(int32_t)(count/2u);}
    *left+=(sl*(int32_t)p->level)>>7;
    *right+=(sr*(int32_t)p->level)>>7;
}
static inline uint32_t v1_duration(uint8_t knob) {
    /* 0 ~= 0.7ms; 127 ~= 2s. Exact knob curve is independent of Opera. */
    uint32_t k = (uint32_t)knob;
    uint32_t ms = 1u + ((k*k*2048u)>>14);
    return (ms*V1_SR)/1000u + 1u;
}
static inline int32_t v1_sustain(uint8_t knob) { return (int32_t)knob*258; }
static inline void v1_env_attack(v1_env_state_t *e)
{ e->stage=1; e->level=0; e->release_step=0; }
static inline void v1_env_release(v1_env_state_t *e, const v1_adsr_patch_t *p) {
    if (!e->stage || e->stage==4) return;
    e->stage=4;
    e->release_step=e->level/(int32_t)v1_duration(p->release)+1;
}
static inline int32_t v1_env_sample(v1_env_state_t *e, const v1_adsr_patch_t *p) {
    int32_t sus = v1_sustain(p->sustain);
    switch(e->stage) {
        case 1:
            e->level += 32767/(int32_t)v1_duration(p->attack)+1;
            if(e->level>=32767) {e->level=32767;e->stage=2;}
            break;
        case 2:
            e->level -= (32767-sus)/(int32_t)v1_duration(p->decay)+1;
            if(e->level<=sus) {e->level=sus;e->stage=3;}
            break;
        case 3: e->level=sus; break;
        case 4:
            e->level-=e->release_step;
            if(e->level<=0) {e->level=0;e->stage=0;}
            break;
        default: e->level=0;
    }
    return e->level;
}
typedef struct { int32_t a1,a2,a3,k; } v1_svf_coef_t;
static inline v1_svf_coef_t v1_filter_coeff(int cutoff, int resonance) {
    int32_t g=v1_cut_g[v1_clip(cutoff,0,127)], k=8192-(resonance*7800)/127;
    int32_t den=4096+((g*(g+k))>>12);
    v1_svf_coef_t c;
    if(den<64)den=64;
    c.a1=(4096<<13)/den;
    c.a2=(c.a1*g)>>12;
    c.a3=(c.a2*g)>>12;
    c.k=k;
    return c;
}
static inline int32_t v1_svf(v1_svf_state_t *s,const v1_svf_coef_t *c,int32_t x,uint8_t mode) {
    int32_t v3 = x-s->ic2;
    int32_t v1 = (c->a1*s->ic1+c->a2*v3)>>13;
    int32_t v2 = s->ic2+((c->a2*s->ic1+c->a3*v3)>>13);
    int32_t lo,hi,bp;
    s->ic1=v1*2-s->ic1;
    s->ic2=v2*2-s->ic2;
    s->ic1=v1_clip(s->ic1,-110000,110000);
    s->ic2=v1_clip(s->ic2,-110000,110000);
    lo=v2; bp=v1; hi=x-((c->k*v1)>>12)-v2;
    switch(mode) {
        case V1_HP12: case V1_HP24: return hi;
        case V1_BP12: case V1_BP24: return bp;
        case V1_NOTCH: return lo+hi;
        default: return lo;
    }
}
#include "vg1_filter_dsp.h"
static inline uint32_t v1_quarter(const v1_engine_t *e, uint32_t bpm) {
    (void)e; if (bpm<40)bpm=40; if(bpm>240)bpm=240;
    return 60u*V1_SR/bpm;
}
static inline uint32_t v1_arp_step_frames(uint32_t bpm,uint8_t rate) {
    return v1_quarter(0,bpm)/(rate==0?2u:rate==2?8u:4u);
}
static inline int32_t v1_tri_lfo(uint32_t ph)
{ return (int32_t)(ph>>16)<32768 ? (int32_t)(ph>>15)-32768 : 98303-(int32_t)(ph>>15); }

static void v1_engine_init(v1_engine_t *e,const v1_patch_t *p) {
    uint8_t *b=(uint8_t*)e;
    uint32_t i;
    for(i=0;i<sizeof(*e);i++) b[i]=0;
    e->patch=*p;
    e->rng=0x14f1ab27u;
    e->gate_smooth=e->gate_smooth_right=32767;
}
static void v1_note_on(v1_engine_t *e,uint8_t note,uint8_t vel) {
    uint32_t i,j,choice=0,oldest=0xffffffffu;
    uint32_t max=e->patch.voice_limit?e->patch.voice_limit:8u;
    if(max>V1_VOICES)max=V1_VOICES;
    if(max<1u)max=1u;
    for(i=0;i<max;i++) {
        if(!e->voices[i].active) {choice=i;break;}
        if((uint32_t)e->voices[i].a.level<oldest) {oldest=(uint32_t)e->voices[i].a.level;choice=i;}
    }
    v1_voice_t *v=&e->voices[choice];
    v->active=1;v->gate=1;v->note=note;v->velocity=vel?vel:1;
    v->age=++e->note_age;
    v->f1.ic1=v->f1.ic2=v->f2.ic1=v->f2.ic2=0;
    v->f1_right.ic1=v->f1_right.ic2=v->f2_right.ic1=v->f2_right.ic2=0;
    v->pole6=v->pole6_right=0;
    v1_env_attack(&v->a);v1_env_attack(&v->f);
    if(e->patch.vg_osc.retrig) {
        for(i=0;i<3;i++) if(e->patch.vg_lfo[i].retrig)e->lfo_ph[i]=0;
    }
    for(i=0;i<3;i++) {
        const v1_osc_patch_t *p=&e->patch.osc[i];
        v1_osc_state_t *o=&v->osc[i];
        uint32_t base = v1_note_inc(note+(int32_t)p->semi,p->fine);
        o->rng=v1_rand(&e->rng);
        for(j=0;j<V1_SUPER;j++) {
            uint32_t vcs=e->patch.vg_osc.vcs?e->patch.vg_osc.vcs:2u;
            int32_t centre=(int32_t)(vcs-1u);
            int32_t det=(int32_t)e->patch.vg_osc.fat;
            int32_t cents=((int32_t)j*2-centre)*det*24/127;
            o->base_inc[j]=o->inc[j]=v1_detune_inc(base,cents);
            if(e->patch.vg_osc.retrig) o->ph[j]=0;
            else o->ph[j]=v1_rand(&e->rng);
        }
    }
}
static void v1_note_off(v1_engine_t *e,uint8_t note) {
    uint32_t i;
    for(i=0;i<V1_VOICES;i++)
        if(e->voices[i].active && e->voices[i].gate && e->voices[i].note==note) {
            e->voices[i].gate=0;
            v1_env_release(&e->voices[i].a,&e->patch.amp);
            v1_env_release(&e->voices[i].f,&e->patch.filter_env);
        }
}
/* The 2006 plugin's ARP modes: up/down/alter/ordered/random, with a
 * separate ON switch. Keep an internal OFF value for MIDI event handling. */
static uint32_t v1_arp_mode(const v1_patch_t *p) {
    if(!p->vg_arp.on) return p->arp_mode;
    switch(p->vg_arp.mode) {
        case VG1_ARP_UP: return V1_ARP_UP;
        case VG1_ARP_DOWN: return V1_ARP_DOWN;
        case VG1_ARP_ALTER: return V1_ARP_UPDOWN;
        case VG1_ARP_ORDERED: return V1_ARP_PLAYED;
        case VG1_ARP_RANDOM: return V1_ARP_RANDOM;
        default:return V1_ARP_UP;
    }
}
static void v1_panic(v1_engine_t *e) {
    uint32_t i;
    for(i=0;i<V1_VOICES;i++) {e->voices[i].active=0;e->voices[i].gate=0;}
    e->held_count=0;e->arp_active=0;
}
static void v1_hold(v1_engine_t *e,uint8_t on,uint8_t note,uint8_t vel) {
    uint32_t i;
    if(on) {
        for(i=0;i<e->held_count;i++) if(e->held[i]==note) return;
        if(e->held_count<16) {e->held[e->held_count]=note;e->held_velocity[e->held_count++]=vel;}
        if(e->patch.vg_arp.on&&e->patch.vg_arp.retrig){e->arp_phase=0;e->arp_pos=0;}
        if(v1_arp_mode(&e->patch)==V1_ARP_OFF)v1_note_on(e,note,vel);
    } else {
        for(i=0;i<e->held_count;i++) if(e->held[i]==note) {
            uint32_t j;
            for(j=i+1;j<e->held_count;j++){e->held[j-1]=e->held[j];e->held_velocity[j-1]=e->held_velocity[j];}
            e->held_count--;break;
        }
        if(v1_arp_mode(&e->patch)==V1_ARP_OFF)v1_note_off(e,note);
        else if(!e->held_count && e->arp_active) {
            v1_note_off(e,e->arp_note);e->arp_active=0;
        }
    }
}
static void v1_arp_tick(v1_engine_t *e) {
    uint32_t i,j,idx,oct,mode=v1_arp_mode(&e->patch),count=e->held_count;
    if(mode==V1_ARP_OFF || !count)return;
    if(e->arp_active){v1_note_off(e,e->arp_note);e->arp_active=0;}
    uint8_t sort[16],vel[16];
    for(i=0;i<count;i++){sort[i]=e->held[i];vel[i]=e->held_velocity[i];}
    if(mode!=V1_ARP_PLAYED)
        for(i=1;i<count;i++){uint8_t sn=sort[i],sv=vel[i];j=i;while(j && sort[j-1]>sn){sort[j]=sort[j-1];vel[j]=vel[j-1];j--;}sort[j]=sn;vel[j]=sv;}
    uint32_t octave_count=e->patch.vg_arp.on?e->patch.vg_arp.octaves:e->patch.arp_octaves;
    uint32_t n=count*(octave_count?octave_count:1u);
    if(mode==V1_ARP_DOWN) idx=n-1u-(e->arp_pos%n);
    else if(mode==V1_ARP_RANDOM) idx=v1_rand(&e->rng)%n;
    else if(mode==V1_ARP_UPDOWN) {uint32_t span=n>1?2u*n-2u:1u;idx=e->arp_pos%span;if(idx>=n)idx=span-idx;}
    else idx=e->arp_pos%n;
    oct=idx/count;
    uint32_t n0=(uint32_t)sort[idx%count]+12u*oct;
    if(n0>127)n0=127;
    e->arp_note=(uint8_t)n0;
    v1_note_on(e,e->arp_note,vel[idx%count]);
    e->arp_active=1;e->arp_pos++;
}
static void v1_engine_render(v1_engine_t *e,int32_t *out,uint32_t n,uint32_t bpm) {
    const v1_patch_t *p=&e->patch;
    uint32_t f,i,j,step_frames=v1_arp_step_frames(bpm,e->patch.arp_rate);
    uint32_t gate_frames=v1_arp_step_frames(bpm,e->patch.trance_rate);
    const uint32_t quarter=v1_quarter(e,bpm);
    if(e->patch.vg_arp.on) step_frames=vg1_rate_samples(quarter,e->patch.vg_arp.speed);
    if(e->patch.vg_gate.enabled)gate_frames=vg1_rate_samples(quarter,e->patch.vg_gate.speed);
    uint32_t delay_frames=v1_quarter(e,bpm)*3u/4u;
    uint32_t lfo_inc[3],vi;
    uint32_t arp_gate=p->vg_arp.on?p->vg_arp.gate:p->arp_gate;
    uint32_t arp_release;
    int32_t bendc,vib_depthc;
    if(e->patch.vg_delay.on) {
        if(e->patch.vg_delay.type==VG1_DELAY_WIDEN)
            delay_frames=220u*(e->patch.vg_delay.widen_ms?e->patch.vg_delay.widen_ms:80u);
        else delay_frames=vg1_rate_samples(quarter,e->patch.vg_delay.speed);
    }
    if(delay_frames>32767u) delay_frames=32767u;
    if(!delay_frames)delay_frames=1u;
    /* Block-derived control rates keep expensive integer division OUT of
       the 44.1kHz sample loop. The 256-frame ISR has ~5.8ms deadline. */
    for(vi=0;vi<3;vi++) {
        uint32_t hz=p->vg_lfo[vi].speed?p->vg_lfo[vi].speed:
                    (uint32_t)p->lfo_rate[vi]*60u/127u;
        lfo_inc[vi]=hz*97392u;
    }
    bendc=(e->bend14*(int32_t)p->bend_range*100)/8192;
    vib_depthc=((int32_t)e->aftertouch*(int32_t)p->vib_depth*100)/(127*127);
    if(!step_frames)step_frames=1;
    if(!gate_frames)gate_frames=1;
    arp_release=(step_frames*(arp_gate+1u))/128u;
    for(f=0;f<n;f++) {
        int32_t dryl=0,dryr=0,left,right;
        if(v1_arp_mode(p)!=V1_ARP_OFF){
            if(e->arp_phase==0 && e->held_count)v1_arp_tick(e);
            if(e->arp_active && e->arp_phase > arp_release) {
                v1_note_off(e,e->arp_note);e->arp_active=0;
            }
            e->arp_phase++;
            if(e->arp_phase>=step_frames)e->arp_phase=0;
        }
        if(e->gate_phase++>=gate_frames){e->gate_phase=1;e->gate_step=(e->gate_step+1u)&15u;}
        uint32_t stereo=p->vg_gate.enabled&&p->vg_gate.stereo;
        uint32_t on=p->vg_gate.enabled?p->vg_gate.enabled:p->trance_enabled;
        uint16_t mask_l=p->vg_gate.enabled?p->vg_gate.left:p->trance_mask;
        uint16_t mask_r=stereo?p->vg_gate.right:mask_l;
        int32_t target=(!on || (mask_l&(1u<<e->gate_step)))?32767:0;
        int32_t target_r=(!on || (mask_r&(1u<<e->gate_step)))?32767:0;
        uint32_t contour=p->vg_gate.enabled?p->vg_gate.contour:64u;
        uint32_t slew=2u+((127u-contour)>>2);
        e->gate_smooth+=(target-e->gate_smooth)/(int32_t)slew;
        e->gate_smooth_right+=(target_r-e->gate_smooth_right)/(int32_t)slew;
        /* Three independent LFOs. Calculations are integer and bounded. */
        for(j=0;j<3;j++) {
            e->lfo_ph[j]+=lfo_inc[j];
            e->lfo[j]=v1_tri_lfo(e->lfo_ph[j]);
        }
        e->vibrato_ph+=(uint32_t)(p->vib_speed+1u)*45000u;
        int32_t vibc=(vib_depthc*v1_sine_sample(e->vibrato_ph))>>15;
        for(i=0;i<V1_VOICES;i++) {
            v1_voice_t *v=&e->voices[i];
            int32_t sl=0,sr=0,fe,amp,cut;
            if(!v->active)continue;
            /* Pitch modulation applies to ALL three oscillators, including
             * every supersaw component. No oscillator's base pitch drifts. */
            if((e->frames&31u)==0u) {
                uint32_t oi,si;
                v->drift=v1_clip(v->drift+(int32_t)(v1_rand(&e->rng)&7u)-3,-256,256);
                int32_t age_cents=(v->drift*(int32_t)p->vg_osc.age)>>10;
                int32_t env_pitch=((int32_t)p->vg_env2.pitch*v->f.level)>>15;
                for(oi=0;oi<3;oi++) {
                    int32_t cents=((int32_t)p->vg_lfo[oi].detune*e->lfo[oi])>>15;
                    cents+=(((int32_t)p->vg_env1.detune*v->a.level)>>15);
                    cents+=env_pitch*100+bendc+vibc+age_cents;
                    if(!oi)cents+=((int32_t)p->lfo_pitch*e->lfo[0])>>16;
                    for(si=0;si<V1_SUPER;si++)
                        v->osc[oi].inc[si]=v1_detune_inc(v->osc[oi].base_inc[si],cents);
                }
            }
            amp=v1_env_sample(&v->a,&p->amp);
            fe=v1_env_sample(&v->f,&p->filter_env);
            if(!amp && !v->gate){v->active=0;continue;}
            uint32_t vcs=p->vg_osc.fat?(p->vg_osc.vcs?p->vg_osc.vcs:2u):1u;
            if(vcs<1u)vcs=1u;
            if(vcs>10u)vcs=10u;
            for(j=0;j<3;j++) {
                int32_t pwm=((int32_t)p->vg_lfo[j].pwm*e->lfo[j])>>15;
                pwm+=((int32_t)p->vg_env1.pwm*amp)>>15;
                pwm+=((int32_t)p->vg_env2.pwm*fe)>>15;
                vg1_osc_stereo(&v->osc[j],&p->osc[j],vcs,pwm,&sl,&sr);
            }
            sl=v1_clip(sl,-50000,50000);
            sr=v1_clip(sr,-50000,50000);
            cut=(int32_t)p->cutoff
               + (((int32_t)p->vg_env1.cutoff*amp)>>15)
               + (((int32_t)p->vg_env2.cutoff*fe)>>15)
               + (((int32_t)p->filter_env_depth*fe)>>15)
               + ((int32_t)p->vg_keytrack.keytrk*((int32_t)v->note-60))/64
               + ((int32_t)p->vg_keytrack.veltrk*((int32_t)v->velocity-64))/64;
            for(j=0;j<3;j++)cut+=((int32_t)p->vg_lfo[j].cutoff*e->lfo[j])>>15;
            cut+=((int32_t)p->lfo_filter*e->lfo[1]>>16);
            cut=v1_clip(cut,0,127);
            v1_svf_coef_t c,c2;
            if((e->frames&31u)==0u) {
                int32_t res=v1_clip((int32_t)p->resonance+(((int32_t)p->vg_env2.resonance*fe)>>15),0,127);
                c=v1_filter_coeff(cut,res);
                c2=v1_filter_coeff(v1_clip(cut+10,0,127),res);
                v->fa1=c.a1;v->fa2=c.a2;v->fa3=c.a3;v->fk=c.k;
                v->fb1=c2.a1;v->fb2=c2.a2;v->fb3=c2.a3;v->bk=c2.k;
            } else {
                c.a1=v->fa1;c.a2=v->fa2;c.a3=v->fa3;c.k=v->fk;
                c2.a1=v->fb1;c2.a2=v->fb2;c2.a3=v->fb3;c2.k=v->bk;
            }
            sl=vg1_filter_render(&v->f1,&v->f2,&v->pole6,&c,&c2,sl,p->filter_type);
            sr=vg1_filter_render(&v->f1_right,&v->f2_right,&v->pole6_right,
                                  &c,&c2,sr,p->filter_type);
            if(p->vg_amp.drive) {
                sl=v1_sat16(sl+((sl*(int32_t)p->vg_amp.drive)>>7));
                sr=v1_sat16(sr+((sr*(int32_t)p->vg_amp.drive)>>7));
            }
            /* Vanguard VELTRK=0 means NO velocity sensitivity. Positive
             * tracking grows with velocity; negative tracking reverses it. */
            int32_t vel_gain=v1_clip(16384+
                ((int32_t)v->velocity-64)*(int32_t)p->vg_amp.veltrk*4,0,32767);
            amp=(amp*vel_gain)>>14;
            if(p->vg_env1.level)
                amp=v1_clip((amp*(127+(int32_t)p->vg_env1.level))>>7,0,32767);
            amp=(amp*(int32_t)p->vg_amp.level)>>7;
            sl=(sl*amp)>>15;
            sr=(sr*amp)>>15;
            dryl+=sl;
            dryr+=sr;
        }
        dryl=v1_clip(dryl,-65535,65535);
        dryr=v1_clip(dryr,-65535,65535);
        /* AMP PAN plus the original SPREAD autopanning parameter.
           Oscillator FAT stereo survives this stage. */
        e->auto_pan_ph+=131072u;
        int32_t pan=v1_clip((int32_t)p->vg_amp.pan+
                  ((int32_t)p->vg_amp.spread*v1_sine_sample(e->auto_pan_ph)>>16),0,127);
        dryl=(dryl*(254-pan*2))/127;
        dryr=(dryr*(pan*2))/127;
        /* Gate is applied to stereo after per-voice synthesis, not copied into
           each oscillator. ReFX 1.6 permits independent L/R gate patterns. */
        /* The original Vanguard has no chorus block. Remove the generic
           prototype chorus entirely. FAT/VCS provide stereo width instead. */
        int32_t mix=p->vg_gate.enabled?p->vg_gate.mix:p->trance_depth;
        int32_t gl=on?(32767-((int32_t)mix*(32767-e->gate_smooth)/127)):32767;
        int32_t gr=on?(32767-((int32_t)mix*(32767-e->gate_smooth_right)/127)):32767;
        left=(dryl*gl)>>15;
        right=(dryr*gr)>>15;
        /* Original Vanguard order: TRANCEGATE -> DELAY -> REVERB.
           Delay supports the documented mono, stereo, cross and widen modes. */
        uint32_t dl=e->delay_pos&32767u;
        int32_t oldl=e->delay_mem[0][(dl-delay_frames)&32767u];
        int32_t oldr=e->delay_mem[1][(dl-delay_frames)&32767u];
        uint32_t dmode=p->vg_delay.on?p->vg_delay.type:VG1_DELAY_CROSS;
        int32_t dfb=p->vg_delay.on?p->vg_delay.feedback:p->delay_feedback;
        int32_t dmix=p->vg_delay.on?p->vg_delay.mix:p->delay;
        int32_t damp=p->vg_delay.on?p->vg_delay.damp:0;
        e->delay_lp[0]+=(oldl-e->delay_lp[0])*(127-damp)/127;
        e->delay_lp[1]+=(oldr-e->delay_lp[1])*(127-damp)/127;
        oldl=e->delay_lp[0]; oldr=e->delay_lp[1];
        int32_t fl=0,fr=0;
        switch(dmode) {
            case VG1_DELAY_MONO: fl=fr=(oldl+oldr)>>1; break;
            case VG1_DELAY_STEREO: fl=oldl;fr=oldr;break;
            case VG1_DELAY_CROSS: fl=oldr;fr=oldl;break;
            case VG1_DELAY_WIDEN: fl=0;fr=oldr;break;
            default:fl=oldr;fr=oldl;break;
        }
        e->delay_mem[0][dl]=(int16_t)v1_sat16(left+((fl*dfb)>>7));
        e->delay_mem[1][dl]=(int16_t)v1_sat16(right+((fr*dfb)>>7));
        e->delay_pos++;
        if(dmode!=VG1_DELAY_WIDEN)left+=(oldl*dmix)>>7;
        right+=(oldr*dmix)>>7;

        /* Room model: four comb paths, stereo mix, damp, size and width.
           Input AFTER delay, as described in the Vanguard 1.6 manual. */
        static const uint16_t taps[4]={1109,1327,1559,1801};
        int32_t room=p->vg_reverb.on?p->vg_reverb.room_size:64;
        int32_t rdamp=p->vg_reverb.on?p->vg_reverb.damp:32;
        int32_t rwidth=p->vg_reverb.on?p->vg_reverb.width:127;
        int32_t rmix=p->vg_reverb.on?p->vg_reverb.mix:p->reverb;
        int32_t rev_in=(left+right)>>2;
        int32_t wetl=0,wetr=0;
        uint32_t rp=e->reverb_pos&2047u;
        for(j=0;j<4;j++) {
            uint32_t tap=taps[j]+(uint32_t)room/2u;
            uint32_t index=(rp+2048u-tap)&2047u;
            int32_t value=e->reverb_mem[j][index];
            e->reverb_lp[j]+=(value-e->reverb_lp[j])*(127-rdamp)/127;
            int32_t fb=e->reverb_lp[j];
            if(j&1u) wetr+=fb; else wetl+=fb;
            e->reverb_mem[j][rp]=(int16_t)v1_sat16((rev_in>>1)+((fb*96)>>7));
        }
        e->reverb_pos++;
        int32_t wetm=(wetl+wetr)>>1;
        wetl=(wetl*rwidth+wetm*(127-rwidth))/127;
        wetr=(wetr*rwidth+wetm*(127-rwidth))/127;
        left+=(wetl*rmix)>>9;
        right+=(wetr*rmix)>>9;
        left=(left*(int32_t)p->master)>>7;
        right=(right*(int32_t)p->master)>>7;
        out[f*2u]+=v1_sat16(left);
        out[f*2u+1u]+=v1_sat16(right);
        if((uint32_t)(left<0?-left:left)>e->render_peak)e->render_peak=(uint32_t)(left<0?-left:left);
        e->frames++;
    }
}
#endif /* V1_CORE_H */
