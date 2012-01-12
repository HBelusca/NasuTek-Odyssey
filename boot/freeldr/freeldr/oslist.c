/*
 *  FreeLoader
 *  Copyright (C) 1998-2003  Brian Palmer  <brianp@sginet.com>
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

static PCSTR CopyString(PCSTR Source)
{
	PSTR Dest;

	if (!Source)
		return NULL;
	Dest = MmHeapAlloc(strlen(Source) + 1);
	if (Dest)
	{
		strcpy(Dest, Source);
	}

	return Dest;
}

struct ObjectInfoHolder
{
    WCHAR Name[256];
};
typedef struct ObjectInfoHolder ObjectHolder;

OperatingSystemItem* InitOperatingSystemList(ULONG* OperatingSystemCountPointer)
{
	ULONG Idx;
	CHAR SettingName[260];
	CHAR SettingValue[260];
	ULONG_PTR SectionId;
	PCHAR TitleStart, TitleEnd;
	PCSTR OsLoadOptions;
	ULONG Count;
    ULONG Index;
    ULONG ValueSize;
	OperatingSystemItem* Items;
    FRLDRHKEY OcdObjects;
    ULONG rc;
    BOOLEAN ret;
    WCHAR ObjectName[256];
    WCHAR ObjectValue[256];
    WCHAR ObjectFriendlyName[256];
    WCHAR ObjectLoadOptions[256];
    ObjectHolder *Objects;
    
    Objects = MmHeapAlloc(255 * sizeof(ObjectHolder));
    
	//
    // First get the Ocd Object Key
    //
    ret = OcdGetObjectKey(&OcdObjects);
    
    if (!ret)
    {
        return NULL;
    }
    
    //
    // Now lets get each object name and count of object for the rest of the functions
    //
	Count = 0;
    Index = 0;
    
    while (TRUE)
    {
        /* Get the Driver's Name */
        ValueSize = sizeof(ObjectName);
        rc = RegEnumKey(OcdObjects, Index, ObjectName, &ValueSize);
        //TRACE_CH(ODYSSEY, "RegEnumKey(): rc %d\n", (int)rc);

        /* Makre sure it's valid, and check if we're done */
        if (rc == ERROR_NO_MORE_ITEMS)
            break;
        if (rc != ERROR_SUCCESS)
        {
            return NULL;
        }

        //
        // Next see if it is type of BootItem, if so, add to the list of bootitems needed
        // to be registered and increase the count val
        //
        ret = OcdReadSetting(ObjectName, L"ObjectType", &ObjectValue, sizeof(ObjectValue));
        if(strcmp(ObjectValue, "BootItem"))
        {
            strcpy(ObjectName, Objects[Count].Name);
            Count++;
        }
        
        Index++;
    }
		
	//
	// Allocate memory to hold operating system lists
	//
	Items = MmHeapAlloc(Count * sizeof(OperatingSystemItem));
	if (!Items)
	{
		return NULL;
	}

	//
	// Now loop through and read the operating system section and display names
	//
	for (Idx = 0; Idx < Count; Idx++)
	{
        strcpy(Objects[Idx].Name, ObjectName);
        
        OcdReadSetting(ObjectName, L"Name", &ObjectFriendlyName, sizeof(ObjectFriendlyName));
        OcdReadSetting(ObjectName, L"Options", &ObjectLoadOptions, sizeof(ObjectLoadOptions));
        
		//
		// Copy the system partition, identifier and options
		//
		Items[Idx].SystemPartition = CopyString(ObjectName);
		Items[Idx].LoadIdentifier = CopyString(ObjectFriendlyName);
		Items[Idx].OsLoadOptions = CopyString(ObjectLoadOptions);
	}

	//
	// Return success
	//
	*OperatingSystemCountPointer = Count;
	return Items;
}
