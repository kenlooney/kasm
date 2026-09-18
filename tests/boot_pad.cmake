# Validate a boot-sector-like image built from the location-aware TIMES padding expression.
foreach(required KASM SOURCE OUTPUT_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Pass -D${required}=<path> before -P")
    endif()
endforeach()

get_filename_component(KASM "${KASM}" ABSOLUTE)
get_filename_component(SOURCE "${SOURCE}" ABSOLUTE)
get_filename_component(OUTPUT_DIR "${OUTPUT_DIR}" ABSOLUTE)
get_filename_component(source_name "${SOURCE}" NAME_WLE)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(REMOVE "${OUTPUT_DIR}/${source_name}.bin")

execute_process(
    COMMAND "${KASM}" "--bits" "64" "${SOURCE}"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    TIMEOUT 10)

if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Encoding ${SOURCE} failed: exit ${status}: ${error}")
endif()

if(NOT EXISTS "${OUTPUT_DIR}/${source_name}.bin")
    message(FATAL_ERROR "Assembler did not create ${source_name}.bin")
endif()

file(READ "${OUTPUT_DIR}/${source_name}.bin" actual_hex HEX)
string(TOUPPER "${actual_hex}" actual_hex)
string(LENGTH "${actual_hex}" hex_length)
math(EXPR actual_len "${hex_length} / 2")
if(NOT actual_len EQUAL 512)
    message(FATAL_ERROR "Expected 512 bytes, got ${actual_len}")
endif()

string(REPEAT "00" 505 zeros)
string(CONCAT expected_hex "B82A000000" "${zeros}" "55AA")
if(NOT "${actual_hex}" STREQUAL "${expected_hex}")
    message(FATAL_ERROR "Expected ${expected_hex}, got ${actual_hex}")
endif()

message(STATUS "boot_pad.asm encoded to 512-byte image")
