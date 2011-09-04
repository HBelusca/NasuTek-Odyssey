
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(softpub.dll softpub.spec)

add_library(softpub SHARED softpub.rc
    ${CMAKE_CURRENT_BINARY_DIR}/softpub.def)

set_entrypoint(softpub 0)

target_link_libraries(softpub wine)

add_importlibs(softpub wintrust)

add_cd_file(TARGET softpub DESTINATION odyssey/system32 FOR all)