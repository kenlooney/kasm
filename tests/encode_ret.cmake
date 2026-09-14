set(path "${CMAKE_CURRENT_BINARY_DIR}/encode-ret.asm")
file(WRITE "${path}" "ret;")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Encoding ret; failed: exit ${status}: ${error}")
endif()

# RET without operands is a single C3 byte.
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT "${output}" STREQUAL "C3 \n")
    message(FATAL_ERROR "Expected RET encoding C3, got: ${output}")
endif()
