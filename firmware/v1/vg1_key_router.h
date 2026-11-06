/* SPDX-License-Identifier: GPL-3.0-only
 * VG-1 FM1 physical key-edge routing.
 * A note-off must always use the note that was transmitted at key-down,
 * even when the octave buttons have changed during a held chord.
 * Pure C, host-testable; no dependency on FM1 platform/interrupts.
 */
#ifndef VG1_KEY_ROUTER_H
#define VG1_KEY_ROUTER_H
#include <stdint.h>
#define VG1_KEY_COUNT 27u
#define VG1_KEY_MASK ((1u<<VG1_KEY_COUNT)-1u)
#define VG1_KEY_FIRST_MIDI 53u
typedef struct {
    uint32_t previous;
    uint8_t held_pitch[VG1_KEY_COUNT];
} vg1_key_router_t;
typedef struct {
    uint8_t on,note;
} vg1_key_event_t;
static inline uint32_t vg1_route_keys(vg1_key_router_t *state,
            uint32_t mask,uint32_t octave,
            vg1_key_event_t output[VG1_KEY_COUNT]) {
    uint32_t k,count=0;
    uint32_t previous=state->previous;
    mask&=VG1_KEY_MASK;
    for(k=0;k<VG1_KEY_COUNT;k++) {
        uint32_t bit=1u<<k;
        if((mask&bit) && !(previous&bit)) {
            int32_t pitch=(int32_t)VG1_KEY_FIRST_MIDI+(int32_t)k
                         +((int32_t)octave-2)*12;
            if(pitch<0)pitch=0;
            if(pitch>127)pitch=127;
            state->held_pitch[k]=(uint8_t)pitch;
            output[count].on=1;
            output[count++].note=(uint8_t)pitch;
        } else if(!(mask&bit) && (previous&bit)) {
            output[count].on=0;
            output[count++].note=state->held_pitch[k];
        }
    }
    state->previous=mask;
    return count;
}
#endif
