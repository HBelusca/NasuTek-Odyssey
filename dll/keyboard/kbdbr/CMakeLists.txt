
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdbr.dll kbdbr.spec)

add_library(kbdbr SHARED kbdbr.c kbdbr.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdbr.def)

set_entrypoint(kbdbr 0)

add_dependencies(kbdbr psdk)

add_cd_file(TARGET kbdbr DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
