/* SPDX-License-Identifier: GPL-3.0-only
 * Vanguard 1.x recreation: user-visible parameter contract transcribed into
 * an original implementation from the 2006 Vanguard v1.6 manual.
 *
 * SOURCE: reFX Vanguard Manual v1.6.0 (26 December 2006), sections:
 * OSC, LFO, FILTER, ENV, AMP, TRANCEGATE, DELAY, REVERB, ARP, GLIDE.
 *
 * Wave/filter NAMES are interoperability identifiers, not claims of
 * reFX DSP identity. DSP implementation is original and must be calibrated
 * against legally obtained reference audio before being called accurate.
 */
#ifndef VG1_REFERENCE_H
#define VG1_REFERENCE_H
#include <stdint.h>

/* Index 0 is the menu's oscillator OFF item; 1..31 its named waveforms. */
enum {
    VG1_W_OFF = 0,
    VG1_W_SQUARE = 1,
    VG1_W_SQUARE2 = 2,
    VG1_W_SAWTOOTH = 3,
    VG1_W_PEAK = 4,
    VG1_W_PEAK_PWM = 5,
    VG1_W_COMBED_SQR1 = 6,
    VG1_W_COMBED_SQR2 = 7,
    VG1_W_COMBED_SQR3 = 8,
    VG1_W_SAW_IO = 9,
    VG1_W_SINE = 10,
    VG1_W_PULSE_PWM = 11,
    VG1_W_PULSE_DBL_PWM = 12,
    VG1_W_PULSE_TPL_PWM = 13,
    VG1_W_BI_PULSE = 14,
    VG1_W_FM_PULSE = 15,
    VG1_W_PWM_SAW = 16,
    VG1_W_BI_SAW = 17,
    VG1_W_SYNCED_SAW = 18,
    VG1_W_AJX_SAW = 19,
    VG1_W_AJX_SAW2 = 20,
    VG1_W_AJX_SAW3 = 21,
    VG1_W_TRI_SAW_MORPH = 22,
    VG1_W_SIN_PEAK_MORPH = 23,
    VG1_W_SINE_FM = 24,
    VG1_W_SINE_AM = 25,
    VG1_W_R2D2 = 26,
    VG1_W_DIGITAL = 27,
    VG1_W_SYNCED_NOISE = 28,
    VG1_W_C64_NOISE = 29,
    VG1_W_HIPASS_NOISE = 30,
    VG1_W_WHITE_NOISE = 31,
    VG1_N_WAVES = 32
};
static const char *const vg1_wave_name[VG1_N_WAVES] = {
    "---",
    "square",
    "square2",
    "sawtooth",
    "peak",
    "peak pwm",
    "combed sqr",
    "combed sqr2",
    "combed sqr3",
    "saw i/o",
    "sine",
    "pulse pwm",
    "pulse dbl pwm",
    "pulse tpl pwm",
    "bi-pulse",
    "fm pulse",
    "pwm saw",
    "bi-saw",
    "synced saw",
    "ajx saw",
    "ajx saw2",
    "ajx saw3",
    "tri/saw morph",
    "sin/peak morph",
    "sine fm",
    "sine am",
    "r2d2",
    "digital",
    "synced noise",
    "c64 noise",
    "hi-pass noise",
    "white noise",
};

/* The ORIGINAL 13 names and order, not invented substitute filter labels. */
enum {
    VG1_F_LOW6 = 0,
    VG1_F_LOW12 = 1,
    VG1_F_LOW24 = 2,
    VG1_F_HIGH6 = 3,
    VG1_F_HIGH24 = 4,
    VG1_F_X = 5,
    VG1_F_BAND24 = 6,
    VG1_F_FORMANT = 7,
    VG1_F_NOTCH = 8,
    VG1_F_NOTCH_LOW = 9,
    VG1_F_M = 10,
    VG1_F_BAND_SHP1 = 11,
    VG1_F_BAND_SHP2 = 12,
    VG1_N_FILTERS = 13
};
static const char *const vg1_filter_name[VG1_N_FILTERS] = {
    "low 6db",
    "low 12db",
    "low 24db",
    "high 6db",
    "high 24db",
    "-X-",
    "band 24db",
    "formant",
    "notch",
    "notch+low",
    "-M-",
    "band+shp1",
    "band+shp2",
};

enum { VG1_ARP_UP, VG1_ARP_DOWN, VG1_ARP_ALTER, VG1_ARP_ORDERED, VG1_ARP_RANDOM, VG1_ARP_N_MODES };
static const char *const vg1_arp_name[VG1_ARP_N_MODES]={"up","down","alter","ordered","random"};
enum { VG1_DELAY_MONO, VG1_DELAY_STEREO, VG1_DELAY_CROSS, VG1_DELAY_WIDEN, VG1_N_DELAY_MODES };
static const char *const vg1_delay_name[VG1_N_DELAY_MODES]={"mono","stereo","cross","widen"};

/* Rates use whole, half, quarter... sixty-fourth notes. Triplet and dotted
 * variations are available for each subdivision, as in Vanguard 1.6. */
enum { VG1_RATE_STRAIGHT,VG1_RATE_TRIPLET,VG1_RATE_DOTTED };
typedef struct {
    uint8_t denominator; /* 1, 2, 4, 8, 16, 32, 64 */
    uint8_t variant;     /* straight, triplet, dotted */
} vg1_rate_t;
static inline uint32_t vg1_rate_samples(uint32_t quarter_samples,vg1_rate_t r) {
    uint32_t d=r.denominator;
    if(d!=1 && d!=2 && d!=4 && d!=8 && d!=16 && d!=32 && d!=64) d=16;
    /* e.g. sixteenth = 1/4 quarter, triplet = 2/3, dotted = 3/2 */
    uint32_t frames=(quarter_samples*4u)/d;
    if(r.variant==VG1_RATE_TRIPLET) frames=frames*2u/3u;
    else if(r.variant==VG1_RATE_DOTTED) frames=frames*3u/2u;
    return frames?frames:1u;
}
/* Manual §ENV: both envelopes are ADSR and each offers four MODULATION outputs.
 * ENV1: level, pwm, cutoff, detune (in cents).
 * ENV2: resonance, pwm, cutoff, pitch (up to +/-48 semitones).
 */
typedef struct { int8_t level,pwm,cutoff,detune; } vg1_env1_mod_t;
typedef struct { int8_t resonance,pwm,cutoff,pitch; } vg1_env2_mod_t;
typedef struct {
    uint8_t speed;     /* 0..60 Hz mapped from 0..127 */
    int8_t detune;     /* Oscillator n only */
    int8_t pwm;        /* Oscillator n only */
    int8_t cutoff;     /* Master filter */
    uint8_t retrig;   /* synchronized note-trigger */
} vg1_lfo_patch_t;
typedef struct {
    uint8_t enabled,stereo,mix,contour;
    uint16_t left,right; /* independent 16-step masks, not mono-only */
    vg1_rate_t speed;
    uint8_t sync_to_song_position;
} vg1_trancegate_patch_t;
typedef struct {
    uint8_t on,mix,type,feedback,damp;
    vg1_rate_t speed;
    uint8_t widen_ms; /* used in widen mode */
} vg1_delay_patch_t;
typedef struct {
    uint8_t on,mix,predelay,room_size,damp,width;
} vg1_reverb_patch_t;
typedef struct {
    uint8_t on,mode,retrig,gate,octaves;
    vg1_rate_t speed;
} vg1_arpeggiator_patch_t;
typedef struct {
    uint8_t on;
    vg1_rate_t speed;
} vg1_glide_patch_t;
typedef struct {
    uint8_t retrig,fat,vcs,age; /* VCS: 2..10 stereo fat voices */
} vg1_osc_global_t;
typedef struct {
    int8_t keytrk,veltrk;
} vg1_filter_tracking_t;
typedef struct {
    uint8_t drive,level,pan,spread;
    int8_t veltrk;
} vg1_amp_patch_t;
#endif
