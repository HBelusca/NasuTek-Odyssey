
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(ntdsapi.dll ntdsapi.spec)

list(APPEND SOURCE
    ntdsapi.c
    ${CMAKE_CURRENT_BINARY_DIR}/ntdsapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/ntdsapi.def)

add_library(ntdsapi SHARED ${SOURCE})
set_entrypoint(ntdsapi 0)

target_link_libraries(ntdsapi wine)

add_importlibs(ntdsapi user32 kernel32 ntdll)
add_importlib_target(ntdsapi.spec)
add_dependencies(ntdsapi psdk)
add_cd_file(TARGET ntdsapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
