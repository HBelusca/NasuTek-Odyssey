
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdinguj.dll kbdinguj.spec)

add_library(kbdinguj SHARED kbdinguj.c kbdinguj.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdinguj.def)

set_entrypoint(kbdinguj 0)

add_dependencies(kbdinguj psdk)

add_cd_file(TARGET kbdinguj DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
