
set_rc_compiler()

list(APPEND SOURCE
    misc.c
    logoff.c
    logoff.rc)

add_executable(logoff ${SOURCE})

set_module_type(logoff win32cui)
add_importlibs(logoff advapi32 user32 msvcrt kernel32)
add_pch(logoff precomp.h)
add_cd_file(TARGET logoff DESTINATION odyssey/system32 FOR all)
