
add_definitions(
    -D__WINESRC__
    -D_WINE)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(mpr.dll mpr.spec)

list(APPEND SOURCE
    auth.c
    mpr_main.c
    multinet.c
    nps.c
    pwcache.c
    wnet.c
    mpr.rc
    ${CMAKE_CURRENT_BINARY_DIR}/mpr_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/mpr.def)

add_library(mpr SHARED ${SOURCE})

set_module_type(mpr win32dll)

target_link_libraries(mpr wine)
add_importlibs(mpr advapi32 user32 msvcrt kernel32 ntdll)
add_cd_file(TARGET mpr DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(mpr.spec)
