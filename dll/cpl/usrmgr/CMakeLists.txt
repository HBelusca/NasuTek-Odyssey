

include_directories(BEFORE ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(usrmgr.cpl usrmgr.spec)

add_library(usrmgr SHARED
    extra.c
    groupprops.c
    groups.c
    misc.c
    userprops.c
    users.c
    usrmgr.c
    usrmgr.rc
    ${CMAKE_CURRENT_BINARY_DIR}/usrmgr.def)

set_module_type(usrmgr cpl UNICODE)

add_importlibs(usrmgr
    advapi32
    user32
    gdi32
    comctl32
    netapi32
    msvcrt
    kernel32
    ntdll)

add_pch(usrmgr usrmgr.h)
add_cd_file(TARGET usrmgr DESTINATION odyssey/system32 FOR all)
