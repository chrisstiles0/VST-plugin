# Skills and resources

Use this index with [FILE_MAP.md](FILE_MAP.md) when a request needs project context. For a self-contained question, answer from the information already supplied. For project work, choose the matching row and open only the files needed to answer or verify the request.

| Situation | Read or use |
| --- | --- |
| Scope, priorities, or acceptance criteria | [PRD](PRD.md). Read the relevant section. |
| Build, dependency, or compiler question | The relevant `CMakeLists.txt` from the file map. Use the contributor README for local setup and the upstream JUCE docs when an API or build option needs verification. |
| DSP, MIDI, automation, or saved state | The owning source file from the file map. Read the PRD when expected behavior needs clarification. |
| Editor design or interaction | [Style guide](STYLE_GUIDE.md) and the owning UI source when it exists. Open the manual if the player's workflow changes. |
| How to use a production feature | [User manual](USER_MANUAL.md). Use the relevant section. |
| Three Osc Test build or playback | `test/README.md` and the specific test source or build file if needed. |
| Requested Windows app or FL Studio operation | Use the **computer-use** skill when direct app interaction is needed and available. For instructions alone, use the manual or Image-Line's documentation. |
| Requested Codex or OpenAI setup help | Use **openai-docs** for that topic. |

Load an explicitly named skill or one that materially helps the current task. Resolve its location from the active skills catalog and read its instructions when needed. Keep the index focused on skills relevant to this project.

## Upstream references

- JUCE target options, dependency integration, and VST3 bundle generation. [JUCE CMake API](https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md)
- Windows compiler and SDK setup. [Visual Studio Build Tools components](https://learn.microsoft.com/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2022)
- FL Studio plug-in locations, verified scans, and generator discovery. [Image-Line plug-in installation guide](https://www.image-line.com/fl-studio-learning/fl-studio-online-manual/html/basics_externalplugins.htm)
