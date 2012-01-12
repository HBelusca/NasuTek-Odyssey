
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

set_rc_compiler()
spec2def(olecli32.dll olecli32.spec)

list(APPEND SOURCE
    olecli_main.c
    ${CMAKE_CURRENT_BINARY_DIR}/olecli32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/olecli32.def)

add_library(olecli32 SHARED ${SOURCE})
set_entrypoint(olecli32 0)

target_link_libraries(olecli32 wine)

add_importlibs(olecli32 ole32 gdi32 kernel32 ntdll)
add_dependencies(olecli32 psdk)
add_cd_file(TARGET olecli32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
