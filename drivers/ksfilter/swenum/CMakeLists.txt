
list(APPEND SOURCE
    swenum.c)

add_library(swenum SHARED ${SOURCE})

set_module_type(swenum kernelmodedriver)
add_importlibs(swenum ks ntoskrnl hal)

add_cd_file(TARGET swenum DESTINATION odyssey/system32/drivers NO_CAB ODYSSEY_PE FOR all)
