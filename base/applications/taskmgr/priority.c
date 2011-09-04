/*
 *  Odyssey Task Manager
 *
 *  priority.c
 *
 *  Copyright (C) 1999 - 2001  Brian Palmer  <brianp@odyssey.org>
 *                2005         Klemens Friedl <frik85@odyssey.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <precomp.h>

void DoSetPriority(DWORD priority)
{
    DWORD   dwProcessId;
    HANDLE  hProcess;
    WCHAR   szText[260];
    WCHAR   szTitle[256];

    dwProcessId = GetSelectedProcessId();

    if (dwProcessId == 0)
        return;

    LoadStringW(hInst, IDS_MSG_TASKMGRWARNING, szTitle, 256);
    LoadStringW(hInst, IDS_MSG_WARNINGCHANGEPRIORITY, szText, 260);
    if (MessageBoxW(hMainWnd, szText, szTitle, MB_YESNO|MB_ICONWARNING) != IDYES)
        return;

    hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    if (!hProcess)
    {
        GetLastErrorText(szText, 260);
        LoadStringW(hInst, IDS_MSG_UNABLECHANGEPRIORITY, szTitle, 256);
        MessageBoxW(hMainWnd, szText, szTitle, MB_OK|MB_ICONSTOP);
        return;
    }

    if (!SetPriorityClass(hProcess, priority))
    {
        GetLastErrorText(szText, 260);
        LoadStringW(hInst, IDS_MSG_UNABLECHANGEPRIORITY, szTitle, 256);
        MessageBoxW(hMainWnd, szText, szTitle, MB_OK|MB_ICONSTOP);
    }

    CloseHandle(hProcess);
}
