
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdaze.dll kbdaze.spec)

add_library(kbdaze SHARED kbdaze.c kbdaze.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdaze.def)

set_entrypoint(kbdaze 0)

add_dependencies(kbdaze psdk)

add_cd_file(TARGET kbdaze DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
