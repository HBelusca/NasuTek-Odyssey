
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdinben.dll kbdinben.spec)

add_library(kbdinben SHARED kbdinben.c kbdinben.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdinben.def)

set_entrypoint(kbdinben 0)

add_dependencies(kbdinben psdk)

add_cd_file(TARGET kbdinben DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
