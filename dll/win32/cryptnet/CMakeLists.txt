
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(cryptnet.dll cryptnet.spec)

list(APPEND SOURCE
    cryptnet_main.c
    ${CMAKE_CURRENT_BINARY_DIR}/cryptnet_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/cryptnet.def)

add_library(cryptnet SHARED ${SOURCE})
set_module_type(cryptnet win32dll)
target_link_libraries(cryptnet wine)
add_delay_importlibs(cryptnet wininet)

add_importlibs(cryptnet
    crypt32
    msvcrt
    kernel32
    ntdll)

add_importlib_target(cryptnet.spec)
add_cd_file(TARGET cryptnet DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
