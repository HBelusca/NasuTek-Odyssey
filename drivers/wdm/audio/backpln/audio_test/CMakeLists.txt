
add_definitions(-DPC_NO_IMPORTS)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/sound
    ${ODYSSEY_SOURCE_DIR}/drivers/wdm/audio/legacy/wdmaud
    ${ODYSSEY_SOURCE_DIR}/lib/3rdparty/libsamplerate)

add_executable(audio_test audio_test.c)

set_module_type(audio_test win32cui)

target_link_libraries(audio_test libsamplerate)

add_importlibs(audio_test setupapi ksuser msvcrt kernel32)
add_dependencies(audio_test bugcodes)
