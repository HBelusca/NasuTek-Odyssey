

spec2def(opengl32.dll opengl32.spec)

list(APPEND SOURCE
    font.c
    gl.c
    opengl32.c
    wgl.c
    ${CMAKE_CURRENT_BINARY_DIR}/opengl32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/opengl32.def)

add_library(opengl32 SHARED ${SOURCE})

set_module_type(opengl32 win32dll UNICODE)

add_importlibs(opengl32
    msvcrt
    gdi32
    user32
    advapi32
    glu32
    kernel32
    ntdll)

add_pch(opengl32 opengl32.h)

add_cd_file(TARGET opengl32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(opengl32.spec)
