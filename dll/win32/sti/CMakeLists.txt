
remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

add_definitions(
    -D__WINESRC__
    -DENTRY_PREFIX=STI_
    -DPROXY_DELEGATION
    -DREGISTER_PROXY_DLL)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(sti.dll sti.spec)

list(APPEND SOURCE
    regsvr.c
    sti.c
    sti_main.c
    ${CMAKE_CURRENT_BINARY_DIR}/proxy.dlldata.c
    ${CMAKE_CURRENT_BINARY_DIR}/sti_wia_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/sti.def)

add_rpcproxy_files(sti_wia.idl)

add_library(sti SHARED ${SOURCE})

set_entrypoint(sti 0)

target_link_libraries(sti
    wine
    uuid
    ${PSEH_LIB})

add_importlibs(sti
    ole32
    oleaut32
    rpcrt4
    advapi32
    msvcrt
    kernel32
    ntdll)

add_cd_file(TARGET sti DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
