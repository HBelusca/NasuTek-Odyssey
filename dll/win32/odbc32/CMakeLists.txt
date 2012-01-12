
add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(odbc32.dll odbc32.spec)

list(APPEND SOURCE
    proxyodbc.c
    ${CMAKE_CURRENT_BINARY_DIR}/odbc32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/odbc32.def)

add_library(odbc32 SHARED ${SOURCE})

set_module_type(odbc32 win32dll)

target_link_libraries(odbc32 wine)

add_importlibs(odbc32
    msvcrt
    advapi32
    kernel32
    ntdll)

add_cd_file(TARGET odbc32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
