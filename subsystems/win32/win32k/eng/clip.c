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
/* $Id: clip.c 53467 2011-08-27 12:38:23Z gadamopoulos $
 *
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey kernel
 * PURPOSE:           GDI Clipping Functions
 * FILE:              subsys/win32k/eng/clip.c
 * PROGRAMER:         Jason Filby
 * REVISION HISTORY:
 *                 21/8/1999: Created
 */

#include <win32k.h>

#define NDEBUG
#include <debug.h>

static __inline int
CompareRightDown(
    const RECTL *r1,
    const RECTL *r2)
{
    int Cmp;

    if (r1->top < r2->top)
    {
        Cmp = -1;
    }
    else if (r2->top < r1->top)
    {
        Cmp = +1;
    }
    else
    {
        ASSERT(r1->bottom == r2->bottom);
        if (r1->left < r2->left)
        {
            Cmp = -1;
        }
        else if (r2->left < r1->left)
        {
            Cmp = +1;
        }
        else
        {
            ASSERT(r1->right == r2->right);
            Cmp = 0;
        }
    }

    return Cmp;
}

static __inline int
CompareRightUp(
    const RECTL *r1,
    const RECTL *r2)
{
    int Cmp;

    if (r1->bottom < r2->bottom)
    {
        Cmp = +1;
    }
    else if (r2->bottom < r1->bottom)
    {
        Cmp = -1;
    }
    else
    {
        ASSERT(r1->top == r2->top);
        if (r1->left < r2->left)
        {
            Cmp = -1;
        }
        else if (r2->left < r1->left)
        {
            Cmp = +1;
        }
        else
        {
            ASSERT(r1->right == r2->right);
            Cmp = 0;
        }
    }

    return Cmp;
}

static __inline int
CompareLeftDown(
    const RECTL *r1,
    const RECTL *r2)
{
    int Cmp;

    if (r1->top < r2->top)
    {
        Cmp = -1;
    }
    else if (r2->top < r1->top)
    {
        Cmp = +1;
    }
    else
    {
        ASSERT(r1->bottom == r2->bottom);
        if (r1->right < r2->right)
        {
            Cmp = +1;
        }
        else if (r2->right < r1->right)
        {
            Cmp = -1;
        }
        else
        {
            ASSERT(r1->left == r2->left);
            Cmp = 0;
        }
    }

    return Cmp;
}

static __inline int
CompareLeftUp(
    const RECTL *r1,
    const RECTL *r2)
{
    int Cmp;

    if (r1->bottom < r2->bottom)
    {
        Cmp = +1;
    }
    else if (r2->bottom < r1->bottom)
    {
        Cmp = -1;
    }
    else
    {
        ASSERT(r1->top == r2->top);
        if (r1->right < r2->right)
        {
            Cmp = +1;
        }
        else if (r2->right < r1->right)
        {
            Cmp = -1;
        }
        else
        {
            ASSERT(r1->left == r2->left);
            Cmp = 0;
        }
    }
    return Cmp;
}

static __inline int
CompareSpans(
    const SPAN *Span1,
    const SPAN *Span2)
{
    int Cmp;

    if (Span1->Y < Span2->Y)
    {
        Cmp = -1;
    }
    else if (Span2->Y < Span1->Y)
    {
        Cmp = +1;
    }
    else
    {
        if (Span1->X < Span2->X)
        {
            Cmp = -1;
        }
        else if (Span2->X < Span1->X)
        {
            Cmp = +1;
        }
        else
        {
            Cmp = 0;
        }
    }

    return Cmp;
}

VOID FASTCALL
IntEngDeleteClipRegion(CLIPOBJ *ClipObj)
{
    EngFreeMem(ObjToGDI(ClipObj, CLIP));
}

CLIPOBJ* FASTCALL
IntEngCreateClipRegion(ULONG count, PRECTL pRect, PRECTL rcBounds)
{
    CLIPGDI *Clip;

    if(count > 1)
    {
        RECTL *dest;

        Clip = EngAllocMem(0, sizeof(CLIPGDI) + ((count - 1) * sizeof(RECTL)), GDITAG_CLIPOBJ);

        if(Clip != NULL)
        {
            Clip->EnumRects.c = count;
            Clip->EnumOrder = CD_ANY;
            for(dest = Clip->EnumRects.arcl;count > 0; count--, dest++, pRect++)
            {
                *dest = *pRect;
            }

            Clip->ClipObj.iDComplexity = DC_COMPLEX;
            Clip->ClipObj.iFComplexity = ((Clip->EnumRects.c <= 4) ? FC_RECT4 : FC_COMPLEX);
            Clip->ClipObj.iMode = TC_RECTANGLES;
            Clip->ClipObj.rclBounds = *rcBounds;

            return GDIToObj(Clip, CLIP);
        }
    }
    else
    {
        Clip = EngAllocMem(0, sizeof(CLIPGDI), GDITAG_CLIPOBJ);

        if(Clip != NULL)
        {
            Clip->EnumRects.c = 1;
            Clip->EnumOrder = CD_ANY;
            Clip->EnumRects.arcl[0] = *rcBounds;

            Clip->ClipObj.iDComplexity = (((rcBounds->top == rcBounds->bottom) &&
                                         (rcBounds->left == rcBounds->right))
                                         ? DC_TRIVIAL : DC_RECT);

            Clip->ClipObj.iFComplexity = FC_RECT;
            Clip->ClipObj.iMode = TC_RECTANGLES;
            Clip->ClipObj.rclBounds = *rcBounds;

            return GDIToObj(Clip, CLIP);
        }
    }

    return NULL;
}

/*
 * @implemented
 */
CLIPOBJ * APIENTRY
EngCreateClip(VOID)
{
    CLIPGDI *Clip = EngAllocMem(FL_ZERO_MEMORY, sizeof(CLIPGDI), GDITAG_CLIPOBJ);
    if(Clip != NULL)
    {
        return GDIToObj(Clip, CLIP);
    }

    return NULL;
}

/*
 * @implemented
 */
VOID APIENTRY
EngDeleteClip(CLIPOBJ *ClipRegion)
{
    EngFreeMem(ObjToGDI(ClipRegion, CLIP));
}

/*
 * @implemented
 */
ULONG APIENTRY
CLIPOBJ_cEnumStart(
    IN CLIPOBJ* ClipObj,
    IN BOOL ShouldDoAll,
    IN ULONG ClipType,
    IN ULONG BuildOrder,
    IN ULONG MaxRects)
{
    CLIPGDI *ClipGDI = ObjToGDI(ClipObj, CLIP);
    SORTCOMP CompareFunc;

    ClipGDI->EnumPos = 0;
    ClipGDI->EnumMax = (MaxRects > 0) ? MaxRects : ClipGDI->EnumRects.c;

    if (CD_ANY != BuildOrder && ClipGDI->EnumOrder != BuildOrder)
    {
        switch (BuildOrder)
        {
            case CD_RIGHTDOWN:
                CompareFunc = (SORTCOMP) CompareRightDown;
                break;

            case CD_RIGHTUP:
                CompareFunc = (SORTCOMP) CompareRightUp;
                break;

            case CD_LEFTDOWN:
                CompareFunc = (SORTCOMP) CompareLeftDown;
                break;

            case CD_LEFTUP:
                CompareFunc = (SORTCOMP) CompareLeftUp;
                break;

            default:
                DPRINT1("Invalid BuildOrder %d\n", BuildOrder);
                BuildOrder = ClipGDI->EnumOrder;
                CompareFunc = NULL;
                break;
        }

        if (NULL != CompareFunc)
        {
            EngSort((PBYTE) ClipGDI->EnumRects.arcl, sizeof(RECTL), ClipGDI->EnumRects.c, CompareFunc);
        }

        ClipGDI->EnumOrder = BuildOrder;
    }

    /* Return the number of rectangles enumerated */
    if ((MaxRects > 0) && (ClipGDI->EnumRects.c > MaxRects))
    {
        return 0xFFFFFFFF;
    }

    return ClipGDI->EnumRects.c;
}

/*
 * @implemented
 */
BOOL APIENTRY
CLIPOBJ_bEnum(
    IN CLIPOBJ* ClipObj,
    IN ULONG ObjSize,
    OUT ULONG *EnumRects)
{
    RECTL *dest, *src;
    CLIPGDI *ClipGDI = ObjToGDI(ClipObj, CLIP);
    ULONG nCopy, i;
    ENUMRECTS* pERects = (ENUMRECTS*)EnumRects;

    //calculate how many rectangles we should copy
    nCopy = min( ClipGDI->EnumMax - ClipGDI->EnumPos,
            min( ClipGDI->EnumRects.c - ClipGDI->EnumPos,
            (ObjSize - sizeof(ULONG)) / sizeof(RECTL)));

    if(nCopy == 0)
    {
        return FALSE;
    }

    /* copy rectangles */
    src = ClipGDI->EnumRects.arcl + ClipGDI->EnumPos;
    for(i = 0, dest = pERects->arcl; i < nCopy; i++, dest++, src++)
    {
        *dest = *src;
    }

    pERects->c = nCopy;

    ClipGDI->EnumPos+=nCopy;

    return ClipGDI->EnumPos < ClipGDI->EnumRects.c;
}

/* EOF */
