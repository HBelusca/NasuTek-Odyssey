
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(version.dll version.spec)

list(APPEND SOURCE
    info.c
    install.c
    resource.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/version.def)

add_library(version SHARED ${SOURCE})

set_entrypoint(version 0)

target_link_libraries(version wine)
add_importlibs(version lz32 kernel32 ntdll)
add_dependencies(version psdk)
add_cd_file(TARGET version DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(version.spec)
