/* SPDX-License-Identifier: GPL-3.0-only
 * OPERA PAD LAB: deterministic, live-keyboard chord follower.
 * There is NO automatic chord walk and NO generated lead melody.
 * A fixed/edited sequencer pattern is voiced against the chord played on track A.
 * This header is deliberately independent of the platform for host testing. */
#ifndef OPERA_PAD_CHORD_H
#define OPERA_PAD_CHORD_H
#include <stdint.h>

typedef struct {
    uint8_t down[128];       /* physical/MIDI notes on track A (not the sequencer) */
    uint8_t root;            /* pitch class, 0=C .. 11=B */
    uint8_t third;           /* 2=sus2, 3=minor, 4=major, 5=sus4 */
    uint8_t valid;           /* holds last complete chord after notes are released */
    uint8_t count;
    uint32_t revision;
} pad_chord_t;

static pad_chord_t pad_chord;
static volatile uint32_t pad_follow_enabled = 1u;
static volatile int32_t pad_follow_octave = 1;    /* reference C4 -> played root in octave 4 */

static void pad_chord_event(uint32_t on, uint32_t note)
{
    uint8_t pc[12] = {0};
    int32_t n, count = 0, bass = 127, best = -1, root = 0, third = 4;
    static const uint8_t kinds[4] = {4, 3, 2, 5};  /* conventional triads win ties */
    if (note >= 128u) return;
    pad_chord.down[note] = on ? 1u : 0u;
    for (n = 0; n < 128; n++) {
        if (!pad_chord.down[n]) continue;
        count++;
        pc[n % 12] = 1;
        if (n < bass) bass = n;
    }
    pad_chord.count = (uint8_t)count;
    /* Do not jump to a false harmony while the player is still pressing a chord.
       No keys down: deliberately keep the most recent chord for the backing. */
    if (count < 3) return;
    for (n = 0; n < 12; n++) {
        int32_t k;
        if (!pc[n]) continue;
        for (k = 0; k < 4; k++) {
            int32_t th = kinds[k], score;
            if (!pc[(n + th) % 12] || !pc[(n + 7) % 12]) continue;
            score = 20 + (k < 2 ? 4 : 0) + ((bass % 12) == n ? 2 : 0);
            if (score > best) { best = score; root = n; third = th; }
        }
    }
    if (best >= 0 && (!pad_chord.valid || pad_chord.root != root || pad_chord.third != third)) {
        pad_chord.root = (uint8_t)root;
        pad_chord.third = (uint8_t)third;
        pad_chord.valid = 1u;
        pad_chord.revision++;
    }
}

/* Translate each C-major-reference step to a note of the player's current chord.
 * C=tonic, E=third, G=fifth, C'=octave. Other notes snap to nearest chord tone.
 * Preserve each step's octave, fixed rhythm, rests, velocity and gate.
 * This is CHORD FOLLOW, not an automatic melody composer. */
static uint32_t pad_chord_pitch(int32_t note)
{
    int32_t rel = note - 60, oct = rel >= 0 ? rel / 12 : -((-rel + 11) / 12);
    int32_t degree = rel - 12 * oct;
    int32_t interval = degree <= 2 ? 0 : degree <= 5 ? pad_chord.third : degree <= 9 ? 7 : 12;
    int32_t out = 48 + (int32_t)pad_chord.root + 12 * (oct + pad_follow_octave) + interval;
    if (out < 1) out = 1;
    if (out > 127) out = 127;
    return (uint32_t)out;
}

#endif /* OPERA_PAD_CHORD_H */
