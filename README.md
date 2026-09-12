# Ken's Assembler

### Run Tests
Configure and build before running CTest:

```powershell
cmake --preset windows-debug -DBUILD_TESTING=ON
cmake --build --preset windows-debug
ctest --test-dir build/windows-debug -C Debug --output-on-failure
```
```bash
cmake --preset GCC-debug -DBUILD_TESTING=ON
cmake --build --preset GCC-debug
ctest --test-dir build/GCC-debug -C Debug --output-on-failure -V
```
The presets create separate build directories under `build`; CTest must point to
the configured directory. For Release, use `windows-release` in all three commands
and replace `-C Debug` with `-C Release`.

To see test output even when tests pass, add `-V` to the command.
