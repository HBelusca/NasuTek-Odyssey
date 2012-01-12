
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdheb.dll kbdheb.spec)

add_library(kbdheb SHARED kbdheb.c kbdheb.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdheb.def)

set_entrypoint(kbdheb 0)

add_dependencies(kbdheb psdk)

add_cd_file(TARGET kbdheb DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
