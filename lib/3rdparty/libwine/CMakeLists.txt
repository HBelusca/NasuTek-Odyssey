
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

list(APPEND SOURCE
    config.c
    debug_ros.c
    loader.c
#    string.c implements _stricmp, already shipped with crt
)

add_library(wine ${SOURCE})

add_library(wineldr loader.c)
