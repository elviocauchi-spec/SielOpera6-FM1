/* SPDX-License-Identifier: GPL-3.0-only
 * OPERA PAD LAB: direct held-note arpeggiator input, without chord guessing.
 * No note is composed or harmonically reinterpreted. The main loop publishes
 * two alternating snapshots, and the audio ISR reads the latest complete one.
 * This keeps the code platform-independent and host-testable. */
#ifndef OPERA_PAD_KEYS_H
#define OPERA_PAD_KEYS_H
#include <stdint.h>

typedef struct { uint32_t bits[4], count; } pad_keys_frame_t;
typedef struct {
    uint8_t down[128];  /* currently held physical/MIDI notes on live track A */
    uint32_t count, revision;
} pad_keys_t;

static pad_keys_t pad_keys;
static pad_keys_frame_t pad_keys_frames[2];
static volatile uint32_t pad_keys_slot;
static volatile uint32_t pad_arp_enabled = 1u;
static volatile int32_t pad_arp_octave;  /* zero means EXACT played MIDI register */

/* The ISR and note events execute on the same processor; the ISR may interrupt
 * a writer, but the writer never interrupts an ISR. Publishing the index last
 * therefore makes each reader see a whole immutable snapshot. */
#ifndef PAD_KEYS_PUBLISH
#define PAD_KEYS_PUBLISH() __asm__ volatile("" ::: "memory")
#endif

static void pad_keys_event(uint32_t on, uint32_t note)
{
    uint32_t i, old, next, word, bit;
    uint8_t pressed = on ? 1u : 0u;
    if (note >= 128u || pad_keys.down[note] == pressed) return;
    pad_keys.down[note] = pressed;
    pad_keys.count = pressed ? pad_keys.count + 1u : pad_keys.count - 1u;
    old = pad_keys_slot;
    next = old ^ 1u;
    for (i = 0; i < 4u; i++) pad_keys_frames[next].bits[i] = pad_keys_frames[old].bits[i];
    word = note >> 5;
    bit = 1u << (note & 31u);
    if (pressed) pad_keys_frames[next].bits[word] |= bit;
    else pad_keys_frames[next].bits[word] &= ~bit;
    pad_keys_frames[next].count = pad_keys.count;
    PAD_KEYS_PUBLISH();
    pad_keys_slot = next;
    pad_keys.revision++;
}

static uint32_t pad_keys_held(void)
{
    return pad_keys_frames[pad_keys_slot].count;
}

/* Sorted by actual pitch: arpeggiates even 1, 2, 3, 4+ key chords. No root
 * selection, scale detection, generated fifth, octave doubling or note snap. */
static uint32_t pad_keys_pick(uint32_t position, uint32_t *note)
{
    const pad_keys_frame_t *f = &pad_keys_frames[pad_keys_slot];
    uint32_t n, seen = 0, target;
    if (!f->count) return 0u;
    target = position % f->count;
    for (n = 0; n < 128u; n++) {
        if (!(f->bits[n >> 5] & (1u << (n & 31u)))) continue;
        if (seen++ == target) { *note = n; return 1u; }
    }
    return 0u;
}

/* Check the key represented by a sequenced MIDI note. In demo mode the OCTAVE
 * setting is an explicit user-controlled transposition, not chord analysis. */
static uint32_t pad_keys_note_held(uint32_t midi_note)
{
    const pad_keys_frame_t *f = &pad_keys_frames[pad_keys_slot];
    int32_t physical = (int32_t)midi_note - pad_arp_octave * 12;
    if (physical < 0 || physical >= 128) return 0u;
    return (f->bits[(uint32_t)physical >> 5] >> (physical & 31)) & 1u;
}

#endif /* OPERA_PAD_KEYS_H */
