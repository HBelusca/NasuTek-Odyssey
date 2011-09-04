/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          Menus
 * FILE:             subsys/win32k/ntuser/menu.c
 * PROGRAMER:        Thomas Weidenmueller (w3seek@users.sourceforge.net)
 * REVISION HISTORY:
 *       07/30/2003  CSH  Created
 */
/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserMenu);

/* INTERNAL ******************************************************************/

/* maximum number of menu items a menu can contain */
#define MAX_MENU_ITEMS (0x4000)
#define MAX_GOINTOSUBMENU (0x10)

#define UpdateMenuItemState(state, change) \
{\
  if((change) & MFS_DISABLED) { \
    (state) |= MFS_DISABLED; \
  } else { \
    (state) &= ~MFS_DISABLED; \
  } \
  if((change) & MFS_CHECKED) { \
    (state) |= MFS_CHECKED; \
  } else { \
    (state) &= ~MFS_CHECKED; \
  } \
  if((change) & MFS_HILITE) { \
    (state) |= MFS_HILITE; \
  } else { \
    (state) &= ~MFS_HILITE; \
  } \
  if((change) & MFS_DEFAULT) { \
    (state) |= MFS_DEFAULT; \
  } else { \
    (state) &= ~MFS_DEFAULT; \
  } \
  if((change) & MF_MOUSESELECT) { \
    (state) |= MF_MOUSESELECT; \
  } else { \
    (state) &= ~MF_MOUSESELECT; \
  } \
}

#define FreeMenuText(MenuItem) \
{ \
  if((MENU_ITEM_TYPE((MenuItem)->fType) == MF_STRING) && \
           (MenuItem)->Text.Length) { \
    ExFreePoolWithTag((MenuItem)->Text.Buffer, TAG_STRING); \
  } \
}


PMENU_OBJECT FASTCALL UserGetMenuObject(HMENU hMenu)
{
   PMENU_OBJECT Menu;

   if (!hMenu)
   {
      EngSetLastError(ERROR_INVALID_MENU_HANDLE);
      return NULL;
   }

   Menu = (PMENU_OBJECT)UserGetObject(gHandleTable, hMenu, otMenu);
   if (!Menu)
   {
      EngSetLastError(ERROR_INVALID_MENU_HANDLE);
      return NULL;
   }

   return Menu;
}


#if 0
void FASTCALL
DumpMenuItemList(PMENU_ITEM MenuItem)
{
   UINT cnt = 0;
   while(MenuItem)
   {
      if(MenuItem->Text.Length)
         DbgPrint(" %d. %wZ\n", ++cnt, &MenuItem->Text);
      else
         DbgPrint(" %d. NO TEXT dwTypeData==%d\n", ++cnt, (DWORD)MenuItem->Text.Buffer);
      DbgPrint("   fType=");
      if(MFT_BITMAP & MenuItem->fType)
         DbgPrint("MFT_BITMAP ");
      if(MFT_MENUBARBREAK & MenuItem->fType)
         DbgPrint("MFT_MENUBARBREAK ");
      if(MFT_MENUBREAK & MenuItem->fType)
         DbgPrint("MFT_MENUBREAK ");
      if(MFT_OWNERDRAW & MenuItem->fType)
         DbgPrint("MFT_OWNERDRAW ");
      if(MFT_RADIOCHECK & MenuItem->fType)
         DbgPrint("MFT_RADIOCHECK ");
      if(MFT_RIGHTJUSTIFY & MenuItem->fType)
         DbgPrint("MFT_RIGHTJUSTIFY ");
      if(MFT_SEPARATOR & MenuItem->fType)
         DbgPrint("MFT_SEPARATOR ");
      if(MFT_STRING & MenuItem->fType)
         DbgPrint("MFT_STRING ");
      DbgPrint("\n   fState=");
      if(MFS_DISABLED & MenuItem->fState)
         DbgPrint("MFS_DISABLED ");
      else
         DbgPrint("MFS_ENABLED ");
      if(MFS_CHECKED & MenuItem->fState)
         DbgPrint("MFS_CHECKED ");
      else
         DbgPrint("MFS_UNCHECKED ");
      if(MFS_HILITE & MenuItem->fState)
         DbgPrint("MFS_HILITE ");
      else
         DbgPrint("MFS_UNHILITE ");
      if(MFS_DEFAULT & MenuItem->fState)
         DbgPrint("MFS_DEFAULT ");
      if(MFS_GRAYED & MenuItem->fState)
         DbgPrint("MFS_GRAYED ");
      DbgPrint("\n   wId=%d\n", MenuItem->wID);
      MenuItem = MenuItem->Next;
   }
   DbgPrint("Entries: %d\n", cnt);
   return;
}
#endif

PMENU_OBJECT FASTCALL
IntGetMenuObject(HMENU hMenu)
{
   PMENU_OBJECT Menu = UserGetMenuObject(hMenu);
   if (Menu)
      Menu->head.cLockObj++;

   return Menu;
}

BOOL FASTCALL
IntFreeMenuItem(PMENU_OBJECT Menu, PMENU_ITEM MenuItem, BOOL bRecurse)
{
   FreeMenuText(MenuItem);
   if(bRecurse && MenuItem->hSubMenu)
   {
      PMENU_OBJECT SubMenu;
      SubMenu = UserGetMenuObject(MenuItem->hSubMenu );
      if(SubMenu)
      {
         IntDestroyMenuObject(SubMenu, bRecurse, TRUE);
      }
   }

   /* Free memory */
   ExFreePool(MenuItem);

   return TRUE;
}

BOOL FASTCALL
IntRemoveMenuItem(PMENU_OBJECT Menu, UINT uPosition, UINT uFlags,
                  BOOL bRecurse)
{
   PMENU_ITEM PrevMenuItem, MenuItem;
   if(IntGetMenuItemByFlag(Menu, uPosition, uFlags, &Menu, &MenuItem,
                           &PrevMenuItem) > -1)
   {
      if(MenuItem)
      {
         if(PrevMenuItem)
            PrevMenuItem->Next = MenuItem->Next;
         else
         {
            Menu->MenuItemList = MenuItem->Next;
         }
         Menu->MenuInfo.MenuItemCount--;
         return IntFreeMenuItem(Menu, MenuItem, bRecurse);
      }
   }
   return FALSE;
}

UINT FASTCALL
IntDeleteMenuItems(PMENU_OBJECT Menu, BOOL bRecurse)
{
   UINT res = 0;
   PMENU_ITEM NextItem;
   PMENU_ITEM CurItem = Menu->MenuItemList;
   while(CurItem)
   {
      NextItem = CurItem->Next;
      IntFreeMenuItem(Menu, CurItem, bRecurse);
      CurItem = NextItem;
      res++;
   }
   Menu->MenuInfo.MenuItemCount = 0;
   Menu->MenuItemList = NULL;
   return res;
}

BOOL FASTCALL
IntDestroyMenuObject(PMENU_OBJECT Menu,
                     BOOL bRecurse, BOOL RemoveFromProcess)
{
   if(Menu)
   {
      PWND Window;
      PWINSTATION_OBJECT WindowStation;
      NTSTATUS Status;

      /* remove all menu items */
      IntDeleteMenuItems(Menu, bRecurse); /* do not destroy submenus */

      if(RemoveFromProcess)
      {
         RemoveEntryList(&Menu->ListEntry);
      }

      Status = ObReferenceObjectByHandle(Menu->Process->Win32WindowStation,
                                         0,
                                         ExWindowStationObjectType,
                                         KernelMode,
                                         (PVOID*)&WindowStation,
                                         NULL);
      if(NT_SUCCESS(Status))
      {
         BOOL ret;
         if (Menu->MenuInfo.Wnd)
         {
            Window = UserGetWindowObject(Menu->MenuInfo.Wnd);
            if (Window)
            {
               Window->IDMenu = 0;
            }
         }
//         UserDereferenceObject(Menu);
         ret = UserDeleteObject(Menu->MenuInfo.Self, otMenu);
         ObDereferenceObject(WindowStation);
         return ret;
      }
   }
   return FALSE;
}

PMENU_OBJECT FASTCALL
IntCreateMenu(PHANDLE Handle, BOOL IsMenuBar)
{
   PMENU_OBJECT Menu;
   PPROCESSINFO CurrentWin32Process;

   Menu = (PMENU_OBJECT)UserCreateObject( gHandleTable,
                                          NULL,
                                          Handle,
                                          otMenu,
                                          sizeof(MENU_OBJECT));
   if(!Menu)
   {
      *Handle = 0;
      return NULL;
   }

   Menu->Process = PsGetCurrentProcess();
   Menu->RtoL = FALSE; /* default */
   Menu->MenuInfo.cbSize = sizeof(MENUINFO); /* not used */
   Menu->MenuInfo.fMask = 0; /* not used */
   Menu->MenuInfo.dwStyle = 0; /* FIXME */
   Menu->MenuInfo.cyMax = 0; /* default */
   Menu->MenuInfo.hbrBack = NULL; /* no brush */
   Menu->MenuInfo.dwContextHelpID = 0; /* default */
   Menu->MenuInfo.dwMenuData = 0; /* default */
   Menu->MenuInfo.Self = *Handle;
   Menu->MenuInfo.FocusedItem = NO_SELECTED_ITEM;
   Menu->MenuInfo.Flags = (IsMenuBar ? 0 : MF_POPUP);
   Menu->MenuInfo.Wnd = NULL;
   Menu->MenuInfo.WndOwner = NULL;
   Menu->MenuInfo.Height = 0;
   Menu->MenuInfo.Width = 0;
   Menu->MenuInfo.TimeToHide = FALSE;

   Menu->MenuInfo.MenuItemCount = 0;
   Menu->MenuItemList = NULL;

   /* Insert menu item into process menu handle list */
   CurrentWin32Process = PsGetCurrentProcessWin32Process();
   InsertTailList(&CurrentWin32Process->MenuListHead, &Menu->ListEntry);

   return Menu;
}

BOOL FASTCALL
IntCloneMenuItems(PMENU_OBJECT Destination, PMENU_OBJECT Source)
{
   PMENU_ITEM MenuItem, NewMenuItem = NULL;
   PMENU_ITEM Old = NULL;

   if(!Source->MenuInfo.MenuItemCount)
      return FALSE;

   MenuItem = Source->MenuItemList;
   while(MenuItem)
   {
      Old = NewMenuItem;
      if(NewMenuItem)
         NewMenuItem->Next = MenuItem;
      NewMenuItem = ExAllocatePoolWithTag(PagedPool, sizeof(MENU_ITEM), TAG_MENUITEM);
      if(!NewMenuItem)
         break;
      NewMenuItem->fType = MenuItem->fType;
      NewMenuItem->fState = MenuItem->fState;
      NewMenuItem->wID = MenuItem->wID;
      NewMenuItem->hSubMenu = MenuItem->hSubMenu;
      NewMenuItem->hbmpChecked = MenuItem->hbmpChecked;
      NewMenuItem->hbmpUnchecked = MenuItem->hbmpUnchecked;
      NewMenuItem->dwItemData = MenuItem->dwItemData;
      if((MENU_ITEM_TYPE(NewMenuItem->fType) == MF_STRING))
      {
         if(MenuItem->Text.Length)
         {
            NewMenuItem->Text.Length = 0;
            NewMenuItem->Text.MaximumLength = MenuItem->Text.MaximumLength;
            NewMenuItem->Text.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool, MenuItem->Text.MaximumLength, TAG_STRING);
            if(!NewMenuItem->Text.Buffer)
            {
               ExFreePoolWithTag(NewMenuItem, TAG_MENUITEM);
               break;
            }
            RtlCopyUnicodeString(&NewMenuItem->Text, &MenuItem->Text);
         }
         else
         {
            NewMenuItem->Text.Buffer = MenuItem->Text.Buffer;
         }
      }
      else
      {
         NewMenuItem->Text.Buffer = MenuItem->Text.Buffer;
      }
      NewMenuItem->hbmpItem = MenuItem->hbmpItem;

      NewMenuItem->Next = NULL;
      if(Old)
         Old->Next = NewMenuItem;
      else
         Destination->MenuItemList = NewMenuItem;
      Destination->MenuInfo.MenuItemCount++;
      MenuItem = MenuItem->Next;
   }

   return TRUE;
}

PMENU_OBJECT FASTCALL
IntCloneMenu(PMENU_OBJECT Source)
{
   PPROCESSINFO CurrentWin32Process;
   HANDLE hMenu;
   PMENU_OBJECT Menu;

   if(!Source)
      return NULL;

   Menu = (PMENU_OBJECT)UserCreateObject( gHandleTable,
                                          NULL,
                                         &hMenu,
                                          otMenu,
                                          sizeof(MENU_OBJECT));
   if(!Menu)
      return NULL;

   Menu->Process = PsGetCurrentProcess();
   Menu->RtoL = Source->RtoL;
   Menu->MenuInfo.cbSize = sizeof(MENUINFO); /* not used */
   Menu->MenuInfo.fMask = Source->MenuInfo.fMask;
   Menu->MenuInfo.dwStyle = Source->MenuInfo.dwStyle;
   Menu->MenuInfo.cyMax = Source->MenuInfo.cyMax;
   Menu->MenuInfo.hbrBack = Source->MenuInfo.hbrBack;
   Menu->MenuInfo.dwContextHelpID = Source->MenuInfo.dwContextHelpID;
   Menu->MenuInfo.dwMenuData = Source->MenuInfo.dwMenuData;
   Menu->MenuInfo.Self = hMenu;
   Menu->MenuInfo.FocusedItem = NO_SELECTED_ITEM;
   Menu->MenuInfo.Wnd = NULL;
   Menu->MenuInfo.WndOwner = NULL;
   Menu->MenuInfo.Height = 0;
   Menu->MenuInfo.Width = 0;
   Menu->MenuInfo.TimeToHide = FALSE;

   Menu->MenuInfo.MenuItemCount = 0;
   Menu->MenuItemList = NULL;

   /* Insert menu item into process menu handle list */
   CurrentWin32Process = PsGetCurrentProcessWin32Process();
   InsertTailList(&CurrentWin32Process->MenuListHead, &Menu->ListEntry);

   IntCloneMenuItems(Menu, Source);

   return Menu;
}

BOOL FASTCALL
IntSetMenuFlagRtoL(PMENU_OBJECT Menu)
{
   Menu->RtoL = TRUE;
   return TRUE;
}

BOOL FASTCALL
IntSetMenuContextHelpId(PMENU_OBJECT Menu, DWORD dwContextHelpId)
{
   Menu->MenuInfo.dwContextHelpID = dwContextHelpId;
   return TRUE;
}

BOOL FASTCALL
IntGetMenuInfo(PMENU_OBJECT Menu, PROSMENUINFO lpmi)
{
   if(lpmi->fMask & MIM_BACKGROUND)
      lpmi->hbrBack = Menu->MenuInfo.hbrBack;
   if(lpmi->fMask & MIM_HELPID)
      lpmi->dwContextHelpID = Menu->MenuInfo.dwContextHelpID;
   if(lpmi->fMask & MIM_MAXHEIGHT)
      lpmi->cyMax = Menu->MenuInfo.cyMax;
   if(lpmi->fMask & MIM_MENUDATA)
      lpmi->dwMenuData = Menu->MenuInfo.dwMenuData;
   if(lpmi->fMask & MIM_STYLE)
      lpmi->dwStyle = Menu->MenuInfo.dwStyle;
   if (sizeof(MENUINFO) < lpmi->cbSize)
   {
      RtlCopyMemory((char *) lpmi + sizeof(MENUINFO),
                    (char *) &Menu->MenuInfo + sizeof(MENUINFO),
                    lpmi->cbSize - sizeof(MENUINFO));
   }
   if (sizeof(ROSMENUINFO) == lpmi->cbSize)
   {
     lpmi->maxBmpSize.cx = Menu->MenuInfo.maxBmpSize.cx;
     lpmi->maxBmpSize.cy = Menu->MenuInfo.maxBmpSize.cy;
   }
   return TRUE;
}

BOOL FASTCALL
IntSetMenuInfo(PMENU_OBJECT Menu, PROSMENUINFO lpmi)
{
   if(lpmi->fMask & MIM_BACKGROUND)
      Menu->MenuInfo.hbrBack = lpmi->hbrBack;
   if(lpmi->fMask & MIM_HELPID)
      Menu->MenuInfo.dwContextHelpID = lpmi->dwContextHelpID;
   if(lpmi->fMask & MIM_MAXHEIGHT)
      Menu->MenuInfo.cyMax = lpmi->cyMax;
   if(lpmi->fMask & MIM_MENUDATA)
      Menu->MenuInfo.dwMenuData = lpmi->dwMenuData;
   if(lpmi->fMask & MIM_STYLE)
      Menu->MenuInfo.dwStyle = lpmi->dwStyle;
   if(lpmi->fMask & MIM_APPLYTOSUBMENUS)
   {
      /* FIXME */
   }
   if (sizeof(MENUINFO) < lpmi->cbSize)
   {
      Menu->MenuInfo.FocusedItem = lpmi->FocusedItem;
      Menu->MenuInfo.Height = lpmi->Height;
      Menu->MenuInfo.Width = lpmi->Width;
      Menu->MenuInfo.Wnd = lpmi->Wnd;
      Menu->MenuInfo.WndOwner = lpmi->WndOwner;
      Menu->MenuInfo.TimeToHide = lpmi->TimeToHide;
   }
   if (sizeof(ROSMENUINFO) == lpmi->cbSize)
   {
     Menu->MenuInfo.maxBmpSize.cx = lpmi->maxBmpSize.cx;
     Menu->MenuInfo.maxBmpSize.cy = lpmi->maxBmpSize.cy;
   }
   return TRUE;
}


int FASTCALL
IntGetMenuItemByFlag(PMENU_OBJECT Menu, UINT uSearchBy, UINT fFlag,
                     PMENU_OBJECT *SubMenu, PMENU_ITEM *MenuItem,
                     PMENU_ITEM *PrevMenuItem)
{
   PMENU_ITEM PrevItem = NULL;
   PMENU_ITEM CurItem = Menu->MenuItemList;
   int p;
   int ret;

   if(MF_BYPOSITION & fFlag)
   {
      p = uSearchBy;
      while(CurItem && (p > 0))
      {
         PrevItem = CurItem;
         CurItem = CurItem->Next;
         p--;
      }
      if(CurItem)
      {
         if(MenuItem)
            *MenuItem = CurItem;
         if(PrevMenuItem)
            *PrevMenuItem = PrevItem;
      }
      else
      {
         if(MenuItem)
            *MenuItem = NULL;
         if(PrevMenuItem)
            *PrevMenuItem = NULL; /* ? */
         return -1;
      }

      return uSearchBy - p;
   }
   else
   {
      p = 0;
      while(CurItem)
      {
         if(CurItem->wID == uSearchBy)
         {
            if(MenuItem)
               *MenuItem = CurItem;
            if(PrevMenuItem)
               *PrevMenuItem = PrevItem;
            if(SubMenu)
                *SubMenu = Menu;

            return p;
         }
         else
         {
            if(CurItem->fType & MF_POPUP)
            {
               PMENU_OBJECT NewMenu = UserGetMenuObject(CurItem->hSubMenu);
               if(NewMenu)
               {
                   ret = IntGetMenuItemByFlag(NewMenu, uSearchBy, fFlag,
                                              SubMenu, MenuItem, PrevMenuItem);
                   if(ret != -1)
                   {
                      return ret;
                   }
               }
            }
         }
         PrevItem = CurItem;
         CurItem = CurItem->Next;
         p++;
      }
   }
   return -1;
}


int FASTCALL
IntInsertMenuItemToList(PMENU_OBJECT Menu, PMENU_ITEM MenuItem, int pos)
{
   PMENU_ITEM CurItem;
   PMENU_ITEM LastItem = NULL;
   UINT npos = 0;

   CurItem = Menu->MenuItemList;
   while(CurItem && (pos != 0))
   {
      LastItem = CurItem;
      CurItem = CurItem->Next;
      pos--;
      npos++;
   }

   if(LastItem)
   {
      /* insert the item after LastItem */
      LastItem->Next = MenuItem;
   }
   else
   {
      /* insert at the beginning */
      Menu->MenuItemList = MenuItem;
   }
   MenuItem->Next = CurItem;
   Menu->MenuInfo.MenuItemCount++;

   return npos;
}

BOOL FASTCALL
IntGetMenuItemInfo(PMENU_OBJECT Menu, /* UNUSED PARAM!! */
                   PMENU_ITEM MenuItem, PROSMENUITEMINFO lpmii)
{
   NTSTATUS Status;

   if(lpmii->fMask & (MIIM_FTYPE | MIIM_TYPE))
   {
      lpmii->fType = MenuItem->fType;
   }
   if(lpmii->fMask & MIIM_BITMAP)
   {
      lpmii->hbmpItem = MenuItem->hbmpItem;
   }
   if(lpmii->fMask & MIIM_CHECKMARKS)
   {
      lpmii->hbmpChecked = MenuItem->hbmpChecked;
      lpmii->hbmpUnchecked = MenuItem->hbmpUnchecked;
   }
   if(lpmii->fMask & MIIM_DATA)
   {
      lpmii->dwItemData = MenuItem->dwItemData;
   }
   if(lpmii->fMask & MIIM_ID)
   {
      lpmii->wID = MenuItem->wID;
   }
   if(lpmii->fMask & MIIM_STATE)
   {
      lpmii->fState = MenuItem->fState;
   }
   if(lpmii->fMask & MIIM_SUBMENU)
   {
      lpmii->hSubMenu = MenuItem->hSubMenu;
   }

   if ((lpmii->fMask & MIIM_STRING) ||
      ((lpmii->fMask & MIIM_TYPE) && (MENU_ITEM_TYPE(lpmii->fType) == MF_STRING)))
   {
      if (lpmii->dwTypeData == NULL)
      {
         lpmii->cch = MenuItem->Text.Length / sizeof(WCHAR);
      }
      else
      {
         Status = MmCopyToCaller(lpmii->dwTypeData, MenuItem->Text.Buffer,
                                 min(lpmii->cch * sizeof(WCHAR),
                                     MenuItem->Text.MaximumLength));
         if (! NT_SUCCESS(Status))
         {
            SetLastNtError(Status);
            return FALSE;
         }
      }
   }

   if (sizeof(ROSMENUITEMINFO) == lpmii->cbSize)
   {
      lpmii->Rect = MenuItem->Rect;
      lpmii->dxTab = MenuItem->dxTab;
      lpmii->lpstr = MenuItem->Text.Buffer; // Use DesktopHeap!
   }

   return TRUE;
}

BOOL FASTCALL
IntSetMenuItemInfo(PMENU_OBJECT MenuObject, PMENU_ITEM MenuItem, PROSMENUITEMINFO lpmii)
{
   PMENU_OBJECT SubMenuObject;
   UINT fTypeMask = (MFT_BITMAP | MFT_MENUBARBREAK | MFT_MENUBREAK | MFT_OWNERDRAW | MFT_RADIOCHECK | MFT_RIGHTJUSTIFY | MFT_SEPARATOR | MF_POPUP);

   if(!MenuItem || !MenuObject || !lpmii)
   {
      return FALSE;
   }
   if (lpmii->fType & ~fTypeMask)
   {
     ERR("IntSetMenuItemInfo invalid fType flags %x\n", lpmii->fType & ~fTypeMask);
     lpmii->fMask &= ~(MIIM_TYPE | MIIM_FTYPE);
   }
   if (lpmii->fMask &  MIIM_TYPE)
   {
      if (lpmii->fMask & ( MIIM_STRING | MIIM_FTYPE | MIIM_BITMAP))
      {
         ERR("IntSetMenuItemInfo: Invalid combination of fMask bits used\n");
         /* this does not happen on Win9x/ME */
         SetLastNtError( ERROR_INVALID_PARAMETER);
         return FALSE;
      }
      /*
       * Delete the menu item type when changing type from
       * MF_STRING.
       */
      if (MenuItem->fType != lpmii->fType &&
            MENU_ITEM_TYPE(MenuItem->fType) == MFT_STRING)
      {
         FreeMenuText(MenuItem);
         RtlInitUnicodeString(&MenuItem->Text, NULL);
      }
      if(lpmii->fType & MFT_BITMAP)
      {
         if(lpmii->hbmpItem)
           MenuItem->hbmpItem = lpmii->hbmpItem;
         else
         { /* Win 9x/Me stuff */
           MenuItem->hbmpItem = (HBITMAP)((ULONG_PTR)(LOWORD(lpmii->dwTypeData)));
         }
      }
      MenuItem->fType |= lpmii->fType;
   }
   if (lpmii->fMask & MIIM_FTYPE )
   {
      if(( lpmii->fType & MFT_BITMAP))
      {
         ERR("IntSetMenuItemInfo: Can not use FTYPE and MFT_BITMAP.\n");
         SetLastNtError( ERROR_INVALID_PARAMETER);
         return FALSE;
      }
      MenuItem->fType |= lpmii->fType; /* Need to save all the flags, this fixed MFT_RIGHTJUSTIFY */
   }
   if(lpmii->fMask & MIIM_BITMAP)
   {
         MenuItem->hbmpItem = lpmii->hbmpItem;
   }
   if(lpmii->fMask & MIIM_CHECKMARKS)
   {
      MenuItem->hbmpChecked = lpmii->hbmpChecked;
      MenuItem->hbmpUnchecked = lpmii->hbmpUnchecked;
   }
   if(lpmii->fMask & MIIM_DATA)
   {
      MenuItem->dwItemData = lpmii->dwItemData;
   }
   if(lpmii->fMask & MIIM_ID)
   {
      MenuItem->wID = lpmii->wID;
   }
   if(lpmii->fMask & MIIM_STATE)
   {
      /* remove MFS_DEFAULT flag from all other menu items if this item
         has the MFS_DEFAULT state */
      if(lpmii->fState & MFS_DEFAULT)
         UserSetMenuDefaultItem(MenuObject, -1, 0);
      /* update the menu item state flags */
      UpdateMenuItemState(MenuItem->fState, lpmii->fState);
   }

   if(lpmii->fMask & MIIM_SUBMENU)
   {
      MenuItem->hSubMenu = lpmii->hSubMenu;
      /* Make sure the submenu is marked as a popup menu */
      if (MenuItem->hSubMenu)
      {
         SubMenuObject = UserGetMenuObject(MenuItem->hSubMenu);
         if (SubMenuObject != NULL)
         {
            SubMenuObject->MenuInfo.Flags |= MF_POPUP;
            MenuItem->fType |= MF_POPUP;
         }
         else
         {
            MenuItem->fType &= ~MF_POPUP;
         }
      }
      else
      {
         MenuItem->fType &= ~MF_POPUP;
      }
   }

   if ((lpmii->fMask & MIIM_STRING) ||
      ((lpmii->fMask & MIIM_TYPE) && (MENU_ITEM_TYPE(lpmii->fType) == MF_STRING)))
   {
      FreeMenuText(MenuItem);

      if(lpmii->dwTypeData && lpmii->cch)
      {
         UNICODE_STRING Source;

         Source.Length =
            Source.MaximumLength = lpmii->cch * sizeof(WCHAR);
         Source.Buffer = lpmii->dwTypeData;

         MenuItem->Text.Buffer = (PWSTR)ExAllocatePoolWithTag(
                                    PagedPool, Source.Length + sizeof(WCHAR), TAG_STRING);
         if(MenuItem->Text.Buffer != NULL)
         {
            MenuItem->Text.Length = 0;
            MenuItem->Text.MaximumLength = Source.Length + sizeof(WCHAR);
            RtlCopyUnicodeString(&MenuItem->Text, &Source);
            MenuItem->Text.Buffer[MenuItem->Text.Length / sizeof(WCHAR)] = 0;
         }
         else
         {
            RtlInitUnicodeString(&MenuItem->Text, NULL);
         }
      }
      else
      {
         if (0 == (MenuObject->MenuInfo.Flags & MF_SYSMENU))
         {
            MenuItem->fType |= MF_SEPARATOR;
         }
         RtlInitUnicodeString(&MenuItem->Text, NULL);
      }
   }

   if (sizeof(ROSMENUITEMINFO) == lpmii->cbSize)
   {
      MenuItem->Rect = lpmii->Rect;
      MenuItem->dxTab = lpmii->dxTab;
      lpmii->lpstr = MenuItem->Text.Buffer; /* Use DesktopHeap! Send back new allocated string or zero */
   }

   return TRUE;
}

BOOL FASTCALL
IntInsertMenuItem(PMENU_OBJECT MenuObject, UINT uItem, BOOL fByPosition,
                  PROSMENUITEMINFO ItemInfo)
{
   int pos = (int)uItem;
   PMENU_ITEM MenuItem;
   PMENU_OBJECT SubMenu = NULL;

   if (MAX_MENU_ITEMS <= MenuObject->MenuInfo.MenuItemCount)
   {
      EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return FALSE;
   }

   if (fByPosition)
   {
      SubMenu = MenuObject;
      /* calculate position */
      if(MenuObject->MenuInfo.MenuItemCount < pos)
      {
         pos = MenuObject->MenuInfo.MenuItemCount;
      }
   }
   else
   {
      pos = IntGetMenuItemByFlag(MenuObject, uItem, MF_BYCOMMAND, &SubMenu, NULL, NULL);
   }
   if (SubMenu == NULL)
   {
       /* default to last position of menu */
      SubMenu = MenuObject;
      pos = MenuObject->MenuInfo.MenuItemCount;
   }


   if (pos < -1)
   {
      pos = -1;
   }

   MenuItem = ExAllocatePoolWithTag(PagedPool, sizeof(MENU_ITEM), TAG_MENUITEM);
   if (NULL == MenuItem)
   {
      EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return FALSE;
   }

   MenuItem->fType = MFT_STRING;
   MenuItem->fState = MFS_ENABLED | MFS_UNCHECKED;
   MenuItem->wID = 0;
   MenuItem->hSubMenu = (HMENU)0;
   MenuItem->hbmpChecked = (HBITMAP)0;
   MenuItem->hbmpUnchecked = (HBITMAP)0;
   MenuItem->dwItemData = 0;
   RtlInitUnicodeString(&MenuItem->Text, NULL);
   MenuItem->hbmpItem = (HBITMAP)0;

   if(!IntSetMenuItemInfo(SubMenu, MenuItem, ItemInfo))
   {
      ExFreePoolWithTag(MenuItem, TAG_MENUITEM);
      return FALSE;
   }

   /* Force size recalculation! */
   MenuObject->MenuInfo.Height = 0;

   pos = IntInsertMenuItemToList(SubMenu, MenuItem, pos);

   TRACE("IntInsertMenuItemToList = %i\n", pos);

   return (pos >= 0);
}

UINT FASTCALL
IntEnableMenuItem(PMENU_OBJECT MenuObject, UINT uIDEnableItem, UINT uEnable)
{
   PMENU_ITEM MenuItem;
   UINT res = IntGetMenuItemByFlag(MenuObject, uIDEnableItem, uEnable, NULL, &MenuItem, NULL);
   if(!MenuItem || (res == (UINT)-1))
   {
      return (UINT)-1;
   }

   res = MenuItem->fState & (MF_GRAYED | MF_DISABLED);

   if(uEnable & MF_DISABLED)
   {
      MenuItem->fState |= MF_DISABLED;
      MenuItem->fState |= uEnable & MF_GRAYED;
   }
   else
   {
      if(uEnable & MF_GRAYED)
      {
         MenuItem->fState |= (MF_GRAYED | MF_DISABLED);
      }
      else
      {
         MenuItem->fState &= ~(MF_DISABLED | MF_GRAYED);
      }
   }

   return res;
}


DWORD FASTCALL
IntBuildMenuItemList(PMENU_OBJECT MenuObject, PVOID Buffer, ULONG nMax)
{
   DWORD res = 0;
   ROSMENUITEMINFO mii;
   PVOID Buf;
   PMENU_ITEM CurItem = MenuObject->MenuItemList;
   PWCHAR StrOut;
   NTSTATUS Status;
   WCHAR NulByte;

   if (0 != nMax)
   {
      if (nMax < MenuObject->MenuInfo.MenuItemCount * sizeof(ROSMENUITEMINFO))
      {
         return 0;
      }
      StrOut = (PWCHAR)((char *) Buffer + MenuObject->MenuInfo.MenuItemCount
                        * sizeof(ROSMENUITEMINFO));
      nMax -= MenuObject->MenuInfo.MenuItemCount * sizeof(ROSMENUITEMINFO);
      Buf = Buffer;
      mii.cbSize = sizeof(ROSMENUITEMINFO);
      mii.fMask = 0;
      NulByte = L'\0';

      while (NULL != CurItem)
      {
         mii.cch = CurItem->Text.Length / sizeof(WCHAR);
         mii.dwItemData = CurItem->dwItemData;
         if (0 != CurItem->Text.Length)
         {
            mii.dwTypeData = StrOut;
         }
         else
         {
            mii.dwTypeData = NULL;
         }
         mii.fState = CurItem->fState;
         mii.fType = CurItem->fType;
         mii.wID = CurItem->wID;
         mii.hbmpChecked = CurItem->hbmpChecked;
         mii.hbmpItem = CurItem->hbmpItem;
         mii.hbmpUnchecked = CurItem->hbmpUnchecked;
         mii.hSubMenu = CurItem->hSubMenu;
         mii.Rect = CurItem->Rect;
         mii.dxTab = CurItem->dxTab;
         mii.lpstr = CurItem->Text.Buffer; // Use DesktopHeap!

         Status = MmCopyToCaller(Buf, &mii, sizeof(ROSMENUITEMINFO));
         if (! NT_SUCCESS(Status))
         {
            SetLastNtError(Status);
            return 0;
         }
         Buf = (PVOID)((ULONG_PTR)Buf + sizeof(ROSMENUITEMINFO));

         if (0 != CurItem->Text.Length
               && (nMax >= CurItem->Text.Length + sizeof(WCHAR)))
         {
            /* copy string */
            Status = MmCopyToCaller(StrOut, CurItem->Text.Buffer,
                                    CurItem->Text.Length);
            if (! NT_SUCCESS(Status))
            {
               SetLastNtError(Status);
               return 0;
            }
            StrOut += CurItem->Text.Length / sizeof(WCHAR);
            Status = MmCopyToCaller(StrOut, &NulByte, sizeof(WCHAR));
            if (! NT_SUCCESS(Status))
            {
               SetLastNtError(Status);
               return 0;
            }
            StrOut++;
            nMax -= CurItem->Text.Length + sizeof(WCHAR);
         }
         else if (0 != CurItem->Text.Length)
         {
            break;
         }

         CurItem = CurItem->Next;
         res++;
      }
   }
   else
   {
      while (NULL != CurItem)
      {
         res += sizeof(ROSMENUITEMINFO) + CurItem->Text.Length + sizeof(WCHAR);
         CurItem = CurItem->Next;
      }
   }

   return res;
}


DWORD FASTCALL
IntCheckMenuItem(PMENU_OBJECT MenuObject, UINT uIDCheckItem, UINT uCheck)
{
   PMENU_ITEM MenuItem;
   int res = -1;

   if((IntGetMenuItemByFlag(MenuObject, uIDCheckItem, uCheck, NULL, &MenuItem, NULL) < 0) || !MenuItem)
   {
      return -1;
   }

   res = (DWORD)(MenuItem->fState & MF_CHECKED);
   if(uCheck & MF_CHECKED)
   {
      MenuItem->fState |= MF_CHECKED;
   }
   else
   {
      MenuItem->fState &= ~MF_CHECKED;
   }

   return (DWORD)res;
}

BOOL FASTCALL
IntHiliteMenuItem(PWND WindowObject, PMENU_OBJECT MenuObject,
                  UINT uItemHilite, UINT uHilite)
{
   PMENU_ITEM MenuItem;
   BOOL res = IntGetMenuItemByFlag(MenuObject, uItemHilite, uHilite, NULL, &MenuItem, NULL);
   if(!MenuItem || !res)
   {
      return FALSE;
   }

   if(uHilite & MF_HILITE)
   {
      MenuItem->fState |= MF_HILITE;
   }
   else
   {
      MenuItem->fState &= ~MF_HILITE;
   }

   /* FIXME - update the window's menu */

   return TRUE;
}

BOOL FASTCALL
UserSetMenuDefaultItem(PMENU_OBJECT MenuObject, UINT uItem, UINT fByPos)
{
   BOOL ret = FALSE;
   PMENU_ITEM MenuItem = MenuObject->MenuItemList;

   if(uItem == (UINT)-1)
   {
      while(MenuItem)
      {
         MenuItem->fState &= ~MFS_DEFAULT;
         MenuItem = MenuItem->Next;
      }
      return TRUE;
   }

   if(fByPos)
   {
      UINT pos = 0;
      while(MenuItem)
      {
         if(pos == uItem)
         {
            MenuItem->fState |= MFS_DEFAULT;
            ret = TRUE;
         }
         else
         {
            MenuItem->fState &= ~MFS_DEFAULT;
         }
         pos++;
         MenuItem = MenuItem->Next;
      }
   }
   else
   {
      while(MenuItem)
      {
         if(!ret && (MenuItem->wID == uItem))
         {
            MenuItem->fState |= MFS_DEFAULT;
            ret = TRUE;
         }
         else
         {
            MenuItem->fState &= ~MFS_DEFAULT;
         }
         MenuItem = MenuItem->Next;
      }
   }
   return ret;
}


UINT FASTCALL
IntGetMenuDefaultItem(PMENU_OBJECT MenuObject, UINT fByPos, UINT gmdiFlags,
                      DWORD *gismc)
{
   UINT x = 0;
   UINT res = -1;
   UINT sres;
   PMENU_OBJECT SubMenuObject;
   PMENU_ITEM MenuItem = MenuObject->MenuItemList;

   while(MenuItem)
   {
      if(MenuItem->fState & MFS_DEFAULT)
      {

         if(!(gmdiFlags & GMDI_USEDISABLED) && (MenuItem->fState & MFS_DISABLED))
            break;

         if(fByPos)
            res = x;
         else
            res = MenuItem->wID;

         if((*gismc < MAX_GOINTOSUBMENU) && (gmdiFlags & GMDI_GOINTOPOPUPS) &&
               MenuItem->hSubMenu)
         {

            SubMenuObject = UserGetMenuObject(MenuItem->hSubMenu);
            if(!SubMenuObject || (SubMenuObject == MenuObject))
               break;

            (*gismc)++;
            sres = IntGetMenuDefaultItem(SubMenuObject, fByPos, gmdiFlags, gismc);
            (*gismc)--;

            if(sres > (UINT)-1)
               res = sres;
         }

         break;
      }

      MenuItem = MenuItem->Next;
      x++;
   }

   return res;
}

VOID FASTCALL
co_IntInitTracking(PWND Window, PMENU_OBJECT Menu, BOOL Popup,
                   UINT Flags)
{
   /* FIXME - hide caret */

   if(!(Flags & TPM_NONOTIFY))
      co_IntSendMessage(Window->head.h, WM_SETCURSOR, (WPARAM)Window->head.h, HTCAPTION);

   /* FIXME - send WM_SETCURSOR message */

   if(!(Flags & TPM_NONOTIFY))
      co_IntSendMessage(Window->head.h, WM_INITMENU, (WPARAM)Menu->MenuInfo.Self, 0);
}

VOID FASTCALL
co_IntExitTracking(PWND Window, PMENU_OBJECT Menu, BOOL Popup,
                   UINT Flags)
{
   if(!(Flags & TPM_NONOTIFY))
      co_IntSendMessage(Window->head.h, WM_EXITMENULOOP, 0 /* FIXME */, 0);

   /* FIXME - Show caret again */
}

INT FASTCALL
IntTrackMenu(PMENU_OBJECT Menu, PWND Window, INT x, INT y,
             RECTL lprect)
{
   return 0;
}

BOOL FASTCALL
co_IntTrackPopupMenu(PMENU_OBJECT Menu, PWND Window,
                     UINT Flags, POINT *Pos, UINT MenuPos, RECTL *ExcludeRect)
{
   co_IntInitTracking(Window, Menu, TRUE, Flags);

   co_IntExitTracking(Window, Menu, TRUE, Flags);
   return FALSE;
}


/*!
 * Internal function. Called when the process is destroyed to free the remaining menu handles.
*/
BOOL FASTCALL
IntCleanupMenus(struct _EPROCESS *Process, PPROCESSINFO Win32Process)
{
   PEPROCESS CurrentProcess;
   PLIST_ENTRY LastHead = NULL;
   PMENU_OBJECT MenuObject;

   CurrentProcess = PsGetCurrentProcess();
   if (CurrentProcess != Process)
   {
      KeAttachProcess(&Process->Pcb);
   }

   while (Win32Process->MenuListHead.Flink != &(Win32Process->MenuListHead) &&
          Win32Process->MenuListHead.Flink != LastHead)
   {
      LastHead = Win32Process->MenuListHead.Flink;
      MenuObject = CONTAINING_RECORD(Win32Process->MenuListHead.Flink, MENU_OBJECT, ListEntry);

      IntDestroyMenuObject(MenuObject, FALSE, TRUE);
   }

   if (CurrentProcess != Process)
   {
      KeDetachProcess();
   }
   return TRUE;
}

BOOLEAN APIENTRY
intGetTitleBarInfo(PWND pWindowObject, PTITLEBARINFO bti)
{

    DWORD dwStyle = 0;
    DWORD dwExStyle = 0;
    BOOLEAN retValue = TRUE;

    if (bti->cbSize == sizeof(TITLEBARINFO))
    {
        RtlZeroMemory(&bti->rgstate[0],sizeof(DWORD)*(CCHILDREN_TITLEBAR+1));

        bti->rgstate[0] = STATE_SYSTEM_FOCUSABLE;

        dwStyle = pWindowObject->style;
        dwExStyle = pWindowObject->ExStyle;

        bti->rcTitleBar.top  = 0;
        bti->rcTitleBar.left = 0;
        bti->rcTitleBar.right  = pWindowObject->rcWindow.right - pWindowObject->rcWindow.left;
        bti->rcTitleBar.bottom = pWindowObject->rcWindow.bottom - pWindowObject->rcWindow.top;

        /* is it iconiced ? */ 
        if ((dwStyle & WS_ICONIC)!=WS_ICONIC)
        {
            /* Remove frame from rectangle */
            if (HAS_THICKFRAME( dwStyle, dwExStyle ))
            {
                /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CXFRAME) and UserGetSystemMetrics(SM_CYFRAME) */
                RECTL_vInflateRect( &bti->rcTitleBar, -UserGetSystemMetrics(SM_CXFRAME), -UserGetSystemMetrics(SM_CYFRAME) );
            }
            else if (HAS_DLGFRAME( dwStyle, dwExStyle ))
            {
                /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CXDLGFRAME) and UserGetSystemMetrics(SM_CYDLGFRAME) */
                RECTL_vInflateRect( &bti->rcTitleBar, -UserGetSystemMetrics(SM_CXDLGFRAME), -UserGetSystemMetrics(SM_CYDLGFRAME));
            }
            else if (HAS_THINFRAME( dwStyle, dwExStyle))
            {
                /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CXBORDER) and UserGetSystemMetrics(SM_CYBORDER) */
                RECTL_vInflateRect( &bti->rcTitleBar, -UserGetSystemMetrics(SM_CXBORDER), -UserGetSystemMetrics(SM_CYBORDER) );
            }

            /* We have additional border information if the window
             * is a child (but not an MDI child) */
            if ( (dwStyle & WS_CHILD)  &&
                 ((dwExStyle & WS_EX_MDICHILD) == 0 ) )
            {
                if (dwExStyle & WS_EX_CLIENTEDGE)
                {
                    /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CXEDGE) and UserGetSystemMetrics(SM_CYEDGE) */
                    RECTL_vInflateRect (&bti->rcTitleBar, -UserGetSystemMetrics(SM_CXEDGE), -UserGetSystemMetrics(SM_CYEDGE));
                }

                if (dwExStyle & WS_EX_STATICEDGE)
                {
                    /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CXBORDER) and UserGetSystemMetrics(SM_CYBORDER) */
                    RECTL_vInflateRect (&bti->rcTitleBar, -UserGetSystemMetrics(SM_CXBORDER), -UserGetSystemMetrics(SM_CYBORDER));
                }
            }
        }

        bti->rcTitleBar.top += pWindowObject->rcWindow.top;
        bti->rcTitleBar.left += pWindowObject->rcWindow.left;
        bti->rcTitleBar.right += pWindowObject->rcWindow.left;

        bti->rcTitleBar.bottom = bti->rcTitleBar.top;
        if (dwExStyle & WS_EX_TOOLWINDOW)
        {
            /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CYSMCAPTION) */
            bti->rcTitleBar.bottom += UserGetSystemMetrics(SM_CYSMCAPTION);
        }
        else 
        {
            /* FIXME : Note this value should exists in pWindowObject for UserGetSystemMetrics(SM_CYCAPTION) and UserGetSystemMetrics(SM_CXSIZE) */
            bti->rcTitleBar.bottom += UserGetSystemMetrics(SM_CYCAPTION);
            bti->rcTitleBar.left += UserGetSystemMetrics(SM_CXSIZE);
        }

        if (dwStyle & WS_CAPTION) 
        {
            bti->rgstate[1] = STATE_SYSTEM_INVISIBLE;
            if (dwStyle & WS_SYSMENU) 
            {
                if (!(dwStyle & (WS_MINIMIZEBOX|WS_MAXIMIZEBOX))) 
                {
                    bti->rgstate[2] = STATE_SYSTEM_INVISIBLE;
                    bti->rgstate[3] = STATE_SYSTEM_INVISIBLE;
                }
                else 
                {
                    if (!(dwStyle & WS_MINIMIZEBOX))
                    {
                        bti->rgstate[2] = STATE_SYSTEM_UNAVAILABLE;
                    }
                    if (!(dwStyle & WS_MAXIMIZEBOX))
                    {
                        bti->rgstate[3] = STATE_SYSTEM_UNAVAILABLE;
                    }
                }

                if (!(dwExStyle & WS_EX_CONTEXTHELP))
                {
                    bti->rgstate[4] = STATE_SYSTEM_INVISIBLE;
                }
                if (pWindowObject->pcls->style & CS_NOCLOSE)
                {
                    bti->rgstate[5] = STATE_SYSTEM_UNAVAILABLE;
                }
            }
            else 
            {
                bti->rgstate[2] = STATE_SYSTEM_INVISIBLE;
                bti->rgstate[3] = STATE_SYSTEM_INVISIBLE;
                bti->rgstate[4] = STATE_SYSTEM_INVISIBLE;
                bti->rgstate[5] = STATE_SYSTEM_INVISIBLE;
            }
        }
        else
        {
            bti->rgstate[0] |= STATE_SYSTEM_INVISIBLE;
        }
    }
    else
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        retValue = FALSE;
    }

    return retValue;
}

DWORD FASTCALL
UserInsertMenuItem(
   PMENU_OBJECT Menu,
   UINT uItem,
   BOOL fByPosition,
   LPCMENUITEMINFOW UnsafeItemInfo)
{
   NTSTATUS Status;
   ROSMENUITEMINFO ItemInfo;

   /* Try to copy the whole MENUITEMINFOW structure */
   Status = MmCopyFromCaller(&ItemInfo, UnsafeItemInfo, sizeof(MENUITEMINFOW));
   if (NT_SUCCESS(Status))
   {
      if (sizeof(MENUITEMINFOW) != ItemInfo.cbSize
         && FIELD_OFFSET(MENUITEMINFOW, hbmpItem) != ItemInfo.cbSize)
      {
         EngSetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
      }
      return IntInsertMenuItem(Menu, uItem, fByPosition, &ItemInfo);
   }

   /* Try to copy without last field (not present in older versions) */
   Status = MmCopyFromCaller(&ItemInfo, UnsafeItemInfo, FIELD_OFFSET(MENUITEMINFOW, hbmpItem));
   if (NT_SUCCESS(Status))
   {
      if (FIELD_OFFSET(MENUITEMINFOW, hbmpItem) != ItemInfo.cbSize)
      {
         EngSetLastError(ERROR_INVALID_PARAMETER);
         return FALSE;
      }
      ItemInfo.hbmpItem = (HBITMAP)0;
      return IntInsertMenuItem(Menu, uItem, fByPosition, &ItemInfo);
   }

   SetLastNtError(Status);
   return FALSE;
}

UINT FASTCALL IntGetMenuState( HMENU hMenu, UINT uId, UINT uFlags)
{
   PMENU_OBJECT MenuObject, SubMenu;
   PMENU_ITEM mi;

   if (!(MenuObject = UserGetMenuObject(hMenu)))
   {
      return (UINT)-1;
   }

   if (IntGetMenuItemByFlag(MenuObject, uId, uFlags, &SubMenu, &mi, NULL))
   {
      if (mi->hSubMenu)
      {
         if (SubMenu)
         {
            UINT nSubItems = SubMenu->MenuInfo.MenuItemCount;
            return (nSubItems << 8) | ((mi->fState | mi->fType) & 0xff);
         }
         else
            return (UINT)-1;
      }
      return (mi->fType | mi->fState);
   }
   return (UINT)-1;
}

HMENU FASTCALL IntGetSubMenu( HMENU hMenu, int nPos)
{
   PMENU_OBJECT MenuObject, SubMenu;

   if (!(MenuObject = UserGetMenuObject(hMenu)))
   {
      return NULL;
   }
   if (IntGetMenuItemByFlag(MenuObject, nPos, MF_BYPOSITION, &SubMenu, NULL, NULL))
   {
      return SubMenu ? UserHMGetHandle(SubMenu) : NULL;
   }
   return NULL;
}

UINT FASTCALL IntFindSubMenu(HMENU *hMenu, HMENU hSubTarget )
{
   PMENU_OBJECT MenuObject;
   PMENU_ITEM mi;
   UINT i;

   if ( (*hMenu) == (HMENU)0xffff || !(MenuObject = UserGetMenuObject(*hMenu)) )
      return NO_SELECTED_ITEM;

   for (i = 0; i < MenuObject->MenuInfo.MenuItemCount; i++)
   {
       if (!IntGetMenuItemByFlag(MenuObject, i, MF_BYPOSITION, NULL, &mi, NULL))
       {
          return NO_SELECTED_ITEM;
       }

       if (!(mi->fType & MF_POPUP)) continue;

       if (mi->hSubMenu == hSubTarget)
       {
          return i;
       }
       else
       {
          HMENU hsubmenu = mi->hSubMenu;
          UINT pos = IntFindSubMenu(&hsubmenu, hSubTarget );
          if (pos != NO_SELECTED_ITEM)
          {
             *hMenu = hsubmenu;
             return pos;
          }
       }
   }
   return NO_SELECTED_ITEM;
}

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
DWORD APIENTRY
NtUserCheckMenuItem(
   HMENU hMenu,
   UINT uIDCheckItem,
   UINT uCheck)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(DWORD);

   TRACE("Enter NtUserCheckMenuItem\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( (DWORD)-1);
   }

   RETURN( IntCheckMenuItem(Menu, uIDCheckItem, uCheck));

CLEANUP:
   TRACE("Leave NtUserCheckMenuItem, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

HMENU FASTCALL UserCreateMenu(BOOL PopupMenu)
{
   PWINSTATION_OBJECT WinStaObject;
   HANDLE Handle;
   PMENU_OBJECT Menu;
   NTSTATUS Status;
   PEPROCESS CurrentProcess = PsGetCurrentProcess();

   if (CsrProcess != CurrentProcess)
   {
      /*
       * CsrProcess does not have a Win32WindowStation
	   *
	   */

      Status = IntValidateWindowStationHandle(PsGetCurrentProcess()->Win32WindowStation,
                     KernelMode,
                     0,
                     &WinStaObject);

       if (!NT_SUCCESS(Status))
       {
          ERR("Validation of window station handle (0x%X) failed\n",
             CurrentProcess->Win32WindowStation);
          SetLastNtError(Status);
          return (HMENU)0;
       }
       Menu = IntCreateMenu(&Handle, !PopupMenu);
       ObDereferenceObject(WinStaObject);
   }
   else
   {
       Menu = IntCreateMenu(&Handle, !PopupMenu);
   }

   if (Menu) UserDereferenceObject(Menu);
   return (HMENU)Handle;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserDeleteMenu(
   HMENU hMenu,
   UINT uPosition,
   UINT uFlags)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserDeleteMenu\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( FALSE);
   }

   RETURN( IntRemoveMenuItem(Menu, uPosition, uFlags, TRUE));

CLEANUP:
   TRACE("Leave NtUserDeleteMenu, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOLEAN APIENTRY
NtUserGetTitleBarInfo(
    HWND hwnd,
    PTITLEBARINFO bti)
{
    PWND WindowObject;
    TITLEBARINFO bartitleinfo;
    DECLARE_RETURN(BOOLEAN);
    BOOLEAN retValue = TRUE;

    TRACE("Enter NtUserGetTitleBarInfo\n");
    UserEnterExclusive();

    /* Vaildate the windows handle */
    if (!(WindowObject = UserGetWindowObject(hwnd)))
    {
        EngSetLastError(ERROR_INVALID_WINDOW_HANDLE);
        retValue = FALSE;
    }

    _SEH2_TRY
    {
        /* Copy our usermode buffer bti to local buffer bartitleinfo */
        ProbeForRead(bti, sizeof(TITLEBARINFO), 1);
        RtlCopyMemory(&bartitleinfo, bti, sizeof(TITLEBARINFO));
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Fail copy the data */ 
        EngSetLastError(ERROR_INVALID_PARAMETER);
        retValue = FALSE;
    }
    _SEH2_END

    /* Get the tile bar info */ 
    if (retValue)
    {
        retValue = intGetTitleBarInfo(WindowObject, &bartitleinfo);
        if (retValue)
        {
            _SEH2_TRY
            {
                /* Copy our buffer to user mode buffer bti */
                ProbeForWrite(bti, sizeof(TITLEBARINFO), 1);
                RtlCopyMemory(bti, &bartitleinfo, sizeof(TITLEBARINFO));
            }
            _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
            {
                /* Fail copy the data */ 
                EngSetLastError(ERROR_INVALID_PARAMETER);
                retValue = FALSE;
            }
            _SEH2_END
        }
    }

    RETURN( retValue );

CLEANUP:
    TRACE("Leave NtUserGetTitleBarInfo, ret=%i\n",_ret_);
    UserLeave();
    END_CLEANUP;
}

/*
 * @implemented
 */
BOOL FASTCALL UserDestroyMenu(HMENU hMenu)
{
   PMENU_OBJECT Menu;

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      return FALSE;
   }

   if(Menu->Process != PsGetCurrentProcess())
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   return IntDestroyMenuObject(Menu, FALSE, TRUE);
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserDestroyMenu(
   HMENU hMenu)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserDestroyMenu\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( FALSE);
   }

   if(Menu->Process != PsGetCurrentProcess())
   {
      EngSetLastError(ERROR_ACCESS_DENIED);
      RETURN( FALSE);
   }

   RETURN( IntDestroyMenuObject(Menu, TRUE, TRUE));

CLEANUP:
   TRACE("Leave NtUserDestroyMenu, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
UINT APIENTRY
NtUserEnableMenuItem(
   HMENU hMenu,
   UINT uIDEnableItem,
   UINT uEnable)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(UINT);

   TRACE("Enter NtUserEnableMenuItem\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(-1);
   }

   RETURN( IntEnableMenuItem(Menu, uIDEnableItem, uEnable));

CLEANUP:
   TRACE("Leave NtUserEnableMenuItem, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserGetMenuBarInfo(
   HWND hwnd,
   LONG idObject,
   LONG idItem,
   PMENUBARINFO pmbi)
{
   BOOL Res = TRUE;
   PMENU_OBJECT MenuObject;
   PMENU_ITEM mi;
   PWND WindowObject;
   HMENU hMenu;
   POINT Offset;
   RECTL Rect;
   MENUBARINFO kmbi;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserGetMenuBarInfo\n");
   UserEnterShared();

   if (!(WindowObject = UserGetWindowObject(hwnd)))
     {
        EngSetLastError(ERROR_INVALID_WINDOW_HANDLE);
        RETURN(FALSE);
     }

   hMenu = (HMENU)(DWORD_PTR)WindowObject->IDMenu;

   if (!(MenuObject = UserGetMenuObject(hMenu)))
     {
       EngSetLastError(ERROR_INVALID_MENU_HANDLE);
       RETURN(FALSE);
     }

   if (pmbi->cbSize != sizeof(MENUBARINFO))
     {
       EngSetLastError(ERROR_INVALID_PARAMETER);
       RETURN(FALSE);
     }

   kmbi.cbSize = sizeof(MENUBARINFO);
   kmbi.fBarFocused = FALSE;
   kmbi.fFocused = FALSE;
   kmbi.hwndMenu = NULL;

   switch (idObject)
   {
      case OBJID_MENU:
      {
         PMENU_OBJECT SubMenuObject;
         kmbi.hMenu = hMenu;
         if (idItem) /* Non-Zero-Based. */
           {
              if (IntGetMenuItemByFlag(MenuObject, idItem-1, MF_BYPOSITION, NULL, &mi, NULL) > -1)
                   kmbi.rcBar = mi->Rect;
              else
                {
                   Res = FALSE;
                   break;
                }
           }
         else
           {
              /* If items is zero we assume info for the menu itself. */
              if (!(IntGetClientOrigin(WindowObject, &Offset)))
                {
                   Res = FALSE;
                   break;
                }
              Rect.left = Offset.x;
              Rect.right = Offset.x + MenuObject->MenuInfo.Width;
              Rect.bottom = Offset.y;
              Rect.top = Offset.y - MenuObject->MenuInfo.Height;
              kmbi.rcBar = Rect;
              TRACE("Rect top = %d bottom = %d left = %d right = %d \n",
                       Rect.top, Rect.bottom, Rect.left, Rect.right);
           }
         if (idItem)
           {
              if (idItem-1 == MenuObject->MenuInfo.FocusedItem)
                    kmbi.fFocused = TRUE;
           }
         if (MenuObject->MenuInfo.FocusedItem != NO_SELECTED_ITEM)
               kmbi.fBarFocused = TRUE;
         SubMenuObject = UserGetMenuObject(MenuObject->MenuItemList->hSubMenu);
         if(SubMenuObject) kmbi.hwndMenu = SubMenuObject->MenuInfo.Wnd;
         TRACE("OBJID_MENU, idItem = %d\n",idItem);
         break;
      }
      case OBJID_CLIENT:
      {
         PMENU_OBJECT SubMenuObject, XSubMenuObject;
         SubMenuObject = UserGetMenuObject(MenuObject->MenuItemList->hSubMenu);
         if(SubMenuObject) kmbi.hMenu = SubMenuObject->MenuInfo.Self;
         else
           {
              Res = FALSE;
              ERR("OBJID_CLIENT, No SubMenu!\n");
              break;
           }
         if (idItem)
           {
              if (IntGetMenuItemByFlag(SubMenuObject, idItem-1, MF_BYPOSITION, NULL, &mi, NULL) > -1)
                   kmbi.rcBar = mi->Rect;
              else
                {
                   Res = FALSE;
                   break;
                }
           }
         else
           {
              PWND SubWinObj;
              if (!(SubWinObj = UserGetWindowObject(SubMenuObject->MenuInfo.Wnd)))
                {
                   Res = FALSE;
                   break;
                }
              if (!(IntGetClientOrigin(SubWinObj, &Offset)))
                {
                   Res = FALSE;
                   break;
                }
              Rect.left = Offset.x;
              Rect.right = Offset.x + SubMenuObject->MenuInfo.Width;
              Rect.top = Offset.y;
              Rect.bottom = Offset.y + SubMenuObject->MenuInfo.Height;
              kmbi.rcBar = Rect;
           }
         if (idItem)
           {
              if (idItem-1 == SubMenuObject->MenuInfo.FocusedItem)
                   kmbi.fFocused = TRUE;
           }
         if (SubMenuObject->MenuInfo.FocusedItem != NO_SELECTED_ITEM)
               kmbi.fBarFocused = TRUE;
         XSubMenuObject = UserGetMenuObject(SubMenuObject->MenuItemList->hSubMenu);
         if (XSubMenuObject) kmbi.hwndMenu = XSubMenuObject->MenuInfo.Wnd;
         TRACE("OBJID_CLIENT, idItem = %d\n",idItem);
         break;
      }
      case OBJID_SYSMENU:
      {
         PMENU_OBJECT SysMenuObject, SubMenuObject;
         if(!(SysMenuObject = IntGetSystemMenu(WindowObject, FALSE, FALSE)))
         {
           Res = FALSE;
           break;
         }
         kmbi.hMenu = SysMenuObject->MenuInfo.Self;
         if (idItem)
           {
              if (IntGetMenuItemByFlag(SysMenuObject, idItem-1, MF_BYPOSITION, NULL, &mi, NULL) > -1)
                   kmbi.rcBar = mi->Rect;
              else
                {
                   Res = FALSE;
                   break;
                }
           }
         else
           {
              PWND SysWinObj;
              if (!(SysWinObj = UserGetWindowObject(SysMenuObject->MenuInfo.Wnd)))
                {
                   Res = FALSE;
                   break;
                }
              if (!(IntGetClientOrigin(SysWinObj, &Offset)))
                {
                   Res = FALSE;
                   break;
                }
              Rect.left = Offset.x;
              Rect.right = Offset.x + SysMenuObject->MenuInfo.Width;
              Rect.top = Offset.y;
              Rect.bottom = Offset.y + SysMenuObject->MenuInfo.Height;
              kmbi.rcBar = Rect;
           }
         if (idItem)
           {
              if (idItem-1 == SysMenuObject->MenuInfo.FocusedItem)
                    kmbi.fFocused = TRUE;
           }
         if (SysMenuObject->MenuInfo.FocusedItem != NO_SELECTED_ITEM)
               kmbi.fBarFocused = TRUE;
         SubMenuObject = UserGetMenuObject(SysMenuObject->MenuItemList->hSubMenu);
         if(SubMenuObject) kmbi.hwndMenu = SubMenuObject->MenuInfo.Wnd;
         TRACE("OBJID_SYSMENU, idItem = %d\n",idItem);
         break;
      }
      default:
         Res = FALSE;
         ERR("Unknown idObject = %d, idItem = %d\n",idObject,idItem);
   }
   if (Res)
     {
        NTSTATUS Status = MmCopyToCaller(pmbi, &kmbi, sizeof(MENUBARINFO));
        if (! NT_SUCCESS(Status))
          {
            SetLastNtError(Status);
            RETURN(FALSE);
          }
     }
   RETURN(Res);

CLEANUP:
   TRACE("Leave NtUserGetMenuBarInfo, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
UINT APIENTRY
NtUserGetMenuIndex(
   HMENU hMenu,
   HMENU hSubMenu)
{
   PMENU_OBJECT Menu, SubMenu;
   PMENU_ITEM MenuItem;
   DECLARE_RETURN(UINT);

   TRACE("Enter NtUserGetMenuIndex\n");
   UserEnterShared();

   if ( !(Menu = UserGetMenuObject(hMenu)) ||
        !(SubMenu = UserGetMenuObject(hSubMenu)) )
      RETURN(0xFFFFFFFF);

   MenuItem = Menu->MenuItemList;
   while(MenuItem)
   {
      if (MenuItem->hSubMenu == hSubMenu)
         RETURN(MenuItem->wID);
      MenuItem = MenuItem->Next;
   }

   RETURN(0xFFFFFFFF);

CLEANUP:
   TRACE("Leave NtUserGetMenuIndex, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserGetMenuItemRect(
   HWND hWnd,
   HMENU hMenu,
   UINT uItem,
   PRECTL lprcItem)
{
   PWND ReferenceWnd;
   LONG XMove, YMove;
   RECTL Rect;
   NTSTATUS Status;
   PMENU_OBJECT Menu;
   PMENU_ITEM MenuItem;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserGetMenuItemRect\n");
   UserEnterShared();

   if (!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(FALSE);
   }

   if (IntGetMenuItemByFlag(Menu, uItem, MF_BYPOSITION, NULL, &MenuItem, NULL) > -1)
        Rect = MenuItem->Rect;
   else
      RETURN(FALSE);

   if(!hWnd)
   {
       hWnd = Menu->MenuInfo.Wnd;
   }

   if (lprcItem == NULL) RETURN( FALSE);

   if (!(ReferenceWnd = UserGetWindowObject(hWnd))) RETURN( FALSE);

   if(MenuItem->fType & MF_POPUP)
   {
     XMove = ReferenceWnd->rcClient.left;
     YMove = ReferenceWnd->rcClient.top;
   }
   else
   {
     XMove = ReferenceWnd->rcWindow.left;
     YMove = ReferenceWnd->rcWindow.top;
   }

   Rect.left   += XMove;
   Rect.top    += YMove;
   Rect.right  += XMove;
   Rect.bottom += YMove;

   Status = MmCopyToCaller(lprcItem, &Rect, sizeof(RECT));
   if (! NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      RETURN( FALSE);
   }
   RETURN( TRUE);

CLEANUP:
   TRACE("Leave NtUserGetMenuItemRect, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserHiliteMenuItem(
   HWND hWnd,
   HMENU hMenu,
   UINT uItemHilite,
   UINT uHilite)
{
   PMENU_OBJECT Menu;
   PWND Window;
   DECLARE_RETURN(BOOLEAN);

   TRACE("Enter NtUserHiliteMenuItem\n");
   UserEnterExclusive();

   if(!(Window = UserGetWindowObject(hWnd)))
   {
      EngSetLastError(ERROR_INVALID_WINDOW_HANDLE);
      RETURN(FALSE);
   }

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      EngSetLastError(ERROR_INVALID_MENU_HANDLE);
      RETURN(FALSE);
   }

   if(Window->IDMenu == (UINT)(UINT_PTR)hMenu)
   {
      RETURN( IntHiliteMenuItem(Window, Menu, uItemHilite, uHilite));
   }

   RETURN(FALSE);

CLEANUP:
   TRACE("Leave NtUserHiliteMenuItem, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

static
BOOL FASTCALL
UserMenuInfo(
   PMENU_OBJECT Menu,
   PROSMENUINFO UnsafeMenuInfo,
   BOOL SetOrGet)
{
   BOOL Res;
   DWORD Size;
   NTSTATUS Status;
   ROSMENUINFO MenuInfo;

   Status = MmCopyFromCaller(&Size, &UnsafeMenuInfo->cbSize, sizeof(DWORD));
   if (! NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      return( FALSE);
   }
   if(Size < sizeof(MENUINFO) || sizeof(ROSMENUINFO) < Size)
   {
      EngSetLastError(ERROR_INVALID_PARAMETER);
      return( FALSE);
   }
   Status = MmCopyFromCaller(&MenuInfo, UnsafeMenuInfo, Size);
   if (! NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      return( FALSE);
   }

   if(SetOrGet)
   {
      /* Set MenuInfo */
      Res = IntSetMenuInfo(Menu, &MenuInfo);
   }
   else
   {
      /* Get MenuInfo */
      Res = IntGetMenuInfo(Menu, &MenuInfo);
      if (Res)
      {
         Status = MmCopyToCaller(UnsafeMenuInfo, &MenuInfo, Size);
         if (! NT_SUCCESS(Status))
         {
            SetLastNtError(Status);
            return( FALSE);
         }
      }
   }

   return( Res);
}

/*
 * @implemented
 */
int APIENTRY
NtUserMenuItemFromPoint(
   HWND hWnd,
   HMENU hMenu,
   DWORD X,
   DWORD Y)
{
   PMENU_OBJECT Menu;
   PWND Window = NULL;
   PMENU_ITEM mi;
   int i;
   DECLARE_RETURN(int);

   TRACE("Enter NtUserMenuItemFromPoint\n");
   UserEnterExclusive();

   if (!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( -1);
   }

   if (!(Window = UserGetWindowObject(Menu->MenuInfo.Wnd)))
   {
      RETURN( -1);
   }

   X -= Window->rcWindow.left;
   Y -= Window->rcWindow.top;

   mi = Menu->MenuItemList;
   for (i = 0; NULL != mi; i++)
   {
      if (RECTL_bPointInRect(&(mi->Rect), X, Y))
      {
         break;
      }
      mi = mi->Next;
   }

   RETURN( (mi ? i : NO_SELECTED_ITEM));

CLEANUP:
   TRACE("Leave NtUserMenuItemFromPoint, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

static
BOOL FASTCALL
UserMenuItemInfo(
   PMENU_OBJECT Menu,
   UINT Item,
   BOOL ByPosition,
   PROSMENUITEMINFO UnsafeItemInfo,
   BOOL SetOrGet)
{
   PMENU_ITEM MenuItem;
   ROSMENUITEMINFO ItemInfo;
   NTSTATUS Status;
   UINT Size;
   BOOL Ret;

   Status = MmCopyFromCaller(&Size, &UnsafeItemInfo->cbSize, sizeof(UINT));
   if (! NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      return( FALSE);
   }
   if (sizeof(MENUITEMINFOW) != Size
         && FIELD_OFFSET(MENUITEMINFOW, hbmpItem) != Size
         && sizeof(ROSMENUITEMINFO) != Size)
   {
      EngSetLastError(ERROR_INVALID_PARAMETER);
      return( FALSE);
   }
   Status = MmCopyFromCaller(&ItemInfo, UnsafeItemInfo, Size);
   if (! NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      return( FALSE);
   }
   /* If this is a pre-0x0500 _WIN32_WINNT MENUITEMINFOW, you can't
      set/get hbmpItem */
   if (FIELD_OFFSET(MENUITEMINFOW, hbmpItem) == Size
         && 0 != (ItemInfo.fMask & MIIM_BITMAP))
   {
      EngSetLastError(ERROR_INVALID_PARAMETER);
      return( FALSE);
   }

   if (IntGetMenuItemByFlag(Menu, Item,
                            (ByPosition ? MF_BYPOSITION : MF_BYCOMMAND),
                            NULL, &MenuItem, NULL) < 0)
   {
      EngSetLastError(ERROR_INVALID_PARAMETER);
// This will crash menu (line 80) correct_behavior test!
// "NT4 and below can't handle a bigger MENUITEMINFO struct" 
      //EngSetLastError(ERROR_MENU_ITEM_NOT_FOUND);
      return( FALSE);
   }

   if (SetOrGet)
   {
      Ret = IntSetMenuItemInfo(Menu, MenuItem, &ItemInfo);
   }
   else
   {
      Ret = IntGetMenuItemInfo(Menu, MenuItem, &ItemInfo);
      if (Ret)
      {
         Status = MmCopyToCaller(UnsafeItemInfo, &ItemInfo, Size);
         if (! NT_SUCCESS(Status))
         {
            SetLastNtError(Status);
            return( FALSE);
         }
      }
   }

   return( Ret);
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserRemoveMenu(
   HMENU hMenu,
   UINT uPosition,
   UINT uFlags)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserRemoveMenu\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( FALSE);
   }

   RETURN(IntRemoveMenuItem(Menu, uPosition, uFlags, FALSE));

CLEANUP:
   TRACE("Leave NtUserRemoveMenu, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;

}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserSetMenuContextHelpId(
   HMENU hMenu,
   DWORD dwContextHelpId)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserSetMenuContextHelpId\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( FALSE);
   }

   RETURN(IntSetMenuContextHelpId(Menu, dwContextHelpId));

CLEANUP:
   TRACE("Leave NtUserSetMenuContextHelpId, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserSetMenuDefaultItem(
   HMENU hMenu,
   UINT uItem,
   UINT fByPos)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserSetMenuDefaultItem\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( FALSE);
   }

   RETURN( UserSetMenuDefaultItem(Menu, uItem, fByPos));

CLEANUP:
   TRACE("Leave NtUserSetMenuDefaultItem, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserSetMenuFlagRtoL(
   HMENU hMenu)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserSetMenuFlagRtoL\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( FALSE);
   }

   RETURN(IntSetMenuFlagRtoL(Menu));

CLEANUP:
   TRACE("Leave NtUserSetMenuFlagRtoL, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserThunkedMenuInfo(
   HMENU hMenu,
   LPCMENUINFO lpcmi)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserThunkedMenuInfo\n");
   UserEnterExclusive();

   if (!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(FALSE);
   }

   RETURN(UserMenuInfo(Menu, (PROSMENUINFO)lpcmi, TRUE));

CLEANUP:
   TRACE("Leave NtUserThunkedMenuInfo, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserThunkedMenuItemInfo(
   HMENU hMenu,
   UINT uItem,
   BOOL fByPosition,
   BOOL bInsert,
   LPMENUITEMINFOW lpmii,
   PUNICODE_STRING lpszCaption)
{
   PMENU_OBJECT Menu;
   NTSTATUS Status;
   UNICODE_STRING lstrCaption;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserThunkedMenuItemInfo\n");
   UserEnterExclusive();

   /* lpszCaption may be NULL, check for it and call RtlInitUnicodeString()
      if bInsert == TRUE call UserInsertMenuItem() else UserSetMenuItemInfo()   */

   if (!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(FALSE);
   }

   lstrCaption.Buffer = NULL;

   /* Check if we got a Caption */
   if (lpszCaption)
   {
      /* Copy the string to kernel mode */
      Status = ProbeAndCaptureUnicodeString( &lstrCaption,
                                                 UserMode,
                                              lpszCaption);
      if (!NT_SUCCESS(Status))
      {
         ERR("Failed to capture MenuItem Caption (status 0x%08x)\n",Status);
         SetLastNtError(Status);
         RETURN(FALSE);
      }       
   }

   if (bInsert) RETURN( UserInsertMenuItem(Menu, uItem, fByPosition, lpmii));

   RETURN( UserMenuItemInfo(Menu, uItem, fByPosition, (PROSMENUITEMINFO)lpmii, TRUE));

CLEANUP:
   TRACE("Leave NtUserThunkedMenuItemInfo, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

////// Odyssey NtUserBad
/*
 * @implemented
 */
DWORD
APIENTRY
NtUserBuildMenuItemList(
   HMENU hMenu,
   VOID* Buffer,
   ULONG nBufSize,
   DWORD Reserved)
{
   DWORD res = -1;
   PMENU_OBJECT Menu;
   DECLARE_RETURN(DWORD);

   TRACE("Enter NtUserBuildMenuItemList\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN( (DWORD)-1);
   }

   if(Buffer)
   {
      res = IntBuildMenuItemList(Menu, Buffer, nBufSize);
   }
   else
   {
      res = Menu->MenuInfo.MenuItemCount;
   }

   RETURN( res);

CLEANUP:
   TRACE("Leave NtUserBuildMenuItemList, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
UINT APIENTRY
NtUserGetMenuDefaultItem(
   HMENU hMenu,
   UINT fByPos,
   UINT gmdiFlags)
{
   PMENU_OBJECT Menu;
   DWORD gismc = 0;
   DECLARE_RETURN(UINT);

   TRACE("Enter NtUserGetMenuDefaultItem\n");
   UserEnterExclusive();

   if(!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(-1);
   }

   RETURN( IntGetMenuDefaultItem(Menu, fByPos, gmdiFlags, &gismc));

CLEANUP:
   TRACE("Leave NtUserGetMenuDefaultItem, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL
APIENTRY
NtUserMenuInfo(
   HMENU hMenu,
   PROSMENUINFO UnsafeMenuInfo,
   BOOL SetOrGet)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserMenuInfo\n");
   UserEnterShared();

   if (!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(FALSE);
   }

   RETURN(UserMenuInfo(Menu, UnsafeMenuInfo, SetOrGet));

CLEANUP:
   TRACE("Leave NtUserMenuInfo, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

/*
 * @implemented
 */
BOOL
APIENTRY
NtUserMenuItemInfo(
   HMENU hMenu,
   UINT Item,
   BOOL ByPosition,
   PROSMENUITEMINFO UnsafeItemInfo,
   BOOL SetOrGet)
{
   PMENU_OBJECT Menu;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserMenuItemInfo\n");
   UserEnterExclusive();

   if (!(Menu = UserGetMenuObject(hMenu)))
   {
      RETURN(FALSE);
   }

   RETURN( UserMenuItemInfo(Menu, Item, ByPosition, UnsafeItemInfo, SetOrGet));

CLEANUP:
   TRACE("Leave NtUserMenuItemInfo, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;

}

/* EOF */
