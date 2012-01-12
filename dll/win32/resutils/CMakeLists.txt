
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(resutils.dll resutils.spec)

list(APPEND SOURCE
    resutils.c
    ${CMAKE_CURRENT_BINARY_DIR}/resutils_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/resutils.def)

add_library(resutils SHARED ${SOURCE})

set_module_type(resutils win32dll)

target_link_libraries(resutils wine)

add_importlibs(resutils msvcrt kernel32 ntdll)

add_cd_file(TARGET resutils DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
