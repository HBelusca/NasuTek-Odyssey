/*
 * PROJECT:     Odyssey System Control Panel Applet
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        dll/cpl/sysdm/advanced.c
 * PURPOSE:     Memory, start-up and profiles settings
 * COPYRIGHT:   Copyright Thomas Weidenmueller <w3seek@odyssey.org>
                Copyright 2006 - 2009 Ged Murphy <gedmurphy@odyssey.org>
 *
 */

#include "precomp.h"

static TCHAR BugLink[] = _T("http://www.odyssey.org/bugzilla");
static TCHAR ReportAsWorkstationKey[] = _T("SYSTEM\\CurrentControlSet\\Control\\Odyssey\\Settings\\Version");


static VOID
OnOK(HWND hwndDlg)
{
    HKEY hKey;
    DWORD ReportAsWorkstation;

    ReportAsWorkstation = (SendDlgItemMessageW(hwndDlg,
                                               IDC_REPORTASWORKSTATION,
                                               BM_GETCHECK,
                                               0,
                                               0) == BST_CHECKED);

    if (RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                       ReportAsWorkstationKey,
                       0,
                       NULL,
                       0,
                       KEY_WRITE,
                       NULL,
                       &hKey,
                       NULL) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey,
                      _T("ReportAsWorkstation"),
                      0,
                      REG_DWORD,
                      (LPBYTE)&ReportAsWorkstation,
                      sizeof(DWORD));

        RegCloseKey(hKey);
    }
}

static VOID
OnInitSysSettingsDialog(HWND hwndDlg)
{
    HKEY hKey;
    DWORD dwVal;
    DWORD dwType = REG_DWORD;
    DWORD cbData = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     ReportAsWorkstationKey,
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueEx(hKey,
                            _T("ReportAsWorkstation"),
                            0,
                            &dwType,
                            (LPBYTE)&dwVal,
                            &cbData) == ERROR_SUCCESS)
        {
            if (dwVal == TRUE)
            {
                // set the check box
                SendDlgItemMessageW(hwndDlg,
                                    IDC_REPORTASWORKSTATION,
                                    BM_SETCHECK,
                                    BST_CHECKED,
                                    0);
            }
        }

        RegCloseKey(hKey);
    }
}

INT_PTR CALLBACK
SysSettingsDlgProc(HWND hwndDlg,
                   UINT uMsg,
                   WPARAM wParam,
                   LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (uMsg)
    {
        case WM_INITDIALOG:
            OnInitSysSettingsDialog(hwndDlg);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    OnOK(hwndDlg);
                    EndDialog(hwndDlg, 0);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}


/* Property page dialog callback */
INT_PTR CALLBACK
AdvancedPageProc(HWND hwndDlg,
                 UINT uMsg,
                 WPARAM wParam,
                 LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (uMsg)
    {
        case WM_INITDIALOG:
            break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_PERFOR:
                    DialogBox(hApplet,
                              MAKEINTRESOURCE(IDD_VIRTMEM),
                              hwndDlg,
                              (DLGPROC)VirtMemDlgProc);
                    break;

                case IDC_USERPROFILE:
                    DialogBox(hApplet,
                              MAKEINTRESOURCE(IDD_USERPROFILE),
                              hwndDlg,
                              (DLGPROC)UserProfileDlgProc);
                    break;

                case IDC_STAREC:
                    DialogBox(hApplet,
                              MAKEINTRESOURCE(IDD_STARTUPRECOVERY),
                              hwndDlg,
                              (DLGPROC)StartRecDlgProc);
                    break;

                case IDC_SYSSETTINGS:
                    DialogBox(hApplet,
                              MAKEINTRESOURCE(IDD_SYSSETTINGS),
                              hwndDlg,
                              (DLGPROC)SysSettingsDlgProc);
                    break;

                case IDC_ENVVAR:
                    DialogBox(hApplet,
                              MAKEINTRESOURCE(IDD_ENVIRONMENT_VARIABLES),
                              hwndDlg,
                              (DLGPROC)EnvironmentDlgProc);
                    break;

                case IDC_ERRORREPORT:
                    ShellExecute(NULL,
                                 _T("open"),
                                 BugLink,
                                 NULL,
                                 NULL,
                                 SW_SHOWNORMAL);
                    break;
            }
        }

        break;
    }

    return FALSE;
}
