
set_rc_compiler()

add_executable(odyssey odyssey.c odyssey.rc)

target_link_libraries(odyssey uuid)

set_module_type(odyssey win32gui UNICODE)
add_importlibs(odyssey gdi32 user32 comctl32 setupapi msvcrt kernel32 ntdll)
add_cd_file(TARGET odyssey DESTINATION odyssey NO_CAB FOR bootcd)
