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
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey GUI/console setup
 * FILE:            subsys/system/setup/setup.c
 * PURPOSE:         Second stage setup
 * PROGRAMMER:      Eric Kohl
 */
#define WIN32_NO_STATUS
#include <windows.h>
#include <tchar.h>
#include <syssetup/syssetup.h>
#include <userenv.h>
#include <tchar.h>

#define NDEBUG
#include <debug.h>


typedef DWORD (WINAPI *PINSTALL_ODYSSEY)(HINSTANCE hInstance);


/* FUNCTIONS ****************************************************************/

LPTSTR lstrchr(LPCTSTR s, TCHAR c)
{
  while (*s)
    {
      if (*s == c)
        return (LPTSTR)s;
      s++;
    }

  if (c == (TCHAR)0)
    return (LPTSTR)s;

  return (LPTSTR)NULL;
}

static VOID
RunNewSetup (HINSTANCE hInstance)
{
  HMODULE hDll;
  PINSTALL_ODYSSEY InstallOdyssey;

  hDll = LoadLibrary (TEXT("syssetup"));
  if (hDll == NULL)
    {
      DPRINT("Failed to load 'syssetup'!\n");
      return;
    }

  DPRINT("Loaded 'syssetup'!\n");
  InstallOdyssey = (PINSTALL_ODYSSEY)GetProcAddress (hDll, "InstallOdyssey");

  if (InstallOdyssey == NULL)
    {
      DPRINT("Failed to get address for 'InstallOdyssey()'!\n");
      FreeLibrary (hDll);
      return;
    }

  InstallOdyssey (hInstance);

  FreeLibrary (hDll);
}

static VOID
RunLiveCD (HINSTANCE hInstance)
{
  HMODULE hDll;
  PINSTALL_ODYSSEY InstallLiveCD;

  hDll = LoadLibrary (TEXT("syssetup"));
  if (hDll == NULL)
    {
      DPRINT("Failed to load 'syssetup'!\n");
      return;
    }

  DPRINT("Loaded 'syssetup'!\n");
  InstallLiveCD = (PINSTALL_ODYSSEY)GetProcAddress (hDll, "InstallLiveCD");

  if (InstallLiveCD == NULL)
    {
      DPRINT("Failed to get address for 'InstallOdyssey()'!\n");
      FreeLibrary (hDll);
      return;
    }

  InstallLiveCD (hInstance);

  FreeLibrary (hDll);
}

int WINAPI
_tWinMain (HINSTANCE hInstance,
	 HINSTANCE hPrevInstance,
	 LPTSTR lpCmdLine,
	 int nShowCmd)
{
  LPTSTR CmdLine;
  LPTSTR p;

  CmdLine = GetCommandLine ();

  DPRINT("CmdLine: <%s>\n",CmdLine);

  p = lstrchr (CmdLine, TEXT('-'));
  if (p == NULL)
    return 0;

  if (!lstrcmpi (p, TEXT("-newsetup")))
    {
      RunNewSetup (hInstance);
    }
  else if (!lstrcmpi (p, TEXT("-mini")))
    {
      RunLiveCD (hInstance);
    }

#if 0
  /* Add new setup types here */
  else if (...)
    {

    }
#endif

  return 0;
}

/* EOF */
