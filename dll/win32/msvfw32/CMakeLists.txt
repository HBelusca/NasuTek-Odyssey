
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

add_definitions(-D__WINESRC__)
add_definitions(-D_WINE)

set_rc_compiler()
spec2def(msvfw32.dll msvfw32.spec)

list(APPEND SOURCE
    mciwnd.c
    msvideo_main.c
    drawdib.c
    rsrc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/msvfw32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/msvfw32.def)

add_library(msvfw32 SHARED ${SOURCE})

set_module_type(msvfw32 win32dll)

target_link_libraries(msvfw32 wine)

add_importlibs(msvfw32 winmm version comctl32 user32 gdi32 advapi32 msvcrt kernel32 ntdll)

add_cd_file(TARGET msvfw32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(msvfw32.spec)
