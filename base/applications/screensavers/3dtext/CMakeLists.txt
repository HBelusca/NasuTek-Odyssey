
set_rc_compiler()

add_executable(3dtext
    3dtext.c
    settings.c
    rsrc.rc)

set_module_type(3dtext win32gui UNICODE)
add_pch(3dtext 3dtext.h)
set_target_properties(3dtext PROPERTIES SUFFIX ".scr")
target_link_libraries(3dtext scrnsave)
add_importlibs(3dtext user32 gdi32 opengl32 glu32 advapi32 msvcrt kernel32)

add_cd_file(TARGET 3dtext DESTINATION odyssey/system32 FOR all)
