
set_cpp()
set_rc_compiler()

include_directories(${ODYSSEY_SOURCE_DIR}/lib/3rdparty/cardlib)

add_executable(sol
    solcreate.cpp
    solgame.cpp
    solitaire.cpp
    rsrc.rc)

target_link_libraries(sol cardlib)
add_pch(sol solitaire.h)
set_module_type(sol win32gui UNICODE)
add_importlibs(sol advapi32 comctl32 user32 gdi32 msvcrt kernel32)
add_cd_file(TARGET sol DESTINATION odyssey/system32 FOR all)
