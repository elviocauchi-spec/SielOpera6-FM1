# OPERA PAD LAB — live chord-following pad synthesizer

Development branch `feature/pad-lab-chord-follow`. **Experimental source, not an FM-1 firmware release.** This branch leaves the existing OPERA firmware on `main` intact.

## Design intent
- **You play every chord and every melody.** No autonomous lead, chord progression or generated melodic phrases.
- **Track A** is the live keyboard pad. The chord detector listens to the actual track-A keyboard/MIDI note-on and note-off events.
- **Track B** supplies a fixed, editable background ostinato. Its **rhythm stays fixed**, but its root/third/fifth/octave adapt to the latest complete chord played on A.
- A newly created unsaved pattern gets a sparse eight-step C-major-reference accompaniment in B; existing saved patterns are not rewritten.
- A new chord takes effect on the sequence's next triggered note. **Releasing the final keyboard key immediately disarms the backing**, sends note-offs to any sounding B voices, and prevents new B steps until another complete triad is played. The B amp envelope and shared reverb/delay fade naturally; the sequencer clock can continue while silent.
- Recognized: major, minor, sus2, sus4 **three-note triads**, including inversions. Extended/ambiguous harmonies need further testing. When one or two notes remain held during a legato chord transition, the previous harmony persists until the next full chord. When **all keys are released**, the harmony is cleared. One or two new notes alone will not restart the backing.
- Background sequence uses the existing step sequencer and engine. **No generative random pitch changes** are introduced.
- The global BACKING page controls follow ON/OFF, B volume, division and accompaniment octave. These first-stage controls are runtime settings; **they are not persisted** after a restart yet.

## First-stage screen/control map
- **HOME / PAD:** TONE (cutoff), MOTION (filter LFO amount), WIDTH (voice spread), SPACE (reverb mix).
- **GLO / BACKING:** FOLLOW, LEVEL B, SPEED, OCTAVE.
- **SEQ:** retains the established step editor and pattern operations.
- **ENV, LFO, FX, EDIT:** still open advanced sound-design pages.
- **ALGORITHM:** selects track A-D, **PRESETS:** changes its sound.
- **PLAY:** runs/stops the backing sequences. In PAD LAB demo mode, B is silent until a complete chord is played on A and stops triggering as soon as the last key is released, whether FOLLOW is ON or FIXED. Turning FOLLOW OFF outside the demo retains the legacy standalone sequencer behavior.
- Current mixer and other navigation behavior remain transitional; full UI consolidation is a subsequent milestone.

## C-major reference notes for the B sequencer
Record/edit a fixed sequence using C4 (root), E4 (third), G4 (fifth), C5 (octave), and rests. The follower maps these notes to the chord you actually play, keeping octaves, velocity, step rhythm, chance, ratchets and gates. Other pitches snap to the closest triad role. It does **not** transpose the melody you play on A.

## Build and safety
```sh
cc -std=c11 -Wall -Wextra -Werror tests/pad_chord_test.c -o /tmp/pad_chord_test
/tmp/pad_chord_test
# Target build is still ./build.sh with the AC79 SDK and JieLi compiler.
```

Do not flash a firmware package until CI builds it and it is tested for audio stability and correct flash layout on the intended FM-1 revision.

## External references
- AMB-1: https://github.com/charlesvestal/fm1-amb
- AISLING: https://github.com/stephencummins/fm1-aisling
Their approaches to spacious DSP and background sequences inspired this design. This implementation does not copy their chord-walk or automatic lead behavior. Retain GPL-3.0-only compliance for the existing OberVave-derived source.
