
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(wtsapi32.dll wtsapi32.spec)

list(APPEND SOURCE
    wtsapi32.c
    ${CMAKE_CURRENT_BINARY_DIR}/wtsapi32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/wtsapi32.def)

add_library(wtsapi32 SHARED ${SOURCE})

set_module_type(wtsapi32 win32dll)

target_link_libraries(wtsapi32 wine)

add_importlibs(wtsapi32 advapi32 msvcrt kernel32 ntdll)

add_cd_file(TARGET wtsapi32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
