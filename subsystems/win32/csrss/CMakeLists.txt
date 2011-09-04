
include_directories(
    include
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/drivers)

add_executable(csrss csrss.c csrss.rc)

set_module_type(csrss nativecui)
target_link_libraries(csrss nt)
add_importlibs(csrss ntdll csrsrv)
add_dependencies(csrss psdk bugcodes)
add_cd_file(TARGET csrss DESTINATION odyssey/system32 FOR all)

add_subdirectory(csrsrv)
add_subdirectory(win32csr)
