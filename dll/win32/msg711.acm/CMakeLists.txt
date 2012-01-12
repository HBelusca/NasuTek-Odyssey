
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(msg711.acm msg711.acm.spec)

list(APPEND SOURCE
    msg711.c
    ${CMAKE_CURRENT_BINARY_DIR}/msg711.def)

add_library(msg711.acm SHARED ${SOURCE})

set_entrypoint(msg711.acm 0)
set_target_properties(msg711.acm PROPERTIES SUFFIX "")
target_link_libraries(msg711.acm wine)
add_importlibs(msg711.acm winmm user32 msvcrt kernel32 ntdll)
add_dependencies(msg711.acm psdk)
add_cd_file(TARGET msg711.acm DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
