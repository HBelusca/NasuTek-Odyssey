#define COBJMACROS
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <cpl.h>
#include <tchar.h>
#include <setupapi.h>
#include <stdio.h>
#include <shlobj.h>
#include <regstr.h>
#include <cplext.h>
#include <dll/desk/deskcplx.h>
#include <cfgmgr32.h>

#include "theme.h"
#include "appearance.h"
#include "preview.h"
#include "draw.h"
#include "monslctl.h"

#include "resource.h"

typedef struct _APPLET
{
    int idIcon;
    int idName;
    int idDescription;
    APPLET_PROC AppletProc;
} APPLET, *PAPPLET;

typedef struct _DIBITMAP
{
    BITMAPFILEHEADER *header;
    BITMAPINFO       *info;
    BYTE             *bits;
    int               width;
    int               height;
} DIBITMAP, *PDIBITMAP;

extern HINSTANCE hApplet;

HMENU
LoadPopupMenu(IN HINSTANCE hInstance,
              IN LPCTSTR lpMenuName);

PDIBITMAP DibLoadImage(LPTSTR lpFilename);
VOID DibFreeImage(PDIBITMAP lpBitmap);

INT AllocAndLoadString(LPTSTR *lpTarget,
                       HINSTANCE hInst,
                       UINT uID);

ULONG __cdecl DbgPrint(PCCH Format,...);

#define MAX_DESK_PAGES        32
#define NUM_SPECTRUM_BITMAPS  3

/* As slider control can't contain user data, we have to keep an
 * array of RESOLUTION_INFO to have our own associated data.
 */
typedef struct _RESOLUTION_INFO
{
	DWORD dmPelsWidth;
	DWORD dmPelsHeight;
} RESOLUTION_INFO, *PRESOLUTION_INFO;

typedef struct _SETTINGS_ENTRY
{
	struct _SETTINGS_ENTRY *Blink;
	struct _SETTINGS_ENTRY *Flink;
	DWORD dmBitsPerPel;
	DWORD dmPelsWidth;
	DWORD dmPelsHeight;
	DWORD dmDisplayFrequency;
} SETTINGS_ENTRY, *PSETTINGS_ENTRY;

typedef struct _DISPLAY_DEVICE_ENTRY
{
	struct _DISPLAY_DEVICE_ENTRY *Flink;
	LPTSTR DeviceDescription;
	LPTSTR DeviceName;
	LPTSTR DeviceKey;
	LPTSTR DeviceID;
	DWORD DeviceStateFlags;
	PSETTINGS_ENTRY Settings; /* sorted by increasing dmPelsHeight, BPP */
	DWORD SettingsCount;
	PRESOLUTION_INFO Resolutions;
	DWORD ResolutionsCount;
	PSETTINGS_ENTRY CurrentSettings; /* Points into Settings list */
	SETTINGS_ENTRY InitialSettings;
} DISPLAY_DEVICE_ENTRY, *PDISPLAY_DEVICE_ENTRY;

typedef struct _GLOBAL_DATA
{
	COLORREF desktop_color;
} GLOBAL_DATA, *PGLOBAL_DATA;

extern GLOBAL_DATA g_GlobalData;

BOOL
DisplayAdvancedSettings(HWND hWndParent, PDISPLAY_DEVICE_ENTRY DisplayDevice);

IDataObject *
CreateDevSettings(PDISPLAY_DEVICE_ENTRY DisplayDeviceInfo);

HPSXA WINAPI SHCreatePropSheetExtArrayEx(HKEY,LPCWSTR,UINT,IDataObject*);

INT_PTR CALLBACK
AdvGeneralPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
