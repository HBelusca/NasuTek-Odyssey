
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(netapi32.dll netapi32.spec)

list(APPEND SOURCE
    access.c
    apibuf.c
    browsr.c
    ds.c
    local_group.c
    nbcmdqueue.c
    nbnamecache.c
    nbt.c
    netapi32.c
    netbios.c
    share.c
    wksta.c
    ${CMAKE_CURRENT_BINARY_DIR}/netapi32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/netapi32.def)

add_library(netapi32 SHARED ${SOURCE})

set_module_type(netapi32 win32dll)

target_link_libraries(netapi32 wine)

add_importlibs(netapi32 iphlpapi ws2_32 advapi32 msvcrt kernel32 ntdll)

add_cd_file(TARGET netapi32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(netapi32.spec)
