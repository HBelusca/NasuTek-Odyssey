
add_definitions(-D__WINESRC__)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(objsel.dll objsel.spec)

list(APPEND SOURCE
    factory.c
    objsel.c
    regsvr.c
    objsel.rc
    ${CMAKE_CURRENT_BINARY_DIR}/objsel.def)

add_library(objsel SHARED ${SOURCE})

set_module_type(objsel win32dll)

target_link_libraries(objsel uuid wine)

add_importlibs(objsel
    ole32
    user32
    advapi32
    msvcrt
    kernel32
    ntdll)

add_pch(objsel objsel_private.h)
add_cd_file(TARGET objsel DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
