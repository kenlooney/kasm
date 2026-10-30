# kenx
A x86 assembler I am writing from scratch
## What is next?
I am planning on writing an graphical emulator for it using SDL3.

## Building on Windows

Install Visual Studio 2026 with the C++ desktop workload, CMake 3.28 or newer
(required by the presets), and vcpkg. Set the `VCPKG_ROOT` environment variable
to your vcpkg installation directory, then restart VS Code if it was already
running so it picks up the variable.

## Building on Linux / WSL2

Install GCC/G++, Make, CMake 3.28 or newer, and a Linux installation of
vcpkg. Set `VCPKG_ROOT` to that installation (not the Windows installation).
For example, if vcpkg is installed at `~/vcpkg`:

```sh
export VCPKG_ROOT="$HOME/vcpkg"
cmake --fresh --preset GCC-debug
cmake --build --preset GCC-debug
ctest --preset GCC-debug
```
