
spec2def(kdcom.dll kdcom.spec)

add_library(kdcom SHARED
    kdcom.c
    kddll.c
    kdserial.c
    ${CMAKE_CURRENT_BINARY_DIR}/kdcom.def)

set_entrypoint(kdcom 0)
set_subsystem(kdcom native)
set_image_base(kdcom 0x00010000)
add_importlibs(kdcom ntoskrnl hal)
add_dependencies(kdcom psdk bugcodes)
add_importlib_target(kdcom.spec)
add_cd_file(TARGET kdcom DESTINATION odyssey/system32 NO_CAB ODYSSEY_PE FOR all)
