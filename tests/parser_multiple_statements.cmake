set(path "${CMAKE_CURRENT_BINARY_DIR}/parser-multiple-statements.asm")

# Newlines separate lines; each instruction still requires a semicolon.
foreach(newline IN ITEMS "\n" "\r\n")
    file(WRITE "${path}" "mov eax, 40+2;${newline}mov eax, 0b00000111;${newline}mov eax, 7;${newline}")
    execute_process(COMMAND "${KASM}" "${path}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
        TIMEOUT 10)
    if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
        message(FATAL_ERROR "Parsing multiple statements failed: exit ${status}: ${error}")
    endif()
    string(REPLACE "\r\n" "\n" output "${output}")
    if(NOT "${output}" STREQUAL "statements = 3\n")
        message(FATAL_ERROR "Expected three statements, got: ${output}")
    endif()
endforeach()

# Blank lines must work at top level and inside nested blocks.
file(WRITE "${path}" "\n{\n\nmov eax, 40+2;\n{\nmov eax, 0b00000111;\n}\n\n}\nmov eax, 7;\n")
execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Parsing multiline blocks failed: exit ${status}: ${error}")
endif()
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT "${output}" STREQUAL "statements = 3\n")
    message(FATAL_ERROR "Expected three statements across nested blocks, got: ${output}")
endif()

# The original sample is missing the second statement's semicolon.
file(WRITE "${path}" "mov eax, 40+2;\nmov eax, 0b00000111\nmov eax, 7;\n")
execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "1" OR NOT "${error}" MATCHES "expected semicolon")
    message(FATAL_ERROR "Expected a missing-semicolon error, got exit ${status}: ${error}")
endif()
if(NOT "${output}" STREQUAL "")
    message(FATAL_ERROR "Invalid statements produced success output: ${output}")
endif()
