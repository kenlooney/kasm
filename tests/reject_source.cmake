foreach(required KASM SOURCE_TEXT BITS EXPECTED_ERROR OUTPUT_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Pass -D${required}=<value> before -P")
    endif()
endforeach()

get_filename_component(KASM "${KASM}" ABSOLUTE)
get_filename_component(OUTPUT_DIR "${OUTPUT_DIR}" ABSOLUTE)
file(MAKE_DIRECTORY "${OUTPUT_DIR}")
set(source "${OUTPUT_DIR}/rejected.asm")
file(WRITE "${source}" "${SOURCE_TEXT}\n")

execute_process(
    COMMAND "${KASM}" --bits "${BITS}" "${source}"
    WORKING_DIRECTORY "${OUTPUT_DIR}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
    TIMEOUT 10)

if(status EQUAL 0)
    message(FATAL_ERROR "Invalid source was unexpectedly accepted: ${SOURCE_TEXT}")
endif()
string(CONCAT diagnostics "${output}\n${error}")
string(FIND "${diagnostics}" "${EXPECTED_ERROR}" error_position)
if(error_position LESS 0)
    message(FATAL_ERROR
        "Expected diagnostic '${EXPECTED_ERROR}' for '${SOURCE_TEXT}', got:\n${diagnostics}")
endif()
