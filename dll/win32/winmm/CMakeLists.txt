
add_definitions(-D_WINE)
include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

spec2def(winmm.dll winmm.spec)

set_rc_compiler()

list(APPEND SOURCE
    driver.c
    joystick.c
    lolvldrv.c
    mci.c
    mmio.c
    playsound.c
    registry.c
    time.c
    winmm.c
    winmm_res.rc
    ${CMAKE_CURRENT_BINARY_DIR}/winmm_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/winmm.def)

add_library(winmm SHARED ${SOURCE})

set_module_type(winmm win32dll)

target_link_libraries(winmm
    wine
    ${PSEH_LIB})

add_importlibs(winmm advapi32 user32 msvcrt kernel32 ntdll)


add_subdirectory(midimap)
add_cd_file(TARGET winmm DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(winmm.spec)
