
add_definitions(
    -D__WINESRC__
    -D_WINE
    -Dclose=_close)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(wininet.dll wininet.spec)

set_rc_compiler()

list(APPEND SOURCE
    cookie.c
    dialogs.c
    ftp.c
    gopher.c
    http.c
    internet.c
    netconnection.c
    urlcache.c
    utility.c
    wininet_main.c
    rsrc.rc
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/wininet_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/wininet.def)

add_library(wininet SHARED ${SOURCE})

set_module_type(wininet win32dll)
target_link_libraries(wininet wine ${PSEH_LIB})
add_delay_importlibs(wininet secur32 crypt32)
add_importlibs(wininet mpr shlwapi shell32 user32 advapi32 ws2_32 msvcrt kernel32 ntdll)
add_cd_file(TARGET wininet DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(wininet.spec)
