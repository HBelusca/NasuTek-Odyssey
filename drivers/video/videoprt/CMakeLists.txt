
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

add_definitions(-D_VIDEOPORT_)

spec2def(videoprt.sys videoprt.spec)

list(APPEND SOURCE
    agp.c
    ddc.c
    dispatch.c
    dma.c
    event.c
    funclist.c
    int10.c
    interrupt.c
    resource.c
    services.c
    spinlock.c
    stubs.c
    timer.c
    videoprt.c
    videoprt.rc
    ${CMAKE_CURRENT_BINARY_DIR}/videoprt.def)

add_library(videoprt SHARED ${SOURCE})

set_module_type(videoprt kernelmodedriver)
add_importlibs(videoprt ntoskrnl hal)

add_pch(videoprt videoprt.h)
add_cd_file(TARGET videoprt DESTINATION odyssey/system32/drivers ODYSSEY_PE FOR all)
add_importlib_target(videoprt.spec)
