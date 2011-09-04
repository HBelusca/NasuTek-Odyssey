/* $Id: guiconsole.c 51481 2011-04-28 19:59:16Z gschneider $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            subsys/csrss/win32csr/guiconsole.c
 * PURPOSE:         Implementation of gui-mode consoles
 */

/* INCLUDES ******************************************************************/

#define NDEBUG
#include "w32csr.h"
#include <debug.h>

/* Not defined in any header file */
extern VOID WINAPI PrivateCsrssManualGuiCheck(LONG Check);

/* GLOBALS *******************************************************************/

typedef struct GUI_CONSOLE_DATA_TAG
{
    HFONT Font;
    unsigned CharWidth;
    unsigned CharHeight;
    BOOL CursorBlinkOn;
    BOOL ForceCursorOff;
    CRITICAL_SECTION Lock;
    HMODULE ConsoleLibrary;
    HANDLE hGuiInitEvent;
    WCHAR FontName[LF_FACESIZE];
    DWORD FontSize;
    DWORD FontWeight;
    DWORD FullScreen;
    DWORD QuickEdit;
    DWORD InsertMode;
    DWORD WindowPosition;
    DWORD UseRasterFonts;
    COLORREF ScreenText;
    COLORREF ScreenBackground;
    COLORREF PopupBackground;
    COLORREF PopupText;
    COLORREF Colors[16];
    WCHAR szProcessName[MAX_PATH];
    BOOL WindowSizeLock;
    POINT OldCursor;
} GUI_CONSOLE_DATA, *PGUI_CONSOLE_DATA;

#ifndef WM_APP
#define WM_APP 0x8000
#endif
#define PM_CREATE_CONSOLE  (WM_APP + 1)
#define PM_DESTROY_CONSOLE (WM_APP + 2)

#define CURSOR_BLINK_TIME 500
#define DEFAULT_ATTRIB (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

static BOOL ConsInitialized = FALSE;
static HWND NotifyWnd;

typedef struct _GUICONSOLE_MENUITEM
{
    UINT uID;
    const struct _GUICONSOLE_MENUITEM *SubMenu;
    WORD wCmdID;
} GUICONSOLE_MENUITEM, *PGUICONSOLE_MENUITEM;

static const GUICONSOLE_MENUITEM GuiConsoleEditMenuItems[] =
{
    { IDS_MARK, NULL, ID_SYSTEM_EDIT_MARK },
    { IDS_COPY, NULL, ID_SYSTEM_EDIT_COPY },
    { IDS_PASTE, NULL, ID_SYSTEM_EDIT_PASTE },
    { IDS_SELECTALL, NULL, ID_SYSTEM_EDIT_SELECTALL },
    { IDS_SCROLL, NULL, ID_SYSTEM_EDIT_SCROLL },
    { IDS_FIND, NULL, ID_SYSTEM_EDIT_FIND },

    { 0, NULL, 0 } /* End of list */
};

static const GUICONSOLE_MENUITEM GuiConsoleMainMenuItems[] =
{
    { (UINT)-1, NULL, 0 }, /* Separator */
    { IDS_EDIT, GuiConsoleEditMenuItems, 0 },
    { IDS_DEFAULTS, NULL, ID_SYSTEM_DEFAULTS },
    { IDS_PROPERTIES, NULL, ID_SYSTEM_PROPERTIES },

    { 0, NULL, 0 } /* End of list */
};

static const COLORREF s_Colors[] =
{
    RGB(0, 0, 0),
    RGB(0, 0, 128),
    RGB(0, 128, 0),
    RGB(0, 128, 128),
    RGB(128, 0, 0),
    RGB(128, 0, 128),
    RGB(128, 128, 0),
    RGB(192, 192, 192),
    RGB(128, 128, 128),
    RGB(0, 0, 255),
    RGB(0, 255, 0),
    RGB(0, 255, 255),
    RGB(255, 0, 0),
    RGB(255, 0, 255),
    RGB(255, 255, 0),
    RGB(255, 255, 255)
};

#define GuiConsoleRGBFromAttribute(GuiData, Attribute) ((GuiData)->Colors[(Attribute) & 0xF])

/* FUNCTIONS *****************************************************************/

static VOID
GuiConsoleAppendMenuItems(HMENU hMenu,
                          const GUICONSOLE_MENUITEM *Items)
{
    UINT i = 0;
    WCHAR szMenuString[255];
    HMENU hSubMenu;
    HINSTANCE hInst = GetModuleHandleW(L"win32csr");

    do
    {
        if (Items[i].uID != (UINT)-1)
        {
            if (LoadStringW(hInst,
                            Items[i].uID,
                            szMenuString,
                            sizeof(szMenuString) / sizeof(szMenuString[0])) > 0)
            {
                if (Items[i].SubMenu != NULL)
                {
                    hSubMenu = CreatePopupMenu();
                    if (hSubMenu != NULL)
                    {
                        GuiConsoleAppendMenuItems(hSubMenu,
                                                  Items[i].SubMenu);

                        if (!AppendMenuW(hMenu,
                                         MF_STRING | MF_POPUP,
                                         (UINT_PTR)hSubMenu,
                                         szMenuString))
                        {
                            DestroyMenu(hSubMenu);
                        }
                    }
                }
                else
                {
                    AppendMenuW(hMenu,
                                MF_STRING,
                                Items[i].wCmdID,
                                szMenuString);
                }
            }
        }
        else
        {
            AppendMenuW(hMenu,
                        MF_SEPARATOR,
                        0,
                        NULL);
        }
        i++;
    } while(!(Items[i].uID == 0 && Items[i].SubMenu == NULL && Items[i].wCmdID == 0));
}

static VOID
GuiConsoleCreateSysMenu(PCSRSS_CONSOLE Console)
{
    HMENU hMenu;
    hMenu = GetSystemMenu(Console->hWindow,
                          FALSE);
    if (hMenu != NULL)
    {
        GuiConsoleAppendMenuItems(hMenu,
                                  GuiConsoleMainMenuItems);
        DrawMenuBar(Console->hWindow);
    }
}

static VOID
GuiConsoleGetDataPointers(HWND hWnd, PCSRSS_CONSOLE *Console, PGUI_CONSOLE_DATA *GuiData)
{
    *Console = (PCSRSS_CONSOLE) GetWindowLongPtrW(hWnd, GWL_USERDATA);
    *GuiData = (NULL == *Console ? NULL : (*Console)->PrivateData);
}

static BOOL
GuiConsoleOpenUserRegistryPathPerProcessId(DWORD ProcessId, PHANDLE hProcHandle, PHKEY hResult, REGSAM samDesired)
{
    HANDLE hProcessToken = NULL;
    HANDLE hProcess;

    BYTE Buffer[256];
    DWORD Length = 0;
    UNICODE_STRING SidName;
    LONG res;
    PTOKEN_USER TokUser;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | READ_CONTROL, FALSE, ProcessId);
    if (!hProcess)
    {
        DPRINT("Error: OpenProcess failed(0x%x)\n", GetLastError());
        return FALSE;
    }

    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hProcessToken))
    {
        DPRINT("Error: OpenProcessToken failed(0x%x)\n", GetLastError());
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!GetTokenInformation(hProcessToken, TokenUser, (PVOID)Buffer, sizeof(Buffer), &Length))
    {
        DPRINT("Error: GetTokenInformation failed(0x%x)\n",GetLastError());
        CloseHandle(hProcess);
        CloseHandle(hProcessToken);
        return FALSE;
    }

    TokUser = ((PTOKEN_USER)Buffer)->User.Sid;
    if (!NT_SUCCESS(RtlConvertSidToUnicodeString(&SidName, TokUser, TRUE)))
    {
        DPRINT("Error: RtlConvertSidToUnicodeString failed(0x%x)\n", GetLastError());
        return FALSE;
    }

    res = RegOpenKeyExW(HKEY_USERS, SidName.Buffer, 0, samDesired, hResult);
    RtlFreeUnicodeString(&SidName);

    CloseHandle(hProcessToken);
    if (hProcHandle)
        *hProcHandle = hProcess;
    else
        CloseHandle(hProcess);

    if (res != ERROR_SUCCESS)
        return FALSE;
    else
        return TRUE;
}

static BOOL
GuiConsoleOpenUserSettings(PGUI_CONSOLE_DATA GuiData, DWORD ProcessId, PHKEY hSubKey, REGSAM samDesired, BOOL bCreate)
{
    WCHAR szProcessName[MAX_PATH];
    WCHAR szBuffer[MAX_PATH];
    UINT fLength, wLength;
    DWORD dwBitmask, dwLength;
    WCHAR CurDrive[] = { 'A',':', 0 };
    HANDLE hProcess;
    HKEY hKey;
    WCHAR * ptr;

    /*
     * console properties are stored under
     * HKCU\Console\*
     *
     * There are 3 ways to store console properties
     *
     *  1. use console title as subkey name
     *    i.e. cmd.exe
     *
     *  2. use application name as subkey name
     *
     *  3. use unexpanded path to console application.
     *     i.e. %SystemRoot%_system32_cmd.exe
     */

    DPRINT("GuiConsoleOpenUserSettings entered\n");

    if (!GuiConsoleOpenUserRegistryPathPerProcessId(ProcessId, &hProcess, &hKey, samDesired))
    {
        DPRINT("GuiConsoleOpenUserRegistryPathPerProcessId failed\n");
        return FALSE;
    }

    /* FIXME we do not getting the process name so no menu will be loading, why ?*/
    fLength = GetProcessImageFileNameW(hProcess, szProcessName, sizeof(GuiData->szProcessName) / sizeof(WCHAR));
    CloseHandle(hProcess);

    //DPRINT1("szProcessName3 : %S\n",szProcessName);

    if (!fLength)
    {
        DPRINT("GetProcessImageFileNameW failed(0x%x)ProcessId %d\n", GetLastError(),hProcess);
        return FALSE;
    }
    /*
     * try the process name as path
     */

    ptr = wcsrchr(szProcessName, L'\\');
    wcscpy(GuiData->szProcessName, ptr);

    swprintf(szBuffer, L"Console%s",ptr);
    DPRINT("#1 Path : %S\n", szBuffer);

    if (bCreate)
    {
        if (RegCreateKeyW(hKey, szBuffer, hSubKey) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return TRUE;
        }
        RegCloseKey(hKey);
        return FALSE;
    }

    if (RegOpenKeyExW(hKey, szBuffer, 0, samDesired, hSubKey) == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return TRUE;
    }

    /*
     * try the "Shortcut to processname" as path
     * FIXME: detect wheter the process was started as a shortcut
     */

    swprintf(szBuffer, L"Console\\Shortcut to %S", ptr);
    DPRINT("#2 Path : %S\n", szBuffer);
    if (RegOpenKeyExW(hKey, szBuffer, 0, samDesired, hSubKey) == ERROR_SUCCESS)
    {
        swprintf(GuiData->szProcessName, L"Shortcut to %S", ptr);
        RegCloseKey(hKey);
        return TRUE;
    }

    /*
     * if the path contains \\Device\\HarddiskVolume1\... remove it
     */

    if (szProcessName[0] == L'\\')
    {
        dwBitmask = GetLogicalDrives();
        while(dwBitmask)
        {
            if (dwBitmask & 0x1)
            {
                dwLength = QueryDosDeviceW(CurDrive, szBuffer, MAX_PATH);
                if (dwLength)
                {
                    if (!memcmp(szBuffer, szProcessName, (dwLength-2)*sizeof(WCHAR)))
                    {
                        wcscpy(szProcessName, CurDrive);
                        RtlMoveMemory(&szProcessName[2], &szProcessName[dwLength-1], fLength - dwLength -1);
                        break;
                    }
                }
            }
            dwBitmask = (dwBitmask >> 1);
            CurDrive[0]++;
        }
    }

    /*
     * last attempt: check whether the file is under %SystemRoot%
     * and use path like Console\%SystemRoot%_dir_dir2_file.exe
     */

    wLength = GetWindowsDirectoryW(szBuffer, MAX_PATH);
    if (wLength)
    {
        if (!wcsncmp(szProcessName, szBuffer, wLength))
        {
            /* replace slashes by underscores */
            while((ptr = wcschr(szProcessName, L'\\')))
                ptr[0] = L'_';

            swprintf(szBuffer, L"Console\\%%SystemRoot%%%S", &szProcessName[wLength]);
            DPRINT("#3 Path : %S\n", szBuffer);
            if (RegOpenKeyExW(hKey, szBuffer, 0, samDesired, hSubKey) == ERROR_SUCCESS)
            {
                swprintf(GuiData->szProcessName, L"%%SystemRoot%%%S", &szProcessName[wLength]);
                RegCloseKey(hKey);
                return TRUE;
            }
        }
    }
    RegCloseKey(hKey);
    return FALSE;
}

static VOID
GuiConsoleWriteUserSettings(PCSRSS_CONSOLE Console, PGUI_CONSOLE_DATA GuiData)
{
    HKEY hKey;
    PCSRSS_PROCESS_DATA ProcessData;

    if (Console->ProcessList.Flink == &Console->ProcessList)
    {
        DPRINT("GuiConsoleWriteUserSettings: No Process!!!\n");
        return;
    }
    ProcessData = CONTAINING_RECORD(Console->ProcessList.Flink, CSRSS_PROCESS_DATA, ProcessEntry);
    if (!GuiConsoleOpenUserSettings(GuiData, PtrToUlong(ProcessData->ProcessId), &hKey, KEY_READ | KEY_WRITE, TRUE))
    {
        return;
    }

    if (Console->ActiveBuffer->CursorInfo.dwSize <= 1)
    {
        RegDeleteKeyW(hKey, L"CursorSize");
    }
    else
    {
        RegSetValueExW(hKey, L"CursorSize", 0, REG_DWORD, (const BYTE *)&Console->ActiveBuffer->CursorInfo.dwSize, sizeof(DWORD));
    }

    if (Console->NumberOfHistoryBuffers == 5)
    {
        RegDeleteKeyW(hKey, L"NumberOfHistoryBuffers");
    }
    else
    {
        DWORD Temp = Console->NumberOfHistoryBuffers;
        RegSetValueExW(hKey, L"NumberOfHistoryBuffers", 0, REG_DWORD, (const BYTE *)&Temp, sizeof(DWORD));
    }

    if (Console->HistoryBufferSize == 50)
    {
        RegDeleteKeyW(hKey, L"HistoryBufferSize");
    }
    else
    {
        DWORD Temp = Console->HistoryBufferSize;
        RegSetValueExW(hKey, L"HistoryBufferSize", 0, REG_DWORD, (const BYTE *)&Temp, sizeof(DWORD));
    }

    if (GuiData->FullScreen == FALSE)
    {
        RegDeleteKeyW(hKey, L"FullScreen");
    }
    else
    {
        RegSetValueExW(hKey, L"FullScreen", 0, REG_DWORD, (const BYTE *)&GuiData->FullScreen, sizeof(DWORD));
    }

    if ( GuiData->QuickEdit == FALSE)
    {
        RegDeleteKeyW(hKey, L"QuickEdit");
    }
    else
    {
        RegSetValueExW(hKey, L"QuickEdit", 0, REG_DWORD, (const BYTE *)&GuiData->QuickEdit, sizeof(DWORD));
    }

    if (GuiData->InsertMode == TRUE)
    {
        RegDeleteKeyW(hKey, L"InsertMode");
    }
    else
    {
        RegSetValueExW(hKey, L"InsertMode", 0, REG_DWORD, (const BYTE *)&GuiData->InsertMode, sizeof(DWORD));
    }

    if (Console->HistoryNoDup == FALSE)
    {
        RegDeleteKeyW(hKey, L"HistoryNoDup");
    }
    else
    {
        DWORD Temp = Console->HistoryNoDup;
        RegSetValueExW(hKey, L"HistoryNoDup", 0, REG_DWORD, (const BYTE *)&Temp, sizeof(DWORD));
    }

    if (GuiData->ScreenText == RGB(192, 192, 192))
    {
        /*
         * MS uses console attributes instead of real color
         */
        RegDeleteKeyW(hKey, L"ScreenText");
    }
    else
    {
        RegSetValueExW(hKey, L"ScreenText", 0, REG_DWORD, (const BYTE *)&GuiData->ScreenText, sizeof(COLORREF));
    }

    if (GuiData->ScreenBackground == RGB(0, 0, 0))
    {
        RegDeleteKeyW(hKey, L"ScreenBackground");
    }
    else
    {
        RegSetValueExW(hKey, L"ScreenBackground", 0, REG_DWORD, (const BYTE *)&GuiData->ScreenBackground, sizeof(COLORREF));
    }

    RegCloseKey(hKey);
}

static void
GuiConsoleReadUserSettings(HKEY hKey, PCSRSS_CONSOLE Console, PGUI_CONSOLE_DATA GuiData, PCSRSS_SCREEN_BUFFER Buffer)
{
    DWORD dwNumSubKeys = 0;
    DWORD dwIndex;
    DWORD dwValueName;
    DWORD dwValue;
    DWORD dwType;
    WCHAR szValueName[MAX_PATH];
    WCHAR szValue[LF_FACESIZE] = L"\0";
    DWORD Value;

    if (RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &dwNumSubKeys, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
    {
        DPRINT("GuiConsoleReadUserSettings: RegQueryInfoKey failed\n");
        return;
    }

    DPRINT("GuiConsoleReadUserSettings entered dwNumSubKeys %d\n", dwNumSubKeys);

    for (dwIndex = 0; dwIndex < dwNumSubKeys; dwIndex++)
    {
        dwValue = sizeof(Value);
        dwValueName = MAX_PATH;

        if (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueName, NULL, &dwType, (BYTE*)&Value, &dwValue) != ERROR_SUCCESS)
        {
            if (dwType == REG_SZ)
            {
                /*
                 * retry in case of string value
                 */
                dwValue = sizeof(szValue);
                dwValueName = LF_FACESIZE;
                if (RegEnumValueW(hKey, dwIndex, szValueName, &dwValueName, NULL, NULL, (BYTE*)szValue, &dwValue) != ERROR_SUCCESS)
                    break;
            }
            else
                break;
        }
        if (!wcscmp(szValueName, L"CursorSize"))
        {
            if (Value == 0x32)
            {
                Buffer->CursorInfo.dwSize = Value;
            }
            else if (Value == 0x64)
            {
                Buffer->CursorInfo.dwSize = Value;
            }
        }
        else if (!wcscmp(szValueName, L"ScreenText"))
        {
            GuiData->ScreenText = Value;
        }
        else if (!wcscmp(szValueName, L"ScreenBackground"))
        {
            GuiData->ScreenBackground = Value;
        }
        else if (!wcscmp(szValueName, L"FaceName"))
        {
            wcscpy(GuiData->FontName, szValue);
        }
        else if (!wcscmp(szValueName, L"FontSize"))
        {
            GuiData->FontSize = Value;
        }
        else if (!wcscmp(szValueName, L"FontWeight"))
        {
            GuiData->FontWeight = Value;
        }
        else if (!wcscmp(szValueName, L"HistoryNoDup"))
        {
            Console->HistoryNoDup = Value;
        }
        else if (!wcscmp(szValueName, L"WindowSize"))
        {
            Console->Size.X = LOWORD(Value);
            Console->Size.Y = HIWORD(Value);
        }
        else if (!wcscmp(szValueName, L"ScreenBufferSize"))
        {
            if(Buffer)
            {
                Buffer->MaxX = LOWORD(Value);
                Buffer->MaxY = HIWORD(Value);
            }
        }
        else if (!wcscmp(szValueName, L"FullScreen"))
        {
            GuiData->FullScreen = Value;
        }
        else if (!wcscmp(szValueName, L"QuickEdit"))
        {
            GuiData->QuickEdit = Value;
        }
        else if (!wcscmp(szValueName, L"InsertMode"))
        {
            GuiData->InsertMode = Value;
        }
    }
}
static VOID
GuiConsoleUseDefaults(PCSRSS_CONSOLE Console, PGUI_CONSOLE_DATA GuiData, PCSRSS_SCREEN_BUFFER Buffer)
{
    /*
     * init guidata with default properties
     */

    wcscpy(GuiData->FontName, L"DejaVu Sans Mono");
    GuiData->FontSize = 0x0008000C; // font is 8x12
    GuiData->FontWeight = FW_NORMAL;
    GuiData->FullScreen = FALSE;
    GuiData->QuickEdit = FALSE;
    GuiData->InsertMode = TRUE;
    GuiData->ScreenText = RGB(192, 192, 192);
    GuiData->ScreenBackground = RGB(0, 0, 0);
    GuiData->PopupText = RGB(128, 0, 128);
    GuiData->PopupBackground = RGB(255, 255, 255);
    GuiData->WindowPosition = UINT_MAX;
    GuiData->UseRasterFonts = TRUE;
    memcpy(GuiData->Colors, s_Colors, sizeof(s_Colors));

    Console->HistoryBufferSize = 50;
    Console->NumberOfHistoryBuffers = 5;
    Console->HistoryNoDup = FALSE;
    Console->Size.X = 80;
    Console->Size.Y = 25;

    if (Buffer)
    {
        Buffer->MaxX = 80;
        Buffer->MaxY = 300;
        Buffer->CursorInfo.bVisible = TRUE;
        Buffer->CursorInfo.dwSize = CSR_DEFAULT_CURSOR_SIZE;
    }
}

VOID
FASTCALL
GuiConsoleInitScrollbar(PCSRSS_CONSOLE Console, HWND hwnd)
{
    SCROLLINFO sInfo;
    PGUI_CONSOLE_DATA GuiData = Console->PrivateData;

    DWORD Width = Console->Size.X * GuiData->CharWidth + 2 * (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE));
    DWORD Height = Console->Size.Y * GuiData->CharHeight + 2 * (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION);

    /* set scrollbar sizes */
    sInfo.cbSize = sizeof(SCROLLINFO);
    sInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    sInfo.nMin = 0;
    if (Console->ActiveBuffer->MaxY > Console->Size.Y)
    {
        sInfo.nMax = Console->ActiveBuffer->MaxY - 1;
        sInfo.nPage = Console->Size.Y;
        sInfo.nPos = Console->ActiveBuffer->ShowY;
        SetScrollInfo(hwnd, SB_VERT, &sInfo, TRUE);
        Width += GetSystemMetrics(SM_CXVSCROLL);
        ShowScrollBar(hwnd, SB_VERT, TRUE);
    }
    else
    {
        ShowScrollBar(hwnd, SB_VERT, FALSE);
    }

    if (Console->ActiveBuffer->MaxX > Console->Size.X)
    {
        sInfo.nMax = Console->ActiveBuffer->MaxX - 1;
        sInfo.nPage = Console->Size.X;
        sInfo.nPos = Console->ActiveBuffer->ShowX;
        SetScrollInfo(hwnd, SB_HORZ, &sInfo, TRUE);
        Height += GetSystemMetrics(SM_CYHSCROLL);
        ShowScrollBar(hwnd, SB_HORZ, TRUE);

    }
    else
    {
        ShowScrollBar(hwnd, SB_HORZ, FALSE);
    }

    SetWindowPos(hwnd, NULL, 0, 0, Width, Height,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
}

static BOOL
GuiConsoleHandleNcCreate(HWND hWnd, CREATESTRUCTW *Create)
{
    PCSRSS_CONSOLE Console = (PCSRSS_CONSOLE) Create->lpCreateParams;
    PGUI_CONSOLE_DATA GuiData = (PGUI_CONSOLE_DATA)Console->PrivateData;
    HDC Dc;
    HFONT OldFont;
    TEXTMETRICW Metrics;
    SIZE CharSize;
    PCSRSS_PROCESS_DATA ProcessData;
    HKEY hKey;

    Console->hWindow = hWnd;

    if (NULL == GuiData)
    {
        DPRINT1("GuiConsoleNcCreate: HeapAlloc failed\n");
        return FALSE;
    }

    GuiConsoleUseDefaults(Console, GuiData, Console->ActiveBuffer);
    if (Console->ProcessList.Flink != &Console->ProcessList)
    {
        ProcessData = CONTAINING_RECORD(Console->ProcessList.Flink, CSRSS_PROCESS_DATA, ProcessEntry);
        if (GuiConsoleOpenUserSettings(GuiData, PtrToUlong(ProcessData->ProcessId), &hKey, KEY_READ, FALSE))
        {
            GuiConsoleReadUserSettings(hKey, Console, GuiData, Console->ActiveBuffer);
            RegCloseKey(hKey);
        }
    }

    InitializeCriticalSection(&GuiData->Lock);

    GuiData->Font = CreateFontW(LOWORD(GuiData->FontSize),
                                0, //HIWORD(GuiData->FontSize),
                                0,
                                TA_BASELINE,
                                GuiData->FontWeight,
                                FALSE,
                                FALSE,
                                FALSE,
                                OEM_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                NONANTIALIASED_QUALITY, FIXED_PITCH | FF_DONTCARE,
                                GuiData->FontName);
    if (NULL == GuiData->Font)
    {
        DPRINT1("GuiConsoleNcCreate: CreateFont failed\n");
        DeleteCriticalSection(&GuiData->Lock);
        HeapFree(Win32CsrApiHeap, 0, GuiData);
        return FALSE;
    }
    Dc = GetDC(hWnd);
    if (NULL == Dc)
    {
        DPRINT1("GuiConsoleNcCreate: GetDC failed\n");
        DeleteObject(GuiData->Font);
        DeleteCriticalSection(&GuiData->Lock);
        HeapFree(Win32CsrApiHeap, 0, GuiData);
        return FALSE;
    }
    OldFont = SelectObject(Dc, GuiData->Font);
    if (NULL == OldFont)
    {
        DPRINT1("GuiConsoleNcCreate: SelectObject failed\n");
        ReleaseDC(hWnd, Dc);
        DeleteObject(GuiData->Font);
        DeleteCriticalSection(&GuiData->Lock);
        HeapFree(Win32CsrApiHeap, 0, GuiData);
        return FALSE;
    }
    if (! GetTextMetricsW(Dc, &Metrics))
    {
        DPRINT1("GuiConsoleNcCreate: GetTextMetrics failed\n");
        SelectObject(Dc, OldFont);
        ReleaseDC(hWnd, Dc);
        DeleteObject(GuiData->Font);
        DeleteCriticalSection(&GuiData->Lock);
        HeapFree(Win32CsrApiHeap, 0, GuiData);
        return FALSE;
    }
    GuiData->CharWidth = Metrics.tmMaxCharWidth;
    GuiData->CharHeight = Metrics.tmHeight + Metrics.tmExternalLeading;

    /* Measure real char width more precisely if possible. */
    if (GetTextExtentPoint32W(Dc, L"R", 1, &CharSize))
        GuiData->CharWidth = CharSize.cx;

    SelectObject(Dc, OldFont);

    ReleaseDC(hWnd, Dc);
    GuiData->CursorBlinkOn = TRUE;
    GuiData->ForceCursorOff = FALSE;

    DPRINT("Console %p GuiData %p\n", Console, GuiData);
    Console->PrivateData = GuiData;
    SetWindowLongPtrW(hWnd, GWL_USERDATA, (DWORD_PTR) Console);

    SetTimer(hWnd, CONGUI_UPDATE_TIMER, CONGUI_UPDATE_TIME, NULL);
    GuiConsoleCreateSysMenu(Console);

    GuiData->WindowSizeLock = TRUE;
    GuiConsoleInitScrollbar(Console, hWnd);
    GuiData->WindowSizeLock = FALSE;

    SetEvent(GuiData->hGuiInitEvent);

    return (BOOL) DefWindowProcW(hWnd, WM_NCCREATE, 0, (LPARAM) Create);
}

static VOID
SmallRectToRect(PCSRSS_CONSOLE Console, PRECT Rect, PSMALL_RECT SmallRect)
{
    PCSRSS_SCREEN_BUFFER Buffer = Console->ActiveBuffer;
    PGUI_CONSOLE_DATA GuiData = Console->PrivateData;
    Rect->left   = (SmallRect->Left       - Buffer->ShowX) * GuiData->CharWidth;
    Rect->top    = (SmallRect->Top        - Buffer->ShowY) * GuiData->CharHeight;
    Rect->right  = (SmallRect->Right  + 1 - Buffer->ShowX) * GuiData->CharWidth;
    Rect->bottom = (SmallRect->Bottom + 1 - Buffer->ShowY) * GuiData->CharHeight;
}

static VOID
GuiConsoleUpdateSelection(PCSRSS_CONSOLE Console, PCOORD coord)
{
    RECT oldRect, newRect;
    HWND hWnd = Console->hWindow;

    SmallRectToRect(Console, &oldRect, &Console->Selection.srSelection);

    if(coord != NULL)
    {
        SMALL_RECT rc;
        /* exchange left/top with right/bottom if required */
        rc.Left   = min(Console->Selection.dwSelectionAnchor.X, coord->X);
        rc.Top    = min(Console->Selection.dwSelectionAnchor.Y, coord->Y);
        rc.Right  = max(Console->Selection.dwSelectionAnchor.X, coord->X);
        rc.Bottom = max(Console->Selection.dwSelectionAnchor.Y, coord->Y);

        SmallRectToRect(Console, &newRect, &rc);

        if (Console->Selection.dwFlags & CONSOLE_SELECTION_NOT_EMPTY)
        {
            if (memcmp(&rc, &Console->Selection.srSelection, sizeof(SMALL_RECT)) != 0)
            {
                HRGN rgn1, rgn2;

                /* calculate the region that needs to be updated */
                if((rgn1 = CreateRectRgnIndirect(&oldRect)))
                {
                    if((rgn2 = CreateRectRgnIndirect(&newRect)))
                    {
                        if(CombineRgn(rgn1, rgn2, rgn1, RGN_XOR) != ERROR)
                        {
                            InvalidateRgn(hWnd, rgn1, FALSE);
                        }

                        DeleteObject(rgn2);
                    }
                    DeleteObject(rgn1);
                }
            }
        }
        else
        {
            InvalidateRect(hWnd, &newRect, FALSE);
        }
        Console->Selection.dwFlags |= CONSOLE_SELECTION_NOT_EMPTY;
        Console->Selection.srSelection = rc;
        ConioPause(Console, PAUSED_FROM_SELECTION);
    }
    else
    {
        /* clear the selection */
        if (Console->Selection.dwFlags & CONSOLE_SELECTION_NOT_EMPTY)
        {
            InvalidateRect(hWnd, &oldRect, FALSE);
        }
        Console->Selection.dwFlags = CONSOLE_NO_SELECTION;
        ConioUnpause(Console, PAUSED_FROM_SELECTION);
    }
}


static VOID
GuiConsolePaint(PCSRSS_CONSOLE Console,
                PGUI_CONSOLE_DATA GuiData,
                HDC hDC,
                PRECT rc)
{
    PCSRSS_SCREEN_BUFFER Buff;
    ULONG TopLine, BottomLine, LeftChar, RightChar;
    ULONG Line, Char, Start;
    PBYTE From;
    PWCHAR To;
    BYTE LastAttribute, Attribute;
    ULONG CursorX, CursorY, CursorHeight;
    HBRUSH CursorBrush, OldBrush;
    HFONT OldFont;

    Buff = Console->ActiveBuffer;

    EnterCriticalSection(&Buff->Header.Console->Lock);

    TopLine = rc->top / GuiData->CharHeight + Buff->ShowY;
    BottomLine = (rc->bottom + (GuiData->CharHeight - 1)) / GuiData->CharHeight - 1 + Buff->ShowY;
    LeftChar = rc->left / GuiData->CharWidth + Buff->ShowX;
    RightChar = (rc->right + (GuiData->CharWidth - 1)) / GuiData->CharWidth - 1 + Buff->ShowX;
    LastAttribute = ConioCoordToPointer(Buff, LeftChar, TopLine)[1];

    SetTextColor(hDC, GuiConsoleRGBFromAttribute(GuiData, LastAttribute));
    SetBkColor(hDC, GuiConsoleRGBFromAttribute(GuiData, LastAttribute >> 4));

    if (BottomLine >= Buff->MaxY) BottomLine = Buff->MaxY - 1;
    if (RightChar >= Buff->MaxX) RightChar = Buff->MaxX - 1;

    OldFont = SelectObject(hDC,
                           GuiData->Font);

    for (Line = TopLine; Line <= BottomLine; Line++)
    {
        WCHAR LineBuffer[80];
        From = ConioCoordToPointer(Buff, LeftChar, Line);
        Start = LeftChar;
        To = LineBuffer;

        for (Char = LeftChar; Char <= RightChar; Char++)
        {
            if (*(From + 1) != LastAttribute || (Char - Start == sizeof(LineBuffer) / sizeof(WCHAR)))
            {
                TextOutW(hDC,
                         (Start - Buff->ShowX) * GuiData->CharWidth,
                         (Line - Buff->ShowY) * GuiData->CharHeight,
                         LineBuffer,
                         Char - Start);
                Start = Char;
                To = LineBuffer;
                Attribute = *(From + 1);
                if (Attribute != LastAttribute)
                {
                    SetTextColor(hDC, GuiConsoleRGBFromAttribute(GuiData, Attribute));
                    SetBkColor(hDC, GuiConsoleRGBFromAttribute(GuiData, Attribute >> 4));
                    LastAttribute = Attribute;
                }
            }

            MultiByteToWideChar(Console->OutputCodePage,
                                0,
                                (PCHAR)From,
                                1,
                                To,
                                1);
            To++;
            From += 2;
        }

        TextOutW(hDC,
                 (Start - Buff->ShowX) * GuiData->CharWidth,
                 (Line - Buff->ShowY) * GuiData->CharHeight,
                 LineBuffer,
                 RightChar - Start + 1);
    }

    if (Buff->CursorInfo.bVisible && GuiData->CursorBlinkOn &&
            !GuiData->ForceCursorOff)
    {
        CursorX = Buff->CurrentX;
        CursorY = Buff->CurrentY;
        if (LeftChar <= CursorX && CursorX <= RightChar &&
                TopLine <= CursorY && CursorY <= BottomLine)
        {
            CursorHeight = ConioEffectiveCursorSize(Console, GuiData->CharHeight);
            From = ConioCoordToPointer(Buff, Buff->CurrentX, Buff->CurrentY) + 1;

            if (*From != DEFAULT_ATTRIB)
            {
                CursorBrush = CreateSolidBrush(GuiConsoleRGBFromAttribute(GuiData, *From));
            }
            else
            {
                CursorBrush = CreateSolidBrush(GuiData->ScreenText);
            }

            OldBrush = SelectObject(hDC,
                                    CursorBrush);
            PatBlt(hDC,
                   (CursorX - Buff->ShowX) * GuiData->CharWidth,
                   (CursorY - Buff->ShowY) * GuiData->CharHeight + (GuiData->CharHeight - CursorHeight),
                   GuiData->CharWidth,
                   CursorHeight,
                   PATCOPY);
            SelectObject(hDC,
                         OldBrush);
            DeleteObject(CursorBrush);
        }
    }

    LeaveCriticalSection(&Buff->Header.Console->Lock);

    SelectObject(hDC,
                 OldFont);
}

static VOID
GuiConsoleHandlePaint(HWND hWnd, HDC hDCPaint)
{
    HDC hDC;
    PAINTSTRUCT ps;
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;

    hDC = BeginPaint(hWnd, &ps);
    if (hDC != NULL &&
            ps.rcPaint.left < ps.rcPaint.right &&
            ps.rcPaint.top < ps.rcPaint.bottom)
    {
        GuiConsoleGetDataPointers(hWnd,
                                  &Console,
                                  &GuiData);
        if (Console != NULL && GuiData != NULL &&
                Console->ActiveBuffer != NULL)
        {
            if (Console->ActiveBuffer->Buffer != NULL)
            {
                EnterCriticalSection(&GuiData->Lock);

                GuiConsolePaint(Console,
                                GuiData,
                                hDC,
                                &ps.rcPaint);

                if (Console->Selection.dwFlags & CONSOLE_SELECTION_NOT_EMPTY)
                {
                    RECT rc;
                    SmallRectToRect(Console, &rc, &Console->Selection.srSelection);

                    /* invert the selection */
                    if (IntersectRect(&rc,
                                      &ps.rcPaint,
                                      &rc))
                    {
                        PatBlt(hDC,
                               rc.left,
                               rc.top,
                               rc.right - rc.left,
                               rc.bottom - rc.top,
                               DSTINVERT);
                    }
                }

                LeaveCriticalSection(&GuiData->Lock);
            }
        }

    }
    EndPaint(hWnd, &ps);
}

static VOID
GuiConsoleHandleKey(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    MSG Message;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    Message.hwnd = hWnd;
    Message.message = msg;
    Message.wParam = wParam;
    Message.lParam = lParam;

    if(msg == WM_CHAR || msg == WM_SYSKEYDOWN)
    {
        /* clear the selection */
        GuiConsoleUpdateSelection(Console, NULL);
    }

    ConioProcessKey(&Message, Console, FALSE);
}

static VOID WINAPI
GuiDrawRegion(PCSRSS_CONSOLE Console, SMALL_RECT *Region)
{
    RECT RegionRect;
    SmallRectToRect(Console, &RegionRect, Region);
    InvalidateRect(Console->hWindow, &RegionRect, FALSE);
}

static VOID
GuiInvalidateCell(PCSRSS_CONSOLE Console, UINT x, UINT y)
{
    SMALL_RECT CellRect = { x, y, x, y };
    GuiDrawRegion(Console, &CellRect);
}

static VOID WINAPI
GuiWriteStream(PCSRSS_CONSOLE Console, SMALL_RECT *Region, LONG CursorStartX, LONG CursorStartY,
               UINT ScrolledLines, CHAR *Buffer, UINT Length)
{
    PGUI_CONSOLE_DATA GuiData = (PGUI_CONSOLE_DATA) Console->PrivateData;
    PCSRSS_SCREEN_BUFFER Buff = Console->ActiveBuffer;
    LONG CursorEndX, CursorEndY;
    RECT ScrollRect;

    if (NULL == Console->hWindow || NULL == GuiData)
    {
        return;
    }

    if (0 != ScrolledLines)
    {
        ScrollRect.left = 0;
        ScrollRect.top = 0;
        ScrollRect.right = Console->Size.X * GuiData->CharWidth;
        ScrollRect.bottom = Region->Top * GuiData->CharHeight;

        ScrollWindowEx(Console->hWindow,
                       0,
                       -(ScrolledLines * GuiData->CharHeight),
                       &ScrollRect,
                       NULL,
                       NULL,
                       NULL,
                       SW_INVALIDATE);
    }

    GuiDrawRegion(Console, Region);

    if (CursorStartX < Region->Left || Region->Right < CursorStartX
            || CursorStartY < Region->Top || Region->Bottom < CursorStartY)
    {
        GuiInvalidateCell(Console, CursorStartX, CursorStartY);
    }

    CursorEndX = Buff->CurrentX;
    CursorEndY = Buff->CurrentY;
    if ((CursorEndX < Region->Left || Region->Right < CursorEndX
            || CursorEndY < Region->Top || Region->Bottom < CursorEndY)
            && (CursorEndX != CursorStartX || CursorEndY != CursorStartY))
    {
        GuiInvalidateCell(Console, CursorEndX, CursorEndY);
    }

    // Set up the update timer (very short interval) - this is a "hack" for getting the OS to
    // repaint the window without having it just freeze up and stay on the screen permanently.
    GuiData->CursorBlinkOn = TRUE;
    SetTimer(Console->hWindow, CONGUI_UPDATE_TIMER, CONGUI_UPDATE_TIME, NULL);
}

static BOOL WINAPI
GuiSetCursorInfo(PCSRSS_CONSOLE Console, PCSRSS_SCREEN_BUFFER Buff)
{
    if (Console->ActiveBuffer == Buff)
    {
        GuiInvalidateCell(Console, Buff->CurrentX, Buff->CurrentY);
    }

    return TRUE;
}

static BOOL WINAPI
GuiSetScreenInfo(PCSRSS_CONSOLE Console, PCSRSS_SCREEN_BUFFER Buff, UINT OldCursorX, UINT OldCursorY)
{
    if (Console->ActiveBuffer == Buff)
    {
        /* Redraw char at old position (removes cursor) */
        GuiInvalidateCell(Console, OldCursorX, OldCursorY);
        /* Redraw char at new position (shows cursor) */
        GuiInvalidateCell(Console, Buff->CurrentX, Buff->CurrentY);
    }

    return TRUE;
}

static BOOL WINAPI
GuiUpdateScreenInfo(PCSRSS_CONSOLE Console, PCSRSS_SCREEN_BUFFER Buff)
{
    PGUI_CONSOLE_DATA GuiData = (PGUI_CONSOLE_DATA) Console->PrivateData;

    if (Console->ActiveBuffer == Buff)
    {
        GuiData->ScreenText = GuiConsoleRGBFromAttribute(GuiData, Buff->DefaultAttrib);
        GuiData->ScreenBackground = GuiConsoleRGBFromAttribute(GuiData, Buff->DefaultAttrib >> 4);
    }

    return TRUE;
}

static VOID
GuiConsoleHandleTimer(HWND hWnd)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    PCSRSS_SCREEN_BUFFER Buff;

    SetTimer(hWnd, CONGUI_UPDATE_TIMER, CURSOR_BLINK_TIME, NULL);

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);

    Buff = Console->ActiveBuffer;
    GuiInvalidateCell(Console, Buff->CurrentX, Buff->CurrentY);
    GuiData->CursorBlinkOn = ! GuiData->CursorBlinkOn;

    if((GuiData->OldCursor.x != Buff->CurrentX) || (GuiData->OldCursor.y != Buff->CurrentY))
    {
        SCROLLINFO xScroll;
        int OldScrollX = -1, OldScrollY = -1;
        int NewScrollX = -1, NewScrollY = -1;

        xScroll.cbSize = sizeof(SCROLLINFO);
        xScroll.fMask = SIF_POS;
        // Capture the original position of the scroll bars and save them.
        if(GetScrollInfo(hWnd, SB_HORZ, &xScroll))OldScrollX = xScroll.nPos;
        if(GetScrollInfo(hWnd, SB_VERT, &xScroll))OldScrollY = xScroll.nPos;

        // If we successfully got the info for the horizontal scrollbar
        if(OldScrollX >= 0)
        {
            if((Buff->CurrentX < Buff->ShowX)||(Buff->CurrentX >= (Buff->ShowX + Console->Size.X)))
            {
                // Handle the horizontal scroll bar
                if(Buff->CurrentX >= Console->Size.X) NewScrollX = Buff->CurrentX - Console->Size.X + 1;
                else NewScrollX = 0;
            }
            else
            {
                NewScrollX = OldScrollX;
            }
        }
        // If we successfully got the info for the vertical scrollbar
        if(OldScrollY >= 0)
        {
            if((Buff->CurrentY < Buff->ShowY) || (Buff->CurrentY >= (Buff->ShowY + Console->Size.Y)))
            {
                // Handle the vertical scroll bar
                if(Buff->CurrentY >= Console->Size.Y) NewScrollY = Buff->CurrentY - Console->Size.Y + 1;
                else NewScrollY = 0;
            }
            else
            {
                NewScrollY = OldScrollY;
            }
        }

        // Adjust scroll bars and refresh the window if the cursor has moved outside the visible area
        // NOTE: OldScroll# and NewScroll# will both be -1 (initial value) if the info for the respective scrollbar
        //       was not obtained successfully in the previous steps. This means their difference is 0 (no scrolling)
        //       and their associated scrollbar is left alone.
        if((OldScrollX != NewScrollX) || (OldScrollY != NewScrollY))
        {
            Buff->ShowX = NewScrollX;
            Buff->ShowY = NewScrollY;
            ScrollWindowEx(hWnd,
                           (OldScrollX - NewScrollX) * GuiData->CharWidth,
                           (OldScrollY - NewScrollY) * GuiData->CharHeight,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           SW_INVALIDATE);
            if(NewScrollX >= 0)
            {
                xScroll.nPos = NewScrollX;
                SetScrollInfo(hWnd, SB_HORZ, &xScroll, TRUE);
            }
            if(NewScrollY >= 0)
            {
                xScroll.nPos = NewScrollY;
                SetScrollInfo(hWnd, SB_VERT, &xScroll, TRUE);
            }
            UpdateWindow(hWnd);
            GuiData->OldCursor.x = Buff->CurrentX;
            GuiData->OldCursor.y = Buff->CurrentY;
        }
    }
}

static VOID
GuiConsoleHandleClose(HWND hWnd)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    PLIST_ENTRY current_entry;
    PCSRSS_PROCESS_DATA current;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);

    EnterCriticalSection(&Console->Lock);

    current_entry = Console->ProcessList.Flink;
    while (current_entry != &Console->ProcessList)
    {
        current = CONTAINING_RECORD(current_entry, CSRSS_PROCESS_DATA, ProcessEntry);
        current_entry = current_entry->Flink;

        /* FIXME: Windows will wait up to 5 seconds for the thread to exit.
         * We shouldn't wait here, though, since the console lock is entered.
         * A copy of the thread list probably needs to be made. */
        ConioConsoleCtrlEvent(CTRL_CLOSE_EVENT, current);
    }

    LeaveCriticalSection(&Console->Lock);
}

static VOID
GuiConsoleHandleNcDestroy(HWND hWnd)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;


    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    KillTimer(hWnd, 1);
    Console->PrivateData = NULL;
    DeleteCriticalSection(&GuiData->Lock);
    GetSystemMenu(hWnd, TRUE);
    if (GuiData->ConsoleLibrary)
        FreeLibrary(GuiData->ConsoleLibrary);

    HeapFree(Win32CsrApiHeap, 0, GuiData);
}

static COORD
PointToCoord(PCSRSS_CONSOLE Console, LPARAM lParam)
{
    PCSRSS_SCREEN_BUFFER Buffer = Console->ActiveBuffer;
    PGUI_CONSOLE_DATA GuiData = Console->PrivateData;
    COORD Coord;
    Coord.X = Buffer->ShowX + ((short)LOWORD(lParam) / (int)GuiData->CharWidth);
    Coord.Y = Buffer->ShowY + ((short)HIWORD(lParam) / (int)GuiData->CharHeight);

    /* Clip coordinate to ensure it's inside buffer */
    if (Coord.X < 0)                  Coord.X = 0;
    else if (Coord.X >= Buffer->MaxX) Coord.X = Buffer->MaxX - 1;
    if (Coord.Y < 0)                  Coord.Y = 0;
    else if (Coord.Y >= Buffer->MaxY) Coord.Y = Buffer->MaxY - 1;
    return Coord;
}

static VOID
GuiConsoleLeftMouseDown(HWND hWnd, LPARAM lParam)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    if (Console == NULL || GuiData == NULL) return;

    Console->Selection.dwSelectionAnchor = PointToCoord(Console, lParam);

    SetCapture(hWnd);

    Console->Selection.dwFlags |= CONSOLE_SELECTION_IN_PROGRESS | CONSOLE_MOUSE_SELECTION | CONSOLE_MOUSE_DOWN;

    GuiConsoleUpdateSelection(Console, &Console->Selection.dwSelectionAnchor);
}

static VOID
GuiConsoleLeftMouseUp(HWND hWnd, LPARAM lParam)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    COORD c;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    if (Console == NULL || GuiData == NULL) return;
    if (!(Console->Selection.dwFlags & CONSOLE_MOUSE_DOWN)) return;

    c = PointToCoord(Console, lParam);

    Console->Selection.dwFlags &= ~CONSOLE_MOUSE_DOWN;

    GuiConsoleUpdateSelection(Console, &c);

    ReleaseCapture();
}

static VOID
GuiConsoleMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    COORD c;

    if (!(wParam & MK_LBUTTON)) return;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    if (Console == NULL || GuiData == NULL) return;
    if (!(Console->Selection.dwFlags & CONSOLE_MOUSE_DOWN)) return;

    c = PointToCoord(Console, lParam); /* TODO: Scroll buffer to bring c into view */

    GuiConsoleUpdateSelection(Console, &c);
}

static VOID
GuiConsoleCopy(HWND hWnd, PCSRSS_CONSOLE Console)
{
    if (OpenClipboard(hWnd) == TRUE)
    {
        HANDLE hData;
        PBYTE ptr;
        LPSTR data, dstPos;
        ULONG selWidth, selHeight;
        ULONG xPos, yPos, size;

        selWidth = Console->Selection.srSelection.Right - Console->Selection.srSelection.Left + 1;
        selHeight = Console->Selection.srSelection.Bottom - Console->Selection.srSelection.Top + 1;
        DPRINT("Selection is (%d|%d) to (%d|%d)\n",
               Console->Selection.srSelection.Left,
               Console->Selection.srSelection.Top,
               Console->Selection.srSelection.Right,
               Console->Selection.srSelection.Bottom);

        /* Basic size for one line and termination */
        size = selWidth + 1;
        if (selHeight > 0)
        {
            /* Multiple line selections have to get \r\n appended */
            size += ((selWidth + 2) * (selHeight - 1));
        }

        /* Allocate memory, it will be passed to the system and may not be freed here */
        hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size);
        if (hData == NULL)
        {
            CloseClipboard();
            return;
        }
        data = GlobalLock(hData);
        if (data == NULL)
        {
            CloseClipboard();
            return;
        }

        DPRINT("Copying %dx%d selection\n", selWidth, selHeight);
        dstPos = data;

        for (yPos = 0; yPos < selHeight; yPos++)
        {
            ptr = ConioCoordToPointer(Console->ActiveBuffer, 
                                      Console->Selection.srSelection.Left,
                                      yPos + Console->Selection.srSelection.Top);
            /* Copy only the characters, leave attributes alone */
            for (xPos = 0; xPos < selWidth; xPos++)
            {
                dstPos[xPos] = ptr[xPos * 2];
            }
            dstPos += selWidth;
            if (yPos != (selHeight - 1))
            {
                strcat(data, "\r\n");
                dstPos += 2;
            }
        }

        DPRINT("Setting data <%s> to clipboard\n", data);
        GlobalUnlock(hData);

        EmptyClipboard();
        SetClipboardData(CF_TEXT, hData);
        CloseClipboard();
    }
}

static VOID
GuiConsolePaste(HWND hWnd, PCSRSS_CONSOLE Console)
{
    if (OpenClipboard(hWnd) == TRUE)
    {
        HANDLE hData;
        LPSTR str;
        size_t len;

        hData = GetClipboardData(CF_TEXT);
        if (hData == NULL)
        {
            CloseClipboard();
            return;
        }

        str = GlobalLock(hData);
        if (str == NULL)
        {
            CloseClipboard();
            return;
        }
        DPRINT("Got data <%s> from clipboard\n", str);
        len = strlen(str);

        ConioWriteConsole(Console, Console->ActiveBuffer, str, len, TRUE);

        GlobalUnlock(hData);
        CloseClipboard();
    }
}

static VOID
GuiConsoleRightMouseDown(HWND hWnd)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    if (Console == NULL || GuiData == NULL) return;

    if (!(Console->Selection.dwFlags & CONSOLE_SELECTION_NOT_EMPTY))
    {
        GuiConsolePaste(hWnd, Console);
    }
    else
    {
        GuiConsoleCopy(hWnd, Console);

        /* Clear the selection */
        GuiConsoleUpdateSelection(Console, NULL);
    }

}


static VOID
GuiConsoleShowConsoleProperties(HWND hWnd, BOOL Defaults, PGUI_CONSOLE_DATA GuiData)
{
    PCSRSS_CONSOLE Console;
    APPLET_PROC CPLFunc;
    TCHAR szBuffer[MAX_PATH];
    ConsoleInfo SharedInfo;

    DPRINT("GuiConsoleShowConsoleProperties entered\n");

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);

    if (GuiData == NULL)
    {
        DPRINT("GuiConsoleGetDataPointers failed\n");
        return;
    }

    if (GuiData->ConsoleLibrary == NULL)
    {
        GetWindowsDirectory(szBuffer,MAX_PATH);
        _tcscat(szBuffer, _T("\\system32\\console.dll"));
        GuiData->ConsoleLibrary = LoadLibrary(szBuffer);

        if (GuiData->ConsoleLibrary == NULL)
        {
            DPRINT1("failed to load console.dll");
            return;
        }
    }

    CPLFunc = (APPLET_PROC) GetProcAddress(GuiData->ConsoleLibrary, _T("CPlApplet"));
    if (!CPLFunc)
    {
        DPRINT("Error: Console.dll misses CPlApplet export\n");
        return;
    }

    /* setup struct */
    SharedInfo.InsertMode = GuiData->InsertMode;
    SharedInfo.HistoryBufferSize = Console->HistoryBufferSize;
    SharedInfo.NumberOfHistoryBuffers = Console->NumberOfHistoryBuffers;
    SharedInfo.ScreenText = GuiData->ScreenText;
    SharedInfo.ScreenBackground = GuiData->ScreenBackground;
    SharedInfo.PopupText = GuiData->PopupText;
    SharedInfo.PopupBackground = GuiData->PopupBackground;
    SharedInfo.WindowSize = (DWORD)MAKELONG(Console->Size.X, Console->Size.Y);
    SharedInfo.WindowPosition = GuiData->WindowPosition;
    SharedInfo.ScreenBuffer = (DWORD)MAKELONG(Console->ActiveBuffer->MaxX, Console->ActiveBuffer->MaxY);
    SharedInfo.UseRasterFonts = GuiData->UseRasterFonts;
    SharedInfo.FontSize = (DWORD)GuiData->FontSize;
    SharedInfo.FontWeight = GuiData->FontWeight;
    SharedInfo.CursorSize = Console->ActiveBuffer->CursorInfo.dwSize;
    SharedInfo.HistoryNoDup = Console->HistoryNoDup;
    SharedInfo.FullScreen = GuiData->FullScreen;
    SharedInfo.QuickEdit = GuiData->QuickEdit;
    memcpy(&SharedInfo.Colors[0], GuiData->Colors, sizeof(s_Colors));

    if (!CPLFunc(hWnd, CPL_INIT, 0, 0))
    {
        DPRINT("Error: failed to initialize console.dll\n");
        return;
    }

    if (CPLFunc(hWnd, CPL_GETCOUNT, 0, 0) != 1)
    {
        DPRINT("Error: console.dll returned unexpected CPL count\n");
        return;
    }

    CPLFunc(hWnd, CPL_DBLCLK, (LPARAM)&SharedInfo, Defaults);
}
static LRESULT
GuiConsoleHandleSysMenuCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    LRESULT Ret = TRUE;
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    COORD bottomRight = { 0, 0 };

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);

    switch(wParam)
    {
    case ID_SYSTEM_EDIT_MARK:
        DPRINT1("Marking not handled yet\n");
        break;

    case ID_SYSTEM_EDIT_COPY:
        GuiConsoleCopy(hWnd, Console);
        break;

    case ID_SYSTEM_EDIT_PASTE:
        GuiConsolePaste(hWnd, Console);
        break;

    case ID_SYSTEM_EDIT_SELECTALL:
        bottomRight.X = Console->Size.X - 1;
        bottomRight.Y = Console->Size.Y - 1;
        GuiConsoleUpdateSelection(Console, &bottomRight);
        break;

    case ID_SYSTEM_EDIT_SCROLL:
        DPRINT1("Scrolling is not handled yet\n");
        break;

    case ID_SYSTEM_EDIT_FIND:
        DPRINT1("Finding is not handled yet\n");
        break;

    case ID_SYSTEM_DEFAULTS:
        GuiConsoleShowConsoleProperties(hWnd, TRUE, GuiData);
        break;

    case ID_SYSTEM_PROPERTIES:
        GuiConsoleShowConsoleProperties(hWnd, FALSE, GuiData);
        break;

    default:
        Ret = DefWindowProcW(hWnd, WM_SYSCOMMAND, wParam, lParam);
        break;
    }
    return Ret;
}

static VOID
GuiConsoleGetMinMaxInfo(HWND hWnd, PMINMAXINFO minMaxInfo)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    DWORD windx, windy;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    if((Console == NULL)|| (GuiData == NULL)) return;

    windx = CONGUI_MIN_WIDTH * GuiData->CharWidth + 2 * (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE));
    windy = CONGUI_MIN_HEIGHT * GuiData->CharHeight + 2 * (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION);

    minMaxInfo->ptMinTrackSize.x = windx;
    minMaxInfo->ptMinTrackSize.y = windy;

    windx = (Console->ActiveBuffer->MaxX) * GuiData->CharWidth + 2 * (GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXEDGE));
    windy = (Console->ActiveBuffer->MaxY) * GuiData->CharHeight + 2 * (GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CYEDGE)) + GetSystemMetrics(SM_CYCAPTION);

    if(Console->Size.X < Console->ActiveBuffer->MaxX) windy += GetSystemMetrics(SM_CYHSCROLL);    // window currently has a horizontal scrollbar
    if(Console->Size.Y < Console->ActiveBuffer->MaxY) windx += GetSystemMetrics(SM_CXVSCROLL);    // window currently has a vertical scrollbar

    minMaxInfo->ptMaxTrackSize.x = windx;
    minMaxInfo->ptMaxTrackSize.y = windy;
}
static VOID
GuiConsoleResize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    PCSRSS_CONSOLE Console;
    PGUI_CONSOLE_DATA GuiData;
    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);
    if((Console == NULL) || (GuiData == NULL)) return;

    if ((GuiData->WindowSizeLock == FALSE) && (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED || wParam == SIZE_MINIMIZED))
    {
        PCSRSS_SCREEN_BUFFER Buff = Console->ActiveBuffer;
        DWORD windx, windy, charx, chary;

        GuiData->WindowSizeLock = TRUE;

        windx = LOWORD(lParam);
        windy = HIWORD(lParam);

        // Compensate for existing scroll bars (because lParam values do not accommodate scroll bar)
        if(Console->Size.X < Buff->MaxX) windy += GetSystemMetrics(SM_CYHSCROLL);    // window currently has a horizontal scrollbar
        if(Console->Size.Y < Buff->MaxY) windx += GetSystemMetrics(SM_CXVSCROLL);    // window currently has a vertical scrollbar

        charx = windx / GuiData->CharWidth;
        chary = windy / GuiData->CharHeight;

        // Character alignment (round size up or down)
        if((windx % GuiData->CharWidth) >= (GuiData->CharWidth / 2)) ++charx;
        if((windy % GuiData->CharHeight) >= (GuiData->CharHeight / 2)) ++chary;

        // Compensate for added scroll bars in new window
        if(charx < Buff->MaxX)windy -= GetSystemMetrics(SM_CYHSCROLL);    // new window will have a horizontal scroll bar
        if(chary < Buff->MaxY)windx -= GetSystemMetrics(SM_CXVSCROLL);    // new window will have a vertical scroll bar

        charx = windx / GuiData->CharWidth;
        chary = windy / GuiData->CharHeight;

        // Character alignment (round size up or down)
        if((windx % GuiData->CharWidth) >= (GuiData->CharWidth / 2)) ++charx;
        if((windy % GuiData->CharHeight) >= (GuiData->CharHeight / 2)) ++chary;

        // Resize window
        if((charx != Console->Size.X) || (chary != Console->Size.Y))
        {
            Console->Size.X = (charx <= Buff->MaxX) ? charx : Buff->MaxX;
            Console->Size.Y = (chary <= Buff->MaxY) ? chary : Buff->MaxY;
        }

        GuiConsoleInitScrollbar(Console, hWnd);

        // Adjust the start of the visible area if we are attempting to show nonexistent areas
        if((Buff->MaxX - Buff->ShowX) < Console->Size.X) Buff->ShowX = Buff->MaxX - Console->Size.X;
        if((Buff->MaxY - Buff->ShowY) < Console->Size.Y) Buff->ShowY = Buff->MaxY - Console->Size.Y;
        InvalidateRect(hWnd, NULL, TRUE);

        GuiData->WindowSizeLock = FALSE;
    }
}

VOID
FASTCALL
GuiConsoleHandleScrollbarMenu()
{
    HMENU hMenu;

    hMenu = CreatePopupMenu();
    if (hMenu == NULL)
    {
        DPRINT("CreatePopupMenu failed\n");
        return;
    }
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLHERE);
    //InsertItem(hMenu, MFT_SEPARATOR, MIIM_FTYPE, 0, NULL, -1);
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLTOP);
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLBOTTOM);
    //InsertItem(hMenu, MFT_SEPARATOR, MIIM_FTYPE, 0, NULL, -1);
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLPAGE_UP);
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLPAGE_DOWN);
    //InsertItem(hMenu, MFT_SEPARATOR, MIIM_FTYPE, 0, NULL, -1);
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLUP);
    //InsertItem(hMenu, MIIM_STRING, MIIM_ID | MIIM_FTYPE | MIIM_STRING, 0, NULL, IDS_SCROLLDOWN);

}

static NTSTATUS WINAPI
GuiResizeBuffer(PCSRSS_CONSOLE Console, PCSRSS_SCREEN_BUFFER ScreenBuffer, COORD Size)
{
    BYTE * Buffer;
    DWORD Offset = 0;
    BYTE * OldPtr;
    USHORT CurrentY;
    BYTE * OldBuffer;
#if HAVE_WMEMSET
    USHORT value = MAKEWORD(' ', ScreenBuffer->DefaultAttrib);
#endif
    DWORD diff;
    DWORD i;

    /* Buffer size is not allowed to be smaller than window size */
    if (Size.X < Console->Size.X || Size.Y < Console->Size.Y)
        return STATUS_INVALID_PARAMETER;

    if (Size.X == ScreenBuffer->MaxX && Size.Y == ScreenBuffer->MaxY)
        return STATUS_SUCCESS;

    Buffer = HeapAlloc(Win32CsrApiHeap, 0, Size.X * Size.Y * 2);
    if (!Buffer)
        return STATUS_NO_MEMORY;

    DPRINT1("Resizing (%d,%d) to (%d,%d)\n", ScreenBuffer->MaxX, ScreenBuffer->MaxY, Size.X, Size.Y);
    OldBuffer = ScreenBuffer->Buffer;

    for (CurrentY = 0; CurrentY < ScreenBuffer->MaxY && CurrentY < Size.Y; CurrentY++)
    {
        OldPtr = ConioCoordToPointer(ScreenBuffer, 0, CurrentY);
        if (Size.X <= ScreenBuffer->MaxX)
        {
            /* reduce size */
            RtlCopyMemory(&Buffer[Offset], OldPtr, Size.X * 2);
            Offset += (Size.X * 2);
        }
        else
        {
            /* enlarge size */
            RtlCopyMemory(&Buffer[Offset], OldPtr, ScreenBuffer->MaxX * 2);
            Offset += (ScreenBuffer->MaxX * 2);

            diff = Size.X - ScreenBuffer->MaxX;
            /* zero new part of it */
#if HAVE_WMEMSET
            wmemset((WCHAR*)&Buffer[Offset], value, diff);
#else
            for (i = 0; i < diff; i++)
            {
                Buffer[Offset++] = ' ';
                Buffer[Offset++] = ScreenBuffer->DefaultAttrib;
            }
#endif
        }
    }

    if (Size.Y > ScreenBuffer->MaxY)
    {
        diff = Size.X * (Size.Y - ScreenBuffer->MaxY);
#if HAVE_WMEMSET
        wmemset((WCHAR*)&Buffer[Offset], value, diff);
#else
        for (i = 0; i < diff; i++)
        {
            Buffer[Offset++] = ' ';
            Buffer[Offset++] = ScreenBuffer->DefaultAttrib;
        }
#endif
    }

    (void)InterlockedExchangePointer((PVOID volatile  *)&ScreenBuffer->Buffer, Buffer);
    HeapFree(Win32CsrApiHeap, 0, OldBuffer);
    ScreenBuffer->MaxX = Size.X;
    ScreenBuffer->MaxY = Size.Y;
    ScreenBuffer->VirtualY = 0;

    /* Ensure cursor and window are within buffer */
    if (ScreenBuffer->CurrentX >= Size.X)
        ScreenBuffer->CurrentX = Size.X - 1;
    if (ScreenBuffer->CurrentY >= Size.Y)
        ScreenBuffer->CurrentY = Size.Y - 1;
    if (ScreenBuffer->ShowX > Size.X - Console->Size.X)
        ScreenBuffer->ShowX = Size.X - Console->Size.X;
    if (ScreenBuffer->ShowY > Size.Y - Console->Size.Y)
        ScreenBuffer->ShowY = Size.Y - Console->Size.Y;

    /* TODO: Should update scrollbar, but can't use anything that
     * calls SendMessage or it could cause deadlock */

    return STATUS_SUCCESS;
}

static VOID
GuiApplyUserSettings(PCSRSS_CONSOLE Console, PGUI_CONSOLE_DATA GuiData, PConsoleInfo pConInfo)
{
    DWORD windx, windy;
    PCSRSS_SCREEN_BUFFER ActiveBuffer = Console->ActiveBuffer;
    COORD BufSize;
    BOOL SizeChanged = FALSE;

    EnterCriticalSection(&Console->Lock);

    /* apply text / background color */
    GuiData->ScreenText = pConInfo->ScreenText;
    GuiData->ScreenBackground = pConInfo->ScreenBackground;

    /* apply cursor size */
    ActiveBuffer->CursorInfo.dwSize = min(max(pConInfo->CursorSize, 1), 100);

    windx = LOWORD(pConInfo->WindowSize);
    windy = HIWORD(pConInfo->WindowSize);

    if (windx != Console->Size.X || windy != Console->Size.Y)
    {
        /* resize window */
        Console->Size.X = windx;
        Console->Size.Y = windy;
        SizeChanged = TRUE;
    }

    BufSize.X = LOWORD(pConInfo->ScreenBuffer);
    BufSize.Y = HIWORD(pConInfo->ScreenBuffer);
    if (BufSize.X != ActiveBuffer->MaxX || BufSize.Y != ActiveBuffer->MaxY)
    {
        if (NT_SUCCESS(GuiResizeBuffer(Console, ActiveBuffer, BufSize)))
            SizeChanged = TRUE;
    }

    if (SizeChanged)
    {
        GuiData->WindowSizeLock = TRUE;
        GuiConsoleInitScrollbar(Console, pConInfo->hConsoleWindow);
        GuiData->WindowSizeLock = FALSE;
    }

    LeaveCriticalSection(&Console->Lock);
    InvalidateRect(pConInfo->hConsoleWindow, NULL, TRUE);
}

static
LRESULT
GuiConsoleHandleScroll(HWND hwnd, UINT uMsg, WPARAM wParam)
{
    PCSRSS_CONSOLE Console;
    PCSRSS_SCREEN_BUFFER Buff;
    PGUI_CONSOLE_DATA GuiData;
    SCROLLINFO sInfo;
    int fnBar;
    int old_pos, Maximum;
    PUSHORT pShowXY;

    GuiConsoleGetDataPointers(hwnd, &Console, &GuiData);
    if (Console == NULL || GuiData == NULL)
        return FALSE;
    Buff = Console->ActiveBuffer;

    if (uMsg == WM_HSCROLL)
    {
        fnBar = SB_HORZ;
        Maximum = Buff->MaxX - Console->Size.X;
        pShowXY = &Buff->ShowX;
    }
    else
    {
        fnBar = SB_VERT;
        Maximum = Buff->MaxY - Console->Size.Y;
        pShowXY = &Buff->ShowY;
    }

    /* set scrollbar sizes */
    sInfo.cbSize = sizeof(SCROLLINFO);
    sInfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE | SIF_TRACKPOS;

    if (!GetScrollInfo(hwnd, fnBar, &sInfo))
    {
        return FALSE;
    }

    old_pos = sInfo.nPos;

    switch(LOWORD(wParam))
    {
    case SB_LINELEFT:
        sInfo.nPos -= 1;
        break;

    case SB_LINERIGHT:
        sInfo.nPos += 1;
        break;

    case SB_PAGELEFT:
        sInfo.nPos -= sInfo.nPage;
        break;

    case SB_PAGERIGHT:
        sInfo.nPos += sInfo.nPage;
        break;

    case SB_THUMBTRACK:
        sInfo.nPos = sInfo.nTrackPos;
        ConioPause(Console, PAUSED_FROM_SCROLLBAR);
        break;

    case SB_THUMBPOSITION:
        ConioUnpause(Console, PAUSED_FROM_SCROLLBAR);
        break;

    case SB_TOP:
        sInfo.nPos = sInfo.nMin;
        break;

    case SB_BOTTOM:
        sInfo.nPos = sInfo.nMax;
        break;

    default:
        break;
    }

    sInfo.nPos = max(sInfo.nPos, 0);
    sInfo.nPos = min(sInfo.nPos, Maximum);

    if (old_pos != sInfo.nPos)
    {
        USHORT OldX = Buff->ShowX;
        USHORT OldY = Buff->ShowY;
        *pShowXY = sInfo.nPos;

        ScrollWindowEx(hwnd,
                       (OldX - Buff->ShowX) * GuiData->CharWidth,
                       (OldY - Buff->ShowY) * GuiData->CharHeight,
                       NULL,
                       NULL,
                       NULL,
                       NULL,
                       SW_INVALIDATE);

        sInfo.fMask = SIF_POS;
        SetScrollInfo(hwnd, fnBar, &sInfo, TRUE);

        UpdateWindow(hwnd);
    }
    return 0;
}

static LRESULT CALLBACK
GuiConsoleWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    PGUI_CONSOLE_DATA GuiData = NULL;
    PCSRSS_CONSOLE Console = NULL;

    GuiConsoleGetDataPointers(hWnd, &Console, &GuiData);

    switch(msg)
    {
    case WM_NCCREATE:
        Result = (LRESULT) GuiConsoleHandleNcCreate(hWnd, (CREATESTRUCTW *) lParam);
        break;
    case WM_PAINT:
        GuiConsoleHandlePaint(hWnd, (HDC)wParam);
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_CHAR:
        GuiConsoleHandleKey(hWnd, msg, wParam, lParam);
        break;
    case WM_TIMER:
        GuiConsoleHandleTimer(hWnd);
        break;
    case WM_CLOSE:
        GuiConsoleHandleClose(hWnd);
        break;
    case WM_NCDESTROY:
        GuiConsoleHandleNcDestroy(hWnd);
        break;
    case WM_LBUTTONDOWN:
        GuiConsoleLeftMouseDown(hWnd, lParam);
        break;
    case WM_LBUTTONUP:
        GuiConsoleLeftMouseUp(hWnd, lParam);
        break;
    case WM_RBUTTONDOWN:
        GuiConsoleRightMouseDown(hWnd);
        break;
    case WM_MOUSEMOVE:
        GuiConsoleMouseMove(hWnd, wParam, lParam);
        break;
    case WM_SYSCOMMAND:
        Result = GuiConsoleHandleSysMenuCommand(hWnd, wParam, lParam);
        break;
    case WM_HSCROLL:
    case WM_VSCROLL:
        Result = GuiConsoleHandleScroll(hWnd, msg, wParam);
        break;
    case WM_GETMINMAXINFO:
        GuiConsoleGetMinMaxInfo(hWnd, (PMINMAXINFO)lParam);
        break;
    case WM_SIZE:
        GuiConsoleResize(hWnd, wParam, lParam);
        break;
    case PM_APPLY_CONSOLE_INFO:
        GuiApplyUserSettings(Console, GuiData, (PConsoleInfo)wParam);
        if (lParam)
        {
            GuiConsoleWriteUserSettings(Console, GuiData);
        }
        break;
    default:
        Result = DefWindowProcW(hWnd, msg, wParam, lParam);
        break;
    }

    return Result;
}

static LRESULT CALLBACK
GuiConsoleNotifyWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND NewWindow;
    LONG WindowCount;
    MSG Msg;
    PWCHAR Buffer, Title;
    PCSRSS_CONSOLE Console = (PCSRSS_CONSOLE) lParam;



    switch(msg)
    {
    case WM_CREATE:
        SetWindowLongW(hWnd, GWL_USERDATA, 0);
        return 0;
    case PM_CREATE_CONSOLE:
        Buffer = HeapAlloc(Win32CsrApiHeap, 0,
                           Console->Title.Length + sizeof(WCHAR));
        if (NULL != Buffer)
        {
            memcpy(Buffer, Console->Title.Buffer, Console->Title.Length);
            Buffer[Console->Title.Length / sizeof(WCHAR)] = L'\0';
            Title = Buffer;
        }
        else
        {
            Title = L"";
        }
        NewWindow = CreateWindowExW(WS_EX_CLIENTEDGE,
                                    L"ConsoleWindowClass",
                                    Title,
                                    WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    NULL,
                                    NULL,
                                    (HINSTANCE) GetModuleHandleW(NULL),
                                    (PVOID) Console);
        if (NULL != Buffer)
        {
            HeapFree(Win32CsrApiHeap, 0, Buffer);
        }
        if (NULL != NewWindow)
        {
            SetWindowLongW(hWnd, GWL_USERDATA, GetWindowLongW(hWnd, GWL_USERDATA) + 1);
            if (wParam)
              {
                ShowWindow(NewWindow, SW_SHOW);
              }
        }
        return (LRESULT) NewWindow;
    case PM_DESTROY_CONSOLE:
        /* Window creation is done using a PostMessage(), so it's possible that the
         * window that we want to destroy doesn't exist yet. So first empty the message
         * queue */
        while(PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Msg);
            DispatchMessageW(&Msg);
        }
        DestroyWindow(Console->hWindow);
        Console->hWindow = NULL;
        WindowCount = GetWindowLongW(hWnd, GWL_USERDATA);
        WindowCount--;
        SetWindowLongW(hWnd, GWL_USERDATA, WindowCount);
        if (0 == WindowCount)
        {
            NotifyWnd = NULL;
            DestroyWindow(hWnd);
            PrivateCsrssManualGuiCheck(-1);
            PostQuitMessage(0);
        }
        return 0;
    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
}

static DWORD WINAPI
GuiConsoleGuiThread(PVOID Data)
{
    MSG msg;
    PHANDLE GraphicsStartupEvent = (PHANDLE) Data;

    NotifyWnd = CreateWindowW(L"Win32CsrCreateNotify",
                              L"",
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              NULL,
                              (HINSTANCE) GetModuleHandleW(NULL),
                              NULL);
    if (NULL == NotifyWnd)
    {
        PrivateCsrssManualGuiCheck(-1);
        SetEvent(*GraphicsStartupEvent);
        return 1;
    }

    SetEvent(*GraphicsStartupEvent);

    while(GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 1;
}

static BOOL
GuiInit(VOID)
{
    WNDCLASSEXW wc;

    if (NULL == NotifyWnd)
    {
        PrivateCsrssManualGuiCheck(+1);
    }

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpszClassName = L"Win32CsrCreateNotify";
    wc.lpfnWndProc = GuiConsoleNotifyWndProc;
    wc.style = 0;
    wc.hInstance = (HINSTANCE) GetModuleHandleW(NULL);
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIconSm = NULL;
    if (RegisterClassExW(&wc) == 0)
    {
        DPRINT1("Failed to register notify wndproc\n");
        return FALSE;
    }

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpszClassName = L"ConsoleWindowClass";
    wc.lpfnWndProc = GuiConsoleWndProc;
    wc.style = 0;
    wc.hInstance = (HINSTANCE) GetModuleHandleW(NULL);
    wc.hIcon = LoadIconW(GetModuleHandleW(L"win32csr"), MAKEINTRESOURCEW(1));
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR) IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wc.lpszMenuName = NULL;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIconSm = LoadImageW(GetModuleHandleW(L"win32csr"), MAKEINTRESOURCEW(1), IMAGE_ICON,
                            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                            LR_SHARED);
    if (RegisterClassExW(&wc) == 0)
    {
        DPRINT1("Failed to register console wndproc\n");
        return FALSE;
    }

    return TRUE;
}

static VOID WINAPI
GuiInitScreenBuffer(PCSRSS_CONSOLE Console, PCSRSS_SCREEN_BUFFER Buffer)
{
    Buffer->DefaultAttrib = DEFAULT_ATTRIB;
}

static BOOL WINAPI
GuiChangeTitle(PCSRSS_CONSOLE Console)
{
    PWCHAR Buffer, Title;

    Buffer = HeapAlloc(Win32CsrApiHeap, 0,
                       Console->Title.Length + sizeof(WCHAR));
    if (NULL != Buffer)
    {
        memcpy(Buffer, Console->Title.Buffer, Console->Title.Length);
        Buffer[Console->Title.Length / sizeof(WCHAR)] = L'\0';
        Title = Buffer;
    }
    else
    {
        Title = L"";
    }

    SendMessageW(Console->hWindow, WM_SETTEXT, 0, (LPARAM) Title);

    if (NULL != Buffer)
    {
        HeapFree(Win32CsrApiHeap, 0, Buffer);
    }

    return TRUE;
}

static BOOL WINAPI
GuiChangeIcon(PCSRSS_CONSOLE Console, HICON hWindowIcon)
{
    SendMessageW(Console->hWindow, WM_SETICON, ICON_BIG, (LPARAM)hWindowIcon);
    SendMessageW(Console->hWindow, WM_SETICON, ICON_SMALL, (LPARAM)hWindowIcon);

    return TRUE;
}

static VOID WINAPI
GuiCleanupConsole(PCSRSS_CONSOLE Console)
{
    SendMessageW(NotifyWnd, PM_DESTROY_CONSOLE, 0, (LPARAM) Console);
}

static CSRSS_CONSOLE_VTBL GuiVtbl =
{
    GuiInitScreenBuffer,
    GuiWriteStream,
    GuiDrawRegion,
    GuiSetCursorInfo,
    GuiSetScreenInfo,
    GuiUpdateScreenInfo,
    GuiChangeTitle,
    GuiCleanupConsole,
    GuiChangeIcon,
    GuiResizeBuffer,
};

NTSTATUS FASTCALL
GuiInitConsole(PCSRSS_CONSOLE Console, BOOL Visible)
{
    HANDLE GraphicsStartupEvent;
    HANDLE ThreadHandle;
    PGUI_CONSOLE_DATA GuiData;

    if (! ConsInitialized)
    {
        ConsInitialized = TRUE;
        if (! GuiInit())
        {
            ConsInitialized = FALSE;
            return STATUS_UNSUCCESSFUL;
        }
    }

    Console->Vtbl = &GuiVtbl;
    if (NULL == NotifyWnd)
    {
        GraphicsStartupEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        if (NULL == GraphicsStartupEvent)
        {
            return STATUS_UNSUCCESSFUL;
        }

        ThreadHandle = CreateThread(NULL,
                                    0,
                                    GuiConsoleGuiThread,
                                    (PVOID) &GraphicsStartupEvent,
                                    0,
                                    NULL);
        if (NULL == ThreadHandle)
        {
            NtClose(GraphicsStartupEvent);
            DPRINT1("Win32Csr: Failed to create graphics console thread. Expect problems\n");
            return STATUS_UNSUCCESSFUL;
        }
        SetThreadPriority(ThreadHandle, THREAD_PRIORITY_HIGHEST);
        CloseHandle(ThreadHandle);

        WaitForSingleObject(GraphicsStartupEvent, INFINITE);
        CloseHandle(GraphicsStartupEvent);

        if (NULL == NotifyWnd)
        {
            DPRINT1("Win32Csr: Failed to create notification window.\n");
            return STATUS_UNSUCCESSFUL;
        }
    }
    GuiData = HeapAlloc(Win32CsrApiHeap, HEAP_ZERO_MEMORY,
                        sizeof(GUI_CONSOLE_DATA));
    if (!GuiData)
    {
        DPRINT1("Win32Csr: Failed to create GUI_CONSOLE_DATA\n");
        return STATUS_UNSUCCESSFUL;
    }

    Console->PrivateData = (PVOID) GuiData;
    /*
     * we need to wait untill the GUI has been fully initialized
     * to retrieve custom settings i.e. WindowSize etc..
     * Ideally we could use SendNotifyMessage for this but its not
     * yet implemented.
     *
     */
    GuiData->hGuiInitEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    /* create console */
    PostMessageW(NotifyWnd, PM_CREATE_CONSOLE, Visible, (LPARAM) Console);

    /* wait untill initialization has finished */
    WaitForSingleObject(GuiData->hGuiInitEvent, INFINITE);
    DPRINT("received event Console %p GuiData %p X %d Y %d\n", Console, Console->PrivateData, Console->Size.X, Console->Size.Y);
    CloseHandle(GuiData->hGuiInitEvent);
    GuiData->hGuiInitEvent = NULL;

    return STATUS_SUCCESS;
}

/* EOF */
