
include_directories( ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl)

add_rpc_files(client ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl/wlansvc.idl)

spec2def(wlanapi.dll wlanapi.spec)

list(APPEND SOURCE
    main.c
    ${CMAKE_CURRENT_BINARY_DIR}/wlanapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/wlanapi.def
    ${CMAKE_CURRENT_BINARY_DIR}/wlansvc_c.c)

add_library(wlanapi SHARED ${SOURCE})

set_entrypoint(wlanapi 0)

target_link_libraries(wlanapi
    wine
    ${PSEH_LIB})

add_importlibs(wlanapi rpcrt4 msvcrt kernel32 ntdll)
add_importlib_target(wlanapi.spec)

add_dependencies(wlanapi psdk)
add_cd_file(TARGET wlanapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
