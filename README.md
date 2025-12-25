# JuceCMakeStarter

CMake-only JUCE MIDI-FX plugin project (VST3 + Standalone) for macOS.

## Layout
- `source/` plugin source code
- `assets/` project assets (images, presets, etc.)
- `JUCE/` JUCE git submodule (added via `git submodule add`)

## Prereqs
- CMake (>= 3.22)
- Ninja (`brew install ninja`)
- A compiler toolchain (Xcode Command Line Tools are fine; we just won’t generate Xcode projects)

## JUCE submodule is read-only (recommended)
This repo treats JUCE as a framework dependency. To avoid accidental edits inside `JUCE/`, you can lock it read-only:

```bash
./scripts/juce-lock.sh
```

To update JUCE later:

```bash
./scripts/juce-unlock.sh
git submodule update --remote --merge
./scripts/juce-lock.sh
```

## Configure + build (recommended)
```bash
cmake --preset macos-debug
cmake --build --preset build-debug
```

Release:
```bash
cmake --preset macos-release
cmake --build --preset build-release
```

## Outputs
Artifacts will be under `build/...` and/or copied to your default plugin locations if supported by your JUCE/CMake setup.
