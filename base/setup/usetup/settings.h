/*
 *  Odyssey kernel
 *  Copyright (C) 2004 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* $Id: settings.h 45685 2010-02-26 11:43:19Z gedmurphy $
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey text-mode setup
 * FILE:            subsys/system/usetup/settings.h
 * PURPOSE:         Device settings support functions
 * PROGRAMMER:      Eric Kohl
 */

#pragma once

PGENERIC_LIST
CreateComputerTypeList(HINF InfFile);

PGENERIC_LIST
CreateDisplayDriverList(HINF InfFile);

BOOLEAN
ProcessComputerFiles(HINF InfFile,
		       PGENERIC_LIST List,
		       PWCHAR* AdditionalSectionName);

BOOLEAN
ProcessDisplayRegistry(HINF InfFile,
		       PGENERIC_LIST List);

PGENERIC_LIST
CreateKeyboardDriverList(HINF InfFile);

PGENERIC_LIST
CreateKeyboardLayoutList(HINF InfFile, WCHAR *DefaultKBLayout);

PGENERIC_LIST 
CreateLanguageList(HINF InfFile, WCHAR * DefaultLanguage);

ULONG
GetDefaultLanguageIndex(VOID);

BOOLEAN
ProcessLocaleRegistry(PGENERIC_LIST List);

BOOLEAN
ProcessKeyboardLayoutRegistry(PGENERIC_LIST List);

BOOLEAN
ProcessKeyboardLayoutFiles(PGENERIC_LIST List);

BOOLEAN
SetGeoID(PWCHAR Id);

/* EOF */
