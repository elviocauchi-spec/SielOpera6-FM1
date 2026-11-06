/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../firmware/src/pad_chord.h"
static void key(uint32_t on, uint32_t note) { pad_chord_event(on, note); }
int main(void)
{
    /* No invented chord or background melody before the player starts. */
    assert(!pad_chord.valid);
    key(1, 62); key(1, 65);
    assert(!pad_chord.valid);              /* partial chord must not trigger */
    key(1, 69);                           /* D minor */
    assert(pad_chord.valid && pad_chord.root == 2 && pad_chord.third == 3);
    assert(pad_chord_pitch(60) == 62);
    assert(pad_chord_pitch(64) == 65);
    assert(pad_chord_pitch(67) == 69);
    assert(pad_chord_pitch(72) == 74);
    key(0, 62); key(0, 65); key(0, 69); /* releasing keys keeps the harmony */
    assert(pad_chord.root == 2 && pad_chord.valid);
    key(1, 67); key(1, 71); key(1, 74); /* G major */
    assert(pad_chord.root == 7 && pad_chord.third == 4);
    assert(pad_chord_pitch(60) == 67);
    assert(pad_chord_pitch(64) == 71);
    assert(pad_chord_pitch(67) == 74);
    key(0, 67); key(0, 71); key(0, 74);
    key(1, 64); key(1, 67); key(1, 72); /* C major, first inversion */
    assert(pad_chord.root == 0 && pad_chord.third == 4);
    key(0, 64); key(0, 67); key(0, 72);
    key(1, 55); key(1, 57); key(1, 62); /* Gsus2: G A D, G in bass */
    assert(pad_chord.root == 7 && pad_chord.third == 2);
    assert(pad_chord_pitch(64) == 69);
    puts("PAD LAB chord follow: all tests passed");
    return 0;
}
