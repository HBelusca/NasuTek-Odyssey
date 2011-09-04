/*
 *  FreeLoader
 *
 *  Copyright (C) 2001  Rex Jolliff
 *  Copyright (C) 2001  Eric Kohl
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

#include <freeldr.h>
#include <cmlib.h>
#include <debug.h>

#define REG_DATA_SIZE_MASK                 0x7FFFFFFF
#define REG_DATA_IN_OFFSET                 0x80000000

DBG_DEFAULT_CHANNEL(REGISTRY);

/* FUNCTIONS ****************************************************************/

static PVOID
NTAPI
CmpAllocate (SIZE_T Size, BOOLEAN Paged, ULONG Tag)
{
  return MmHeapAlloc(Size);
}


static VOID
NTAPI
CmpFree (PVOID Ptr, IN ULONG Quota)
{
  MmHeapFree(Ptr);
}

static BOOLEAN
RegImportValue (PHHIVE Hive,
        PCM_KEY_VALUE ValueCell,
        FRLDRHKEY Key)
{
  PVOID DataCell;
  PWCHAR wName;
  LONG Error;
  ULONG DataLength;
  ULONG i;

  if (ValueCell->Signature != CM_KEY_VALUE_SIGNATURE)
    {
      ERR("Invalid key cell!\n");
      return FALSE;
    }

  if (ValueCell->Flags & VALUE_COMP_NAME)
    {
      wName = MmHeapAlloc ((ValueCell->NameLength + 1)*sizeof(WCHAR));
      for (i = 0; i < ValueCell->NameLength; i++)
        {
          wName[i] = ((PCHAR)ValueCell->Name)[i];
        }
      wName[ValueCell->NameLength] = 0;
    }
  else
    {
      wName = MmHeapAlloc (ValueCell->NameLength + sizeof(WCHAR));
      memcpy (wName,
      ValueCell->Name,
      ValueCell->NameLength);
      wName[ValueCell->NameLength / sizeof(WCHAR)] = 0;
    }

  DataLength = ValueCell->DataLength & REG_DATA_SIZE_MASK;

  TRACE("ValueName: '%S'\n", wName);
  TRACE("DataLength: %u\n", DataLength);

  if (DataLength <= sizeof(HCELL_INDEX) && (ValueCell->DataLength & REG_DATA_IN_OFFSET))
    {
      Error = RegSetValue(Key,
                          wName,
                          ValueCell->Type,
                          (PCHAR)&ValueCell->Data,
                          DataLength);
      if (Error != ERROR_SUCCESS)
        {
            ERR("RegSetValue() failed!\n");
            MmHeapFree (wName);
            return FALSE;
        }
    }
  else
    {
      DataCell = (PVOID)HvGetCell (Hive, ValueCell->Data);
      TRACE("DataCell: %x\n", DataCell);

      Error = RegSetValue (Key,
                           wName,
                           ValueCell->Type,
                           DataCell,
                           DataLength);

      if (Error != ERROR_SUCCESS)
        {
          ERR("RegSetValue() failed!\n");
          MmHeapFree (wName);
          return FALSE;
        }
    }

  MmHeapFree (wName);

  return TRUE;
}

static BOOLEAN
RegImportSubKey(PHHIVE Hive,
                PCM_KEY_NODE KeyCell,
                FRLDRHKEY ParentKey);

static BOOLEAN
RegImportIndexSubKey(PHHIVE Hive,
                PCM_KEY_INDEX IndexCell,
                FRLDRHKEY ParentKey)
{
    ULONG i;

    TRACE("IndexCell: %x\n", IndexCell);

    /* Enumerate and add subkeys */
    if (IndexCell->Signature == CM_KEY_INDEX_ROOT ||
        IndexCell->Signature == CM_KEY_INDEX_LEAF)
    {
        for (i = 0; i < IndexCell->Count; i++)
        {
            PCM_KEY_INDEX SubIndexCell = HvGetCell(Hive, IndexCell->List[i]);
            if (!RegImportIndexSubKey(Hive, SubIndexCell, ParentKey))
                return FALSE;
        }
    }
    else if (IndexCell->Signature == CM_KEY_FAST_LEAF ||
        IndexCell->Signature == CM_KEY_HASH_LEAF)
    {
        PCM_KEY_FAST_INDEX HashCell = (PCM_KEY_FAST_INDEX)IndexCell;
        for (i = 0; i < HashCell->Count; i++)
        {
            PCM_KEY_NODE SubKeyCell = HvGetCell(Hive, HashCell->List[i].Cell);
            if (!RegImportSubKey(Hive, SubKeyCell, ParentKey))
                return FALSE;
        }
    }
    else
    {
        ASSERT(FALSE);
    }

    return TRUE;
}


static BOOLEAN
RegImportSubKey(PHHIVE Hive,
                PCM_KEY_NODE KeyCell,
                FRLDRHKEY ParentKey)
{
    PCM_KEY_INDEX IndexCell;
    PVALUE_LIST_CELL ValueListCell;
    PCM_KEY_VALUE ValueCell = NULL;
    PWCHAR wName;
    FRLDRHKEY SubKey;
    LONG Error;
    ULONG i;


    TRACE("KeyCell: %x\n", KeyCell);
    TRACE("KeyCell->Signature: %x\n", KeyCell->Signature);
    if (KeyCell->Signature != CM_KEY_NODE_SIGNATURE)
    {
        ERR("Invalid key cell Signature!\n");
        return FALSE;
    }

    if (KeyCell->Flags & KEY_COMP_NAME)
    {
        wName = MmHeapAlloc ((KeyCell->NameLength + 1) * sizeof(WCHAR));
        for (i = 0; i < KeyCell->NameLength; i++)
        {
            wName[i] = ((PCHAR)KeyCell->Name)[i];
        }
        wName[KeyCell->NameLength] = 0;
    }
    else
    {
        wName = MmHeapAlloc (KeyCell->NameLength + sizeof(WCHAR));
        memcpy (wName,
            KeyCell->Name,
            KeyCell->NameLength);
        wName[KeyCell->NameLength/sizeof(WCHAR)] = 0;
    }

    TRACE("KeyName: '%S'\n", wName);

    /* Create new sub key */
    Error = RegCreateKey (ParentKey,
        wName,
        &SubKey);
    MmHeapFree (wName);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegCreateKey() failed!\n");
        return FALSE;
    }
    TRACE("Subkeys: %u\n", KeyCell->SubKeyCounts);
    TRACE("Values: %u\n", KeyCell->ValueList.Count);

    /* Enumerate and add values */
    if (KeyCell->ValueList.Count > 0)
    {
        ValueListCell = (PVALUE_LIST_CELL) HvGetCell (Hive, KeyCell->ValueList.List);
        TRACE("ValueListCell: %x\n", ValueListCell);

        for (i = 0; i < KeyCell->ValueList.Count; i++)
        {
            TRACE("ValueOffset[%d]: %x\n", i, ValueListCell->ValueOffset[i]);

            ValueCell = (PCM_KEY_VALUE) HvGetCell (Hive, ValueListCell->ValueOffset[i]);

            TRACE("ValueCell[%d]: %x\n", i, ValueCell);

            if (!RegImportValue(Hive, ValueCell, SubKey))
                return FALSE;
        }
    }

    /* Enumerate and add subkeys */
    if (KeyCell->SubKeyCounts[Stable] > 0)
    {
        IndexCell = HvGetCell (Hive, KeyCell->SubKeyLists[Stable]);

        if (!RegImportIndexSubKey(Hive, IndexCell, SubKey))
            return FALSE;
    }

    return TRUE;
}


BOOLEAN
RegImportBinaryHive(PCHAR ChunkBase,
		    ULONG ChunkSize)
{
  PCM_KEY_NODE KeyCell;
  PCM_KEY_FAST_INDEX HashCell;
  PCM_KEY_NODE SubKeyCell;
  FRLDRHKEY SystemKey;
  ULONG i;
  LONG Error;
  PCMHIVE CmHive;
  PHHIVE Hive;
  NTSTATUS Status;

  TRACE("RegImportBinaryHive(%x, %u) called\n",ChunkBase,ChunkSize);

  CmHive = CmpAllocate(sizeof(CMHIVE), TRUE, 0);
  Status = HvInitialize (&CmHive->Hive,
                         HINIT_FLAT,
                         0,
                         0,
                         ChunkBase, 
                         CmpAllocate,
                         CmpFree,
                         NULL,
                         NULL,
                         NULL,
                         NULL,
                         1,
                         NULL);
  if (!NT_SUCCESS(Status))
    {
      CmpFree(CmHive, 0);
      ERR("Invalid hive Signature!\n");
      return FALSE;
    }

  Hive = &CmHive->Hive;
  KeyCell = (PCM_KEY_NODE)HvGetCell (Hive, Hive->BaseBlock->RootCell);
  TRACE("KeyCell: %x\n", KeyCell);
  TRACE("KeyCell->Signature: %x\n", KeyCell->Signature);
  if (KeyCell->Signature != CM_KEY_NODE_SIGNATURE)
    {
      ERR("Invalid key cell Signature!\n");
      return FALSE;
    }

  TRACE("Subkeys: %u\n", KeyCell->SubKeyCounts);
  TRACE("Values: %u\n", KeyCell->ValueList.Count);

  /* Open 'System' key */
  Error = RegOpenKey(NULL,
             L"\\Registry\\Machine\\SYSTEM",
             &SystemKey);
  if (Error != ERROR_SUCCESS)
    {
      ERR("Failed to open 'system' key!\n");
      return FALSE;
    }

  /* Enumerate and add subkeys */
  if (KeyCell->SubKeyCounts[Stable] > 0)
    {
      HashCell = (PCM_KEY_FAST_INDEX)HvGetCell (Hive, KeyCell->SubKeyLists[Stable]);
      TRACE("HashCell: %x\n", HashCell);
      TRACE("SubKeyCounts: %x\n", KeyCell->SubKeyCounts[Stable]);

      for (i = 0; i < KeyCell->SubKeyCounts[Stable]; i++)
        {
          TRACE("Cell[%d]: %x\n", i, HashCell->List[i].Cell);

          SubKeyCell = (PCM_KEY_NODE)HvGetCell (Hive, HashCell->List[i].Cell);

          TRACE("SubKeyCell[%d]: %x\n", i, SubKeyCell);

          if (!RegImportSubKey(Hive, SubKeyCell, SystemKey))
            return FALSE;
        }
    }

  return TRUE;
}

/* EOF */
