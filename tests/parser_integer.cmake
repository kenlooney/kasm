set(path "${CMAKE_CURRENT_BINARY_DIR}/parser-integer.asm")
file(WRITE "${path}" "42")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0")
    message(FATAL_ERROR "Parsing 42 failed: exit ${status}: ${error}")
endif()
if(NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Parsing 42 produced a diagnostic: ${error}")
endif()

# Expect exactly one EX_INT node with no children, selected as the root.
string(REPLACE "\r\n" "\n" output "${output}")
set(expected "node 0: kind=0 value=42 left=-1 right=-1\nroot = 0\n")
if(NOT "${output}" STREQUAL "${expected}")
    message(FATAL_ERROR "Parsing 42 produced an unexpected AST: ${output}")
endif()
