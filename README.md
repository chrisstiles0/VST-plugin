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

## Contributing

Check for an existing branch for the task before creating one. See the [Git workflow in AGENTS.md](AGENTS.md#git-workflow) for branch names, reuse, and pull requests.

## Build from source

The production synth is in setup. The steps below are the planned workflow once the root `CMakeLists.txt` and `src/` are available. If you have the local Three Osc Test experiment, follow `test/README.md` to build it today.

1. Install Git, CMake, and Visual Studio 2022 Build Tools with **Desktop development with C++**, the MSVC x64 tools, and a Windows SDK. The supported toolchain versions will be recorded here when the production build is set up. [Microsoft Build Tools components](https://learn.microsoft.com/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2022)
2. In **Developer PowerShell for VS 2022**, open the repository root and run the x64 Release build.

   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   cmake --build build --config Release --parallel
   ```

3. Find the complete `.vst3` bundle under `build/<target>_artefacts/Release/VST3/`. Close FL Studio and copy the bundle to `C:\Program Files\Common Files\VST3\`. Windows may prompt for administrator permission.
4. Open FL Studio and go to **Options → Manage plugins**. Enable **Verify plugins**, scan for installed plug-ins, then add the synth to the Channel Rack. [FL Studio plug-in installation guide](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm)
