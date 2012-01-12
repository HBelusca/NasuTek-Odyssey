

include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine
    ${ODYSSEY_SOURCE_DIR}/dll/win32/dhcpcsvc/include
    ${ODYSSEY_SOURCE_DIR}/lib/tdilib)

spec2def(iphlpapi.dll iphlpapi.spec)

list(APPEND SOURCE
    dhcp_odyssey.c
    ifenum_odyssey.c
    ipstats_odyssey.c
    iphlpapi_main.c
    media.c
    registry.c
    resinfo_odyssey.c
    route_odyssey.c
    iphlpapi.rc
    ${CMAKE_CURRENT_BINARY_DIR}/iphlpapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/iphlpapi.def)

add_library(iphlpapi SHARED ${SOURCE})

set_module_type(iphlpapi win32dll UNICODE)
target_link_libraries(iphlpapi wine tdilib)
add_importlibs(iphlpapi dhcpcsvc advapi32 ws2_32 msvcrt kernel32 ntdll)
add_pch(iphlpapi iphlpapi_private.h)
add_cd_file(TARGET iphlpapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(iphlpapi.spec)
