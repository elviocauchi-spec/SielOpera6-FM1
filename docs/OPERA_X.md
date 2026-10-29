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
