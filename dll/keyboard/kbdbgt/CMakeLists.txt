
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdbgt.dll kbdbgt.spec)

add_library(kbdbgt SHARED kbdbgt.c kbdbgt.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdbgt.def)

set_entrypoint(kbdbgt 0)

add_dependencies(kbdbgt psdk)

add_cd_file(TARGET kbdbgt DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
