
add_definitions(-D__WINESRC__)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(mlang.dll mlang.spec)

list(APPEND SOURCE
    mlang.c
    regsvr.c
    ${CMAKE_CURRENT_BINARY_DIR}/mlang_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/mlang.def)

add_library(mlang SHARED ${SOURCE})
set_entrypoint(mlang 0)
target_link_libraries(mlang uuid wine)
add_delay_importlibs(mlang oleaut32)

add_importlibs(mlang
    ole32
    user32
    gdi32
    advapi32
    kernel32
    ntdll)

add_importlib_target(mlang.spec)
add_dependencies(mlang psdk)
add_cd_file(TARGET mlang DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
