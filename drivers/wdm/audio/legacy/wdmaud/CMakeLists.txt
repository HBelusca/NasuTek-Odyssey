
add_definitions(-D_COMDDK_)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/lib/drivers/sound/mmixer
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/sound)

add_library(wdmaud SHARED
    control.c
    deviface.c
    entry.c
    mmixer.c
    sup.c
    wdmaud.rc)

set_module_type(wdmaud kernelmodedriver)

target_link_libraries(wdmaud
    mmixer
    libcntpr
    ${PSEH_LIB})

add_pch(wdmaud wdmaud.h)
add_importlibs(wdmaud ntoskrnl ks hal)
add_cd_file(TARGET wdmaud DESTINATION odyssey/system32/drivers ODYSSEY_PE FOR all)