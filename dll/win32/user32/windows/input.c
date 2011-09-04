/*
 *  Odyssey kernel
 *  Copyright (C) 1998, 1999, 2000, 2001 ReactOS Team; (C) 2011 NasuTek Enterprises
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * PROJECT:         Odyssey user32.dll
 * FILE:            dll/win32/user32/windows/input.c
 * PURPOSE:         Input
 * PROGRAMMER:      Casper S. Hornstrup (chorns@users.sourceforge.net)
 * UPDATE HISTORY:
 *      09-05-2001  CSH  Created
 */

/* INCLUDES ******************************************************************/

#include <user32.h>

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(user32);

/* GLOBALS *******************************************************************/



/* FUNCTIONS *****************************************************************/


/*
 * @implemented
 */
BOOL
WINAPI
DragDetect(
  HWND hWnd,
  POINT pt)
{
  return NtUserDragDetect(hWnd, pt);
#if 0
  MSG msg;
  RECT rect;
  POINT tmp;
  ULONG dx = GetSystemMetrics(SM_CXDRAG);
  ULONG dy = GetSystemMetrics(SM_CYDRAG);

  rect.left = pt.x - dx;
  rect.right = pt.x + dx;
  rect.top = pt.y - dy;
  rect.bottom = pt.y + dy;

  SetCapture(hWnd);

  for (;;)
  {
    while (
    PeekMessageW(&msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE) ||
    PeekMessageW(&msg, 0, WM_KEYFIRST,   WM_KEYLAST,   PM_REMOVE)
    )
    {
      if (msg.message == WM_LBUTTONUP)
      {
        ReleaseCapture();
        return FALSE;
      }
      if (msg.message == WM_MOUSEMOVE)
      {
        tmp.x = LOWORD(msg.lParam);
        tmp.y = HIWORD(msg.lParam);
        if (!PtInRect(&rect, tmp))
        {
          ReleaseCapture();
          return TRUE;
        }
      }
      if (msg.message == WM_KEYDOWN)
      {
         if (msg.wParam == VK_ESCAPE)
         {
             ReleaseCapture();
             return TRUE;
         }
      }
    }
    WaitMessage();
  }
  return 0;
#endif
}

/*
 * @implemented
 */
BOOL WINAPI
EnableWindow(HWND hWnd, BOOL bEnable)
{
  return NtUserxEnableWindow(hWnd, bEnable);
}

/*
 * @implemented
 */
SHORT WINAPI
GetAsyncKeyState(int vKey)
{
 if (vKey < 0 || vKey > 256)
    return 0;
 return (SHORT) NtUserGetAsyncKeyState((DWORD) vKey);
}


/*
 * @implemented
 */
HKL WINAPI
GetKeyboardLayout(DWORD idThread)
{
  return NtUserxGetKeyboardLayout(idThread);
}


/*
 * @implemented
 */
UINT WINAPI
GetKBCodePage(VOID)
{
  return GetOEMCP();
}


/*
 * @implemented
 */
int WINAPI
GetKeyNameTextA(LONG lParam,
		LPSTR lpString,
		int nSize)
{
  LPWSTR intermediateString =
    HeapAlloc(GetProcessHeap(),0,nSize * sizeof(WCHAR));
  int ret = 0;
  UINT wstrLen = 0;
  BOOL defChar = FALSE;

  if( !intermediateString ) return 0;
  ret = GetKeyNameTextW(lParam,intermediateString,nSize);
  if( ret == 0 ) { lpString[0] = 0; return 0; }

  wstrLen = wcslen( intermediateString );
  ret = WideCharToMultiByte(CP_ACP, 0,
			    intermediateString, wstrLen,
			    lpString, nSize, ".", &defChar );
  lpString[ret] = 0;
  HeapFree(GetProcessHeap(),0,intermediateString);

  return ret;
}

/*
 * @implemented
 */
int WINAPI
GetKeyNameTextW(LONG lParam,
		LPWSTR lpString,
		int nSize)
{
  return NtUserGetKeyNameText( lParam, lpString, nSize );
}

/*
 * @implemented
 */
SHORT WINAPI
GetKeyState(int nVirtKey)
{
 return (SHORT) NtUserGetKeyState((DWORD) nVirtKey);
}

/*
 * @implemented
 */
BOOL WINAPI
GetKeyboardLayoutNameA(LPSTR pwszKLID)
{
  WCHAR buf[KL_NAMELENGTH];

  if (GetKeyboardLayoutNameW(buf))
    return WideCharToMultiByte( CP_ACP, 0, buf, -1, pwszKLID, KL_NAMELENGTH, NULL, NULL ) != 0;
  return FALSE;
}


/*
 * @implemented
 */
BOOL WINAPI
GetKeyboardLayoutNameW(LPWSTR pwszKLID)
{
  return NtUserGetKeyboardLayoutName( pwszKLID );
}


/*
 * @implemented
 */
int WINAPI
GetKeyboardType(int nTypeFlag)
{
  return NtUserxGetKeyboardType( nTypeFlag );
}

/*
 * @implemented
 */
BOOL WINAPI
GetLastInputInfo(PLASTINPUTINFO plii)
{
  TRACE("%p\n", plii);

  if (plii->cbSize != sizeof (*plii) )
  {
     SetLastError(ERROR_INVALID_PARAMETER);
     return FALSE;
  }

  plii->dwTime = gpsi->dwLastRITEventTickCount;
  return TRUE;
}

/*
 * @implemented
 */
HKL WINAPI
LoadKeyboardLayoutA(LPCSTR pwszKLID,
		    UINT Flags)
{
  return NtUserLoadKeyboardLayoutEx( NULL, 0, NULL, NULL, NULL,
               strtoul(pwszKLID, NULL, 16),
               Flags);
}

/*
 * @implemented
 */
HKL WINAPI
LoadKeyboardLayoutW(LPCWSTR pwszKLID,
		    UINT Flags)
{
  // Look at revision 25596 to see how it's done in windows.
  // We will do things our own way. Also be compatible too!
  return NtUserLoadKeyboardLayoutEx( NULL, 0, NULL, NULL, NULL,
               wcstoul(pwszKLID, NULL, 16),
               Flags);
}

/*
 * @implemented
 */
UINT WINAPI
MapVirtualKeyA(UINT uCode,
	       UINT uMapType)
{
  return MapVirtualKeyExA( uCode, uMapType, GetKeyboardLayout( 0 ) );
}

/*
 * @implemented
 */
UINT WINAPI
MapVirtualKeyExA(UINT uCode,
		 UINT uMapType,
		 HKL dwhkl)
{
  return MapVirtualKeyExW( uCode, uMapType, dwhkl );
}


/*
 * @implemented
 */
UINT WINAPI
MapVirtualKeyExW(UINT uCode,
		 UINT uMapType,
		 HKL dwhkl)
{
  return NtUserMapVirtualKeyEx( uCode, uMapType, 0, dwhkl );
}


/*
 * @implemented
 */
UINT WINAPI
MapVirtualKeyW(UINT uCode,
	       UINT uMapType)
{
  return MapVirtualKeyExW( uCode, uMapType, GetKeyboardLayout( 0 ) );
}


/*
 * @implemented
 */
DWORD WINAPI
OemKeyScan(WORD wOemChar)
{
  WCHAR p;
  SHORT Vk;
  UINT Scan;

  MultiByteToWideChar(CP_OEMCP, 0, (PCSTR)&wOemChar, 1, &p, 1);
  Vk = VkKeyScanW(p);
  Scan = MapVirtualKeyW((Vk & 0x00ff), 0);
  if(!Scan) return -1;
  /*
     Page 450-1, MS W2k SuperBible by SAMS. Return, low word has the
     scan code and high word has the shift state.
   */
  return ((Vk & 0xff00) << 8) | Scan;
}


/*
 * @implemented
 */
BOOL WINAPI
SetDoubleClickTime(UINT uInterval)
{
  return (BOOL)NtUserSystemParametersInfo(SPI_SETDOUBLECLICKTIME,
                                             uInterval,
                                             NULL,
                                             0);
}


/*
 * @implemented
 */
BOOL
WINAPI
SwapMouseButton(
  BOOL fSwap)
{
  return NtUserxSwapMouseButton(fSwap);
}


/*
 * @implemented
 */
int WINAPI
ToAscii(UINT uVirtKey,
	UINT uScanCode,
	CONST BYTE *lpKeyState,
	LPWORD lpChar,
	UINT uFlags)
{
  return ToAsciiEx(uVirtKey, uScanCode, lpKeyState, lpChar, uFlags, 0);
}


/*
 * @implemented
 */
int WINAPI
ToAsciiEx(UINT uVirtKey,
	  UINT uScanCode,
	  CONST BYTE *lpKeyState,
	  LPWORD lpChar,
	  UINT uFlags,
	  HKL dwhkl)
{
  WCHAR UniChars[2];
  int Ret, CharCount;

  Ret = ToUnicodeEx(uVirtKey, uScanCode, lpKeyState, UniChars, 2, uFlags, dwhkl);
  CharCount = (Ret < 0 ? 1 : Ret);
  WideCharToMultiByte(CP_ACP, 0, UniChars, CharCount, (LPSTR) lpChar, 2, NULL, NULL);

  return Ret;
}


/*
 * @implemented
 */
int WINAPI
ToUnicode(UINT wVirtKey,
	  UINT wScanCode,
	  CONST BYTE *lpKeyState,
	  LPWSTR pwszBuff,
	  int cchBuff,
	  UINT wFlags)
{
  return ToUnicodeEx( wVirtKey, wScanCode, lpKeyState, pwszBuff, cchBuff,
		      wFlags, 0 );
}


/*
 * @implemented
 */
int WINAPI
ToUnicodeEx(UINT wVirtKey,
	    UINT wScanCode,
	    CONST BYTE *lpKeyState,
	    LPWSTR pwszBuff,
	    int cchBuff,
	    UINT wFlags,
	    HKL dwhkl)
{
  return NtUserToUnicodeEx( wVirtKey, wScanCode, (PBYTE)lpKeyState, pwszBuff, cchBuff,
			    wFlags, dwhkl );
}



/*
 * @implemented
 */
SHORT WINAPI
VkKeyScanA(CHAR ch)
{
  WCHAR wChar;

  if (IsDBCSLeadByte(ch)) return -1;

  MultiByteToWideChar(CP_ACP, 0, &ch, 1, &wChar, 1);
  return VkKeyScanW(wChar);
}


/*
 * @implemented
 */
SHORT WINAPI
VkKeyScanExA(CHAR ch,
	     HKL dwhkl)
{
  WCHAR wChar;

  if (IsDBCSLeadByte(ch)) return -1;

  MultiByteToWideChar(CP_ACP, 0, &ch, 1, &wChar, 1);
  return VkKeyScanExW(wChar, dwhkl);
}


/*
 * @implemented
 */
SHORT WINAPI
VkKeyScanExW(WCHAR ch,
	     HKL dwhkl)
{
  return (SHORT) NtUserVkKeyScanEx(ch, dwhkl, TRUE);
}


/*
 * @implemented
 */
SHORT WINAPI
VkKeyScanW(WCHAR ch)
{
  return (SHORT) NtUserVkKeyScanEx(ch, 0, FALSE);
}


/*
 * @implemented
 */
VOID
WINAPI
keybd_event(
	    BYTE bVk,
	    BYTE bScan,
	    DWORD dwFlags,
	    ULONG_PTR dwExtraInfo)


{
  INPUT Input;

  Input.type = INPUT_KEYBOARD;
  Input.ki.wVk = bVk;
  Input.ki.wScan = bScan;
  Input.ki.dwFlags = dwFlags;
  Input.ki.time = 0;
  Input.ki.dwExtraInfo = dwExtraInfo;

  NtUserSendInput(1, &Input, sizeof(INPUT));
}


/*
 * @implemented
 */
VOID
WINAPI
mouse_event(
	    DWORD dwFlags,
	    DWORD dx,
	    DWORD dy,
	    DWORD dwData,
	    ULONG_PTR dwExtraInfo)
{
  INPUT Input;

  Input.type = INPUT_MOUSE;
  Input.mi.dx = dx;
  Input.mi.dy = dy;
  Input.mi.mouseData = dwData;
  Input.mi.dwFlags = dwFlags;
  Input.mi.time = 0;
  Input.mi.dwExtraInfo = dwExtraInfo;

  NtUserSendInput(1, &Input, sizeof(INPUT));
}

/* EOF */
