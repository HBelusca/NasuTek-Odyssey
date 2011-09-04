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
 * FILE:             drivers/filesystem/ntfs/fcb.c
 * PURPOSE:          NTFS filesystem driver
 * PROGRAMMER:       Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include "ntfs.h"

#define NDEBUG
#include <debug.h>

/* GLOBALS *****************************************************************/



/* MACROS *******************************************************************/

#define TAG_FCB 'BCFI'



/* FUNCTIONS ****************************************************************/

static PWCHAR
NtfsGetNextPathElement(PWCHAR FileName)
{
  if (*FileName == L'\0')
  {
    return(NULL);
  }

  while (*FileName != L'\0' && *FileName != L'\\')
  {
    FileName++;
  }

  return(FileName);
}


static VOID
NtfsWSubString(PWCHAR pTarget, const PWCHAR pSource, size_t pLength)
{
  wcsncpy (pTarget, pSource, pLength);
  pTarget [pLength] = L'\0';
}


PNTFS_FCB
NtfsCreateFCB(PCWSTR FileName, PNTFS_VCB Vcb)
{
  PNTFS_FCB Fcb;

  ASSERT(Vcb);
  ASSERT(Vcb->Identifier.Type == NTFS_TYPE_VCB);

  Fcb = ExAllocatePoolWithTag(NonPagedPool, sizeof(NTFS_FCB), TAG_FCB);
  RtlZeroMemory(Fcb, sizeof(NTFS_FCB));

  Fcb->Identifier.Type = NTFS_TYPE_FCB;
  Fcb->Identifier.Size = sizeof(NTFS_TYPE_FCB);
  
  Fcb->Vcb = Vcb;

  if (FileName)
  {
    wcscpy(Fcb->PathName, FileName);
    if (wcsrchr(Fcb->PathName, '\\') != 0)
    {
      Fcb->ObjectName = wcsrchr(Fcb->PathName, '\\');
    }
    else
    {
      Fcb->ObjectName = Fcb->PathName;
    }
  }

  ExInitializeResourceLite(&Fcb->MainResource);
  
  Fcb->RFCB.Resource = &(Fcb->MainResource);

  return(Fcb);
}


VOID
NtfsDestroyFCB(PNTFS_FCB Fcb)
{
  ASSERT(Fcb);
  ASSERT(Fcb->Identifier.Type == NTFS_TYPE_FCB);

  ExDeleteResourceLite(&Fcb->MainResource);

  ExFreePool(Fcb);
}


BOOLEAN
NtfsFCBIsDirectory(PNTFS_FCB Fcb)
{
//  return(Fcb->entry.Attrib & FILE_ATTRIBUTE_DIRECTORY);
//  return(Fcb->Entry.FileFlags & 0x02);
  return(TRUE);
}


BOOLEAN
NtfsFCBIsRoot(PNTFS_FCB Fcb)
{
  return(wcscmp(Fcb->PathName, L"\\") == 0);
}


VOID
NtfsGrabFCB(PNTFS_VCB Vcb,
            PNTFS_FCB Fcb)
{
  KIRQL  oldIrql;

  DPRINT("grabbing FCB at %p: %S, refCount:%d\n",
         Fcb,
         Fcb->PathName,
         Fcb->RefCount);

  KeAcquireSpinLock(&Vcb->FcbListLock, &oldIrql);
  Fcb->RefCount++;
  KeReleaseSpinLock(&Vcb->FcbListLock, oldIrql);
}


VOID
NtfsReleaseFCB(PNTFS_VCB Vcb,
               PNTFS_FCB Fcb)
{
  KIRQL  oldIrql;

  DPRINT("releasing FCB at %p: %S, refCount:%d\n",
         Fcb,
         Fcb->PathName,
         Fcb->RefCount);

  KeAcquireSpinLock(&Vcb->FcbListLock, &oldIrql);
  Fcb->RefCount--;
  if (Fcb->RefCount <= 0 && !NtfsFCBIsDirectory(Fcb))
  {
    RemoveEntryList(&Fcb->FcbListEntry);
    CcUninitializeCacheMap(Fcb->FileObject, NULL, NULL);
    NtfsDestroyFCB(Fcb);
  }
  KeReleaseSpinLock(&Vcb->FcbListLock, oldIrql);
}


VOID
NtfsAddFCBToTable(PNTFS_VCB Vcb,
                  PNTFS_FCB Fcb)
{
  KIRQL  oldIrql;

  KeAcquireSpinLock(&Vcb->FcbListLock, &oldIrql);
  Fcb->Vcb = Vcb;
  InsertTailList(&Vcb->FcbListHead, &Fcb->FcbListEntry);
  KeReleaseSpinLock(&Vcb->FcbListLock, oldIrql);
}


PNTFS_FCB
NtfsGrabFCBFromTable(PNTFS_VCB Vcb,
                     PCWSTR FileName)
{
  KIRQL  oldIrql;
  PNTFS_FCB Fcb;
  PLIST_ENTRY  current_entry;

  KeAcquireSpinLock(&Vcb->FcbListLock, &oldIrql);

  if (FileName == NULL || *FileName == 0)
  {
    DPRINT("Return FCB for stream file object\n");
    Fcb = Vcb->StreamFileObject->FsContext;
    Fcb->RefCount++;
    KeReleaseSpinLock(&Vcb->FcbListLock, oldIrql);
    return(Fcb);
  }

  current_entry = Vcb->FcbListHead.Flink;
  while (current_entry != &Vcb->FcbListHead)
  {
    Fcb = CONTAINING_RECORD(current_entry, NTFS_FCB, FcbListEntry);

    DPRINT("Comparing '%S' and '%S'\n", FileName, Fcb->PathName);
    if (_wcsicmp(FileName, Fcb->PathName) == 0)
    {
      Fcb->RefCount++;
      KeReleaseSpinLock(&Vcb->FcbListLock, oldIrql);
      return(Fcb);
    }

      //FIXME: need to compare against short name in FCB here

    current_entry = current_entry->Flink;
  }
  KeReleaseSpinLock(&Vcb->FcbListLock, oldIrql);

  return(NULL);
}


NTSTATUS
NtfsFCBInitializeCache(PNTFS_VCB Vcb,
                       PNTFS_FCB Fcb)
{
  PFILE_OBJECT FileObject;
  NTSTATUS Status;
  PNTFS_CCB  newCCB;

  FileObject = IoCreateStreamFileObject(NULL, Vcb->StorageDevice);

  newCCB = ExAllocatePoolWithTag(NonPagedPool, sizeof(NTFS_CCB), TAG_CCB);
  if (newCCB == NULL)
  {
      return(STATUS_INSUFFICIENT_RESOURCES);
  }
  RtlZeroMemory(newCCB, sizeof(NTFS_CCB));

  newCCB->Identifier.Type = NTFS_TYPE_CCB;
  newCCB->Identifier.Size = sizeof(NTFS_TYPE_CCB);

  FileObject->SectionObjectPointer = &Fcb->SectionObjectPointers;
  FileObject->FsContext = Fcb;
  FileObject->FsContext2 = newCCB;
  newCCB->PtrFileObject = FileObject;
  Fcb->FileObject = FileObject;
  Fcb->Vcb = Vcb;

  Status = STATUS_SUCCESS;
  CcInitializeCacheMap(FileObject,
                       (PCC_FILE_SIZES)(&Fcb->RFCB.AllocationSize),
                       FALSE,
                       &(NtfsGlobalData->CacheMgrCallbacks),
                       Fcb);

  ObDereferenceObject(FileObject);
  Fcb->Flags |= FCB_CACHE_INITIALIZED;

  return(Status);
}


PNTFS_FCB
NtfsMakeRootFCB(PNTFS_VCB Vcb)
{
  PNTFS_FCB Fcb;

  Fcb = NtfsCreateFCB(L"\\", Vcb);

//  memset(Fcb->entry.Filename, ' ', 11);

//  Fcb->Entry.DataLengthL = Vcb->CdInfo.RootSize;
//  Fcb->Entry.ExtentLocationL = Vcb->CdInfo.RootStart;
//  Fcb->Entry.FileFlags = 0x02; // FILE_ATTRIBUTE_DIRECTORY;
  Fcb->RefCount = 1;
  Fcb->DirIndex = 0;
  Fcb->RFCB.FileSize.QuadPart = PAGE_SIZE;//Vcb->CdInfo.RootSize;
  Fcb->RFCB.ValidDataLength.QuadPart = PAGE_SIZE;//Vcb->CdInfo.RootSize;
  Fcb->RFCB.AllocationSize.QuadPart = PAGE_SIZE;//Vcb->CdInfo.RootSize;

  NtfsFCBInitializeCache(Vcb, Fcb);
  NtfsAddFCBToTable(Vcb, Fcb);
  NtfsGrabFCB(Vcb, Fcb);

  return(Fcb);
}


PNTFS_FCB
NtfsOpenRootFCB(PNTFS_VCB Vcb)
{
  PNTFS_FCB Fcb;

  Fcb = NtfsGrabFCBFromTable(Vcb, L"\\");
  if (Fcb == NULL)
  {
    Fcb = NtfsMakeRootFCB(Vcb);
  }

  return(Fcb);
}


#if 0
static VOID
NtfsGetDirEntryName(PDEVICE_EXTENSION DeviceExt,
		    PDIR_RECORD Record,
		    PWSTR Name)
/*
 * FUNCTION: Retrieves the file name, be it in short or long file name format
 */
{
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
	  NtfsSwapString(Name, Record->FileId, Record->FileIdLength);
	}
    }

  DPRINT("Name '%S'\n", Name);
}


NTSTATUS
NtfsMakeFCBFromDirEntry(PVCB Vcb,
			PFCB DirectoryFCB,
			PWSTR Name,
			PDIR_RECORD Record,
			PFCB * fileFCB)
{
  WCHAR pathName[MAX_PATH];
  PFCB rcFCB;
  ULONG Size;

  if (Name [0] != 0 && wcslen (DirectoryFCB->PathName) +
        sizeof(WCHAR) + wcslen (Name) > MAX_PATH)
    {
      return(STATUS_OBJECT_NAME_INVALID);
    }

  wcscpy(pathName, DirectoryFCB->PathName);
  if (!NtfsFCBIsRoot(DirectoryFCB))
    {
      wcscat(pathName, L"\\");
    }

  if (Name[0] != 0)
    {
      wcscat(pathName, Name);
    }
  else
    {
      WCHAR entryName[MAX_PATH];

      NtfsGetDirEntryName(Vcb, Record, entryName);
      wcscat(pathName, entryName);
    }

  rcFCB = NtfsCreateFCB(pathName, Vcb);
  memcpy(&rcFCB->Entry, Record, sizeof(DIR_RECORD));

  Size = rcFCB->Entry.DataLengthL;

  rcFCB->RFCB.FileSize.QuadPart = Size;
  rcFCB->RFCB.ValidDataLength.QuadPart = Size;
  rcFCB->RFCB.AllocationSize.QuadPart = ROUND_UP(Size, BLOCKSIZE);
//  DPRINT1("%S %d %d\n", longName, Size, (ULONG)rcFCB->RFCB.AllocationSize.QuadPart);
  NtfsFCBInitializeCache(Vcb, rcFCB);
  rcFCB->RefCount++;
  NtfsAddFCBToTable(Vcb, rcFCB);
  *fileFCB = rcFCB;

  return(STATUS_SUCCESS);
}
#endif


NTSTATUS
NtfsAttachFCBToFileObject(PNTFS_VCB Vcb,
                          PNTFS_FCB Fcb,
                          PFILE_OBJECT FileObject)
{
  PNTFS_CCB  newCCB;

  newCCB = ExAllocatePoolWithTag(NonPagedPool, sizeof(NTFS_CCB), TAG_CCB);
  if (newCCB == NULL)
  {
    return(STATUS_INSUFFICIENT_RESOURCES);
  }
  RtlZeroMemory(newCCB, sizeof(NTFS_CCB));

  newCCB->Identifier.Type = NTFS_TYPE_CCB;
  newCCB->Identifier.Size = sizeof(NTFS_TYPE_CCB);

  FileObject->SectionObjectPointer = &Fcb->SectionObjectPointers;
  FileObject->FsContext = Fcb;
  FileObject->FsContext2 = newCCB;
  newCCB->PtrFileObject = FileObject;
  Fcb->Vcb = Vcb;

  if (!(Fcb->Flags & FCB_CACHE_INITIALIZED))
  {
    CcInitializeCacheMap(FileObject,
                         (PCC_FILE_SIZES)(&Fcb->RFCB.AllocationSize),
                         FALSE,
                         NULL,
                         NULL);

    Fcb->Flags |= FCB_CACHE_INITIALIZED;
  }

  //DPRINT("file open: fcb:%x file size: %d\n", Fcb, Fcb->Entry.DataLengthL);

  return(STATUS_SUCCESS);
}


static NTSTATUS
NtfsDirFindFile(PNTFS_VCB Vcb,
                PNTFS_FCB DirectoryFcb,
                PWSTR FileToFind,
                PNTFS_FCB *FoundFCB)
{
#if 0
  WCHAR TempName[2];
  WCHAR Name[256];
  PVOID Block;
  ULONG FirstSector;
  ULONG DirSize;
  PDIR_RECORD Record;
  ULONG Offset;
  ULONG BlockOffset;
  NTSTATUS Status;

  LARGE_INTEGER StreamOffset;
  PVOID Context;

  ASSERT(DeviceExt);
  ASSERT(DirectoryFcb);
  ASSERT(FileToFind);

  DPRINT("NtfsDirFindFile(VCB:%08x, dirFCB:%08x, File:%S)\n",
	 DeviceExt,
	 DirectoryFcb,
	 FileToFind);
  DPRINT("Dir Path:%S\n", DirectoryFcb->PathName);

  /*  default to '.' if no filename specified */
  if (wcslen(FileToFind) == 0)
    {
      TempName[0] = L'.';
      TempName[1] = 0;
      FileToFind = TempName;
    }

  DirSize = DirectoryFcb->Entry.DataLengthL;
  StreamOffset.QuadPart = (LONGLONG)DirectoryFcb->Entry.ExtentLocationL * (LONGLONG)BLOCKSIZE;

  if(!CcMapData(DeviceExt->StreamFileObject, &StreamOffset,
		BLOCKSIZE, TRUE, &Context, &Block))
  {
    DPRINT("CcMapData() failed\n");
    return(STATUS_UNSUCCESSFUL);
  }

  Offset = 0;
  BlockOffset = 0;
  Record = (PDIR_RECORD)Block;
  while(TRUE)
    {
      if (Record->RecordLength == 0)
	{
	  DPRINT("RecordLength == 0  Stopped!\n");
	  break;
	}

      DPRINT("RecordLength %u  ExtAttrRecordLength %u  NameLength %u\n",
	     Record->RecordLength, Record->ExtAttrRecordLength, Record->FileIdLength);

      NtfsGetDirEntryName(DeviceExt, Record, Name);
      DPRINT("Name '%S'\n", Name);

      if (wstrcmpjoki(Name, FileToFind))
	{
	  DPRINT("Match found, %S\n", Name);
	  Status = NtfsMakeFCBFromDirEntry(DeviceExt,
					   DirectoryFcb,
					   Name,
					   Record,
					   FoundFCB);

	  CcUnpinData(Context);

	  return(Status);
	}

      Offset += Record->RecordLength;
      BlockOffset += Record->RecordLength;
      Record = (PDIR_RECORD)(Block + BlockOffset);
      if (BlockOffset >= BLOCKSIZE || Record->RecordLength == 0)
	{
	  DPRINT("Map next sector\n");
	  CcUnpinData(Context);
	  StreamOffset.QuadPart += BLOCKSIZE;
	  Offset = ROUND_UP(Offset, BLOCKSIZE);
	  BlockOffset = 0;

	  if (!CcMapData(DeviceExt->StreamFileObject,
			 &StreamOffset,
			 BLOCKSIZE, TRUE,
			 &Context, &Block))
	    {
	      DPRINT("CcMapData() failed\n");
	      return(STATUS_UNSUCCESSFUL);
	    }
	  Record = (PDIR_RECORD)(Block + BlockOffset);
	}

      if (Offset >= DirSize)
	break;
    }

  CcUnpinData(Context);
#endif
  return(STATUS_OBJECT_NAME_NOT_FOUND);
}


NTSTATUS
NtfsGetFCBForFile(PNTFS_VCB Vcb,
                  PNTFS_FCB *pParentFCB,
                  PNTFS_FCB *pFCB,
                  const PWSTR pFileName)
{
  NTSTATUS Status;
  WCHAR  pathName [MAX_PATH];
  WCHAR  elementName [MAX_PATH];
  PWCHAR  currentElement;
  PNTFS_FCB  FCB;
  PNTFS_FCB  parentFCB;

  DPRINT("NtfsGetFCBForFile(%p, %p, %p, '%S')\n",
         Vcb,
         pParentFCB,
         pFCB,
         pFileName);

  /* Dummy code */
//  FCB = NtfsOpenRootFCB(Vcb);
//  *pFCB = FCB;
//  *pParentFCB = NULL;

#if 1
  /* Trivial case, open of the root directory on volume */
  if (pFileName [0] == L'\0' || wcscmp(pFileName, L"\\") == 0)
  {
    DPRINT("returning root FCB\n");

    FCB = NtfsOpenRootFCB(Vcb);
    *pFCB = FCB;
    *pParentFCB = NULL;

    return((FCB != NULL) ? STATUS_SUCCESS : STATUS_OBJECT_PATH_NOT_FOUND);
  }
  else
  {
    currentElement = pFileName + 1;
    wcscpy (pathName, L"\\");
    FCB = NtfsOpenRootFCB (Vcb);
  }
  parentFCB = NULL;

  /* Parse filename and check each path element for existance and access */
  while (NtfsGetNextPathElement(currentElement) != 0)
  {
    /*  Skip blank directory levels */
    if ((NtfsGetNextPathElement(currentElement) - currentElement) == 0)
    {
      currentElement++;
      continue;
    }

    DPRINT("Parsing, currentElement:%S\n", currentElement);
    DPRINT("  parentFCB:%p FCB:%p\n", parentFCB, FCB);

    /* Descend to next directory level */
    if (parentFCB)
    {
      NtfsReleaseFCB(Vcb, parentFCB);
      parentFCB = NULL;
    }

    /* fail if element in FCB is not a directory */
    if (!NtfsFCBIsDirectory(FCB))
    {
      DPRINT("Element in requested path is not a directory\n");

      NtfsReleaseFCB(Vcb, FCB);
      FCB = 0;
      *pParentFCB = NULL;
      *pFCB = NULL;

      return(STATUS_OBJECT_PATH_NOT_FOUND);
    }
    parentFCB = FCB;

    /* Extract next directory level into dirName */
    NtfsWSubString(pathName,
                   pFileName,
                   NtfsGetNextPathElement(currentElement) - pFileName);
    DPRINT("  pathName:%S\n", pathName);

    FCB = NtfsGrabFCBFromTable(Vcb, pathName);
    if (FCB == NULL)
    {
      NtfsWSubString(elementName,
                     currentElement,
                     NtfsGetNextPathElement(currentElement) - currentElement);
      DPRINT("  elementName:%S\n", elementName);

      Status = NtfsDirFindFile(Vcb, parentFCB, elementName, &FCB);
      if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
      {
        *pParentFCB = parentFCB;
        *pFCB = NULL;
        currentElement = NtfsGetNextPathElement(currentElement);
        if (*currentElement == L'\0' || NtfsGetNextPathElement(currentElement + 1) == 0)
        {
          return(STATUS_OBJECT_NAME_NOT_FOUND);
        }
        else
        {
          return(STATUS_OBJECT_PATH_NOT_FOUND);
        }
      }
      else if (!NT_SUCCESS(Status))
      {
        NtfsReleaseFCB(Vcb, parentFCB);
        *pParentFCB = NULL;
        *pFCB = NULL;

        return(Status);
      }
    }
    currentElement = NtfsGetNextPathElement(currentElement);
  }

  *pParentFCB = parentFCB;
  *pFCB = FCB;
#endif

  return(STATUS_SUCCESS);
}

/* EOF */
