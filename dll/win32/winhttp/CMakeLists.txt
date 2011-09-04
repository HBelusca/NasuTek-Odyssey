
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(winhttp.dll winhttp.spec)

list(APPEND SOURCE
    cookie.c
    handle.c
    main.c
    net.c
    request.c
    session.c
    url.c
    ${CMAKE_CURRENT_BINARY_DIR}/winhttp.def)

add_library(winhttp SHARED ${SOURCE})

set_module_type(winhttp win32dll)
target_link_libraries(winhttp wine)
add_delay_importlibs(winhttp crypt32)
add_importlibs(winhttp shlwapi wininet ws2_32 msvcrt advapi32 kernel32 ntdll)
add_importlib_target(winhttp.spec)
add_cd_file(TARGET winhttp DESTINATION odyssey/system32 FOR all)
