
include_directories(
    include
    ${ODYSSEY_SOURCE_DIR}/lib/3rdparty/adns/src
    ${ODYSSEY_SOURCE_DIR}/lib/3rdparty/adns/adns_win32)

add_definitions(-DADNS_JGAA_WIN32)

spec2def(dnsapi.dll dnsapi.spec)

list(APPEND SOURCE
    dnsapi/adns.c
    dnsapi/context.c
    dnsapi/memory.c
    dnsapi/names.c
    dnsapi/query.c
    dnsapi/record.c
    dnsapi/stubs.c
    dnsapi.rc
    ${CMAKE_CURRENT_BINARY_DIR}/dnsapi.def)

add_library(dnsapi SHARED ${SOURCE})

set_entrypoint(dnsapi 0)
target_link_libraries(dnsapi adns)
add_importlibs(dnsapi user32 ws2_32 iphlpapi msvcrt kernel32 ntdll)
add_pch(dnsapi dnsapi/precomp.h)
add_dependencies(dnsapi psdk)
add_cd_file(TARGET dnsapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(dnsapi.spec)
