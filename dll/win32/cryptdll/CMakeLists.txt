
add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(cryptdll.dll cryptdll.spec)

list(APPEND SOURCE
    cryptdll.c
    stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/cryptdll.def)

add_library(cryptdll SHARED ${SOURCE})

set_module_type(cryptdll win32dll)
target_link_libraries(cryptdll wine)
add_importlibs(cryptdll advapi32 msvcrt kernel32 ntdll)
add_cd_file(TARGET cryptdll DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
