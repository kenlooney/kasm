# Validate the layout contract for a 16-bit boot-sector image.
foreach(required KASM SOURCE OUTPUT_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Pass -D${required}=<path> before -P")
    endif()
endforeach()

get_filename_component(KASM "${KASM}" ABSOLUTE)
get_filename_component(SOURCE "${SOURCE}" ABSOLUTE)
get_filename_component(OUTPUT_DIR "${OUTPUT_DIR}" ABSOLUTE)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(REMOVE "${OUTPUT_DIR}/program.bin")

execute_process(
    COMMAND "${KASM}" "--bits" "16" "${SOURCE}"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    TIMEOUT 10)

if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Encoding ${SOURCE} failed: exit ${status}: ${error}")
endif()

if(NOT EXISTS "${OUTPUT_DIR}/program.bin")
    message(FATAL_ERROR "Assembler did not create program.bin")
endif()

file(READ "${OUTPUT_DIR}/program.bin" actual_hex HEX)
string(TOUPPER "${actual_hex}" actual_hex)
string(LENGTH "${actual_hex}" hex_length)
math(EXPR actual_length "${hex_length} / 2")
if(NOT actual_length EQUAL 512)
    message(FATAL_ERROR "Boot image must be exactly 512 bytes, got ${actual_length}")
endif()

# The image starts with mov ax, 42; add ax, 3;.
string(SUBSTRING "${actual_hex}" 0 12 instruction_prefix)
if(NOT "${instruction_prefix}" STREQUAL "B82A00050300")
    message(FATAL_ERROR
        "Boot image must start with B8 2A 00 05 03 00, got ${instruction_prefix}")
endif()

string(SUBSTRING "${actual_hex}" 12 1008 data_area)
string(REPEAT "00" 504 zero_data)
if(NOT "${data_area}" STREQUAL "${zero_data}")
    message(FATAL_ERROR "Boot image data area must be zero-filled")
endif()

string(SUBSTRING "${actual_hex}" 1020 4 signature)
if(NOT "${signature}" STREQUAL "55AA")
    message(FATAL_ERROR "Boot image must end with 55 AA, got ${signature}")
endif()

message(STATUS "Validated 16-bit boot image: 512 bytes, zero-filled data, signature 55 AA")