# Control fixture line endings independently of the host and Git settings.
function(check_comments name input newline_style expected_output)
    set(path "${CMAKE_CURRENT_BINARY_DIR}/lexer-comments-${name}.asm")
    if(newline_style STREQUAL "CRLF")
        string(REPLACE "\n" "\r\n" input "${input}")
    endif()
    if(input MATCHES "\n")
        file(CONFIGURE OUTPUT "${path}" CONTENT "${input}" @ONLY NEWLINE_STYLE ${newline_style})
        if(NOT input MATCHES "\n$")
            string(LENGTH "${input}" start)
            if(newline_style STREQUAL "CRLF")
                math(EXPR end "${start} + 2")
            else()
                math(EXPR end "${start} + 1")
            endif()
            string(APPEND expected_output "token 13 [${start},${end}) value=0\n")
        endif()
    else()
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
    "token 12 [0,2) value=42\ntoken 13 [17,18) value=0\ntoken 1 [18,19) value=0\ntoken 12 [19,20) value=7\n")
check_comments(line_crlf "42// ignored + 99\n+7" CRLF
    "token 12 [0,2) value=42\ntoken 13 [17,19) value=0\ntoken 1 [19,20) value=0\ntoken 12 [20,21) value=7\n")
check_comments(line_eof "42// ignored" LF "token 12 [0,2) value=42\n")
check_comments(line_only "// ignored" LF "")
check_comments(block_lf "42/* a\nb */+7" LF
    "token 12 [0,2) value=42\ntoken 1 [11,12) value=0\ntoken 12 [12,13) value=7\n")
check_comments(block_crlf "42/* a\nb */+7" CRLF
    "token 12 [0,2) value=42\ntoken 1 [12,13) value=0\ntoken 12 [13,14) value=7\n")
check_comments(block_only "/* ignored\n + 99 */" LF "")
check_comments(block_eof "42/**/" LF "token 12 [0,2) value=42\n")
check_comments(adjacent "/**//*x*///y\n7" LF "token 13 [12,13) value=0\ntoken 12 [13,14) value=7\n")
check_comments(separate_identifiers "a/**/b" LF
    "token 11 [0,1) value=0\ntoken 11 [5,6) value=0\n")

check_comments(line_cr "42// ignored\r+7" LF
    "token 12 [0,2) value=42\ntoken 13 [12,13) value=0\ntoken 1 [13,14) value=0\ntoken 12 [14,15) value=7\n")
check_comments(blank_lines "\n\n" LF
    "token 13 [0,1) value=0\ntoken 13 [1,2) value=0\n")
string(REPEAT "/**/" 1024 many_comments)
check_comments(many "${many_comments}" LF "")

# Check the opening location and full span, including CR/CRLF line counting.
foreach(ending IN ITEMS "\n" "\r" "\r\n")
    set(path "${CMAKE_CURRENT_BINARY_DIR}/lexer-unterminated.asm")
    file(CONFIGURE OUTPUT "${path}" CONTENT "a${ending} /*oops" @ONLY NEWLINE_STYLE LF)
    if(ending STREQUAL "\r\n")
        file(CONFIGURE OUTPUT "${path}" CONTENT "a\n /*oops" @ONLY NEWLINE_STYLE CRLF)
    endif()
    execute_process(COMMAND "${KASM}" "${path}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(NOT "${status}" STREQUAL "1" OR
       NOT error MATCHES ":2:2: error: unterminated block comment")
        message(FATAL_ERROR "unterminated comment: exit ${status}: ${error}")
    endif()
endforeach()
foreach(input IN ITEMS "/*" "/*oops" "/**")
    set(path "${CMAKE_CURRENT_BINARY_DIR}/lexer-unterminated-eof.asm")
    file(WRITE "${path}" "${input}")
    string(LENGTH "${input}" end)
    execute_process(COMMAND "${KASM}" "${path}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    string(FIND "${error}" "[bytes 0..${end})" span_position)
    if(NOT "${status}" STREQUAL "1" OR span_position LESS 0 OR
       NOT error MATCHES ":1:1: error: unterminated block comment")
        message(FATAL_ERROR "unterminated EOF: exit ${status}: ${error}")
    endif()
endforeach()
