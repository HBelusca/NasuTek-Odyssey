
add_definitions(-D__USE_W32_SOCKETS)

add_executable(finger 
    finger.c
    err.c
    getopt.c
    net.c
    finger.rc)

set_module_type(finger win32cui)
add_pch(finger precomp.h)
add_importlibs(finger ws2_32 msvcrt kernel32)
add_cd_file(TARGET finger DESTINATION odyssey/system32 FOR all)
