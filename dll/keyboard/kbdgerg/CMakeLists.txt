
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdgerg.dll kbdgerg.spec)

add_library(kbdgerg SHARED kbdgerg.c kbdgerg.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdgerg.def)

set_entrypoint(kbdgerg 0)

add_dependencies(kbdgerg psdk)

add_cd_file(TARGET kbdgerg DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
