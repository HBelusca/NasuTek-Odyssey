
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

spec2def(updspapi.dll updspapi.spec)

list(APPEND SOURCE
    main.c
    ${CMAKE_CURRENT_BINARY_DIR}/updspapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/updspapi.def)

add_library(updspapi SHARED ${SOURCE})
set_entrypoint(updspapi 0)

target_link_libraries(updspapi wine)

add_importlibs(updspapi setupapi kernel32 ntdll)
add_dependencies(updspapi psdk)

add_cd_file(TARGET updspapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
