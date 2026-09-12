# Ken's Assembler

## Build, test, install, and package

With CMake 3.15+ and a C compiler installed, run from the repository root:

```sh
cmake -S . -B build/local -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/local --config Release --parallel
ctest --test-dir build/local -C Release --output-on-failure --no-tests=error
cmake --install build/local --config Release --prefix install/local
cpack --config build/local/CPackConfig.cmake -C Release -G ZIP -B dist
```

The `ctest --test-dir` command above requires CMake 3.20+; with older
versions, run `ctest` from the build directory instead.
The existing presets remain available for local Visual Studio 2026 and GCC builds
(the preset file requires CMake 3.28+).

The installed executable is in `install/local/bin`. ZIP packages in `dist/`
contain `bin/kasm` (or `bin/kasm.exe`) and the README and license under
`share/doc/kasm`. Extract a ZIP and run the executable from its `bin` directory.
Windows builds use the static MSVC runtime. Linux packages target the runner's
Linux environment; compatibility with older distributions is not guaranteed.

## GitHub Actions and releases

The workflow in `.github/workflows/ci.yml` runs on branch pushes, pull requests,
and manual dispatch. It builds and tests Debug and Release on Windows 2022
(Visual Studio 2022, x64) and Ubuntu 24.04 (GCC, x64), installs each build, and
runs the installed executable. Only pushes to `main` additionally create and
extract Release ZIPs and run the packaged executables before uploading artifacts.
Pushes to `dev` or other branches, pull requests, and manual runs build, test,
and install without producing ZIPs or publishing releases. Tag pushes do not
trigger this workflow.
The current test checks the program's greeting; add further CTest tests as the
assembler grows.

Download the `kasm-Windows-x64` and `kasm-Linux-x64` artifacts from a successful
`main` push run. Each artifact contains its platform's CPack ZIP; artifacts are kept
for 14 days.

To publish, merge your changes from `dev` into `main`. After all four builds
pass, the workflow publishes both ZIPs to a new GitHub Release with generated
release notes. The release title uses the CMake version, for example `kasm 0.1.0`.
The tag is automatically named `v<version>-build.<run number>` (for example
`v0.1.0-build.3`) and points to the exact commit built. The build number keeps
tags unique when multiple merges use the same version. Previous releases remain available; rerunning
the same workflow run replaces that release's matching assets. ZIP filenames
use the version in `project(kasm VERSION ...)` in `CMakeLists.txt`, which you can
bump when appropriate without needing to change it for every merge.

Direct pushes to `main` also publish. To enforce merges only, protect `main`
with a repository rule requiring a pull request before merging.
Publishing uses the built-in
`GITHUB_TOKEN` with `contents: write` only in the release job; no personal token
is required. GitHub Actions must be enabled for the repository.
