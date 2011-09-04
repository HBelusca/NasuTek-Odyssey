/*
 * FILE:             DirEntry.c
 * PURPOSE:          Routines to manipulate directory entries.
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PROGRAMMER:       Jason Filby (jasonfilby@yahoo.com)
 *                   Rex Jolliff (rex@lvcablemodem.com)
 *                   Herve Poussineau (odyssey@poussine.freesurf.fr)
 */

/*  -------------------------------------------------------  INCLUDES  */

#define NDEBUG
#include "vfat.h"

ULONG
vfatDirEntryGetFirstCluster (PDEVICE_EXTENSION  pDeviceExt,
                             PDIR_ENTRY  pFatDirEntry)
{
    ULONG cluster;

    if (pDeviceExt->FatInfo.FatType == FAT32)
    {
        cluster = pFatDirEntry->Fat.FirstCluster |
                 (pFatDirEntry->Fat.FirstClusterHigh << 16);
    }
    else if (pDeviceExt->Flags & VCB_IS_FATX)
    {
        cluster = pFatDirEntry->FatX.FirstCluster;
    }
    else
    {
        cluster = pFatDirEntry->Fat.FirstCluster;
    }

    return  cluster;
}

static
BOOLEAN
FATIsDirectoryEmpty(PVFATFCB Fcb)
{
    LARGE_INTEGER FileOffset;
    PVOID Context = NULL;
    PFAT_DIR_ENTRY FatDirEntry;
    ULONG Index, MaxIndex;

    if (vfatFCBIsRoot(Fcb))
    {
        Index = 0;
    }
    else
    {
        Index = 2;
    }

    FileOffset.QuadPart = 0;
    MaxIndex = Fcb->RFCB.FileSize.u.LowPart / sizeof(FAT_DIR_ENTRY);

    while (Index < MaxIndex)
    {
        if (Context == NULL || (Index % FAT_ENTRIES_PER_PAGE) == 0)
        {
            if (Context != NULL)
            {
                CcUnpinData(Context);
            }

            if (!CcMapData(Fcb->FileObject, &FileOffset, sizeof(FAT_DIR_ENTRY), TRUE, &Context, (PVOID*)&FatDirEntry))
            {
                return TRUE;
            }

            FatDirEntry += Index % FAT_ENTRIES_PER_PAGE;
            FileOffset.QuadPart += PAGE_SIZE;
        }

        if (FAT_ENTRY_END(FatDirEntry))
        {
            CcUnpinData(Context);
            return TRUE;
        }

        if (!FAT_ENTRY_DELETED(FatDirEntry))
        {
            CcUnpinData(Context);
            return FALSE;
        }

        Index++;
        FatDirEntry++;
    }

    if (Context)
    {
        CcUnpinData(Context);
    }

    return TRUE;
}

static
BOOLEAN
FATXIsDirectoryEmpty(PVFATFCB Fcb)
{
    LARGE_INTEGER FileOffset;
    PVOID Context = NULL;
    PFATX_DIR_ENTRY FatXDirEntry;
    ULONG Index = 0, MaxIndex;

    FileOffset.QuadPart = 0;
    MaxIndex = Fcb->RFCB.FileSize.u.LowPart / sizeof(FATX_DIR_ENTRY);

    while (Index < MaxIndex)
    {
        if (Context == NULL || (Index % FATX_ENTRIES_PER_PAGE) == 0)
        {
            if (Context != NULL)
            {
                CcUnpinData(Context);
            }

            if (!CcMapData(Fcb->FileObject, &FileOffset, sizeof(FATX_DIR_ENTRY), TRUE, &Context, (PVOID*)&FatXDirEntry))
            {
                return TRUE;
            }

            FatXDirEntry += Index % FATX_ENTRIES_PER_PAGE;
            FileOffset.QuadPart += PAGE_SIZE;
        }

        if (FATX_ENTRY_END(FatXDirEntry))
        {
            CcUnpinData(Context);
            return TRUE;
        }

        if (!FATX_ENTRY_DELETED(FatXDirEntry))
        {
            CcUnpinData(Context);
            return FALSE;
        }

        Index++;
        FatXDirEntry++;
    }

    if (Context)
    {
        CcUnpinData(Context);
    }

    return TRUE;
}

BOOLEAN
VfatIsDirectoryEmpty(PVFATFCB Fcb)
{
    if (Fcb->Flags & FCB_IS_FATX_ENTRY)
        return FATXIsDirectoryEmpty(Fcb);
    else
        return FATIsDirectoryEmpty(Fcb);
}

NTSTATUS
FATGetNextDirEntry(PVOID *pContext,
                   PVOID *pPage,
                   IN PVFATFCB pDirFcb,
                   PVFAT_DIRENTRY_CONTEXT DirContext,
                   BOOLEAN First)
{
    ULONG dirMap;
    PWCHAR pName;
    LARGE_INTEGER FileOffset;
    PFAT_DIR_ENTRY fatDirEntry;
    slot * longNameEntry;
    ULONG index;

    UCHAR CheckSum, shortCheckSum;
    USHORT i;
    BOOLEAN Valid = TRUE;
    BOOLEAN Back = FALSE;

    DirContext->LongNameU.Buffer[0] = 0;

    FileOffset.u.HighPart = 0;
    FileOffset.u.LowPart = ROUND_DOWN(DirContext->DirIndex * sizeof(FAT_DIR_ENTRY), PAGE_SIZE);

    if (*pContext == NULL || (DirContext->DirIndex % FAT_ENTRIES_PER_PAGE) == 0)
    {
        if (*pContext != NULL)
        {
            CcUnpinData(*pContext);
        }

        if (FileOffset.u.LowPart >= pDirFcb->RFCB.FileSize.u.LowPart ||
            !CcMapData(pDirFcb->FileObject, &FileOffset, PAGE_SIZE, TRUE, pContext, pPage))
        {
            *pContext = NULL;
            return STATUS_NO_MORE_ENTRIES;
        }
    }

    fatDirEntry = (PFAT_DIR_ENTRY)(*pPage) + DirContext->DirIndex % FAT_ENTRIES_PER_PAGE;
    longNameEntry = (slot*) fatDirEntry;
    dirMap = 0;

    if (First)
    {
        /* This is the first call to vfatGetNextDirEntry. Possible the start index points
         * into a long name or points to a short name with an assigned long name.
         * We must go back to the real start of the entry */
        while (DirContext->DirIndex > 0 &&
               !FAT_ENTRY_END(fatDirEntry) &&
               !FAT_ENTRY_DELETED(fatDirEntry) &&
               ((!FAT_ENTRY_LONG(fatDirEntry) && !Back) ||
               (FAT_ENTRY_LONG(fatDirEntry) && !(longNameEntry->id & 0x40))))
        {
            DirContext->DirIndex--;
            Back = TRUE;

            if ((DirContext->DirIndex % FAT_ENTRIES_PER_PAGE) == FAT_ENTRIES_PER_PAGE - 1)
            {
                CcUnpinData(*pContext);
                FileOffset.u.LowPart -= PAGE_SIZE;

                if (FileOffset.u.LowPart >= pDirFcb->RFCB.FileSize.u.LowPart ||
                    !CcMapData(pDirFcb->FileObject, &FileOffset, PAGE_SIZE, TRUE, pContext, pPage))
                {
                    *pContext = NULL;
                    return STATUS_NO_MORE_ENTRIES;
                }

                fatDirEntry = (PFAT_DIR_ENTRY)(*pPage) + DirContext->DirIndex % FAT_ENTRIES_PER_PAGE;
                longNameEntry = (slot*) fatDirEntry;
            }
            else
            {
                fatDirEntry--;
                longNameEntry--;
            }
        }

        if (Back && !FAT_ENTRY_END(fatDirEntry) &&
           (FAT_ENTRY_DELETED(fatDirEntry) || !FAT_ENTRY_LONG(fatDirEntry)))
        {
            DirContext->DirIndex++;

            if ((DirContext->DirIndex % FAT_ENTRIES_PER_PAGE) == 0)
            {
                CcUnpinData(*pContext);
                FileOffset.u.LowPart += PAGE_SIZE;

                if (FileOffset.u.LowPart >= pDirFcb->RFCB.FileSize.u.LowPart ||
                   !CcMapData(pDirFcb->FileObject, &FileOffset, PAGE_SIZE, TRUE, pContext, pPage))
                {
                    *pContext = NULL;
                    return STATUS_NO_MORE_ENTRIES;
                }

                fatDirEntry = (PFAT_DIR_ENTRY)*pPage;
                longNameEntry = (slot*) *pPage;
            }
            else
            {
                fatDirEntry++;
                longNameEntry++;
            }
        }
    }

    DirContext->StartIndex = DirContext->DirIndex;
    CheckSum = 0;

    while (TRUE)
    {
        if (FAT_ENTRY_END(fatDirEntry))
        {
            CcUnpinData(*pContext);
            *pContext = NULL;
            return STATUS_NO_MORE_ENTRIES;
        }
    
        if (FAT_ENTRY_DELETED(fatDirEntry))
        {
            dirMap = 0;
            DirContext->LongNameU.Buffer[0] = 0;
            DirContext->StartIndex = DirContext->DirIndex + 1;
        }
        else
        {
            if (FAT_ENTRY_LONG(fatDirEntry))
            {
                if (dirMap == 0)
                {
                    DPRINT ("  long name entry found at %d\n", DirContext->DirIndex);
                    RtlZeroMemory(DirContext->LongNameU.Buffer, DirContext->LongNameU.MaximumLength);
                    CheckSum = longNameEntry->alias_checksum;
                    Valid = TRUE;
                }

                DPRINT("  name chunk1:[%.*S] chunk2:[%.*S] chunk3:[%.*S]\n",
                    5, longNameEntry->name0_4,
                    6, longNameEntry->name5_10,
                    2, longNameEntry->name11_12);

                index = (longNameEntry->id & 0x1f) - 1;
                dirMap |= 1 << index;
                pName = DirContext->LongNameU.Buffer + 13 * index;

                RtlCopyMemory(pName, longNameEntry->name0_4, 5 * sizeof(WCHAR));
                RtlCopyMemory(pName + 5, longNameEntry->name5_10, 6 * sizeof(WCHAR));
                RtlCopyMemory(pName + 11, longNameEntry->name11_12, 2 * sizeof(WCHAR));

                DPRINT ("  longName: [%S]\n", DirContext->LongNameU.Buffer);

                if (CheckSum != longNameEntry->alias_checksum)
                {
                     DPRINT1("Found wrong alias checksum in long name entry (first %x, current %x, %S)\n",
                             CheckSum, longNameEntry->alias_checksum, DirContext->LongNameU.Buffer);
                Valid = FALSE;
                }
            }
            else
            {
                shortCheckSum = 0;
                for (i = 0; i < 11; i++)
                {
                    shortCheckSum = (((shortCheckSum & 1) << 7)
                                  | ((shortCheckSum & 0xfe) >> 1))
                                  + fatDirEntry->ShortName[i];
                }

                if (shortCheckSum != CheckSum && DirContext->LongNameU.Buffer[0])
                {
                    DPRINT1("Checksum from long and short name is not equal (short: %x, long: %x, %S)\n",
                        shortCheckSum, CheckSum, DirContext->LongNameU.Buffer);
                    DirContext->LongNameU.Buffer[0] = 0;
                }

                if (Valid == FALSE)
                {
                    DirContext->LongNameU.Buffer[0] = 0;
                }

            RtlCopyMemory (&DirContext->DirEntry.Fat, fatDirEntry, sizeof (FAT_DIR_ENTRY));
            break;
            }
        }

        DirContext->DirIndex++;

        if ((DirContext->DirIndex % FAT_ENTRIES_PER_PAGE) == 0)
        {
            CcUnpinData(*pContext);
            FileOffset.u.LowPart += PAGE_SIZE;

            if (FileOffset.u.LowPart >= pDirFcb->RFCB.FileSize.u.LowPart ||
                !CcMapData(pDirFcb->FileObject, &FileOffset, PAGE_SIZE, TRUE, pContext, pPage))
            {
                *pContext = NULL;
                return STATUS_NO_MORE_ENTRIES;
            }

            fatDirEntry = (PFAT_DIR_ENTRY)*pPage;
            longNameEntry = (slot*) *pPage;
        }
        else
        {
            fatDirEntry++;
            longNameEntry++;
        }
    }

    DirContext->LongNameU.Length = wcslen(DirContext->LongNameU.Buffer) * sizeof(WCHAR);
    vfat8Dot3ToString(&DirContext->DirEntry.Fat, &DirContext->ShortNameU);

    if (DirContext->LongNameU.Length == 0)
    {
        RtlCopyUnicodeString(&DirContext->LongNameU, &DirContext->ShortNameU);
    }

    return STATUS_SUCCESS;
}

NTSTATUS FATXGetNextDirEntry(PVOID * pContext,
			     PVOID * pPage,
                             IN PVFATFCB pDirFcb,
			     PVFAT_DIRENTRY_CONTEXT DirContext,
			     BOOLEAN First)
{
   LARGE_INTEGER FileOffset;
   PFATX_DIR_ENTRY fatxDirEntry;
   OEM_STRING StringO;
   ULONG DirIndex = DirContext->DirIndex;

   FileOffset.u.HighPart = 0;

   if (!vfatFCBIsRoot(pDirFcb))
   {
      /* need to add . and .. entries */
      switch (DirContext->DirIndex)
      {
         case 0: /* entry . */
         {
            DirContext->ShortNameU.Buffer[0] = 0;
            DirContext->ShortNameU.Length = 0;
            DirContext->LongNameU.Buffer[0] = L'.';
            DirContext->LongNameU.Length = sizeof(WCHAR);
            RtlCopyMemory(&DirContext->DirEntry.FatX, &pDirFcb->entry.FatX, sizeof(FATX_DIR_ENTRY));
            DirContext->DirEntry.FatX.Filename[0] = '.';
            DirContext->DirEntry.FatX.FilenameLength = 1;
            DirContext->StartIndex = 0;
            return STATUS_SUCCESS;
         }
         case 1: /* entry .. */
         {
            DirContext->ShortNameU.Buffer[0] = 0;
            DirContext->ShortNameU.Length = 0;
            DirContext->LongNameU.Buffer[0] = DirContext->LongNameU.Buffer[1] = L'.';
            DirContext->LongNameU.Length = 2 * sizeof(WCHAR);
            RtlCopyMemory(&DirContext->DirEntry.FatX, &pDirFcb->entry.FatX, sizeof(FATX_DIR_ENTRY));
            DirContext->DirEntry.FatX.Filename[0] = DirContext->DirEntry.FatX.Filename[1] = '.';
            DirContext->DirEntry.FatX.FilenameLength = 2;
            DirContext->StartIndex = 1;
            return STATUS_SUCCESS;
         }
         default:
            DirIndex -= 2;
      }
   }

   if (*pContext == NULL || (DirIndex % FATX_ENTRIES_PER_PAGE) == 0)
   {
      if (*pContext != NULL)
      {
         CcUnpinData(*pContext);
      }
      FileOffset.u.LowPart = ROUND_DOWN(DirIndex * sizeof(FATX_DIR_ENTRY), PAGE_SIZE);
      if (FileOffset.u.LowPart >= pDirFcb->RFCB.FileSize.u.LowPart ||
          !CcMapData(pDirFcb->FileObject, &FileOffset, PAGE_SIZE, TRUE, pContext, pPage))
      {
         *pContext = NULL;
         return STATUS_NO_MORE_ENTRIES;
      }
   }

   fatxDirEntry = (PFATX_DIR_ENTRY)(*pPage) + DirIndex % FATX_ENTRIES_PER_PAGE;

   DirContext->StartIndex = DirContext->DirIndex;

   while (TRUE)
   {
      if (FATX_ENTRY_END(fatxDirEntry))
      {
          CcUnpinData(*pContext);
          *pContext = NULL;
          return STATUS_NO_MORE_ENTRIES;
      }

      if (!FATX_ENTRY_DELETED(fatxDirEntry))
      {
          RtlCopyMemory(&DirContext->DirEntry.FatX, fatxDirEntry, sizeof(FATX_DIR_ENTRY));
          break;
      }
      DirContext->DirIndex++;
      DirContext->StartIndex++;
      DirIndex++;
      if ((DirIndex % FATX_ENTRIES_PER_PAGE) == 0)
      {
         CcUnpinData(*pContext);
         FileOffset.u.LowPart += PAGE_SIZE;
         if (FileOffset.u.LowPart >= pDirFcb->RFCB.FileSize.u.LowPart ||
             !CcMapData(pDirFcb->FileObject, &FileOffset, PAGE_SIZE, TRUE, pContext, pPage))
         {
            *pContext = NULL;
   	    return STATUS_NO_MORE_ENTRIES;
         }
         fatxDirEntry = (PFATX_DIR_ENTRY)*pPage;
      }
      else
      {
         fatxDirEntry++;
      }
   }
   DirContext->ShortNameU.Buffer[0] = 0;
   DirContext->ShortNameU.Length = 0;
   StringO.Buffer = (PCHAR)fatxDirEntry->Filename;
   StringO.Length = StringO.MaximumLength = fatxDirEntry->FilenameLength;
   RtlOemStringToUnicodeString(&DirContext->LongNameU, &StringO, FALSE);
   return STATUS_SUCCESS;
}
