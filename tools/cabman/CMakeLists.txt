list(APPEND SOURCE
    cabinet.cxx
    dfp.cxx
    main.cxx
    mszip.cxx
    raw.cxx)

include_directories(${ODYSSEY_SOURCE_DIR}/lib/3rdparty/zlib)

add_executable(cabman ${SOURCE})

target_link_libraries(cabman zlibhost)