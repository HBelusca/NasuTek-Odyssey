
set_rc_compiler()

spec2def(intl.cpl intl.spec)

add_library(intl SHARED
    currency.c
    date.c
    generalp.c
    intl.c
    inplocale.c
    numbers.c
    time.c
    misc.c
    kblayouts.c
    languages.c
    advanced.c
    sort.c
    intl.rc)

set_module_type(intl cpl UNICODE)

add_importlibs(intl
    msvcrt
    user32
    comctl32
    advapi32
    setupapi
    shell32
    kernel32)

add_pch(intl intl.h)
add_cd_file(TARGET intl DESTINATION odyssey/system32 FOR all)
