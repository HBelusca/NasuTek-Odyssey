
spec2def(pciidex.sys pciidex.spec)

list(APPEND SOURCE
    fdo.c
    miniport.c
    misc.c
    pciidex.c
    pdo.c
    pciidex.rc
    ${CMAKE_CURRENT_BINARY_DIR}/pciidex.def)

add_library(pciidex SHARED ${SOURCE})
add_pch(pciidex pciidex.h)
set_module_type(pciidex kernelmodedriver)
add_importlibs(pciidex ntoskrnl)
add_cd_file(TARGET pciidex DESTINATION odyssey/system32/drivers NO_CAB ODYSSEY_PE FOR all)
add_importlib_target(pciidex.spec)
