/*
 *  FreeLoader
 *
 *  Copyright (C) 2001, 2002  Eric Kohl
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
#include <debug.h>

DBG_DEFAULT_CHANNEL(REGISTRY);

static FRLDRHKEY RootKey;

VOID
RegInitializeRegistry (VOID)
{
    /* Create root key */
    RootKey = MmHeapAlloc(sizeof(KEY));

    InitializeListHead(&RootKey->SubKeyList);
    InitializeListHead(&RootKey->ValueList);
    InitializeListHead(&RootKey->KeyList);

    RootKey->SubKeyCount = 0;
    RootKey->ValueCount = 0;

    RootKey->NameSize = 4;
    RootKey->Name = MmHeapAlloc(4);
    wcscpy (RootKey->Name, L"\\");

    RootKey->DataType = 0;
    RootKey->DataSize = 0;
    RootKey->Data = NULL;

    /* Create 'SYSTEM' key */
    RegCreateKey (RootKey,
                  L"Registry\\Machine\\SYSTEM",
                  NULL);
}


LONG
RegInitCurrentControlSet(BOOLEAN LastKnownGood)
{
    WCHAR ControlSetKeyName[80];
    FRLDRHKEY SelectKey;
    FRLDRHKEY SystemKey;
    FRLDRHKEY ControlSetKey;
    FRLDRHKEY LinkKey;
    ULONG CurrentSet = 0;
    ULONG DefaultSet = 0;
    ULONG LastKnownGoodSet = 0;
    ULONG DataSize;
    LONG Error;

    Error = RegOpenKey(NULL,
                       L"\\Registry\\Machine\\SYSTEM\\Select",
                       &SelectKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegOpenKey() failed (Error %u)\n", (int)Error);
        return Error;
    }

    DataSize = sizeof(ULONG);
    Error = RegQueryValue(SelectKey,
                          L"Default",
                          NULL,
                          (PUCHAR)&DefaultSet,
                          &DataSize);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegQueryValue('Default') failed (Error %u)\n", (int)Error);
        return Error;
    }

    DataSize = sizeof(ULONG);
    Error = RegQueryValue(SelectKey,
                          L"LastKnownGood",
                          NULL,
                          (PUCHAR)&LastKnownGoodSet,
                          &DataSize);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegQueryValue('Default') failed (Error %u)\n", (int)Error);
        return Error;
    }

    CurrentSet = (LastKnownGood == TRUE) ? LastKnownGoodSet : DefaultSet;
    wcscpy(ControlSetKeyName, L"ControlSet");
    switch(CurrentSet)
    {
        case 1:
            wcscat(ControlSetKeyName, L"001");
            break;
        case 2:
            wcscat(ControlSetKeyName, L"002");
            break;
        case 3:
            wcscat(ControlSetKeyName, L"003");
            break;
        case 4:
            wcscat(ControlSetKeyName, L"004");
            break;
        case 5:
            wcscat(ControlSetKeyName, L"005");
            break;
    }

    Error = RegOpenKey(NULL,
                       L"\\Registry\\Machine\\SYSTEM",
                       &SystemKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegOpenKey(SystemKey) failed (Error %lu)\n", Error);
        return Error;
    }

    Error = RegOpenKey(SystemKey,
                       ControlSetKeyName,
                       &ControlSetKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegOpenKey(ControlSetKey) failed (Error %lu)\n", Error);
        return Error;
    }

    Error = RegCreateKey(SystemKey,
                         L"CurrentControlSet",
                         &LinkKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegCreateKey(LinkKey) failed (Error %lu)\n", Error);
        return Error;
    }

    Error = RegSetValue(LinkKey,
                        NULL,
                        REG_LINK,
                        (PCHAR)&ControlSetKey,
                        sizeof(PVOID));
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegSetValue(LinkKey) failed (Error %lu)\n", Error);
        return Error;
    }

    return ERROR_SUCCESS;
}


LONG
RegCreateKey(FRLDRHKEY ParentKey,
             PCWSTR KeyName,
             PFRLDRHKEY Key)
{
    PLIST_ENTRY Ptr;
    FRLDRHKEY SearchKey = NULL;
    FRLDRHKEY CurrentKey;
    FRLDRHKEY NewKey;
    PWCHAR p;
    PCWSTR name;
    int subkeyLength;
    int stringLength;
    ULONG NameSize;
    int CmpResult;

    TRACE("KeyName '%S'\n", KeyName);

    if (*KeyName == L'\\')
    {
        KeyName++;
        CurrentKey = RootKey;
    }
    else if (ParentKey == NULL)
    {
        CurrentKey = RootKey;
    }
    else
    {
        CurrentKey = ParentKey;
    }

    /* Check whether current key is a link */
    if (CurrentKey->DataType == REG_LINK)
    {
        CurrentKey = (FRLDRHKEY)CurrentKey->Data;
    }

    while (*KeyName != 0)
    {
        TRACE("KeyName '%S'\n", KeyName);

        if (*KeyName == L'\\')
            KeyName++;
        p = wcschr(KeyName, L'\\');
        if ((p != NULL) && (p != KeyName))
        {
            subkeyLength = p - KeyName;
            stringLength = subkeyLength + 1;
            name = KeyName;
        }
        else
        {
            subkeyLength = wcslen(KeyName);
            stringLength = subkeyLength;
            name = KeyName;
        }
        NameSize = (subkeyLength + 1) * sizeof(WCHAR);

        Ptr = CurrentKey->SubKeyList.Flink;
        CmpResult = 1;
        while (Ptr != &CurrentKey->SubKeyList)
        {
            TRACE("Ptr 0x%x\n", Ptr);

            SearchKey = CONTAINING_RECORD(Ptr, KEY, KeyList);
            TRACE("SearchKey 0x%x\n", SearchKey);
            TRACE("Searching '%S'\n", SearchKey->Name);
            CmpResult = _wcsnicmp(SearchKey->Name, name, subkeyLength);

            if (CmpResult == 0 && SearchKey->NameSize == NameSize) break;
            else if (CmpResult == -1) break;

            Ptr = Ptr->Flink;
        }

        if (CmpResult != 0)
        {
            /* no key found -> create new subkey */
            NewKey = MmHeapAlloc(sizeof(KEY));
            if (NewKey == NULL) return ERROR_OUTOFMEMORY;

            InitializeListHead(&NewKey->SubKeyList);
            InitializeListHead(&NewKey->ValueList);

            NewKey->SubKeyCount = 0;
            NewKey->ValueCount = 0;

            NewKey->DataType = 0;
            NewKey->DataSize = 0;
            NewKey->Data = NULL;

            InsertTailList(Ptr, &NewKey->KeyList);
            CurrentKey->SubKeyCount++;

            NewKey->NameSize = NameSize;
            NewKey->Name = (PWCHAR)MmHeapAlloc(NewKey->NameSize);
            if (NewKey->Name == NULL) return ERROR_OUTOFMEMORY;

            memcpy(NewKey->Name, name, NewKey->NameSize - sizeof(WCHAR));
            NewKey->Name[subkeyLength] = 0;

            TRACE("NewKey 0x%x\n", NewKey);
            TRACE("NewKey '%S'  Length %d\n", NewKey->Name, NewKey->NameSize);

            CurrentKey = NewKey;
        }
        else
        {
            CurrentKey = SearchKey;

            /* Check whether current key is a link */
            if (CurrentKey->DataType == REG_LINK)
            {
                CurrentKey = (FRLDRHKEY)CurrentKey->Data;
            }
        }

        KeyName = KeyName + stringLength;
    }

    if (Key != NULL) *Key = CurrentKey;

    return ERROR_SUCCESS;
}


LONG
RegDeleteKey(FRLDRHKEY Key,
             PCWSTR Name)
{

    if (wcschr(Name, L'\\') != NULL) return ERROR_INVALID_PARAMETER;

    return ERROR_SUCCESS;
}


LONG
RegEnumKey(FRLDRHKEY Key,
           ULONG Index,
           PWCHAR Name,
           ULONG* NameSize)
{
    PLIST_ENTRY Ptr;
    FRLDRHKEY SearchKey;
    ULONG Count = 0;
    ULONG Size;

    Ptr = Key->SubKeyList.Flink;
    while (Ptr != &Key->SubKeyList)
    {
        if (Index == Count) break;

        Count++;
        Ptr = Ptr->Flink;
    }

    if (Ptr == &Key->SubKeyList) return ERROR_NO_MORE_ITEMS;

    SearchKey = CONTAINING_RECORD(Ptr, KEY, KeyList);

    TRACE("Name '%S'  Length %d\n", SearchKey->Name, SearchKey->NameSize);

    Size = min(SearchKey->NameSize, *NameSize);
    *NameSize = Size;
    memcpy(Name, SearchKey->Name, Size);

    return ERROR_SUCCESS;
}


LONG
RegOpenKey(FRLDRHKEY ParentKey,
           PCWSTR KeyName,
           PFRLDRHKEY Key)
{
    PLIST_ENTRY Ptr;
    FRLDRHKEY SearchKey = NULL;
    FRLDRHKEY CurrentKey;
    PWCHAR p;
    PCWSTR name;
    int subkeyLength;
    int stringLength;
    ULONG NameSize;

    TRACE("KeyName '%S'\n", KeyName);

    *Key = NULL;

    if (*KeyName == L'\\')
    {
        KeyName++;
        CurrentKey = RootKey;
    }
    else if (ParentKey == NULL)
    {
        CurrentKey = RootKey;
    }
    else
    {
        CurrentKey = ParentKey;
    }

    /* Check whether current key is a link */
    if (CurrentKey->DataType == REG_LINK)
    {
        CurrentKey = (FRLDRHKEY)CurrentKey->Data;
    }

    while (*KeyName != 0)
    {
        TRACE("KeyName '%S'\n", KeyName);

        if (*KeyName == L'\\') KeyName++;
        p = wcschr(KeyName, L'\\');
        if ((p != NULL) && (p != KeyName))
        {
            subkeyLength = p - KeyName;
            stringLength = subkeyLength + 1;
            name = KeyName;
        }
        else
        {
            subkeyLength = wcslen(KeyName);
            stringLength = subkeyLength;
            name = KeyName;
        }
        NameSize = (subkeyLength + 1) * sizeof(WCHAR);

        Ptr = CurrentKey->SubKeyList.Flink;
        while (Ptr != &CurrentKey->SubKeyList)
        {
            TRACE("Ptr 0x%x\n", Ptr);

            SearchKey = CONTAINING_RECORD(Ptr, KEY, KeyList);

            TRACE("SearchKey 0x%x\n", SearchKey);
            TRACE("Searching '%S'\n", SearchKey->Name);

            if (SearchKey->NameSize == NameSize &&
                _wcsnicmp(SearchKey->Name, name, subkeyLength) == 0) break;

            Ptr = Ptr->Flink;
        }

        if (Ptr == &CurrentKey->SubKeyList)
        {
            return ERROR_PATH_NOT_FOUND;
        }
        else
        {
            CurrentKey = SearchKey;

            /* Check whether current key is a link */
            if (CurrentKey->DataType == REG_LINK)
            {
                CurrentKey = (FRLDRHKEY)CurrentKey->Data;
            }
        }

        KeyName = KeyName + stringLength;
    }

    if (Key != NULL)
        *Key = CurrentKey;

    return ERROR_SUCCESS;
}


LONG
RegSetValue(FRLDRHKEY Key,
            PCWSTR ValueName,
            ULONG Type,
            PCSTR Data,
            ULONG DataSize)
{
    PLIST_ENTRY Ptr;
    PVALUE Value = NULL;

    TRACE("Key 0x%p, ValueName '%S', Type %ld, Data 0x%p, DataSize %ld\n",
            Key, ValueName, Type, Data, DataSize);

    if ((ValueName == NULL) || (*ValueName == 0))
    {
        /* set default value */
        if ((Key->Data != NULL) && (Key->DataSize > sizeof(PUCHAR)))
        {
            MmHeapFree(Key->Data);
        }

        if (DataSize <= sizeof(PUCHAR))
        {
            Key->DataSize = DataSize;
            Key->DataType = Type;
            memcpy(&Key->Data, Data, DataSize);
        }
        else
        {
            Key->Data = MmHeapAlloc(DataSize);
            Key->DataSize = DataSize;
            Key->DataType = Type;
            memcpy(Key->Data, Data, DataSize);
        }
    }
    else
    {
        /* set non-default value */
        Ptr = Key->ValueList.Flink;
        while (Ptr != &Key->ValueList)
        {
            Value = CONTAINING_RECORD(Ptr, VALUE, ValueList);

            TRACE("Value->Name '%S'\n", Value->Name);

            if (_wcsicmp(Value->Name, ValueName) == 0) break;

            Ptr = Ptr->Flink;
        }

        if (Ptr == &Key->ValueList)
        {
            /* add new value */
            TRACE("No value found - adding new value\n");

            Value = (PVALUE)MmHeapAlloc(sizeof(VALUE));
            if (Value == NULL) return ERROR_OUTOFMEMORY;

            InsertTailList(&Key->ValueList, &Value->ValueList);
            Key->ValueCount++;

            Value->NameSize = (wcslen(ValueName)+1) * sizeof(WCHAR);
            Value->Name = MmHeapAlloc(Value->NameSize);
            if (Value->Name == NULL) return ERROR_OUTOFMEMORY;
            wcscpy(Value->Name, ValueName);
            Value->DataType = REG_NONE;
            Value->DataSize = 0;
            Value->Data = NULL;
        }

        /* set new value */
        if ((Value->Data != NULL) && (Value->DataSize > sizeof(PUCHAR)))
        {
            MmHeapFree(Value->Data);
        }

        if (DataSize <= sizeof(PUCHAR))
        {
            Value->DataSize = DataSize;
            Value->DataType = Type;
            memcpy(&Value->Data, Data, DataSize);
        }
        else
        {
            Value->Data = MmHeapAlloc(DataSize);
            if (Value->Data == NULL) return ERROR_OUTOFMEMORY;
            Value->DataType = Type;
            Value->DataSize = DataSize;
            memcpy(Value->Data, Data, DataSize);
        }
    }
    return(ERROR_SUCCESS);
}


LONG
RegQueryValue(FRLDRHKEY Key,
              PCWSTR ValueName,
              ULONG* Type,
              PUCHAR Data,
              ULONG* DataSize)
{
    ULONG Size;
    PLIST_ENTRY Ptr;
    PVALUE Value = NULL;

    if ((ValueName == NULL) || (*ValueName == 0))
    {
        /* query default value */
        if (Key->Data == NULL) return ERROR_INVALID_PARAMETER;

        if (Type != NULL)
            *Type = Key->DataType;
        if ((Data != NULL) && (DataSize != NULL))
        {
            if (Key->DataSize <= sizeof(PUCHAR))
            {
                Size = min(Key->DataSize, *DataSize);
                memcpy(Data, &Key->Data, Size);
                *DataSize = Size;
            }
            else
            {
                Size = min(Key->DataSize, *DataSize);
                memcpy(Data, Key->Data, Size);
                *DataSize = Size;
            }
        }
        else if ((Data == NULL) && (DataSize != NULL))
        {
            *DataSize = Key->DataSize;
        }
    }
    else
    {
        /* query non-default value */
        Ptr = Key->ValueList.Flink;
        while (Ptr != &Key->ValueList)
        {
            Value = CONTAINING_RECORD(Ptr, VALUE, ValueList);

            TRACE("Searching for '%S'. Value name '%S'\n", ValueName, Value->Name);

            if (_wcsicmp(Value->Name, ValueName) == 0) break;

            Ptr = Ptr->Flink;
        }

        if (Ptr == &Key->ValueList) return ERROR_INVALID_PARAMETER;

        if (Type != NULL) *Type = Value->DataType;
        if ((Data != NULL) && (DataSize != NULL))
        {
            if (Value->DataSize <= sizeof(PUCHAR))
            {
                Size = min(Value->DataSize, *DataSize);
                memcpy(Data, &Value->Data, Size);
                *DataSize = Size;
            }
            else
            {
                Size = min(Value->DataSize, *DataSize);
                memcpy(Data, Value->Data, Size);
                *DataSize = Size;
            }
        }
        else if ((Data == NULL) && (DataSize != NULL))
        {
            *DataSize = Value->DataSize;
        }
    }

    return ERROR_SUCCESS;
}


LONG
RegDeleteValue(FRLDRHKEY Key,
               PCWSTR ValueName)
{
    PLIST_ENTRY Ptr;
    PVALUE Value = NULL;

    if ((ValueName == NULL) || (*ValueName == 0))
    {
        /* delete default value */
        if (Key->Data != NULL) MmFreeMemory(Key->Data);
        Key->Data = NULL;
        Key->DataSize = 0;
        Key->DataType = 0;
    }
    else
    {
        /* delete non-default value */
        Ptr = Key->ValueList.Flink;
        while (Ptr != &Key->ValueList)
        {
            Value = CONTAINING_RECORD(Ptr, VALUE, ValueList);
            if (_wcsicmp(Value->Name, ValueName) == 0) break;

            Ptr = Ptr->Flink;
        }

        if (Ptr == &Key->ValueList) return ERROR_INVALID_PARAMETER;

        /* delete value */
        Key->ValueCount--;
        if (Value->Name != NULL) MmFreeMemory(Value->Name);
        Value->Name = NULL;
        Value->NameSize = 0;

        if (Value->DataSize > sizeof(PUCHAR))
        {
            if (Value->Data != NULL) MmFreeMemory(Value->Data);
        }
        Value->Data = NULL;
        Value->DataSize = 0;
        Value->DataType = 0;

        RemoveEntryList(&Value->ValueList);
        MmFreeMemory(Value);
    }
    return ERROR_SUCCESS;
}


LONG
RegEnumValue(FRLDRHKEY Key,
             ULONG Index,
             PWCHAR ValueName,
             ULONG* NameSize,
             ULONG* Type,
             PUCHAR Data,
             ULONG* DataSize)
{
    PLIST_ENTRY Ptr;
    PVALUE Value;
    ULONG Count = 0;

    if (Key->Data != NULL)
    {
        if (Index > 0)
        {
            Index--;
        }
        else
        {
            /* enumerate default value */
            if (ValueName != NULL) *ValueName = 0;
            if (Type != NULL) *Type = Key->DataType;
            if (Data != NULL)
            {
                if (Key->DataSize <= sizeof(PUCHAR))
                {
                    memcpy(Data, &Key->Data, min(Key->DataSize, *DataSize));
                }
                else
                {
                    memcpy(Data, Key->Data, min(Key->DataSize, *DataSize));
                }
            }

            if (DataSize != NULL) *DataSize = min(Key->DataSize, *DataSize);

            return ERROR_SUCCESS;
        }
    }

    Ptr = Key->ValueList.Flink;
    while (Ptr != &Key->ValueList)
    {
        if (Index == Count) break;

        Count++;
        Ptr = Ptr->Flink;
    }

    if (Ptr == &Key->ValueList) return ERROR_NO_MORE_ITEMS;

    Value = CONTAINING_RECORD(Ptr, VALUE, ValueList);

    /* enumerate non-default value */
    if (ValueName != NULL)
    {
        memcpy(ValueName, Value->Name, min(Value->NameSize, *NameSize));
    }
    if (Type != NULL) *Type = Value->DataType;

    if (Data != NULL)
    {
        if (Value->DataSize <= sizeof(PUCHAR))
        {
            memcpy(Data, &Value->Data, min(Value->DataSize, *DataSize));
        }
        else
        {
            memcpy(Data, Value->Data, min(Value->DataSize, *DataSize));
        }
    }

    if (DataSize != NULL) *DataSize = min(Value->DataSize, *DataSize);

    return ERROR_SUCCESS;
}


ULONG
RegGetSubKeyCount (FRLDRHKEY Key)
{
    return Key->SubKeyCount;
}


ULONG
RegGetValueCount (FRLDRHKEY Key)
{
    if (Key->DataSize != 0) return Key->ValueCount + 1;

    return Key->ValueCount;
}

/* EOF */
