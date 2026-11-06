/* SPDX-License-Identifier: GPL-3.0-only
 * V-1 FM1: original trance-oriented synthesizer DSP.
 * Entirely new instrument and voice model; ONLY the FM1 HAL is reused.
 * No reFX binary, presets, samples, artwork or proprietary DSP.
 * The fixed-point audio path deliberately makes no libm calls.
 */
#ifndef V1_CORE_H
#define V1_CORE_H
#include <stdint.h>
#define V1_SR 44100u
#define V1_VOICES 8u
#define V1_OSCS 3u
#define V1_SUPER 5u

enum { V1_OFF, V1_SAW, V1_PULSE, V1_TRI, V1_SINE, V1_SUPERSAW, V1_NOISE };
enum { V1_LP12, V1_HP12, V1_BP12, V1_NOTCH, V1_LP24, V1_HP24, V1_BP24 };
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
} v1_patch_t;

typedef struct { uint32_t ph[5], inc[5], base_inc[5]; uint32_t rng; } v1_osc_state_t;
typedef struct { int32_t ic1, ic2; } v1_svf_state_t;
typedef struct { uint8_t stage; int32_t level, release_step; } v1_env_state_t;
typedef struct {
    v1_osc_state_t osc[3];
    v1_svf_state_t f1, f2;
    v1_env_state_t a, f;
    uint32_t age;
    int32_t fa1,fa2,fa3,fk; /* filter coefficients, updated at control rate */
    uint8_t note, velocity, gate, active;
} v1_voice_t;

typedef struct {
    v1_patch_t patch;
    v1_voice_t voices[V1_VOICES];
    uint32_t note_age, rng;
    uint32_t lfo_ph[3], frames;
    int32_t lfo[3];
    uint16_t gate_step;
    uint32_t gate_phase;
    int32_t gate_smooth;
    uint8_t held[16], held_velocity[16], held_count, arp_pos;
    uint8_t arp_note, arp_active, arp_direction;
    uint32_t arp_phase;
    int16_t chorus_mem[2][1024];
    uint32_t chorus_pos;
    int16_t delay_mem[2][32768];
    uint32_t delay_pos;
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
static inline uint32_t v1_detune_inc(uint32_t inc, int32_t cents) {
    int32_t k = v1_clip(65536 + cents*38, 4096, 131072);
    return (uint32_t)(((uint64_t)inc*(uint32_t)k)>>16);
}
static inline uint32_t v1_note_inc(int32_t note, int32_t cents) {
    uint32_t inc = v1_pitch_inc[v1_clip(note,0,127)];
    return v1_detune_inc(inc, cents);
}
static inline int32_t v1_osc_sample(v1_osc_state_t *o, const v1_osc_patch_t *p) {
    uint32_t j, n = p->wave == V1_SUPERSAW ? V1_SUPER : 1u;
    int32_t sum = 0;
    if (!p->level || p->wave == V1_OFF) return 0;
    if (p->wave == V1_NOISE)
        return (int32_t)(v1_rand(&o->rng)>>16)-32768;
    for (j=0;j<n;j++) {
        uint32_t ph = o->ph[j] += o->inc[j], inc = o->inc[j];
        int32_t x;
        switch (p->wave) {
            case V1_SAW: case V1_SUPERSAW: x = v1_saw_sample(ph,inc); break;
            case V1_PULSE: {
                uint32_t pw = ((uint32_t)(p->pw+8u)<<24);
                x = v1_saw_sample(ph,inc) - v1_saw_sample(ph+pw,inc);
                break;
            }
            case V1_TRI: x = v1_triangle_sample(ph); break;
            case V1_SINE: x = v1_sine_sample(ph); break;
            default: x=0; break;
        }
        sum += x;
    }
    return (sum / (int32_t)n) * (int32_t)p->level >> 7;
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
    e->gate_smooth=32767;
}
static void v1_note_on(v1_engine_t *e,uint8_t note,uint8_t vel) {
    uint32_t i,j,choice=0,oldest=0xffffffffu;
    for(i=0;i<V1_VOICES;i++) {
        if(!e->voices[i].active) {choice=i;break;}
        if(e->voices[i].a.level<(int32_t)oldest) {oldest=(uint32_t)e->voices[i].a.level;choice=i;}
    }
    v1_voice_t *v=&e->voices[choice];
    v->active=1;v->gate=1;v->note=note;v->velocity=vel?vel:1;
    v->age=++e->note_age;v->f1.ic1=v->f1.ic2=v->f2.ic1=v->f2.ic2=0;
    v1_env_attack(&v->a);v1_env_attack(&v->f);
    for(i=0;i<3;i++) {
        const v1_osc_patch_t *p=&e->patch.osc[i];
        v1_osc_state_t *o=&v->osc[i];
        uint32_t base = v1_note_inc(note+(int32_t)p->semi,p->fine);
        o->rng=v1_rand(&e->rng);
        for(j=0;j<V1_SUPER;j++) {
            int32_t dist=(int32_t)j-2;
            int32_t cents=dist*((int32_t)p->detune*40/127);
            o->base_inc[j]=o->inc[j]=v1_detune_inc(base,cents);
            o->ph[j]=v1_rand(&e->rng);
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
        if(e->patch.arp_mode==V1_ARP_OFF)v1_note_on(e,note,vel);
    } else {
        for(i=0;i<e->held_count;i++) if(e->held[i]==note) {
            uint32_t j;
            for(j=i+1;j<e->held_count;j++){e->held[j-1]=e->held[j];e->held_velocity[j-1]=e->held_velocity[j];}
            e->held_count--;break;
        }
        if(e->patch.arp_mode==V1_ARP_OFF)v1_note_off(e,note);
        else if(!e->held_count && e->arp_active) {
            v1_note_off(e,e->arp_note);e->arp_active=0;
        }
    }
}
static void v1_arp_tick(v1_engine_t *e) {
    uint32_t i,j,idx,oct,mode=e->patch.arp_mode,count=e->held_count;
    if(mode==V1_ARP_OFF || !count)return;
    if(e->arp_active){v1_note_off(e,e->arp_note);e->arp_active=0;}
    uint8_t sort[16],vel[16];
    for(i=0;i<count;i++){sort[i]=e->held[i];vel[i]=e->held_velocity[i];}
    if(mode!=V1_ARP_PLAYED)
        for(i=1;i<count;i++){uint8_t sn=sort[i],sv=vel[i];j=i;while(j && sort[j-1]>sn){sort[j]=sort[j-1];vel[j]=vel[j-1];j--;}sort[j]=sn;vel[j]=sv;}
    uint32_t n=count*(e->patch.arp_octaves?e->patch.arp_octaves:1u);
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
    uint32_t f,i,j,step_frames=v1_arp_step_frames(bpm,e->patch.arp_rate);
    uint32_t gate_frames=v1_arp_step_frames(bpm,e->patch.trance_rate);
    uint32_t delay_frames=v1_quarter(e,bpm)*3u/4u;
    if(delay_frames>32767u) delay_frames=32767u;
    const v1_patch_t *p=&e->patch;
    if(!step_frames)step_frames=1;
    if(!gate_frames)gate_frames=1;
    for(f=0;f<n;f++) {
        int32_t dry=0, left,right;
        if(p->arp_mode!=V1_ARP_OFF){
            if(e->arp_phase==0 && e->held_count)v1_arp_tick(e);
            if(e->arp_active && e->arp_phase > (step_frames*(uint32_t)(p->arp_gate+1u))/128u) {
                v1_note_off(e,e->arp_note);e->arp_active=0;
            }
            e->arp_phase++;
            if(e->arp_phase>=step_frames)e->arp_phase=0;
        }
        if(e->gate_phase++>=gate_frames){e->gate_phase=1;e->gate_step=(e->gate_step+1u)&15u;}
        int32_t target=(!p->trance_enabled || (p->trance_mask&(1u<<e->gate_step)))?32767:0;
        e->gate_smooth+=(target-e->gate_smooth)>>5;
        /* Three independent LFOs. Calculations are integer and bounded. */
        for(j=0;j<3;j++) {
            uint32_t inc=(uint32_t)(p->lfo_rate[j]+1u)*50000u; /* ~0.5 to 65 Hz */
            e->lfo_ph[j]+=inc;
            e->lfo[j]=v1_tri_lfo(e->lfo_ph[j]);
        }
        for(i=0;i<V1_VOICES;i++) {
            v1_voice_t *v=&e->voices[i];
            int32_t s=0,fe,amp,cut;
            if(!v->active)continue;
            /* Pitch modulation applies to ALL three oscillators, including
             * every supersaw component. No oscillator's base pitch drifts. */
            if((e->frames&31u)==0u) {
                uint32_t oi,si;
                int32_t cents=((int32_t)p->lfo_pitch*e->lfo[0])>>16;
                for(oi=0;oi<3;oi++) {
                    uint32_t count=p->osc[oi].wave==V1_SUPERSAW?5u:1u;
                    for(si=0;si<count;si++)
                        v->osc[oi].inc[si]=v1_detune_inc(v->osc[oi].base_inc[si],cents);
                }
            }
            amp=v1_env_sample(&v->a,&p->amp);
            fe=v1_env_sample(&v->f,&p->filter_env);
            if(!amp && !v->gate){v->active=0;continue;}
            for(j=0;j<3;j++) s+=v1_osc_sample(&v->osc[j],&p->osc[j]);
            s=v1_clip(s,-50000,50000);
            if(p->drive)s=v1_sat16(s+((int32_t)p->drive*s>>7));
            cut=(int32_t)p->cutoff + (((int32_t)p->filter_env_depth*fe)>>15)
                + ((int32_t)p->lfo_filter*e->lfo[1]>>16);
            cut=v1_clip(cut,0,127);
            v1_svf_coef_t c;
            if((e->frames&31u)==0u) {
                c=v1_filter_coeff(cut,p->resonance);
                v->fa1=c.a1;v->fa2=c.a2;v->fa3=c.a3;v->fk=c.k;
            } else {c.a1=v->fa1;c.a2=v->fa2;c.a3=v->fa3;c.k=v->fk;}
            s=v1_svf(&v->f1,&c,s,p->filter_type);
            if(p->filter_type>=V1_LP24)s=v1_svf(&v->f2,&c,s,p->filter_type);
            amp=(amp*(int32_t)v->velocity)>>7;
            if(p->lfo_amp)amp=(amp*(32767-((p->lfo_amp*(e->lfo[2]+32768))>>8)))>>15;
            s=(s*amp)>>15;
            dry+=s;
        }
        dry=v1_clip(dry,-65535,65535);
        if(p->trance_enabled)
            dry=(dry*(32767-((int32_t)p->trance_depth*(32767-e->gate_smooth)/127)))>>15;
        /* Effects deliberately post-gate: echoed/reverberant tails survive gates. */
        int32_t ci=e->chorus_pos&1023u;
        uint32_t mod=(uint32_t)(e->lfo[2]+32768);
        uint32_t d1=430u+(mod>>9), d2=600u-(mod>>9);
        int32_t chl=e->chorus_mem[0][(ci-d1)&1023u];
        int32_t chr=e->chorus_mem[1][(ci-d2)&1023u];
        e->chorus_mem[0][ci]=(int16_t)v1_sat16(dry);
        e->chorus_mem[1][ci]=(int16_t)v1_sat16(dry);
        e->chorus_pos++;
        left=dry+((chl*(int32_t)p->chorus)>>8);
        right=dry+((chr*(int32_t)p->chorus)>>8);
        uint32_t dl=e->delay_pos&32767u;
        int32_t oldl=e->delay_mem[0][(dl-delay_frames)&32767u];
        int32_t oldr=e->delay_mem[1][(dl-delay_frames)&32767u];
        e->delay_mem[0][dl]=(int16_t)v1_sat16(left+((oldr*(int32_t)p->delay_feedback)>>7));
        e->delay_mem[1][dl]=(int16_t)v1_sat16(right+((oldl*(int32_t)p->delay_feedback)>>7));
        e->delay_pos++;
        left+=(oldl*(int32_t)p->delay)>>7;
        right+=(oldr*(int32_t)p->delay)>>7;
        static const uint16_t tap[4]={1109,1327,1559,1801};
        int32_t wet=0;
        uint32_t rp=e->reverb_pos&2047u;
        for(j=0;j<4;j++){
            uint32_t index=(rp+2048u-tap[j])&2047u;
            int32_t value=e->reverb_mem[j][index];
            wet+=value;
            e->reverb_mem[j][rp]=(int16_t)v1_sat16((dry>>2)+((value*96)>>7));
        }
        e->reverb_pos++;
        left+=(wet*(int32_t)p->reverb)>>10;
        right+=(wet*(int32_t)p->reverb)>>10;
        left=(left*(int32_t)p->master)>>7;
        right=(right*(int32_t)p->master)>>7;
        out[f*2u]+=v1_sat16(left);
        out[f*2u+1u]+=v1_sat16(right);
        if((uint32_t)(left<0?-left:left)>e->render_peak)e->render_peak=(uint32_t)(left<0?-left:left);
        e->frames++;
    }
}
#endif /* V1_CORE_H */
