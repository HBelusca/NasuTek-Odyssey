
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdsw.dll kbdsw.spec)

add_library(kbdsw SHARED kbdsw.c kbdsw.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdsw.def)

set_entrypoint(kbdsw 0)

add_dependencies(kbdsw psdk)

add_cd_file(TARGET kbdsw DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
