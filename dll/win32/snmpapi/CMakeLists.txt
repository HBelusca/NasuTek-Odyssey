
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)
set_rc_compiler()

spec2def(snmpapi.dll snmpapi.spec)

list(APPEND SOURCE
    main.c
    snmpapi.rc
    ${CMAKE_CURRENT_BINARY_DIR}/snmpapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/snmpapi.def)

add_library(snmpapi SHARED ${SOURCE})

set_module_type(snmpapi win32dll)

target_link_libraries(snmpapi wine)

add_importlibs(snmpapi msvcrt kernel32 ntdll)

add_cd_file(TARGET snmpapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(snmpapi.spec)
