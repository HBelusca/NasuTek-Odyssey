/*
 *  Odyssey kernel
 *  Copyright (C) 2002,2003 ReactOS Team; (C) 2011 NasuTek Enterprises
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
 * FILE:             drivers/filesystem/ntfs/dirctl.c
 * PURPOSE:          NTFS filesystem driver
 * PROGRAMMER:       Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include "ntfs.h"

#define NDEBUG
#include <debug.h>

/* GLOBALS *****************************************************************/


/* FUNCTIONS ****************************************************************/

#if 0
static NTSTATUS
CdfsGetEntryName(PDEVICE_EXTENSION DeviceExt,
		 PVOID *Context,
		 PVOID *Block,
		 PLARGE_INTEGER StreamOffset,
		 ULONG DirLength,
		 PVOID *Ptr,
		 PWSTR Name,
		 PULONG pIndex,
		 PULONG pIndex2)
/*
 * FUNCTION: Retrieves the file name, be it in short or long file name format
 */
{
  PDIR_RECORD Record;
  NTSTATUS Status;
  ULONG Index = 0;
  ULONG Offset = 0;
  ULONG BlockOffset = 0;

  Record = (PDIR_RECORD)*Block;
  while(Index < *pIndex)
    {
      BlockOffset += Record->RecordLength;
      Offset += Record->RecordLength;

      Record = (PDIR_RECORD)(*Block + BlockOffset);
      if (BlockOffset >= BLOCKSIZE || Record->RecordLength == 0)
	{
	  DPRINT("Map next sector\n");
	  CcUnpinData(*Context);
	  StreamOffset->QuadPart += BLOCKSIZE;
	  Offset = ROUND_UP(Offset, BLOCKSIZE);
	  BlockOffset = 0;

	  if (!CcMapData(DeviceExt->StreamFileObject,
			 StreamOffset,
			 BLOCKSIZE, TRUE,
			 Context, Block))
	    {
	      DPRINT("CcMapData() failed\n");
	      return(STATUS_UNSUCCESSFUL);
	    }
	  Record = (PDIR_RECORD)(*Block + BlockOffset);
	}

      if (Offset >= DirLength)
	return(STATUS_NO_MORE_ENTRIES);

      Index++;
    }

  DPRINT("Index %lu  RecordLength %lu  Offset %lu\n",
	 Index, Record->RecordLength, Offset);

  if (Record->FileIdLength == 1 && Record->FileId[0] == 0)
    {
      wcscpy(Name, L".");
    }
  else if (Record->FileIdLength == 1 && Record->FileId[0] == 1)
    {
      wcscpy(Name, L"..");
    }
  else
    {
      if (DeviceExt->CdInfo.JolietLevel == 0)
	{
	  ULONG i;

	  for (i = 0; i < Record->FileIdLength && Record->FileId[i] != ';'; i++)
	    Name[i] = (WCHAR)Record->FileId[i];
	  Name[i] = 0;
	}
      else
	{
	  CdfsSwapString(Name, Record->FileId, Record->FileIdLength);
	}
    }

  DPRINT("Name '%S'\n", Name);

  *Ptr = Record;

  *pIndex = Index;

  return(STATUS_SUCCESS);
}


static NTSTATUS
CdfsFindFile(PDEVICE_EXTENSION DeviceExt,
	     PFCB Fcb,
	     PFCB Parent,
	     PWSTR FileToFind,
	     PULONG pDirIndex,
	     PULONG pDirIndex2)
/*
 * FUNCTION: Find a file
 */
{
  WCHAR name[256];
  WCHAR TempStr[2];
  PVOID Block;
  NTSTATUS Status;
  ULONG len;
  ULONG DirIndex;
  ULONG Offset;
  ULONG Read;
  BOOLEAN IsRoot;
  PVOID Context = NULL;
  ULONG DirSize;
  PUCHAR Ptr;
  PDIR_RECORD Record;
  LARGE_INTEGER StreamOffset;

  DPRINT("FindFile(Parent %x, FileToFind '%S', DirIndex: %d)\n",
	 Parent, FileToFind, pDirIndex ? *pDirIndex : 0);
  DPRINT("FindFile: old Pathname %x, old Objectname %x)\n",
	 Fcb->PathName, Fcb->ObjectName);

  IsRoot = FALSE;
  DirIndex = 0;
  if (wcslen (FileToFind) == 0)
    {
      CHECKPOINT;
      TempStr[0] = (WCHAR) '.';
      TempStr[1] = 0;
      FileToFind = (PWSTR)&TempStr;
    }

  if (Parent)
    {
      if (Parent->Entry.ExtentLocationL == DeviceExt->CdInfo.RootStart)
	{
	  IsRoot = TRUE;
	}
    }
  else
    {
      IsRoot = TRUE;
    }

  if (IsRoot == TRUE)
    {
      StreamOffset.QuadPart = (LONGLONG)DeviceExt->CdInfo.RootStart * (LONGLONG)BLOCKSIZE;
      DirSize = DeviceExt->CdInfo.RootSize;


      if (FileToFind[0] == 0 || (FileToFind[0] == '\\' && FileToFind[1] == 0)
	  || (FileToFind[0] == '.' && FileToFind[1] == 0))
	{
	  /* it's root : complete essentials fields then return ok */
	  RtlZeroMemory(Fcb, sizeof(FCB));

	  Fcb->PathName[0]='\\';
	  Fcb->ObjectName = &Fcb->PathName[1];
	  Fcb->Entry.ExtentLocationL = DeviceExt->CdInfo.RootStart;
	  Fcb->Entry.DataLengthL = DeviceExt->CdInfo.RootSize;
	  Fcb->Entry.FileFlags = 0x02; //FILE_ATTRIBUTE_DIRECTORY;

	  if (pDirIndex)
	    *pDirIndex = 0;
	  if (pDirIndex2)
	    *pDirIndex2 = 0;
	  DPRINT("CdfsFindFile: new Pathname %S, new Objectname %S)\n",Fcb->PathName, Fcb->ObjectName);
	  return (STATUS_SUCCESS);
	}
    }
  else
    {
      StreamOffset.QuadPart = (LONGLONG)Parent->Entry.ExtentLocationL * (LONGLONG)BLOCKSIZE;
      DirSize = Parent->Entry.DataLengthL;
    }

  DPRINT("StreamOffset %I64u  DirSize %lu\n", StreamOffset.QuadPart, DirSize);

  if (pDirIndex && (*pDirIndex))
    DirIndex = *pDirIndex;

  if(!CcMapData(DeviceExt->StreamFileObject, &StreamOffset,
		BLOCKSIZE, TRUE, &Context, &Block))
  {
    DPRINT("CcMapData() failed\n");
    return(STATUS_UNSUCCESSFUL);
  }

  Ptr = (PUCHAR)Block;
  while(TRUE)
    {
      Record = (PDIR_RECORD)Ptr;
      if (Record->RecordLength == 0)
	{
	  DPRINT1("Stopped!\n");
	  break;
	}

      DPRINT("RecordLength %u  ExtAttrRecordLength %u  NameLength %u\n",
	     Record->RecordLength, Record->ExtAttrRecordLength, Record->FileIdLength);

      Status = CdfsGetEntryName(DeviceExt, &Context, &Block, &StreamOffset,
				DirSize, (PVOID*)&Ptr, name, &DirIndex, pDirIndex2);
      if (Status == STATUS_NO_MORE_ENTRIES)
	{
	  break;
	}
      else if (Status == STATUS_UNSUCCESSFUL)
	{
	  /* Note: the directory cache has already been unpinned */
	  return(Status);
	}

      DPRINT("Name '%S'\n", name);

      if (wstrcmpjoki(name, FileToFind)) /* || wstrcmpjoki (name2, FileToFind)) */
	{
	  if (Parent && Parent->PathName)
	    {
	      len = wcslen(Parent->PathName);
	      memcpy(Fcb->PathName, Parent->PathName, len*sizeof(WCHAR));
	      Fcb->ObjectName=&Fcb->PathName[len];
	      if (len != 1 || Fcb->PathName[0] != '\\')
		{
		  Fcb->ObjectName[0] = '\\';
		  Fcb->ObjectName = &Fcb->ObjectName[1];
		}
	    }
	  else
	    {
	      Fcb->ObjectName=Fcb->PathName;
	      Fcb->ObjectName[0]='\\';
	      Fcb->ObjectName=&Fcb->ObjectName[1];
	    }

	  DPRINT("PathName '%S'  ObjectName '%S'\n", Fcb->PathName, Fcb->ObjectName);

	  memcpy(&Fcb->Entry, Ptr, sizeof(DIR_RECORD));
	  wcsncpy(Fcb->ObjectName, name, MAX_PATH);
	  if (pDirIndex)
	    *pDirIndex = DirIndex;

	  DPRINT("FindFile: new Pathname %S, new Objectname %S, DirIndex %d\n",
		 Fcb->PathName, Fcb->ObjectName, DirIndex);

	  CcUnpinData(Context);

	  return(STATUS_SUCCESS);
	}


      Ptr = Ptr + Record->RecordLength;
      DirIndex++;

      if (((ULONG)Ptr - (ULONG)Block) >= DirSize)
	{
	  DPRINT("Stopped!\n");
	  break;
	}
    }

  CcUnpinData(Context);

  if (pDirIndex)
    *pDirIndex = DirIndex;

  return(STATUS_UNSUCCESSFUL);
}


static NTSTATUS
CdfsGetNameInformation(PFCB Fcb,
		       PDEVICE_EXTENSION DeviceExt,
		       PFILE_NAMES_INFORMATION Info,
		       ULONG BufferLength)
{
  ULONG Length;

  DPRINT("CdfsGetNameInformation() called\n");

  Length = wcslen(Fcb->ObjectName) * sizeof(WCHAR);
  if ((sizeof (FILE_BOTH_DIRECTORY_INFORMATION) + Length) > BufferLength)
    return(STATUS_BUFFER_OVERFLOW);

  Info->FileNameLength = Length;
  Info->NextEntryOffset =
    ROUND_UP(sizeof(FILE_BOTH_DIRECTORY_INFORMATION) + Length, 4);
  memcpy(Info->FileName, Fcb->ObjectName, Length);

  return(STATUS_SUCCESS);
}


static NTSTATUS
CdfsGetDirectoryInformation(PFCB Fcb,
			    PDEVICE_EXTENSION DeviceExt,
			    PFILE_DIRECTORY_INFORMATION Info,
			    ULONG BufferLength)
{
  ULONG Length;

  DPRINT("CdfsGetDirectoryInformation() called\n");

  Length = wcslen(Fcb->ObjectName) * sizeof(WCHAR);
  if ((sizeof (FILE_BOTH_DIRECTORY_INFORMATION) + Length) > BufferLength)
    return(STATUS_BUFFER_OVERFLOW);

  Info->FileNameLength = Length;
  Info->NextEntryOffset =
    ROUND_UP(sizeof(FILE_BOTH_DIRECTORY_INFORMATION) + Length, 4);
  memcpy(Info->FileName, Fcb->ObjectName, Length);

  /* Convert file times */
  CdfsDateTimeToFileTime(Fcb,
			 &Info->CreationTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->LastAccessTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->LastWriteTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->ChangeTime);

  /* Convert file flags */
  CdfsFileFlagsToAttributes(Fcb,
			    &Info->FileAttributes);

  Info->EndOfFile.QuadPart = Fcb->Entry.DataLengthL;

  /* Make AllocSize a rounded up multiple of the sector size */
  Info->AllocationSize.QuadPart = ROUND_UP(Fcb->Entry.DataLengthL, BLOCKSIZE);

//  Info->FileIndex=;

  return(STATUS_SUCCESS);
}


static NTSTATUS
CdfsGetFullDirectoryInformation(PFCB Fcb,
				PDEVICE_EXTENSION DeviceExt,
				PFILE_FULL_DIRECTORY_INFORMATION Info,
				ULONG BufferLength)
{
  ULONG Length;

  DPRINT("CdfsGetFullDirectoryInformation() called\n");

  Length = wcslen(Fcb->ObjectName) * sizeof(WCHAR);
  if ((sizeof (FILE_BOTH_DIRECTORY_INFORMATION) + Length) > BufferLength)
    return(STATUS_BUFFER_OVERFLOW);

  Info->FileNameLength = Length;
  Info->NextEntryOffset =
    ROUND_UP(sizeof(FILE_BOTH_DIRECTORY_INFORMATION) + Length, 4);
  memcpy(Info->FileName, Fcb->ObjectName, Length);

  /* Convert file times */
  CdfsDateTimeToFileTime(Fcb,
			 &Info->CreationTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->LastAccessTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->LastWriteTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->ChangeTime);

  /* Convert file flags */
  CdfsFileFlagsToAttributes(Fcb,
			    &Info->FileAttributes);

  Info->EndOfFile.QuadPart = Fcb->Entry.DataLengthL;

  /* Make AllocSize a rounded up multiple of the sector size */
  Info->AllocationSize.QuadPart = ROUND_UP(Fcb->Entry.DataLengthL, BLOCKSIZE);

//  Info->FileIndex=;
  Info->EaSize = 0;

  return(STATUS_SUCCESS);
}


static NTSTATUS
CdfsGetBothDirectoryInformation(PFCB Fcb,
				PDEVICE_EXTENSION DeviceExt,
				PFILE_BOTH_DIRECTORY_INFORMATION Info,
				ULONG BufferLength)
{
  ULONG Length;

  DPRINT("CdfsGetBothDirectoryInformation() called\n");

  Length = wcslen(Fcb->ObjectName) * sizeof(WCHAR);
  if ((sizeof (FILE_BOTH_DIRECTORY_INFORMATION) + Length) > BufferLength)
    return(STATUS_BUFFER_OVERFLOW);

  Info->FileNameLength = Length;
  Info->NextEntryOffset =
    ROUND_UP(sizeof(FILE_BOTH_DIRECTORY_INFORMATION) + Length, 4);
  memcpy(Info->FileName, Fcb->ObjectName, Length);

  /* Convert file times */
  CdfsDateTimeToFileTime(Fcb,
			 &Info->CreationTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->LastAccessTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->LastWriteTime);
  CdfsDateTimeToFileTime(Fcb,
			 &Info->ChangeTime);

  /* Convert file flags */
  CdfsFileFlagsToAttributes(Fcb,
			    &Info->FileAttributes);

  Info->EndOfFile.QuadPart = Fcb->Entry.DataLengthL;

  /* Make AllocSize a rounded up multiple of the sector size */
  Info->AllocationSize.QuadPart = ROUND_UP(Fcb->Entry.DataLengthL, BLOCKSIZE);

//  Info->FileIndex=;
  Info->EaSize = 0;

  if (DeviceExt->CdInfo.JolietLevel == 0)
    {
      /* Standard ISO-9660 format */
      Info->ShortNameLength = Length;
      memcpy(Info->ShortName, Fcb->ObjectName, Length);
    }
  else
    {
      /* Joliet extension */

      /* FIXME: Copy or create a short file name */

      Info->ShortName[0] = 0;
      Info->ShortNameLength = 0;
    }

  return(STATUS_SUCCESS);
}
#endif

NTSTATUS
NtfsQueryDirectory(PNTFS_IRP_CONTEXT IrpContext)
{
  PIRP Irp;
  PDEVICE_OBJECT DeviceObject;
  PDEVICE_EXTENSION DeviceExtension;
  LONG BufferLength = 0;
  PUNICODE_STRING SearchPattern = NULL;
  FILE_INFORMATION_CLASS FileInformationClass;
  ULONG FileIndex = 0;
  PUCHAR Buffer = NULL;
  PFILE_NAMES_INFORMATION Buffer0 = NULL;
  PNTFS_FCB Fcb;
  PNTFS_CCB Ccb;
//  FCB TempFcb;
  BOOLEAN First = FALSE;
  PIO_STACK_LOCATION Stack;
  PFILE_OBJECT FileObject;
  NTSTATUS Status = STATUS_SUCCESS;

  DPRINT1("NtfsQueryDirectory() called\n");

  ASSERT(IrpContext);
  Irp = IrpContext->Irp;
  DeviceObject = IrpContext->DeviceObject;

  DeviceExtension = DeviceObject->DeviceExtension;
  Stack = IoGetCurrentIrpStackLocation(Irp);
  FileObject = Stack->FileObject;

  Ccb = (PNTFS_CCB)FileObject->FsContext2;
  Fcb = (PNTFS_FCB)FileObject->FsContext;

  /* Obtain the callers parameters */
  BufferLength = Stack->Parameters.QueryDirectory.Length;
  SearchPattern = Stack->Parameters.QueryDirectory.FileName;
  FileInformationClass =
    Stack->Parameters.QueryDirectory.FileInformationClass;
  FileIndex = Stack->Parameters.QueryDirectory.FileIndex;


  if (SearchPattern != NULL)
  {
    if (!Ccb->DirectorySearchPattern)
    {
      First = TRUE;
      Ccb->DirectorySearchPattern =
        ExAllocatePoolWithTag(NonPagedPool, SearchPattern->Length + sizeof(WCHAR), TAG_NTFS);
      if (!Ccb->DirectorySearchPattern)
      {
        return(STATUS_INSUFFICIENT_RESOURCES);
      }

      memcpy(Ccb->DirectorySearchPattern,
             SearchPattern->Buffer,
             SearchPattern->Length);
      Ccb->DirectorySearchPattern[SearchPattern->Length / sizeof(WCHAR)] = 0;
    }
  }
  else if (!Ccb->DirectorySearchPattern)
  {
    First = TRUE;
    Ccb->DirectorySearchPattern = ExAllocatePoolWithTag(NonPagedPool, 2 * sizeof(WCHAR), TAG_NTFS);
    if (!Ccb->DirectorySearchPattern)
    {
      return(STATUS_INSUFFICIENT_RESOURCES);
    }
    Ccb->DirectorySearchPattern[0] = L'*';
    Ccb->DirectorySearchPattern[1] = 0;
  }
  DPRINT("Search pattern '%S'\n", Ccb->DirectorySearchPattern);

  /* Determine directory index */
  if (Stack->Flags & SL_INDEX_SPECIFIED)
  {
    Ccb->Entry = Ccb->CurrentByteOffset.u.LowPart;
  }
  else if (First || (Stack->Flags & SL_RESTART_SCAN))
  {
    Ccb->Entry = 0;
  }

  /* Determine Buffer for result */
  if (Irp->MdlAddress)
  {
    Buffer = MmGetSystemAddressForMdl(Irp->MdlAddress);
  }
  else
  {
    Buffer = Irp->UserBuffer;
  }
  DPRINT("Buffer=%p tofind=%S\n", Buffer, Ccb->DirectorySearchPattern);
#if 0
  TempFcb.ObjectName = TempFcb.PathName;
  while (Status == STATUS_SUCCESS && BufferLength > 0)
    {
      Status = CdfsFindFile(DeviceExtension,
			    &TempFcb,
			    Fcb,
			    Ccb->DirectorySearchPattern,
			    &Ccb->Entry,
			    NULL);
      DPRINT("Found %S, Status=%x, entry %x\n", TempFcb.ObjectName, Status, Ccb->Entry);

      if (NT_SUCCESS(Status))
	{
	  switch (FileInformationClass)
	    {
	      case FileNameInformation:
		Status = CdfsGetNameInformation(&TempFcb,
						DeviceExtension,
						(PFILE_NAMES_INFORMATION)Buffer,
						BufferLength);
		break;

	      case FileDirectoryInformation:
		Status = CdfsGetDirectoryInformation(&TempFcb,
						     DeviceExtension,
						     (PFILE_DIRECTORY_INFORMATION)Buffer,
						     BufferLength);
		break;

	      case FileFullDirectoryInformation:
		Status = CdfsGetFullDirectoryInformation(&TempFcb,
							 DeviceExtension,
							 (PFILE_FULL_DIRECTORY_INFORMATION)Buffer,
							 BufferLength);
		break;

	      case FileBothDirectoryInformation:
		Status = NtfsGetBothDirectoryInformation(&TempFcb,
							 DeviceExtension,
							 (PFILE_BOTH_DIRECTORY_INFORMATION)Buffer,
							 BufferLength);
		break;

	      default:
		Status = STATUS_INVALID_INFO_CLASS;
	    }

	  if (Status == STATUS_BUFFER_OVERFLOW)
	    {
	      if (Buffer0)
		{
		  Buffer0->NextEntryOffset = 0;
		}
	      break;
	    }
	}
      else
	{
	  if (Buffer0)
	    {
	      Buffer0->NextEntryOffset = 0;
	    }

	  if (First)
	    {
	      Status = STATUS_NO_SUCH_FILE;
	    }
	  else
	    {
	      Status = STATUS_NO_MORE_FILES;
	    }
	  break;
	}

      Buffer0 = (PFILE_NAMES_INFORMATION)Buffer;
      Buffer0->FileIndex = FileIndex++;
      Ccb->Entry++;

      if (Stack->Flags & SL_RETURN_SINGLE_ENTRY)
	{
	  break;
	}
      BufferLength -= Buffer0->NextEntryOffset;
      Buffer += Buffer0->NextEntryOffset;
    }
#endif

  if (Buffer0)
  {
    Buffer0->NextEntryOffset = 0;
  }

  if (FileIndex > 0)
  {
    Status = STATUS_SUCCESS;
  }

//  return(Status);
  return(STATUS_NO_MORE_FILES);
}



NTSTATUS NTAPI
NtfsFsdDirectoryControl(PDEVICE_OBJECT DeviceObject,
                        PIRP Irp)
{
  PNTFS_IRP_CONTEXT IrpContext = NULL;
  NTSTATUS Status = STATUS_UNSUCCESSFUL;

  DPRINT1("NtfsDirectoryControl() called\n");

  FsRtlEnterFileSystem();
  ASSERT(DeviceObject);
  ASSERT(Irp);

  NtfsIsIrpTopLevel(Irp);

  IrpContext = NtfsAllocateIrpContext(DeviceObject, Irp);
  if (IrpContext)
  {
    switch (IrpContext->MinorFunction)
    {
      case IRP_MN_QUERY_DIRECTORY:
        Status = NtfsQueryDirectory(IrpContext);
        break;
      
      case IRP_MN_NOTIFY_CHANGE_DIRECTORY:
        DPRINT1("IRP_MN_NOTIFY_CHANGE_DIRECTORY\n");
        Status = STATUS_NOT_IMPLEMENTED;
        break;
        
      default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
  }
  else
    Status = STATUS_INSUFFICIENT_RESOURCES;
  
  Irp->IoStatus.Status = Status;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp, IO_NO_INCREMENT);
  
  if (IrpContext)
    ExFreePoolWithTag(IrpContext, 'PRIN');
  
  IoSetTopLevelIrp(NULL);
  FsRtlExitFileSystem();
  return Status;
}

/* EOF */
