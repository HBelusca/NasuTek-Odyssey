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
 /*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey kernel
 * PURPOSE:           GDI Internal Objects
 * FILE:              subsystem/win32/win32k/eng/objects.h
 * PROGRAMER:         Jason Filby
 * REVISION HISTORY:
 *                 21/8/1999: Created
 */

#pragma once

/* Structure of internal gdi objects that win32k manages for ddi engine:
   |---------------------------------|
   |         Public part             |
   |      accessed from engine       |
   |---------------------------------|
   |        Private part             |
   |       managed by gdi            |
   |_________________________________|

---------------------------------------------------------------------------*/

/* EXtended CLip and Window Region Object */
typedef struct _XCLIPOBJ
{
  WNDOBJ;
  PVOID   pClipRgn;    /* prgnRao_ or (prgnVis_ if (prgnRao_ == z)) */
  RECTL   rclClipRgn;
  PVOID   pscanClipRgn; /* Ptr to regions rect buffer based on iDirection. */
  DWORD   cScan;
  DWORD   reserved;
  ULONG   ulBSize;
  LONG    lscnSize;
  ULONG   ulObjSize;
  ULONG   iDirection;
  ULONG   ulClipType;
  DWORD   reserved1;
  LONG    lUpDown;
  DWORD   reserved2;
  BOOL    bShouldDoAll;
  DWORD   nComplexity; /* count/mode based on # of rect in regions scan. */
  PVOID   pDDA;        /* Pointer to a large drawing structure. */
} XCLIPOBJ, *PXCLIPOBJ;
/*
  EngCreateClip allocates XCLIPOBJ and RGNOBJ, pco->co.pClipRgn = &pco->ro.
  {
    XCLIPOBJ co;
    RGNOBJ   ro;
  }
 */
typedef struct _CLIPGDI {
  CLIPOBJ ClipObj;
  ULONG EnumPos;
  ULONG EnumOrder;
  ULONG EnumMax;
  ENUMRECTS EnumRects;
} CLIPGDI, *PCLIPGDI;

/*ei What is this for? */
typedef struct _DRVFUNCTIONSGDI {
  HDEV  hdev;
  DRVFN Functions[INDEX_LAST];
} DRVFUNCTIONSGDI;

typedef struct _FLOATGDI {
  ULONG Dummy;
} FLOATGDI;

typedef struct _FONTGDI {
  FONTOBJ     FontObj;
  ULONG       iUnique;
  FLONG       flType;
  union{
  DHPDEV      dhpdev;
  FT_Face     face;
  };

  LONG        lMaxNegA;
  LONG        lMaxNegC;
  LONG        lMinWidthD;

  LPWSTR      Filename;
  BYTE        Underline;
  BYTE        StrikeOut;
} FONTGDI, *PFONTGDI;

typedef struct _PATHGDI {
  PATHOBJ PathObj;
} PATHGDI;

typedef struct _WNDGDI {
  WNDOBJ            WndObj;
  HWND              Hwnd;
  CLIPOBJ           *ClientClipObj;
  WNDOBJCHANGEPROC  ChangeProc;
  FLONG             Flags;
  int               PixelFormat;
} WNDGDI, *PWNDGDI;

typedef struct _XFORMGDI {
  ULONG Dummy;
  /* XFORMOBJ has no public members */
} XFORMGDI;

/* as the *OBJ structures are located at the beginning of the *GDI structures
   we can simply typecast the pointer */
#define ObjToGDI(ClipObj, Type) (Type##GDI *)(ClipObj)
#define GDIToObj(ClipGDI, Type) (Type##OBJ *)(ClipGDI)
