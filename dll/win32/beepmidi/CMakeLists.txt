
spec2def(beepmidi.dll beepmidi.spec)

list(APPEND SOURCE
    beepmidi.c
    ${CMAKE_CURRENT_BINARY_DIR}/beepmidi.def)

add_library(beepmidi SHARED ${SOURCE})

set_module_type(beepmidi win32dll ENTRYPOINT 0 UNICODE)

add_importlibs(beepmidi winmm kernel32 ntdll msvcrt)
add_importlib_target(beepmidi.spec)

add_dependencies(beepmidi psdk)
add_cd_file(TARGET beepmidi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
