
add_definitions(
    -D__NTDRIVER__
    -DKERNEL)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/lib/drivers/sound/soundblaster
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/sound)

add_library(uartmidi midiuart.c)
add_dependencies(uartmidi bugcodes)
