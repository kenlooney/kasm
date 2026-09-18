set(path "${CMAKE_CURRENT_BINARY_DIR}/semantic-expression.asm")
file(WRITE "${path}" "mov eax, (10+4)*3;")

execute_process(COMMAND "${KASM}" "${path}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
    TIMEOUT 10)
if(NOT "${status}" STREQUAL "0" OR NOT "${error}" STREQUAL "")
    message(FATAL_ERROR "Evaluating mov eax, (10+4)*3; failed: exit ${status}: ${error}")
endif()

# Parentheses make the addition evaluate before multiplication: 14 * 3 = 42.
string(REPLACE "\r\n" "\n" output "${output}")
set(expected "statements = 1\nvalue = 42\n")
if(NOT "${output}" STREQUAL "${expected}")
    message(FATAL_ERROR "Expected one statement with value 42, got: ${output}")
endif()

foreach(case IN ITEMS add_overflow subtract_overflow multiply_overflow)
    set(path "${CMAKE_CURRENT_BINARY_DIR}/semantic-expression-${case}.asm")
    if(case STREQUAL "add_overflow")
        file(WRITE "${path}" "mov eax, 9223372036854775807+1;")
    elseif(case STREQUAL "subtract_overflow")
        file(WRITE "${path}" "mov eax, -9223372036854775807-2;")
    else()
        file(WRITE "${path}" "mov eax, 3037000500*3037000500;")
    endif()
    execute_process(COMMAND "${KASM}" "${path}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
        TIMEOUT 10)
    if("${status}" STREQUAL "0" OR
       NOT "${error}" MATCHES "expression outside signed 32-bit range")
        message(FATAL_ERROR "${case} was not rejected safely: exit ${status}: ${error}")
    endif()
endforeach()
