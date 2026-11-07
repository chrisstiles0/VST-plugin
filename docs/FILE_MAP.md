# File map

Each row identifies a maintained file, what it owns, and its exact location relative to the repository root. Use the smallest set of files that covers the task. Prefer names that describe the feature or responsibility, such as `WavetableOscillator.cpp`.

| File | Responsibilities | Location from repo root |
| --- | --- | --- |
| `.gitignore` | Git ignore rules for the local experiment and production build output. | [`.gitignore`](../.gitignore) |
| `AGENTS.md` | Approval workflow, reading order, dependency policy, and code quality standards. | [`AGENTS.md`](../AGENTS.md) |
| `README.md` | Contributor orientation, project layout, and production build status. | [`README.md`](../README.md) |
| `FILE_MAP.md` | File names, responsibilities, exact paths, and planned source locations. | [`docs/FILE_MAP.md`](FILE_MAP.md) |
| `PRD.md` | Product goal, reference patches, first-build scope, setup decisions, and acceptance criteria. | [`docs/PRD.md`](PRD.md) |
| `SKILLS_AND_RESOURCES.md` | Task routing, when to load skills, and where to find technical references. | [`docs/SKILLS_AND_RESOURCES.md`](SKILLS_AND_RESOURCES.md) |
| `STYLE_GUIDE.md` | Editor fonts, colours, geometry, control states, and visual checks. | [`docs/STYLE_GUIDE.md`](STYLE_GUIDE.md) |
| `USER_MANUAL.md` | Player-facing installation, controls, workflow, and project recall. | [`docs/USER_MANUAL.md`](USER_MANUAL.md) |

## Planned production paths

| Responsibility | Planned location |
| --- | --- |
| VST3 build target, dependency pins, and bundle output | `CMakeLists.txt` |
| Host entry point, MIDI and audio I/O, parameters, and project state | `src/PluginProcessor.cpp` |
| Oscillators, voices, envelopes, filter, and modulation when they need separate files | `src/dsp/` |
| Editor layout, controls, and shared visual values when a custom editor is needed | `src/ui/` |

## Local experiment and generated output

| File or directory | Responsibilities | Location from repo root |
| --- | --- | --- |
| `README.md` | Three Osc Test build, installation, and FL Studio checks. | `test/README.md` |
| `CMakeLists.txt` | Three Osc Test VST3 target and its JUCE pin. | `test/CMakeLists.txt` |
| `PluginProcessor.cpp` | Three Osc Test synthesis, parameters, editor, and state. | `test/PluginProcessor.cpp` |
| `build/` | Generated projects, fetched dependencies, and VST3 build output. | `build/` for production and `test/build/` for the experiment |

The experiment is local and Git-ignored. Generated files are indexed by directory.
