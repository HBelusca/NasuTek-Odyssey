
spec2def(msvcrt.dll msvcrt.spec)

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

set_rc_compiler()

list(APPEND SOURCE
    dllmain.c
    stubs.c
    msvcrt.rc
    ${CMAKE_CURRENT_BINARY_DIR}/msvcrt.def)

add_library(msvcrt SHARED ${SOURCE})

set_module_type(msvcrt win32dll ENTRYPOINT DllMain 12)

add_dependencies(msvcrt psdk)

target_link_libraries(msvcrt crt wine ${PSEH_LIB})

if(MSVC)
    # export of deleting destructor "name"
    add_target_link_flags(msvcrt "/ignore:4102")
endif()

add_importlibs(msvcrt kernel32 ntdll)
add_pch(msvcrt precomp.h)
add_cd_file(TARGET msvcrt DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
add_importlib_target(msvcrt.spec)
