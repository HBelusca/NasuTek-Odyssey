
add_definitions(
    -D__WINESRC__
    -D_USE_MATH_DEFINES)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(gdiplus.dll gdiplus.spec)

list(APPEND SOURCE
    brush.c
    customlinecap.c
    font.c
    gdiplus.c
    graphics.c
    graphicspath.c
    image.c
    imageattributes.c
    matrix.c
    pathiterator.c
    pen.c
    region.c
    stringformat.c
    ${CMAKE_CURRENT_BINARY_DIR}/gdiplus_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/gdiplus.def)

if(MSVC)
    set_source_files_properties(${ODYSSEY_BINARY_DIR}/include/psdk/wincodec_i.c PROPERTIES GENERATED TRUE)
    list(APPEND SOURCE
        ${ODYSSEY_BINARY_DIR}/include/psdk/wincodec_i.c)
endif()

add_library(gdiplus SHARED ${SOURCE})
set_module_type(gdiplus win32dll)
target_link_libraries(gdiplus uuid wine)
add_delay_importlibs(gdiplus windowscodecs)

add_importlibs(gdiplus
    msvcrt
    shlwapi
    oleaut32
    ole32
    user32
    gdi32
    kernel32
    ntdll)

add_importlib_target(gdiplus.spec)

if(ARCH MATCHES amd64)
    target_link_libraries(gdiplus crt)
endif()

add_cd_file(TARGET gdiplus DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
