
add_definitions(-D__WINESRC__)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(hnetcfg.dll hnetcfg.spec)

list(APPEND SOURCE
    apps.c
    hnetcfg.c
    manager.c
    policy.c
    port.c
    profile.c
    regsvr.c
    service.c
    ${CMAKE_CURRENT_BINARY_DIR}/hnetcfg.def)

add_library(hnetcfg SHARED ${SOURCE})

set_module_type(hnetcfg win32dll)

target_link_libraries(hnetcfg wine uuid)

add_importlibs(hnetcfg
    ole32
    advapi32
    msvcrt
    kernel32
    ntdll)

add_cd_file(TARGET hnetcfg DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
