list(APPEND INF_FILES
    acpi.inf
    audio.inf
    battery.inf
    bda.inf
    cdrom.inf
    cpu.inf
    disk.inf
    display.inf
    fdc.inf
    font.inf
    hal.inf
    hdc.inf
    intl.inf
    keyboard.inf
    ks.inf
    kscaptur.inf
    layout.inf
    machine.inf
    msmouse.inf
    NET_NIC.inf
    netamd.inf
    netisa.inf
    netrtpnt.inf
    nettcpip.inf
    ports.inf
    scsi.inf
    unknown.inf
    usb.inf
    usbport.inf
    usbstor.inf
    xboxdisp.inf
)

foreach(item ${INF_FILES})
    add_cd_file(FILE "${CMAKE_CURRENT_SOURCE_DIR}/${item}" DESTINATION odyssey/inf ODYSSEY_PE FOR all)
endforeach(item)

add_cd_file(FILE ${CMAKE_CURRENT_SOURCE_DIR}/syssetup.inf DESTINATION odyssey/inf ODYSSEY_PE FOR all)