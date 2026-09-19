cmake_minimum_required(VERSION 3.20)
project(kasm VERSION 0.36.0 LANGUAGES C)

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
    add_test(NAME elf.cli COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/exit42.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/elf-cli/$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/elf_cli.cmake")
    add_executable(coff_writer_test tests/coff_writer.c src/coff.c src/bytes.c src/binary.c)
    target_include_directories(coff_writer_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/coff-tests")
    add_test(NAME coff.writer COMMAND coff_writer_test "${CMAKE_CURRENT_BINARY_DIR}/coff-tests/relocation.obj")
    add_test(NAME coff.cli COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/coff-cli/$<CONFIG>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/coff_cli.cmake")
    if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(coff_dir "${CMAKE_CURRENT_BINARY_DIR}/coff-link/$<CONFIG>")
        add_custom_command(OUTPUT "${coff_dir}/coff_answer.obj"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${coff_dir}"
            COMMAND "${CMAKE_COMMAND}" -E chdir "${coff_dir}" "$<TARGET_FILE:kasm>"
                --format coff --export answer -o "${coff_dir}/coff_answer.obj"
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/coff_answer.asm"
            DEPENDS kasm "${CMAKE_CURRENT_SOURCE_DIR}/examples/coff_answer.asm" VERBATIM)
        add_custom_command(OUTPUT "${coff_dir}/external.obj"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${coff_dir}"
            COMMAND "$<TARGET_FILE:coff_writer_test>" "${coff_dir}/external.obj"
            DEPENDS coff_writer_test VERBATIM)
        set_source_files_properties("${coff_dir}/coff_answer.obj" "${coff_dir}/external.obj"
            PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE)
        add_executable(coff_link_test tests/coff_caller.c "${coff_dir}/coff_answer.obj")
        add_executable(coff_external_link_test tests/coff_external_caller.c "${coff_dir}/external.obj")
        add_test(NAME coff.linked_answer COMMAND coff_link_test)
        add_test(NAME coff.linked_external COMMAND coff_external_link_test)
    endif()
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
    add_test(NAME encode.mode16_mov COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/mode16_mov.asm"
        "-DBITS=16"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/mode16_mov"
        "-DEXPECTED_HEX=B8 2A 00 B9 07 00 BA 03 00"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/mode16_mov_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME encode.mode16_sbb COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/mode16_sbb.asm"
        "-DBITS=16"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/mode16_sbb"
        "-DEXPECTED_HEX=81 D8 01 00 81 D9 02 00 81 DA 03 00"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/mode16_sbb_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME encode.control COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/control.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/control"
        "-DEXPECTED_HEX=F4 F3 90"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/control_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME encode.exit42 COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/exit42.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/exit42"
        "-DEXPECTED_HEX=B8 3C 00 00 00 BF 2A 00 00 00 0F 05"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/exit42_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    function(add_rejected_source_test name source_text bits expected_error)
        add_test(NAME semantic.${name} COMMAND "${CMAKE_COMMAND}"
            "-DKASM=$<TARGET_FILE:kasm>"
            "-DSOURCE_TEXT=${source_text}"
            "-DBITS=${bits}"
            "-DEXPECTED_ERROR=${expected_error}"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/rejected/$<CONFIG>/${name}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/reject_source.cmake")
    endfunction()
    add_rejected_source_test(syscall_operand "syscall 1;" 64 "expected semicolon")
    add_rejected_source_test(syscall_mode16 "syscall;" 16
        "syscall is only supported in 64-bit mode")
    add_rejected_source_test(edi_mode16 "mov edi,42;" 16
        "edi is only supported in 64-bit mode")
    add_test(NAME encode.mode16_segment COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/mode16_segment.asm"
        "-DBITS=16"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/mode16_segment"
        "-DEXPECTED_HEX=1E 1F 0F A0 0F A1 0F A8 0F A9"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/mode16_segment_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    foreach(data_case data_bytes data_jump data_words data_words_jump data_dwords data_qwords data_all_jump)
        if(data_case STREQUAL "data_bytes")
            set(data_hex "41 42 00 AA 55")
        elseif(data_case STREQUAL "data_jump")
            set(data_hex "E9 05 00 00 00 41 42 00 AA 55 B8 2A 00 00 00 C3")
        elseif(data_case STREQUAL "data_words")
            set(data_hex "34 12 00 00 FF FF AA 55 41 00")
        elseif(data_case STREQUAL "data_words_jump")
            set(data_hex "E9 06 00 00 00 41 34 12 00 FF FF B8 2A 00 00 00 C3")
        elseif(data_case STREQUAL "data_dwords")
            set(data_hex "00 00 00 00 FF FF FF 7F 78 56 34 12 2A 00 00 00")
        elseif(data_case STREQUAL "data_qwords")
            set(data_hex "00 00 00 00 00 00 00 00 FF FF FF 7F 00 00 00 00 78 56 34 12 00 00 00 00 2A 00 00 00 00 00 00 00")
        else()
            set(data_hex "E9 0F 00 00 00 41 34 12 2A 00 00 00 2A 00 00 00 00 00 00 00 B8 2A 00 00 00 C3")
        endif()
        add_test(NAME encode.${data_case} COMMAND "${CMAKE_COMMAND}"
            "-DKASM=$<TARGET_FILE:kasm>"
            "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/${data_case}.asm"
            "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/${data_case}"
            "-DEXPECTED_HEX=${data_hex}"
            "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/data_expected.txt"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    endforeach()
    add_test(NAME encode.data_symbols COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/data_symbols.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/data_symbols"
        "-DEXPECTED_HEX=03 00 00 C3"
        "-DEXPECTED_DECODE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/tests/data_expected.txt"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
    add_test(NAME semantic.data_invalid COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/data_invalid"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/data_invalid.cmake")
    add_test(NAME encode.times_fill COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/times_fill.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/times_fill"
        "-DEXPECTED_HEX=AA AA AA 34 12 00 00 34 12 00 00 2A 00 00 00 2A 00 00 00 2A 00 00 00 00 00 00 00 2A 00 00 00 00 00 00 00"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/encode_file.cmake")
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
    add_test(NAME semantic.reject_eax_in_mode16 COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/mov_42.asm"
        "-DBITS=16"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/reject_mode_16"
        "-DEXPECTED_ERROR=16-bit mode requires 16-bit registers: ax, cx, or dx"
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

    add_test(NAME encode.boot_pad COMMAND "${CMAKE_COMMAND}"
        "-DKASM=$<TARGET_FILE:kasm>"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/examples/boot_pad.asm"
        "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/examples/$<CONFIG>/boot_pad"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/boot_pad.cmake")

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
