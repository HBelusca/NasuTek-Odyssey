
list(APPEND SOURCE
    usbehci.c
    hardware.c
    physmem.c
    hwiface.c
    transfer.c
    fdo.c
    pdo.c
    common.c
    misc.c
    irp.c
    usbiffn.c
    usbehci.rc)

add_library(usbehci SHARED ${SOURCE})

set_module_type(usbehci kernelmodedriver)
add_importlibs(usbehci ntoskrnl hal)

add_cab_target(usbehci 2)
