
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdest.dll kbdest.spec)

add_library(kbdest SHARED kbdest.c kbdest.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdest.def)

set_entrypoint(kbdest 0)

add_dependencies(kbdest psdk)

add_cd_file(TARGET kbdest DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
