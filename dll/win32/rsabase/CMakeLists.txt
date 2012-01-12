
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(rsabase.dll rsabase.spec)

list(APPEND SOURCE
    version.rc
    ${CMAKE_CURRENT_BINARY_DIR}/rsabase.def)

add_library(rsabase SHARED ${SOURCE})

set_entrypoint(rsabase 0)
target_link_libraries(rsabase wine)
add_importlibs(rsabase rsaenh ntdll)
add_cd_file(TARGET rsabase DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
