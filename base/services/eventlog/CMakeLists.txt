
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/idl
    ${CMAKE_CURRENT_BINARY_DIR})

add_rpc_files(server ${ODYSSEY_SOURCE_DIR}/include/odyssey/idl/eventlogrpc.idl)


list(APPEND SOURCE
    eventlog.c
    eventsource.c
    logport.c
    eventlog.rc
    rpc.c
    file.c
    ${CMAKE_CURRENT_BINARY_DIR}/eventlogrpc_s.c)

add_executable(eventlog ${SOURCE})

if(NOT MSVC)
    target_link_libraries(eventlog pseh)
endif()

set_module_type(eventlog win32cui UNICODE)
add_importlibs(eventlog advapi32 rpcrt4 msvcrt kernel32 ntdll)
add_pch(eventlog eventlog.h)
add_cd_file(TARGET eventlog DESTINATION odyssey/system32 FOR all)
