
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdarmw.dll kbdarmw.spec)

add_library(kbdarmw SHARED kbdarmw.c kbdarmw.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdarmw.def)

set_entrypoint(kbdarmw 0)

add_dependencies(kbdarmw psdk)

add_cd_file(TARGET kbdarmw DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
