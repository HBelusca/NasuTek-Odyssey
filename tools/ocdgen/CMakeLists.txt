
add_definitions(-DMKHIVE_HOST)

include_directories(
    ${ODYSSEY_SOURCE_DIR}/lib/newinflib
    ${ODYSSEY_SOURCE_DIR}/lib/cmlib
    ${ODYSSEY_SOURCE_DIR}/lib/rtl)

list(APPEND SOURCE
    binhive.c
    cmi.c
    mkhive.c
    reginf.c
    registry.c
    rtl.c)

add_executable(ocdgen ${SOURCE})
target_link_libraries(ocdgen unicode cmlibhost inflibhost)
