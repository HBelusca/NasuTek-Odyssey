
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbduzb.dll kbduzb.spec)

add_library(kbduzb SHARED kbduzb.c kbduzb.rc ${CMAKE_CURRENT_BINARY_DIR}/kbduzb.def)

set_entrypoint(kbduzb 0)

add_dependencies(kbduzb psdk)

add_cd_file(TARGET kbduzb DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
