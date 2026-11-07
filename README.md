# VST wavetable synth

A Windows x64 VST3 instrument for FL Studio. The first playable milestone covers a wide unison lead, a modulated bass, and an evolving pad.

## Start here

- [AGENTS.md](AGENTS.md) covers the approval workflow, file routing, technology choices, and code standards.
- [File map](docs/FILE_MAP.md) lists each maintained file, its responsibilities, and its path.
- [Skills and resources](docs/SKILLS_AND_RESOURCES.md) points to the guidance and references relevant to each kind of task.
- [Product requirements](docs/PRD.md) defines the first build, setup decisions, and FL Studio acceptance checks.
- [Style guide](docs/STYLE_GUIDE.md) defines the plug-in editor's visual defaults.
- [User manual](docs/USER_MANUAL.md) documents player-facing features and FL Studio workflows.

## Project layout

The root contains contributor docs and `docs/` holds the product and player documentation. The production build will use a root `CMakeLists.txt` with code in `src/`. Keep the host entry point easy to find and split DSP and UI into `src/dsp/` and `src/ui/` when they need their own files.

The local `test/` folder contains the Three Osc Test experiment. It is Git-ignored and only available on machines where it was created or copied. Its own README contains build and FL Studio install steps.

## Build status

The production synth is in setup. Its proposed toolchain is C++, JUCE, CMake, MSVC, and the Windows SDK. The exact versions and reference FL Studio environment will be recorded here once agreed. Use `build/` for generated projects, fetched dependencies, and compiled output.
