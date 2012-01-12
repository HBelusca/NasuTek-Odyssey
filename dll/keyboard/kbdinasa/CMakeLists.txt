
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdinasa.dll kbdinasa.spec)

add_library(kbdinasa SHARED kbdinasa.c kbdinasa.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdinasa.def)

set_entrypoint(kbdinasa 0)

add_dependencies(kbdinasa psdk)

add_cd_file(TARGET kbdinasa DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
