
set_rc_compiler()

list(APPEND SOURCE
    about.c
    control.c
    create.c
    delete.c
    dependencies_tv1.c
    dependencies_tv2.c
    export.c
    listview.c
    mainwnd.c
    misc.c
    progress.c
    propsheet.c
    propsheet_depends.c
    propsheet_general.c
    query.c
    servman.c
    start.c
    stop.c
    stop_dependencies.c
    servman.rc)

add_executable(servman ${SOURCE})

set_module_type(servman win32gui UNICODE)

add_importlibs(servman user32 gdi32 advapi32 version comctl32 shell32 comdlg32 msvcrt kernel32)

add_pch(servman precomp.h)

add_cd_file(TARGET servman DESTINATION odyssey/system32 FOR all)
