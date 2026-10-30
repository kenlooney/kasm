add_executable(kasm main.cpp)
target_include_directories(kasm PUBLIC "include")
# Link against the asmcore library
target_link_libraries(kasm PUBLIC asmcore)
if(MSVC)
  add_compile_options(/W4 /wd4996 /wd4200 /wd4127 /wd4146)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
  if(NOT WIN32)
    # chmod() in out/elf.c is POSIX, not C11
    add_compile_definitions(_POSIX_C_SOURCE=200809L)
  endif()
endif()