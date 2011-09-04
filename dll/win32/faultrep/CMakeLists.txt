
add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(faultrep.dll faultrep.spec)

list(APPEND SOURCE
    faultrep.c
    ${CMAKE_CURRENT_BINARY_DIR}/faultrep_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/faultrep.def)
    
add_library(faultrep SHARED ${SOURCE})

set_module_type(faultrep win32dll)
target_link_libraries(faultrep wine)
add_importlibs(faultrep advapi32 msvcrt kernel32 ntdll)
add_cd_file(TARGET faultrep DESTINATION odyssey/system32 FOR all)
