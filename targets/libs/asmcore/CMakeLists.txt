file(GLOB_RECURSE ASMCORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp")
add_library(asmcore SHARED ${ASMCORE_SOURCES})
set_target_properties(asmcore PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(asmcore PUBLIC "include")
