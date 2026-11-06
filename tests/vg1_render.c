/* SPDX-License-Identifier: GPL-3.0-only
 * VG-1 offline renderer. Compiles the exact firmware DSP + factory programs
 * and writes 44.1kHz/16-bit/stereo PCM WAV files for critical listening.
 *
 * Usage: gcc -O2 -std=c11 -Wall -Wextra tests/vg1_render.c -o /tmp/vg1_render
 *        /tmp/vg1_render build/vg1-audio
 *
 * No reFX binaries, preset banks or wavetable assets are included.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "../firmware/v1/vg1_factory.h"

#define BLOCK 256u
#define PRESET_SECONDS 3u
static v1_engine_t engine;
static int32_t pcm[BLOCK*2u];
static uint64_t total_output_frames;
static uint64_t total_output_wavs;

static void le16(uint8_t *b,uint32_t v) {
    b[0]=(uint8_t)v;b[1]=(uint8_t)(v>>8);
}
static void le32(uint8_t *b,uint32_t v) {
    le16(b,v);le16(b+2,v>>16);
}
static int mkdir_if_needed(const char *s) {
    if(mkdir(s,0755)==0||errno==EEXIST)return 0;
    perror(s);return -1;
}
static void slug(char *dst,size_t cap,const char *src) {
    size_t i=0;
    while(*src&&i+1u<cap) {
        char c=*src++;
        if(c>='a'&&c<='z')c-='a'-'A';
        if(c==' '||c=='/'||c=='+')c='_';
        if((c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-')
            dst[i++]=c;
    }
    dst[i]=0;
}
static void place_note(uint8_t note,uint8_t velocity,uint8_t on) {
    v1_hold(&engine,on,note,velocity);
}
/* Render a fixed-duration note and, for ARP programs, a 3-note chord. */
static int write_wav(const char *path,const v1_patch_t *patch,uint32_t frames,
                     uint8_t note,uint32_t hold_frames,int chord) {
    uint8_t hdr[44]={0};
    uint32_t written=0;
    FILE *f=fopen(path,"wb");
    if(!f) {perror(path);return -1;}
    memcpy(hdr,"RIFF",4);le32(hdr+4,36+frames*4u);
    memcpy(hdr+8,"WAVEfmt ",8);le32(hdr+16,16u);le16(hdr+20,1u);
    le16(hdr+22,2u);le32(hdr+24,V1_SR);
    le32(hdr+28,V1_SR*4u);le16(hdr+32,4u);le16(hdr+34,16u);
    memcpy(hdr+36,"data",4);le32(hdr+40,frames*4u);
    if(fwrite(hdr,1,44,f)!=44){fclose(f);return -1;}
    v1_engine_init(&engine,patch);
    place_note(note,106,1);
    if(chord){place_note((uint8_t)(note+4u),100,1);place_note((uint8_t)(note+7u),98,1);}
    while(written<frames) {
        uint32_t n=frames-written;
        if(n>BLOCK)n=BLOCK;
        if(written<hold_frames && written+n>hold_frames)n=hold_frames-written;
        if(written==hold_frames) {
            place_note(note,0,0);
            if(chord){place_note((uint8_t)(note+4u),0,0);place_note((uint8_t)(note+7u),0,0);}
        }
        memset(pcm,0,sizeof pcm);
        v1_engine_render(&engine,pcm,n,128u);
        uint8_t bytes[BLOCK*4u];
        uint32_t i;
        for(i=0;i<n*2u;i++) {
            int32_t x=pcm[i];
            if(x>32767)x=32767;
            if(x<-32768)x=-32768;
            le16(bytes+i*2u,(uint16_t)(int16_t)x);
        }
        if(fwrite(bytes,4u,n,f)!=n){fclose(f);return -1;}
        written+=n;
    }
    fclose(f);
    total_output_frames+=frames;
    total_output_wavs++;
    printf("rendered %-40s frames=%u\\n",path,frames);
    return 0;
}
static v1_patch_t diagnostic_patch(uint32_t type) {
    v1_patch_t p=V1_FACTORY[0];
    uint32_t j;
    for(j=0;j<3u;j++) {
        p.osc[j].wave=VG1_W_OFF;
        p.osc[j].level=0;
        p.osc[j].pw=64;
        p.vg_lfo[j].speed=0;
        p.vg_lfo[j].detune=0;
        p.vg_lfo[j].pwm=0;
        p.vg_lfo[j].cutoff=0;
    }
    p.osc[0].wave=(uint8_t)type;
    p.osc[0].level=120;
    p.osc[0].semi=0;
    p.vg_osc.retrig=1;p.vg_osc.fat=0;p.vg_osc.age=0;
    p.vg_env1.level=0;p.vg_env1.pwm=0;p.vg_env1.cutoff=0;p.vg_env1.detune=0;
    p.vg_env2.resonance=0;p.vg_env2.pwm=0;p.vg_env2.cutoff=0;p.vg_env2.pitch=0;
    p.amp.attack=0;p.amp.decay=0;p.amp.sustain=127;p.amp.release=5;
    p.filter_env.attack=0;p.filter_env.decay=0;p.filter_env.sustain=127;
    p.filter_type=VG1_F_LOW6;p.cutoff=125;p.resonance=0;
    p.vg_keytrack.keytrk=0;p.vg_keytrack.veltrk=0;
    p.vg_amp.level=112;p.vg_amp.drive=0;p.vg_amp.pan=64;p.vg_amp.spread=0;
    p.vg_arp.on=0;p.vg_glide.on=0;p.vg_gate.enabled=0;
    p.vg_delay.on=0;p.vg_reverb.on=0;
    p.bend_range=0;p.vib_depth=0;
    p.master=112;p.voice_limit=1;
    return p;
}
static int render_all(const char *base) {
    char folder[512],path[640],sname[64];
    uint32_t i;
    if(mkdir_if_needed(base))return -1;
    snprintf(folder,sizeof(folder),"%s/presets",base);
    if(mkdir_if_needed(folder))return -1;
    for(i=0;i<V1_PRESETS;i++) {
        const v1_patch_t *p=&V1_FACTORY[i];
        uint8_t note=(i==1u||i==4u)?43u:(i==2u||i==7u)?72u:60u;
        int chord=(i==3u||i==5u||i==6u);
        slug(sname,sizeof sname,p->name);
        snprintf(path,sizeof(path),"%s/%02u_%s.wav",folder,i+1u,sname);
        if(write_wav(path,p,PRESET_SECONDS*V1_SR,note,2u*V1_SR,chord))return -1;
    }
    snprintf(folder,sizeof(folder),"%s/oscillators",base);
    if(mkdir_if_needed(folder))return -1;
    for(i=1u;i<VG1_N_WAVES;i++) {
        v1_patch_t p=diagnostic_patch(i);
        slug(sname,sizeof sname,vg1_wave_name[i]);
        snprintf(path,sizeof path,"%s/%02u_%s_C4.wav",folder,i,sname);
        if(write_wav(path,&p,V1_SR/2u,60u,V1_SR*4u/10u,0))return -1;
    }
    printf("VG-1 audio renders complete: %llu WAVs, %llu frames (44.1kHz stereo)\\n",
            (unsigned long long)total_output_wavs,(unsigned long long)total_output_frames);
    return 0;
}
int main(int argc,char **argv) {
    if(argc!=2) {fprintf(stderr,"Usage: %s output-directory\\n",argv[0]);return 2;}
    return render_all(argv[1])?1:0;
}
