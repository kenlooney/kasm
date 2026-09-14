set(path "${CMAKE_CURRENT_BINARY_DIR}/parser-multiply-precedence.asm")
file(WRITE "${path}" "2+3*4")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0")
    message(FATAL_ERROR "Parsing 2+3*4 failed: exit ${status}: ${error}")
endif()
if(NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Parsing 2+3*4 produced a diagnostic: ${error}")
endif()

# Expect 2+(3*4), with multiplication as the addition's right child.
string(REPLACE "\r\n" "\n" output "${output}")
string(CONCAT expected
    "node 0: kind=0 value=2 left=-1 right=-1\n"
    "node 1: kind=0 value=3 left=-1 right=-1\n"
    "node 2: kind=0 value=4 left=-1 right=-1\n"
    "node 3: kind=3 value=0 left=1 right=2\n"
    "node 4: kind=1 value=0 left=0 right=3\n"
    "root = 4\n")
if(NOT "${output}" STREQUAL "${expected}")
    message(FATAL_ERROR "Parsing 2+3*4 produced an unexpected AST: ${output}")
endif()
