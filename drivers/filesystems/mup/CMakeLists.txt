
add_library(mup SHARED
    create.c
    mup.c
    mup.rc)

set_module_type(mup kernelmodedriver)
add_importlibs(mup ntoskrnl hal)
add_pch(mup mup.h)
add_cd_file(TARGET mup DESTINATION odyssey/system32/drivers ODYSSEY_PE FOR all)