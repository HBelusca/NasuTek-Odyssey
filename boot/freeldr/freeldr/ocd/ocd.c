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
#include <debug.h>

DBG_DEFAULT_CHANNEL(OCD);

BOOLEAN OcdInitialize()
{
    RegInitializeRegistry();

    CHAR FreeldrPath[MAX_PATH];
    BOOLEAN ret;
    
	//
	// Create full freeldr.ini path
	//
	MachDiskGetBootPath(FreeldrPath, sizeof(FreeldrPath));
    
    ret = OcdInitHive(FreeldrPath);
    
    if(!ret)
    {
        TRACE("OcdInitialize() Could not open OCD Hive file.");
        return FALSE;
    }
    
    return TRUE;
}

BOOLEAN OcdReadSetting(WCHAR ObjectId, WCHAR SettingName, WCHAR SettingValue, ULONG ValueSize)
{
    FRLDRHKEY SelectKey;
    FRLDRHKEY BootConfigurationKey;
    LONG Error;

    Error = RegOpenKey(NULL,
                       L"\\Registry\\Machine\\BootConfiguration\\Objects",
                       &SelectKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegOpenKey() Boot Configuration Hive Object failed (Error %u)\n", (int)Error);
        return FALSE;
    }

    Error = RegOpenKey(SelectKey,
                       ObjectId,
                       &BootConfigurationKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegOpenKey() Object '%s' failed (Error %u)\n", ObjectId, (int)Error);
        return FALSE;
    }
    
    Error = RegQueryValue(BootConfigurationKey,
                          SettingName,
                          NULL,
                          (PUCHAR)&SettingValue,
                          &ValueSize);
                          
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegQueryValue('%s') failed (Error %u)\n", SettingName, (int)Error);
        return FALSE;
    }
    
	return TRUE;
}

BOOLEAN OcdGetObjectKey(FRLDRHKEY SelectKey)
{
    LONG Error;

    Error = RegOpenKey(NULL,
                       L"\\Registry\\Machine\\BootConfiguration\\Objects",
                       &SelectKey);
    if (Error != ERROR_SUCCESS)
    {
        ERR("RegOpenKey() Boot Configuration Hive Object failed (Error %u)\n", (int)Error);
        return FALSE;
    }
    
	return TRUE;
}
