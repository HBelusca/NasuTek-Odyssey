

spec2def(odbccp32i.cpl odbccp32.spec)

add_library(odbccp32i SHARED odbccp32.c odbccp32.rc ${CMAKE_CURRENT_BINARY_DIR}/odbccp32.def)

set_module_type(odbccp32i cpl UNICODE)

add_importlibs(odbccp32i user32 comctl32 msvcrt kernel32)
add_cd_file(TARGET odbccp32i DESTINATION odyssey/system32 FOR all)
