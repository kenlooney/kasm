# Control fixture line endings independently of the host and Git settings.
function(check_comments name input newline_style expected_output)
    set(path "${CMAKE_CURRENT_BINARY_DIR}/lexer-comments-${name}.asm")
    if(input MATCHES "\n")
        file(CONFIGURE OUTPUT "${path}" CONTENT "${input}" @ONLY
            NEWLINE_STYLE ${newline_style})
    else()
        # Preserve EOF immediately after the comment, without a final newline.
        file(WRITE "${path}" "${input}")
    endif()
    execute_process(COMMAND "${KASM}" "${path}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
        TIMEOUT 10)
    string(REPLACE "\r\n" "\n" output "${output}")
    if(NOT "${status}" STREQUAL "0")
        message(FATAL_ERROR "${name}: exit ${status}: ${error}")
    endif()
    if(NOT "${error}" STREQUAL "")
        message(FATAL_ERROR "${name}: unexpected diagnostic: ${error}")
    endif()
    if(NOT "${output}" STREQUAL "${expected_output}")
        message(FATAL_ERROR "${name}: unexpected tokens: ${output}")
    endif()
endfunction()

check_comments(line_lf "42// ignored + 99\n+7" LF
    "token 12 [0,2) value=42\ntoken 1 [18,19) value=0\ntoken 12 [19,20) value=7\n")
check_comments(line_crlf "42// ignored + 99\n+7" CRLF
    "token 12 [0,2) value=42\ntoken 1 [19,20) value=0\ntoken 12 [20,21) value=7\n")
check_comments(line_eof "42// ignored" LF "token 12 [0,2) value=42\n")
check_comments(line_only "// ignored" LF "")
check_comments(block_lf "42/* a\nb */+7" LF
    "token 12 [0,2) value=42\ntoken 1 [11,12) value=0\ntoken 12 [12,13) value=7\n")
check_comments(block_crlf "42/* a\nb */+7" CRLF
    "token 12 [0,2) value=42\ntoken 1 [12,13) value=0\ntoken 12 [13,14) value=7\n")
check_comments(block_only "/* ignored\n + 99 */" LF "")
check_comments(block_eof "42/**/" LF "token 12 [0,2) value=42\n")
check_comments(adjacent "/**//*x*///y\n7" LF "token 12 [13,14) value=7\n")
check_comments(separate_identifiers "a/**/b" LF
    "token 11 [0,1) value=0\ntoken 11 [5,6) value=0\n")
