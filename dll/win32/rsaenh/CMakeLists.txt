
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(rsaenh.dll rsaenh.spec)

list(APPEND SOURCE
    aes.c
    des.c
    handle.c
    implglue.c
    md2.c
    mpi.c
    rc2.c
    rc4.c
    rsa.c
    rsaenh.c
    sha2.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/rsaenh.def)

add_library(rsaenh SHARED ${SOURCE})

set_module_type(rsaenh win32dll)
target_link_libraries(rsaenh wine)
add_importlibs(rsaenh msvcrt crypt32 advapi32 kernel32 ntdll)
add_pch(rsaenh tomcrypt.h)
add_importlib_target(rsaenh.spec)
add_cd_file(TARGET rsaenh DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
