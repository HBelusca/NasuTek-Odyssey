
add_definitions(-D__WINESRC__)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(comcat.dll comcat.spec)

list(APPEND SOURCE
    comcat_main.c
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/comcat.def)

add_library(comcat SHARED ${SOURCE})

set_entrypoint(comcat 0)

target_link_libraries(comcat uuid wine)

add_importlibs(comcat ole32 user32 advapi32 ntdll)

add_dependencies(comcat psdk)
add_cd_file(TARGET comcat DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
