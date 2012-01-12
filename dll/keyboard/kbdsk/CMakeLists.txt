
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdsk.dll kbdsk.spec)

add_library(kbdsk SHARED kbdsk.c kbdsk.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdsk.def)

set_entrypoint(kbdsk 0)

add_dependencies(kbdsk psdk)

add_cd_file(TARGET kbdsk DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
