
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdsg.dll kbdsg.spec)

add_library(kbdsg SHARED kbdsg.c kbdsg.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdsg.def)

set_entrypoint(kbdsg 0)

add_dependencies(kbdsg psdk)

add_cd_file(TARGET kbdsg DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
