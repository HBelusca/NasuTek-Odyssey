
spec2def(msvcrt20.dll msvcrt20.spec)

add_definitions(
    -DUSE_MSVCRT_PREFIX
    -D_MSVCRT_
    -D_MSVCRT_LIB_
    -D_MT
    -D_CTYPE_DISABLE_MACROS
    -D_NO_INLINING
    -DCRTDLL
    -D__MINGW_IMPORT="")

include_directories(${ODYSSEY_SOURCE_DIR}/lib/sdk/crt/include)

list(APPEND SOURCE
    msvcrt20.c
    stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/msvcrt20_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/msvcrt20.def)

add_library(msvcrt20 SHARED ${SOURCE})
set_module_type(msvcrt20 win32dll ENTRYPOINT DllMain 12)

add_dependencies(msvcrt20 psdk)

target_link_libraries(msvcrt20 crt wine ${PSEH_LIB})

if(MSVC)
    # export of deleting destructor "name"
    add_target_link_flags(msvcrt20 "/ignore:4102")
endif()

add_importlibs(msvcrt20 kernel32 ntdll)

add_cd_file(TARGET msvcrt20 DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
