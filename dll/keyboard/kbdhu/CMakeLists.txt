
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdhu.dll kbdhu.spec)

add_library(kbdhu SHARED kbdhu.c kbdhu.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdhu.def)

set_entrypoint(kbdhu 0)

add_dependencies(kbdhu psdk)

add_cd_file(TARGET kbdhu DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
