
add_typelib(mshtml__tlb mshtml_tlb.idl)
add_dependencies(mshtml__tlb stdole2)

add_definitions(-D__WINESRC__)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

add_library(mshtml.tlb SHARED rsrc.rc)

set_entrypoint(mshtml.tlb 0)
set_target_properties(mshtml.tlb PROPERTIES SUFFIX "")
add_dependencies(mshtml.tlb mshtml__tlb)
add_cd_file(TARGET mshtml.tlb DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
