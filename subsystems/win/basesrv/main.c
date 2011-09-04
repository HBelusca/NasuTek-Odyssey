/* $Id: main.c 43790 2009-10-27 10:34:16Z dgorbachev $
 *
 * dllmain.c - Odyssey/Win32 base enviroment subsystem server
 *
 * Odyssey Operating System
 *
 * --------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * --------------------------------------------------------------------
 */
#include "basesrv.h"

#define NDEBUG
#include <debug.h>

HANDLE DllHandle = 0;

/* FUNCTIONS *****************************************************************/

BOOL WINAPI DllMain(HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  if (DLL_PROCESS_ATTACH == dwReason)
    {
      DllHandle = hDll;
    }

  return TRUE;
}

/* EOF */
