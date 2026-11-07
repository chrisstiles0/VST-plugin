# Wavetable Synth

A Windows x64 VST3 instrument for FL Studio. The first playable build includes a wide unison lead, a modulated bass, and an evolving pad.

## Start here

- [AGENTS.md](AGENTS.md) covers approvals, file routing, and code standards.
- [File map](docs/FILE_MAP.md) lists each maintained file and its responsibility.
- [Skills and resources](docs/SKILLS_AND_RESOURCES.md) points to project guidance and technical references.
- [Product requirements](docs/PRD.md) defines the milestones and FL Studio acceptance checks.
- [Style guide](docs/STYLE_GUIDE.md) defines the editor's visual defaults.
- [User manual](docs/USER_MANUAL.md) covers installation, sound design, and the first-build test phrases.

## Build from source

The Milestone 1 build targets Visual Studio 2022 Build Tools with MSVC v143 (19.44.35229), CMake 3.31.6, and Windows SDK 10.0.26100.0. The FL Studio test target is 21.0.3.3517 x64.

Install Git, CMake, and the Visual Studio **Desktop development with C++** workload with x64 tools and the Windows SDK. JUCE provides the VST3 wrapper, parameter state, MIDI and audio buffers, and editor components. CMake fetches the pinned JUCE 8.0.17-security release on the first configure. [Microsoft Build Tools components](https://learn.microsoft.com/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2022) · [JUCE 8 licence](https://juce.com/legal/juce-8-licence/)

In Developer PowerShell for VS 2022, run these commands from the repository root.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target WavetableSynth_VST3 --parallel
```

The complete bundle is generated at `build/WavetableSynth_artefacts/Release/VST3/Wavetable Synth.vst3`. Follow the [installation steps](docs/USER_MANUAL.md#install-and-load-in-fl-studio).

The optional, Git-ignored `test/` directory contains the separate Three Osc Test experiment. Its README covers that build.
