/*
 * PROJECT:         Odyssey FAT file system driver
 * LICENSE:         GNU GPLv3 as published by the Free Software Foundation
 * FILE:            drivers/filesystems/fastfat/shutdown.c
 * PURPOSE:         Shutdown support routines
 * PROGRAMMERS:     Aleksey Bragin (aleksey@odyssey.org)
 */

/* INCLUDES *****************************************************************/

#define NDEBUG
#include "fastfat.h"

/* FUNCTIONS ****************************************************************/

NTSTATUS
NTAPI
FatShutdown(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    DPRINT1("FatShutdown(DeviceObject %p, Irp %p)\n", DeviceObject, Irp);

    return STATUS_NOT_IMPLEMENTED;
}

/* EOF */
