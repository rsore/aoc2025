# Advent Of Code 2025

This is my solutions for the challenges from [Advent Of Code 2025](https://adventofcode.com/2025).

## Requirements

### Windows
 - Microsoft Visual C++ (MSVC) Build Tools

### Linux
 - Git LFS (e.g. `dnf install git-lfs && git lfs install`)
 - Wayland development package (e.g. `wayland-devel`)
 - xkbcommon development pacakge (e.g. `libxkbcommon-devel`)
 - X11 development packages (e.g. `libX11-devel`, `libXcursor-devel`, `libXrandr-devel`, `libXinerama-devel`, `libXi-devel`)
 - C compiler (gcc/clang)

## Build

### Windows
Open repo in an x64 Native Tools Command Prompt, to have access to `cl`.
```bash
cl bs.c             # Bootstrap build system once
bs.exe [options...] # Build aoc2025 program. Use --help for help menu
```

### Linux
```bash
gcc -o bs bs.c      # Bootstrap build system once
./bs [options...]   # Build aoc2025 program. Use --help for help menu
```


## Editor integration
- When invoking `bs` you may pass `--emit-compile-commands` to generate a clangd-compatible `compile_commands.json` file.
- When invoking `bs` you may pass `--emit-vscode-tasks` to generate tasks in `.vscode` directory for building and running Stellar inside VSCode.
- For Emacs users, after compiling `bs` once, you may use the `.dir-locals.el` file in this repo to get access to a Transient-based UI for building Stellar. It creates a keybinding (`C-c b`) to open the UI. If you prefer a direct command you may use `(stellar/transient)` to open the same UI.
