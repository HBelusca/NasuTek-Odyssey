/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey System Libraries
 * FILE:            lib/gdi32/include/precomp.h
 * PURPOSE:         User-Mode Win32 GDI Library Header
 * PROGRAMMER:      Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/

/* Definitions */
#define WIN32_NO_STATUS
#define NTOS_MODE_USER

#include <stdio.h>

/* SDK/DDK/NDK Headers. */
#include <windows.h>
#include <ndk/rtlfuncs.h>
#include <wingdi.h>
#include <winddi.h>
#include <d3dnthal.h>
#include <prntfont.h>
#include <winddiui.h>
#include <winspool.h>

#include <pseh/pseh2.h>

#include <ddraw.h>
#include <ddrawi.h>
#include <ddrawint.h>
#include <ddrawgdi.h>
#include <d3dhal.h>

/* Public Win32K Headers */
#include <win32k/ntgdityp.h>
#include <ntgdi.h>
#include <win32k/ntgdihdl.h>

/* Private GDI32 Header */
#include "gdi32p.h"

/* Deprecated NTGDI calls which shouldn't exist */
#include <win32k/ntgdibad.h>

/* EOF */
