
spec2def(userenv.dll userenv.spec)

set_rc_compiler()

list(APPEND SOURCE
    desktop.c
    directory.c
    environment.c
    gpolicy.c
    misc.c
    profile.c
    registry.c
    setup.c
    userenv.c
    userenv.rc
    ${CMAKE_CURRENT_BINARY_DIR}/userenv_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/userenv.def)

add_library(userenv SHARED ${SOURCE})

set_module_type(userenv win32dll)

target_link_libraries(userenv uuid)

add_importlibs(userenv advapi32 user32 msvcrt kernel32 ntdll)
add_pch(userenv precomp.h)

add_cd_file(TARGET userenv DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(userenv.spec)
