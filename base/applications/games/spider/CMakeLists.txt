
set_cpp()

include_directories(${ODYSSEY_SOURCE_DIR}/lib/3rdparty/cardlib ${CMAKE_CURRENT_SOURCE_DIR})

set_rc_compiler()

add_executable(spider
    spider.cpp
    spigame.cpp
    rsrc.rc)

target_link_libraries(spider cardlib)
add_pch(spider spider.h)
set_module_type(spider win32gui UNICODE)
add_importlibs(spider advapi32 comctl32 user32 gdi32 msvcrt kernel32)
add_cd_file(TARGET spider DESTINATION odyssey/system32 FOR all)
