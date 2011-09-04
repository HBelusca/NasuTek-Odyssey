
include_directories(${ODYSSEY_BINARY_DIR}/include/odyssey)
add_library(netevent SHARED netevt.rc)
set_module_type(netevent win32dll ENTRYPOINT 0 UNICODE)
add_dependencies(netevent neteventmsg)
add_cd_file(TARGET netevent DESTINATION odyssey/system32 FOR all)
