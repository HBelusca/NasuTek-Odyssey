
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdinmal.dll kbdinmal.spec)

add_library(kbdinmal SHARED kbdinmal.c kbdinmal.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdinmal.def)

set_entrypoint(kbdinmal 0)

add_dependencies(kbdinmal psdk)

add_cd_file(TARGET kbdinmal DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
