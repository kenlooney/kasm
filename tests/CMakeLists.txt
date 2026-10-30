# Source Test
add_executable(source_test source_test.cpp)
target_link_libraries(source_test PRIVATE asmfw)
add_test(NAME source_test COMMAND source_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Position Test
add_executable(position_test position_test.cpp)
target_link_libraries(position_test PRIVATE asmfw)
add_test(NAME position_test COMMAND position_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Diagnostic Test
add_executable(diagnostic_test diagnostic_test.cpp)
target_link_libraries(diagnostic_test PRIVATE asmfw)
add_test(NAME diagnostic_test COMMAND diagnostic_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Span Test
add_executable(span_test span_test.cpp)
target_link_libraries(span_test PRIVATE asmfw)
add_test(NAME span_test COMMAND span_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Lexer Test
add_executable(lexer_test lexer_test.cpp)
target_link_libraries(lexer_test PRIVATE kasmcore)
add_test(NAME lexer_test COMMAND lexer_test ${CMAKE_SOURCE_DIR}/examples/source.asm)

# Integer Test
add_executable(integer_test integer_test.cpp)
target_link_libraries(integer_test PRIVATE asmfw)
add_test(NAME integer_test COMMAND integer_test)

# Expression Test
add_executable(expression_test expression_test.cpp)
target_link_libraries(expression_test PRIVATE kasmcore)
add_test(NAME expression_test COMMAND expression_test ${CMAKE_SOURCE_DIR}/examples/expression.asm)
