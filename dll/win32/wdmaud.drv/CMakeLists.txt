

add_definitions(-DNDEBUG=1)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/sound
    ${ODYSSEY_SOURCE_DIR}/drivers/wdm/audio/legacy/wdmaud
    ${ODYSSEY_SOURCE_DIR}/lib/drivers/sound/mmixer
    ${ODYSSEY_SOURCE_DIR}/lib/3rdparty/libsamplerate)

spec2def(wdmaud.drv wdmaud.spec)

list(APPEND SOURCE
    wdmaud.c
    mixer.c
    mmixer.c
    legacy.c
    wdmaud.rc
    ${CMAKE_CURRENT_BINARY_DIR}/wdmaud.def)

add_library(wdmaud.drv SHARED ${SOURCE})

set_module_type(wdmaud.drv win32dll UNICODE)

set_target_properties(wdmaud.drv PROPERTIES SUFFIX "")

target_link_libraries(wdmaud.drv
    mmebuddy
    libsamplerate
    mmixer)

add_importlibs(wdmaud.drv user32 winmm advapi32 msvcrt setupapi ksuser kernel32 ntdll)
add_pch(wdmaud.drv wdmaud.h)
add_cd_file(TARGET wdmaud.drv DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
