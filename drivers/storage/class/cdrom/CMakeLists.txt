
include_directories(..)

add_library(cdrom SHARED cdrom.c findscsi.c cdrom.rc)

set_module_type(cdrom kernelmodedriver)
add_importlibs(cdrom
    class2
    scsiport
    ntoskrnl
    hal)

add_pch(cdrom precomp.h)
add_cd_file(TARGET cdrom DESTINATION odyssey/system32/drivers NO_CAB ODYSSEY_PE FOR all)
