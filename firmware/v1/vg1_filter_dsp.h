/* SPDX-License-Identifier: GPL-3.0-only
 * Vanguard 1.x filter topology prototypes. The 13 user-facing modes and their
 * broad behavior follow the 2006 manual; these are original approximations,
 * not claimed transfers of reFX's filters.
 *
 * Included after v1_svf(). 12/24 dB stages share a topology, while dual-band,
 * formant, and band-shaper modes use independent dedicated signal paths.
 */
#ifndef VG1_FILTER_DSP_H
#define VG1_FILTER_DSP_H
static inline int32_t vg1_filter_render(v1_voice_t *v,
                   const v1_svf_coef_t *c,const v1_svf_coef_t *c2,
                   int32_t sample,uint8_t filter_type) {
    int32_t a,b,lo,hi,bp,alpha;
    switch(filter_type) {
        case VG1_F_LOW6: case VG1_F_HIGH6:
            /* A proper one-pole section for first-order 6 dB/oct. */
            alpha=v1_clip(c->a2,0,4096);
            v->pole6+=((sample-v->pole6)*alpha)>>12;
            return filter_type==VG1_F_LOW6 ? v->pole6 : sample-v->pole6;
        case VG1_F_LOW12: return v1_svf(&v->f1,c,sample,V1_LP12);
        case VG1_F_LOW24:
            a=v1_svf(&v->f1,c,sample,V1_LP12);
            return v1_svf(&v->f2,c,a,V1_LP12);
        case VG1_F_HIGH24:
            a=v1_svf(&v->f1,c,sample,V1_HP24);
            return v1_svf(&v->f2,c,a,V1_HP24);
        case VG1_F_X:
            a=v1_svf(&v->f1,c,sample,V1_BP12);
            b=v1_svf(&v->f2,c2,sample,V1_BP12);
            return a-b;
        case VG1_F_BAND24:
            a=v1_svf(&v->f1,c,sample,V1_BP12);
            return v1_svf(&v->f2,c,a,V1_BP12);
        case VG1_F_FORMANT:
            a=v1_svf(&v->f1,c,sample,V1_BP12);
            b=v1_svf(&v->f2,c2,sample,V1_BP12);
            return (a+b)/2;
        case VG1_F_NOTCH:
            return v1_svf(&v->f1,c,sample,V1_NOTCH);
        case VG1_F_NOTCH_LOW:
            bp=v1_svf(&v->f1,c,sample,V1_BP12);
            lo=v1_svf(&v->f2,c,sample,V1_LP12);
            return lo-bp/2;
        case VG1_F_M:
            a=v1_svf(&v->f1,c,sample,V1_BP12);
            b=v1_svf(&v->f2,c2,sample,V1_BP12);
            return a+b;
        case VG1_F_BAND_SHP1:
            a=v1_svf(&v->f1,c,sample,V1_BP12);
            if(a>12000)return 12000+(a-12000)/4;
            if(a< -12000)return -12000+(a+12000)/4;
            return a;
        case VG1_F_BAND_SHP2:
            a=v1_svf(&v->f1,c,sample,V1_BP12);
            return v1_clip(a*3,-25000,25000);
        default: return v1_svf(&v->f1,c,sample,V1_LP12);
    }
}
#endif
