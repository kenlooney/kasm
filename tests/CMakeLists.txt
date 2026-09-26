add_subdirectory(unit)

add_test(NAME kasm.cli.version COMMAND kasm)
set_tests_properties(kasm.cli.version PROPERTIES
    PASS_REGULAR_EXPRESSION "kasm [0-9]+\\.[0-9]+\\.[0-9]+"
)
