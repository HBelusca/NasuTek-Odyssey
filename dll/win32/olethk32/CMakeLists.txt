
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

set_rc_compiler()
spec2def(olethk32.dll olethk32.spec)

list(APPEND SOURCE
    main.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/olethk32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/olethk32.def)

add_library(olethk32 SHARED ${SOURCE})

set_module_type(olethk32 win32dll)

target_link_libraries(olethk32 wine)

add_importlibs(olethk32 msvcrt kernel32 ntdll)
add_dependencies(olethk32 psdk)
add_cd_file(TARGET olethk32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
