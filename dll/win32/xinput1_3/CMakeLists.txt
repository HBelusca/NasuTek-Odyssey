
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(xinput1_3.dll xinput1_3.spec)

list(APPEND SOURCE
    xinput1_3_main.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/xinput1_3.def)

add_library(xinput1_3 SHARED ${SOURCE})

add_importlib_target(xinput1_3.spec)
set_module_type(xinput1_3 win32dll)
target_link_libraries(xinput1_3 wine)
add_importlibs(xinput1_3 msvcrt kernel32 ntdll)
add_cd_file(TARGET xinput1_3 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
