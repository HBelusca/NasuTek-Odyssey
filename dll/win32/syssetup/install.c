/*
 *  Odyssey kernel
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
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey system libraries
 * PURPOSE:           System setup
 * FILE:              dll/win32/syssetup/install.c
 * PROGRAMER:         Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include "precomp.h"

#define NDEBUG
#include <debug.h>

DWORD WINAPI
CMP_WaitNoPendingInstallEvents(DWORD dwTimeout);

/* GLOBALS ******************************************************************/

PSID DomainSid = NULL;
PSID AdminSid = NULL;

HINF hSysSetupInf = INVALID_HANDLE_VALUE;

/* FUNCTIONS ****************************************************************/

static VOID
DebugPrint(char* fmt,...)
{
    char buffer[512];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);
    va_end(ap);

    LogItem(SYSSETUP_SEVERITY_FATAL_ERROR, L"Failed");

    strcat(buffer, "\nRebooting now!");
    MessageBoxA(NULL,
                buffer,
                "Odyssey Setup",
                MB_OK);
}


HRESULT CreateShellLink(LPCTSTR linkPath, LPCTSTR cmd, LPCTSTR arg, LPCTSTR dir, LPCTSTR iconPath, int icon_nr, LPCTSTR comment)
{
    IShellLink* psl;
    IPersistFile* ppf;
#ifndef _UNICODE
    WCHAR buffer[MAX_PATH];
#endif /* _UNICODE */

    HRESULT hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLink, (LPVOID*)&psl);

    if (SUCCEEDED(hr))
    {
        hr = psl->lpVtbl->SetPath(psl, cmd);

        if (arg)
        {
            hr = psl->lpVtbl->SetArguments(psl, arg);
        }

        if (dir)
        {
            hr = psl->lpVtbl->SetWorkingDirectory(psl, dir);
        }

        if (iconPath)
        {
            hr = psl->lpVtbl->SetIconLocation(psl, iconPath, icon_nr);
        }

        if (comment)
        {
            hr = psl->lpVtbl->SetDescription(psl, comment);
        }

        hr = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hr))
        {
#ifdef _UNICODE
            hr = ppf->lpVtbl->Save(ppf, linkPath, TRUE);
#else /* _UNICODE */
            MultiByteToWideChar(CP_ACP, 0, linkPath, -1, buffer, MAX_PATH);

            hr = ppf->lpVtbl->Save(ppf, buffer, TRUE);
#endif /* _UNICODE */

            ppf->lpVtbl->Release(ppf);
        }

        psl->lpVtbl->Release(psl);
    }

    return hr;
}


static BOOL
CreateShortcut(int csidl, LPCTSTR folder, UINT nIdName, LPCTSTR command, UINT nIdTitle, BOOL bCheckExistence)
{
    TCHAR path[MAX_PATH];
    TCHAR exeName[MAX_PATH];
    TCHAR title[256];
    TCHAR name[256];
    LPTSTR p = path;
    TCHAR szWorkingDir[MAX_PATH];
    LPTSTR lpWorkingDir = NULL;
    LPTSTR lpFilePart;
    DWORD dwLen;

    if (ExpandEnvironmentStrings(command,
                                 path,
                                 sizeof(path) / sizeof(path[0])) == 0)
    {
        _tcscpy(path,
                command);
    }

    if (bCheckExistence)
    {
        if ((_taccess(path, 0 )) == -1)
            /* Expected error, don't return FALSE */
            return TRUE;
    }

    dwLen = GetFullPathName(path,
                            sizeof(szWorkingDir) / sizeof(szWorkingDir[0]),
                            szWorkingDir,
                            &lpFilePart);
    if (dwLen != 0 && dwLen <= sizeof(szWorkingDir) / sizeof(szWorkingDir[0]))
    {
        /* Save the file name */
        _tcscpy(exeName, lpFilePart);

        if (lpFilePart != NULL)
        {
            /* We're only interested in the path. Cut the file name off.
               Also remove the trailing backslash unless the working directory
               is only going to be a drive, ie. C:\ */
            *(lpFilePart--) = _T('\0');
            if (!(lpFilePart - szWorkingDir == 2 && szWorkingDir[1] == _T(':') &&
                  szWorkingDir[2] == _T('\\')))
            {
                *lpFilePart = _T('\0');
            }
        }

        lpWorkingDir = szWorkingDir;
    }


    if (!SHGetSpecialFolderPath(0, path, csidl, TRUE))
        return FALSE;

    if (folder)
    {
        p = PathAddBackslash(p);
        _tcscpy(p, folder);
    }

    p = PathAddBackslash(p);

    if (!LoadString(hDllInstance, nIdName, name, sizeof(name)/sizeof(name[0])))
        return FALSE;
    _tcscpy(p, name);

    if (!LoadString(hDllInstance, nIdTitle, title, sizeof(title)/sizeof(title[0])))
        return FALSE;

    // FIXME: we should pass 'command' straight in here, but shell32 doesn't expand it
    return SUCCEEDED(CreateShellLink(path, exeName, _T(""), lpWorkingDir, NULL, 0, title));
}


static BOOL
CreateShortcutFolder(int csidl, UINT nID, LPTSTR name, int nameLen)
{
    TCHAR path[MAX_PATH];
    LPTSTR p;

    if (!SHGetSpecialFolderPath(0, path, csidl, TRUE))
        return FALSE;

    if (!LoadString(hDllInstance, nID, name, nameLen))
        return FALSE;

    p = PathAddBackslash(path);
    _tcscpy(p, name);

    return CreateDirectory(path, NULL) || GetLastError()==ERROR_ALREADY_EXISTS;
}


static BOOL
CreateRandomSid(
    OUT PSID *Sid)
{
    SID_IDENTIFIER_AUTHORITY SystemAuthority = {SECURITY_NT_AUTHORITY};
    LARGE_INTEGER SystemTime;
    PULONG Seed;
    NTSTATUS Status;

    NtQuerySystemTime(&SystemTime);
    Seed = &SystemTime.u.LowPart;

    Status = RtlAllocateAndInitializeSid(
        &SystemAuthority,
        4,
        SECURITY_NT_NON_UNIQUE,
        RtlUniform(Seed),
        RtlUniform(Seed),
        RtlUniform(Seed),
        SECURITY_NULL_RID,
        SECURITY_NULL_RID,
        SECURITY_NULL_RID,
        SECURITY_NULL_RID,
        Sid);
    return NT_SUCCESS(Status);
}


static VOID
AppendRidToSid(
    OUT PSID *Dst,
    IN PSID Src,
    IN ULONG NewRid)
{
    ULONG Rid[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    UCHAR RidCount;
    ULONG i;

    RidCount = *RtlSubAuthorityCountSid (Src);

    for (i = 0; i < RidCount; i++)
        Rid[i] = *RtlSubAuthoritySid (Src, i);

    if (RidCount < 8)
    {
        Rid[RidCount] = NewRid;
        RidCount++;
    }

    RtlAllocateAndInitializeSid(
        RtlIdentifierAuthoritySid(Src),
        RidCount,
        Rid[0],
        Rid[1],
        Rid[2],
        Rid[3],
        Rid[4],
        Rid[5],
        Rid[6],
        Rid[7],
        Dst);
}


static VOID
CreateTempDir(
    IN LPCWSTR VarName)
{
    WCHAR szTempDir[MAX_PATH];
    WCHAR szBuffer[MAX_PATH];
    DWORD dwLength;
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                     L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment",
                     0,
                     KEY_QUERY_VALUE,
                     &hKey))
    {
        DebugPrint("Error: %lu\n", GetLastError());
        return;
    }

    /* Get temp dir */
    dwLength = MAX_PATH * sizeof(WCHAR);
    if (RegQueryValueExW(hKey,
                        VarName,
                        NULL,
                        NULL,
                        (LPBYTE)szBuffer,
                        &dwLength))
    {
        DebugPrint("Error: %lu\n", GetLastError());
        RegCloseKey(hKey);
        return;
    }

    /* Expand it */
    if (!ExpandEnvironmentStringsW(szBuffer,
                                  szTempDir,
                                  MAX_PATH))
    {
        DebugPrint("Error: %lu\n", GetLastError());
        RegCloseKey(hKey);
        return;
    }

    /* Create profiles directory */
    if (!CreateDirectoryW(szTempDir, NULL))
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            DebugPrint("Error: %lu\n", GetLastError());
            RegCloseKey(hKey);
            return;
        }
    }

    RegCloseKey(hKey);
}


BOOL
InstallSysSetupInfDevices(VOID)
{
    INFCONTEXT InfContext;
    WCHAR LineBuffer[256];
    DWORD LineLength;

    if (!SetupFindFirstLineW(hSysSetupInf,
                            L"DeviceInfsToInstall",
                            NULL,
                            &InfContext))
    {
        return FALSE;
    }

    do
    {
        if (!SetupGetStringFieldW(&InfContext,
                                 0,
                                 LineBuffer,
                                 sizeof(LineBuffer)/sizeof(LineBuffer[0]),
                                 &LineLength))
        {
            return FALSE;
        }

        if (!SetupDiInstallClassW(NULL, LineBuffer, DI_QUIETINSTALL, NULL))
        {
            return FALSE;
        }
    }
    while (SetupFindNextLine(&InfContext, &InfContext));

    return TRUE;
}
BOOL
InstallSysSetupInfComponents(VOID)
{
    INFCONTEXT InfContext;
    WCHAR NameBuffer[256];
    WCHAR SectionBuffer[256];
    HINF hComponentInf = INVALID_HANDLE_VALUE;

    if (!SetupFindFirstLineW(hSysSetupInf,
                            L"Infs.Always",
                            NULL,
                            &InfContext))
    {
        DPRINT("No Inf.Always section found\n");
    }
    else
    {
        do
        {
            if (!SetupGetStringFieldW(&InfContext,
                                     1, // Get the component name
                                     NameBuffer,
                                     sizeof(NameBuffer)/sizeof(NameBuffer[0]),
                                     NULL))
            {
                DebugPrint("Error while trying to get component name \n");
                return FALSE;
            }

            if (!SetupGetStringFieldW(&InfContext,
                                     2, // Get the component install section
                                     SectionBuffer,
                                     sizeof(SectionBuffer)/sizeof(SectionBuffer[0]),
                                     NULL))
            {
                DebugPrint("Error while trying to get component install section \n");
                return FALSE;
            }

            DPRINT("Trying to execute install section '%S' from '%S' \n", SectionBuffer , NameBuffer);

            hComponentInf = SetupOpenInfFileW(NameBuffer,
                                              NULL,
                                              INF_STYLE_WIN4,
                                              NULL);

            if (hComponentInf == INVALID_HANDLE_VALUE)
            {
                DebugPrint("SetupOpenInfFileW() failed to open '%S' (Error: %lu)\n", NameBuffer ,GetLastError());
                return FALSE;
            }

            if (!SetupInstallFromInfSectionW(NULL,
                                            hComponentInf,
                                            SectionBuffer,
                                            SPINST_ALL,
                                            NULL,
                                            NULL,
                                            SP_COPY_NEWER,
                                            SetupDefaultQueueCallbackW,
                                            NULL,
                                            NULL,
                                            NULL))
           {
                DebugPrint("Error while trying to install : %S (Error: %lu)\n", NameBuffer, GetLastError());
                SetupCloseInfFile(hComponentInf);
                return FALSE;
           }

           SetupCloseInfFile(hComponentInf);
        }
        while (SetupFindNextLine(&InfContext, &InfContext));
    }

    return TRUE;
}

static BOOL
EnableUserModePnpManager(VOID)
{
    SC_HANDLE hSCManager = NULL;
    SC_HANDLE hService = NULL;
    BOOL ret = FALSE;

    hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hSCManager == NULL)
    {
        DPRINT1("Unable to open the service control manager.\n");
        DPRINT1("Last Error %d\n", GetLastError());
        goto cleanup;
    }

    hService = OpenServiceW(hSCManager,
                            L"PlugPlay",
                            SERVICE_CHANGE_CONFIG | SERVICE_START);
    if (hService == NULL)
    {
        DPRINT1("Unable to open PlugPlay service\n");
        goto cleanup;
    }

    ret = ChangeServiceConfigW(hService,
                               SERVICE_NO_CHANGE,
                               SERVICE_AUTO_START,
                               SERVICE_NO_CHANGE,
                               NULL, NULL, NULL,
                               NULL, NULL, NULL, NULL);
    if (!ret)
    {
        DPRINT1("Unable to change the service configuration\n");
        goto cleanup;
    }

    ret = StartServiceW(hService, 0, NULL);
    if ((!ret) && (GetLastError() != ERROR_SERVICE_ALREADY_RUNNING))
    {
        DPRINT1("Unable to start service\n");
        goto cleanup;
    }

    ret = TRUE;

cleanup:
    if (hSCManager != NULL)
        CloseServiceHandle(hSCManager);
    if (hService != NULL)
        CloseServiceHandle(hService);
    return ret;
}


static INT_PTR CALLBACK
StatusMessageWindowProc(
    IN HWND hwndDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            WCHAR szMsg[256];

            if (!LoadStringW(hDllInstance, IDS_STATUS_INSTALL_DEV, szMsg, sizeof(szMsg)/sizeof(szMsg[0])))
                return FALSE;
            SetDlgItemTextW(hwndDlg, IDC_STATUSLABEL, szMsg);
            return TRUE;
        }
    }
    return FALSE;
}


static DWORD WINAPI
ShowStatusMessageThread(
    IN LPVOID lpParameter)
{
    HWND *phWnd = (HWND *)lpParameter;
    HWND hWnd;
    MSG Msg;

    hWnd = CreateDialogParam(
        hDllInstance,
        MAKEINTRESOURCE(IDD_STATUSWINDOW_DLG),
        GetDesktopWindow(),
        StatusMessageWindowProc,
        (LPARAM)NULL);
    if (!hWnd)
        return 0;
    *phWnd = hWnd;

    ShowWindow(hWnd, SW_SHOW);

    /* Message loop for the Status window */
    while (GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

    return 0;
}

static LONG
ReadRegSzKey(
    IN HKEY hKey,
    IN LPCWSTR pszKey,
    OUT LPWSTR* pValue)
{
    LONG rc;
    DWORD dwType;
    DWORD cbData = 0;
    LPWSTR Value;

    if (!pValue)
        return ERROR_INVALID_PARAMETER;

    *pValue = NULL;
    rc = RegQueryValueExW(hKey, pszKey, NULL, &dwType, NULL, &cbData);
    if (rc != ERROR_SUCCESS)
        return rc;
    if (dwType != REG_SZ)
        return ERROR_FILE_NOT_FOUND;
    Value = HeapAlloc(GetProcessHeap(), 0, cbData + sizeof(WCHAR));
    if (!Value)
        return ERROR_NOT_ENOUGH_MEMORY;
    rc = RegQueryValueExW(hKey, pszKey, NULL, NULL, (LPBYTE)Value, &cbData);
    if (rc != ERROR_SUCCESS)
    {
        HeapFree(GetProcessHeap(), 0, Value);
        return rc;
    }
    /* NULL-terminate the string */
    Value[cbData / sizeof(WCHAR)] = '\0';

    *pValue = Value;
    return ERROR_SUCCESS;
}

static BOOL
IsConsoleBoot(VOID)
{
    HKEY ControlKey = NULL;
    LPWSTR SystemStartOptions = NULL;
    LPWSTR CurrentOption, NextOption; /* Pointers into SystemStartOptions */
    BOOL ConsoleBoot = FALSE;
    LONG rc;

    rc = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Control",
        0,
        KEY_QUERY_VALUE,
        &ControlKey);

    rc = ReadRegSzKey(ControlKey, L"SystemStartOptions", &SystemStartOptions);
    if (rc != ERROR_SUCCESS)
        goto cleanup;

    /* Check for CMDCONS in SystemStartOptions */
    CurrentOption = SystemStartOptions;
    while (CurrentOption)
    {
        NextOption = wcschr(CurrentOption, L' ');
        if (NextOption)
            *NextOption = L'\0';
        if (wcsicmp(CurrentOption, L"CONSOLE") == 0)
        {
            DPRINT("Found %S. Switching to console boot\n", CurrentOption);
            ConsoleBoot = TRUE;
            goto cleanup;
        }
        CurrentOption = NextOption ? NextOption + 1 : NULL;
    }

cleanup:
    if (ControlKey != NULL)
        RegCloseKey(ControlKey);
    HeapFree(GetProcessHeap(), 0, SystemStartOptions);
    return ConsoleBoot;
}

static BOOL
CommonInstall(VOID)
{
    HWND hWnd = NULL;

    hSysSetupInf = SetupOpenInfFileW(
        L"syssetup.inf",
        NULL,
        INF_STYLE_WIN4,
        NULL);
    if (hSysSetupInf == INVALID_HANDLE_VALUE)
    {
        DebugPrint("SetupOpenInfFileW() failed to open 'syssetup.inf' (Error: %lu)\n", GetLastError());
        return FALSE;
    }

    if (!InstallSysSetupInfDevices())
    {
        DebugPrint("InstallSysSetupInfDevices() failed!\n");
        SetupCloseInfFile(hSysSetupInf);
        return FALSE;
    }

    if(!InstallSysSetupInfComponents())
    {
        DebugPrint("InstallSysSetupInfComponents() failed!\n");
        SetupCloseInfFile(hSysSetupInf);
        return FALSE;
    }

    if (!IsConsoleBoot())
    {
        CreateThread(
            NULL,
            0,
            ShowStatusMessageThread,
            (LPVOID)&hWnd,
            0,
            NULL);
    }

    if (!EnableUserModePnpManager())
    {
       DebugPrint("EnableUserModePnpManager() failed!\n");
       SetupCloseInfFile(hSysSetupInf);
       EndDialog(hWnd, 0);
       return FALSE;
    }

    if (CMP_WaitNoPendingInstallEvents(INFINITE) != WAIT_OBJECT_0)
    {
      DebugPrint("CMP_WaitNoPendingInstallEvents() failed!\n");
      SetupCloseInfFile(hSysSetupInf);
      EndDialog(hWnd, 0);
      return FALSE;
    }

    EndDialog(hWnd, 0);
    return TRUE;
}

DWORD WINAPI
InstallLiveCD(IN HINSTANCE hInstance)
{
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL res;

    if (!CommonInstall())
        goto cleanup;
    SetupCloseInfFile(hSysSetupInf);

    /* Run the shell */
    StartupInfo.cb = sizeof(STARTUPINFOW);
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.lpTitle = NULL;
    StartupInfo.dwFlags = 0;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = 0;
    res = CreateProcessW(
        L"userinit.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &StartupInfo,
        &ProcessInformation);
    if (!res)
        goto cleanup;

    return 0;

cleanup:
    MessageBoxW(
        NULL,
        L"You can shutdown your computer, or press ENTER to reboot",
        L"Odyssey LiveCD",
        MB_OK);
    return 0;
}


static BOOL
CreateShortcuts(VOID)
{
    TCHAR szFolder[256];

    CoInitialize(NULL);

    /* Create desktop shortcuts */
    CreateShortcut(CSIDL_DESKTOP, NULL, IDS_SHORT_CMD, _T("%SystemRoot%\\system32\\cmd.exe"), IDS_CMT_CMD, TRUE);

    /* Create program startmenu shortcuts */
    CreateShortcut(CSIDL_PROGRAMS, NULL, IDS_SHORT_EXPLORER, _T("%SystemRoot%\\explorer.exe"), IDS_CMT_EXPLORER, TRUE);
    CreateShortcut(CSIDL_PROGRAMS, NULL, IDS_SHORT_DOWNLOADER, _T("%SystemRoot%\\system32\\rapps.exe"), IDS_CMT_DOWNLOADER, TRUE);

    /* Create administrative tools startmenu shortcuts */
    CreateShortcut(CSIDL_COMMON_ADMINTOOLS, NULL, IDS_SHORT_SERVICE, _T("%SystemRoot%\\system32\\servman.exe"), IDS_CMT_SERVMAN, TRUE);
    CreateShortcut(CSIDL_COMMON_ADMINTOOLS, NULL, IDS_SHORT_DEVICE, _T("%SystemRoot%\\system32\\devmgmt.exe"), IDS_CMT_DEVMGMT, TRUE);
    CreateShortcut(CSIDL_COMMON_ADMINTOOLS, NULL, IDS_SHORT_EVENTVIEW, _T("%SystemRoot%\\system32\\eventvwr.exe"), IDS_CMT_EVENTVIEW, TRUE);
    CreateShortcut(CSIDL_COMMON_ADMINTOOLS, NULL, IDS_SHORT_MSCONFIG, _T("%SystemRoot%\\system32\\msconfig.exe"), IDS_CMT_MSCONFIG, TRUE);

    /* Create and fill Accessories subfolder */
    if (CreateShortcutFolder(CSIDL_PROGRAMS, IDS_ACCESSORIES, szFolder, sizeof(szFolder)/sizeof(szFolder[0])))
    {
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_CALC, _T("%SystemRoot%\\system32\\calc.exe"), IDS_CMT_CALC, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_CMD, _T("%SystemRoot%\\system32\\cmd.exe"), IDS_CMT_CMD, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_NOTEPAD, _T("%SystemRoot%\\system32\\notepad.exe"), IDS_CMT_NOTEPAD, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_RDESKTOP, _T("%SystemRoot%\\system32\\mstsc.exe"), IDS_CMT_RDESKTOP, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_SNAP, _T("%SystemRoot%\\system32\\screenshot.exe"), IDS_CMT_SCREENSHOT, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_WORDPAD, _T("%SystemRoot%\\system32\\wordpad.exe"), IDS_CMT_WORDPAD, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_PAINT, _T("%SystemRoot%\\system32\\mspaint.exe"), IDS_CMT_PAINT, TRUE);
    }

    /* Create System Tools subfolder and fill if the exe is available */
    if (CreateShortcutFolder(CSIDL_PROGRAMS, IDS_SYS_TOOLS, szFolder, sizeof(szFolder)/sizeof(szFolder[0])))
    {
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_CHARMAP, _T("%SystemRoot%\\system32\\charmap.exe"), IDS_CMT_CHARMAP, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_KBSWITCH, _T("%SystemRoot%\\system32\\kbswitch.exe"), IDS_CMT_KBSWITCH, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_REGEDIT, _T("%SystemRoot%\\regedit.exe"), IDS_CMT_REGEDIT, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_DXDIAG, _T("%SystemRoot%\\system32\\dxdiag.exe"), IDS_CMT_DXDIAG, TRUE);
    }

    /* Create Accessibility subfolder and fill if the exe is available */
    if (CreateShortcutFolder(CSIDL_PROGRAMS, IDS_SYS_ACCESSIBILITY, szFolder, sizeof(szFolder)/sizeof(szFolder[0])))
    {
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_MAGNIFY, _T("%SystemRoot%\\system32\\magnify.exe"), IDS_CMT_MAGNIFY, TRUE);
    }

    /* Create Entertainment subfolder and fill if the exe is available */
    if (CreateShortcutFolder(CSIDL_PROGRAMS, IDS_SYS_ENTERTAINMENT, szFolder, sizeof(szFolder)/sizeof(szFolder[0])))
    {
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_MPLAY32, _T("%SystemRoot%\\system32\\mplay32.exe"), IDS_CMT_MPLAY32, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_SNDVOL32, _T("%SystemRoot%\\system32\\sndvol32.exe"), IDS_CMT_SNDVOL32, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_SNDREC32, _T("%SystemRoot%\\system32\\sndrec32.exe"), IDS_CMT_SNDREC32, TRUE);
    }

    /* Create Games subfolder and fill if the exe is available */
    if (CreateShortcutFolder(CSIDL_PROGRAMS, IDS_GAMES, szFolder, sizeof(szFolder)/sizeof(szFolder[0])))
    {
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_SOLITAIRE, _T("%SystemRoot%\\system32\\sol.exe"), IDS_CMT_SOLITAIRE, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_WINEMINE, _T("%SystemRoot%\\system32\\winmine.exe"), IDS_CMT_WINEMINE, TRUE);
        CreateShortcut(CSIDL_PROGRAMS, szFolder, IDS_SHORT_SPIDER, _T("%SystemRoot%\\system32\\spider.exe"), IDS_CMT_SPIDER, TRUE);
    }

    CoUninitialize();

    return TRUE;
}

static BOOL
SetSetupType(DWORD dwSetupType)
{
    DWORD dwError;
    HKEY hKey;

    dwError = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SYSTEM\\Setup",
        0,
        KEY_SET_VALUE,
        &hKey);
    if (dwError != ERROR_SUCCESS)
        return FALSE;

    dwError = RegSetValueExW(
        hKey,
        L"SetupType",
        0,
        REG_DWORD,
        (LPBYTE)&dwSetupType,
        sizeof(DWORD));
    RegCloseKey(hKey);
    if (dwError != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
}

DWORD WINAPI
InstallOdyssey(HINSTANCE hInstance)
{
    TCHAR szBuffer[MAX_PATH];
    DWORD LastError;
    HANDLE token;
    TOKEN_PRIVILEGES privs;
    HKEY hKey;

    InitializeSetupActionLog(FALSE);
    LogItem(SYSSETUP_SEVERITY_INFORMATION, L"Installing Odyssey");

    if (!InitializeProfiles())
    {
        DebugPrint("InitializeProfiles() failed");
        return 0;
    }

    if (!CreateShortcuts())
    {
        DebugPrint("InitializeProfiles() failed");
        return 0;
    }

    /* Initialize the Security Account Manager (SAM) */
    if (!SamInitializeSAM())
    {
        DebugPrint("SamInitializeSAM() failed!");
        return 0;
    }

    /* Create the semi-random Domain-SID */
    if (!CreateRandomSid(&DomainSid))
    {
        DebugPrint("Domain-SID creation failed!");
        return 0;
    }

    /* Set the Domain SID (aka Computer SID) */
    if (!SamSetDomainSid(DomainSid))
    {
        DebugPrint("SamSetDomainSid() failed!");
        RtlFreeSid(DomainSid);
        return 0;
    }

    /* Append the Admin-RID */
    AppendRidToSid(&AdminSid, DomainSid, DOMAIN_USER_RID_ADMIN);

    CreateTempDir(L"TEMP");
    CreateTempDir(L"TMP");

    if (GetWindowsDirectory(szBuffer, sizeof(szBuffer) / sizeof(TCHAR)))
    {
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                          0,
                          KEY_WRITE,
                          &hKey) == ERROR_SUCCESS)
        {
            RegSetValueExW(hKey,
                           L"PathName",
                           0,
                           REG_SZ,
                           (LPBYTE)szBuffer,
                           (wcslen(szBuffer) + 1) * sizeof(WCHAR));

            RegSetValueExW(hKey,
                           L"SystemRoot",
                           0,
                           REG_SZ,
                           (LPBYTE)szBuffer,
                           (wcslen(szBuffer) + 1) * sizeof(WCHAR));

            RegCloseKey(hKey);
        }

        PathAddBackslash(szBuffer);
        _tcscat(szBuffer, _T("system"));
        CreateDirectory(szBuffer, NULL);
    }

    if (!CommonInstall())
        return 0;

    InstallWizard();

    /* Create the Administrator account */
    if (!SamCreateUser(L"Administrator", L"", AdminSid))
    {
        /* Check what the error was.
         * If the Admin Account already exists, then it means Setup
         * wasn't allowed to finish properly. Instead of rebooting
         * and not completing it, let it restart instead
         */
        LastError = GetLastError();
        if (LastError != ERROR_USER_EXISTS)
        {
            DebugPrint("SamCreateUser() failed!");
            RtlFreeSid(AdminSid);
            RtlFreeSid(DomainSid);
            return 0;
        }
    }

    RtlFreeSid(AdminSid);
    RtlFreeSid(DomainSid);

    /* ROS HACK, as long as NtUnloadKey is not implemented */
    {
        NTSTATUS Status = NtUnloadKey(NULL);
        if (Status == STATUS_NOT_IMPLEMENTED)
        {
            /* Create the Administrator profile */
            PROFILEINFOW ProfileInfo;
            HANDLE hToken;
            BOOL ret;
#define LOGON32_LOGON_NETWORK 3
            ret = LogonUserW(L"Administrator", L"", L"", LOGON32_LOGON_NETWORK, LOGON32_PROVIDER_DEFAULT, &hToken);
            if (!ret)
            {
                DebugPrint("LogonUserW() failed!");
                return 0;
            }
            ZeroMemory(&ProfileInfo, sizeof(PROFILEINFOW));
            ProfileInfo.dwSize = sizeof(PROFILEINFOW);
            ProfileInfo.lpUserName = L"Administrator";
            ProfileInfo.dwFlags = PI_NOUI;
            LoadUserProfileW(hToken, &ProfileInfo);
            CloseHandle(hToken);
        }
        else
        {
            DPRINT1("ROS HACK not needed anymore. Please remove it\n");
        }
    }
    /* END OF ROS HACK */

    SetupCloseInfFile(hSysSetupInf);
    SetSetupType(0);

    LogItem(SYSSETUP_SEVERITY_INFORMATION, L"Installing Odyssey done");
    TerminateSetupActionLog();

    /* Get shutdown privilege */
    if (! OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
    {
        DebugPrint("OpenProcessToken() failed!");
        return 0;
    }
    if (!LookupPrivilegeValue(
        NULL,
        SE_SHUTDOWN_NAME,
        &privs.Privileges[0].Luid))
    {
        DebugPrint("LookupPrivilegeValue() failed!");
        return 0;
    }
    privs.PrivilegeCount = 1;
    privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (AdjustTokenPrivileges(
        token,
        FALSE,
        &privs,
        0,
        (PTOKEN_PRIVILEGES)NULL,
        NULL) == 0)
    {
        DebugPrint("AdjustTokenPrivileges() failed!");
        return 0;
    }

    ExitWindowsEx(EWX_REBOOT, 0);
    return 0;
}


/*
 * @unimplemented
 */
DWORD WINAPI
SetupChangeFontSize(
    IN HANDLE hWnd,
    IN LPCWSTR lpszFontSize)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/*
 * @unimplemented
 */
DWORD WINAPI
SetupChangeLocaleEx(HWND hWnd,
                    LCID Lcid,
                    LPCWSTR lpSrcRootPath,
                    char Unknown,
                    DWORD dwUnused1,
                    DWORD dwUnused2)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/*
 * @implemented
 */
DWORD WINAPI
SetupChangeLocale(HWND hWnd, LCID Lcid)
{
    return SetupChangeLocaleEx(hWnd, Lcid, NULL, 0, 0, 0);
}
