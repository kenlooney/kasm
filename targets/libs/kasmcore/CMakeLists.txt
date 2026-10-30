file(GLOB_RECURSE KASMCORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp")

add_library(kasmcore SHARED ${KASMCORE_SOURCES})
target_include_directories(kasmcore PUBLIC "include")
target_link_libraries(kasmcore PUBLIC asmfw)
