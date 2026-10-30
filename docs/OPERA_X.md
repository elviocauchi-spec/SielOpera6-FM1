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
