
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(icmp.dll icmp.spec)

list(APPEND SOURCE
    icmp_main.c
    icmp.rc
    ${CMAKE_CURRENT_BINARY_DIR}/icmp_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/icmp.def)

add_library(icmp SHARED ${SOURCE})

set_module_type(icmp win32dll)

target_link_libraries(icmp wine)
add_importlibs(icmp ws2_32 msvcrt kernel32 ntdll)
add_importlib_target(icmp.spec)
add_cd_file(TARGET icmp DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
