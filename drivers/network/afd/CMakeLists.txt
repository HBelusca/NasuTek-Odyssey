
include_directories(
    BEFORE include
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/drivers)

list(APPEND SOURCE
    afd/bind.c
    afd/connect.c
    afd/context.c
    afd/info.c
    afd/listen.c
    afd/lock.c
    afd/main.c
    afd/read.c
    afd/select.c
    afd/tdi.c
    afd/tdiconn.c
    afd/write.c
    afd.rc)

add_library(afd SHARED ${SOURCE})
target_link_libraries(afd ${PSEH_LIB})

set_module_type(afd kernelmodedriver)
add_importlibs(afd ntoskrnl hal)
add_pch(afd include/afd.h)
add_cd_file(TARGET afd DESTINATION odyssey/system32/drivers ODYSSEY_PE FOR all)
