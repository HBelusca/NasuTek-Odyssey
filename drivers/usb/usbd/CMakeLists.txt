
spec2def(usbd.sys usbd.spec)

add_library(usbd SHARED
    usbd.c
    usbd.rc
    ${CMAKE_CURRENT_BINARY_DIR}/usbd.def)

set_module_type(usbd kernelmodedriver)
add_importlibs(usbd ntoskrnl hal)
add_importlib_target(usbd.spec)
