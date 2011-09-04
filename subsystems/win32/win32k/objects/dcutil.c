
#include <win32k.h>

#define NDEBUG
#include <debug.h>

COLORREF FASTCALL
IntGdiSetBkColor(HDC hDC, COLORREF color)
{
    COLORREF oldColor;
    PDC dc;
    PDC_ATTR pdcattr;
    HBRUSH hBrush;

    if (!(dc = DC_LockDc(hDC)))
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return CLR_INVALID;
    }
    pdcattr = dc->pdcattr;
    oldColor = pdcattr->crBackgroundClr;
    pdcattr->crBackgroundClr = color;
    pdcattr->ulBackgroundClr = (ULONG)color;
    pdcattr->ulDirty_ &= ~(DIRTY_BACKGROUND|DIRTY_LINE|DIRTY_FILL); // Clear Flag if set.
    hBrush = pdcattr->hbrush;
    DC_UnlockDc(dc);
    NtGdiSelectBrush(hDC, hBrush);
    return oldColor;
}

INT FASTCALL
IntGdiSetBkMode(HDC hDC, INT Mode)
{
    COLORREF oldMode;
    PDC dc;
    PDC_ATTR pdcattr;

    if (!(dc = DC_LockDc(hDC)))
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return CLR_INVALID;
    }
    pdcattr = dc->pdcattr;
    oldMode = pdcattr->lBkMode;
    pdcattr->jBkMode = Mode;
    pdcattr->lBkMode = Mode;
    DC_UnlockDc(dc);
    return oldMode;
}

UINT
FASTCALL
IntGdiSetTextAlign(HDC  hDC,
                   UINT  Mode)
{
    UINT prevAlign;
    DC *dc;
    PDC_ATTR pdcattr;

    dc = DC_LockDc(hDC);
    if (!dc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return GDI_ERROR;
    }
    pdcattr = dc->pdcattr;
    prevAlign = pdcattr->lTextAlign;
    pdcattr->lTextAlign = Mode;
    DC_UnlockDc(dc);
    return  prevAlign;
}

COLORREF
FASTCALL
IntGdiSetTextColor(HDC hDC,
                   COLORREF color)
{
    COLORREF crOldColor;
    PDC pdc;
    PDC_ATTR pdcattr;

    pdc = DC_LockDc(hDC);
    if (!pdc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return CLR_INVALID;
    }
    pdcattr = pdc->pdcattr;

    // What about ulForegroundClr, like in gdi32?
    crOldColor = pdcattr->crForegroundClr;
    pdcattr->crForegroundClr = color;
    DC_vUpdateTextBrush(pdc);

    DC_UnlockDc(pdc);

    return  crOldColor;
}

VOID
FASTCALL
DCU_SetDcUndeletable(HDC  hDC)
{
    PDC dc = DC_LockDc(hDC);
    if (!dc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return;
    }

    dc->fs |= DC_FLAG_PERMANENT;
    DC_UnlockDc(dc);
    return;
}

#if 0
BOOL FASTCALL
IntIsPrimarySurface(SURFOBJ *SurfObj)
{
    if (PrimarySurface.pSurface == NULL)
    {
        return FALSE;
    }
    return SurfObj->hsurf == PrimarySurface.pSurface; // <- FIXME: WTF?
}
#endif

BOOL
FASTCALL
IntSetDefaultRegion(PDC pdc)
{
    PSURFACE pSurface;
    PROSRGNDATA prgn;
    RECTL rclWnd, rclClip;

    IntGdiReleaseRaoRgn(pdc);

    rclWnd.left   = 0;
    rclWnd.top    = 0;
    rclWnd.right  = pdc->dclevel.sizl.cx;
    rclWnd.bottom = pdc->dclevel.sizl.cy;
    rclClip = rclWnd;

//    EngAcquireSemaphoreShared(pdc->ppdev->hsemDevLock);
    if (pdc->ppdev->flFlags & PDEV_META_DEVICE)
    {
        pSurface = pdc->dclevel.pSurface;
        if (pSurface && pSurface->flags & PDEV_SURFACE)
        {
            rclClip.left   += pdc->ppdev->ptlOrigion.x;
            rclClip.top    += pdc->ppdev->ptlOrigion.y;
            rclClip.right  += pdc->ppdev->ptlOrigion.x;
            rclClip.bottom += pdc->ppdev->ptlOrigion.y;
        }
    }
//    EngReleaseSemaphore(pdc->ppdev->hsemDevLock);

    prgn = pdc->prgnVis;

    if (prgn && prgn != prgnDefault)
    {
        REGION_SetRectRgn( prgn,
                           rclClip.left,
                           rclClip.top,
                           rclClip.right ,
                           rclClip.bottom );
    }
    else
    {
        prgn = IntSysCreateRectpRgn( rclClip.left,
                                     rclClip.top,
                                     rclClip.right ,
                                     rclClip.bottom );
        pdc->prgnVis = prgn;
    }

    if (prgn)
    {
        pdc->ptlDCOrig.x = 0;
        pdc->ptlDCOrig.y = 0;
        pdc->erclWindow = rclWnd;
        pdc->erclClip = rclClip;
        /* Might be an InitDC or DCE....*/
        pdc->ptlFillOrigin.x = pdc->dcattr.VisRectRegion.Rect.right;
        pdc->ptlFillOrigin.y = pdc->dcattr.VisRectRegion.Rect.bottom;
        return TRUE;
    }

    pdc->prgnVis = prgnDefault;
    return FALSE;
}


BOOL APIENTRY
NtGdiCancelDC(HDC  hDC)
{
    UNIMPLEMENTED;
    return FALSE;
}


WORD APIENTRY
IntGdiSetHookFlags(HDC hDC, WORD Flags)
{
    WORD wRet;
    DC *dc = DC_LockDc(hDC);

    if (NULL == dc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    wRet = dc->fs & DC_FLAG_DIRTY_RAO; // Fixme wrong flag!

    /* "Undocumented Windows" info is slightly confusing.
     */

    DPRINT("DC %p, Flags %04x\n", hDC, Flags);

    if (Flags & DCHF_INVALIDATEVISRGN)
    {
        /* hVisRgn has to be updated */
        dc->fs |= DC_FLAG_DIRTY_RAO;
    }
    else if (Flags & DCHF_VALIDATEVISRGN || 0 == Flags)
    {
        dc->fs &= ~DC_FLAG_DIRTY_RAO;
    }

    DC_UnlockDc(dc);

    return wRet;
}


BOOL
APIENTRY
NtGdiGetDCDword(
    HDC hDC,
    UINT u,
    DWORD *Result)
{
    BOOL Ret = TRUE;
    PDC pdc;
    PDC_ATTR pdcattr;

    DWORD SafeResult = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    if (!Result)
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pdc = DC_LockDc(hDC);
    if (!pdc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    pdcattr = pdc->pdcattr;

    switch (u)
    {
        case GdiGetJournal:
            break;

        case GdiGetRelAbs:
            SafeResult = pdcattr->lRelAbs;
            break;

        case GdiGetBreakExtra:
            SafeResult = pdcattr->lBreakExtra;
            break;

        case GdiGerCharBreak:
            SafeResult = pdcattr->cBreak;
            break;

        case GdiGetArcDirection:
            if (pdcattr->dwLayout & LAYOUT_RTL)
                SafeResult = AD_CLOCKWISE - ((pdc->dclevel.flPath & DCPATH_CLOCKWISE) != 0);
            else
                SafeResult = ((pdc->dclevel.flPath & DCPATH_CLOCKWISE) != 0) + AD_COUNTERCLOCKWISE;
            break;

        case GdiGetEMFRestorDc:
            break;

        case GdiGetFontLanguageInfo:
            SafeResult = IntGetFontLanguageInfo(pdc);
            break;

        case GdiGetIsMemDc:
            SafeResult = pdc->dctype;
            break;

        case GdiGetMapMode:
            SafeResult = pdcattr->iMapMode;
            break;

        case GdiGetTextCharExtra:
            SafeResult = pdcattr->lTextExtra;
            break;

        default:
            EngSetLastError(ERROR_INVALID_PARAMETER);
            Ret = FALSE;
            break;
    }

    if (Ret)
    {
        _SEH2_TRY
        {
            ProbeForWrite(Result, sizeof(DWORD), 1);
            *Result = SafeResult;
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = _SEH2_GetExceptionCode();
        }
        _SEH2_END;

        if (!NT_SUCCESS(Status))
        {
            SetLastNtError(Status);
            Ret = FALSE;
        }
    }

    DC_UnlockDc(pdc);
    return Ret;
}

BOOL
APIENTRY
NtGdiGetAndSetDCDword(
    HDC hDC,
    UINT u,
    DWORD dwIn,
    DWORD *Result)
{
    BOOL Ret = TRUE;
    PDC pdc;
    PDC_ATTR pdcattr;

    DWORD SafeResult = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    if (!Result)
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pdc = DC_LockDc(hDC);
    if (!pdc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }
    pdcattr = pdc->pdcattr;

    switch (u)
    {
        case GdiGetSetCopyCount:
            SafeResult = pdc->ulCopyCount;
            pdc->ulCopyCount = dwIn;
            break;

        case GdiGetSetTextAlign:
            SafeResult = pdcattr->lTextAlign;
            pdcattr->lTextAlign = dwIn;
            // pdcattr->flTextAlign = dwIn; // Flags!
            break;

        case GdiGetSetRelAbs:
            SafeResult = pdcattr->lRelAbs;
            pdcattr->lRelAbs = dwIn;
            break;

        case GdiGetSetTextCharExtra:
            SafeResult = pdcattr->lTextExtra;
            pdcattr->lTextExtra = dwIn;
            break;

        case GdiGetSetSelectFont:
            break;

        case GdiGetSetMapperFlagsInternal:
            if (dwIn & ~1)
            {
                EngSetLastError(ERROR_INVALID_PARAMETER);
                Ret = FALSE;
                break;
            }
            SafeResult = pdcattr->flFontMapper;
            pdcattr->flFontMapper = dwIn;
            break;

        case GdiGetSetMapMode:
            SafeResult = IntGdiSetMapMode(pdc, dwIn);
            break;

        case GdiGetSetArcDirection:
            if (dwIn != AD_COUNTERCLOCKWISE && dwIn != AD_CLOCKWISE)
            {
                EngSetLastError(ERROR_INVALID_PARAMETER);
                Ret = FALSE;
                break;
            }
            if (pdcattr->dwLayout & LAYOUT_RTL) // Right to Left
            {
                SafeResult = AD_CLOCKWISE - ((pdc->dclevel.flPath & DCPATH_CLOCKWISE) != 0);
                if (dwIn == AD_CLOCKWISE)
                {
                    pdc->dclevel.flPath &= ~DCPATH_CLOCKWISE;
                    break;
                }
                pdc->dclevel.flPath |= DCPATH_CLOCKWISE;
            }
            else // Left to Right
            {
                SafeResult = ((pdc->dclevel.flPath & DCPATH_CLOCKWISE) != 0) +
                             AD_COUNTERCLOCKWISE;
                if (dwIn == AD_COUNTERCLOCKWISE)
                {
                    pdc->dclevel.flPath &= ~DCPATH_CLOCKWISE;
                    break;
                }
                pdc->dclevel.flPath |= DCPATH_CLOCKWISE;
            }
            break;

        default:
            EngSetLastError(ERROR_INVALID_PARAMETER);
            Ret = FALSE;
            break;
    }

    if (Ret)
    {
        _SEH2_TRY
        {
            ProbeForWrite(Result, sizeof(DWORD), 1);
            *Result = SafeResult;
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = _SEH2_GetExceptionCode();
        }
        _SEH2_END;

        if (!NT_SUCCESS(Status))
        {
            SetLastNtError(Status);
            Ret = FALSE;
        }
    }

    DC_UnlockDc(pdc);
    return Ret;
}

DWORD
APIENTRY
NtGdiGetBoundsRect(
    IN HDC hdc,
    OUT LPRECT prc,
    IN DWORD flags)
{
    DWORD ret;
    PDC pdc;

    /* Lock the DC */
    if (!(pdc = DC_LockDc(hdc))) return 0;

    /* Get the return value */
    ret = pdc->fs & DC_ACCUM_APP ? DCB_ENABLE : DCB_DISABLE;
    ret |= RECTL_bIsEmptyRect(&pdc->erclBoundsApp) ? DCB_RESET : DCB_SET;

    /* Copy the rect to the caller */
    _SEH2_TRY
    {
        ProbeForWrite(prc, sizeof(RECT), 1);
        *prc = pdc->erclBoundsApp;
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        ret = 0;
    }
    _SEH2_END;

    if (flags & DCB_RESET)
    {
        RECTL_vSetEmptyRect(&pdc->erclBoundsApp);
    }

    DC_UnlockDc(pdc);
    return ret;
}


DWORD
APIENTRY
NtGdiSetBoundsRect(
    IN HDC hdc,
    IN LPRECT prc,
    IN DWORD flags)
{
    DWORD ret;
    PDC pdc;
    RECTL rcl;

    /* Verify arguments */
    if ((flags & DCB_ENABLE) && (flags & DCB_DISABLE)) return 0;

    /* Lock the DC */
    if (!(pdc = DC_LockDc(hdc))) return 0;

    /* Get the return value */
    ret = pdc->fs & DC_ACCUM_APP ? DCB_ENABLE : DCB_DISABLE;
    ret |= RECTL_bIsEmptyRect(&pdc->erclBoundsApp) ? DCB_RESET : DCB_SET;

    if (flags & DCB_RESET)
    {
        RECTL_vSetEmptyRect(&pdc->erclBoundsApp);
    }

    if (flags & DCB_ACCUMULATE)
    {
        /* Capture the rect */
        _SEH2_TRY
        {
            ProbeForRead(prc, sizeof(RECT), 1);
            rcl = *prc;
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            DC_UnlockDc(pdc);
            _SEH2_YIELD(return 0;)
        }
        _SEH2_END;

        RECTL_vMakeWellOrdered(&rcl);
        RECTL_bUnionRect(&pdc->erclBoundsApp, &pdc->erclBoundsApp, &rcl);
    }

    if (flags & DCB_ENABLE) pdc->fs |= DC_ACCUM_APP;
    if (flags & DCB_DISABLE) pdc->fs &= ~DC_ACCUM_APP;
    DC_UnlockDc(pdc);
    return ret;
}
