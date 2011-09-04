#pragma once

PMENU_OBJECT FASTCALL UserGetMenuObject(HMENU hMenu);

#define ASSERT_REFS_CO(_obj_) \
{ \
   LONG ref = ((PHEAD)_obj_)->cLockObj;\
   if (!(ref >= 1)){ \
      ERR_CH(UserObj, "ASSERT: obj 0x%x, refs %i\n", _obj_, ref); \
      ASSERT(FALSE); \
   } \
}

#if 0
#define ASSERT_REFS_CO(_obj_) \
{ \
   PSINGLE_LIST_ENTRY e; \
   BOOL gotit=FALSE; \
   LONG ref = ((PHEAD)_obj_)->cLockObj;\
   if (!(ref >= 1)){ \
      ERR_CH(UserObj, "obj 0x%x, refs %i\n", _obj_, ref); \
      ASSERT(FALSE); \
   } \
   \
   e = PsGetCurrentThreadWin32Thread()->ReferencesList.Next; \
   while (e) \
   { \
      PUSER_REFERENCE_ENTRY ref = CONTAINING_RECORD(e, USER_REFERENCE_ENTRY, Entry); \
      if (ref->obj == _obj_){ gotit=TRUE; break; } \
      e = e->Next; \
   } \
   ASSERT(gotit); \
}
#endif

#define DUMP_REFS(obj) TRACE_CH(UserObj,"obj 0x%x, refs %i\n",obj, ((PHEAD)obj)->cLockObj)

PWND FASTCALL IntGetWindowObject(HWND hWnd);

/*************** WINSTA.C ***************/

HWINSTA FASTCALL UserGetProcessWindowStation(VOID);

/*************** FOCUS.C ***************/

HWND FASTCALL UserGetActiveWindow(VOID);

HWND FASTCALL UserGetForegroundWindow(VOID);

HWND FASTCALL co_UserSetFocus(PWND Window);

/*************** WINDC.C ***************/

INT FASTCALL UserReleaseDC(PWND Window, HDC hDc, BOOL EndPaint);
HDC FASTCALL UserGetDCEx(PWND Window OPTIONAL, HANDLE ClipRegion, ULONG Flags);
HDC FASTCALL UserGetWindowDC(PWND Wnd);

/*************** SESSION.C ***************/

extern PRTL_ATOM_TABLE gAtomTable;
NTSTATUS FASTCALL InitSessionImpl(VOID);

/*************** METRIC.C ***************/

BOOL FASTCALL InitMetrics(VOID);
ULONG FASTCALL UserGetSystemMetrics(ULONG Index);

/*************** KEYBOARD.C ***************/

DWORD FASTCALL UserGetKeyState(DWORD key);
DWORD FASTCALL UserGetKeyboardType(DWORD TypeFlag);
HKL FASTCALL UserGetKeyboardLayout(DWORD dwThreadId);


/*************** MISC.C ***************/

BOOL FASTCALL
UserSystemParametersInfo(
  UINT uiAction,
  UINT uiParam,
  PVOID pvParam,
  UINT fWinIni);

/*************** MESSAGE.C ***************/

BOOL FASTCALL
UserPostMessage(HWND Wnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

/*************** WINDOW.C ***************/

PWND FASTCALL UserGetWindowObject(HWND hWnd);
VOID FASTCALL co_DestroyThreadWindows(struct _ETHREAD *Thread);
HWND FASTCALL UserGetShellWindow(VOID);
HDC FASTCALL UserGetDCEx(PWND Window OPTIONAL, HANDLE ClipRegion, ULONG Flags);
BOOLEAN FASTCALL co_UserDestroyWindow(PWND Wnd);
PWND FASTCALL UserGetAncestor(PWND Wnd, UINT Type);

/*************** MENU.C ***************/

HMENU FASTCALL UserCreateMenu(BOOL PopupMenu);
BOOL FASTCALL UserSetMenuDefaultItem(PMENU_OBJECT Menu, UINT uItem, UINT fByPos);
BOOL FASTCALL UserDestroyMenu(HMENU hMenu);

/*************** SCROLLBAR.C ***************/

DWORD FASTCALL
co_UserShowScrollBar(PWND Window, int wBar, DWORD bShow);

/* EOF */
