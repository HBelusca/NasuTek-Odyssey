
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(sensapi.dll sensapi.spec)

list(APPEND SOURCE
    sensapi.c
    ${CMAKE_CURRENT_BINARY_DIR}/sensapi.def)

add_library(sensapi SHARED ${SOURCE})

set_module_type(sensapi win32dll)
target_link_libraries(sensapi wine)
add_importlibs(sensapi msvcrt kernel32 ntdll)
add_importlib_target(sensapi.spec)
add_cd_file(TARGET sensapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
