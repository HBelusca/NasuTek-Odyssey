
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

set_rc_compiler()
spec2def(printui.dll printui.spec)

list(APPEND SOURCE
    printui.c
    printui.rc
    ${CMAKE_CURRENT_BINARY_DIR}/printui_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/printui.def)

add_library(printui SHARED ${SOURCE})
set_entrypoint(printui 0)

target_link_libraries(printui wine)

add_importlibs(printui shell32 kernel32 ntdll)
add_dependencies(printui psdk)
add_cd_file(TARGET printui DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
