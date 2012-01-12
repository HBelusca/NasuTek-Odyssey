
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(hlink.dll hlink.spec)

list(APPEND SOURCE
    browse_ctx.c
    extserv.c
    hlink_main.c
    link.c
    ${CMAKE_CURRENT_BINARY_DIR}/hlink_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/hlink.def)

add_library(hlink SHARED ${SOURCE})
set_module_type(hlink win32dll)
target_link_libraries(hlink uuid wine)
add_delay_importlibs(hlink urlmon)

add_importlibs(hlink
    shell32
    ole32
    advapi32
    msvcrt
    kernel32
    ntdll)

add_pch(hlink hlink_private.h)
add_importlib_target(hlink.spec)
add_cd_file(TARGET hlink DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
