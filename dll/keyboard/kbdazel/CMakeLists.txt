
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdazel.dll kbdazel.spec)

add_library(kbdazel SHARED kbdazel.c kbdazel.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdazel.def)

set_entrypoint(kbdazel 0)

add_dependencies(kbdazel psdk)

add_cd_file(TARGET kbdazel DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
