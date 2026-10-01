if(NOT DEFINED PROGRAM OR NOT DEFINED INPUT OR NOT DEFINED EXPECTED)
    message(FATAL_ERROR "PROGRAM, INPUT, and EXPECTED are required")
endif()

execute_process(
    COMMAND "${PROGRAM}" "${INPUT}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
)

set(combined_output "${output}${error}")

if(result EQUAL 0)
    message(FATAL_ERROR
        "Expected command to fail, but it succeeded:\n${combined_output}")
endif()

if(NOT combined_output MATCHES "${EXPECTED}")
    message(FATAL_ERROR
        "Expected output matching '${EXPECTED}', got:\n${combined_output}")
endif()

message(STATUS "Observed expected failure: ${combined_output}")
