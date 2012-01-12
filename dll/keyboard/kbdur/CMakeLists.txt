
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdur.dll kbdur.spec)

add_library(kbdur SHARED kbdur.c kbdur.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdur.def)

set_entrypoint(kbdur 0)

add_dependencies(kbdur psdk)

add_cd_file(TARGET kbdur DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
