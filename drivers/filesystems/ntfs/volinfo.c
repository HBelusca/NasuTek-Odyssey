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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * FILE:             drivers/filesystem/ntfs/volume.c
 * PURPOSE:          NTFS filesystem driver
 * PROGRAMMER:       Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include "ntfs.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

static NTSTATUS
NtfsGetFsVolumeInformation(PDEVICE_OBJECT DeviceObject,
                           PFILE_FS_VOLUME_INFORMATION FsVolumeInfo,
                           PULONG BufferLength)
{
  DPRINT("NtfsGetFsVolumeInformation() called\n");
  DPRINT("FsVolumeInfo = %p\n", FsVolumeInfo);
  DPRINT("BufferLength %lu\n", *BufferLength);

  DPRINT("Vpb %p\n", DeviceObject->Vpb);

  DPRINT("Required length %lu\n",
         sizeof(FILE_FS_VOLUME_INFORMATION) + DeviceObject->Vpb->VolumeLabelLength);
  DPRINT("LabelLength %hu\n",
         DeviceObject->Vpb->VolumeLabelLength);
  DPRINT("Label %*.S\n",
         DeviceObject->Vpb->VolumeLabelLength / sizeof(WCHAR),
         DeviceObject->Vpb->VolumeLabel);

  if (*BufferLength < sizeof(FILE_FS_VOLUME_INFORMATION))
    return STATUS_INFO_LENGTH_MISMATCH;

  if (*BufferLength < (sizeof(FILE_FS_VOLUME_INFORMATION) + DeviceObject->Vpb->VolumeLabelLength))
    return STATUS_BUFFER_OVERFLOW;

  /* valid entries */
  FsVolumeInfo->VolumeSerialNumber = DeviceObject->Vpb->SerialNumber;
  FsVolumeInfo->VolumeLabelLength = DeviceObject->Vpb->VolumeLabelLength;
  memcpy(FsVolumeInfo->VolumeLabel,
         DeviceObject->Vpb->VolumeLabel,
         DeviceObject->Vpb->VolumeLabelLength);

  /* dummy entries */
  FsVolumeInfo->VolumeCreationTime.QuadPart = 0;
  FsVolumeInfo->SupportsObjects = FALSE;

  *BufferLength -= (sizeof(FILE_FS_VOLUME_INFORMATION) + DeviceObject->Vpb->VolumeLabelLength);

  DPRINT("BufferLength %lu\n", *BufferLength);
  DPRINT("NtfsGetFsVolumeInformation() done\n");

  return STATUS_SUCCESS;
}


static NTSTATUS
NtfsGetFsAttributeInformation(PDEVICE_EXTENSION DeviceExt,
                              PFILE_FS_ATTRIBUTE_INFORMATION FsAttributeInfo,
                              PULONG BufferLength)
{
  DPRINT("NtfsGetFsAttributeInformation()\n");
  DPRINT("FsAttributeInfo = %p\n", FsAttributeInfo);
  DPRINT("BufferLength %lu\n", *BufferLength);
  DPRINT("Required length %lu\n", (sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 8));

  if (*BufferLength < sizeof (FILE_FS_ATTRIBUTE_INFORMATION))
    return(STATUS_INFO_LENGTH_MISMATCH);

  if (*BufferLength < (sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 8))
    return(STATUS_BUFFER_OVERFLOW);

  FsAttributeInfo->FileSystemAttributes =
    FILE_CASE_PRESERVED_NAMES | FILE_UNICODE_ON_DISK;
  FsAttributeInfo->MaximumComponentNameLength = 255;
  FsAttributeInfo->FileSystemNameLength = 8;

  memcpy(FsAttributeInfo->FileSystemName, L"NTFS", 8);

  DPRINT("Finished NtfsGetFsAttributeInformation()\n");

  *BufferLength -= (sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + 8);
  DPRINT("BufferLength %lu\n", *BufferLength);

  return(STATUS_SUCCESS);
}


static NTSTATUS
NtfsGetFsSizeInformation(PDEVICE_OBJECT DeviceObject,
                         PFILE_FS_SIZE_INFORMATION FsSizeInfo,
                         PULONG BufferLength)
{
  PDEVICE_EXTENSION DeviceExt;
  NTSTATUS Status = STATUS_SUCCESS;

  DPRINT("NtfsGetFsSizeInformation()\n");
  DPRINT("FsSizeInfo = %p\n", FsSizeInfo);

  if (*BufferLength < sizeof(FILE_FS_SIZE_INFORMATION))
    return(STATUS_BUFFER_OVERFLOW);

  DeviceExt = DeviceObject->DeviceExtension;

  FsSizeInfo->AvailableAllocationUnits.QuadPart = 0;
  FsSizeInfo->TotalAllocationUnits.QuadPart = DeviceExt->NtfsInfo.SectorCount; /* ?? */
  FsSizeInfo->SectorsPerAllocationUnit = DeviceExt->NtfsInfo.SectorsPerCluster;
  FsSizeInfo->BytesPerSector = DeviceExt->NtfsInfo.BytesPerSector;

  DPRINT("Finished NtfsGetFsSizeInformation()\n");
  if (NT_SUCCESS(Status))
    *BufferLength -= sizeof(FILE_FS_SIZE_INFORMATION);

  return(Status);
}


static NTSTATUS
NtfsGetFsDeviceInformation(PFILE_FS_DEVICE_INFORMATION FsDeviceInfo,
                           PULONG BufferLength)
{
  DPRINT("NtfsGetFsDeviceInformation()\n");
  DPRINT("FsDeviceInfo = %p\n", FsDeviceInfo);
  DPRINT("BufferLength %lu\n", *BufferLength);
  DPRINT("Required length %lu\n", sizeof(FILE_FS_DEVICE_INFORMATION));

  if (*BufferLength < sizeof(FILE_FS_DEVICE_INFORMATION))
    return(STATUS_BUFFER_OVERFLOW);

  FsDeviceInfo->DeviceType = FILE_DEVICE_DISK;
  FsDeviceInfo->Characteristics = 0; /* FIXME: fix this !! */

  DPRINT("NtfsGetFsDeviceInformation() finished.\n");

  *BufferLength -= sizeof(FILE_FS_DEVICE_INFORMATION);
  DPRINT("BufferLength %lu\n", *BufferLength);

  return(STATUS_SUCCESS);
}



NTSTATUS
NtfsQueryVolumeInformation(PNTFS_IRP_CONTEXT IrpContext)
{
  PIRP Irp;
  PDEVICE_OBJECT DeviceObject;
  FS_INFORMATION_CLASS FsInformationClass;
  PIO_STACK_LOCATION Stack;
  NTSTATUS Status = STATUS_SUCCESS;
  PVOID SystemBuffer;
  ULONG BufferLength;

  DPRINT("NtfsQueryVolumeInformation() called\n");

  ASSERT(IrpContext);

  Irp = IrpContext->Irp;
  DeviceObject = IrpContext->DeviceObject;
  Stack = IoGetCurrentIrpStackLocation(Irp);
  FsInformationClass = Stack->Parameters.QueryVolume.FsInformationClass;
  BufferLength = Stack->Parameters.QueryVolume.Length;
  SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
  RtlZeroMemory(SystemBuffer, BufferLength);

  DPRINT("FsInformationClass %d\n", FsInformationClass);
  DPRINT("SystemBuffer %p\n", SystemBuffer);

  switch (FsInformationClass)
  {
    case FileFsVolumeInformation:
      Status = NtfsGetFsVolumeInformation(DeviceObject,
                                          SystemBuffer,
                                          &BufferLength);
      break;

    case FileFsAttributeInformation:
      Status = NtfsGetFsAttributeInformation(DeviceObject->DeviceExtension,
                                             SystemBuffer,
                                             &BufferLength);
      break;

    case FileFsSizeInformation:
      Status = NtfsGetFsSizeInformation(DeviceObject,
                                        SystemBuffer,
                                        &BufferLength);
      break;

    case FileFsDeviceInformation:
      Status = NtfsGetFsDeviceInformation(SystemBuffer,
                                          &BufferLength);
      break;

    default:
      Status = STATUS_NOT_SUPPORTED;
  }

  if (NT_SUCCESS(Status))
    Irp->IoStatus.Information =
      Stack->Parameters.QueryVolume.Length - BufferLength;
  else
    Irp->IoStatus.Information = 0;

  return Status;
}


NTSTATUS
NtfsSetVolumeInformation(PNTFS_IRP_CONTEXT IrpContext)
{
  PIRP Irp;
  
  DPRINT("NtfsSetVolumeInformation() called\n");

  ASSERT(IrpContext);

  Irp = IrpContext->Irp;
  Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
  Irp->IoStatus.Information = 0;

  return STATUS_NOT_SUPPORTED;
}

/* EOF */
