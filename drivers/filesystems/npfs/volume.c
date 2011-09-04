/*
* COPYRIGHT:        See COPYING in the top level directory
* PROJECT:          Odyssey kernel
* FILE:             drivers/fs/npfs/volume.c
* PURPOSE:          Named pipe filesystem
* PROGRAMMER:       Eric Kohl
*/

/* INCLUDES *****************************************************************/

#include "npfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

static NTSTATUS
NpfsQueryFsDeviceInformation(PFILE_FS_DEVICE_INFORMATION FsDeviceInfo,
                             PULONG BufferLength)
{
    DPRINT("NpfsQueryFsDeviceInformation()\n");
    DPRINT("FsDeviceInfo = %p\n", FsDeviceInfo);

    if (*BufferLength < sizeof(FILE_FS_DEVICE_INFORMATION))
    {
        *BufferLength = sizeof(FILE_FS_DEVICE_INFORMATION);
        return STATUS_BUFFER_OVERFLOW;
    }

    FsDeviceInfo->DeviceType = FILE_DEVICE_NAMED_PIPE;
    FsDeviceInfo->Characteristics = 0;

    *BufferLength = sizeof(FILE_FS_DEVICE_INFORMATION);

    DPRINT("NpfsQueryFsDeviceInformation() finished.\n");

    return STATUS_SUCCESS;
}


static NTSTATUS
NpfsQueryFsAttributeInformation(PFILE_FS_ATTRIBUTE_INFORMATION FsAttributeInfo,
                                PULONG BufferLength)
{
    DPRINT("NpfsQueryFsAttributeInformation() called.\n");
    DPRINT("FsAttributeInfo = %p\n", FsAttributeInfo);

    if (*BufferLength < sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 8)
    {
        *BufferLength = (sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 8);
        return STATUS_BUFFER_OVERFLOW;
    }

    FsAttributeInfo->FileSystemAttributes = FILE_CASE_PRESERVED_NAMES;
    FsAttributeInfo->MaximumComponentNameLength = 255;
    FsAttributeInfo->FileSystemNameLength = 8;
    wcscpy(FsAttributeInfo->FileSystemName,
        L"NPFS");

    DPRINT("NpfsQueryFsAttributeInformation() finished.\n");
    *BufferLength = (sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 8);

    return STATUS_SUCCESS;
}


NTSTATUS NTAPI
NpfsQueryVolumeInformation(PDEVICE_OBJECT DeviceObject,
                           PIRP Irp)
{
    PIO_STACK_LOCATION Stack;
    FS_INFORMATION_CLASS FsInformationClass;
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID SystemBuffer;
    ULONG BufferLength;

    /* PRECONDITION */
    ASSERT(DeviceObject != NULL);
    ASSERT(Irp != NULL);

    DPRINT("NpfsQueryVolumeInformation(DeviceObject %p, Irp %p)\n",
        DeviceObject,
        Irp);

    Stack = IoGetCurrentIrpStackLocation(Irp);
    FsInformationClass = Stack->Parameters.QueryVolume.FsInformationClass;
    BufferLength = Stack->Parameters.QueryVolume.Length;
    SystemBuffer = Irp->AssociatedIrp.SystemBuffer;

    DPRINT("FsInformationClass %d\n", FsInformationClass);
    DPRINT("SystemBuffer %p\n", SystemBuffer);

    switch (FsInformationClass)
    {
    case FileFsDeviceInformation:
        Status = NpfsQueryFsDeviceInformation(SystemBuffer,
            &BufferLength);
        break;

    case FileFsAttributeInformation:
        Status = NpfsQueryFsAttributeInformation(SystemBuffer,
            &BufferLength);
        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = BufferLength;

    IoCompleteRequest(Irp,
        IO_NO_INCREMENT);

    return Status;
}

/* EOF */
