
include_directories(${ODYSSEY_SOURCE_DIR}/subsystems/win32/csrss/include)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys)

spec2def(csrsrv.dll csrsrv.spec)

list(APPEND SOURCE
    api/process.c
    api/user.c
    api/wapi.c
    procsup.c
    thredsup.c
    init.c
    ${CMAKE_CURRENT_BINARY_DIR}/csrsrv.def)

add_library(csrsrv SHARED ${SOURCE})

target_link_libraries(csrsrv ${PSEH_LIB})

set_module_type(csrsrv nativedll)

add_importlibs(csrsrv ntdll smdll)

add_pch(csrsrv srv.h)

add_dependencies(csrsrv psdk bugcodes)
add_cd_file(TARGET csrsrv DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(csrsrv.spec)
