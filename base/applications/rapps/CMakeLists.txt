

list(APPEND SOURCE
    aboutdlg.c
    available.c
    installdlg.c
    installed.c
    listview.c
    loaddlg.c
    misc.c
    parser.c
    richedit.c
    settingsdlg.c
    splitter.c
    statusbar.c
    toolbar.c
    treeview.c
    winmain.c
    rapps.rc)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${ODYSSEY_BINARY_DIR}/include/odyssey)

set_rc_compiler()

add_executable(rapps ${SOURCE})
add_pch(rapps rapps.h)
set_module_type(rapps win32gui UNICODE)
target_link_libraries(rapps uuid)

add_importlibs(rapps advapi32 comctl32 gdi32 urlmon user32 shell32 shlwapi msvcrt kernel32 ntdll)
add_dependencies(rapps rappsmsg)
add_message_headers(rappsmsg.mc)
add_cd_file(TARGET rapps DESTINATION odyssey/system32 FOR all)
