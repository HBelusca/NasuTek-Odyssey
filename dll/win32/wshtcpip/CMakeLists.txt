

include_directories(${ODYSSEY_SOURCE_DIR}/lib/tdilib)

spec2def(wshtcpip.dll wshtcpip.spec)

list(APPEND SOURCE
    wshtcpip.c
    wshtcpip.rc
    ${CMAKE_CURRENT_BINARY_DIR}/wshtcpip.def)

add_library(wshtcpip SHARED ${SOURCE})

set_module_type(wshtcpip win32dll UNICODE ENTRYPOINT 0)

target_link_libraries(wshtcpip tdilib)
add_importlibs(wshtcpip ws2_32 kernel32 ntdll)
add_dependencies(wshtcpip psdk)
add_cd_file(TARGET wshtcpip DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
