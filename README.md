# DevForge

**DevForge** is an offline-first Linux desktop developer toolbox built with C++17 and Qt 6.

The first release focuses on a simple workflow: select a CMake project, configure and build it, and inspect its Git status from one desktop window.

## Features in 0.1.0

- Dark Qt 6 desktop interface
- Choose a local project directory
- Detect a CMake project through `CMakeLists.txt`
- Configure with CMake and Ninja
- Build with CMake
- Check Git status
- View command output and errors in the application
- GitHub Actions Linux x86_64 build

## Requirements

- Linux desktop environment
- Qt 6 Widgets (required to build DevForge)
- CMake
- Ninja
- A C++17 compiler
- Git (for the Git status feature)

The selected project must contain a `CMakeLists.txt` file to use the build action.

## Build from source

On Debian or Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential cmake ninja-build qt6-base-dev git
git clone https://github.com/minax2036-beep/DevForge.git
cd DevForge
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/DevForge
```

## Download a GitHub Actions build

1. Open the repository's **Actions** tab.
2. Select the latest successful **Build DevForge** run.
3. Under **Artifacts**, download `DevForge-linux-x86_64`.
4. Extract it and run the executable in a Linux desktop session.

For example:

```bash
tar -xzf DevForge-linux-x86_64.tar.gz
chmod +x DevForge
./DevForge
```

The artifact is a Linux x86_64 executable, not a universal Linux package. The target system needs compatible Qt 6 runtime libraries and a graphical desktop session.

## Current limitations

- Only CMake projects are supported by the build action.
- The build directory is created as `build/` inside the selected project.
- The current release does not modify source files or run arbitrary build scripts automatically.
- Project history, settings persistence, and plugin support are not implemented yet.

## Roadmap

### 0.2.x — Project Hub
- Remember recently opened projects
- Project metadata and quick actions
- Clearer project validation

### 0.3.x — Build Engine
- Build presets and build-type selection
- Configure/build error summaries
- Better cancellation and process handling

### 0.4.x — Git Manager
- Branch and repository overview
- Diff preview
- Safer common Git actions

### 0.5.x — Artifact Forge
- Organize generated build artifacts
- Export project bundles
- Create release packages

### Later
- Optional plugin system
- Customizable themes
- More build-system integrations

## License

No license has been declared yet. Until a license is added, do not assume that the source is available for redistribution under an open-source license.
