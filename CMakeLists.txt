cmake_minimum_required(VERSION 3.20)
project(kasm VERSION 0.1.0 LANGUAGES C)

file(GLOB_RECURSE SOURCES CONFIG_DEPENDS "src/*.c")
add_executable(kasm
    ${SOURCES})
target_include_directories(kasm PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Keep Windows ZIPs runnable without a separate Visual C++ runtime install.
if(MSVC)
    set_property(TARGET kasm PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
# Install rules
install(TARGETS kasm RUNTIME DESTINATION bin)
install(FILES README.md LICENSE DESTINATION share/doc/kasm)

include(CTest)
if(BUILD_TESTING)
    add_test(NAME kasm.smoke COMMAND kasm "${CMAKE_CURRENT_SOURCE_DIR}/tests/sample.asm")
    set_tests_properties(kasm.smoke PROPERTIES
        PASS_REGULAR_EXPRESSION "1:1 byte 104\n1:2 byte 101\n1:3 byte 108\n1:4 byte 108\n1:5 byte 111"
        TIMEOUT 10)
    add_test(NAME kasm.walk COMMAND kasm "${CMAKE_CURRENT_SOURCE_DIR}/tests/walk.asm")
    set_tests_properties(kasm.walk PROPERTIES
        PASS_REGULAR_EXPRESSION "1:1 byte 97\n1:2 byte 10\n2:1 byte 98"
        TIMEOUT 10)
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Kenneth Looney")
set(CPACK_PACKAGE_CONTACT "ken.looney05@gmail.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME
    "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
# Prevent publishing a tag whose version differs from the packaged version.
if(DEFINED KASM_RELEASE_TAG AND NOT KASM_RELEASE_TAG STREQUAL "v${PROJECT_VERSION}")
    message(FATAL_ERROR "Release tag must be v${PROJECT_VERSION}, got ${KASM_RELEASE_TAG}")
endif()
include(CPack)
