
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)
set_rc_compiler()

spec2def(uxtheme.dll uxtheme.spec)

list(APPEND SOURCE
    buffer.c
    draw.c
    main.c
    metric.c
    msstyles.c
    property.c
    stylemap.c
    system.c
    uxini.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/uxtheme_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/uxtheme.def)

add_library(uxtheme SHARED ${SOURCE})
set_module_type(uxtheme win32dll)
target_link_libraries(uxtheme wine)
add_delay_importlibs(uxtheme msimg32)
add_importlibs(uxtheme user32 advapi32 gdi32 msvcrt kernel32 ntdll)
add_cd_file(TARGET uxtheme DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(uxtheme.spec)
