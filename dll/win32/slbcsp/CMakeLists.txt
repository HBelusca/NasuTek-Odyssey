
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(slbcsp.dll slbcsp.spec)

list(APPEND SOURCE
    main.c
    ${CMAKE_CURRENT_BINARY_DIR}/slbcsp_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/slbcsp.def)

add_library(slbcsp SHARED ${SOURCE})

set_module_type(slbcsp win32dll)

target_link_libraries(slbcsp wine)

add_importlibs(slbcsp msvcrt kernel32 ntdll)

add_cd_file(TARGET slbcsp DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
