
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()
spec2def(shdoclc.dll shdoclc.spec)

add_library(shdoclc SHARED rsrc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/shdoclc.def)

set_entrypoint(shdoclc 0)

target_link_libraries(shdoclc wine)

add_importlibs(shdoclc kernel32 ntdll)
add_dependencies(shdoclc psdk)

add_cd_file(TARGET shdoclc DESTINATION odyssey/system32 ODYSSEY_PE FOR all)