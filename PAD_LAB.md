# OPERA PAD LAB — direct held-note arpeggiator and ten-pad synth bank

Development branch `feature/pad-lab-chord-follow`. **Experimental source, not an FM-1 firmware release.** This branch leaves the existing OPERA firmware on `main` intact.

## Design intent
- **Track A is always the notes you play.** In PAD LAB audition mode, **track B arpeggiates the exact held MIDI notes** in ascending order. There is no guessed root, chord type, added fifth, replacement tone, or generated melody.
- A single key works; two-note chords, inversions, dense chords and suspended chords all work without special cases. The B trigger rhythm is fixed and reproducible.
- B uses four sustained triggers over two bars at default quarter-note SPEED, and wraps around the list of currently held keys: C-E-G -> C, E, G, C; D-F-A -> D, F, A, D.
- Releasing a key immediately releases B's active copy of that pitch. Releasing **all** keys disarms B, cancels pending B hits and fades the sound naturally through the envelope/effects.
- B runs only while PLAY transport is running and B ARP is ON. Press PLAY to stop the transport.
- **PRESETS keeps the backing running while browsing sounds on A.** Previously PRESETS exited the demo, which was why B disappeared on other presets. Browsing sounds in the preview does not save them or overwrite your existing song/program choice.
- **Ten contrasting PAD LAB sounds now occupy program numbers 71–80:** CELESTIAL (bright ensemble), NIGHTTIDE (dark sub-octave drone), AURORA (glassy band-pass), MIST FLOW (soft flowing texture), SILK STR (PWM strings), EMBERWAVE (brassy sweep), GLASS SEA (high-pass air), DEEP FIELD (sub/bass texture), VOWEL PAD (choir-like band-pass), ORBIT PWM (wide animated pulses). These are different oscillator/filter/envelope programs, not merely different display names.
- The PRESETS wheel skips the unsaved, identical **INIT** slots 81–99 (and the original factory INIT at #32); it still includes any slot where the user actually saved a program.
- **Important:** while PAD LAB audition is active, positions 71–80 use the new factory sounds *regardless of older user saves* in those positions, to ensure the intended preview. Leaving the preview restores normal saved-program priority; no stored patch is erased.
- The preview's B LEVEL changes ONLY the B mixer gain. It never applies a full mix reset, never changes A's gain/pan/sends and never writes the demo B setting into saved-song globals. Default B LEVEL is 112.
- The original stored patterns, mixer and patch programs remain available outside the demo. The old chord-classifier module and its tests were removed; **the audio and keyboard paths contain no chord recognition at all**.

## Performance controls
- **HOME / PAD:** TONE, MOTION, WIDTH, SPACE, as before, to shape A's live sound.
- **BACKING:** B ARP (on/off), B LEVEL (0-127), SPEED (division) and OCTAVE (explicit transposition of B, default 0 = played pitch).
- **PRESETS:** browse track A sounds while the B arpeggiator continues to play.
- **PLAY:** start/stop the backing rhythm.
- **SELECT button on BACKING:** toggle audition mode and return to the saved arrangement.
- **SEQ editor and ALGORITHM:** leave the audition to preserve existing independent track editing behavior.

## Build and safety
```sh
cc -std=c11 -O2 -Wall -Wextra -Werror tests/pad_keys_test.c -o /tmp/pad_keys_test
/tmp/pad_keys_test
cc -std=c11 -O2 -Wall -Wextra -Werror tests/pad_presets_test.c -o /tmp/pad_presets_test
/tmp/pad_presets_test
python3 tests/pad_lab_audition_test.py
# Target build is still ./build.sh with the AC79 SDK and JieLi compiler.
```

Do not flash a firmware package until CI builds it and it is tested for audio stability and correct flash layout on the intended FM-1 revision.

## External references
- AMB-1: https://github.com/charlesvestal/fm1-amb
- AISLING: https://github.com/stephencummins/fm1-aisling
Their approaches to spacious DSP and background sequences inspired this design. This implementation does not copy their chord-walk or automatic lead behavior. Retain GPL-3.0-only compliance for the existing OberVave-derived source.
