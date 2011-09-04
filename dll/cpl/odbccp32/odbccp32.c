/* $Id: odbccp32.c 26263 2007-04-04 18:43:24Z janderwald $
 *
 * PROJECT:         Odyssey ODBC Control Panel Applet
 * FILE:            lib/cpl/main/main.c
 * PURPOSE:         applet initialization
 * PROGRAMMER:      Johannes Anderwald
 */

#include "odbccp32.h"

HINSTANCE hApplet = NULL;
APPLET_PROC ODBCProc = NULL;
HMODULE hLibrary = NULL;


LONG
CALLBACK
CPlApplet(HWND hwndCpl,
		  UINT uMsg,
		  LPARAM lParam1,
		  LPARAM lParam2)
{
	if (ODBCProc == NULL)
	{
		TCHAR szBuffer[MAX_PATH];

		if (ExpandEnvironmentStrings(_T("%systemroot%\\system32\\odbccp32.dll"),
									 szBuffer,
									 sizeof(szBuffer) / sizeof(TCHAR)) > 0)
		{
			hLibrary = LoadLibrary(szBuffer);
			if (hLibrary)
			{
				ODBCProc = (APPLET_PROC)GetProcAddress(hLibrary, "ODBCCPlApplet");
			}
		}
	}

	if (ODBCProc)
	{
		return ODBCProc(hwndCpl, uMsg, lParam1, lParam2);
	}
	else
	{
		if(hLibrary)
		{
			FreeLibrary(hLibrary);
		}

		TerminateProcess(GetCurrentProcess(), -1);
		return (LONG)-1;
	}
}


BOOL
WINAPI
DllMain(HINSTANCE hinstDLL,
		DWORD dwReason,
		LPVOID lpReserved)
{
	INITCOMMONCONTROLSEX InitControls;
	UNREFERENCED_PARAMETER(lpReserved);

	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:

			InitControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
			InitControls.dwICC = ICC_LISTVIEW_CLASSES | ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;
			InitCommonControlsEx(&InitControls);

			hApplet = hinstDLL;
			break;
  }

  return TRUE;
}

