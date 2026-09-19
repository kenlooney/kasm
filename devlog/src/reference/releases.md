# Using a release

Download the Windows x64 or Linux x64 ZIP from
[GitHub Releases](https://github.com/kenlooney/kasm/releases) and extract it.
Each archive contains `bin/kasm.exe` (Windows) or `bin/kasm` (Linux), plus the
project README and the Apache 2.0 license under `share/doc/kasm`.

From the extracted package directory, create `example.asm` containing
`mov eax,42;`, then run:

```powershell
.\bin\kasm.exe example.asm
.\bin\kasm.exe --bits 64 example.asm
```

```bash
./bin/kasm example.asm
./bin/kasm --bits 64 example.asm
```

Expected output with the current source build:

```text
B8 2A 00 00 00
0000: mov eax, 42
Decoding successful.
```

The CLI accepts either one source-file path, which defaults to 64-bit mode,
or `--bits 16|32|64` followed by one source-file path. The current
implementation supports the initial 16-bit forms described in
[The 16-bit instruction set](sixteen-bit.md) and the existing 64-bit path;
32-bit mode is rejected. It prints one line of uppercase hexadecimal bytes,
with a space after each byte, followed by a decoded listing and
`Decoding successful.` when decoding succeeds. Older releases may print only
the hexadecimal line. For `program.asm`, the CLI writes `program.bin` (raw
bytes) and `program.h` (C declarations) in the **current working directory**,
replacing previous files with those names before decoding. Redirecting
stdout saves text, including the listing, rather than a raw binary. Unless a
full listing is shown, the encoding examples elsewhere in this reference show
only the first hexadecimal line. Instructions are encoded in source order and
are not executed by the CLI. Windows packages use the static MSVC runtime;
Linux packages are built on Ubuntu 24.04.

Source-file links in this reference point to the repository on GitHub; source
files and test drivers are not included in the binary ZIPs.

## Packaging and releases

The repository workflow builds and tests Debug and Release configurations on
Windows x64 and Linux x64. On a push to `main`, successful builds produce two
Release ZIPs through CPack and publish a GitHub Release. Tags use
`v<version>-build.<run-number>`; the version comes from
`project(kasm VERSION ...)` in
[CMakeLists.txt](https://github.com/kenlooney/kasm/blob/main/CMakeLists.txt).
A merge to `main` triggers this process through its push.

To build a Windows ZIP locally from the repository root:

```powershell
cmake --preset windows-release -DBUILD_TESTING=ON
cmake --build --preset windows-release
ctest --test-dir build/windows-release -C Release --output-on-failure
cpack --config build/windows-release/CPackConfig.cmake -C Release -G ZIP -B dist
```

The package installs the executable under `bin` and documentation under
`share/doc/kasm`. GitHub's automatically generated source archives are
separate from these executable packages.

## License

Copyright 2026 Kenneth Looney. Licensed under the Apache License, Version 2.0.
See [LICENSE](https://github.com/kenlooney/kasm/blob/main/LICENSE), included
beside the README in release packages.
