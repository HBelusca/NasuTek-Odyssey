
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(sccbase.dll sccbase.spec)

list(APPEND SOURCE
    main.c
    ${CMAKE_CURRENT_BINARY_DIR}/sccbase_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/sccbase.def)

add_library(sccbase SHARED ${SOURCE})

set_module_type(sccbase win32dll)

target_link_libraries(sccbase wine)

add_importlibs(sccbase msvcrt kernel32 ntdll)

add_cd_file(TARGET sccbase DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
