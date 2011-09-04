
add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(iccvid.dll iccvid.spec)

list(APPEND SOURCE
    iccvid.c
    rsrc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/iccvid.def)

add_library(iccvid SHARED ${SOURCE})

set_module_type(iccvid win32dll)
target_link_libraries(iccvid wine)
add_importlibs(iccvid user32 msvcrt kernel32 ntdll)
add_cd_file(TARGET iccvid DESTINATION odyssey/system32 FOR all)
