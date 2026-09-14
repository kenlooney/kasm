set(path "${CMAKE_CURRENT_BINARY_DIR}/encode-mov-42.asm")
file(WRITE "${path}" "mov eax,42;")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Processing mov eax, 42; failed: exit ${status}: ${error}")
endif()

# B8 selects MOV EAX, imm32; 42 is encoded as four little-endian bytes.
string(REPLACE "\r\n" "\n" output "${output}")
if(NOT "${output}" STREQUAL "B8 2A 00 00 00 \n")
    message(FATAL_ERROR "Expected MOV encoding B8 2A 00 00 00, got: ${output}")
endif()
