
add_library(pci SHARED
    fdo.c
    pci.c
    pdo.c
    pci.rc)

set_module_type(pci kernelmodedriver)
add_importlibs(pci ntoskrnl hal)
add_pch(pci pci.h)
add_cd_file(TARGET pci DESTINATION odyssey/system32/drivers NO_CAB ODYSSEY_PE FOR all)
