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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* $Id: drivesup.c 47686 2010-06-07 21:38:49Z spetreolle $
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey text-mode setup
 * FILE:            subsys/system/usetup/drivesup.c
 * PURPOSE:         Drive support functions
 * PROGRAMMER:      Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include "usetup.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

NTSTATUS
GetSourcePaths(PUNICODE_STRING SourcePath,
	       PUNICODE_STRING SourceRootPath,
	       PUNICODE_STRING SourceRootDir)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  UNICODE_STRING LinkName;
  UNICODE_STRING SourceName;
  WCHAR SourceBuffer[MAX_PATH] = {L'\0'};
  HANDLE Handle;
  NTSTATUS Status;
  ULONG Length;
  PWCHAR Ptr;

  RtlInitUnicodeString(&LinkName,
		       L"\\SystemRoot");

  InitializeObjectAttributes(&ObjectAttributes,
			     &LinkName,
			     OBJ_CASE_INSENSITIVE,
			     NULL,
			     NULL);

  Status = NtOpenSymbolicLinkObject(&Handle,
				    SYMBOLIC_LINK_ALL_ACCESS,
				    &ObjectAttributes);
  if (!NT_SUCCESS(Status))
    return(Status);

  SourceName.Length = 0;
  SourceName.MaximumLength = MAX_PATH * sizeof(WCHAR);
  SourceName.Buffer = SourceBuffer;

  Status = NtQuerySymbolicLinkObject(Handle,
				     &SourceName,
				     &Length);
  NtClose(Handle);

  if (NT_SUCCESS(Status))
    {
      RtlCreateUnicodeString(SourcePath,
                             SourceName.Buffer);

      /* strip trailing directory */
      Ptr = wcsrchr(SourceName.Buffer, L'\\');
      if (Ptr)
      {
          RtlCreateUnicodeString(SourceRootDir, Ptr);
          *Ptr = 0;
      }
      else
          RtlCreateUnicodeString(SourceRootDir, L"");

      RtlCreateUnicodeString(SourceRootPath,
                             SourceName.Buffer);
    }

  NtClose(Handle);

  return(STATUS_SUCCESS);
}


/* EOF */
