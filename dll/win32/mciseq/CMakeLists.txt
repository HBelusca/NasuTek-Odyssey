
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(mciseq.dll mciseq.spec)

list(APPEND SOURCE
    mcimidi.c
    ${CMAKE_CURRENT_BINARY_DIR}/mciseq.def)

add_library(mciseq SHARED ${SOURCE})

set_entrypoint(mciseq 0)
target_link_libraries(mciseq wine)
add_importlibs(mciseq winmm user32 kernel32 ntdll msvcrt)
add_dependencies(mciseq psdk)
add_cd_file(TARGET mciseq DESTINATION odyssey/system32 FOR all)
