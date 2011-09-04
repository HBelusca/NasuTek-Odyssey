
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys)

list(APPEND SOURCE
    compses.c
    connect.c
    execpgm.c
    lookupss.c)
    
add_library(smlib ${SOURCE})
add_pch(smlib precomp.h)
add_dependencies(smlib psdk)
