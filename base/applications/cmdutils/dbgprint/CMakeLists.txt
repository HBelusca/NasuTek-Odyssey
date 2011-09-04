
add_executable(dbgprint dbgprint.c)
set_module_type(dbgprint win32cui)
add_importlibs(dbgprint msvcrt kernel32)
add_cd_file(TARGET dbgprint DESTINATION odyssey/system32 FOR all)
