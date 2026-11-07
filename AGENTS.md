# Project instructions

## Approval before work

Before starting any new task, including small edits, research, setup, and development, tell Chris what you understand the task to be, list your assumptions, and describe the actions and verification you propose. Wait for explicit approval before starting. If there are no assumptions, say so. If the scope changes or a new assumption would change the result, explain the change and wait for approval on that part. Routine steps within an approved scope share that approval.

## Git workflow

Use `main` as the integration branch. Keep each task on a branch whose name describes its purpose.

| Prefix | Use | Example |
| --- | --- | --- |
| `feature/` | New or expanded functionality | `feature/wavetable-import` |
| `bugfix/` | Corrections to existing behavior | `bugfix/preset-recall` |
| `docs/` | Documentation changes | `docs/git-branch-workflow` |
| `chore/` | Build, dependency, or repository maintenance | `chore/update-build-tools` |

Use a short, lowercase, hyphenated name after the prefix.

1. Run `git fetch origin` and `git branch --all`. If a local or remote branch already covers the same active work, check its history and continue there. Reuse it when revisiting the same task, including review fixes. If it exists only on the remote, use `git switch --track origin/<branch-name>`. Coordinate with anyone already working on it.
2. If no suitable branch exists, run `git switch main` and `git pull --ff-only origin main`, then create the task branch with `git switch -c <branch-name>`. If the name is taken by different work, choose a name that describes the difference.
3. Keep commits focused. Run the relevant checks and use a pull request into `main` for review and merge. Delete the merged branch once any dependent work has moved off it.

## Find files and resources

Answer simple questions from the prompt and conversation when that supplies enough information. When a task needs project context, start with [FILE_MAP.md](docs/FILE_MAP.md) and [SKILLS_AND_RESOURCES.md](docs/SKILLS_AND_RESOURCES.md) in either order. Open only the files and skill instructions they route you to for the task. Follow includes and call sites as needed. Start searches with `rg --files <path>` or `rg -n "<pattern>" <path>` in the relevant directory and widen the search if the mapped paths are insufficient.

Name files for the feature or responsibility they own. Update the file map in the same change when adding, removing, moving, renaming, or repurposing a maintained file. Create planned paths when approved work needs them. Use the root `build/` for generated output and search `test/` when the task concerns the local experiment.

Use [STYLE_GUIDE.md](docs/STYLE_GUIDE.md) for editor changes. Update [USER_MANUAL.md](docs/USER_MANUAL.md) when a major player-facing feature is added or its workflow changes.

## Technologies and dependencies

- Target Windows x64 and FL Studio first. Build the instrument as a VST3 using C++, JUCE, CMake, MSVC, and the Windows SDK.
- Pin the JUCE version in the root build configuration. Record the supported compiler, CMake, SDK, and FL Studio versions in the README when the production toolchain is agreed. The local experiment has its own dependency pin.
- Get approval before adding or upgrading dependencies. Record why the dependency is needed and check the applicable licence before sharing a build.

## Code quality

- Make the smallest change that covers the approved requirement. Reuse JUCE and existing project code where suitable. Create files and abstractions when they make a responsibility easier to find or verify.
- Keep the audio callback free of heap allocation, blocking locks, logging, and file I/O in project code. Prepare resources before processing and smooth continuous controls that affect the signal.
- Check output gain with a single note at normal velocity and with a full chord. Set a usable default level and leave headroom under the expected voice load.
- Keep the plug-in identity and parameter IDs stable. Version saved state and check that projects created with supported earlier builds can reopen with their settings.
- Verify affected behavior through its normal entry point. For plug-in changes, build the Windows x64 Release VST3 and use the relevant FL Studio checks from the PRD. Report any checks that could not run.
