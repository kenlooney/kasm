# Assemble a boot-sector source file and execute its image in emulator16.
foreach(required KASM EMULATOR SOURCE OUTPUT_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Pass -D${required}=<path> before -P")
    endif()
endforeach()

get_filename_component(KASM "${KASM}" ABSOLUTE)
get_filename_component(EMULATOR "${EMULATOR}" ABSOLUTE)
get_filename_component(SOURCE "${SOURCE}" ABSOLUTE)
get_filename_component(OUTPUT_DIR "${OUTPUT_DIR}" ABSOLUTE)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
file(REMOVE "${OUTPUT_DIR}/program.bin")

execute_process(
    COMMAND "${KASM}" "--bits" "16" "${SOURCE}"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE assemble_status
    OUTPUT_VARIABLE assemble_output
    ERROR_VARIABLE assemble_error
    TIMEOUT 10)
if(NOT "${assemble_status}" STREQUAL "0" OR NOT "${assemble_error}" STREQUAL "")
    message(FATAL_ERROR "Encoding ${SOURCE} failed: exit ${assemble_status}: ${assemble_error}")
endif()

if(NOT EXISTS "${OUTPUT_DIR}/program.bin")
    message(FATAL_ERROR "Assembler did not create program.bin")
endif()

execute_process(
    COMMAND "${EMULATOR}" "${OUTPUT_DIR}/program.bin"
    RESULT_VARIABLE emulator_status
    OUTPUT_VARIABLE emulator_output
    ERROR_VARIABLE emulator_error
    TIMEOUT 10)
if(NOT "${emulator_status}" STREQUAL "0")
    message(FATAL_ERROR "Boot image emulator failed: ${emulator_error}\n${emulator_output}")
endif()

message(STATUS "${emulator_output}")