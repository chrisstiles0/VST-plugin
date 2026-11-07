# User manual

The production wavetable synth is in planning. The sections below describe the intended first-release workflow. If you have the local **Three Osc Test** project, use `test/README.md` for its build and playback steps.

## Install and load in FL Studio

The production release will be a Windows x64 VST3 instrument. After installing the release bundle, open **Options → Manage plugins**, enable **Verify plugins**, and scan for installed plug-ins. Add the instrument to the Channel Rack, then draw or play notes in its Piano Roll. The install method and the final name will be documented with the first build. [Image-Line plug-in installation guide](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm)

## Shape a sound

| Feature | Planned workflow |
| --- | --- |
| Wavetable oscillator | Pick a built-in table and move its position control to blend between waveforms. |
| Unison | Increase the unison count, then use detune and spread to adjust pitch separation and stereo width. |
| Amp envelope | Use attack to set how quickly a note starts, decay and sustain to shape the held note, and release to set how long it fades after note-off. |
| Low-pass filter | Lower cutoff to darken the sound. Raise resonance to emphasise frequencies around the cutoff. |
| LFO | Choose free-running or note-triggered playback. Set the speed, then adjust how much the LFO moves wavetable position and cutoff independently. |
| Output | Set the instrument level while listening to both single notes and chords. |

## Start from a patch

Use **Init** as a starting point for sound design. The first release will also include a wide unison lead, a modulated bass, and an evolving pad.

## Automate controls and reopen a project

Use FL Studio automation for wavetable position, filter cutoff and resonance, LFO rate and depths, and output level. Save the FL Studio project to keep each instance's sound settings and automation with the arrangement.
