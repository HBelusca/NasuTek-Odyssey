
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(imaadp32.acm imaadp32.acm.spec)

list(APPEND SOURCE
    imaadp32.c
    ${CMAKE_CURRENT_BINARY_DIR}/imaadp32.def)

add_library(imaadp32.acm SHARED ${SOURCE})

set_entrypoint(imaadp32.acm 0)
set_target_properties(imaadp32.acm PROPERTIES SUFFIX "")
target_link_libraries(imaadp32.acm wine)
add_importlibs(imaadp32.acm winmm user32 msvcrt kernel32 ntdll)
add_dependencies(imaadp32.acm psdk)
add_cd_file(TARGET imaadp32.acm DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
