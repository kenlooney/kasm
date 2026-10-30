# Source Test
add_executable(source_test source_test.cpp)
target_link_libraries(source_test PRIVATE asmcore)
add_test(NAME source_test COMMAND source_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Position Test
add_executable(position_test position_test.cpp)
target_link_libraries(position_test PRIVATE asmcore)
add_test(NAME position_test COMMAND position_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Diagnostic Test
add_executable(diagnostic_test diagnostic_test.cpp)
target_link_libraries(diagnostic_test PRIVATE asmcore)
add_test(NAME diagnostic_test COMMAND diagnostic_test ${CMAKE_SOURCE_DIR}/examples/source.asm)