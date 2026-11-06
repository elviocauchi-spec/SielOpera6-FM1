/* SPDX-License-Identifier: GPL-3.0-only
 * Compare the exact firmware DSP cost on a CPU host (NOT AC79-cycle profiling).
 * Compile once with -DVG1_LEGACY_ENVELOPES -DVG1_LEGACY_FAT_DIVIDE and
 * once without. Runs 256-frame callbacks using the same patch/chords.
 */
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "../firmware/v1/vg1_factory.h"
static v1_engine_t eng;
static int32_t out[2*256u];
static const uint8_t keys[8]={60,64,67,72,74,76,77,79};
static volatile uint64_t sink;
static void bench(uint32_t preset,uint32_t voices) {
    const uint32_t iterations=8,frames_per_run=V1_SR;
    clock_t start=clock();
    uint64_t check=0;
    for(uint32_t r=0;r<iterations;r++) {
        v1_engine_init(&eng,&V1_FACTORY[preset]);
        for(uint32_t k=0;k<voices;k++)v1_hold(&eng,1,keys[k],100);
        for(uint32_t f=0;f<frames_per_run;f+=256u) {
            uint32_t n=frames_per_run-f;
            if(n>256u)n=256u;
            memset(out,0,sizeof out);
            v1_engine_render(&eng,out,n,128);
            /* Checksum and volatile sink ensure the compiler cannot omit audio. */
            for(uint32_t i=0;i<2u*n;i+=16u)
                check+=(uint32_t)out[i];
        }
    }
    clock_t stop=clock();
    sink^=check;
    const double ms=(double)(stop-start)*1000.0/CLOCKS_PER_SEC;
    printf("VG1 BENCH %-14s voices=%u FAT=%u audio_sec=%u host_cpu_ms=%.2f checksum=%llu\n",
           V1_FACTORY[preset].name,voices,vg1_fat_render_count(&V1_FACTORY[preset],voices),
           iterations,ms,(unsigned long long)check);
}
int main(void) {
#ifdef VG1_LEGACY_ENVELOPES
    puts("VG1 HOST BENCH: legacy per-sample envelope and variable FAT divide");
#else
    puts("VG1 HOST BENCH: optimized control-rate envelopes and constant FAT divide");
#endif
    for(uint32_t preset=0; preset<V1_PRESETS; preset+=3) {
        for(uint32_t voices=2;voices<=8;voices+=2)bench(preset,voices);
    }
    printf("Bench checksum=%llu\n",(unsigned long long)sink);
    return 0;
}
