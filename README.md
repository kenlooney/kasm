# Ken's Assembler

### Run Tests
Run these commands from the repository root (the directory containing
`CMakePresets.json`). Configure and build before running CTest:

```powershell
cmake --preset windows-debug -DBUILD_TESTING=ON
cmake --build --preset windows-debug
ctest --test-dir build/windows-debug -C Debug --output-on-failure -V
```
```bash
cmake --preset GCC-debug -DBUILD_TESTING=ON
cmake --build --preset GCC-debug
ctest --test-dir build/GCC-debug -C Debug --output-on-failure -V
```
The presets create separate build directories under `build`; CTest must point to
the configured directory. For Release, use `windows-release` in all three commands
and replace `-C Debug` with `-C Release`.

If your terminal is already in `build/GCC-debug`, run `cd ../..` first to return
to the repository root before using the commands above.

To see test output even when tests pass, add `-V` to the command.
