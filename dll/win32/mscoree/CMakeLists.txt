
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(mscoree.dll mscoree.spec)

list(APPEND SOURCE
    corruntimehost.c
    mscoree_main.c
    ${CMAKE_CURRENT_BINARY_DIR}/mscoree_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/mscoree.def)

add_library(mscoree SHARED ${SOURCE})

set_module_type(mscoree win32dll)
target_link_libraries(mscoree wine uuid)
add_importlibs(mscoree advapi32 shell32 msvcrt kernel32 ntdll)
add_cd_file(TARGET mscoree DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
