
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()
spec2def(oleacc.dll oleacc.spec)

list(APPEND SOURCE
    main.c
    oleacc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/oleacc_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/oleacc.def)

add_library(oleacc SHARED ${SOURCE})

set_module_type(oleacc win32dll)

target_link_libraries(oleacc wine)

add_importlibs(oleacc user32 msvcrt kernel32 ntdll)

add_importlib_target(oleacc.spec)
add_cd_file(TARGET oleacc DESTINATION odyssey/system32 FOR all)
