
add_definitions(
    -D__WINESRC__
    -D_WINE)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(mciwave.dll mciwave.spec)

list(APPEND SOURCE
    mciwave.c
    ${CMAKE_CURRENT_BINARY_DIR}/mciwave.def)

add_library(mciwave SHARED ${SOURCE})

set_entrypoint(mciwave 0)
target_link_libraries(mciwave wine)
add_importlibs(mciwave user32 winmm msvcrt kernel32 ntdll)
add_dependencies(mciwave psdk)
add_cd_file(TARGET mciwave DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
