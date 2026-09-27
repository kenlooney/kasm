add_subdirectory(unit)

# This test has its own main function, so it can exercise library APIs without
# adding temporary lesson code to the real kasm command-line program.
add_executable(kasm_cursor_cli cursor_cli.c)
target_link_libraries(kasm_cursor_cli PRIVATE kasm::core)

add_executable(kasm_diagnostic_cli diagnostic_cli.c)
target_link_libraries(kasm_diagnostic_cli PRIVATE kasm::core)

add_executable(kasm_puncuation_cli puncuation_cli.c)
target_link_libraries(kasm_puncuation_cli PRIVATE kasm::core)

add_executable(kasm_ident_numbers_cli ident_numbers_cli.c)
target_link_libraries(kasm_ident_numbers_cli PRIVATE kasm::core)

if(MSVC)
    target_compile_options(kasm_cursor_cli PRIVATE /W4)
    target_compile_options(kasm_diagnostic_cli PRIVATE /W4)
    target_compile_options(kasm_puncuation_cli PRIVATE /W4)
    target_compile_options(kasm_ident_numbers_cli PRIVATE /W4)

    set_property(TARGET kasm_cursor_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_diagnostic_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_puncuation_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_ident_numbers_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(kasm_cursor_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_diagnostic_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_puncuation_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_ident_numbers_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME kasm.cursor.cli
    COMMAND kasm_cursor_cli "${PROJECT_SOURCE_DIR}/examples/sample.asm"
)

add_test(NAME kasm.cli.requires_source COMMAND kasm)
set_tests_properties(kasm.cli.requires_source PROPERTIES
    WILL_FAIL TRUE
)

add_test(
    NAME kasm.puncuation.cli
    COMMAND kasm_puncuation_cli "${PROJECT_SOURCE_DIR}/examples/punctuation.asm"
)

add_test(
    NAME kasm.ident_numbers.cli
    COMMAND kasm_ident_numbers_cli "${PROJECT_SOURCE_DIR}/examples/ident_numbers.asm"
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
add_test(
    NAME kasm.diagnostic.cli
    COMMAND kasm_diagnostic_cli "${PROJECT_SOURCE_DIR}/examples/oops.asm"
)
set_tests_properties(kasm.diagnostic.cli PROPERTIES
    WILL_FAIL TRUE
)
