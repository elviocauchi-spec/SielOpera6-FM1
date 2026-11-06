/* SPDX-License-Identifier: GPL-3.0-only
 * Original DSP approximations of the Vanguard 1.6 oscillator menu.
 * The names and modulation functions follow the manual; the transfer
 * functions are newly written and REQUIRE sonic A/B calibration.
 * Include from v1_core.h AFTER the fixed-point oscillator primitives.
 */
#ifndef VG1_WAVE_DSP_H
#define VG1_WAVE_DSP_H
/* x is a 32-bit cyclic phase. The pulse transitions are polyBLEP-corrected
 * using two saw discontinuities. "pwm" always has a stable 5%-95% limit. */
static inline int32_t vg1_pulse(uint32_t ph,uint32_t inc,uint32_t width) {
    return v1_saw_sample(ph,inc)-v1_saw_sample(ph+width,inc);
}
static inline uint32_t vg1_pw(uint8_t pw,int32_t mod) {
    int32_t p=v1_clip((int32_t)pw+mod,6,121);
    return (uint32_t)p*0x02020202u; /* fraction 0..1 across 0..127 */
}
static inline int32_t vg1_triangle(uint32_t ph) {
    return v1_triangle_sample(ph);
}
static inline int32_t vg1_wave_eval(v1_osc_state_t *o,uint32_t phase,
                                    uint32_t step,uint32_t pw,uint8_t wave) {
    int32_t saw=v1_saw_sample(phase,step);
    int32_t sine, a, b;
    uint32_t ph2;
    switch(wave) {
        case VG1_W_SQUARE: return vg1_pulse(phase,step,0x80000000u);
        case VG1_W_SQUARE2: return (vg1_pulse(phase,step,0x55555555u)
                                  +vg1_pulse(phase,step,0xaaaaaaaa))/2;
        case VG1_W_SAWTOOTH: return saw;
        case VG1_W_PEAK: return vg1_pulse(phase,step,0x028f5c28u);
        case VG1_W_PEAK_PWM: return vg1_pulse(phase,step,pw>>3);
        case VG1_W_COMBED_SQR1:
            return (vg1_pulse(phase,step,0x80000000u)+vg1_pulse(phase*2u,step*2u,0x80000000u))/2;
        case VG1_W_COMBED_SQR2:
            return (vg1_pulse(phase,step,0x80000000u)+vg1_pulse(phase*3u,step*3u,0x80000000u))/2;
        case VG1_W_COMBED_SQR3:
            return (vg1_pulse(phase,step,0x80000000u)+vg1_pulse(phase*4u,step*4u,0x80000000u))/2;
        case VG1_W_SAW_IO:
            return (phase&0x80000000u) ? v1_sine_sample(phase*2u) : v1_saw_sample(phase*2u,step*2u);
        case VG1_W_SINE: return v1_sine_sample(phase);
        case VG1_W_PULSE_PWM: return vg1_pulse(phase,step,pw);
        case VG1_W_PULSE_DBL_PWM:
            return (vg1_pulse(phase,step,pw)+vg1_pulse(phase*2u,step*2u,pw))/2;
        case VG1_W_PULSE_TPL_PWM:
            return (vg1_pulse(phase,step,pw)+vg1_pulse(phase*2u,step*2u,pw)
                   +vg1_pulse(phase*3u,step*3u,pw))/3;
        case VG1_W_BI_PULSE:
            return (vg1_pulse(phase,step,0x40000000u)+vg1_pulse(phase,step,pw))/2;
        case VG1_W_FM_PULSE:
            ph2=phase+(uint32_t)(((int64_t)v1_sine_sample(phase)*((int32_t)(pw>>24)-64))*512);
            return vg1_pulse(ph2,step,0x80000000u);
        case VG1_W_PWM_SAW:
            return (saw+v1_saw_sample(phase+pw,step))/2;
        case VG1_W_BI_SAW:
            return (saw+v1_saw_sample(phase*2u,step*2u))/2;
        case VG1_W_SYNCED_SAW:
            ph2=(uint32_t)(((uint64_t)phase*(2u+(pw>>29)))&0xffffffffu);
            return v1_saw_sample(ph2,step*(2u+(pw>>29)));
        case VG1_W_AJX_SAW:
            return (saw+v1_saw_sample(phase+0x10000000u,step))/2;
        case VG1_W_AJX_SAW2:
            return (2*saw+v1_saw_sample(phase+0x30000000u,step))/3;
        case VG1_W_AJX_SAW3:
            return (saw+v1_saw_sample(phase+0x54000000u,step)
                       +v1_saw_sample(phase+0x9a000000u,step))/3;
        case VG1_W_TRI_SAW_MORPH:
            a=vg1_triangle(phase);b=(int32_t)(pw>>25);
            return (a*(127-b)+saw*b)/127;
        case VG1_W_SIN_PEAK_MORPH:
            a=v1_sine_sample(phase);b=(int32_t)(pw>>25);
            return (a*(127-b)+vg1_pulse(phase,step,0x05555555u)*b)/127;
        case VG1_W_SINE_FM:
            sine=v1_sine_sample(phase);
            return v1_sine_sample(phase+(uint32_t)((int64_t)sine*((int32_t)(pw>>25))*128));
        case VG1_W_SINE_AM:
            sine=v1_sine_sample(phase);
            return (sine*(32767-((int32_t)(pw>>25)*v1_sine_sample(phase*2u)>>7)))>>15;
        case VG1_W_R2D2:
            return v1_sine_sample(phase*(1u+(v1_rand(&o->rng)&3u)));
        case VG1_W_DIGITAL:
            return (saw+vg1_pulse(phase*3u,step*3u,pw))/2;
        case VG1_W_SYNCED_NOISE:
            if(phase<step)o->noise_prev=(int32_t)(v1_rand(&o->rng)>>16)-32768;
            return o->noise_prev;
        case VG1_W_C64_NOISE:
            if((phase>>24)!=(uint8_t)((phase-step)>>24))o->noise_prev=(int32_t)(v1_rand(&o->rng)>>16)-32768;
            return ((o->noise_prev>>8)<<8);
        case VG1_W_HIPASS_NOISE:
            a=(int32_t)(v1_rand(&o->rng)>>16)-32768;
            b=a-o->noise_prev;
            o->noise_prev=a;
            return v1_clip(b,-32767,32767);
        case VG1_W_WHITE_NOISE:
            return (int32_t)(v1_rand(&o->rng)>>16)-32768;
        default: return 0;
    }
}
#endif
