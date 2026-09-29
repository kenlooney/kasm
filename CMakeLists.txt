cmake_minimum_required(VERSION 3.20)

project(
    kasm
    VERSION 0.12.0
    DESCRIPTION "A small x86 assembler written in C"
    HOMEPAGE_URL "https://github.com/kenlooney/kasm"
    LANGUAGES C
)

include(GNUInstallDirs)

option(KASM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
set(KASM_PACKAGE_VERSION "v${PROJECT_VERSION}" CACHE STRING
    "Version label used in package filenames")

set(KASM_GENERATED_INCLUDE_DIR "${PROJECT_BINARY_DIR}/generated/include")
set(KASM_GENERATED_VERSION_HEADER
    "${KASM_GENERATED_INCLUDE_DIR}/kasm/version.h")
file(MAKE_DIRECTORY "${KASM_GENERATED_INCLUDE_DIR}/kasm")
configure_file(
    include/kasm/version.h.in
    "${KASM_GENERATED_VERSION_HEADER}"
    @ONLY
)
file(GLOB_RECURSE KASM_SOURCE_FILES CONFIGURE_DEPENDS "src/*.c")
list(REMOVE_ITEM KASM_SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/main.c")
add_library(kasm_core STATIC
    ${KASM_SOURCE_FILES}
    "${KASM_GENERATED_VERSION_HEADER}"
)
add_library(kasm::core ALIAS kasm_core)

target_compile_features(kasm_core PUBLIC c_std_11)
target_include_directories(kasm_core
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${KASM_GENERATED_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_executable(kasm src/main.c)
target_link_libraries(kasm PRIVATE kasm::core)

foreach(target kasm_core kasm)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(KASM_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        if(KASM_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endforeach()

# Keep Windows ZIPs runnable without a separate Visual C++ runtime install.
if(MSVC)
    set_property(TARGET kasm PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set_property(TARGET kasm_core PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(TARGETS kasm RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES README.md LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# A release workflow passes the tag so a version mismatch cannot be published.
if(DEFINED KASM_RELEASE_TAG AND NOT KASM_RELEASE_TAG STREQUAL "v${PROJECT_VERSION}")
    message(FATAL_ERROR
        "Release tag must be v${PROJECT_VERSION}, got ${KASM_RELEASE_TAG}")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Kenneth Looney")
set(CPACK_PACKAGE_CONTACT "ken.looney05@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/packages")
set(CPACK_PACKAGE_FILE_NAME
    "${PROJECT_NAME}-${KASM_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
