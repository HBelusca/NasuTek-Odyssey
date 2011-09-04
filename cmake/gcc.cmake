
# Compiler Core
add_compile_flags("-pipe -fms-extensions")

# Debugging (Note: DWARF-4 on 4.5.1 when we ship)
add_compile_flags("-gdwarf-2 -g2 -femit-struct-debug-detailed=none -feliminate-unused-debug-types")

# For some reason, cmake sets -fPIC, and we don't want it
string(REPLACE "-fPIC" "" CMAKE_SHARED_LIBRARY_ASM_FLAGS ${CMAKE_SHARED_LIBRARY_ASM_FLAGS})

# Tuning
if(ARCH MATCHES i386)
    add_compile_flags("-march=${OARCH} -mtune=${TUNE}")
else()
    add_compile_flags("-march=${OARCH}")
endif()

# Warnings
add_compile_flags("-Wall -Wno-char-subscripts -Wpointer-arith -Wno-multichar -Wno-error=uninitialized -Wno-unused-value")

if(ARCH MATCHES amd64)
    add_compile_flags("-Wno-format")
elseif(ARCH MATCHES arm)
    add_compile_flags("-Wno-attributes")
endif()

# Optimizations
if(OPTIMIZE STREQUAL "1")
    add_compile_flags("-Os")
elseif(OPTIMIZE STREQUAL "2")
    add_compile_flags("-Os")
elseif(OPTIMIZE STREQUAL "3")
    add_compile_flags("-O1")
elseif(OPTIMIZE STREQUAL "4")
    add_compile_flags("-O2")
elseif(OPTIMIZE STREQUAL "5")
    add_compile_flags("-O3")
endif()

add_compile_flags("-fno-strict-aliasing")

if(ARCH MATCHES i386)
    add_compile_flags("-mpreferred-stack-boundary=2 -fno-set-stack-executable -fno-optimize-sibling-calls -fno-omit-frame-pointer")
    if(OPTIMIZE STREQUAL "1")
        add_compile_flags("-ftracer -momit-leaf-frame-pointer")
    endif()
elseif(ARCH MATCHES amd64)
    add_compile_flags("-mpreferred-stack-boundary=4")
    if(OPTIMIZE STREQUAL "1")
        add_compile_flags("-ftracer -momit-leaf-frame-pointer")
    endif()
elseif(ARCH MATCHES arm)
    if(OPTIMIZE STREQUAL "1")
        add_compile_flags("-ftracer")
    endif()
endif()

# Other
if(ARCH MATCHES amd64)
    add_definitions(-U_X86_ -UWIN32)
elseif(ARCH MATCHES arm)
    add_definitions(-U_UNICODE -UUNICODE)
    add_definitions(-D__MSVCRT__) # DUBIOUS
endif()

# alternative arch name
if(ARCH MATCHES amd64)
    set(ARCH2 x86_64)
else()
    set(ARCH2 ${ARCH})
endif()

# Linking
if(ARCH MATCHES i386)
    link_directories(${ODYSSEY_SOURCE_DIR}/importlibs)
endif()

link_directories(${ODYSSEY_BINARY_DIR}/lib/sdk/crt)

set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_EXE_LINKER_FLAGS "-nodefaultlibs -nostdlib -Wl,--enable-auto-image-base -Wl,--disable-auto-import -Wl,--disable-stdcall-fixup")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_INIT} -Wl,--disable-stdcall-fixup")

set(CMAKE_C_CREATE_SHARED_LIBRARY "<CMAKE_C_COMPILER> <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
set(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
set(CMAKE_RC_CREATE_SHARED_LIBRARY "<CMAKE_C_COMPILER> <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")

set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> -x assembler-with-cpp -o <OBJECT> -I${ODYSSEY_SOURCE_DIR}/include/asm -I${ODYSSEY_BINARY_DIR}/include/asm <FLAGS> <DEFINES> -D__ASM__ -c <SOURCE>")

#set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> <CMAKE_C_LINK_FLAGS> <DEFINES> -I${ODYSSEY_SOURCE_DIR}/include/psdk -I${ODYSSEY_BINARY_DIR}/include/psdk -I${ODYSSEY_SOURCE_DIR}/include/ -I${ODYSSEY_SOURCE_DIR}/include/odyssey -I${ODYSSEY_BINARY_DIR}/include/odyssey -I${ODYSSEY_SOURCE_DIR}/include/odyssey/wine -I${ODYSSEY_SOURCE_DIR}/include/crt -I${ODYSSEY_SOURCE_DIR}/include/crt/mingw32 -O coff -o <OBJECT>")

# Temporary, until windres issues are fixed
get_target_property(WRC native-wrc IMPORTED_LOCATION_NOCONFIG)
set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_C_COMPILER> -DRC_INVOKED -D__WIN32__=1 -D__FLAT__=1 <DEFINES> -I${ODYSSEY_SOURCE_DIR}/include/psdk -I${ODYSSEY_BINARY_DIR}/include/psdk -I${ODYSSEY_SOURCE_DIR}/include/ -I${ODYSSEY_SOURCE_DIR}/include/odyssey -I${ODYSSEY_BINARY_DIR}/include/odyssey -I${ODYSSEY_SOURCE_DIR}/include/odyssey/wine -I${ODYSSEY_SOURCE_DIR}/include/crt -I${ODYSSEY_SOURCE_DIR}/include/crt/mingw32 -xc -E <SOURCE> -o <OBJECT>"
    "${WRC} -i <OBJECT> -o <OBJECT>"
    "<CMAKE_RC_COMPILER> -i <OBJECT> -J res -O coff -o <OBJECT>")

# Optional 3rd parameter: stdcall stack bytes
function(set_entrypoint MODULE ENTRYPOINT)
    if(${ENTRYPOINT} STREQUAL "0")
        add_target_link_flags(${MODULE} "-Wl,-entry,0")
    elseif(ARCH MATCHES i386)
        set(_entrysymbol _${ENTRYPOINT})
        if (${ARGC} GREATER 2)
            set(_entrysymbol ${_entrysymbol}@${ARGV2})
        endif()
        add_target_link_flags(${MODULE} "-Wl,-entry,${_entrysymbol}")
    else()
        add_target_link_flags(${MODULE} "-Wl,-entry,${ENTRYPOINT}")
    endif()
endfunction()

function(set_subsystem MODULE SUBSYSTEM)
    add_target_link_flags(${MODULE} "-Wl,--subsystem,${SUBSYSTEM}")
endfunction()

function(set_image_base MODULE IMAGE_BASE)
    add_target_link_flags(${MODULE} "-Wl,--image-base,${IMAGE_BASE}")
endfunction()

function(set_module_type_toolchain MODULE TYPE)
    if(IS_CPP)
        target_link_libraries(${MODULE} stlport -lsupc++ -lgcc)
    endif()

    if(${TYPE} STREQUAL kernelmodedriver)
        add_target_link_flags(${MODULE} "-Wl,--exclude-all-symbols -Wl,-file-alignment=0x1000 -Wl,-section-alignment=0x1000")
    endif()
endfunction()

# Workaround lack of mingw RC support in cmake
function(set_rc_compiler)
    get_directory_property(defines COMPILE_DEFINITIONS)
    get_directory_property(includes INCLUDE_DIRECTORIES)

    foreach(arg ${defines})
        set(rc_result_defs "${rc_result_defs} -D${arg}")
    endforeach()

    foreach(arg ${includes})
        set(rc_result_incs "-I${arg} ${rc_result_incs}")
    endforeach()

    #set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> ${rc_result_defs} ${rc_result_incs} -i <SOURCE> -O coff -o <OBJECT>" PARENT_SCOPE)
    set(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_C_COMPILER> -DRC_INVOKED -D__WIN32__=1 -D__FLAT__=1 ${rc_result_defs} -I${CMAKE_CURRENT_SOURCE_DIR} ${rc_result_incs} -xc -E <SOURCE> -o <OBJECT>"
    "${WRC} -I${CMAKE_CURRENT_SOURCE_DIR} ${rc_result_incs} -i <OBJECT> -o <OBJECT>"
    "<CMAKE_RC_COMPILER> -i <OBJECT> -J res -O coff -o <OBJECT>" PARENT_SCOPE)
endfunction()

function(add_delay_importlibs MODULE)
    foreach(LIB ${ARGN})
        target_link_libraries(${MODULE} ${CMAKE_BINARY_DIR}/importlibs/lib${LIB}_delayed.a)
        add_dependencies(${MODULE} lib${LIB}_delayed)
    endforeach()
    target_link_libraries(${MODULE} delayimp)
endfunction()

if(NOT ARCH MATCHES i386)
    set(DECO_OPTION "-@")
endif()

function(add_importlib_target _exports_file)

    get_filename_component(_name ${_exports_file} NAME_WE)
    get_filename_component(_extension ${_exports_file} EXT)
    get_target_property(_suffix ${_name} SUFFIX)
    if(${_suffix} STREQUAL "_suffix-NOTFOUND")
        get_target_property(_type ${_name} TYPE)
        if(${_type} MATCHES EXECUTABLE)
            set(_suffix ".exe")
        else()
            set(_suffix ".dll")
        endif()
    endif()

    if (${_extension} STREQUAL ".spec")

        # Normal importlib creation
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/importlibs/lib${_name}.a
            COMMAND native-spec2def -n=${_name}${_suffix} -a=${ARCH2} -d=${CMAKE_CURRENT_BINARY_DIR}/${_name}_implib.def ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file}
            COMMAND ${MINGW_PREFIX}dlltool --def ${CMAKE_CURRENT_BINARY_DIR}/${_name}_implib.def --kill-at --output-lib=${CMAKE_BINARY_DIR}/importlibs/lib${_name}.a
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file} native-spec2def)

        # Delayed importlib creation
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/importlibs/lib${_name}_delayed.a
            COMMAND native-spec2def -n=${_name}${_suffix} -a=${ARCH2} -d=${CMAKE_CURRENT_BINARY_DIR}/${_name}_delayed_implib.def ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file}
            COMMAND ${MINGW_PREFIX}dlltool --def ${CMAKE_CURRENT_BINARY_DIR}/${_name}_delayed_implib.def --kill-at --output-delaylib ${CMAKE_BINARY_DIR}/importlibs/lib${_name}_delayed.a
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file} native-spec2def)

    elseif(${_extension} STREQUAL ".def")
        message("Use of def files for import libs is deprecated: ${_exports_file}")
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/importlibs/lib${_name}.a
            COMMAND ${MINGW_PREFIX}dlltool --def ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file} --kill-at --output-lib=${CMAKE_BINARY_DIR}/importlibs/lib${_name}.a
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file})
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/importlibs/lib${_name}_delayed.a
            COMMAND ${MINGW_PREFIX}dlltool --def ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file} --kill-at --output-delaylib ${CMAKE_BINARY_DIR}/importlibs/lib${_name}_delayed.a
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_exports_file})
    else()
        message(FATAL_ERROR "Unsupported exports file extension: ${_extension}")
    endif()

    # Normal importlib target
    add_custom_target(
        lib${_name}
        DEPENDS ${CMAKE_BINARY_DIR}/importlibs/lib${_name}.a)
    # Delayed importlib target
    add_custom_target(
        lib${_name}_delayed
        DEPENDS ${CMAKE_BINARY_DIR}/importlibs/lib${_name}_delayed.a)

endfunction()

function(spec2def _dllname _spec_file)

    if(${ARGC} GREATER 2)
        set(_file ${ARGV2})
    else()
        get_filename_component(_file ${_spec_file} NAME_WE)
    endif()

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}.def ${CMAKE_CURRENT_BINARY_DIR}/${_file}_stubs.c
        COMMAND native-spec2def -n=${_dllname} --kill-at -a=${ARCH2} -d=${CMAKE_CURRENT_BINARY_DIR}/${_file}.def -s=${CMAKE_CURRENT_BINARY_DIR}/${_file}_stubs.c ${CMAKE_CURRENT_SOURCE_DIR}/${_spec_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_spec_file} native-spec2def)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_file}.def
        PROPERTIES GENERATED TRUE EXTERNAL_OBJECT TRUE)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_file}_stubs.c PROPERTIES GENERATED TRUE)
endfunction()

macro(macro_mc FILE)
    set(COMMAND_MC ${MINGW_PREFIX}windmc -A -b ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}.mc -r ${ODYSSEY_BINARY_DIR}/include/odyssey -h ${ODYSSEY_BINARY_DIR}/include/odyssey)
endmacro()

#pseh lib, needed with mingw
set(PSEH_LIB "pseh")

# Macros
if(PCH)
    macro(_PCH_GET_COMPILE_FLAGS _target_name _out_compile_flags _header_filename)
        # Add the precompiled header to the build
        get_filename_component(_FILE ${_header_filename} NAME)
        set(_gch_filename "${_FILE}.gch")
        list(APPEND ${_out_compile_flags} -c ${_header_filename} -o ${_gch_filename})

        # This gets us our includes
        get_directory_property(DIRINC INCLUDE_DIRECTORIES)
        foreach(item ${DIRINC})
            list(APPEND ${_out_compile_flags} -I${item})
        endforeach()

        # This our definitions
        get_directory_property(_compiler_flags DEFINITIONS)
        list(APPEND ${_out_compile_flags} ${_compiler_flags})

        # This gets any specific definitions that were added with set-target-property
        get_target_property(_target_defs ${_target_name} COMPILE_DEFINITIONS)
        if (_target_defs)
            foreach(item ${_target_defs})
                list(APPEND ${_out_compile_flags} -D${item})
            endforeach()
        endif()

        if(IS_CPP)
            list(APPEND ${_out_compile_flags} ${CMAKE_CXX_FLAGS})
        else()
            list(APPEND ${_out_compile_flags} ${CMAKE_C_FLAGS})
        endif()

        separate_arguments(${_out_compile_flags})
    endmacro()

    macro(add_pch _target_name _FILE)
        set(_header_filename ${CMAKE_CURRENT_SOURCE_DIR}/${_FILE})
        get_filename_component(_basename ${_FILE} NAME)
        set(_gch_filename ${_basename}.gch)
        _PCH_GET_COMPILE_FLAGS(${_target_name} _args ${_header_filename})

        if(IS_CPP)
            set(__lang CXX)
            set(__compiler ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1})
        else()
            set(__lang C)
            set(__compiler ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1})
        endif()

        add_custom_command(OUTPUT ${_gch_filename}
            COMMAND ${__compiler} ${_args}
            IMPLICIT_DEPENDS ${__lang} ${_header_filename}
            DEPENDS ${_header_filename} ${ARGN})
        get_target_property(_src_files ${_target_name} SOURCES)
        add_target_compile_flags(${_target_name} "-fpch-preprocess -Winvalid-pch -Wno-error=invalid-pch")
        #set dependency checking : depends on precompiled header only which already depends on deeper header
        set_target_properties(${_target_name} PROPERTIES IMPLICIT_DEPENDS_INCLUDE_TRANSFORM "\"${_basename}\"=;<${_basename}>=")
    endmacro()
else()
    macro(add_pch _target_name _FILE)
    endmacro()
endif()

function(CreateBootSectorTarget _target_name _asm_file _object_file _base_address)
    get_filename_component(OBJECT_PATH ${_object_file} PATH)
    get_filename_component(OBJECT_NAME ${_object_file} NAME)
    file(MAKE_DIRECTORY ${OBJECT_PATH})
    get_directory_property(defines COMPILE_DEFINITIONS)
    get_directory_property(includes INCLUDE_DIRECTORIES)

    foreach(arg ${defines})
        set(result_defs ${result_defs} -D${arg})
    endforeach()

    foreach(arg ${includes})
        set(result_incs -I${arg} ${result_incs})
    endforeach()

    add_custom_command(
        OUTPUT ${_object_file}
        COMMAND nasm -o ${_object_file} ${result_incs} ${result_defs} -f bin ${_asm_file}
        DEPENDS ${_asm_file})
    set_source_files_properties(${_object_file} PROPERTIES GENERATED TRUE)
    add_custom_target(${_target_name} ALL DEPENDS ${_object_file})
endfunction()

function(CreateBootSectorTarget2 _target_name _asm_file _binary_file _base_address)
    set(_object_file ${_binary_file}.o)

    add_custom_command(
        OUTPUT ${_object_file}
        COMMAND ${CMAKE_ASM_COMPILER} -x assembler-with-cpp -o ${_object_file} -I${ODYSSEY_SOURCE_DIR}/include/asm -I${ODYSSEY_BINARY_DIR}/include/asm -D__ASM__ -c ${_asm_file}
        DEPENDS ${_asm_file})

    add_custom_command(
        OUTPUT ${_binary_file}
        COMMAND native-obj2bin ${_object_file} ${_binary_file} ${_base_address}
        # COMMAND objcopy --output-target binary --image-base 0x${_base_address} ${_object_file} ${_binary_file}
        DEPENDS ${_object_file} native-obj2bin)

    set_source_files_properties(${_object_file} ${_binary_file} PROPERTIES GENERATED TRUE)

    add_custom_target(${_target_name} ALL DEPENDS ${_binary_file})

endfunction()
