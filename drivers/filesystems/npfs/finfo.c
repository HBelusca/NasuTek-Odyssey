/*
* COPYRIGHT:  See COPYING in the top level directory
* PROJECT:    Odyssey kernel
* FILE:       drivers/fs/np/finfo.c
* PURPOSE:    Named pipe filesystem
* PROGRAMMER: Eric Kohl
*/

/* INCLUDES ******************************************************************/

#include "npfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

static
NTSTATUS
NpfsSetPipeInformation(PDEVICE_OBJECT DeviceObject,
                       PNPFS_CCB Ccb,
                       PFILE_PIPE_INFORMATION Info,
                       PULONG BufferLength)
{
    PNPFS_FCB Fcb;
    PFILE_PIPE_INFORMATION Request;
    DPRINT("NpfsSetPipeInformation()\n");

    if (*BufferLength < sizeof(FILE_PIPE_INFORMATION))
    {
        /* Buffer too small */
        return STATUS_INFO_LENGTH_MISMATCH;
    }


    /* Get the Pipe and data */
    Fcb = Ccb->Fcb;
    Request = (PFILE_PIPE_INFORMATION)Info;

    if ((Fcb->PipeType == FILE_PIPE_BYTE_STREAM_TYPE) && (Request->ReadMode == FILE_PIPE_MESSAGE_MODE))
    {
        DPRINT("Cannot change readmode to message type on a byte type pipe!\n");
        return STATUS_ACCESS_DENIED;
    }

    /* Set Pipe Data */
    if (Ccb->PipeEnd == FILE_PIPE_CLIENT_END)
    {
        Fcb->ClientReadMode = Request->ReadMode;
    }
    else 
    {
        Fcb->ServerReadMode = Request->ReadMode;
    }

    Fcb->CompletionMode =  Request->CompletionMode;

    /* Return Success */
    return STATUS_SUCCESS;
}

static
NTSTATUS
NpfsSetPipeRemoteInformation(PDEVICE_OBJECT DeviceObject,
                             PNPFS_CCB Ccb,
                             PFILE_PIPE_INFORMATION Info,
                             PULONG BufferLength)
{
    PNPFS_FCB Fcb;
    PFILE_PIPE_REMOTE_INFORMATION Request;
    DPRINT("NpfsSetPipeRemoteInformation()\n");

    if (*BufferLength < sizeof(FILE_PIPE_REMOTE_INFORMATION))
    {
        /* Buffer too small */
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Get the Pipe and data */
    Fcb = Ccb->Fcb;
    Request = (PFILE_PIPE_REMOTE_INFORMATION)Info;

    /* Set the Settings */
    Fcb->TimeOut = Request->CollectDataTime;
    Fcb->InboundQuota = Request->MaximumCollectionCount;

    /* Return Success */
    return STATUS_SUCCESS;
}

static
NTSTATUS
NpfsQueryPipeInformation(PDEVICE_OBJECT DeviceObject,
                         PNPFS_CCB Ccb,
                         PFILE_PIPE_INFORMATION Info,
                         PULONG BufferLength)
{
    PNPFS_FCB Fcb;
    ULONG ConnectionSideReadMode;
    DPRINT("NpfsQueryPipeInformation()\n");

    if (*BufferLength < sizeof(FILE_PIPE_INFORMATION))
    {
        /* Buffer too small */
        *BufferLength = sizeof(FILE_PIPE_INFORMATION);
        return STATUS_BUFFER_OVERFLOW;
    }

    /* Get the Pipe */
    Fcb = Ccb->Fcb;

    /* Clear Info */
    RtlZeroMemory(Info, sizeof(FILE_PIPE_INFORMATION));

    if (Ccb->PipeEnd == FILE_PIPE_CLIENT_END) ConnectionSideReadMode=Ccb->Fcb->ClientReadMode;
    else ConnectionSideReadMode = Ccb->Fcb->ServerReadMode;

    /* Return Info */
    Info->CompletionMode = Fcb->CompletionMode;
    Info->ReadMode = ConnectionSideReadMode;

    /* Return success */
    *BufferLength = sizeof(FILE_PIPE_INFORMATION);
    return STATUS_SUCCESS;
}

static
NTSTATUS
NpfsQueryPipeRemoteInformation(PDEVICE_OBJECT DeviceObject,
                               PNPFS_CCB Ccb,
                               PFILE_PIPE_REMOTE_INFORMATION Info,
                               PULONG BufferLength)
{
    PNPFS_FCB Fcb;
    DPRINT("NpfsQueryPipeRemoteInformation()\n");

    if (*BufferLength < sizeof(FILE_PIPE_REMOTE_INFORMATION))
    {
        /* Buffer too small */
        *BufferLength = sizeof(FILE_PIPE_REMOTE_INFORMATION);
        return STATUS_BUFFER_OVERFLOW;
    }

    /* Get the Pipe */
    Fcb = Ccb->Fcb;

    /* Clear Info */
    RtlZeroMemory(Info, sizeof(FILE_PIPE_REMOTE_INFORMATION));

    /* Return Info */
    Info->MaximumCollectionCount = Fcb->InboundQuota;
    Info->CollectDataTime = Fcb->TimeOut;

    /* Return success */
    *BufferLength = sizeof(FILE_PIPE_REMOTE_INFORMATION);
    return STATUS_SUCCESS;
}


static NTSTATUS
NpfsQueryLocalPipeInformation(PDEVICE_OBJECT DeviceObject,
                              PNPFS_CCB Ccb,
                              PFILE_PIPE_LOCAL_INFORMATION Info,
                              PULONG BufferLength)
{
    PNPFS_FCB Fcb;

    DPRINT("NpfsQueryLocalPipeInformation()\n");

    if (*BufferLength < sizeof(FILE_PIPE_REMOTE_INFORMATION))
    {
        /* Buffer too small */
        *BufferLength = sizeof(FILE_PIPE_REMOTE_INFORMATION);
        return STATUS_BUFFER_OVERFLOW;
    }

    /* Get the Pipe */
    Fcb = Ccb->Fcb;

    /* Clear Info */
    RtlZeroMemory(Info,
        sizeof(FILE_PIPE_LOCAL_INFORMATION));

    /* Return Info */
    Info->NamedPipeType = Fcb->PipeType;
    Info->NamedPipeConfiguration = Fcb->PipeConfiguration;
    Info->MaximumInstances = Fcb->MaximumInstances;
    Info->CurrentInstances = Fcb->CurrentInstances;
    Info->InboundQuota = Fcb->InboundQuota;
    Info->OutboundQuota = Fcb->OutboundQuota;
    Info->NamedPipeState = Ccb->PipeState;
    Info->NamedPipeEnd = Ccb->PipeEnd;

    if (Ccb->PipeEnd == FILE_PIPE_SERVER_END)
    {
        Info->ReadDataAvailable = Ccb->ReadDataAvailable;
        Info->WriteQuotaAvailable = Ccb->WriteQuotaAvailable;
    }
    else if (Ccb->OtherSide != NULL)
    {
        Info->ReadDataAvailable = Ccb->OtherSide->ReadDataAvailable;
        Info->WriteQuotaAvailable = Ccb->OtherSide->WriteQuotaAvailable;
    }

    *BufferLength = sizeof(FILE_PIPE_LOCAL_INFORMATION);
    return STATUS_SUCCESS;
}


NTSTATUS NTAPI
NpfsQueryInformation(PDEVICE_OBJECT DeviceObject,
                     PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    FILE_INFORMATION_CLASS FileInformationClass;
    PFILE_OBJECT FileObject;
    PNPFS_VCB Vcb;
    PNPFS_FCB Fcb;
    PNPFS_CCB Ccb;
    PVOID SystemBuffer;
    ULONG BufferLength;
    NTSTATUS Status;

    DPRINT("NpfsQueryInformation(DeviceObject %p Irp %p)\n", DeviceObject, Irp);

    IoStack = IoGetCurrentIrpStackLocation (Irp);
    FileInformationClass = IoStack->Parameters.QueryFile.FileInformationClass;
    Vcb = (PNPFS_VCB)DeviceObject->DeviceExtension;
    FileObject = IoStack->FileObject;
    Ccb = (PNPFS_CCB)FileObject->FsContext2;
    Fcb = Ccb->Fcb;

    SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
    BufferLength = IoStack->Parameters.QueryFile.Length;

    DPRINT("Pipe name: %wZ\n", &Fcb->PipeName);
    DPRINT("FileInformationClass %d\n", FileInformationClass);
    DPRINT("SystemBuffer %p\n", SystemBuffer);
    DPRINT("BufferLength %lu\n", BufferLength);

    switch (FileInformationClass)
    {
    case FilePipeInformation:
        Status = NpfsQueryPipeInformation(DeviceObject,
            Ccb,
            SystemBuffer,
            &BufferLength);
        break;

    case FilePipeLocalInformation:
        Status = NpfsQueryLocalPipeInformation(DeviceObject,
            Ccb,
            SystemBuffer,
            &BufferLength);
        break;

    case FilePipeRemoteInformation:
        Status = NpfsQueryPipeRemoteInformation(DeviceObject,
            Ccb,
            SystemBuffer,
            &BufferLength);
        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
        BufferLength = 0;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = BufferLength;

    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return Status;
}


NTSTATUS NTAPI
NpfsSetInformation(PDEVICE_OBJECT DeviceObject,
                   PIRP Irp)
{
    PIO_STACK_LOCATION IoStack;
    FILE_INFORMATION_CLASS FileInformationClass;
    PFILE_OBJECT FileObject;
    PNPFS_FCB Fcb;
    PNPFS_CCB Ccb;
    PVOID SystemBuffer;
    ULONG BufferLength;
    NTSTATUS Status;

    DPRINT("NpfsSetInformation(DeviceObject %p Irp %p)\n", DeviceObject, Irp);

    IoStack = IoGetCurrentIrpStackLocation (Irp);
    FileInformationClass = IoStack->Parameters.QueryFile.FileInformationClass;
    FileObject = IoStack->FileObject;
    Ccb = (PNPFS_CCB)FileObject->FsContext2;
    Fcb = Ccb->Fcb;

    SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
    BufferLength = IoStack->Parameters.QueryFile.Length;

    DPRINT("Pipe name: %wZ\n", &Fcb->PipeName);
    DPRINT("FileInformationClass %d\n", FileInformationClass);
    DPRINT("SystemBuffer %p\n", SystemBuffer);
    DPRINT("BufferLength %lu\n", BufferLength);

    switch (FileInformationClass)
    {
    case FilePipeInformation:
        /* Call the handler */
        Status = NpfsSetPipeInformation(DeviceObject,
            Ccb,
            SystemBuffer,
            &BufferLength);
        break;

    case FilePipeLocalInformation:
        Status = STATUS_NOT_IMPLEMENTED;
        break;

    case FilePipeRemoteInformation:
        /* Call the handler */
        Status = NpfsSetPipeRemoteInformation(DeviceObject,
            Ccb,
            SystemBuffer,
            &BufferLength);
        break;
    default:
        Status = STATUS_NOT_SUPPORTED;
    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}

/* EOF */
