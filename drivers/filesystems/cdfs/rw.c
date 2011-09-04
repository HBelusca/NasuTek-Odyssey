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
/* $Id: rw.c 43790 2009-10-27 10:34:16Z dgorbachev $
*
* COPYRIGHT:        See COPYING in the top level directory
* PROJECT:          Odyssey kernel
* FILE:             drivers/fs/cdfs/rw.c
* PURPOSE:          CDROM (ISO 9660) filesystem driver
* PROGRAMMER:       Art Yerkes
*                   Eric Kohl
*/

/* INCLUDES *****************************************************************/

#include "cdfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))
#define ROUND_DOWN(N, S) ((N) - ((N) % (S)))


/* FUNCTIONS ****************************************************************/

static NTSTATUS
CdfsReadFile(PDEVICE_EXTENSION DeviceExt,
             PFILE_OBJECT FileObject,
             PUCHAR Buffer,
             ULONG Length,
             ULONG ReadOffset,
             ULONG IrpFlags,
             PULONG LengthRead)
             /*
             * FUNCTION: Reads data from a file
             */
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCCB Ccb;
    PFCB Fcb;

    DPRINT("CdfsReadFile(ReadOffset %lu  Length %lu)\n", ReadOffset, Length);

    *LengthRead = 0;

    if (Length == 0)
        return(STATUS_SUCCESS);

    Ccb = (PCCB)FileObject->FsContext2;
    Fcb = (PFCB)FileObject->FsContext;

    if (ReadOffset >= Fcb->Entry.DataLengthL)
        return(STATUS_END_OF_FILE);

    DPRINT("Reading %d bytes at %d\n", Length, ReadOffset);

    if (!(IrpFlags & (IRP_NOCACHE|IRP_PAGING_IO)))
    {
        LARGE_INTEGER FileOffset;
        IO_STATUS_BLOCK IoStatus;
        CC_FILE_SIZES FileSizes;

        if (ReadOffset + Length > Fcb->Entry.DataLengthL)
            Length = Fcb->Entry.DataLengthL - ReadOffset;

        if (FileObject->PrivateCacheMap == NULL)
        {
            FileSizes.AllocationSize = Fcb->RFCB.AllocationSize;
            FileSizes.FileSize = Fcb->RFCB.FileSize;
            FileSizes.ValidDataLength = Fcb->RFCB.ValidDataLength;

            DPRINT("Attach FCB to File: Size %08x%08x\n", 
                Fcb->RFCB.ValidDataLength.HighPart,
                Fcb->RFCB.ValidDataLength.LowPart);

            CcInitializeCacheMap(FileObject,
                &FileSizes,
                FALSE,
                &(CdfsGlobalData->CacheMgrCallbacks),
                Fcb);
        }

        FileOffset.QuadPart = (LONGLONG)ReadOffset;
        CcCopyRead(FileObject,
            &FileOffset,
            Length,
            TRUE,
            Buffer,
            &IoStatus);
        *LengthRead = IoStatus.Information;

        return(IoStatus.Status);
    }

    if ((ReadOffset % BLOCKSIZE) != 0 || (Length % BLOCKSIZE) != 0)
    {
        /* Then we need to do a partial or misaligned read ... */
        PVOID PageBuf = ExAllocatePool(NonPagedPool, BLOCKSIZE);
        PCHAR ReadInPage = (PCHAR)PageBuf + (ReadOffset & (BLOCKSIZE - 1));
        PCHAR TargetRead = (PCHAR)Buffer;
        ULONG ActualReadOffset, EndOfExtent, ReadLen;

        if (!PageBuf)
        {
            return STATUS_NO_MEMORY;
        }

        ActualReadOffset = ReadOffset & ~(BLOCKSIZE - 1);
        EndOfExtent = ReadOffset + Length;

        while (ActualReadOffset < EndOfExtent)
        {
            Status = CdfsReadSectors
                (DeviceExt->StorageDevice,
                Fcb->Entry.ExtentLocationL + (ActualReadOffset / BLOCKSIZE),
                1,
                PageBuf,
                FALSE);

            if (!NT_SUCCESS(Status))
                break;

            ReadLen = BLOCKSIZE - (ActualReadOffset & (BLOCKSIZE - 1));
            if (ReadLen > EndOfExtent - ActualReadOffset)
            {
                ReadLen = EndOfExtent - ActualReadOffset;
            }

            RtlCopyMemory(TargetRead, ReadInPage, ReadLen);

            ActualReadOffset += ReadLen;
            TargetRead += ReadLen;	  
        }

        ExFreePool(PageBuf);
    }
    else
    {
        if (ReadOffset + Length > ROUND_UP(Fcb->Entry.DataLengthL, BLOCKSIZE))
            Length = ROUND_UP(Fcb->Entry.DataLengthL, BLOCKSIZE) - ReadOffset;

        Status = CdfsReadSectors(DeviceExt->StorageDevice,
            Fcb->Entry.ExtentLocationL + (ReadOffset / BLOCKSIZE),
            Length / BLOCKSIZE,
            Buffer,
            FALSE);
        if (NT_SUCCESS(Status))
        {
            *LengthRead = Length;
            if (Length + ReadOffset > Fcb->Entry.DataLengthL)
            {
                memset(Buffer + Fcb->Entry.DataLengthL - ReadOffset,
                    0,
                    Length + ReadOffset - Fcb->Entry.DataLengthL);
            }
        }
    }

    return Status;
}


NTSTATUS NTAPI
CdfsRead(PDEVICE_OBJECT DeviceObject,
         PIRP Irp)
{
    PDEVICE_EXTENSION DeviceExt;
    PIO_STACK_LOCATION Stack;
    PFILE_OBJECT FileObject;
    PVOID Buffer = NULL;
    ULONG ReadLength;
    LARGE_INTEGER ReadOffset;
    ULONG ReturnedReadLength = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    DPRINT("CdfsRead(DeviceObject %x, Irp %x)\n",DeviceObject,Irp);

    DeviceExt = DeviceObject->DeviceExtension;
    Stack = IoGetCurrentIrpStackLocation(Irp);
    FileObject = Stack->FileObject;

    ReadLength = Stack->Parameters.Read.Length;
    ReadOffset = Stack->Parameters.Read.ByteOffset;
    if (ReadLength) Buffer = MmGetSystemAddressForMdl(Irp->MdlAddress);

    Status = CdfsReadFile(DeviceExt,
        FileObject,
        Buffer,
        ReadLength,
        ReadOffset.u.LowPart,
        Irp->Flags,
        &ReturnedReadLength);
    if (NT_SUCCESS(Status))
    {
        if (FileObject->Flags & FO_SYNCHRONOUS_IO)
        {
            FileObject->CurrentByteOffset.QuadPart =
                ReadOffset.QuadPart + ReturnedReadLength;
        }
        Irp->IoStatus.Information = ReturnedReadLength;
    }
    else
    {
        Irp->IoStatus.Information = 0;
    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return(Status);
}


NTSTATUS NTAPI
CdfsWrite(PDEVICE_OBJECT DeviceObject,
          PIRP Irp)
{
    DPRINT("CdfsWrite(DeviceObject %x Irp %x)\n",DeviceObject,Irp);

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    Irp->IoStatus.Information = 0;
    return(STATUS_NOT_SUPPORTED);
}

/* EOF */
