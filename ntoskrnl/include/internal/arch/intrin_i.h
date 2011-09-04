/*
 *  Odyssey kernel
 *  Copyright (C) 1998, 1999, 2000, 2001 ReactOS Team; (C) 2011 NasuTek Enterprises
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

#ifdef _M_IX86
#include "../i386/intrin_i.h"
#elif defined(_M_PPC)
#include "../powerpc/intrin_i.h"
#elif defined(_M_MIPS)
#include "../mips/intrin_i.h"
#elif defined(_M_ARM)
#include "../arm/intrin_i.h"
#elif defined(_M_AMD64)
#include "../amd64/intrin_i.h"
#else
#error "Unknown processor"
#endif

/* EOF */
