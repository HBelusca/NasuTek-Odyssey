
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdgneo.dll kbdgneo.spec)

add_library(kbdgneo SHARED kbdgneo.c kbdgneo.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdgneo.def)

set_entrypoint(kbdgneo 0)

add_dependencies(kbdgneo psdk)

add_cd_file(TARGET kbdgneo DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
