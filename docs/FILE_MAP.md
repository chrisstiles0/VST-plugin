# File map

| File | Responsibility | Location from repo root |
| --- | --- | --- |
| `.gitignore` | Ignore rules for the local experiment and generated output. | [`.gitignore`](../.gitignore) |
| `AGENTS.md` | Approval workflow, file routing, dependency policy, and code standards. | [`AGENTS.md`](../AGENTS.md) |
| `README.md` | Contributor orientation, toolchain, and build instructions. | [`README.md`](../README.md) |
| `FILE_MAP.md` | Maintained file names, responsibilities, and paths. | [`docs/FILE_MAP.md`](FILE_MAP.md) |
| `PRD.md` | Product goal, milestones, reference patches, and acceptance criteria. | [`docs/PRD.md`](PRD.md) |
| `SKILLS_AND_RESOURCES.md` | Task routing, skills, and technical references. | [`docs/SKILLS_AND_RESOURCES.md`](SKILLS_AND_RESOURCES.md) |
| `STYLE_GUIDE.md` | Editor typography, colours, geometry, control states, and visual checks. | [`docs/STYLE_GUIDE.md`](STYLE_GUIDE.md) |
| `USER_MANUAL.md` | Installation, sound design, automation, and audition phrases. | [`docs/USER_MANUAL.md`](USER_MANUAL.md) |
| `CMakeLists.txt` | VST3 target, identity, JUCE pin, and build configuration. | [`CMakeLists.txt`](../CMakeLists.txt) |
| `PluginProcessor.cpp` | VST3 host entry point, audio and MIDI I/O, and versioned instance state. | [`src/PluginProcessor.cpp`](../src/PluginProcessor.cpp) |
| `SynthParameters.h` | Shared parameter IDs, settings, and patch interface. | [`src/SynthParameters.h`](../src/SynthParameters.h) |
| `SynthParameters.cpp` | Parameter definitions, ranges, defaults, and factory patch values. | [`src/SynthParameters.cpp`](../src/SynthParameters.cpp) |
| `WavetableSynth.h` | DSP interface, voice storage, and editor modulation snapshot. | [`src/dsp/WavetableSynth.h`](../src/dsp/WavetableSynth.h) |
| `WavetableSynth.cpp` | Wavetable generation, MIDI voice allocation, rendering, filter, and modulation. | [`src/dsp/WavetableSynth.cpp`](../src/dsp/WavetableSynth.cpp) |
| `WavetableImport.h` | Imported table and audio thread handoff interfaces. | [`src/dsp/WavetableImport.h`](../src/dsp/WavetableImport.h) |
| `WavetableImport.cpp` | WAV validation, band-limited frames, project encoding, and table lifetime. | [`src/dsp/WavetableImport.cpp`](../src/dsp/WavetableImport.cpp) |
| `PluginEditor.h` | Editor creation interface. | [`src/ui/PluginEditor.h`](../src/ui/PluginEditor.h) |
| `PluginEditor.cpp` | Layout, look and feel, control bindings, and waveform and LFO displays. | [`src/ui/PluginEditor.cpp`](../src/ui/PluginEditor.cpp) |

## Local experiment and generated output

| File or directory | Responsibility | Location from repo root |
| --- | --- | --- |
| `README.md` | Three Osc Test build, installation, and FL Studio checks. | `test/README.md` |
| `CMakeLists.txt` | Three Osc Test VST3 target and its JUCE pin. | `test/CMakeLists.txt` |
| `PluginProcessor.cpp` | Three Osc Test synthesis, parameters, editor, and state. | `test/PluginProcessor.cpp` |
| `build/` | Generated projects, fetched dependencies, and VST3 build output. | `build/` for production and `test/build/` for the experiment |

The local experiment and build output are Git-ignored.
