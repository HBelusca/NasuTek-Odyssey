
add_executable(svchost svchost.c svchost.rc)

set_module_type(svchost win32cui)
add_importlibs(svchost advapi32 msvcrt kernel32 ntdll)
add_cd_file(TARGET svchost DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
