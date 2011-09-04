/*
 *  Odyssey W32 Subsystem
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* $Id: perfcnt.c 47036 2010-04-26 13:58:46Z gadamopoulos $
 *
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey kernel
 * PURPOSE:           GDI Driver Performance Counter Functions
 * FILE:              subsys/win32k/eng/perfcnt.c
 * PROGRAMER:         Ge van Geldorp
 */

#include <win32k.h>

#define NDEBUG
#include <debug.h>

/*
 * @implemented
 */
VOID APIENTRY
EngQueryPerformanceFrequency(LONGLONG *Frequency)
{
  LARGE_INTEGER Freq;

  KeQueryPerformanceCounter(&Freq);
  *Frequency = Freq.QuadPart;
}

/*
 * @implemented
 */
VOID APIENTRY
EngQueryPerformanceCounter(LONGLONG *Count)
{
  LARGE_INTEGER PerfCount;

  PerfCount = KeQueryPerformanceCounter(NULL);
  *Count = PerfCount.QuadPart;
}
