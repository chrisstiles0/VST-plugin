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

## First playable build

- **Oscillator and voices.** One oscillator with built-in band-limited wavetables, continuous frame position, and pitch controls. Interpolate between samples and frames. Support up to seven unison voices per note with detune and stereo spread, plus eight-note polyphony, MIDI velocity, predictable voice stealing, and an amp ADSR.
- **Filter and modulation.** One resonant low-pass filter per note and one free-running or note-triggered LFO with independent depths for wavetable position and cutoff.
- **Editor and patches.** One screen with oscillator, envelope, filter, LFO, and output controls. Provide Init and the three reference patches. Expose wavetable position, cutoff, resonance, LFO rate and depths, and output level to FL Studio automation. Save all sound settings independently for each instance.

**Later scope** · Additional oscillators and synthesis engines, custom wavetable import and editing, tempo sync, a modulation matrix, effects, sequencing, user preset files, and other hosts or platforms. Use FL Studio mixer effects for arrangement references that need them.

## Acceptance checks

- **Host and recall.** A Release build passes a verified scan as a generator. Two instances play separate Piano Roll patterns, route stereo to the mixer independently, export offline, and restore settings and automation after a project restart. Note-triggered patches reproduce the same audible result from the same project start.
- **Sound.** Compare the three patches against their reference phrases and try them in a short arrangement. At least one should be worth keeping in a real track. Check rapid automation, repeated notes, and voice stealing for clicks or stuck notes. Check aliasing with a bright-table pitch sweep.
- **Performance.** On the agreed machine, run eight notes at seven-way unison with modulation for 60 seconds at 48 kHz and 256 samples. Require zero underruns and record processing time. Exercise 44.1 and 48 kHz and buffers of 128, 256, and 512 samples where supported.

## Setup before implementation

1. Agree on the reference patches, test phrases, and sweep criteria. Record the Windows and FL Studio versions, CPU, driver, sample rate, and buffer.
2. Pin the toolchain, agree on the build and install steps, and confirm the distribution plan. [JUCE licence agreement](https://juce.com/legal/juce-8-licence/)
3. Sketch the editor and agree on control ranges, defaults, stable plug-in and parameter IDs, and versioned state.

Start implementation with a minimal VST3 that scans, plays MIDI, and recalls one parameter after a project restart.
