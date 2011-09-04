
include_directories(
    lib
    ${ODYSSEY_SOURCE_DIR}/include/odyssey/libs/expat)

add_definitions(-DHAVE_EXPAT_CONFIG_H)

list(APPEND SOURCE
    lib/xmlparse.c
    lib/xmlrole.c
    lib/xmltok.c)

add_library(expat ${SOURCE})
