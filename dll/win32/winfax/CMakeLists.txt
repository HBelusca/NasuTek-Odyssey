
spec2def(winfax.dll winfax.spec)

list(APPEND SOURCE
    winfax.c
    winfax.rc
    ${CMAKE_CURRENT_BINARY_DIR}/winfax.def)

add_library(winfax SHARED ${SOURCE})

set_entrypoint(winfax 0)

add_importlibs(winfax kernel32 ntdll)
add_dependencies(winfax psdk)
add_cd_file(TARGET winfax DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
