
spec2def(ufat.dll ufat.spec)

list(APPEND SOURCE
    ufat.c
    ufat.rc
    ${CMAKE_CURRENT_BINARY_DIR}/ufat.def)

add_library(ufat SHARED ${SOURCE})

set_module_type(ufat nativedll)
target_link_libraries(ufat vfatlib)
add_importlibs(ufat ntdll)
add_cd_file(TARGET ufat DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
