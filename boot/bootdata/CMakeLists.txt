
add_subdirectory(packages)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/instcd)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bootcd)

add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/txtsetup.sif DESTINATION odyssey NO_CAB FOR bootcd instcd regtest)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hivecls_${ARCH}.inf DESTINATION odyssey NO_CAB NAME_ON_CD hivecls.inf FOR bootcd instcd regtest)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hivedef_${ARCH}.inf DESTINATION odyssey NO_CAB NAME_ON_CD hivedef.inf FOR bootcd instcd regtest)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hivesft_${ARCH}.inf DESTINATION odyssey NO_CAB NAME_ON_CD hivesft.inf FOR bootcd instcd regtest)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/hivesys_${ARCH}.inf DESTINATION odyssey NO_CAB NAME_ON_CD hivesys.inf FOR bootcd instcd regtest)

# ocd hives
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/instcd/ocd
    COMMAND native-ocdgen ${CMAKE_CURRENT_SOURCE_DIR}/instcd_ocd.inf ${CMAKE_CURRENT_BINARY_DIR}/instcd/ocd
    DEPENDS native-ocdgen ${CMAKE_CURRENT_SOURCE_DIR}/instcd_ocd.inf)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ocd
    COMMAND native-ocdgen ${CMAKE_CURRENT_SOURCE_DIR}/livecd_ocd.inf ${CMAKE_CURRENT_BINARY_DIR}/ocd
    DEPENDS native-ocdgen ${CMAKE_CURRENT_SOURCE_DIR}/livecd_ocd.inf)

# livecd hives
list(APPEND CD_HIVES
    ${CMAKE_CURRENT_SOURCE_DIR}/livecd.inf
    ${CMAKE_CURRENT_SOURCE_DIR}/hiveinst_${ARCH}.inf)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sam
        ${CMAKE_CURRENT_BINARY_DIR}/default
        ${CMAKE_CURRENT_BINARY_DIR}/security
        ${CMAKE_CURRENT_BINARY_DIR}/software
        ${CMAKE_CURRENT_BINARY_DIR}/system
    COMMAND native-mkhive ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${ARCH} ${CD_HIVES}
    DEPENDS native-mkhive ${CD_HIVES})

add_custom_target(livecd_hives
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sam
        ${CMAKE_CURRENT_BINARY_DIR}/default
        ${CMAKE_CURRENT_BINARY_DIR}/security
        ${CMAKE_CURRENT_BINARY_DIR}/software
        ${CMAKE_CURRENT_BINARY_DIR}/system
        ${CMAKE_CURRENT_BINARY_DIR}/ocd)

add_cd_file(
    FILE ${CMAKE_CURRENT_BINARY_DIR}/sam
        ${CMAKE_CURRENT_BINARY_DIR}/default
        ${CMAKE_CURRENT_BINARY_DIR}/security
        ${CMAKE_CURRENT_BINARY_DIR}/software
        ${CMAKE_CURRENT_BINARY_DIR}/system
    TARGET livecd_hives
    DESTINATION odyssey/system32/config
    FOR livecd)

add_cd_file(
    FILE ${CMAKE_CURRENT_BINARY_DIR}/ocd
    TARGET livecd_hives
    DESTINATION root
    FOR livecd)

# instcd hives
list(APPEND INSTCD_HIVES
    ${CMAKE_CURRENT_SOURCE_DIR}/instcd.inf
    ${CMAKE_CURRENT_SOURCE_DIR}/hiveinst_${ARCH}.inf)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/instcd/sam
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/default
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/security
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/software
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/system
    COMMAND native-mkhive ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/instcd ${ARCH} ${INSTCD_HIVES}
    DEPENDS native-mkhive ${INSTCD_HIVES})

add_custom_target(instcd_hives
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/instcd/sam
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/default
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/security
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/software
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/system
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/ocd)

add_cd_file(
    FILE ${CMAKE_CURRENT_BINARY_DIR}/instcd/sam
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/default
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/security
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/software
        ${CMAKE_CURRENT_BINARY_DIR}/instcd/system
    TARGET instcd_hives
    DESTINATION odyssey/system32/config
    NO_CAB
    FOR instcd)

add_cd_file(
    FILE ${CMAKE_CURRENT_BINARY_DIR}/instcd/ocd
    TARGET instcd_hives
    DESTINATION root
    NO_CAB
    FOR instcd)    

# bootcd ocd
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bootcd/ocd
    COMMAND native-ocdgen ${CMAKE_CURRENT_SOURCE_DIR}/bootcd_ocd.inf ${CMAKE_CURRENT_BINARY_DIR}/bootcd/ocd
    DEPENDS native-ocdgen ${CMAKE_CURRENT_SOURCE_DIR}/bootcd_ocd.inf)

add_custom_target(bootcd_ocd
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bootcd/ocd)

add_cd_file(
    FILE ${CMAKE_CURRENT_BINARY_DIR}/bootcd/ocd
    TARGET bootcd_ocd
    DESTINATION root
    NO_CAB
    FOR bootcd)

#regtest
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bootcdregtest/regtest.cmd DESTINATION odyssey/bin FOR all)

#freeldr.ini
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/instcd.ini DESTINATION root NAME_ON_CD freeldr.ini NO_CAB FOR instcd)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bootcd.ini DESTINATION root NAME_ON_CD freeldr.ini NO_CAB FOR bootcd regtest)
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/livecd.ini DESTINATION root NAME_ON_CD freeldr.ini FOR livecd)

#autorun.inf
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/autorun.inf DESTINATION root NO_CAB FOR bootcd instcd regtest)

#unattend
add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bootcdregtest/unattend.inf DESTINATION odyssey NO_CAB FOR regtest)
#uncomment this for unattend bootcd
#add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/bootcd/unattend.inf DESTINATION odyssey NO_CAB FOR bootcd)