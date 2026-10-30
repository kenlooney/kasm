# Source Test
add_executable(source_test source_test.cpp)
target_link_libraries(source_test PRIVATE asmcore)
add_test(NAME source_test COMMAND source_test ${CMAKE_SOURCE_DIR}/examples/source.asm)