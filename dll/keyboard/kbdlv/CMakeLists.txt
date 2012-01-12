
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdlv.dll kbdlv.spec)

add_library(kbdlv SHARED kbdlv.c kbdlv.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdlv.def)

set_entrypoint(kbdlv 0)

add_dependencies(kbdlv psdk)

add_cd_file(TARGET kbdlv DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
