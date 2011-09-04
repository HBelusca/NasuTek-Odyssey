
include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl
    ${CMAKE_CURRENT_BINARY_DIR})

add_rpc_files(server ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl/svcctl.idl)


list(APPEND SOURCE
    config.c
    database.c
    driver.c
    groupdb.c
    rpcserver.c
    services.c
    services.rc
    ${CMAKE_CURRENT_BINARY_DIR}/svcctl_s.c)

add_executable(services ${SOURCE})

if(NOT MSVC)
    target_link_libraries(services pseh)
endif()

set_module_type(services win32cui UNICODE)
add_importlibs(services user32 advapi32 rpcrt4 msvcrt kernel32 ntdll)
add_pch(services services.h ${CMAKE_CURRENT_BINARY_DIR}/svcctl_s.h)
add_cd_file(TARGET services DESTINATION odyssey/system32 FOR all)
