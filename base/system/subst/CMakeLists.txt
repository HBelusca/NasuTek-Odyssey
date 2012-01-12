
set_rc_compiler()

add_executable(subst subst.c subst.rc)

set_module_type(subst win32cui)
add_importlibs(subst msvcrt kernel32 user32 ntdll)
add_cd_file(TARGET subst DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
