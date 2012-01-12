
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdes.dll kbdes.spec)

add_library(kbdes SHARED kbdes.c kbdes.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdes.def)

set_entrypoint(kbdes 0)

add_dependencies(kbdes psdk)

add_cd_file(TARGET kbdes DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
