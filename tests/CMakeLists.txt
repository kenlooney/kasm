add_subdirectory(unit)

add_test(NAME kasm.cli.requires_source COMMAND kasm)
set_tests_properties(kasm.cli.requires_source PROPERTIES
    WILL_FAIL TRUE
)

# Run the built executable with a source file from the repository. Using
# PROJECT_SOURCE_DIR makes the path work regardless of the build directory.
add_test(
    NAME kasm.cli.loads_source
    COMMAND kasm "${PROJECT_SOURCE_DIR}/examples/first.asm"
)
set_tests_properties(kasm.cli.loads_source PROPERTIES
    # A zero exit code and this output mean the source file loaded successfully.
    PASS_REGULAR_EXPRESSION "kasm [0-9]+\\.[0-9]+\\.[0-9]+"
)
