add_executable(kasm_unit_tests test_kasm.c)
target_link_libraries(kasm_unit_tests PRIVATE kasm::core)

if(MSVC)
    target_compile_options(kasm_unit_tests PRIVATE /W4)
    if(KASM_WARNINGS_AS_ERRORS)
        target_compile_options(kasm_unit_tests PRIVATE /WX)
    endif()
    set_property(TARGET kasm_unit_tests PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
    target_compile_options(kasm_unit_tests PRIVATE -Wall -Wextra -Wpedantic)
    if(KASM_WARNINGS_AS_ERRORS)
        target_compile_options(kasm_unit_tests PRIVATE -Werror)
    endif()
endif()

add_test(NAME kasm.unit COMMAND kasm_unit_tests)
