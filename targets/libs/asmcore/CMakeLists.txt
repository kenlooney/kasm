file(GLOB_RECURSE ASMCORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp")
add_library(asmcore ${ASMCORE_SOURCES})
target_include_directories(asmcore PUBLIC "include")