
A fork of [pxtone Collage](https://pxtone.org). The primary goal is for it to run natively on all desktop operating systems. Please see the [roadmap](ROADMAP.md) for the current status.

## Background
These are the respective versions for each pxtone project at the time of handoff (e.g., the version this is based off of).

|Program	|Version No.|
|--|--|
|ptCollage|0.9.3.2		|
|ptVoice	|0.9.4.1		|
|ptNoise	|0.9.3.1		|
|ptPlayer	|0.9.3.1		|

This is not pxtone engine -- the unmodified version of that can be found at [pxtone.org/developer](https://pxtone.org/developer), and the version used in this repository [here](https://github.com/ewancg/pxtone).
The original version of the code provided can be found [here](https://www.cavestory.org/downloads/pxtoneProjectT-220401b-open230815c.zip). The source was provided by the original author of this software ([Studio Pixel](https://studiopixel.jp)) around January 5 2021 and explicit permission to release under the [zlib license](LICENSE) was given on July 21 2025.

## Dependencies
- zlib (provided)
- libpng (provided)
- libvorbis / OGG (provided)
- WinAPI *or* Wine (planned for removal; aiming to avoid OS-specific APIs)
- XAudio2 on DirectX 9.0 (planned for removal; non-functional on MSYS2, reliant on DirectX SDK or Wine's FAudio)

All external dependencies were also provided with known working versions.

#### System-specific dependencies
- Wine (for non-Windows platforms)
  - Wine will only be a dependency as long as cross-platform support is still in progress. Until then, you can use it to:
    - Run the Windows PE on your non-Windows OS, by giving Wine the executable
    - Build a native ELF/Mach-O/whatever for your OS which finds Wine itself

#### Planned dependencies
- SDL3 (for windowing & rendering)
- miniaudio (for audio mixing & output; would be provided)
- sds (to port away from Windows managed strings)

## Building
The CMakeLists.txt should have a description for each option, which you can see with `cmake -LH`.

[^1]: Visual Studio support is not actively tested.
[^2]: Non-Nix build options for Linux are not actively tested.
[^3]: macOS is not actively tested.

### Building on Windows
Options for building on Windows:
1. [MSYS2](#building-in-msys2): since CMake will be the primary build system going forward and MSYS2 more closely reflects the conventions being adopted, it is the recommended choice for building on Windows.
2. [Visual Studio](#building-in-visual-studio): Visual Studio support has been preserved.[^1]

### Building on Linux
Options for building on Linux:
- [Nix](#building-with-nix): since Nix can most easily provide the `winelib`-compatible toolchain currently required, it is the recommended choice for building on Linux. `flake.nix` is provided for [building with NixOS or another Nix-equipped distribution](#building-with-nix).
- CMake: `CMakeLists.txt` can be used without Nix if the environment has a usable `winelib` toolchain, and you know how to convey that to CMake.[^2] This will be equally preferable to Nix once Wine is no longer a dependency.

### Building on macOS
- [Nix](#building-with-nix): `flake.nix` is provided for [building with nix-darwin](#building-with-nix).[^3]
- CMake: `CMakeLists.txt` can be used without nix-darwin if the environment has a usable `winelib` toolchain, and you know how to convey that to CMake.[^3]

---

#### Building in MSYS2
Install the following prerequisite packages one time using `pacman -S mingw-w64-ucrt-x86_64-libogg mingw-w64-ucrt-x86_64-libvorbis mingw-w64-ucrt-x86_64-libpng` unless you want to use the bundled versions.

This command builds all tools using the packages installed from the previous command:
`cmake -B build -S . -DBUNDLE_BUILD_DIR=release -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`

The command will change if you did not install the packages and wish to use the provided ones.
`cmake -B build -S . -DBUNDLE_BUILD_DIR=release -DBUNDLED_VORBIS=on -DBUNDLED_PNG=on -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`

#### Building in Visual Studio
The Visual Studio project files have been kept intact, the resource files have been preserved to be compatible with `rc.exe`, and the intention is that the project can still be opened by & developed with Visual Studio. There are two ways to open the project:
  - Proceed without a project & load `CMakeLists.txt` by selecting CMake in the File menu
  - Open `pxtoneProjectT.sln` and rely on the original project structure

#### Building with Nix 
The provided flake installs Wine, exposes it to the development environment, provides a toolchain file that CMake uses to hook itself up to `winelib`, then exposes its location to the development environment. 

`libpng` must not be bundled when `winelib` is in use.

This command builds all tools. 
`cmake -B build -S . --toolchain "$WINELIB64_TOOLCHAIN" -DBUNDLED_PNG=off -DBUNDLE_BUILD_DIR=release -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel`

`nix build` will do this and output to `result`.
