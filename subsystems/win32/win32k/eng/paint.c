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
/* $Id: paint.c 49275 2010-10-25 17:36:27Z tkreuzer $
 *
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey kernel
 * PURPOSE:           GDI Driver Paint Functions
 * FILE:              subsys/win32k/eng/paint.c
 * PROGRAMER:         Jason Filby
 * REVISION HISTORY:
 *                 3/7/1999: Created
 */

#include <win32k.h>

#define NDEBUG
#include <debug.h>

BOOL APIENTRY FillSolid(SURFOBJ *pso, PRECTL pRect, ULONG iColor)
{
  LONG y;
  ULONG LineWidth;

  ASSERT(pso);
  ASSERT(pRect);
  LineWidth  = pRect->right - pRect->left;
  DPRINT(" LineWidth: %d, top: %d, bottom: %d\n", LineWidth, pRect->top, pRect->bottom);
  for (y = pRect->top; y < pRect->bottom; y++)
  {
    DibFunctionsForBitmapFormat[pso->iBitmapFormat].DIB_HLine(
      pso, pRect->left, pRect->right, y, iColor);
  }
  return TRUE;
}

BOOL APIENTRY
EngPaintRgn(SURFOBJ *pso, CLIPOBJ *ClipRegion, ULONG iColor, MIX Mix,
            BRUSHOBJ *BrushObj, POINTL *BrushPoint)
{
  RECT_ENUM RectEnum;
  BOOL EnumMore;
  ULONG i;

  ASSERT(pso);
  ASSERT(ClipRegion);

  DPRINT("ClipRegion->iMode:%d, ClipRegion->iDComplexity: %d\n Color: %d", ClipRegion->iMode, ClipRegion->iDComplexity, iColor);
  switch(ClipRegion->iMode) {

    case TC_RECTANGLES:

    /* Rectangular clipping can be handled without enumeration.
       Note that trivial clipping is not possible, since the clipping
       region defines the area to fill */

    if (ClipRegion->iDComplexity == DC_RECT)
    {
      FillSolid(pso, &(ClipRegion->rclBounds), iColor);
    } else {

      /* Enumerate all the rectangles and draw them */
      CLIPOBJ_cEnumStart(ClipRegion, FALSE, CT_RECTANGLES, CD_ANY, 0);

      do {
        EnumMore = CLIPOBJ_bEnum(ClipRegion, sizeof(RectEnum), (PVOID) &RectEnum);
        for (i = 0; i < RectEnum.c; i++) {
          FillSolid(pso, RectEnum.arcl + i, iColor);
        }
      } while (EnumMore);
    }

    return(TRUE);

    default:
       return(FALSE);
  }
}

/*
 * @unimplemented
 */
BOOL APIENTRY
EngPaint(IN SURFOBJ *pso,
	 IN CLIPOBJ *ClipRegion,
	 IN BRUSHOBJ *Brush,
	 IN POINTL *BrushOrigin,
	 IN MIX  Mix)
{
  BOOLEAN ret;

  // FIXME: We only support a brush's solid color attribute
  ret = EngPaintRgn(pso, ClipRegion, Brush->iSolidColor, Mix, Brush, BrushOrigin);

  return ret;
}

BOOL APIENTRY
IntEngPaint(IN SURFOBJ *pso,
            IN CLIPOBJ *ClipRegion,
            IN BRUSHOBJ *Brush,
            IN POINTL *BrushOrigin,
            IN MIX  Mix)
{
  SURFACE *psurf = CONTAINING_RECORD(pso, SURFACE, SurfObj);
  BOOL ret;

  DPRINT("pso->iType == %d\n", pso->iType);
  /* Is the surface's Paint function hooked? */
  if((pso->iType!=STYPE_BITMAP) && (psurf->flags & HOOK_PAINT))
  {
    // Call the driver's DrvPaint
    ret = GDIDEVFUNCS(pso).Paint(
      pso, ClipRegion, Brush, BrushOrigin, Mix);
    return ret;
  }
  return EngPaint(pso, ClipRegion, Brush, BrushOrigin, Mix );

}
/* EOF */
