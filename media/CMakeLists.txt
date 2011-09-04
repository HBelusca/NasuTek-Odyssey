add_subdirectory(fonts)
add_subdirectory(inf)
add_subdirectory(nls)
add_subdirectory(vgafonts)

add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/drivers/etc/hosts DESTINATION odyssey/system32/drivers/etc FOR all)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/drivers/etc/KDBinit DESTINATION odyssey/system32/drivers/etc FOR all)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/drivers/etc/services DESTINATION odyssey/system32/drivers/etc FOR all)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/sounds/Odyssey_LogOn.wav DESTINATION odyssey/media FOR all)