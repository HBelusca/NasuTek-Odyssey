
set_rc_compiler()

list(APPEND SOURCE
    console.c
    misc.c
    mmc.c
    mmc.rc)

add_executable(mmcclient ${SOURCE})

set_module_type(mmcclient win32gui UNICODE)
add_importlibs(mmcclient user32 gdi32 comdlg32 advapi32 shell32 comctl32 msvcrt kernel32)
add_pch(mmcclient precomp.h)
