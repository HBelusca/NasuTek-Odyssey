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

#include <win32k.h>

#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

BOOL
FASTCALL
RECTL_bUnionRect(RECTL *prclDst, const RECTL *prcl1, const RECTL *prcl2)
{
    if (RECTL_bIsEmptyRect(prcl1))
    {
        if (RECTL_bIsEmptyRect(prcl2))
	    {
	        RECTL_vSetEmptyRect(prclDst);
	        return FALSE;
	    }
        else
	    {
	        *prclDst = *prcl2;
	    }
    }
    else
    {
        if (RECTL_bIsEmptyRect(prcl2))
	    {
	        *prclDst = *prcl1;
	    }
        else
	    {
	        prclDst->left = min(prcl1->left, prcl2->left);
	        prclDst->top = min(prcl1->top, prcl2->top);
	        prclDst->right = max(prcl1->right, prcl2->right);
	        prclDst->bottom = max(prcl1->bottom, prcl2->bottom);
	    }
    }

    return TRUE;
}


BOOL
FASTCALL
RECTL_bIntersectRect(RECTL* prclDst, const RECTL* prcl1, const RECTL* prcl2)
{
    prclDst->left  = max(prcl1->left, prcl2->left);
    prclDst->right = min(prcl1->right, prcl2->right);

    if (prclDst->left < prclDst->right)
    {
        prclDst->top    = max(prcl1->top, prcl2->top);
        prclDst->bottom = min(prcl1->bottom, prcl2->bottom);

        if (prclDst->top < prclDst->bottom)
        {
            return TRUE;
        }
    }

    RECTL_vSetEmptyRect(prclDst);

    return FALSE;
}

VOID
FASTCALL
RECTL_vMakeWellOrdered(RECTL *prcl)
{
    LONG lTmp;
    if (prcl->left > prcl->right)
    {
        lTmp = prcl->left;
        prcl->left = prcl->right;
        prcl->right = lTmp;       
    }
    if (prcl->top > prcl->bottom)
    {
        lTmp = prcl->top;
        prcl->top = prcl->bottom;
        prcl->bottom = lTmp;       
    }
}

VOID 
FASTCALL
RECTL_vInflateRect(RECTL *rect, INT dx, INT dy)
{
    rect->left -= dx;
    rect->top -= dy;
    rect->right += dx;
    rect->bottom += dy;
}

/* EOF */
