/* SPDX-License-Identifier: GPL-3.0-only */
/* Opera X v0.19: selectable, backwards-compatible effect routing.
 *
 * The FM1 saves an exact-size settings struct and exact-size pattern
 * snapshot. Adding bytes to either would make previously saved data fail
 * its length/CRC check. The DRIVE TYPE field (0..3) has six unused bits;
 * store global/pattern FX route in bits 2..3, leaving its original type
 * in bits 0..1. Older firmware already uses type % NDV (NDV == 4) and
 * thus still plays the same drive type if a v0.19 song is loaded.
 *
 * Mode 0: original parallel returns, bit-exact routing of v0.18.
 * Mode 1: delay wet fed into reverb, as well as the original direct sends.
 * Mode 2: reverb wet fed into ping-pong delay, retaining direct sends.
 *
 * This design is a wet cascade, not a full serial insert: dry synth,
 * chorus and each effect's return remain audible in every mode. This
 * avoids feedback loops and keeps independent per-track send controls.
 */
#ifndef OBV_X_FX_ROUTING_H
#define OBV_X_FX_ROUTING_H

enum { OX_FX_PARALLEL = 0, OX_FX_DELAY_REVERB = 1,
       OX_FX_REVERB_DELAY = 2, OX_FX_ROUTE_COUNT = 3 };

static inline uint8_t opera_x_fx_route(uint8_t packed_drive_type)
{
    uint8_t value = (packed_drive_type >> 2) & 3u;
    return value < OX_FX_ROUTE_COUNT ? value : OX_FX_PARALLEL;
}
static inline uint8_t opera_x_fx_drive_type(uint8_t packed_drive_type)
{
    return packed_drive_type & 3u;
}
static inline uint8_t opera_x_fx_with_route(uint8_t packed_drive_type, uint8_t route)
{
    if (route >= OX_FX_ROUTE_COUNT) route = OX_FX_PARALLEL;
    return (uint8_t)((packed_drive_type & 3u) | (route << 2));
}
static inline uint8_t opera_x_fx_with_drive(uint8_t packed_drive_type, uint8_t drive)
{
    return (uint8_t)((packed_drive_type & 12u) | (drive & 3u));
}
/* Feed the stereo effect's WET output to the next stage's mono input,
 * preserving the original direct send. Both wet streams are averaged,
 * then scaled by the FIRST effect's mix (Q8), and clamped for headroom.
 * Return amplitudes continue to use their normal separate mix knobs. */
static inline int32_t opera_x_fx_chain_feed(int32_t direct,
                                            int32_t wet_l, int32_t wet_r,
                                            int32_t mix_q8)
{
    int32_t middle = (wet_l + wet_r) >> 1;
    int32_t cascade = (middle * mix_q8) >> 8;
    int32_t combined = direct + cascade;
    return combined > 32767 ? 32767 : combined < -32767 ? -32767 : combined;
}

#endif /* OBV_X_FX_ROUTING_H */
