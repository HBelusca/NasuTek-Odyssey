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

#pragma once

#include <pshpack1.h>
typedef struct
{
	UCHAR		DriveMapCount;		// Count of drives currently mapped
	CHAR		DriveMap[8];		// Map of BIOS drives
} DRIVE_MAP_LIST, *PDRIVE_MAP_LIST;
#include <poppack.h>

VOID	DriveMapMapDrivesInSection(PCSTR SectionName);
BOOLEAN	DriveMapIsValidDriveString(PCSTR DriveString);			// Checks the drive string ("hd0") for validity
UCHAR		DriveMapGetBiosDriveNumber(PCSTR DeviceName);			// Returns a BIOS drive number for any given device name (e.g. 0x80 for 'hd0')
VOID	DriveMapInstallInt13Handler(PDRIVE_MAP_LIST DriveMap);	// Installs the int 13h handler for the drive mapper
VOID	DriveMapRemoveInt13Handler(VOID);						// Removes a previously installed int 13h drive map handler

extern PVOID			DriveMapInt13HandlerStart;
extern PVOID			DriveMapInt13HandlerEnd;
extern ULONG				DriveMapOldInt13HandlerAddress;
extern DRIVE_MAP_LIST	DriveMapInt13HandlerMapList;
