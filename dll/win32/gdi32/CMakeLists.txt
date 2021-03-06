

add_definitions(-DLANGPACK)

include_directories(include)

spec2def(gdi32.dll gdi32.spec)

if(ARCH MATCHES i386)
    list(APPEND SOURCE
        objects/efloat.c)
endif()

list(APPEND SOURCE
    main/dllmain.c
    misc/heap.c
    misc/gdientry.c
    misc/hacks.c
    misc/historic.c
    misc/misc.c
    misc/stubs.c
    misc/stubsa.c
    misc/stubsw.c
    misc/wingl.c
    objects/arc.c
    objects/bitmap.c
    objects/brush.c
    objects/coord.c
    objects/dc.c
    objects/eng.c
    objects/enhmfile.c
    objects/font.c
    objects/icm.c
    objects/linedda.c
    objects/metafile.c
    objects/painting.c
    objects/printdrv.c
    objects/palette.c
    objects/pen.c
    objects/region.c
    objects/text.c
    objects/utils.c
    objects/path.c
    gdi32.rc
    ${CMAKE_CURRENT_BINARY_DIR}/gdi32.def)

add_library(gdi32 SHARED ${SOURCE})

set_module_type(gdi32 win32dll UNICODE)

target_link_libraries(gdi32
    win32ksys
    dxguid
    ${PSEH_LIB})

add_importlibs(gdi32 user32 advapi32 msvcrt kernel32 ntdll)
add_pch(gdi32 include/precomp.h)
add_cd_file(TARGET gdi32 DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
add_importlib_target(gdi32.spec)
