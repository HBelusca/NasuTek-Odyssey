/*
 *  Odyssey Task Manager
 *
 *  run.h
 *
 *  Copyright (C) 1999 - 2001  Brian Palmer  <brianp@odyssey.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * run.h - definitions necessary to use Microsoft's "Run" dialog
 * Undocumented Windows call
 * use the type below to declare a function pointer
 *
 * Information taken from http://www.geocities.com/SiliconValley/4942/
 * Copyright � 1998-1999 James Holderness. All Rights Reserved.
 * jholderness@geocities.com
 */

#pragma once

void TaskManager_OnFileNew(void);

typedef	void (WINAPI *RUNFILEDLG)(
HWND    hwndOwner,
HICON   hIcon,
LPCWSTR lpstrDirectory,
LPCWSTR lpstrTitle,
LPCWSTR  lpstrDescription,
UINT    uFlags);

/*
 * Flags for RunFileDlg
 */

#define	RFF_NOBROWSE		0x01	/* Removes the browse button. */
#define	RFF_NODEFAULT		0x02	/* No default item selected. */
#define	RFF_CALCDIRECTORY	0x04	/* Calculates the working directory from the file name. */
#define	RFF_NOLABEL			0x08	/* Removes the edit box label. */
#define	RFF_NOSEPARATEMEM	0x20	/* Removes the Separate Memory Space check box (Windows NT only). */
