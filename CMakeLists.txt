cmake_minimum_required(VERSION 3.24)
project(kasm VERSION 0.1.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
include(CTest)
add_subdirectory(targets)

if(BUILD_TESTING AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_subdirectory(tests)
endif()
