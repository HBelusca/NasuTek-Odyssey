
add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(compstui.dll compstui.spec)

list(APPEND SOURCE
    compstui_main.c
    ${CMAKE_CURRENT_BINARY_DIR}/compstui.def)

add_library(compstui SHARED ${SOURCE})

set_module_type(compstui win32dll)

target_link_libraries(compstui wine)

add_importlibs(compstui msvcrt kernel32 ntdll)
add_cd_file(TARGET compstui DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(compstui.spec)
