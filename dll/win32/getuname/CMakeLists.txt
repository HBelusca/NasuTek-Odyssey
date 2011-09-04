

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(getuname.dll getuname.spec)

add_library(getuname SHARED
    getuname.c
    getuname.rc
    ${CMAKE_CURRENT_BINARY_DIR}/getuname.def)

add_importlib_target(getuname.spec)
set_module_type(getuname win32dll UNICODE)
add_importlibs(getuname msvcrt kernel32 ntdll)
add_cd_file(TARGET getuname DESTINATION odyssey/system32 FOR all)
