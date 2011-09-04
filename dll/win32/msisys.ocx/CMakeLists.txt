
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(msisys.ocx msisys.ocx.spec)

list(APPEND SOURCE
    msisys.c
    ${CMAKE_CURRENT_BINARY_DIR}/msisys.def)

add_library(msisys SHARED ${SOURCE})

set_module_type(msisys win32ocx)
target_link_libraries(msisys uuid wine)
add_importlibs(msisys msvcrt kernel32 ntdll)
add_cd_file(TARGET msisys DESTINATION odyssey/system32 FOR all)
