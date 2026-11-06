/* SPDX-License-Identifier: GPL-3.0-only
 * VG-1: FM1 key and musical note release regression.
 * Tests octave-shifted physical key-ups and Gated Motion note-off decay,
 * using EXACT firmware oscillator, envelope, delay and reverb code.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../firmware/v1/vg1_key_router.h"
#include "../firmware/v1/vg1_factory.h"
#define CHECK(x,msg) do {if(!(x)){fprintf(stderr,"FAIL: %s\n",msg);return 1;}}while(0)
static v1_engine_t engine;
static int32_t audio[256*2];
static uint32_t render_seconds(uint32_t sec) {
    uint32_t frames=sec*V1_SR,largest=0;
    while(frames) {
        uint32_t n=frames>256?256:frames,i;
        memset(audio,0,sizeof audio);
        v1_engine_render(&engine,audio,n,128);
        for(i=0;i<2*n;i++) {
            uint32_t sample=(uint32_t)(audio[i]<0?-audio[i]:audio[i]);
            if(sample>largest)largest=sample;
        }
        frames-=n;
    }
    return largest;
}
static int active_voices(void) {
    uint32_t i,count=0;
    for(i=0;i<V1_VOICES;i++)if(engine.voices[i].active)count++;
    return (int)count;
}
int main(void) {
    vg1_key_router_t keys={0};
    vg1_key_event_t ev[VG1_KEY_COUNT];
    uint32_t n;
    n=vg1_route_keys(&keys,(1u<<0)|(1u<<4)|(1u<<7),2,ev);
    CHECK(n==3 && ev[0].on && ev[0].note==53 && ev[1].note==57 &&
          ev[2].note==60,"three physical notes at default octave");
    n=vg1_route_keys(&keys,(1u<<0)|(1u<<4)|(1u<<7),4,ev);
    CHECK(n==0,"changing octave while keys held must not retrigger");
    n=vg1_route_keys(&keys,(1u<<0)|(1u<<7),4,ev);
    CHECK(n==1 && !ev[0].on && ev[0].note==57,
          "released key must send original note pitch after octave change");
    n=vg1_route_keys(&keys,0,4,ev);
    CHECK(n==2 && !ev[0].on && ev[0].note==53 && !ev[1].on &&
          ev[1].note==60,"remaining shifted key-ups match original pitches");
    n=vg1_route_keys(&keys,1u<<0,4,ev);
    CHECK(n==1 && ev[0].on && ev[0].note==77,
          "new note on uses changed octave");
    n=vg1_route_keys(&keys,0,0,ev);
    CHECK(n==1 && !ev[0].on && ev[0].note==77,
          "release survives moving octave in opposite direction");
    puts("PASS: FM1 key-up matches original MIDI pitch through octave shifts");

    v1_engine_init(&engine,&V1_FACTORY[3]); /* GATED MOTION */
    v1_hold(&engine,1,60,106);
    v1_hold(&engine,1,64,100);
    v1_hold(&engine,1,67,98);
    uint32_t playing=render_seconds(1);
    CHECK(playing>500,"Gated Motion should be audible when keys down");
    v1_hold(&engine,0,60,0);
    v1_hold(&engine,0,64,0);
    v1_hold(&engine,0,67,0);
    CHECK(engine.held_count==0,"note-offs must remove all chord keys");
    CHECK(!engine.voices[0].gate,"C4 must be released");
    uint32_t first_tail=render_seconds(1);
    uint32_t second_tail=render_seconds(1);
    CHECK(active_voices()==0,"all voices must terminate after ENV1 RELEASE");
    uint32_t final_tail=render_seconds(3);
    CHECK(final_tail<25,
          "unheld Gated Motion must become inaudible; effects cannot sustain indefinitely");
    printf("PASS: Gated Motion note-off: playing_peak=%u, first_tail_peak=%u, second_tail_peak=%u, last_3s_peak=%u, active_voices=%d\n",
           playing,first_tail,second_tail,final_tail,active_voices());
    return 0;
}
