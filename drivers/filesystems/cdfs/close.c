/*
*  Odyssey kernel
*  Copyright (C) 2002 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* $Id: close.c 43790 2009-10-27 10:34:16Z dgorbachev $
*
* COPYRIGHT:        See COPYING in the top level directory
* PROJECT:          Odyssey kernel
* FILE:             services/fs/cdfs/close.c
* PURPOSE:          CDROM (ISO 9660) filesystem driver
* PROGRAMMER:       Art Yerkes
* UPDATE HISTORY:
*/

/* INCLUDES *****************************************************************/

#include "cdfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

NTSTATUS
CdfsCloseFile(PDEVICE_EXTENSION DeviceExt,
              PFILE_OBJECT FileObject)
              /*
              * FUNCTION: Closes a file
              */
{
    PCCB Ccb;

    DPRINT("CdfsCloseFile(DeviceExt %x, FileObject %x)\n",
        DeviceExt,
        FileObject);

    Ccb = (PCCB)(FileObject->FsContext2);

    DPRINT("Ccb %x\n", Ccb);
    if (Ccb == NULL)
    {
        return(STATUS_SUCCESS);
    }

    FileObject->FsContext2 = NULL;

    if (FileObject->FileName.Buffer)
    {
        // This a FO, that was created outside from FSD.
        // Some FO's are created with IoCreateStreamFileObject() insid from FSD.
        // This FO's don't have a FileName.
        CdfsReleaseFCB(DeviceExt, FileObject->FsContext);
    }

    if (Ccb->DirectorySearchPattern.Buffer)
    {
        ExFreePoolWithTag(Ccb->DirectorySearchPattern.Buffer, TAG_CCB);
    }
    ExFreePoolWithTag(Ccb, TAG_CCB);

    return(STATUS_SUCCESS);
}


NTSTATUS NTAPI
CdfsClose(PDEVICE_OBJECT DeviceObject,
          PIRP Irp)
{
    PDEVICE_EXTENSION DeviceExtension;
    PIO_STACK_LOCATION Stack;
    PFILE_OBJECT FileObject;
    NTSTATUS Status;

    DPRINT("CdfsClose() called\n");

    if (DeviceObject == CdfsGlobalData->DeviceObject)
    {
        DPRINT("Closing file system\n");
        Status = STATUS_SUCCESS;
        goto ByeBye;
    }

    Stack = IoGetCurrentIrpStackLocation(Irp);
    FileObject = Stack->FileObject;
    DeviceExtension = DeviceObject->DeviceExtension;

    Status = CdfsCloseFile(DeviceExtension,FileObject);

ByeBye:
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return(Status);
}

/* EOF */

