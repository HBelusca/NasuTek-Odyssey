
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdcz.dll kbdcz.spec)

add_library(kbdcz SHARED kbdcz.c kbdcz.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdcz.def)

set_entrypoint(kbdcz 0)

add_dependencies(kbdcz psdk)

add_cd_file(TARGET kbdcz DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
