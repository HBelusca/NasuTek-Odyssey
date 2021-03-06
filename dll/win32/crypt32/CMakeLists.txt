
add_definitions(
    -D__WINESRC__
    -D_WINE)

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/wine)

set_rc_compiler()

spec2def(crypt32.dll crypt32.spec)

list(APPEND SOURCE
    base64.c
    cert.c
    chain.c
    collectionstore.c
    context.c
    crl.c
    ctl.c
    decode.c
    encode.c
    filestore.c
    main.c
    message.c
    msg.c
    object.c
    oid.c
    proplist.c
    protectdata.c
    provstore.c
    regstore.c
    rootstore.c
    serialize.c
    sip.c
    store.c
    str.c
    crypt32.rc
    ${CMAKE_CURRENT_BINARY_DIR}/crypt32_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/crypt32.def)

add_library(crypt32 SHARED ${SOURCE})

set_module_type(crypt32 win32dll)

target_link_libraries(crypt32
    wine
    ${PSEH_LIB}
    oldnames)

add_importlibs(crypt32 user32 advapi32 msvcrt kernel32 ntdll)
add_cd_file(TARGET crypt32 DESTINATION odyssey/system32 ODYSSEY_PE FOR all)
add_importlib_target(crypt32.spec)
