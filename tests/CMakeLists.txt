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

add_executable(parentheses_test parentheses_test.cpp)
target_link_libraries(parentheses_test PRIVATE kasmcore)
add_test(NAME parentheses_test COMMAND parentheses_test)

add_executable(operator_alias_test operator_alias_test.cpp)
target_link_libraries(operator_alias_test PRIVATE kasmcore)
add_test(NAME operator_alias_test COMMAND operator_alias_test)

add_executable(subtraction_test subtraction_test.cpp)
target_link_libraries(subtraction_test PRIVATE kasmcore)
add_test(NAME subtraction_test COMMAND subtraction_test)

add_executable(expression_ast_test expression_ast_test.cpp)
target_link_libraries(expression_ast_test PRIVATE kasmcore)
add_test(NAME expression_ast_test COMMAND expression_ast_test)

add_executable(expression_comments_test expression_comments_test.cpp)
target_link_libraries(expression_comments_test PRIVATE kasmcore)
add_test(NAME expression_comments_test COMMAND expression_comments_test)

add_executable(statement_test statement_test.cpp)
target_link_libraries(statement_test PRIVATE kasmcore)
add_test(NAME statement_test COMMAND statement_test)

add_executable(move_alias_test move_alias_test.cpp)
target_link_libraries(move_alias_test PRIVATE kasmcore)
add_test(NAME move_alias_test COMMAND move_alias_test)

add_executable(move_statement_test move_statement_test.cpp)
target_link_libraries(move_statement_test PRIVATE kasmcore)
add_test(NAME move_statement_test COMMAND move_statement_test)

add_executable(declaration_test declaration_test.cpp)
target_link_libraries(declaration_test PRIVATE kasmcore)
add_test(NAME declaration_test COMMAND declaration_test)

add_executable(semantic_declaration_test semantic_declaration_test.cpp)
target_link_libraries(semantic_declaration_test PRIVATE kasmcore)
add_test(NAME semantic_declaration_test COMMAND semantic_declaration_test)

add_executable(declaration_source_test declaration_source_test.cpp)
target_link_libraries(declaration_source_test PRIVATE kasmcore)
add_test(NAME declaration_source_test COMMAND declaration_source_test ${CMAKE_SOURCE_DIR}/examples/declare.asm)

add_executable(bin_to_file_test bin_to_file_test.cpp)
target_link_libraries(bin_to_file_test PRIVATE kasmcore)
add_test(NAME bin_to_file_test COMMAND bin_to_file_test ${CMAKE_SOURCE_DIR}/examples/declare.asm ${CMAKE_SOURCE_DIR}/bin/output.bin)
