
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(inetmib1.dll inetmib1.spec)

list(APPEND SOURCE
    main.c 
    ${CMAKE_CURRENT_BINARY_DIR}/inetmib1_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/inetmib1.def)

add_library(inetmib1 SHARED ${SOURCE})
set_module_type(inetmib1 win32dll)
target_link_libraries(inetmib1 wine)
add_delay_importlibs(inetmib1 iphlpapi)

add_importlibs(inetmib1
    msvcrt
    snmpapi
    kernel32
    ntdll)

add_cd_file(TARGET inetmib1 DESTINATION odyssey/system32 FOR all)
