

spec2def(wing32.dll wing32.spec)

list(APPEND SOURCE
    wing32.c
    ${CMAKE_CURRENT_BINARY_DIR}/wing32.def)

add_library(wing32 SHARED ${SOURCE})

set_module_type(wing32 win32dll UNICODE ENTRYPOINT 0)

add_importlibs(wing32 user32 gdi32 msvcrt)
add_dependencies(wing32 psdk)
add_cd_file(TARGET wing32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
