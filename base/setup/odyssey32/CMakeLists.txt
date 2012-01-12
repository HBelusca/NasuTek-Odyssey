
set_rc_compiler()

add_executable(odyssey32 odyssey.c odyssey.rc)

target_link_libraries(odyssey32 uuid)

set_module_type(odyssey32 win32gui UNICODE)
add_importlibs(odyssey32 gdi32 user32 comctl32 setupapi msvcrt kernel32 ntdll)
add_cd_file(TARGET odyssey32 DESTINATION odyssey NO_CAB FOR bootcd instcd)
