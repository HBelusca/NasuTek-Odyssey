
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(xinput9_1_0.dll xinput9_1_0.spec)

list(APPEND SOURCE
    xinput9_1_0_main.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/xinput9_1_0.def)

add_library(xinput9_1_0 SHARED ${SOURCE})

set_module_type(xinput9_1_0 win32dll)
add_importlibs(xinput9_1_0 msvcrt kernel32 xinput1_3)
add_cd_file(TARGET xinput9_1_0 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
