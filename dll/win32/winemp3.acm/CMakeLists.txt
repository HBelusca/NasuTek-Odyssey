
add_definitions(
    -D__WINESRC__
    -DWIN32)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/libmpg123)

spec2def(winemp3.acm winemp3.acm.spec)

add_library(winemp3.acm SHARED
    mpegl3.c
    ${CMAKE_CURRENT_BINARY_DIR}/winemp3.def)

set_entrypoint(winemp3.acm 0)
set_target_properties(winemp3.acm PROPERTIES SUFFIX "")
target_link_libraries(winemp3.acm wine libmpg123 oldnames)

add_importlibs(winemp3.acm
    winmm
    user32
    msvcrt
    kernel32
    ntdll)

add_dependencies(winemp3.acm psdk)
add_cd_file(TARGET winemp3.acm DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
