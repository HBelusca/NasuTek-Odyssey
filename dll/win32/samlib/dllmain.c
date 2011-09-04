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
/* $Id: dllmain.c 53233 2011-08-14 16:59:43Z akhaldi $
 *
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey system libraries
 * PURPOSE:           SAM interface library
 * FILE:              lib/samlib/dllmain.c
 * PROGRAMER:         Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include "precomp.h"

//#define LOG_DEBUG_MESSAGES

/* GLOBALS *******************************************************************/


/* FUNCTIONS *****************************************************************/

BOOL WINAPI
DllMain (HINSTANCE hInstance,
	 DWORD dwReason,
	 LPVOID lpReserved)
{

  return TRUE;
}


void
DebugPrint (char* fmt,...)
{
#ifdef LOG_DEBUG_MESSAGES
  char FileName[MAX_PATH];
  HANDLE hLogFile;
  DWORD dwBytesWritten;
#endif
  char buffer[512];
  va_list ap;

  va_start (ap, fmt);
  vsprintf (buffer, fmt, ap);
  va_end (ap);

  OutputDebugStringA (buffer);

#ifdef LOG_DEBUG_MESSAGES
  strcpy (FileName, "C:\\odyssey\\samlib.log");
  hLogFile = CreateFileA (FileName,
			  GENERIC_WRITE,
			  0,
			  NULL,
			  OPEN_ALWAYS,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL);
  if (hLogFile == INVALID_HANDLE_VALUE)
    return;

  if (SetFilePointer(hLogFile, 0, NULL, FILE_END) == 0xFFFFFFFF)
    {
      CloseHandle (hLogFile);
      return;
    }

  WriteFile (hLogFile,
	     buffer,
	     strlen(buffer),
	     &dwBytesWritten,
	     NULL);

  CloseHandle (hLogFile);
#endif
}

/* EOF */
