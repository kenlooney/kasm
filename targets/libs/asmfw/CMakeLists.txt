file(GLOB_RECURSE ASMCORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp")
add_library(asmfw SHARED ${ASMCORE_SOURCES})
set_target_properties(asmfw PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(asmfw PUBLIC "include")
