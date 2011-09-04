/*
 * Regedit About Dialog Box
 *
 * Copyright (C) 2002 Robert Dickenson <robd@odyssey.org>
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

#include <regedit.h>


void ShowAboutBox(HWND hWnd)
{
    TCHAR AppStr[255];
    LoadString(hInst, IDS_APP_TITLE, AppStr, sizeof(AppStr)/sizeof(TCHAR));
    ShellAbout(hWnd, AppStr, _T(""), LoadIcon(hInst, MAKEINTRESOURCE(IDI_REGEDIT)));
}
