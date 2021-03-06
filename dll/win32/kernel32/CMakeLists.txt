
add_definitions(-D_KERNEL32_)

include_directories(${ODYSSEY_SOURCE_DIR}/include/odyssey/subsys)

set_rc_compiler()

spec2def(kernel32.dll kernel32.spec)

list(APPEND SOURCE
	client/appcache.c
	client/atom.c
	client/compname.c
    client/debugger.c
	client/dllmain.c
	client/environ.c
    client/except.c
    client/fiber.c
	client/handle.c
    client/heapmem.c
    client/job.c
	client/loader.c
	client/perfcnt.c
	client/power.c
    client/proc.c
	client/resntfy.c
    client/session.c
    client/synch.c
	client/sysinfo.c
	client/time.c
	client/timerqueue.c
	client/toolhelp.c
	client/utils.c
    client/thread.c
	client/vdm.c
	client/version.c
    client/virtmem.c
    client/vista.c
    client/file/backup.c
    client/file/bintype.c
    client/file/cnotify.c
    client/file/copy.c
    client/file/console.c
    client/file/create.c
    client/file/curdir.c
    client/file/delete.c
    client/file/deviceio.c
    client/file/dir.c
    client/file/dosdev.c
    client/file/file.c
    client/file/filemap.c
    client/file/find.c
    client/file/hardlink.c
    client/file/iocompl.c
    client/file/lfile.c
    client/file/lock.c
    client/file/mailslot.c
    client/file/move.c
    client/file/npipe.c
    client/file/pipe.c
    client/file/rw.c
    client/file/tape.c
    client/file/volume.c
    wine/actctx.c
    wine/comm.c
    wine/lzexpand.c
    wine/muldiv.c
    wine/profile.c
    wine/res.c
    wine/timezone.c
    winnls/string/casemap.c
    winnls/string/chartype.c
    winnls/string/collation.c
    winnls/string/fold.c
    winnls/string/format_msg.c
    winnls/string/lang.c
    winnls/string/lcformat.c
    winnls/string/lstring.c
    winnls/string/nls.c
    winnls/string/sortkey.c
    kernel32.rc
    ${CMAKE_CURRENT_BINARY_DIR}/kernel32.def)

if(ARCH MATCHES i386)
list(APPEND SOURCE
    client/i386/fiber.S
    client/i386/thread.S)
elseif(ARCH MATCHES amd64)
list(APPEND SOURCE
    client/amd64/fiber.S
    client/amd64/thread.S)
endif(ARCH MATCHES i386)

add_library(kernel32 SHARED ${SOURCE})

set_module_type(kernel32 win32dll ENTRYPOINT DllMain 12)

target_link_libraries(kernel32 wine chkstk ${PSEH_LIB})

add_importlibs(kernel32 ntdll)
add_pch(kernel32 k32.h)
add_dependencies(kernel32 psdk errcodes asm)
add_cd_file(TARGET kernel32 DESTINATION odyssey/system32 ODYSSEY_PE ODYSSEY_PE FOR all)
add_importlib_target(kernel32.spec)
