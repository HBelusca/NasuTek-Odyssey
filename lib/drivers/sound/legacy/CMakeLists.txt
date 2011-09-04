
add_definitions(
    -D__NTDRIVER__
    -DKERNEL)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/lib/drivers/sound/soundblaster
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/sound)

add_library(audioleg devname.c hardware.c)
add_dependencies(audioleg bugcodes)
