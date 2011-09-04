/*
*  Odyssey kernel
*  Copyright (C) 2002, 2003, 2004 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* $Id: create.c 52546 2011-07-05 13:55:39Z rharabien $
*
* COPYRIGHT:        See COPYING in the top level directory
* PROJECT:          Odyssey kernel
* FILE:             services/fs/cdfs/cdfs.c
* PURPOSE:          CDROM (ISO 9660) filesystem driver
* PROGRAMMER:       Art Yerkes
*                   Eric Kohl
*/

/* INCLUDES *****************************************************************/

#include "cdfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

static NTSTATUS
CdfsMakeAbsoluteFilename(PFILE_OBJECT FileObject,
                         PUNICODE_STRING RelativeFileName,
                         PUNICODE_STRING AbsoluteFileName)
{
    USHORT Length;
    PFCB Fcb;
    NTSTATUS Status;

    DPRINT("try related for %wZ\n", RelativeFileName);
    Fcb = FileObject->FsContext;
    ASSERT(Fcb);

    /* verify related object is a directory and target name
    don't start with \. */
    if ((Fcb->Entry.FileFlags & FILE_FLAG_DIRECTORY) == 0 ||
        RelativeFileName->Buffer[0] == L'\\')
    {
        return STATUS_INVALID_PARAMETER;
    }

    /* construct absolute path name */
    Length = (wcslen(Fcb->PathName) * sizeof(WCHAR)) +
        sizeof(WCHAR) +
        RelativeFileName->Length +
        sizeof(WCHAR);
    AbsoluteFileName->Length = 0;
    AbsoluteFileName->MaximumLength = Length;
    AbsoluteFileName->Buffer = ExAllocatePool(NonPagedPool,
        Length);
    if (AbsoluteFileName->Buffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = RtlAppendUnicodeToString(AbsoluteFileName,
        Fcb->PathName);
    if (!NT_SUCCESS(Status))
    {
        RtlFreeUnicodeString(AbsoluteFileName);
        return Status;
    }

    if (!CdfsFCBIsRoot(Fcb))
    {
        Status = RtlAppendUnicodeToString(AbsoluteFileName,
            L"\\");
        if (!NT_SUCCESS(Status))
        {
            RtlFreeUnicodeString(AbsoluteFileName);
            return Status;
        }
    }

    Status = RtlAppendUnicodeStringToString(AbsoluteFileName,
        RelativeFileName);
    if (!NT_SUCCESS(Status))
    {
        RtlFreeUnicodeString(AbsoluteFileName);
        return Status;
    }

    return STATUS_SUCCESS;
}


/*
* FUNCTION: Opens a file
*/
static NTSTATUS
CdfsOpenFile(PDEVICE_EXTENSION DeviceExt,
             PFILE_OBJECT FileObject,
             PUNICODE_STRING FileName)
{
    PFCB ParentFcb;
    PFCB Fcb;
    NTSTATUS Status;
    UNICODE_STRING AbsFileName;

    DPRINT("CdfsOpenFile(%08lx, %08lx, %wZ)\n", DeviceExt, FileObject, FileName);

    if (FileObject->RelatedFileObject)
    {
        DPRINT("Converting relative filename to absolute filename\n");

        Status = CdfsMakeAbsoluteFilename(FileObject->RelatedFileObject,
            FileName,
            &AbsFileName);
        if (!NT_SUCCESS(Status))
        {
            return Status;
        }

        FileName = &AbsFileName;
    }

    Status = CdfsDeviceIoControl (DeviceExt->StorageDevice,
        IOCTL_CDROM_CHECK_VERIFY,
        NULL,
        0,
        NULL,
        0,
        FALSE);
    DPRINT ("Status %lx\n", Status);
    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_NO_MEDIA_IN_DEVICE || Status == STATUS_VERIFY_REQUIRED)
        {
            DeviceExt->VolumeDevice->Flags |= DO_VERIFY_VOLUME;
        }
        DPRINT1 ("Status %lx\n", Status);
        return Status;
    }

    DPRINT("PathName to open: %wZ\n", FileName);

    /*  try first to find an existing FCB in memory  */
    DPRINT("Checking for existing FCB in memory\n");
    Fcb = CdfsGrabFCBFromTable(DeviceExt,
        FileName);
    if (Fcb == NULL)
    {
        DPRINT("No existing FCB found, making a new one if file exists.\n");
        Status = CdfsGetFCBForFile(DeviceExt,
            &ParentFcb,
            &Fcb,
            FileName);
        if (ParentFcb != NULL)
        {
            CdfsReleaseFCB(DeviceExt,
                ParentFcb);
        }

        if (!NT_SUCCESS (Status))
        {
            DPRINT("Could not make a new FCB, status: %x\n", Status);

            if (FileName == &AbsFileName)
                RtlFreeUnicodeString(&AbsFileName);

            return Status;
        }
    }

    DPRINT("Attaching FCB to fileObject\n");
    Status = CdfsAttachFCBToFileObject(DeviceExt,
        Fcb,
        FileObject);

    if ((FileName == &AbsFileName) && AbsFileName.Buffer)
        ExFreePool(AbsFileName.Buffer);

    return Status;
}


/*
* FUNCTION: Opens a file
*/
static NTSTATUS
CdfsCreateFile(PDEVICE_OBJECT DeviceObject,
               PIRP Irp)
{
    PDEVICE_EXTENSION DeviceExt;
    PIO_STACK_LOCATION Stack;
    PFILE_OBJECT FileObject;
    ULONG RequestedDisposition;
    ULONG RequestedOptions;
    PFCB Fcb;
    NTSTATUS Status;

    DPRINT("CdfsCreateFile() called\n");

    DeviceExt = DeviceObject->DeviceExtension;
    ASSERT(DeviceExt);
    Stack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT(Stack);

    RequestedDisposition = ((Stack->Parameters.Create.Options >> 24) & 0xff);
    RequestedOptions = Stack->Parameters.Create.Options & FILE_VALID_OPTION_FLAGS;
    DPRINT("RequestedDisposition %x, RequestedOptions %x\n",
        RequestedDisposition, RequestedOptions);

    FileObject = Stack->FileObject;

    if (RequestedDisposition == FILE_CREATE ||
        RequestedDisposition == FILE_OVERWRITE_IF ||
        RequestedDisposition == FILE_SUPERSEDE)
    {
        return STATUS_ACCESS_DENIED;
    }

    Status = CdfsOpenFile(DeviceExt,
        FileObject,
        &FileObject->FileName);
    if (NT_SUCCESS(Status))
    {
        Fcb = FileObject->FsContext;

        /* Check whether the file has the requested attributes */
        if (RequestedOptions & FILE_NON_DIRECTORY_FILE && CdfsFCBIsDirectory(Fcb))
        {
            CdfsCloseFile (DeviceExt, FileObject);
            return STATUS_FILE_IS_A_DIRECTORY;
        }

        if (RequestedOptions & FILE_DIRECTORY_FILE && !CdfsFCBIsDirectory(Fcb))
        {
            CdfsCloseFile (DeviceExt, FileObject);
            return STATUS_NOT_A_DIRECTORY;
        }
    }

    /*
    * If the directory containing the file to open doesn't exist then
    * fail immediately
    */
    Irp->IoStatus.Information = (NT_SUCCESS(Status)) ? FILE_OPENED : 0;
    Irp->IoStatus.Status = Status;

    return Status;
}


NTSTATUS NTAPI
CdfsCreate(PDEVICE_OBJECT DeviceObject,
           PIRP Irp)
{
    PDEVICE_EXTENSION DeviceExt;
    NTSTATUS Status;

    if (DeviceObject == CdfsGlobalData->DeviceObject)
    {
        /* DeviceObject represents FileSystem instead of logical volume */
        DPRINT("Opening file system\n");
        Irp->IoStatus.Information = FILE_OPENED;
        Status = STATUS_SUCCESS;
        goto ByeBye;
    }

    DeviceExt = DeviceObject->DeviceExtension;

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&DeviceExt->DirResource,
        TRUE);
    Status = CdfsCreateFile(DeviceObject,
        Irp);
    ExReleaseResourceLite(&DeviceExt->DirResource);
    KeLeaveCriticalRegion();

ByeBye:
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp,
        NT_SUCCESS(Status) ? IO_DISK_INCREMENT : IO_NO_INCREMENT);

    return Status;
}

/* EOF */
