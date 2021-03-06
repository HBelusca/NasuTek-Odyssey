
include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl
    ${CMAKE_CURRENT_BINARY_DIR})
    
add_definitions(
    -D_SETUPAPI_
    -D_CFGMGR32_
    -Dwcsnicmp=_wcsnicmp)

add_rpc_files(client ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl/pnp.idl)

set_rc_compiler()

spec2def(setupapi.dll setupapi.spec)

list(APPEND SOURCE
    cfgmgr.c
    devclass.c
    devinst.c
    dirid.c
    diskspace.c
    driver.c
    install.c
    interface.c
    misc.c
    parser.c
    query.c
    queue.c
    setupcab.c
    stringtable.c
    stubs.c
    rpc.c
    setupapi.rc
    ${CMAKE_CURRENT_BINARY_DIR}/setupapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/setupapi.def
    ${CMAKE_CURRENT_BINARY_DIR}/pnp_c.c)

add_library(setupapi SHARED ${SOURCE})
set_module_type(setupapi win32dll UNICODE)
target_link_libraries(setupapi uuid wine ${PSEH_LIB})
add_delay_importlibs(setupapi shell32)

add_importlibs(setupapi
    msvcrt
    gdi32
    comctl32
    advapi32
    user32
    rpcrt4
    version
    kernel32
    ntdll)

add_pch(setupapi setupapi_private.h)
add_cd_file(TARGET setupapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(setupapi.spec)
