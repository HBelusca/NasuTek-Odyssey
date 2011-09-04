/*
*  Odyssey kernel
*  Copyright (C) 2002, 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/* $Id: cdfs.c 48654 2010-08-30 11:51:17Z mjmartin $
*
* COPYRIGHT:        See COPYING in the top level directory
* PROJECT:          Odyssey kernel
* FILE:             drivers/fs/cdfs/cdfs.c
* PURPOSE:          CDROM (ISO 9660) filesystem driver
* PROGRAMMER:       Art Yerkes
*                   Eric Kohl
*/

/* INCLUDES *****************************************************************/

#include "cdfs.h"

#define NDEBUG
#include <debug.h>

/* GLOBALS ******************************************************************/

PCDFS_GLOBAL_DATA CdfsGlobalData;


/* FUNCTIONS ****************************************************************/

NTSTATUS NTAPI
DriverEntry(PDRIVER_OBJECT DriverObject,
            PUNICODE_STRING RegistryPath)
            /*
            * FUNCTION: Called by the system to initalize the driver
            * ARGUMENTS:
            *           DriverObject = object describing this driver
            *           RegistryPath = path to our configuration entries
            * RETURNS: Success or failure
            */
{
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS Status;
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Cdfs");

    DPRINT("CDFS 0.0.3\n");

    Status = IoCreateDevice(DriverObject,
        sizeof(CDFS_GLOBAL_DATA),
        &DeviceName,
        FILE_DEVICE_CD_ROM_FILE_SYSTEM,
        0,
        FALSE,
        &DeviceObject);
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    /* Initialize global data */
    CdfsGlobalData = DeviceObject->DeviceExtension;
    RtlZeroMemory(CdfsGlobalData,
        sizeof(CDFS_GLOBAL_DATA));
    CdfsGlobalData->DriverObject = DriverObject;
    CdfsGlobalData->DeviceObject = DeviceObject;

    /* Initialize driver data */
    DeviceObject->Flags = DO_DIRECT_IO;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = CdfsClose;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = CdfsCleanup;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = CdfsCreate;
    DriverObject->MajorFunction[IRP_MJ_READ] = CdfsRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = CdfsWrite;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] =
        CdfsFileSystemControl;
    DriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] =
        CdfsDirectoryControl;
    DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] =
        CdfsQueryInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] =
        CdfsSetInformation;
    DriverObject->MajorFunction[IRP_MJ_QUERY_VOLUME_INFORMATION] =
        CdfsQueryVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_SET_VOLUME_INFORMATION] =
        CdfsSetVolumeInformation;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
        CdfsDeviceControl;

    DriverObject->DriverUnload = NULL;

    /* Cache manager */
    CdfsGlobalData->CacheMgrCallbacks.AcquireForLazyWrite = CdfsAcquireForLazyWrite;
    CdfsGlobalData->CacheMgrCallbacks.ReleaseFromLazyWrite = CdfsReleaseFromLazyWrite;
    CdfsGlobalData->CacheMgrCallbacks.AcquireForReadAhead = CdfsAcquireForLazyWrite;
    CdfsGlobalData->CacheMgrCallbacks.ReleaseFromReadAhead = CdfsReleaseFromLazyWrite;

    DeviceObject->Flags |= DO_LOW_PRIORITY_FILESYSTEM;

    IoRegisterFileSystem(DeviceObject);
    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return(STATUS_SUCCESS);
}


BOOLEAN NTAPI
CdfsAcquireForLazyWrite(IN PVOID Context,
                        IN BOOLEAN Wait)
{
    PFCB Fcb = (PFCB)Context;
    ASSERT(Fcb);
    DPRINT("CdfsAcquireForLazyWrite(): Fcb %p\n", Fcb);

    if (!ExAcquireResourceExclusiveLite(&(Fcb->MainResource), Wait))
    {
        DPRINT("CdfsAcquireForLazyWrite(): ExReleaseResourceLite failed.\n");
        return FALSE;
    }
    return TRUE;
}

VOID NTAPI
CdfsReleaseFromLazyWrite(IN PVOID Context)
{
    PFCB Fcb = (PFCB)Context;
    ASSERT(Fcb);
    DPRINT("CdfsReleaseFromLazyWrite(): Fcb %p\n", Fcb);

    ExReleaseResourceLite(&(Fcb->MainResource));
}
