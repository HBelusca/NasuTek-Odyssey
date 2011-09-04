
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(traffic.dll traffic.spec)

list(APPEND SOURCE
    traffic_main.c
    ${CMAKE_CURRENT_BINARY_DIR}/traffic_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/traffic.def)

add_library(traffic SHARED ${SOURCE})

set_module_type(traffic win32dll)

target_link_libraries(traffic wine)

add_importlibs(traffic msvcrt kernel32 ntdll)
add_dependencies(traffic psdk)
add_cd_file(TARGET traffic DESTINATION odyssey/system32 FOR all)
