# OPERA X — musical synthesis branch (v0.9 sound lab)

This is a deliberately **experimental** branch for the M-VAVE FM1.
It forks the working `v08-factory-development` Opera 6 preview. Stable
`main` and the original Opera 6 factory sound bank **00–94** are not
modified. The objective is a useful, expressive synthesizer for pads,
strings, leads, basses, keys and evolving textures, not a checklist.

## First usable additions

Wave selection on OSC 1 and OSC 2 retains its original four values and
adds four more, so existing saved patches and all SIEL factory
oscillator codes retain their meaning.

| Wave choice | Meaning | Notes |
| --- | --- | --- |
| OFF / SAW / PULSE / SAW+PLS | Original engine | Same DSP path as v0.8.7 |
| SINE | Interpolated sine | Smooth basses, fundamentals and layered keys |
| TRI | Triangle | Mellow, harmonically gentle |
| SUP SAW | 3 independently phased polyBLEP saws | Wide pads and unison leads |
| SUP PLS | 3 independently phased polyBLEP pulses | Animated strings and vintage textures |

For the two SUPER waveforms, the OSC's existing **PW** control also
sets detune width, 0–127. The pulse version still uses pulse width
itself and can receive PWM. The detune range is bounded at about
+/-13.4 cents and calculated once per 16-sample control block.

## Four new playable sounds

The four formerly INIT-only programs now have default sounds, *unless
the user has previously saved a program in that slot* (saved programs
always win; we never erase flash):

- **95 X CLOUDPAD** — gentle wide saw plus triangle, slow release
- **96 X SILKSTR** — detuned pulse strings plus upper triangle
- **97 X HOTLEAD** — agile mono supersaw lead with glide and filter bite
- **98 X DEEPBASS** — fundamental sine plus a gritty, detuned pulse sub

Programs **00–89** are still the original packed SIEL factory records
translated by the existing preview converter; **90–94** are service /
calibration records. Their waveform codes still use 0–3.

The mixer effects are global return effects. Loading a new program
does not replace their currently running chorus/delay/reverb settings
or chop their reverb tails. To hear lush pads, set tasteful CHORUS and
REVERB returns and, if desired, adjust that track's sends.

## Performance / limitations

- This branch still allocates **six simultaneous synthesizer voices**,
  shared by four tracks; documentation from upstream mentioning eight
  is not the actual current `NV` constant.
- One SUP waveform needs three polyBLEP oscillators instead of one;
  using SUP on both oscillators of all six voices is the stress case.
  **No hardware performance guarantee exists yet.**
- DSP is fixed-point. Sine uses a small 256-entry interpolated table,
  triangle is computed, and detune increments are computed per block.
- The existing GLOBAL page displays `ui_render_us`, the measured
  time to render a half-buffer, with approximately **5800 microseconds**
  available. Measure under worst-case six-note chords with both SUP
  oscillators, resonant filter, all effects and active sequencer, on
  actual FM1 hardware. Aim for significant margin (<=70% peak; about
  4000 microseconds) before releasing for regular use.
- The host test covers signal range, oscillator codes and patch data;
  it cannot replace listening or timing measurements on real hardware.

## Next stages (subject to hardware profiling)

1. Refine these four sounds by ear; add velocity and aftertouch
   expression where input hardware supports it.
2. Add a second tempo-syncable modulation source with dedicated routings
   and keep modulation smooth at note edges.
3. Add spectral/wavetable scanning (band-limited tables or equivalent)
   with a cost-aware quality mode; develop elegant keys and plucks.
4. Improve stereo motion / diffusion, effects and sound library after
   measuring the CPU ceiling.
5. More presets, structured categories and sound browsing without
   displacing saved user programs or the verified Opera 6 bank.

Build and tests: `.github/workflows/opera-v08-preview.yml` on branch
`opera-x-development`; action artifact named
`OPERA-X-v0.9-SOUND-LAB-PREVIEW`.

## v0.10 expressive-stereo development

**Why:** user listening feedback described v0.9 as "ordinary".
The v0.9 wide waves were being summed to mono before the filter, so
there was little sense of width, and their LFO was already busy with
traditional pitch or PWM modulation.

v0.10 deliberately changes the four **95–98 X patches only**:
- Two decorrelated detuned side oscillators contribute a separate
  mid/side stereo component, with a gentle anti-harshness low-pass.
- A separate, deliberately slow triangle modulation sweeps the filter
  without occupying the original sound's LFO.
- The four patches have different depths, widths, oscillator balances,
  envelope timings, and performance intents.
- Stereo and modulation settings live in unused patch bytes 74–77
  behind marker 0xB9; original SIEL programs have no marker and retain
  their exact v0.9 sound. Original SIEL footage still uses bytes 78–79.
- The hardware I²S codec and USB path are interleaved left/right.
  Use headphones or stereo line capture to evaluate the added width;
  the built-in loudspeaker cannot reproduce stereo separation.

**Limitations:** The global chorus, delay and plate reverb are still
shared mixer effects and intentionally persist across program changes.
No X preset silently overwrites the user's global effect settings.
For spacious pads, turn up the CHORUS and REVERB returns on the FX pages
and give the playing track appropriate effect sends in MIXER.

**Please compare v0.9 vs v0.10:** 95 CLOUDPAD and 96 SILKSTR are the
most revealing tests. Hold a chord and listen to the slow changing
brightness and left/right independence. 97 HOTLEAD should remain
focused and forward; 98 DEEPBASS is deliberately mostly mono.

**Performance:** at full chord polyphony, active FX and two SUP
oscillators, check GLOBAL's audio render-time readout. The audio half
buffer is about 5800us; a prudent release threshold is a worst-case
peak of <=4000us on real FM1, with further stress testing before we
raise polyphony. Host unit tests and a successful build do not establish
hardware CPU margin or guarantee subjective sonic quality.

## v0.11 EVOLVE preview — first substantial sound-design corrections

Based on listening feedback, the prior patch motion was too slow and
each note was hard-panned to a different physical-card location.

### Audible and layered LFO motion

- A track's original LFO still has an editable RATE (LFO page) and
  FILTER / PWM / PITCH depths (LFO DEST page).
- CLOUDPAD now has a substantial ~0.65 Hz original LFO filter sweep
  as well as its separate 4-second expressive filter arc. Its earlier
  vibrato depth of 4 quantized to *zero* in the existing squared depth
  curve; the new nonzero depth is deliberate.
- SILKSTR has noticeable PWM and filter motion on the original LFO,
  plus an independent ~5-second filter arc.
- A second timbre modulation signal (2x the expressive LFO phase with
  an offset) animates the wide-wave detune and the harmonic balance.
  The filter, beating and oscillator layer are no longer all static.
- The slow motion is still stored in the existing v0.10 reserved patch
  extension bytes. No factory SIEL 00–94 programs change.
- On the four X factory programs, OX_RATE is specified in the patch
  internally (currently not a separately exposed panel control).
  To hear obvious manual modulation edits, use the traditional LFO
  and LFO DEST pages, especially FILTER, PWM and RATE.

### Chord stereo

The original voice-card pan spread deliberately distributed different
held notes left and right. This is inappropriate for wide pads.
The X POLY voice note pan is now centered; the independent detuned
oscillators make each note internally stereo. Legacy SIEL patches
keep their previous voice-card positioning.

### DRIVE signal-chain fixes

The original parallel DRIVE return used a gain around 4000 but shifted
by 15 bits, producing only 12% full-scale return at MIX 127. v0.11
uses a proper Q15 range (0–25500) for musically meaningful wet gain,
without bypassing the existing output limiter.

Also, DRIVE is a *send/return effect*, not a direct insert: previously
the mixer often had DRIVE SEND = 0, so changing the DRIVE page did
nothing. Loading an unsaved factory X preview 95–98 now selects
distinct per-track DRIVE sends (32, 38, 88, 75). This does not
change the shared effects chain. After loading a preset, MIXER's
DRIVE send can be adjusted by the player; loading the X factory
sound again restores its recommended send. Saved user patches
do not auto-replace mixer sends.

The DRIVE send, tone filter and return have independent LEFT and RIGHT
channels so a wide pad no longer collapses through the distortion
path. Global chorus/reverb remain shared effects; they must be enabled
separately if desired.

### Listening and profiling checklist

1. In stereo headphones, load 95 and play a four-note chord.
   Notes should no longer jump from ear to ear. Hold it for at
   least 8 seconds to hear filter, wavering detune and layer movement.
2. Compare 96 with 95, especially its more pronounced PWM flutter.
3. In the LFO page change RATE, then in LFO DEST increase FILTER or
   PWM: the changes should be audible. Avoid extreme pitch depths
   until recording and tuning have been checked.
4. For 97 or 98 open DRIVE: changing TYPE, AMOUNT and MIX should be
   obvious, not a subtle volume difference. Set MIXER DRIVE SEND
   higher/lower for more/less parallel saturation.
5. On real FM1 under a six-note sustained 95 pad plus chorus, reverb
   and drive, read GLOBAL's audio render time (half-buffer deadline
   roughly 5800us); target <=4000us with margin under sustained
   maximum load. Software tests cannot verify real hardware timing.

Still missing: a fully editable modulation matrix, elaborate
multi-layer wavetable synthesis and integrated per-preset ambience.
Those remain future engine milestones, not features of v0.11.

## v0.12 MORPH — spectral pad and vocal synth instead of two similar saw pads

**User feedback driving this iteration:** 95 stayed mostly static, 96
sounded too much like 95, and DRIVE finally worked. We keep v0.11's
working DRIVE correction and centered polyphonic stereo.

This is an actual new oscillator architecture, not an extra filter LFO:

- **EVOLVE** (wave 8): interpolated wavetable transitioning between a
  rich warm harmonic spectrum and a much brighter metallic/air spectrum.
- **VOX MORPH** (wave 9): different, independently synthesized table
  family, moving between two contrasting synthetic vowel/formant spectra.
- Each family has two endpoints and five different harmonic-band tables
  (maximum 32/16/8/4/2 partials), chosen by the note's fundamental
  frequency to limit aliasing. Tables are generated offline, occupying
  **10,280 bytes of static ROM**, no additional wavetable RAM.
- **MORPH** replaces the oscillator's PW label when either wavetable
  wave is selected. Its 0..127 value sets the center of the timbre.
  A separate incommensurate-speed modulation source scans around that
  center for the X factory sounds. This is a change in the harmonic
  content at the OSCILLATOR, even with a static open filter.
- A second, independently clocked (non-identical rate) LFO replaces
  the old precisely 2:1 correlated modulator. This means that spectral
  sweep, filter motion and auxiliary detuning do not repeat as one
  simple rhythmic loop.

### Rebuilt sound identities

| Program | Name | Main oscillator | Supporting oscillator |
| --- | --- | --- | --- |
| 95 | X AURORA | EVOLVE spectral morph | detuned supersaw for gentle stereo bloom |
| 96 | X VOXSCAPE | VOX MORPH synthetic vowels | higher superpulse, subtle breath |
| 97 | X HOTLEAD | supersaw | saw; unchanged core |
| 98 | X DEEPBASS | sine | superpulse; unchanged core |

These X programs are in formerly INIT-only factory slots 95–98.
Any previously saved user program in a slot will **override** the
new ROM factory sound. A user's older saved program will not suddenly
change waveform or become X AURORA, and the original 95 SIEL records
00–94 are still untouched. To audition an updated ROM sound, the slot
must not be occupied by saved user data.

### How to judge v0.12

Use headphones. On 95, hold a mid-register chord for 5–10 seconds;
the harmonic shimmer should move independently of the filter. In
the oscillator page, change MORPH from near 0 to near 127 and listen
for the harmonic content changing even with no effects. On 96, use a
similar chord: it should be a distinctly vocal/formant color and
not another wide saw pad. If you hear two mostly identical sounds,
verify the names displayed are **X AURORA** and **X VOXSCAPE**, and
check whether old user patches were saved into 95 or 96.

The DRIVE page works as in v0.11; the global chorus and reverb
remain shared and don't automatically reset on preset changes.

**Still unverified on hardware:** real render time for a six-note chord
with both wavetable oscillators, global effects and DRIVE; the FM1
audio half-buffer deadline is roughly 5800 microseconds. Aim for
<=4000 microseconds peak under stress. A successful firmware build
and waveform assertions do not establish beautiful sound by ear,
performance headroom or alias-free operation over all notes.

## v0.13 FULL-RANGE MORPH — correction of the 0–15% useful-range defect

User listening reports: AURORA's MORPH audibly changed mostly between
0 and about 15% of the knob; VOXSCAPE had only slightly more useful
travel. That is a failure of the instrument's expressiveness, even
though the v0.12 numerical endpoint tests passed.

v0.12 only interpolated between **two endpoints**. A linear waveform
interpolation does not guarantee perceptually uniform sonic changes.
Furthermore, the separate morph LFO applied up to +/-depth/2 to the
manual center, clipping at 0 and 127 for much of its movement.

v0.13 corrects **both** issues:

- MORPH has **five distinct spectral anchor stages**, occurring at
  display values approximately 0%, 25%, 50%, 75%, and 100%.
- AURORA: **WARM -> HOLLOW -> RICH -> ETCHED -> GLASS**.
- VOXSCAPE: **OO -> OH -> AH -> EH -> EE**.
- Each adjacent pair changes the harmonic structure, not merely the
  volume or the filter setting. Four interpolated transitions fill
  all of the physical 0–127 knob range.
- Five harmonic-cap tables per stage select an alias-reducing spectrum
  based on oscillator pitch, as before. The ROM table occupies
  **25,700 static bytes**, versus v0.12's 10,280 bytes. There is
  **no additional audio-rate oscillator, sample-rate conversion, or
  dynamically allocated RAM**.
- The independent morph LFO now always centers on the manual knob.
  Its maximum swing is limited by the nearest endpoint; the automatic
  LFO can never pin the effective position against the 0 or 127
  limits while the user adjusts MORPH.
- The host regression scans the waveform at **0, 16, 32, 48, 64,
  80, 96, 112, 127** for both families and several pitch bands. It also
  checks changes after the **actual FM1 12 dB low-pass filter**, so
  differences hidden by filtering cannot pass merely from waveform math.
  Each neighboring interval must change the generated waveform by
  a measurable amount. It also checks every manual control value
  remains monotonic under full positive and negative modulation.
- DRIVE's fix, six-voice structure, stereo chord imaging, original
  SIEL presets 00–94 and the other Opera X sounds are unchanged.

### Listening check

For a controlled A/B, **turn down chorus, reverb, and DRIVE returns**,
hold a single mid-register note, and sweep OSC 1 MORPH slowly
through 0%, 25%, 50%, 75%, and 100%. The actual oscillator tone
should evolve through **four clear steps of character**, not just
change at the beginning. Repeat with 96 VOXSCAPE.

Then restore your usual effects and hold a chord for at least 8
seconds. The separate LFOs should add slower motion around the
selected character; adjusting MORPH must remain expressive.

One user-reported GLOBAL figure for v0.12 was approximately
**2500 us on the online emulator** (about 43% of a 5800-us
half-buffer budget). That is encouraging but was not a controlled
worst-case hardware measurement. v0.13 adds 15,420 ROM bytes and
a small amount of waveform-selection work. Recheck GLOBAL under
maximum six-voice load and effects before considering this safe for
physical FM1 performance. Our desired worst-case CPU target on
hardware remains around <=4000us peak.

Neither waveform-distance regression nor a clean build guarantees
subjectively even morphing; user listening is decisive.

## Opera X v0.15 — live, editable LFO destinations

This builds on the v0.14 absolute-frequency two-formant VOX oscillator and
the already working v0.13 five-stage MORPH and v0.11 DRIVE return.
Two new editable routing depths appear on the existing **LFO+** page:

| Knob | Label | Destination | Range |
| --- | --- | --- | --- |
| 1 | PITCH> | Legacy oscillator pitch destination | Existing values |
| 2 | PWM> | Legacy oscillator PWM destination | Existing values |
| **3** | **ENV AMT** | LFO changes the **depth** of the filter envelope | 0–100% |
| **4** | **MORPH** | LFO moves spectral/VOX oscillator MORPH position | 0–100% |

**LFO+ is the third page when pressing LFO repeatedly.** The first LFO
page sets SHAPE, RATE, DELAY and PHASE. The second LFO DEST page still
sets PITCH, PWM, FILTER and AMP amounts. Both new destinations use that
same main LFO, including per-note phase spread and delay/fade.

The new ENV AMT modulation changes the signed amount by up to +/-24
of the existing ENV AMT control steps (roughly +/-36 semitones of
envelope influence at full envelope level). It does not directly change
attack, decay, sustain or release times. A note's filter-envelope ADSR
must be producing a nonzero signal for ENV AMT modulation to be audible.
The original FILTER destination still directly moves filter cutoff
without regard to envelope level.

The new MORPH routing works on OSC 1 or OSC 2 whenever its wave is
**EVOLVE** or **VOX MORPH**. It acts *in addition* to Opera X's earlier
automatic spectral motion, using the real editable LFO. Its movement
tapers at the exact 0 and 127 endpoints, so turning the manual MORPH
knob fully left or right always selects the true end spectrum.
At intermediate MORPH positions it can create clearly audible
timbre movement. For VOX MORPH, both wavetable content and the two
independent absolute-frequency formant filters follow the same LFO
movement. Formant coefficients are computed once per 16-frame control
block, never in the audio sample loop.

### Compatibility with earlier programs and user saves

**The sound parameter and patch file formats have not changed:**
NPARAM=63, a fixed 10-character name at bytes 63..72, and an 80-byte
saved patch. The new route marker is at byte 73. For marked Opera X
patches, route values use free bytes 78 and 79; for a vintage SIEL
patch, values use bytes 74 and 75, preserving the SIEL oscillator
footage marker and setting in 78 and 79. An older patch with no marker
has both destinations OFF, regardless of what is in spare bytes.
A first edit safely enables and initializes just the two route slots.

The stock factory preview defaults are:
- **95 X AURORA:** LFO ENV AMT 44/127, MORPH 66/127.
- **96 X VOXSCAPE:** LFO ENV AMT 30/127, MORPH 78/127.
- **97 X HOTLEAD:** LFO ENV AMT 18/127, MORPH OFF.
- **98 X DEEPBASS:** Both routes OFF.

**Saved user patches always take precedence** over new ROM factory
defaults. If you previously saved over 95 or 96, the firmware will not
overwrite the saved version. You can, however, turn up the new LFO+
destinations on that saved patch, then SAVE it again.

### Listening and performance check

1. Load the unsaved 95 preset and hold a chord. On LFO+ turn MORPH
   to 0%, then to 50–70%. Listen for changes in the harmonic spectrum.
2. On the LFO page increase RATE; the motion should become faster.
   Try changing the LFO SHAPE from triangle to square for an obvious
   rhythmic comparison.
3. On LFO+ increase ENV AMT from 0 to 70%; with the filter envelope
   active you should hear its evolving intensity change. LFO DEST
   FILTER still provides an independent direct filter sweep.
4. Load 96 and sweep MORPH on OSC 1 as well as LFO+ MORPH. Unlike a
   harmonic-only approximation, the two formant resonances in this
   version remain at constant frequencies across keyboard notes.
5. Finally hold six-note chords with DRIVE and FX enabled, and read
   the GLOBAL audio render time. The previous emulator reported
   approximately 2570 us; v0.15 includes additional per-voice filter
   work on VOX and must be benchmarked before claiming it is safe on
   the physical FM1.

These virtual LFO+ routes are currently editable through the FM1
panel and persist with SAVE; they are **not yet assignable as separate
MIDI CC parameters or sequencer automation lanes**. The original
parameter indices, MIDI CC mappings and factory SIEL conversion
remain intact.

## Opera X v0.16 — ENV AMT now has a meaningful sweep

**User feedback:** with v0.15 the new LFO -> ENV AMT modulation was
underwhelming, although GLOBAL displayed about **3200 us** on the
online emulator (previous v0.13 reported 2570 us). The latter is
about **55%** of our roughly 5800-us render deadline, but it is not
a hardware worst-case test and must not be interpreted as spare
polyphony without further measurement.

### Why it sounded weak

Aurora's factory FILTER CUTOFF was 101 and its ENV AMT was +19
(control value 83, offset from the bipolar center at 64).
The sustained filter envelope added roughly 23 semitones of cutoff,
bringing its operating position to around MIDI 132 after the engine's
additional +8-semitone offset, close to its 136-semitone coefficient
ceiling. Turning ENV AMT's LFO depth upward therefore spent much of
the cycle making an already-open filter even more open.
Voxscape had a similar problem with an initial cutoff of 110.

This is an interaction of envelope sustain, ENV AMT, filter cutoff
and coefficient ceiling, not a failure of the encoder or the LFO
signal itself.

### What was changed

- Aurora's FILTER CUTOFF defaults to **88**, Voxscape to **93**.
  This gives both pads enough brightness to remain recognizable,
  but room to move in both directions.
- The LFO+ **ENV AMT** destination remains labeled the same and is
  still a saved patch parameter 0..127 (0–100% display).
- Its response is now *musically asymmetric*: at maximum depth the
  LFO's positive peak adds up to **+12** ENV AMT units; the negative
  peak subtracts up to **56**. This deliberately favors a deep,
  breathing downward sweep over an upper-frequency peak that would
  otherwise hit the filter's cutoff ceiling.
- Crucially, **LFO = 0 means no change**. So note-on and LFO-delay
  stages never receive an unintended baseline cutoff shift; route
  depth zero is exactly compatible with the original filter equation.
- The modulation is *envelope gated*. When the filter envelope is
  zero it does nothing; this is not equivalent to LFO DEST FILTER
  (which moves cutoff directly). Raising the filter ENV sustain or
  changing its attack/decay alters the effect dramatically.
- Aurora's default route ENV AMT was raised 44 -> **63**;
  Voxscape's 30 -> **52**. The lead and bass settings remain.
- No additional DSP is run per audio sample: fixed-point multiplications
  and shifts are performed once per active voice/control block.
- New host regression tests simulate held sustained envelopes for both
  factory pads and confirm large cutoff movement after the actual
  12 dB filter, instead of testing only abstract depth values.

### Listening test

1. Load unsaved factory **95 X AURORA**. Open LFO+ (press LFO three
   times) and set **ENV AMT to zero**. Hold a chord for at least 5 s.
2. Turn ENV AMT through **30%, 60%, 100%**, without changing the master
   level. Listen for a more dramatic dark/bright breathing quality.
3. Make the modulation especially obvious: on the first LFO page,
   change SHAPE to **SQUARE** and RATE to around the middle. With
   ENV AMT at 70–100%, the tone should alternate between a bright,
   full envelope and a darker, weakened/inverted envelope.
4. Repeat on **96 X VOXSCAPE**. The formant resonances follow its
   separate editable MORPH destination; ENV AMT moves the ordinary
   synth filter, which can dramatically darken the vowel excitation.
5. Compare LFO DEST **FILTER**: this direct cutoff motion still works
   even with FILTER ENV sustain set to zero, whereas LFO+ ENV AMT does
   not. That's how to verify the two destinations are genuinely
   different.
6. Check GLOBAL under your actual six-voice/FX stress case. Target
   <=4000 us peak with a 5800-us budget; a single 3200-us reading
   doesn't establish safe margin on physical FM1 hardware.

### Compatibility

Original Opera 6 patches 00–94, their waveform/footage data, factory
names, saved program format, editable LFO+ route storage, the new
absolute-frequency vocal formant filter, MORPH and the working DRIVE
send/return are unchanged. Any saved user patch takes priority over
new ROM factory defaults 95 and 96. If you previously saved 95/96,
you can still raise ENV AMT manually, but the new factory cutoff
values will not be forced onto your custom patches.
