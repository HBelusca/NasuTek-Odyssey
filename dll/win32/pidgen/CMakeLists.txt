
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

set_rc_compiler()
spec2def(pidgen.dll pidgen.spec)

list(APPEND SOURCE
    main.c
    rsrc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/pidgen_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/pidgen.def)

add_library(pidgen SHARED ${SOURCE})

set_module_type(pidgen win32dll)

target_link_libraries(pidgen wine)

add_importlibs(pidgen msvcrt kernel32 ntdll)

add_cd_file(TARGET pidgen DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
