
spec2def(user32.dll user32.spec)

include_directories(
    BEFORE ${ODYSSEY_SOURCE_DIR}/include/odyssey/wine
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys
    include)

set_rc_compiler()

list(APPEND SOURCE
    controls/appswitch.c
    controls/button.c
    controls/combo.c
    controls/edit.c
    controls/icontitle.c
    controls/listbox.c
    controls/regcontrol.c
    controls/scrollbar.c
    controls/static.c
    misc/dde.c
    misc/ddeclient.c
    misc/ddeserver.c
    misc/desktop.c
    misc/display.c
    misc/dllmain.c
    misc/exit.c
    misc/exticon.c
    misc/imm.c
    misc/misc.c
    misc/object.c
    misc/resources.c
    misc/rtlstr.c
    misc/stubs.c
    misc/timer.c
    misc/usrapihk.c
    misc/winhelp.c
    misc/winsta.c
    windows/accel.c
    windows/caret.c
    windows/class.c
    windows/clipboard.c
    windows/cursoricon.c
    windows/dc.c
    windows/defwnd.c
    windows/dialog.c
    windows/draw.c
    windows/font.c
    windows/hook.c
    windows/input.c
    windows/mdi.c
    windows/menu.c
    windows/messagebox.c
    windows/message.c
    windows/nonclient.c
    windows/paint.c
    windows/prop.c
    windows/rect.c
    windows/spy.c
    windows/text.c
    windows/window.c
    windows/winpos.c
    user32.rc
    ${CMAKE_CURRENT_BINARY_DIR}/user32.def)

add_library(user32 SHARED ${SOURCE})
set_module_type(user32 win32dll UNICODE)

target_link_libraries(user32
    user32_wsprintf
    wine
    win32ksys
    ${PSEH_LIB})

add_delay_importlibs(user32 imm32)
add_importlibs(user32 gdi32 advapi32 msvcrt kernel32 ntdll)
add_pch(user32 include/user32.h)
add_cd_file(TARGET user32 DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
add_importlib_target(user32.spec)
