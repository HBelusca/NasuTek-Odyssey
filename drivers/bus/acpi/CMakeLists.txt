
include_directories(
    include
    acpica/include)

if(ARCH MATCHES amd64)
    add_definitions(-DWIN64)
endif()

add_library(acpica
    acpica/dispatcher/dsfield.c
    acpica/dispatcher/dsinit.c
    acpica/dispatcher/dsmethod.c
    acpica/dispatcher/dsmthdat.c
    acpica/dispatcher/dsobject.c
    acpica/dispatcher/dsopcode.c
    acpica/dispatcher/dsutils.c
    acpica/dispatcher/dswexec.c
    acpica/dispatcher/dswload.c
    acpica/dispatcher/dswscope.c
    acpica/dispatcher/dswstate.c
    acpica/events/evevent.c
    acpica/events/evgpe.c
    acpica/events/evgpeblk.c
    acpica/events/evmisc.c
    acpica/events/evregion.c
    acpica/events/evrgnini.c
    acpica/events/evsci.c
    acpica/events/evxface.c
    acpica/events/evxfevnt.c
    acpica/events/evxfregn.c
    acpica/executer/exconfig.c
    acpica/executer/exconvrt.c
    acpica/executer/excreate.c
    acpica/executer/exdump.c
    acpica/executer/exfield.c
    acpica/executer/exfldio.c
    acpica/executer/exmisc.c
    acpica/executer/exmutex.c
    acpica/executer/exoparg1.c
    acpica/executer/exoparg2.c
    acpica/executer/exoparg3.c
    acpica/executer/exoparg6.c
    acpica/executer/exnames.c
    acpica/executer/exprep.c
    acpica/executer/exregion.c
    acpica/executer/exresnte.c
    acpica/executer/exresolv.c
    acpica/executer/exresop.c
    acpica/executer/exstore.c
    acpica/executer/exstoren.c
    acpica/executer/exstorob.c
    acpica/executer/exsystem.c
    acpica/executer/exutils.c
    acpica/hardware/hwacpi.c
    acpica/hardware/hwgpe.c
    acpica/hardware/hwregs.c
    acpica/hardware/hwsleep.c
    acpica/hardware/hwtimer.c
    acpica/hardware/hwvalid.c
    acpica/hardware/hwxface.c
    acpica/namespace/nsaccess.c
    acpica/namespace/nsalloc.c
    acpica/namespace/nsdump.c
    acpica/namespace/nsdumpdv.c
    acpica/namespace/nseval.c
    acpica/namespace/nsinit.c
    acpica/namespace/nsload.c
    acpica/namespace/nsnames.c
    acpica/namespace/nsobject.c
    acpica/namespace/nsparse.c
    acpica/namespace/nspredef.c
    acpica/namespace/nsrepair.c
    acpica/namespace/nsrepair2.c
    acpica/namespace/nssearch.c
    acpica/namespace/nsutils.c
    acpica/namespace/nswalk.c
    acpica/namespace/nsxfeval.c
    acpica/namespace/nsxfname.c
    acpica/namespace/nsxfobj.c
    acpica/parser/psargs.c
    acpica/parser/psloop.c
    acpica/parser/psopcode.c
    acpica/parser/psparse.c
    acpica/parser/psscope.c
    acpica/parser/pstree.c
    acpica/parser/psutils.c
    acpica/parser/pswalk.c
    acpica/parser/psxface.c
    acpica/resources/rsaddr.c
    acpica/resources/rscalc.c
    acpica/resources/rscreate.c
    acpica/resources/rsdump.c
    acpica/resources/rsinfo.c
    acpica/resources/rsio.c
    acpica/resources/rsirq.c
    acpica/resources/rslist.c
    acpica/resources/rsmemory.c
    acpica/resources/rsmisc.c
    acpica/resources/rsutils.c
    acpica/resources/rsxface.c
    acpica/tables/tbfadt.c
    acpica/tables/tbfind.c
    acpica/tables/tbinstal.c
    acpica/tables/tbutils.c
    acpica/tables/tbxface.c
    acpica/tables/tbxfroot.c
    acpica/utilities/utalloc.c
    acpica/utilities/utcache.c
    acpica/utilities/utclib.c
    acpica/utilities/utcopy.c
    acpica/utilities/utdebug.c
    acpica/utilities/utdelete.c
    acpica/utilities/uteval.c
    acpica/utilities/utglobal.c
    acpica/utilities/utids.c
    acpica/utilities/utinit.c
    acpica/utilities/utlock.c
    acpica/utilities/utmath.c
    acpica/utilities/utmisc.c
    acpica/utilities/utmutex.c
    acpica/utilities/utobject.c
    acpica/utilities/utresrc.c
    acpica/utilities/utstate.c
    acpica/utilities/uttrack.c
    acpica/utilities/utxface.c)

add_dependencies(acpica bugcodes)

add_library(acpi SHARED
    busmgr/bus.c
    busmgr/button.c
    busmgr/power.c
    busmgr/utils.c
    busmgr/system.c
    osl.c
    acpienum.c
    eval.c
    interface.c
    pnp.c
    power.c
    buspdo.c
    main.c)

set_module_type(acpi kernelmodedriver)

target_link_libraries(acpi
    wdmguid
    acpica)

add_importlibs(acpi ntoskrnl hal)

add_cd_file(TARGET acpi DESTINATION odyssey/system32/drivers NO_CAB ODYSSEY_PE FOR all)

add_subdirectory(cmbatt)
add_subdirectory(compbatt)
