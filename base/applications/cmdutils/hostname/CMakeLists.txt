
add_executable(hostname hostname.c hostname.rc)

set_module_type(hostname win32cui)
add_importlibs(hostname msvcrt kernel32)
add_cd_file(TARGET hostname DESTINATION odyssey/system32 FOR all)
