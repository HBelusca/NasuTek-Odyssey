/*
 *  Odyssey W32 Subsystem
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* $Id: float.c 47036 2010-04-26 13:58:46Z gadamopoulos $
 *
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey kernel
 * PURPOSE:           Engine floating point functions
 * FILE:              subsys/win32k/eng/float.c
 * PROGRAMER:         Jason Filby
 * REVISION HISTORY:
 */

/* INCLUDES *****************************************************************/

#include <win32k.h>

#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

BOOL
APIENTRY
EngRestoreFloatingPointState ( IN VOID *Buffer )
{
  NTSTATUS Status;
  Status = KeRestoreFloatingPointState((PKFLOATING_SAVE)Buffer);
  if (!NT_SUCCESS(Status))
    {
      return FALSE;
    }
  return TRUE;
}

ULONG
APIENTRY
EngSaveFloatingPointState(OUT VOID  *Buffer,
     IN ULONG  BufferSize)
{
  KFLOATING_SAVE TempBuffer;
  NTSTATUS Status;
  if (Buffer == NULL || BufferSize == 0)
    {
      /* Check for floating point support. */
      Status = KeSaveFloatingPointState(&TempBuffer);
      if (Status != STATUS_SUCCESS)
 {
   return(0);
 }
      KeRestoreFloatingPointState(&TempBuffer);
      return(sizeof(KFLOATING_SAVE));
    }
  if (BufferSize < sizeof(KFLOATING_SAVE))
    {
      return(0);
    }
  Status = KeSaveFloatingPointState((PKFLOATING_SAVE)Buffer);
  if (!NT_SUCCESS(Status))
    {
      return FALSE;
    }
  return TRUE;
}
