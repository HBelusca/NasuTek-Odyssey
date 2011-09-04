
add_definitions(-D_IMAGEHLP_SOURCE_)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(imagehlp.dll imagehlp.spec)

list(APPEND SOURCE
    access.c
    imagehlp_main.c
    integrity.c
    modify.c
    imagehlp.rc
    ${CMAKE_CURRENT_BINARY_DIR}/imagehlp_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/imagehlp.def)

add_library(imagehlp SHARED ${SOURCE})

set_module_type(imagehlp win32dll)
target_link_libraries(imagehlp wine)
add_importlibs(imagehlp dbghelp msvcrt kernel32 ntdll)
add_importlib_target(imagehlp.spec)
add_pch(imagehlp precomp.h)
add_cd_file(TARGET imagehlp DESTINATION odyssey/system32 FOR all)
