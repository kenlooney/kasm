set(path "${CMAKE_CURRENT_BINARY_DIR}/semantic-expression.asm")
file(WRITE "${path}" "mov eax, (10+4)*3;")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Evaluating mov eax, (10+4)*3; failed: exit ${status}: ${error}")
endif()

# Parentheses make the addition evaluate before multiplication: 14 * 3 = 42.
string(REPLACE "\r\n" "\n" output "${output}")
set(expected "statements = 1\nvalue = 42\n")
if(NOT "${output}" STREQUAL "${expected}")
    message(FATAL_ERROR "Expected one statement with value 42, got: ${output}")
endif()
