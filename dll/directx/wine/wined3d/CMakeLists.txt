
add_definitions(
    -D__WINESRC__
    -D_USE_MATH_DEFINES
    -DUSE_WIN32_OPENGL)

include_directories(BEFORE ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

if(MSVC)
    add_definitions(-Disnan=_isnan)
    add_definitions(-Disinf=!_finite)
endif()

spec2def(wined3d.dll wined3d.spec)

list(APPEND SOURCE
    ati_fragment_shader.c
    arb_program_shader.c
    buffer.c
    clipper.c
    context.c
    device.c
    directx.c
    drawprim.c
    gl_compat.c
    glsl_shader.c
    nvidia_texture_shader.c
    palette.c
    query.c
    resource.c
    shader.c
    shader_sm1.c
    shader_sm4.c
    state.c
    stateblock.c
    surface.c
    swapchain.c
    texture.c
    utils.c
    vertexdeclaration.c
    view.c
    volume.c
    wined3d_main.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/wined3d.def)

if(ARCH MATCHES amd64)
list(APPEND SOURCE
     powf.c
     sqrtf.c)
endif()

add_library(wined3d SHARED ${SOURCE})
set_module_type(wined3d win32dll)

target_link_libraries(wined3d wine)

add_importlibs(wined3d
    user32
    opengl32
    gdi32
    advapi32
    msvcrt
    kernel32
    ntdll)

add_dependencies(wined3d wineheaders)
add_pch(wined3d wined3d_private.h)
add_cd_file(TARGET wined3d DESTINATION odyssey/system32 FOR all)
add_importlib_target(wined3d.spec)
