
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdru.dll kbdru.spec)

add_library(kbdru SHARED kbdru.c kbdru.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdru.def)

set_entrypoint(kbdru 0)

add_dependencies(kbdru psdk)

add_cd_file(TARGET kbdru DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
