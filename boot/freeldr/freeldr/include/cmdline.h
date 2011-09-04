/* $Id: cmdline.h 45685 2010-02-26 11:43:19Z gedmurphy $
 *
 *  FreeLdr boot loader
 *  Copyright (C) 2002, 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
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

#pragma once

typedef struct tagCMDLINEINFO
{
  const char *DefaultOperatingSystem;
  LONG TimeOut;
} CMDLINEINFO, *PCMDLINEINFO;

void CmdLineParse(char *CmdLine);

const char *CmdLineGetDefaultOS(void);
LONG CmdLineGetTimeOut(void);

/* EOF */
