
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdko.dll kbdko.spec)

add_library(kbdko SHARED kbdko.c kbdko.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdko.def)

set_entrypoint(kbdko 0)

add_dependencies(kbdko psdk)

add_cd_file(TARGET kbdko DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
