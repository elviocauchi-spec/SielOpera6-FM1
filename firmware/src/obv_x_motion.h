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

/* Extract and gently low-pass the stereo side component. These are pure
 * fixed-point operations, and can be tested with the host compiler. */
static inline int32_t opera_x_side_pair(int32_t left, int32_t right, int32_t level)
{
    return ((left - right) * level) >> 8; /* assumes side oscillator <= +/-32768 */
}
static inline int32_t opera_x_side_smooth(int32_t previous, int32_t next)
{
    return previous + ((next - previous) >> 3);
}

/* A second, harmonically related but phase-offset motion source drives
 * oscillator balance and beating, independently of the filter's slow arc.
 * No new oscillator or audio-rate DSP: evaluated once per 16 frames. */
static inline uint32_t opera_x_timbre_inc(uint8_t rate)
{
    /* Pitch-independent, different frequency from the filter modulation.
     * Interval 18 in the exponential LFO table gives a non-integer ratio. */
    uint32_t fast = (uint32_t)rate + 18u;
    return opera_x_motion_inc((uint8_t)(fast > 127u ? 127u : fast));
}
static inline int32_t opera_x_timbre_motion(uint32_t independent_phase)
{
    return opera_x_motion(independent_phase + 0x60000000u);
}
/* Manual MORPH must remain useful across all 0..127, even while a
 * strong LFO is moving the waveform. The previous centered +/-depth/2
 * modulation pushed lower and upper settings against hard 0/127 clamps,
 * making large regions of knob travel insensitive.
 *
 * Here motion is a controlled fraction of the available headroom around
 * the *manually chosen* center. The knob always reaches both exact end
 * spectra; modulation goes gracefully to zero at the extremes.
 * Evaluated per 16-frame control block, not per audio sample. */
static inline uint8_t opera_x_wt_morph(uint8_t base, int32_t timbre, int32_t slow, uint8_t depth)
{
    int32_t movement = (timbre * 3 + slow) >> 2;  /* -32768..32767 */
    int32_t center = base > 127u ? 127 : (int32_t)base;
    int32_t room = center < 127 - center ? center : 127 - center;
    int32_t span = (room * depth * 3) / (127 * 4); /* <=75% of headroom */
    int32_t value = center + ((movement * span) >> 15);
    return (uint8_t)(value < 0 ? 0 : value > 127 ? 127 : value);
}
/* Detune width changes by up to +/-depth/4, never leaving 0..127. */
static inline uint8_t opera_x_animated_detune(uint8_t base, int32_t motion, uint8_t depth)
{
    int32_t d = (motion * (int32_t)depth) >> 17;
    int32_t w = (int32_t)base + d;
    return (uint8_t)(w < 0 ? 0 : w > 127 ? 127 : w);
}
/* A companion harmonic layer swells +/- depth/256 of its own level.
 * It does not pump the entire note, and it never makes the layer negative. */
static inline int32_t opera_x_animated_level(int32_t base, int32_t motion, uint8_t depth)
{
    int32_t delta = (motion * (int32_t)depth) >> 15;
    return (base * (256 + delta)) >> 8;
}
/* A chord's notes remain centered. Opera X creates width within each
 * voice; only other patches retain the original card-position pan scheme. */
static inline int32_t opera_x_note_pan(const uint8_t *patch, int32_t requested)
{
    return opera_x_expressive(patch) && patch[P_MODE] == M_POLY ? 0 : requested;
}
#endif /* OBV_X_MOTION_H */
