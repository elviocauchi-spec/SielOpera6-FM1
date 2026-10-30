/* SPDX-License-Identifier: GPL-3.0-only */
/* OPERA X v0.10: independent slow motion and stereo-width metadata.
 *
 * The 80-byte patch has 63 parameter bytes and 10 name bytes. Exactly four
 * unused bytes at 74..77 are reserved for this extension. Original SIEL
 * patches use 78..79 for oscillator footage: neither region is moved.
 * Older saved patches contain spaces at 74..77; they stay bit-compatible.
 */
#ifndef OBV_X_MOTION_H
#define OBV_X_MOTION_H

#define OX_MAGIC (PATCH_BYTES - 6u)       /* 74: feature marker */
#define OX_RATE  (PATCH_BYTES - 5u)       /* 75: motion speed (LFO table index) */
#define OX_TONE  (PATCH_BYTES - 4u)       /* 76: filter motion depth 0..127 */
#define OX_WIDTH (PATCH_BYTES - 3u)       /* 77: stereo side level 0..127 */
#define OX_TAG 0xB9u

static inline int opera_x_expressive(const uint8_t *v)
{
    return v[OX_MAGIC] == OX_TAG && v[OX_RATE] <= 127u
        && v[OX_TONE] <= 127u && v[OX_WIDTH] <= 127u;
}

/* Independent triangle LFO, unlike the legacy LFO which may be busy with
 * PWM or vibrato. Output: -32768..+32767 in fixed point. */
static inline int32_t opera_x_motion(uint32_t ph)
{
    return (ph & 0x80000000u)
        ? 32767 - (int32_t)((ph - 0x80000000u) >> 15)
        : (int32_t)(ph >> 15) - 32768;
}

/* 0.25x the original LFO rate; musical presets live in its slower range. */
static inline uint32_t opera_x_motion_inc(uint8_t rate)
{
    return LFO_INC[rate] >> 2;
}

/* At maximum depth 127, +/-11.9 semitones; smooth small swings around
 * a pad's cutoff use tone values in the 35..75 range. */
static inline int32_t opera_x_filter_delta(int32_t motion, uint8_t depth)
{
    return (int32_t)(((motion * (int32_t)depth) >> 15) * 24);
}

/* Stereo side signal is always zero when disabled. Width cannot cause
 * numerical wrap, and the oscillator's mono contribution stays untouched. */
static inline int32_t opera_x_width_gain(uint8_t width)
{
    return (int32_t)width * 1150 / 127;  /* Q12, max ~28% */
}
#endif /* OBV_X_MOTION_H */
