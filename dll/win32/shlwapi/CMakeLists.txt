
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)
add_definitions(-D__WINESRC__)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

spec2def(shlwapi.dll shlwapi.spec)

list(APPEND SOURCE
    assoc.c
    clist.c
    istream.c
    msgbox.c
    ordinal.c
    path.c
    reg.c
    regstream.c
    shlwapi_main.c
    stopwatch.c
    string.c
    thread.c
    url.c
    wsprintf.c
    shlwapi.rc
    ${CMAKE_CURRENT_BINARY_DIR}/shlwapi_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/shlwapi.def)

add_library(shlwapi SHARED ${SOURCE})
set_module_type(shlwapi win32dll)
target_link_libraries(shlwapi wine uuid)
add_delay_importlibs(shlwapi userenv oleaut32 ole32 comctl32 comdlg32 mpr mlang urlmon shell32 winmm version)
add_importlibs(shlwapi user32 gdi32 advapi32 msvcrt wininet kernel32 ntdll)
add_cd_file(TARGET shlwapi DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(shlwapi.spec)
