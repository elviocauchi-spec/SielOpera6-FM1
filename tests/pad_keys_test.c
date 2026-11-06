/* SPDX-License-Identifier: GPL-3.0-only
 * Deterministic host tests for exact, physically held-note arpeggiation.
 * No chord recognition: the user is always the pitch source. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../firmware/src/pad_keys.h"

static void key(uint32_t on, uint32_t note) { pad_keys_event(on, note); }
static void expect(uint32_t pos, uint32_t expected)
{
    uint32_t actual = 255u;
    assert(pad_keys_pick(pos, &actual));
    assert(actual == expected);
}

int main(void)
{
    uint32_t note = 255;
    assert(pad_keys_held() == 0u);
    assert(!pad_keys_pick(0, &note));

    /* A single note is a valid arp input; no triad gate. */
    key(1, 60);
    assert(pad_keys_held() == 1u);
    expect(0, 60); expect(1, 60); expect(8, 60);

    /* Duplicated note-on must not inflate the held-key count. */
    key(1, 60);
    assert(pad_keys_held() == 1u);

    /* The exact notes of an inverted / extended chord appear, sorted,
     * including all extensions: not snapped to a guessed harmony. */
    key(1, 67); key(1, 64); key(1, 71);
    assert(pad_keys_held() == 4u);
    expect(0, 60); expect(1, 64); expect(2, 67); expect(3, 71);
    expect(4, 60); expect(5, 64);

    key(0, 64);
    assert(pad_keys_held() == 3u);
    expect(0, 60); expect(1, 67); expect(2, 71); expect(3, 60);
    assert(!pad_keys_note_held(64));
    assert(pad_keys_note_held(67));
    assert(pad_keys.revision == 5u); /* first, three more, one release */

    /* Change arp octave ONLY by an explicit user control. */
    pad_arp_octave = 1;
    assert(pad_keys_note_held(72));
    assert(!pad_keys_note_held(60));
    pad_arp_octave = 0;

    /* Out-of-range and stray note-offs never invent a note. */
    key(1, 128); key(0, 124);
    assert(pad_keys_held() == 3u);
    key(0, 60); key(0, 67); key(0, 71);
    assert(pad_keys_held() == 0u);
    assert(!pad_keys_pick(0, &note));
    assert(!pad_keys_note_held(60));

    key(1, 54); key(1, 59); /* two keys alone can arpeggiate */
    expect(0, 54); expect(1, 59); expect(2, 54);
    key(0, 54); key(0, 59);

    puts("PAD LAB direct held-key arpeggiator: PASS");
    return 0;
}
