
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(spoolss.dll spoolss.spec)

list(APPEND SOURCE
    spoolss_main.c
    router.c
    ${CMAKE_CURRENT_BINARY_DIR}/spoolss_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/spoolss.def)

add_library(spoolss SHARED ${SOURCE})

set_module_type(spoolss win32dll)

target_link_libraries(spoolss wine)

add_importlibs(spoolss winspool msvcrt kernel32 ntdll)
add_importlib_target(spoolss.spec)

add_cd_file(TARGET spoolss DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
