cmake_minimum_required(VERSION 3.20)
project(kasm VERSION 0.23.0 LANGUAGES C)

file(GLOB_RECURSE SOURCES CONFIG_DEPENDS "src/*.c")
add_executable(kasm
    ${SOURCES})
target_include_directories(kasm PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Keep Windows ZIPs runnable without a separate Visual C++ runtime install.
if(MSVC)
    set_property(TARGET kasm PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
# Install rules
install(TARGETS kasm RUNTIME DESTINATION bin)
install(FILES README.md LICENSE DESTINATION share/doc/kasm)

include(CTest)

if(BUILD_TESTING)
    add_executable(semantic_test_driver
        tests/semantic_driver.c
        src/decode.c
        src/layout.c
        src/output.c
        src/binary.c
        src/semantic.c
        src/program.c
        src/expr.c
        src/lexer.c
        src/source.c
        src/cursor.c
        src/diagnostic.c)
    target_include_directories(semantic_test_driver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    function(add_example_test name source expected_hex)
        add_test(NAME encode.${name} COMMAND "${CMAKE_COMMAND}"
            "-DKASM=$<TARGET_FILE:kasm>"
            "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/${source}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/${name}"
            "-DEXPECTED_HEX=${expected_hex}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    endfunction()
    add_example_test(mov_42 examples/mov_42.asm "B8 2A 00 00 00")
    add_example_test(ret examples/ret.asm "C3")
    add_example_test(program examples/program.asm "B8 2A 00 00 00 C3")
    foreach(data_case data_bytes data_jump data_words data_words_jump)
        if(data_case STREQUAL "data_bytes")
            set(data_hex "41 42 00 AA 55")
        elseif(data_case STREQUAL "data_jump")
            set(data_hex "E9 05 00 00 00 41 42 00 AA 55 B8 2A 00 00 00 C3")
        elseif(data_case STREQUAL "data_words")
            set(data_hex "34 12 00 00 FF FF AA 55 41 00")
        else()
            set(data_hex "E9 06 00 00 00 41 34 12 00 FF FF B8 2A 00 00 00 C3")
        endif()
        add_test(NAME encode.${data_case} COMMAND "${CMAKE_COMMAND}"
            "-DKASM=$<TARGET_FILE:kasm>"
            "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/${data_case}.asm"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/${data_case}"
            "-DEXPECTED_HEX=${data_hex}"
            "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/data_expected.txt"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    endforeach()
    add_test(NAME semantic.data_invalid COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/data_invalid"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/data_invalid.cmake")
    add_test(NAME encode.ecx_registers COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/ecx_registers.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/ecx_registers"
        "-DEXPECTED_HEX=B9 2A 00 00 00 81 C1 07 00 00 00 81 E9 03 00 00 00 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/ecx_registers_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME encode.edx_registers COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/edx_registers.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/edx_registers"
        "-DEXPECTED_HEX=BA 2A 00 00 00 81 C2 07 00 00 00 81 EA 03 00 00 00 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/edx_registers_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME semantic.reject_mode_16 COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/mov_42.asm"
        "-DBITS=16"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/reject_mode_16"
        "-DEXPECTED_ERROR=16-bit mode is not supported"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/reject_mode.cmake")
    add_test(NAME semantic.reject_mode_32 COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/mov_42.asm"
        "-DBITS=32"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/reject_mode_32"
        "-DEXPECTED_ERROR=32-bit mode is not supported"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/reject_mode.cmake")
    add_test(NAME encode.xor COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/xor.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/xor"
        "-DEXPECTED_HEX=35 2A 00 00 00 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/xor_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME encode.add COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/add.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/add"
        "-DEXPECTED_HEX=B8 23 00 00 00 05 07 00 00 00 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/add_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME decode.example COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/decode.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/decode"
        "-DEXPECTED_HEX=B8 03 00 00 00 FF C8 0F 85 F8 FF FF FF B8 2A 00 00 00 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/decode_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME decode.inc_jz COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/decode_inc_jz.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/decode_inc_jz"
        "-DEXPECTED_HEX=B8 29 00 00 00 FF C0 0F 84 01 00 00 00 C3 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/decode_inc_jz_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")

    add_test(NAME semantic.expression COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:semantic_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/semantic_expression.cmake")

    
    add_executable(program_blocks_test
        tests/program_blocks.c
        src/program.c
        src/expr.c
        src/lexer.c
        src/source.c
        src/cursor.c
        src/diagnostic.c)

    
    target_include_directories(program_blocks_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME parser.blocks COMMAND "${CMAKE_COMMAND}"
        "-DDRIVER=$<TARGET_FILE:program_blocks_test>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser_blocks.cmake")

    add_executable(expr_test_driver
        tests/expr_driver.c
        src/expr.c
        src/lexer.c
        src/source.c
        src/cursor.c
        src/diagnostic.c)
    target_include_directories(expr_test_driver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME parser.multiple_statements COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:semantic_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser_multiple_statements.cmake")

    add_executable(program_growth_test
        tests/program_growth.c
        src/program.c
        src/expr.c
        src/lexer.c
        src/source.c
        src/cursor.c
        src/diagnostic.c)
    target_include_directories(program_growth_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME parser.program_growth COMMAND program_growth_test)

    add_executable(lexer_test_driver
        tests/lexer_driver.c
        src/lexer.c
        src/source.c
        src/cursor.c
        src/diagnostic.c)
    target_include_directories(lexer_test_driver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

    add_test(NAME parser.integer COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:expr_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser_integer.cmake")
    add_test(NAME parser.add_subtract COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:expr_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser_add_subtract.cmake")
    add_test(NAME parser.multiply_precedence COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:expr_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser_multiply_precedence.cmake")
    add_test(NAME parser.parentheses_precedence COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:expr_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/parser_parentheses_precedence.cmake")
    add_test(NAME lexer.literals COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:lexer_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/lexer_literals.cmake")
    add_test(NAME lexer.comments COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:lexer_test_driver>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/lexer_comments.cmake")
    
   
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Kenneth Looney")
set(CPACK_PACKAGE_CONTACT "ken.looney05@gmail.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME
    "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
# Prevent publishing a tag whose version differs from the packaged version.
if(DEFINED KASM_RELEASE_TAG AND NOT KASM_RELEASE_TAG STREQUAL "v${PROJECT_VERSION}")
    message(FATAL_ERROR "Release tag must be v${PROJECT_VERSION}, got ${KASM_RELEASE_TAG}")
endif()
include(CPack)
