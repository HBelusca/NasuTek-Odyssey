
add_definitions(-Dlint)

add_executable(ftp
    cmds.c
    cmdtab.c
    domacro.c
    fake.c
    ftp.c
    main.c
    ruserpass.c
    ftp.rc)

set_module_type(ftp win32cui)
add_importlibs(ftp ws2_32 iphlpapi msvcrt kernel32)
add_pch(ftp precomp.h)

if(MSVC)
    target_link_libraries(ftp oldnames)
    add_importlibs(ftp ntdll)
endif()

add_cd_file(TARGET ftp DESTINATION odyssey/system32 FOR all)
