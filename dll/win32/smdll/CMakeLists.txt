
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys)

spec2def(smdll.dll smdll.spec)

list(APPEND SOURCE
    dllmain.c
    query.c
    smdll.rc
    ${CMAKE_CURRENT_BINARY_DIR}/smdll.def)

add_library(smdll SHARED ${SOURCE})

target_link_libraries(smdll smlib)
set_entrypoint(smdll DllMainCRTStartup 12)
add_importlibs(smdll ntdll)
add_pch(smdll precomp.h)
add_dependencies(smdll psdk)
add_cd_file(TARGET smdll DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(smdll.spec)
