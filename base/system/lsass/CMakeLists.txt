

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys)

add_executable(lsass lsass.c lsass.rc)

set_module_type(lsass win32gui UNICODE)
add_importlibs(lsass advapi32 lsasrv msvcrt kernel32 ntdll)
add_cd_file(TARGET lsass DESTINATION odyssey/system32 FOR all)
