
add_definitions(-D__WINESRC__)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

add_typelib(std_ole_v1 std_ole_v1.idl)

spec2def(stdole32.tlb stdole32.tlb.spec)

list(APPEND SOURCE
    rsrc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/stdole32.def)

add_library(stdole32.tlb SHARED ${SOURCE})

set_entrypoint(stdole32.tlb 0)
set_target_properties(stdole32.tlb PROPERTIES SUFFIX "")

add_dependencies(stdole32.tlb std_ole_v1)
add_cd_file(TARGET stdole32.tlb DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
