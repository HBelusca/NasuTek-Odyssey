
add_definitions(-D_BATTERYCLASS_)

spec2def(battc.sys battc.spec)

add_library(battc SHARED
            battc.c
            battc.rc
            ${CMAKE_CURRENT_BINARY_DIR}/battc.def)

set_module_type(battc kernelmodedriver)
add_importlibs(battc ntoskrnl hal)

add_cd_file(TARGET battc DESTINATION odyssey/system32/drivers ODYSSEY_PE FOR all)
add_importlib_target(battc.spec)
