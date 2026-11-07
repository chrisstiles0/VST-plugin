# PRD · FL Studio wavetable synth

**Draft** · Windows x64 VST3 for FL Studio. Proposed stack is C++, JUCE, CMake, and MSVC.

## Goal

Shape and automate sounds in one editor, then reopen the FL Studio project with each instance's sound intact. The first playable build should cover three patches that work together in a short arrangement.

| Patch | Sound target |
| --- | --- |
| Wide unison lead | Clear center pitch with audible detune and stereo width. |
| Modulated bass | Repeatable attack, steady low end, and a timbre sweep on held notes. |
| Evolving pad | Timbre changes over a sustained chord with a smooth release. |

Choose references from any instrument or recording. Capture one MIDI phrase and tempo per patch, a dry render where available, and any effects essential to the finished sound.

## Milestone 1 · First playable build

- **Oscillator and voices.** One oscillator with built-in band-limited wavetables, continuous frame position, and pitch controls. Interpolate between samples and frames. Support up to seven unison voices per note with detune and stereo spread, plus eight-note polyphony, MIDI velocity, predictable voice stealing, and an amp ADSR.
- **Filter and modulation.** One resonant low-pass filter per note and one free-running or note-triggered LFO with independent depths for wavetable position and cutoff.
- **Editor and patches.** One screen with oscillator, envelope, filter, LFO, and output controls. Provide Init and the three reference patches. Expose wavetable position, cutoff, resonance, LFO rate and depths, and output level to FL Studio automation. Save all sound settings independently for each instance.

Use FL Studio mixer effects for arrangement references that need them.

### Acceptance checks

- **Host and recall.** A Release build passes a verified scan as a generator. Two instances play separate Piano Roll patterns, route stereo to the mixer independently, export offline, and restore settings and automation after a project restart. Note-triggered patches reproduce the same audible result from the same project start.
- **Sound.** Compare the three patches against their reference phrases and try them in a short arrangement. At least one should be worth keeping in a real track. Check rapid automation, repeated notes, and voice stealing for clicks or stuck notes. Check aliasing with a bright-table pitch sweep.
- **Performance.** On the agreed machine, run eight notes at seven-way unison with modulation for 60 seconds at 48 kHz and 256 samples. Require zero underruns and record processing time. Exercise 44.1 and 48 kHz and buffers of 128, 256, and 512 samples where supported.

### Gate for Milestone 2

Milestone 2 implementation starts only after all Milestone 1 checks pass on the agreed FL Studio setup, every known defect affecting Milestone 1 functionality or usability is fixed and retested, and Chris explicitly signs off. For sign-off, use all three patches in a short arrangement and verify sound shaping, automation, usable output levels on single notes and chords, offline export, and project recall. Record the test results and sign-off.

Chris approved the start of Milestone 2 on 7 November 2026. The Milestone 1 Release build passed VST3 playback, automation parameter, state, offline rendering, and unison level checks in the test host. FL Studio project reopen and native context-menu automation were still unverified at handoff.

## Milestone 2 · Expanded sound design

- **Layered sound.** Add a second wavetable oscillator with independent tuning and level so it can provide a separate layer or clean sub. Support importing custom wavetable files.
- **Modulation and effects.** Add tempo-synced LFOs, editable modulation routing, and an onboard drive and delay chain.
- **Presets.** Save and load native user presets across projects. Add factory patches that demonstrate the new controls.

### Acceptance checks

- Build a layered patch with an imported wavetable. Verify that each oscillator can be tuned and levelled independently and that drive and delay can be adjusted and bypassed.
- Route a synced LFO to a supported destination. Change the FL Studio project tempo and verify that the modulation follows it.
- Save a native preset and load it in another project. Verify oscillator settings, the custom wavetable, modulation routes, and effects are restored.
- Repeat the Milestone 1 checks, including reopening a project saved with a Milestone 1 build.

**Beyond Milestone 2** · Additional synthesis engines, wavetable editing, sequencing, and support for other hosts or platforms.

## Setup before implementation

1. Agree on the reference patches, test phrases, and sweep criteria. Record the Windows and FL Studio versions, CPU, driver, sample rate, and buffer.
2. Pin the toolchain, agree on the build and install steps, and confirm the distribution plan. [JUCE licence agreement](https://juce.com/legal/juce-8-licence/)
3. Sketch the editor and agree on control ranges, defaults, stable plug-in and parameter IDs, and versioned state.

Start implementation with a minimal VST3 that scans, plays MIDI, and recalls one parameter after a project restart.
