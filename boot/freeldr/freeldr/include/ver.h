/*
 *  FreeLoader
 *  Copyright (C) 1998-2005  Brian Palmer  <brianp@sginet.com>
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

#ifndef __ASM__
#pragma once
#endif

/* just some stuff */
#define VERSION			"FreeLoader v3.0"
#define COPYRIGHT		"Copyright (C) 1998-2007 ReactOS Team; (C) 2011 NasuTek Enterprises"
#define AUTHOR_EMAIL	"<www.odyssey.org>"
#define BY_AUTHOR		"by Odyssey Team"

// FreeLoader version defines
//
// NOTE:
// If you fix bugs then you increment the patch version
// If you add features then you increment the minor version and zero the patch version
// If you add major functionality then you increment the major version and zero the minor & patch versions
//
#define FREELOADER_MAJOR_VERSION	3
#define FREELOADER_MINOR_VERSION	0
#define FREELOADER_PATCH_VERSION	0


#ifndef __ASM__

PCHAR	GetFreeLoaderVersionString(VOID);

#endif // __ASM__
