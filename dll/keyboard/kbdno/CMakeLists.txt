
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdno.dll kbdno.spec)

add_library(kbdno SHARED kbdno.c kbdno.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdno.def)

set_entrypoint(kbdno 0)

add_dependencies(kbdno psdk)

add_cd_file(TARGET kbdno DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
