
set_rc_compiler()

add_executable(runonce runonce.c runonce.rc)

set_module_type(runonce win32gui UNICODE)
add_importlibs(runonce advapi32 user32 msvcrt kernel32 ntdll)
add_cd_file(TARGET runonce DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
