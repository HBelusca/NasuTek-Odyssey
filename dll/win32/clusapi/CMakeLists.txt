
add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()
spec2def(clusapi.dll clusapi.spec)

list(APPEND SOURCE
    clusapi.c
    ${CMAKE_CURRENT_BINARY_DIR}/clusapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/clusapi.def)

add_library(clusapi SHARED ${SOURCE})

set_module_type(clusapi win32dll)

target_link_libraries(clusapi wine)

add_importlibs(clusapi msvcrt kernel32 ntdll)

add_cd_file(TARGET clusapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
