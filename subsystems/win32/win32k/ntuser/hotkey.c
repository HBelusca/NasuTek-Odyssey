/*
 *  Odyssey W32 Subsystem
 *  Copyright (C) 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
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
 * PURPOSE:          HotKey support
 * FILE:             subsys/win32k/ntuser/hotkey.c
 * PROGRAMER:        Eric Kohl
 * REVISION HISTORY:
 *       02-11-2003  EK  Created
 */



/*

FIXME: Hotkey notifications are triggered by keyboard input (physical or programatically)
and since only desktops on WinSta0 can recieve input in seems very wrong to allow
windows/threads on destops not belonging to WinSta0 to set hotkeys (recieve notifications).

-Gunnar
*/


/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserHotkey);

/* GLOBALS *******************************************************************/

LIST_ENTRY gHotkeyList;

/* FUNCTIONS *****************************************************************/

INIT_FUNCTION
NTSTATUS
NTAPI
InitHotkeyImpl(VOID)
{
   InitializeListHead(&gHotkeyList);

   return STATUS_SUCCESS;
}

#if 0 //not used
NTSTATUS FASTCALL
CleanupHotKeys(VOID)
{

   return STATUS_SUCCESS;
}
#endif

BOOL FASTCALL
GetHotKey (UINT fsModifiers,
           UINT vk,
           struct _ETHREAD **Thread,
           HWND *hWnd,
           int *id)
{
   PHOT_KEY_ITEM HotKeyItem;

   LIST_FOR_EACH(HotKeyItem, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if (HotKeyItem->fsModifiers == fsModifiers &&
            HotKeyItem->vk == vk)
      {
         if (Thread != NULL)
            *Thread = HotKeyItem->Thread;

         if (hWnd != NULL)
            *hWnd = HotKeyItem->hWnd;

         if (id != NULL)
            *id = HotKeyItem->id;

         return TRUE;
      }
   }

   return FALSE;
}

VOID FASTCALL
UnregisterWindowHotKeys(PWND Window)
{
   PHOT_KEY_ITEM HotKeyItem, tmp;

   LIST_FOR_EACH_SAFE(HotKeyItem, tmp, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if (HotKeyItem->hWnd == Window->head.h)
      {
         RemoveEntryList (&HotKeyItem->ListEntry);
         ExFreePool (HotKeyItem);
      }
   }

}

VOID FASTCALL
UnregisterThreadHotKeys(struct _ETHREAD *Thread)
{
   PHOT_KEY_ITEM HotKeyItem, tmp;

   LIST_FOR_EACH_SAFE(HotKeyItem, tmp, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if (HotKeyItem->Thread == Thread)
      {
         RemoveEntryList (&HotKeyItem->ListEntry);
         ExFreePool (HotKeyItem);
      }
   }

}

static
BOOL FASTCALL
IsHotKey (UINT fsModifiers, UINT vk)
{
   PHOT_KEY_ITEM HotKeyItem;

   LIST_FOR_EACH(HotKeyItem, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if (HotKeyItem->fsModifiers == fsModifiers && HotKeyItem->vk == vk)
      {
         return TRUE;
      }
   }

   return FALSE;
}

//
// Get/SetHotKey message support.
//
UINT FASTCALL
DefWndGetHotKey( HWND hwnd )
{
   PHOT_KEY_ITEM HotKeyItem;

   ERR("DefWndGetHotKey\n");

   if (IsListEmpty(&gHotkeyList)) return 0;

   LIST_FOR_EACH(HotKeyItem, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if ( HotKeyItem->hWnd == hwnd &&
           HotKeyItem->id == IDHOT_ODYSSEY )
      {
         return MAKELONG(HotKeyItem->vk, HotKeyItem->fsModifiers);
      }
   }
   return 0;
}

INT FASTCALL 
DefWndSetHotKey( PWND pWnd, WPARAM wParam )
{
   UINT fsModifiers, vk;
   PHOT_KEY_ITEM HotKeyItem;
   HWND hWnd;
   BOOL HaveSameWnd = FALSE;
   INT Ret = 1;

   ERR("DefWndSetHotKey wParam 0x%x\n", wParam);

   // A hot key cannot be associated with a child window.
   if (pWnd->style & WS_CHILD) return 0;

   // VK_ESCAPE, VK_SPACE, and VK_TAB are invalid hot keys.
   if ( LOWORD(wParam) == VK_ESCAPE ||
        LOWORD(wParam) == VK_SPACE ||
        LOWORD(wParam) == VK_TAB ) return -1;

   vk = LOWORD(wParam);
   fsModifiers = HIWORD(wParam);   
   hWnd = UserHMGetHandle(pWnd);

   if (wParam)
   {
      LIST_FOR_EACH(HotKeyItem, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
      {
         if ( HotKeyItem->fsModifiers == fsModifiers &&
              HotKeyItem->vk == vk &&
              HotKeyItem->id == IDHOT_ODYSSEY )
         {
            if (HotKeyItem->hWnd != hWnd)
               Ret = 2; // Another window already has the same hot key.
            break;
         }
      }
   }

   LIST_FOR_EACH(HotKeyItem, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if ( HotKeyItem->hWnd == hWnd &&
           HotKeyItem->id == IDHOT_ODYSSEY )
      {
         HaveSameWnd = TRUE;
         break;
      }
   }

   if (HaveSameWnd)
   {
      if (wParam == 0)
      { // Setting wParam to NULL removes the hot key associated with a window.
         UnregisterWindowHotKeys(pWnd);
      }
      else
      { /* A window can only have one hot key. If the window already has a hot key
           associated with it, the new hot key replaces the old one. */
         HotKeyItem->fsModifiers = fsModifiers;
         HotKeyItem->vk = vk;
      }
   }
   else // 
   {
      if (wParam == 0)
         return 1; // Do nothing, exit.

      HotKeyItem = ExAllocatePoolWithTag (PagedPool, sizeof(HOT_KEY_ITEM), USERTAG_HOTKEY);
      if (HotKeyItem == NULL)
      {
        return 0;
      }

      HotKeyItem->Thread = pWnd->head.pti->pEThread;
      HotKeyItem->hWnd = hWnd;
      HotKeyItem->id = IDHOT_ODYSSEY; // Don't care, these hot keys are unrelated to the hot keys set by RegisterHotKey.
      HotKeyItem->fsModifiers = fsModifiers;
      HotKeyItem->vk = vk;

      InsertHeadList (&gHotkeyList, &HotKeyItem->ListEntry);
   }
   return Ret;
}

/* SYSCALLS *****************************************************************/


BOOL APIENTRY
NtUserRegisterHotKey(HWND hWnd,
                     int id,
                     UINT fsModifiers,
                     UINT vk)
{
   PHOT_KEY_ITEM HotKeyItem;
   PWND Window;
   PETHREAD HotKeyThread;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserRegisterHotKey\n");
   UserEnterExclusive();

   if (hWnd == NULL)
   {
      HotKeyThread = PsGetCurrentThread();
   }
   else
   {
      if(!(Window = UserGetWindowObject(hWnd)))
      {
         RETURN( FALSE);
      }
      HotKeyThread = Window->head.pti->pEThread;
   }

   /* Check for existing hotkey */
   if (IsHotKey (fsModifiers, vk))
   {
      RETURN( FALSE);
   }

   HotKeyItem = ExAllocatePoolWithTag (PagedPool, sizeof(HOT_KEY_ITEM), USERTAG_HOTKEY);
   if (HotKeyItem == NULL)
   {
      RETURN( FALSE);
   }

   HotKeyItem->Thread = HotKeyThread;
   HotKeyItem->hWnd = hWnd;
   HotKeyItem->id = id;
   HotKeyItem->fsModifiers = fsModifiers;
   HotKeyItem->vk = vk;

   InsertHeadList (&gHotkeyList, &HotKeyItem->ListEntry);

   RETURN( TRUE);

CLEANUP:
   TRACE("Leave NtUserRegisterHotKey, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}


BOOL APIENTRY
NtUserUnregisterHotKey(HWND hWnd, int id)
{
   PHOT_KEY_ITEM HotKeyItem;
   PWND Window;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserUnregisterHotKey\n");
   UserEnterExclusive();

   if(!(Window = UserGetWindowObject(hWnd)))
   {
      RETURN( FALSE);
   }

   LIST_FOR_EACH(HotKeyItem, &gHotkeyList, HOT_KEY_ITEM, ListEntry)
   {
      if (HotKeyItem->hWnd == hWnd && HotKeyItem->id == id)
      {
         RemoveEntryList (&HotKeyItem->ListEntry);
         ExFreePoolWithTag(HotKeyItem, USERTAG_HOTKEY);

         RETURN( TRUE);
      }
   }

   RETURN( FALSE);

CLEANUP:
   TRACE("Leave NtUserUnregisterHotKey, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/* EOF */
