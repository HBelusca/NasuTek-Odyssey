

spec2def(wshirda.dll wshirda.spec)

list(APPEND SOURCE
    wshirda.c
    wshirda.rc
    ${CMAKE_CURRENT_BINARY_DIR}/wshirda.def)

add_library(wshirda SHARED ${SOURCE})

set_module_type(wshirda win32dll UNICODE)
add_importlibs(wshirda ws2_32 msvcrt kernel32 ntdll)
add_cd_file(TARGET wshirda DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
