
add_subdirectory(access)
add_subdirectory(appwiz)
add_subdirectory(console)#Warning: console\\/lang/cs-CZ.rc:143: unrecognized escape sequence
add_subdirectory(desk)
add_subdirectory(hdwwiz)
if(NOT MSVC)
    add_subdirectory(inetcpl) # FIXME: msvc build.
endif()
add_subdirectory(input)
add_subdirectory(intl)
add_subdirectory(joy)
#add_subdirectory(liccpa)#disabled in trunk
add_subdirectory(main)
add_subdirectory(mmsys)
add_subdirectory(ncpa)
add_subdirectory(odbccp32)
add_subdirectory(powercfg)
add_subdirectory(sysdm)
add_subdirectory(telephon)
add_subdirectory(timedate)
add_subdirectory(usrmgr)
