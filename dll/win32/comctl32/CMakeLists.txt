
set_rc_compiler()

add_definitions(
    -D__WINESRC__
    -D_WINE
    -D_COMCTL32_)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

include_directories(BEFORE ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(comctl32.dll comctl32.spec)

list(APPEND SOURCE
    animate.c
    comboex.c
    comctl32undoc.c
    commctrl.c
    datetime.c
    dpa.c
    draglist.c
    dsa.c
    flatsb.c
    header.c
    hotkey.c
    imagelist.c
    ipaddress.c
    listview.c
    monthcal.c
    nativefont.c
    pager.c
    progress.c
    propsheet.c
    rebar.c
    smoothscroll.c
    status.c
    string.c
    syslink.c
    tab.c
    theme_button.c
    theme_combo.c
    theme_dialog.c
    theme_edit.c
    theme_listbox.c
    theming.c
    toolbar.c
    tooltips.c
    trackbar.c
    treeview.c
    updown.c
    stubs.c
    rsrc.rc
    ${CMAKE_CURRENT_BINARY_DIR}/comctl32.def)

add_library(comctl32 SHARED ${SOURCE})
set_module_type(comctl32 win32dll UNICODE)
target_link_libraries(comctl32 uuid wine)
add_delay_importlibs(comctl32 winmm uxtheme)

add_importlibs(comctl32
    user32
    gdi32
    advapi32
    msvcrt
    kernel32
    ntdll)

add_pch(comctl32 comctl32.h)
add_cd_file(TARGET comctl32 DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
add_importlib_target(comctl32.spec)
