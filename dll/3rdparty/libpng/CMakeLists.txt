
add_definitions(
    -DWIN32
    -DNDEBUG
    -D_WINDOWS
    -DPNG_BUILD_DLL)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/libpng
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/zlib)

add_library(libpng SHARED
    png.c
    pngerror.c
    pngget.c
    pngmem.c
    pngpread.c
    pngread.c
    pngrio.c
    pngrtran.c
    pngrutil.c
    pngset.c
    pngtest.c
    pngtrans.c
    pngwio.c
    pngwrite.c
    pngwtran.c
    pngwutil.c)

set_entrypoint(libpng 0)
target_link_libraries(libpng zlib)
add_importlibs(libpng msvcrt kernel32)
if(ARCH MATCHES amd64)
    add_importlibs(libpng ntdll)
endif()
add_dependencies(libpng psdk)
add_cd_file(TARGET libpng DESTINATION odyssey/system32 FOR all)
