
set_rc_compiler()

list(APPEND SOURCE
    about.c
    affinity.c
    applpage.c
    column.c
    dbgchnl.c
    debug.c
    endproc.c
    graph.c
    optnmenu.c
    perfdata.c
    perfpage.c
    priority.c
    procpage.c
    proclist.c
    run.c
    trayicon.c
    taskmgr.c
    graphctl.c
    taskmgr.rc)

add_executable(taskmgr ${SOURCE})

set_module_type(taskmgr win32gui UNICODE)
add_importlibs(taskmgr advapi32 user32 gdi32 shell32 comctl32 msvcrt kernel32 ntdll)
add_pch(taskmgr precomp.h)
add_cd_file(TARGET taskmgr DESTINATION odyssey/system32 FOR all)
