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

add_executable(kasm_expr_cli expr_cli.c)
target_link_libraries(kasm_expr_cli PRIVATE kasm::core)

add_executable(kasm_instructions_cli instructions_cli.c)
target_link_libraries(kasm_instructions_cli PRIVATE kasm::core)

add_executable(kasm_braces_cli braces_cli.c)
target_link_libraries(kasm_braces_cli PRIVATE kasm::core)

add_executable(kasm_semi_cli semi_cli.c)
target_link_libraries(kasm_semi_cli PRIVATE kasm::core)

add_executable(kasm_bytebuffer_cli bytebuffer_cli.c)
target_link_libraries(kasm_bytebuffer_cli PRIVATE kasm::core)

add_executable(kasm_real_mode_cli real_mode_cli.c)
target_link_libraries(kasm_real_mode_cli PRIVATE kasm::core)

add_executable(kasm_bootsect_cli bootsect_cli.c)
target_link_libraries(kasm_bootsect_cli PRIVATE kasm::core)

add_executable(kasm_startup_cli startup_cli.c)
target_link_libraries(kasm_startup_cli PRIVATE kasm::core)

add_executable(kasm_inc_cli inc_cli.c)
target_link_libraries(kasm_inc_cli PRIVATE kasm::core)

add_executable(kasm_encoded_register_cli encoded_register_cli.c)
target_link_libraries(kasm_encoded_register_cli PRIVATE kasm::core)

add_executable(kasm_confirm_reg_mov_cli confirm_reg_mov_cli.c)
target_link_libraries(kasm_confirm_reg_mov_cli PRIVATE kasm::core)

add_executable(kasm_segment_mov_cli segment_mov_cli.c)
target_link_libraries(kasm_segment_mov_cli PRIVATE kasm::core)

if(MSVC)
    target_compile_options(kasm_cursor_cli PRIVATE /W4)
    target_compile_options(kasm_diagnostic_cli PRIVATE /W4)
    target_compile_options(kasm_puncuation_cli PRIVATE /W4)
    target_compile_options(kasm_ident_numbers_cli PRIVATE /W4)
    target_compile_options(kasm_expr_cli PRIVATE /W4)
    target_compile_options(kasm_instructions_cli PRIVATE /W4)
    target_compile_options(kasm_braces_cli PRIVATE /W4)
    target_compile_options(kasm_semi_cli PRIVATE /W4)
    target_compile_options(kasm_bytebuffer_cli PRIVATE /W4)
    target_compile_options(kasm_real_mode_cli PRIVATE /W4)
    target_compile_options(kasm_bootsect_cli PRIVATE /W4)
    target_compile_options(kasm_startup_cli PRIVATE /W4)
    target_compile_options(kasm_inc_cli PRIVATE /W4)
    target_compile_options(kasm_encoded_register_cli PRIVATE /W4)
    target_compile_options(kasm_segment_mov_cli PRIVATE /W4)
    target_compile_options(kasm_confirm_reg_mov_cli PRIVATE /W4)

    set_property(TARGET kasm_cursor_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_diagnostic_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_puncuation_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_ident_numbers_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_expr_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_instructions_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_braces_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_semi_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_bytebuffer_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_real_mode_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_bootsect_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_startup_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_inc_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_encoded_register_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_segment_mov_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_confirm_reg_mov_cli PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(kasm_cursor_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_diagnostic_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_puncuation_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_ident_numbers_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_expr_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_instructions_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_braces_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_semi_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_bytebuffer_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_real_mode_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_bootsect_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_startup_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_inc_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_encoded_register_cli PRIVATE
        -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_confirm_reg_mov_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(kasm_segment_mov_cli PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(
    NAME kasm.inc.cli
    COMMAND kasm_inc_cli "${PROJECT_SOURCE_DIR}/examples/inc.asm"
)

add_test(
    NAME kasm.encoded_register.cli
    COMMAND kasm_encoded_register_cli
        "${PROJECT_SOURCE_DIR}/examples/encoded_register.asm"
)

set(KASM_ENCODED_REGISTER_IMAGE
    "${CMAKE_CURRENT_BINARY_DIR}/encoded_register.bin")

add_test(
    NAME kasm.encoded_register.image
    COMMAND kasm
        "${PROJECT_SOURCE_DIR}/examples/encoded_register.asm"
        "${KASM_ENCODED_REGISTER_IMAGE}"
)
set_tests_properties(kasm.encoded_register.image PROPERTIES
    FIXTURES_SETUP kasm_encoded_register_image
)

add_test(
    NAME kemu.encoded_register.cli
    COMMAND kemu "${KASM_ENCODED_REGISTER_IMAGE}"
)
set_tests_properties(kemu.encoded_register.cli PROPERTIES
    FIXTURES_REQUIRED kasm_encoded_register_image
    PASS_REGULAR_EXPRESSION
        "halted after 43 instructions at 0000:7C2F, AX=1234"
)

set(KASM_FAR_PAIR_IMAGE
    "${CMAKE_CURRENT_BINARY_DIR}/far_jump_pair.bin")
add_test(
    NAME kasm.far_pair.image
    COMMAND kasm
        "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/far_jump_pair.asm"
        "${KASM_FAR_PAIR_IMAGE}"
)
set_tests_properties(kasm.far_pair.image PROPERTIES
    FIXTURES_SETUP kasm_far_pair_image
)
add_test(
    NAME kemu.far_pair.cli
    COMMAND kemu "${KASM_FAR_PAIR_IMAGE}"
)
set_tests_properties(kemu.far_pair.cli PROPERTIES
    FIXTURES_REQUIRED kasm_far_pair_image
    PASS_REGULAR_EXPRESSION
        "halted after 2 instructions at 0700:0C06, AX=0000"
)

set(KASM_FAR_NORMALIZE_IMAGE
    "${CMAKE_CURRENT_BINARY_DIR}/far_jump_normalize.bin")
add_test(
    NAME kasm.far_normalize.image
    COMMAND kasm
        "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/far_jump_normalize.asm"
        "${KASM_FAR_NORMALIZE_IMAGE}"
)
set_tests_properties(kasm.far_normalize.image PROPERTIES
    FIXTURES_SETUP kasm_far_normalize_image
)
add_test(
    NAME kemu.far_normalize.cli
    COMMAND kemu "${KASM_FAR_NORMALIZE_IMAGE}"
)
set_tests_properties(kemu.far_normalize.cli PROPERTIES
    FIXTURES_REQUIRED kasm_far_normalize_image
    PASS_REGULAR_EXPRESSION
        "halted after 2 instructions at 0000:7C06, AX=0000"
)

set(KASM_FAR_TRUNCATED_IMAGE
    "${CMAKE_CURRENT_BINARY_DIR}/far_jump_truncated.bin")
add_test(
    NAME kasm.far_truncated.image
    COMMAND kasm
        "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/far_jump_truncated.asm"
        "${KASM_FAR_TRUNCATED_IMAGE}"
)
set_tests_properties(kasm.far_truncated.image PROPERTIES
    FIXTURES_SETUP kasm_far_truncated_image
)
add_test(
    NAME kemu.far_truncated.cli
    COMMAND ${CMAKE_COMMAND}
        "-DPROGRAM=$<TARGET_FILE:kemu>"
        "-DINPUT=${KASM_FAR_TRUNCATED_IMAGE}"
        "-DEXPECTED=truncated far JMP instruction at 0000:7C00"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/expect_failure.cmake"
)
set_tests_properties(kemu.far_truncated.cli PROPERTIES
    FIXTURES_REQUIRED kasm_far_truncated_image
)

set(KASM_FAR_OUTSIDE_IMAGE
    "${CMAKE_CURRENT_BINARY_DIR}/far_jump_outside.bin")
add_test(
    NAME kasm.far_outside.image
    COMMAND kasm
        "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/far_jump_outside.asm"
        "${KASM_FAR_OUTSIDE_IMAGE}"
)
set_tests_properties(kasm.far_outside.image PROPERTIES
    FIXTURES_SETUP kasm_far_outside_image
)
add_test(
    NAME kemu.far_outside.cli
    COMMAND ${CMAKE_COMMAND}
        "-DPROGRAM=$<TARGET_FILE:kemu>"
        "-DINPUT=${KASM_FAR_OUTSIDE_IMAGE}"
        "-DEXPECTED=far JMP target outside loaded image at 0000:0000"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/expect_failure.cmake"
)
set_tests_properties(kemu.far_outside.cli PROPERTIES
    FIXTURES_REQUIRED kasm_far_outside_image
)

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

add_test(
    NAME kasm.expr.cli
    COMMAND kasm_expr_cli "${PROJECT_SOURCE_DIR}/examples/expr.asm"
)

add_test(
    NAME kasm.confirm_reg_mov.cli
    COMMAND kasm_confirm_reg_mov_cli "${PROJECT_SOURCE_DIR}/examples/confirm_reg_mov.asm"
)

add_test(
    NAME kasm.segment_mov.cli
    COMMAND kasm_segment_mov_cli "${PROJECT_SOURCE_DIR}/examples/segment_mov.asm"
)

add_test(
    NAME kasm.instructions.cli
    COMMAND kasm_instructions_cli "${PROJECT_SOURCE_DIR}/examples/instructions.asm"
)
add_test(
    NAME kasm.braces.cli
    COMMAND kasm_braces_cli "${PROJECT_SOURCE_DIR}/examples/braces.asm"
)
add_test(
    NAME kasm.semi.cli
    COMMAND kasm_semi_cli "${PROJECT_SOURCE_DIR}/examples/semi.asm"
)
add_test(
    NAME kasm.bytebuffer.cli
    COMMAND kasm_bytebuffer_cli "${PROJECT_SOURCE_DIR}/examples/semi.asm"
)
add_test(
    NAME kasm.real_mode.cli
    COMMAND kasm_real_mode_cli "${PROJECT_SOURCE_DIR}/examples/real_mode.asm"
)

add_test(
    NAME kasm.bootsect.cli
    COMMAND kasm_bootsect_cli
        "${PROJECT_SOURCE_DIR}/examples/bootsect.asm"
        "${CMAKE_CURRENT_BINARY_DIR}/bootsect.bin"
)

add_test(
    NAME kasm.startup.cli
    COMMAND kasm_startup_cli
        "${PROJECT_SOURCE_DIR}/examples/startup.asm"
        "${CMAKE_CURRENT_BINARY_DIR}/startup.bin"
)

# Run the built executable with a source file from the repository. Using
# PROJECT_SOURCE_DIR makes the path work regardless of the build directory.
add_test(
    NAME kasm.cli.loads_source
    COMMAND kasm
        "${PROJECT_SOURCE_DIR}/examples/first.asm"
        "${CMAKE_CURRENT_BINARY_DIR}/first.bin"
)
add_test(
    NAME kasm.diagnostic.cli
    COMMAND kasm_diagnostic_cli "${PROJECT_SOURCE_DIR}/examples/oops.asm"
)
set_tests_properties(kasm.diagnostic.cli PROPERTIES
    WILL_FAIL TRUE
)
