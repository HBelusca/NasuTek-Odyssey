
include_directories(${ODYSSEY_SOURCE_DIR}/ntoskrnl/include)

spec2def(kbdusr.dll kbdusr.spec)

add_library(kbdusr SHARED kbdusr.c kbdusr.rc ${CMAKE_CURRENT_BINARY_DIR}/kbdusr.def)

set_entrypoint(kbdusr 0)

add_dependencies(kbdusr psdk)

add_cd_file(TARGET kbdusr DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
