/*
 * PROJECT:     Odyssey Services
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        base/applications/mscutils/servman/about.c
 * PURPOSE:     About dialog box message handler
 * COPYRIGHT:   Copyright 2005-2007 Ged Murphy <gedmurphy@odyssey.org>
 *
 */

#include "precomp.h"

INT_PTR CALLBACK
AboutDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HWND hLicenseEditWnd;

        hLicenseEditWnd = GetDlgItem(hDlg,
                                     IDC_LICENSE_EDIT);
        if (hLicenseEditWnd)
        {
            LPTSTR lpString;

            if (AllocAndLoadString(&lpString,
                                   hInstance,
                                   IDS_LICENSE))
            {
                SetWindowText(hLicenseEditWnd,
                              lpString);

                LocalFree(lpString);
            }
        }

        return TRUE;
    }

    case WM_COMMAND:

        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            EndDialog(hDlg,
                      LOWORD(wParam));
            return TRUE;
        }

        break;
    }

    return FALSE;
}
