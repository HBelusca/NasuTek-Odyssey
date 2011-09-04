
add_definitions(
    -DUNICODE -D_UNICODE
    -DDEBUG_NT4=1)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/sound)

list(APPEND SOURCE
    detect.c
    registry.c
    control.c)

add_library(mment4 ${SOURCE})
add_pch(mment4 precomp.h)
add_dependencies(mment4 psdk)
