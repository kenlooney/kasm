file(GLOB_RECURSE KASMCORE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "include/*.hpp")

add_library(kasmcore SHARED ${KASMCORE_SOURCES})
set_target_properties(kasmcore PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(kasmcore PUBLIC "include")
target_link_libraries(kasmcore PUBLIC asmfw)
