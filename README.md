# kasm
Ken's Assembler

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
runs the installed executable. Release builds additionally create and extract a
ZIP and run the packaged executable before uploading it as a workflow artifact.
The current test checks the program's greeting; add further CTest tests as the
assembler grows.

Download the `kasm-Windows-x64` and `kasm-Linux-x64` artifacts from a successful
Actions run. Each artifact contains its platform's CPack ZIP; artifacts are kept
for 14 days.

To publish a release:

1. Set the version in `project(kasm VERSION ...)` in `CMakeLists.txt`.
2. Commit and push the changes, including the workflow.
3. Push a matching version tag, for example:

   ```sh
   git tag v0.1.0
   git push origin v0.1.0
   ```

After all four builds pass, the tag workflow publishes both ZIPs to a GitHub
Release with generated release notes. Tags must exactly match the CMake version
prefixed with `v`; a mismatch fails configuration. Re-running a tag workflow
replaces assets on an existing release. Publishing uses the built-in
`GITHUB_TOKEN` with `contents: write` only in the release job; no personal token
is required. GitHub Actions must be enabled for the repository.
