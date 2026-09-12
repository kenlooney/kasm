cmake_minimum_required(VERSION 3.10.0)
project(kasm VERSION 0.1.0 LANGUAGES C)

add_executable(kasm main.c)
# Install rules
install(TARGETS kasm RUNTIME DESTINATION bin)
install(FILES README.md DESTINATION share/doc/kasm)

include(CTest)
enable_testing()

# CPack configuration
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Kenneth Looney")
set(CPACK_PACKAGE_CONTACT "ken.looney05@gmail.com")
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# Windows: ZIP + NSIS installer
if(WIN32)
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "HelloCPack Installer")
    set(CPACK_NSIS_CONTACT "kenneth@example.com")
endif()

# Linux: TGZ + DEB package
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kenneth") # required
endif()
include(CPack)
