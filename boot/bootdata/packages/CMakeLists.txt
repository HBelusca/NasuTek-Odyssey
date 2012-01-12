#odyssey.dff

# odyssey.dff is the concatenation of two files :
#   - odyssey.dff.in, which is a static one and can be altered to
# add custom modules/files to odyssey.cab
#   - odyssey.dff.dyn (dyn as dynamic) which is generated at configure time by our cmake scripts
# If you want to slip-stream anything into the bootcd, then you want to alter odyssey.dff.in

# Idea taken from there : http://www.cmake.org/pipermail/cmake/2010-July/038028.html
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/concat.cmake "
    file(READ \${SRC1} S1)
    file(READ \${SRC2} S2)
    file(WRITE \${DST} \"\${S1}\${S2}\")
")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff.dyn "")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff
    COMMAND ${CMAKE_COMMAND} -D SRC1=${CMAKE_CURRENT_SOURCE_DIR}/odyssey.dff.in
                             -D SRC2=${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff.dyn
                             -D DST=${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff
                             -P ${CMAKE_CURRENT_BINARY_DIR}/concat.cmake
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/odyssey.dff.in
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff.dyn
)

# And now we build odyssey.cab

# First we create odyssey.inf
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/odyssey.inf
    COMMAND native-cabman -C ${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff -L ${CMAKE_CURRENT_BINARY_DIR} -I -P ${ODYSSEY_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff native-cabman)

# Then we create the actual cab file using a custom target
add_custom_target(odyssey_cab
    COMMAND native-cabman -C ${CMAKE_CURRENT_BINARY_DIR}/odyssey.dff -RC ${CMAKE_CURRENT_BINARY_DIR}/odyssey.inf -L ${CMAKE_CURRENT_BINARY_DIR} -N -P ${ODYSSEY_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/odyssey.inf native-cabman)

add_cd_file(TARGET odyssey_cab FILE ${CMAKE_CURRENT_BINARY_DIR}/odyssey.cab ${CMAKE_CURRENT_BINARY_DIR}/odyssey.inf DESTINATION odyssey NO_CAB FOR bootcd instcd regtest)
