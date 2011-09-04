
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdgeo.dll kbdgeo.spec)

add_library(kbdgeo SHARED kbdgeo.c kbdgeo.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdgeo.def)

set_entrypoint(kbdgeo 0)

add_dependencies(kbdgeo psdk)

add_cd_file(TARGET kbdgeo DESTINATION odyssey/system32 FOR all)
