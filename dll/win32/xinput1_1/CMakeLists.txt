
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(xinput1_1.dll xinput1_1.spec)

list(APPEND SOURCE
    xinput1_1_main.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/xinput1_1.def)

add_library(xinput1_1 SHARED ${SOURCE})

set_module_type(xinput1_1 win32dll)
add_importlibs(xinput1_1 msvcrt kernel32 xinput1_3)
add_cd_file(TARGET xinput1_1 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
