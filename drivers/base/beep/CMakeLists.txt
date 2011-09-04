
add_library(beep SHARED beep.c beep.rc)
set_module_type(beep kernelmodedriver)
add_importlibs(beep ntoskrnl hal)
add_cd_file(TARGET beep DESTINATION odyssey/system32/drivers FOR all)
