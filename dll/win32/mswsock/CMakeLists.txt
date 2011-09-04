

add_definitions(-DLE)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(mswsock.dll mswsock.spec)

list(APPEND SOURCE
    extensions.c
    stubs.c
    mswsock.rc
    ${CMAKE_CURRENT_BINARY_DIR}/mswsock.def)

add_library(mswsock SHARED ${SOURCE})

set_module_type(mswsock win32dll UNICODE)
add_importlibs(mswsock ws2_32 msvcrt kernel32)
add_pch(mswsock precomp.h)
add_cd_file(TARGET mswsock DESTINATION odyssey/system32 FOR all)
add_importlib_target(mswsock.spec)
