add_subdirectory(unit)

add_test(NAME kasm.cli.greeting COMMAND kasm)
set_tests_properties(kasm.cli.greeting PROPERTIES
    PASS_REGULAR_EXPRESSION "Hello, Kasm!"
)
