
add_rpc_files(server
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine/epm.idl
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine/irot.idl)
    

list(APPEND SOURCE
    epmp.c
    irotp.c
    rpcss_main.c
    service_main.c
    rpcss.rc
    ${CMAKE_CURRENT_BINARY_DIR}/epm_s.c
    ${CMAKE_CURRENT_BINARY_DIR}/irot_s.c)

include_directories(${ODYSSEY_BINARY_DIR}/include/odyssey/wine)

add_executable(rpcss ${SOURCE})

target_link_libraries(rpcss
    ${PSEH_LIB}
    wine)

set_module_type(rpcss win32cui UNICODE)

add_importlibs(rpcss advapi32 rpcrt4 msvcrt kernel32 ntdll)
add_cd_file(TARGET rpcss DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
