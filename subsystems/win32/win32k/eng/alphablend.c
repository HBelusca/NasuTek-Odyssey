/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          GDI alpha blending functions
 * FILE:             subsystems/win32/win32k/eng/alphablend.c
 * PROGRAMER:        Jason Filby
 */

#include <win32k.h>

#define NDEBUG
#include <debug.h>


/*
 * @implemented
 */
BOOL
APIENTRY
EngAlphaBlend(IN SURFOBJ *psoDest,
              IN SURFOBJ *psoSource,
              IN CLIPOBJ *ClipRegion,
              IN XLATEOBJ *ColorTranslation,
              IN PRECTL DestRect,
              IN PRECTL SourceRect,
              IN BLENDOBJ *BlendObj)
{
    RECTL              InputRect;
    RECTL              OutputRect;
    RECTL              ClipRect;
    RECTL              CombinedRect;
    RECTL              Rect;
    POINTL             Translate;
    INTENG_ENTER_LEAVE EnterLeaveSource;
    INTENG_ENTER_LEAVE EnterLeaveDest;
    SURFOBJ*           InputObj;
    SURFOBJ*           OutputObj;
    LONG               ClippingType;
    RECT_ENUM          RectEnum;
    BOOL               EnumMore;
    INT                i;
    BOOLEAN            Ret;

    DPRINT("EngAlphaBlend(psoDest:0x%p, psoSource:0x%p, ClipRegion:0x%p, ColorTranslation:0x%p,\n", psoDest, psoSource, ClipRegion, ColorTranslation);
    DPRINT("              DestRect:{0x%x, 0x%x, 0x%x, 0x%x}, SourceRect:{0x%x, 0x%x, 0x%x, 0x%x},\n",
           DestRect->left, DestRect->top, DestRect->right, DestRect->bottom,
           SourceRect->left, SourceRect->top, SourceRect->right, SourceRect->bottom);
    DPRINT("              BlendObj:{0x%x, 0x%x, 0x%x, 0x%x}\n", BlendObj->BlendFunction.BlendOp,
           BlendObj->BlendFunction.BlendFlags, BlendObj->BlendFunction.SourceConstantAlpha,
           BlendObj->BlendFunction.AlphaFormat);

    /* Validate output */
    OutputRect = *DestRect;
    if (OutputRect.right < OutputRect.left)
    {
        OutputRect.left = DestRect->right;
        OutputRect.right = DestRect->left;
    }
    if (OutputRect.bottom < OutputRect.top)
    {
        OutputRect.left = DestRect->right;
        OutputRect.right = DestRect->left;
    }

    /* Validate input */
    InputRect = *SourceRect;
    if ( (InputRect.top < 0) || (InputRect.bottom < 0) ||
         (InputRect.left < 0) || (InputRect.right < 0) ||
         InputRect.right > psoSource->sizlBitmap.cx ||
         InputRect.bottom > psoSource->sizlBitmap.cy )
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (psoDest == psoSource &&
            !(OutputRect.left >= SourceRect->right || InputRect.left >= OutputRect.right ||
              OutputRect.top >= SourceRect->bottom || InputRect.top >= OutputRect.bottom))
    {
        DPRINT1("Source and destination rectangles overlap!\n");
        return FALSE;
    }

    if (BlendObj->BlendFunction.BlendOp != AC_SRC_OVER)
    {
        DPRINT1("BlendOp != AC_SRC_OVER (0x%x)\n", BlendObj->BlendFunction.BlendOp);
        return FALSE;
    }
    if (BlendObj->BlendFunction.BlendFlags != 0)
    {
        DPRINT1("BlendFlags != 0 (0x%x)\n", BlendObj->BlendFunction.BlendFlags);
        return FALSE;
    }
    if ((BlendObj->BlendFunction.AlphaFormat & ~AC_SRC_ALPHA) != 0)
    {
        DPRINT1("Unsupported AlphaFormat (0x%x)\n", BlendObj->BlendFunction.AlphaFormat);
        return FALSE;
    }

    /* Check if there is anything to draw */
    if (ClipRegion != NULL &&
            (ClipRegion->rclBounds.left >= ClipRegion->rclBounds.right ||
             ClipRegion->rclBounds.top >= ClipRegion->rclBounds.bottom))
    {
        /* Nothing to do */
        return TRUE;
    }

    /* Now call the DIB function */
    if (!IntEngEnter(&EnterLeaveSource, psoSource, &InputRect, TRUE, &Translate, &InputObj))
    {
        return FALSE;
    }
    InputRect.left +=  Translate.x;
    InputRect.right +=  Translate.x;
    InputRect.top +=  Translate.y;
    InputRect.bottom +=  Translate.y;

    if (!IntEngEnter(&EnterLeaveDest, psoDest, &OutputRect, FALSE, &Translate, &OutputObj))
    {
        return FALSE;
    }
    OutputRect.left += Translate.x;
    OutputRect.right += Translate.x;
    OutputRect.top += Translate.y;
    OutputRect.bottom += Translate.y;

    Ret = FALSE;
    ClippingType = (ClipRegion == NULL) ? DC_TRIVIAL : ClipRegion->iDComplexity;
    switch (ClippingType)
    {
        case DC_TRIVIAL:
            Ret = DibFunctionsForBitmapFormat[OutputObj->iBitmapFormat].DIB_AlphaBlend(
                      OutputObj, InputObj, &OutputRect, &InputRect, ClipRegion, ColorTranslation, BlendObj);
            break;

        case DC_RECT:
            ClipRect.left = ClipRegion->rclBounds.left + Translate.x;
            ClipRect.right = ClipRegion->rclBounds.right + Translate.x;
            ClipRect.top = ClipRegion->rclBounds.top + Translate.y;
            ClipRect.bottom = ClipRegion->rclBounds.bottom + Translate.y;
            if (RECTL_bIntersectRect(&CombinedRect, &OutputRect, &ClipRect))
            {
                Rect.left = InputRect.left + CombinedRect.left - OutputRect.left;
                Rect.right = InputRect.right + CombinedRect.right - OutputRect.right;
                Rect.top = InputRect.top + CombinedRect.top - OutputRect.top;
                Rect.bottom = InputRect.bottom + CombinedRect.bottom - OutputRect.bottom;
                Ret = DibFunctionsForBitmapFormat[OutputObj->iBitmapFormat].DIB_AlphaBlend(
                          OutputObj, InputObj, &CombinedRect, &Rect, ClipRegion, ColorTranslation, BlendObj);
            }
            break;

        case DC_COMPLEX:
            Ret = TRUE;
            CLIPOBJ_cEnumStart(ClipRegion, FALSE, CT_RECTANGLES, CD_ANY, 0);
            do
            {
                EnumMore = CLIPOBJ_bEnum(ClipRegion,(ULONG) sizeof(RectEnum),
                                         (PVOID) &RectEnum);

                for (i = 0; i < RectEnum.c; i++)
                {
                    ClipRect.left = RectEnum.arcl[i].left + Translate.x;
                    ClipRect.right = RectEnum.arcl[i].right + Translate.x;
                    ClipRect.top = RectEnum.arcl[i].top + Translate.y;
                    ClipRect.bottom = RectEnum.arcl[i].bottom + Translate.y;
                    if (RECTL_bIntersectRect(&CombinedRect, &OutputRect, &ClipRect))
                    {
                        Rect.left = InputRect.left + CombinedRect.left - OutputRect.left;
                        Rect.right = InputRect.right + CombinedRect.right - OutputRect.right;
                        Rect.top = InputRect.top + CombinedRect.top - OutputRect.top;
                        Rect.bottom = InputRect.bottom + CombinedRect.bottom - OutputRect.bottom;
                        Ret = DibFunctionsForBitmapFormat[OutputObj->iBitmapFormat].DIB_AlphaBlend(
                                  OutputObj, InputObj, &CombinedRect, &Rect, ClipRegion, ColorTranslation, BlendObj) && Ret;
                    }
                }
            }
            while (EnumMore);
            break;

        default:
            UNIMPLEMENTED;
            ASSERT(FALSE);
            break;
    }

    IntEngLeave(&EnterLeaveDest);
    IntEngLeave(&EnterLeaveSource);

    return Ret;
}

BOOL APIENTRY
IntEngAlphaBlend(IN SURFOBJ *psoDest,
                 IN SURFOBJ *psoSource,
                 IN CLIPOBJ *ClipRegion,
                 IN XLATEOBJ *ColorTranslation,
                 IN PRECTL DestRect,
                 IN PRECTL SourceRect,
                 IN BLENDOBJ *BlendObj)
{
    BOOL ret = FALSE;
    SURFACE *psurfDest;
    SURFACE *psurfSource;

    ASSERT(psoDest);
    psurfDest = CONTAINING_RECORD(psoDest, SURFACE, SurfObj);

    ASSERT(psoSource);
    psurfSource = CONTAINING_RECORD(psoSource, SURFACE, SurfObj);

    ASSERT(DestRect);
    ASSERT(SourceRect);

    /* Check if there is anything to draw */
    if (ClipRegion != NULL &&
            (ClipRegion->rclBounds.left >= ClipRegion->rclBounds.right ||
             ClipRegion->rclBounds.top >= ClipRegion->rclBounds.bottom))
    {
        /* Nothing to do */
        return TRUE;
    }

    /* Call the driver's DrvAlphaBlend if available */
    if (psurfDest->flags & HOOK_ALPHABLEND)
    {
        ret = GDIDEVFUNCS(psoDest).AlphaBlend(
                  psoDest, psoSource, ClipRegion, ColorTranslation,
                  DestRect, SourceRect, BlendObj);
    }

    if (! ret)
    {
        ret = EngAlphaBlend(psoDest, psoSource, ClipRegion, ColorTranslation,
                            DestRect, SourceRect, BlendObj);
    }

    return ret;
}

/*
 * @implemented
 */
BOOL
APIENTRY
NtGdiEngAlphaBlend(IN SURFOBJ *psoDest,
                   IN SURFOBJ *psoSource,
                   IN CLIPOBJ *ClipRegion,
                   IN XLATEOBJ *ColorTranslation,
                   IN PRECTL upDestRect,
                   IN PRECTL upSourceRect,
                   IN BLENDOBJ *BlendObj)
{
    RECTL DestRect;
    RECTL SourceRect;

    _SEH2_TRY
    {
        ProbeForRead(upDestRect, sizeof(RECTL), 1);
        RtlCopyMemory(&DestRect,upDestRect, sizeof(RECTL));

        ProbeForRead(upSourceRect, sizeof(RECTL), 1);
        RtlCopyMemory(&SourceRect, upSourceRect, sizeof(RECTL));

    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        _SEH2_YIELD(return FALSE);
    }
    _SEH2_END;

    return EngAlphaBlend(psoDest, psoSource, ClipRegion, ColorTranslation, &DestRect, &SourceRect, BlendObj);
}

