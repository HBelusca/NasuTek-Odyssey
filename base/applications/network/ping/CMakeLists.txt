
add_definitions(-D__USE_W32_SOCKETS)

set_rc_compiler()

add_executable(ping ping.c ping.rc)

set_module_type(ping win32cui UNICODE)
add_importlibs(ping ws2_32 msvcrt kernel32)

if(MSVC)
    add_importlibs(ping ntdll)
endif()

add_cd_file(TARGET ping DESTINATION odyssey/system32 FOR all)
