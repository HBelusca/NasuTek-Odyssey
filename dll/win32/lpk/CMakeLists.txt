

add_definitions(-DLANGPACK)
include_directories(include)

spec2def(lpk.dll lpk.spec)

list(APPEND SOURCE
    dllmain.c
    stub.c
    lpk.rc
    ${CMAKE_CURRENT_BINARY_DIR}/lpk.def)

add_library(lpk SHARED ${SOURCE})

set_module_type(lpk win32dll UNICODE)

add_importlib_target(lpk.spec)
add_importlibs(lpk user32 usp10 msvcrt kernel32 ntdll)
add_pch(lpk ros_lpk.h)
add_cd_file(TARGET lpk DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
