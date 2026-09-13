# Generate exact bytes so checkout line endings cannot alter these cases.
function(check_lexer name input expected_status expected_output)
    set(path "${CMAKE_CURRENT_BINARY_DIR}/lexer-${name}.asm")
    if(name STREQUAL "crlf")
        file(CONFIGURE OUTPUT "${path}" CONTENT "${input}" @ONLY NEWLINE_STYLE CRLF)
    else()
        file(CONFIGURE OUTPUT "${path}" CONTENT "${input}" @ONLY NEWLINE_STYLE LF)
    endif()
    execute_process(COMMAND "${KASM}" "${path}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    string(REPLACE "\r\n" "\n" output "${output}")
    if(NOT "${status}" STREQUAL "${expected_status}")
        message(FATAL_ERROR "${name}: exit ${status}: ${error}")
    endif()
    if(expected_status EQUAL 0)
        if(NOT "${output}" STREQUAL "${expected_output}")
            message(FATAL_ERROR "${name}: unexpected tokens: ${output}")
        endif()
    elseif(NOT error MATCHES "invalid character or integer out of range")
        message(FATAL_ERROR "${name}: missing diagnostic: ${error}")
    endif()
endfunction()


check_lexer(lf "abc_2 42\n0x2A 0b101010 052 0" 0
    "token 11 [0,5) value=0\ntoken 12 [6,8) value=42\ntoken 12 [9,13) value=42\ntoken 12 [14,22) value=42\ntoken 12 [23,26) value=42\ntoken 12 [27,28) value=0\n")
check_lexer(crlf "a\n\t42 \n" 0
    "token 11 [0,1) value=0\ntoken 12 [4,6) value=42\n")
check_lexer(max "9223372036854775807" 0
    "token 12 [0,19) value=9223372036854775807\n")
check_lexer(decimal_add "42+7" 0
    "token 12 [0,2) value=42\ntoken 1 [2,3) value=0\ntoken 12 [3,4) value=7\n")
check_lexer(binary_add "0b10100001 + 7" 0
    "token 12 [0,10) value=161\ntoken 1 [11,12) value=0\ntoken 12 [13,14) value=7\n")
foreach(invalid IN ITEMS 0x 0b 0b102 08 0xGG 123abc 9223372036854775808)
    check_lexer("invalid-${invalid}" "${invalid}" 1 "")
endforeach()
