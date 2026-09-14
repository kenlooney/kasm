set(path "${CMAKE_CURRENT_BINARY_DIR}/parser-add-subtract.asm")
file(WRITE "${path}" "0b00000001+0b00000010-0b00000011")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0")
    message(FATAL_ERROR "Parsing 1+2-3 failed: exit ${status}: ${error}")
endif()
if(NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Parsing 1+2-3 produced a diagnostic: ${error}")
endif()

# Expect (1+2)-3, with the addition as the subtraction's left child.
string(REPLACE "\r\n" "\n" output "${output}")
string(CONCAT expected
    "node 0: kind=0 value=1 left=-1 right=-1\n"
    "node 1: kind=0 value=2 left=-1 right=-1\n"
    "node 2: kind=1 value=0 left=0 right=1\n"
    "node 3: kind=0 value=3 left=-1 right=-1\n"
    "node 4: kind=2 value=0 left=2 right=3\n"
    "root = 4\n")
if(NOT "${output}" STREQUAL "${expected}")
    message(FATAL_ERROR "Parsing 1+2-3 produced an unexpected AST: ${output}")
endif()
