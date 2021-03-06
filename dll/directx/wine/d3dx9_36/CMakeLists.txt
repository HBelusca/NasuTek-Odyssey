set_rc_compiler()

spec2def(d3dx9_36.dll d3dx9_36.spec)

list(APPEND SOURCE
    core.c
    d3dx9_36_main.c
    font.c
    math.c
    mesh.c
    shader.c
    sprite.c
    surface.c
    texture.c
    util.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/d3dx9_36_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/d3dx9_36.def)

add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

add_library(d3dx9_36 SHARED ${SOURCE})
set_module_type(d3dx9_36 win32dll)

target_link_libraries(d3dx9_36
    wine)

add_importlibs(d3dx9_36
    d3d9
    user32
    gdi32
    advapi32
    msvcrt
    kernel32
    ntdll)

add_dependencies(d3dx9_36 wineheaders)
add_cd_file(TARGET d3dx9_36 DESTINATION odyssey/system32 FOR all)
add_importlib_target(d3dx9_36.spec)
