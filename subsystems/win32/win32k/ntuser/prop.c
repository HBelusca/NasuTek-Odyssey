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
/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          Window properties
 * FILE:             subsys/win32k/ntuser/prop.c
 * PROGRAMER:        Casper S. Hornstrup (chorns@users.sourceforge.net)
 * REVISION HISTORY:
 *       06-06-2001  CSH  Created
 */
/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserProp);

/* STATIC FUNCTIONS **********************************************************/

PPROPERTY FASTCALL
IntGetProp(PWND Window, ATOM Atom)
{
   PLIST_ENTRY ListEntry;
   PPROPERTY Property;

   ListEntry = Window->PropListHead.Flink;
   while (ListEntry != &Window->PropListHead)
   {
      Property = CONTAINING_RECORD(ListEntry, PROPERTY, PropListEntry);
      if (Property->Atom == Atom)
      {
         return(Property);
      }
      ListEntry = ListEntry->Flink;
   }
   return(NULL);
}

BOOL FASTCALL
IntRemoveProp(PWND Window, ATOM Atom)
{
   PPROPERTY Prop;
   Prop = IntGetProp(Window, Atom);

   if (Prop == NULL)
   {
      return FALSE;
   }
   RemoveEntryList(&Prop->PropListEntry);
   UserHeapFree(Prop);
   Window->PropListItems--;
   return TRUE;
}

BOOL FASTCALL
IntSetProp(PWND pWnd, ATOM Atom, HANDLE Data)
{
   PPROPERTY Prop;

   Prop = IntGetProp(pWnd, Atom);

   if (Prop == NULL)
   {
      Prop = UserHeapAlloc(sizeof(PROPERTY));
      if (Prop == NULL)
      {
         return FALSE;
      }
      Prop->Atom = Atom;
      InsertTailList(&pWnd->PropListHead, &Prop->PropListEntry);
      pWnd->PropListItems++;
   }

   Prop->Data = Data;
   return TRUE;
}

/* FUNCTIONS *****************************************************************/

NTSTATUS APIENTRY
NtUserBuildPropList(HWND hWnd,
                    LPVOID Buffer,
                    DWORD BufferSize,
                    DWORD *Count)
{
   PWND Window;
   PPROPERTY Property;
   PLIST_ENTRY ListEntry;
   PROPLISTITEM listitem, *li;
   NTSTATUS Status;
   DWORD Cnt = 0;
   DECLARE_RETURN(NTSTATUS);

   TRACE("Enter NtUserBuildPropList\n");
   UserEnterShared();

   if (!(Window = UserGetWindowObject(hWnd)))
   {
      RETURN( STATUS_INVALID_HANDLE);
   }

   if(Buffer)
   {
      if(!BufferSize || (BufferSize % sizeof(PROPLISTITEM) != 0))
      {
         RETURN( STATUS_INVALID_PARAMETER);
      }

      /* copy list */
      li = (PROPLISTITEM *)Buffer;
      ListEntry = Window->PropListHead.Flink;
      while((BufferSize >= sizeof(PROPLISTITEM)) && (ListEntry != &Window->PropListHead))
      {
         Property = CONTAINING_RECORD(ListEntry, PROPERTY, PropListEntry);
         listitem.Atom = Property->Atom;
         listitem.Data = Property->Data;

         Status = MmCopyToCaller(li, &listitem, sizeof(PROPLISTITEM));
         if(!NT_SUCCESS(Status))
         {
            RETURN( Status);
         }

         BufferSize -= sizeof(PROPLISTITEM);
         Cnt++;
         li++;
         ListEntry = ListEntry->Flink;
      }

   }
   else
   {
      Cnt = Window->PropListItems * sizeof(PROPLISTITEM);
   }

   if(Count)
   {
      Status = MmCopyToCaller(Count, &Cnt, sizeof(DWORD));
      if(!NT_SUCCESS(Status))
      {
         RETURN( Status);
      }
   }

   RETURN( STATUS_SUCCESS);

CLEANUP:
   TRACE("Leave NtUserBuildPropList, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

HANDLE APIENTRY
NtUserRemoveProp(HWND hWnd, ATOM Atom)
{
   PWND Window;
   PPROPERTY Prop;
   HANDLE Data;
   DECLARE_RETURN(HANDLE);

   TRACE("Enter NtUserRemoveProp\n");
   UserEnterExclusive();

   if (!(Window = UserGetWindowObject(hWnd)))
   {
      RETURN( NULL);
   }

   Prop = IntGetProp(Window, Atom);

   if (Prop == NULL)
   {
      RETURN(NULL);
   }
   Data = Prop->Data;
   RemoveEntryList(&Prop->PropListEntry);
   UserHeapFree(Prop);
   Window->PropListItems--;

   RETURN(Data);

CLEANUP:
   TRACE("Leave NtUserRemoveProp, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

BOOL APIENTRY
NtUserSetProp(HWND hWnd, ATOM Atom, HANDLE Data)
{
   PWND Window;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserSetProp\n");
   UserEnterExclusive();

   if (!(Window = UserGetWindowObject(hWnd)))
   {
      RETURN( FALSE);
   }

   RETURN( IntSetProp(Window, Atom, Data));

CLEANUP:
   TRACE("Leave NtUserSetProp, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/* EOF */
