/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          Support for physical devices
 * FILE:             subsystems/win32/win32k/eng/pdevobj.c
 * PROGRAMER:        Timo Kreuzer (timo.kreuzer@odyssey.org)
 */

#include <win32k.h>

#include <intrin.h>

#define NDEBUG
#include <debug.h>

PPDEVOBJ gppdevPrimary = NULL;

static PPDEVOBJ gppdevList = NULL;
static HSEMAPHORE ghsemPDEV;

INIT_FUNCTION
NTSTATUS
NTAPI
InitPDEVImpl()
{
    ghsemPDEV = EngCreateSemaphore();
    if (!ghsemPDEV) return STATUS_INSUFFICIENT_RESOURCES;
    return STATUS_SUCCESS;
}


PPDEVOBJ
PDEVOBJ_AllocPDEV()
{
    PPDEVOBJ ppdev;

    ppdev = ExAllocatePoolWithTag(PagedPool, sizeof(PDEVOBJ), GDITAG_PDEV);
    if (!ppdev)
        return NULL;

    RtlZeroMemory(ppdev, sizeof(PDEVOBJ));

    ppdev->cPdevRefs = 1;

    return ppdev;
}

VOID
NTAPI
PDEVOBJ_vRelease(PPDEVOBJ ppdev)
{
    /* Lock loader */
    EngAcquireSemaphore(ghsemPDEV);

    /* Decrease reference count */
    --ppdev->cPdevRefs;

    ASSERT(ppdev->cPdevRefs >= 0) ;

    /* Check if references are left */
    if (ppdev->cPdevRefs == 0)
    {
        /* Do we have a surface? */
        if(ppdev->pSurface)
        {
            /* Release the surface and let the driver free it */
            SURFACE_ShareUnlockSurface(ppdev->pSurface);
            ppdev->pfn.DisableSurface(ppdev->dhpdev);
        }

        /* Do we have a palette? */
        if(ppdev->ppalSurf)
        {
            PALETTE_ShareUnlockPalette(ppdev->ppalSurf);
        }

        /* Disable PDEV */
        ppdev->pfn.DisablePDEV(ppdev->dhpdev);

        /* Remove it from list */
        if( ppdev == gppdevList )
            gppdevList = ppdev->ppdevNext ;
        else
        {
            PPDEVOBJ ppdevCurrent = gppdevList;
            BOOL found = FALSE ;
            while (!found && ppdevCurrent->ppdevNext)
            {
                if (ppdevCurrent->ppdevNext == ppdev)
                    found = TRUE;
                else
                    ppdevCurrent = ppdevCurrent->ppdevNext ;
            }
            if(found)
                ppdevCurrent->ppdevNext = ppdev->ppdevNext;
        }

        /* Is this the primary one ? */
        if (ppdev == gppdevPrimary)
            gppdevPrimary = NULL;

        /* Free it */
        ExFreePoolWithTag(ppdev, GDITAG_PDEV );
    }

    /* Unlock loader */
    EngReleaseSemaphore(ghsemPDEV);

}

BOOL
NTAPI
PDEVOBJ_bEnablePDEV(
    PPDEVOBJ ppdev,
    PDEVMODEW pdevmode,
    PWSTR pwszLogAddress)
{
    PFN_DrvEnablePDEV pfnEnablePDEV;

    DPRINT("PDEVOBJ_bEnablePDEV()\n");

    /* Get the DrvEnablePDEV function */
    pfnEnablePDEV = ppdev->pldev->pfn.EnablePDEV;

    /* Call the drivers DrvEnablePDEV function */
    ppdev->dhpdev = pfnEnablePDEV(pdevmode,
                                  pwszLogAddress,
                                  HS_DDI_MAX,
                                  ppdev->ahsurf,
                                  sizeof(GDIINFO),
                                  &ppdev->gdiinfo,
                                  sizeof(DEVINFO),
                                  &ppdev->devinfo,
                                  (HDEV)ppdev,
                                  ppdev->pGraphicsDevice->pwszDescription,
                                  ppdev->pGraphicsDevice->DeviceObject);

    /* Fix up some values */
    if (ppdev->gdiinfo.ulLogPixelsX == 0)
        ppdev->gdiinfo.ulLogPixelsX = 96;

    if (ppdev->gdiinfo.ulLogPixelsY == 0)
        ppdev->gdiinfo.ulLogPixelsY = 96;

    /* Setup Palette */
    ppdev->ppalSurf = PALETTE_ShareLockPalette(ppdev->devinfo.hpalDefault);

    DPRINT("PDEVOBJ_bEnablePDEV - dhpdev = %p\n", ppdev->dhpdev);

    return TRUE;
}

VOID
NTAPI
PDEVOBJ_vCompletePDEV(
    PPDEVOBJ ppdev)
{
    /* Call the drivers DrvCompletePDEV function */
    ppdev->pldev->pfn.CompletePDEV(ppdev->dhpdev, (HDEV)ppdev);
}

PSURFACE
NTAPI
PDEVOBJ_pSurface(
    PPDEVOBJ ppdev)
{
    HSURF hsurf;

    /* Check if we already have a surface */
    if (ppdev->pSurface)
    {
        /* Increment reference count */
        GDIOBJ_vReferenceObjectByPointer(&ppdev->pSurface->BaseObject);
    }
    else
    {
        /* Call the drivers DrvEnableSurface */
        hsurf = ppdev->pldev->pfn.EnableSurface(ppdev->dhpdev);

        /* Lock the surface */
        ppdev->pSurface = SURFACE_ShareLockSurface(hsurf);
    }

    DPRINT("PDEVOBJ_pSurface() returning %p\n", ppdev->pSurface);
    return ppdev->pSurface;
}

PDEVMODEW
NTAPI
PDEVOBJ_pdmMatchDevMode(
    PPDEVOBJ ppdev,
    PDEVMODEW pdm)
{
    PGRAPHICS_DEVICE pGraphicsDevice;
    PDEVMODEW pdmCurrent;
    INT i;
    DWORD dwFields;

    pGraphicsDevice = ppdev->pGraphicsDevice;

    for (i = 0; i < pGraphicsDevice->cDevModes; i++)
    {
        pdmCurrent = pGraphicsDevice->pDevModeList[i].pdm;

        /* Compare asked DEVMODE fields
         * Only compare those that are valid in both DEVMODE structs */
        dwFields = pdmCurrent->dmFields & pdm->dmFields ;
        /* For now, we only need those */
        if ((dwFields & DM_BITSPERPEL) &&
                (pdmCurrent->dmBitsPerPel != pdm->dmBitsPerPel))
            continue;
        if ((dwFields & DM_PELSWIDTH) &&
                (pdmCurrent->dmPelsWidth != pdm->dmPelsWidth))
            continue;
        if ((dwFields & DM_PELSHEIGHT) &&
                (pdmCurrent->dmPelsHeight != pdm->dmPelsHeight))
            continue;
        if ((dwFields & DM_DISPLAYFREQUENCY) &&
                (pdmCurrent->dmDisplayFrequency != pdm->dmDisplayFrequency))
            continue;

        /* Match! Return the DEVMODE */
        return pdmCurrent;
    }

    /* Nothing found */
    return NULL;
}

static
PPDEVOBJ
EngpCreatePDEV(
    PUNICODE_STRING pustrDeviceName,
    PDEVMODEW pdm)
{
    PGRAPHICS_DEVICE pGraphicsDevice;
    PPDEVOBJ ppdev;

    /* Try to find the GRAPHICS_DEVICE */
    if (pustrDeviceName)
    {
        pGraphicsDevice = EngpFindGraphicsDevice(pustrDeviceName, 0, 0);
        if (!pGraphicsDevice)
        {
            DPRINT1("No GRAPHICS_DEVICE found for %ls!\n",
                    pustrDeviceName ? pustrDeviceName->Buffer : 0);
            return NULL;
        }
    }
    else
    {
        pGraphicsDevice = gpPrimaryGraphicsDevice;
    }

    /* Allocate a new PDEVOBJ */
    ppdev = PDEVOBJ_AllocPDEV();
    if (!ppdev)
    {
        DPRINT1("failed to allocate a PDEV\n");
        return NULL;
    }

    /* If no DEVMODEW is given, ... */
    if (!pdm)
    {
        /* ... use the device's default one */
        pdm = pGraphicsDevice->pDevModeList[pGraphicsDevice->iDefaultMode].pdm;
        DPRINT("Using iDefaultMode = %ld\n", pGraphicsDevice->iDefaultMode);
    }

    /* Try to get a diplay driver */
    ppdev->pldev = EngLoadImageEx(pdm->dmDeviceName, LDEV_DEVICE_DISPLAY);
    if (!ppdev->pldev)
    {
        DPRINT1("Could not load display driver '%ls'\n",
                pGraphicsDevice->pDiplayDrivers);
        ExFreePoolWithTag(ppdev, GDITAG_PDEV);
        return NULL;
    }

    /* Copy the function table */
    ppdev->pfn = ppdev->pldev->pfn;

    /* Set MovePointer function */
    ppdev->pfnMovePointer = ppdev->pfn.MovePointer;
    if (!ppdev->pfnMovePointer)
        ppdev->pfnMovePointer = EngMovePointer;

    ppdev->pGraphicsDevice = pGraphicsDevice;
    ppdev->hsemDevLock = EngCreateSemaphore();
    // Should we change the ative mode of pGraphicsDevice ?
    ppdev->pdmwDev = PDEVOBJ_pdmMatchDevMode(ppdev, pdm) ;

    /* FIXME! */
    ppdev->flFlags = PDEV_DISPLAY;

    /* HACK: Don't use the pointer */
    ppdev->Pointer.Exclude.right = -1;

    /* Call the driver to enable the PDEV */
    if (!PDEVOBJ_bEnablePDEV(ppdev, pdm, NULL))
    {
        DPRINT1("Failed to enable PDEV!\n");
        ASSERT(FALSE);
    }

    /* FIXME: this must be done in a better way */
    pGraphicsDevice->StateFlags |= DISPLAY_DEVICE_ATTACHED_TO_DESKTOP;

    /* Tell the driver that the PDEV is ready */
    PDEVOBJ_vCompletePDEV(ppdev);

    /* Return the PDEV */
    return ppdev;
}

VOID
NTAPI
PDEVOBJ_vSwitchPdev(
    PPDEVOBJ ppdev,
    PPDEVOBJ ppdev2)
{
    PDEVOBJ pdevTmp;
    DWORD tmpStateFlags;

    /* Exchange data */
    pdevTmp = *ppdev;

    /* Exchange driver functions */
    ppdev->pfn = ppdev2->pfn;
    ppdev2->pfn = pdevTmp.pfn;

    /* Exchange LDEVs */
    ppdev->pldev = ppdev2->pldev;
    ppdev2->pldev = pdevTmp.pldev;

    /* Exchange DHPDEV */
    ppdev->dhpdev = ppdev2->dhpdev;
    ppdev2->dhpdev = pdevTmp.dhpdev;

    /* Exchange surfaces and associate them with their new PDEV */
    ppdev->pSurface = ppdev2->pSurface;
    ppdev2->pSurface = pdevTmp.pSurface;
    ppdev->pSurface->SurfObj.hdev = (HDEV)ppdev;
    ppdev2->pSurface->SurfObj.hdev = (HDEV)ppdev2;

    /* Exchange devinfo */
    ppdev->devinfo = ppdev2->devinfo;
    ppdev2->devinfo = pdevTmp.devinfo;

    /* Exchange gdiinfo */
    ppdev->gdiinfo = ppdev2->gdiinfo;
    ppdev2->gdiinfo = pdevTmp.gdiinfo;

    /* Exchange DEVMODE */
    ppdev->pdmwDev = ppdev2->pdmwDev;
    ppdev2->pdmwDev = pdevTmp.pdmwDev;

    /* Exchange state flags */
    tmpStateFlags = ppdev->pGraphicsDevice->StateFlags;
    ppdev->pGraphicsDevice->StateFlags = ppdev2->pGraphicsDevice->StateFlags;
    ppdev2->pGraphicsDevice->StateFlags = tmpStateFlags;

    /* Notify each driver instance of its new HDEV association */
    ppdev->pfn.CompletePDEV(ppdev->dhpdev, (HDEV)ppdev);
    ppdev2->pfn.CompletePDEV(ppdev2->dhpdev, (HDEV)ppdev2);
}


BOOL
NTAPI
PDEVOBJ_bSwitchMode(
    PPDEVOBJ ppdev,
    PDEVMODEW pdm)
{
    UNICODE_STRING ustrDevice;
    PPDEVOBJ ppdevTmp;
    PSURFACE pSurface;
    BOOL retval = FALSE;

    /* Lock the PDEV */
    EngAcquireSemaphore(ppdev->hsemDevLock);
    /* And everything else */
    EngAcquireSemaphore(ghsemPDEV);

    DPRINT1("PDEVOBJ_bSwitchMode, ppdev = %p, pSurface = %p\n", ppdev, ppdev->pSurface);

    // Lookup the GraphicsDevice + select DEVMODE
    // pdm = PDEVOBJ_pdmMatchDevMode(ppdev, pdm);

    /* 1. Temporarily disable the current PDEV */
    if (!ppdev->pfn.AssertMode(ppdev->dhpdev, FALSE))
    {
        DPRINT1("DrvAssertMode failed\n");
        goto leave;
    }

    /* 2. Create new PDEV */
    RtlInitUnicodeString(&ustrDevice, ppdev->pGraphicsDevice->szWinDeviceName);
    ppdevTmp = EngpCreatePDEV(&ustrDevice, pdm);
    if (!ppdevTmp)
    {
        DPRINT1("Failed to create a new PDEV\n");
        goto leave;
    }

    /* 3. Create a new surface */
    pSurface = PDEVOBJ_pSurface(ppdevTmp);
    if (!pSurface)
    {
        DPRINT1("DrvEnableSurface failed\n");
        goto leave;
    }

    /* 4. Get DirectDraw information */
    /* 5. Enable DirectDraw Not traced */
    /* 6. Copy old PDEV state to new PDEV instance */

    /* 7. Switch the PDEVs */
    PDEVOBJ_vSwitchPdev(ppdev, ppdevTmp);

    /* 8. Disable DirectDraw */

    PDEVOBJ_vRelease(ppdevTmp);

    /* Update primary display capabilities */
    if(ppdev == gppdevPrimary)
    {
        PDEVOBJ_vGetDeviceCaps(ppdev, &GdiHandleTable->DevCaps);
    }

    /* Success! */
    retval = TRUE;
leave:
    /* Unlock PDEV */
    EngReleaseSemaphore(ppdev->hsemDevLock);
    EngReleaseSemaphore(ghsemPDEV);

    DPRINT1("leave, ppdev = %p, pSurface = %p\n", ppdev, ppdev->pSurface);

    return retval;
}


PPDEVOBJ
NTAPI
EngpGetPDEV(
    PUNICODE_STRING pustrDeviceName)
{
    UNICODE_STRING ustrCurrent;
    PPDEVOBJ ppdev;
    PGRAPHICS_DEVICE pGraphicsDevice;

    /* Acquire PDEV lock */
    EngAcquireSemaphore(ghsemPDEV);

    /* If no device name is given, ... */
    if (!pustrDeviceName && gppdevPrimary)
    {
        /* ... use the primary PDEV */
        ppdev = gppdevPrimary;

        /* Reference the pdev */
        InterlockedIncrement(&ppdev->cPdevRefs);
        goto leave;
    }

    /* Loop all present PDEVs */
    for (ppdev = gppdevList; ppdev; ppdev = ppdev->ppdevNext)
    {
        /* Get a pointer to the GRAPHICS_DEVICE */
        pGraphicsDevice = ppdev->pGraphicsDevice;

        /* Compare the name */
        RtlInitUnicodeString(&ustrCurrent, pGraphicsDevice->szWinDeviceName);
        if (RtlEqualUnicodeString(pustrDeviceName, &ustrCurrent, FALSE))
        {
            /* Found! Reference the PDEV */
            InterlockedIncrement(&ppdev->cPdevRefs);
            break;
        }
    }

    /* Did we find one? */
    if (!ppdev)
    {
        /* No, create a new PDEV */
        ppdev = EngpCreatePDEV(pustrDeviceName, NULL);
        if (ppdev)
        {
            /* Insert the PDEV into the list */
            ppdev->ppdevNext = gppdevList;
            gppdevList = ppdev;

            /* Set as primary PDEV, if we don't have one yet */
            if (!gppdevPrimary)
            {
                gppdevPrimary = ppdev;
                ppdev->pGraphicsDevice->StateFlags |= DISPLAY_DEVICE_PRIMARY_DEVICE;
            }
        }
    }

leave:
    /* Release PDEV lock */
    EngReleaseSemaphore(ghsemPDEV);

    return ppdev;
}

INT
NTAPI
PDEVOBJ_iGetColorManagementCaps(PPDEVOBJ ppdev)
{
    INT ret = CM_NONE;

    if (ppdev->flFlags & PDEV_DISPLAY)
    {
        if (ppdev->devinfo.iDitherFormat == BMF_8BPP ||
            ppdev->devinfo.flGraphicsCaps2 & GCAPS2_CHANGEGAMMARAMP)
            ret = CM_GAMMA_RAMP;
    }

    if (ppdev->devinfo.flGraphicsCaps & GCAPS_CMYKCOLOR)
        ret |= CM_CMYK_COLOR;
    if (ppdev->devinfo.flGraphicsCaps & GCAPS_ICM)
        ret |= CM_DEVICE_ICM;

    return ret;
}

VOID
NTAPI
PDEVOBJ_vGetDeviceCaps(
    IN PPDEVOBJ ppdev,
    OUT PDEVCAPS pDevCaps)
{
    PGDIINFO pGdiInfo = &ppdev->gdiinfo;

    pDevCaps->ulVersion = pGdiInfo->ulVersion;
    pDevCaps->ulTechnology = pGdiInfo->ulTechnology;
    pDevCaps->ulHorzSizeM = (pGdiInfo->ulHorzSize + 500) / 1000;
    pDevCaps->ulVertSizeM = (pGdiInfo->ulVertSize + 500) / 1000;
    pDevCaps->ulHorzSize = pGdiInfo->ulHorzSize;
    pDevCaps->ulVertSize = pGdiInfo->ulVertSize;
    pDevCaps->ulHorzRes = pGdiInfo->ulHorzRes;
    pDevCaps->ulVertRes = pGdiInfo->ulVertRes;
    pDevCaps->ulBitsPixel = pGdiInfo->cBitsPixel;
    if (pDevCaps->ulBitsPixel == 15) pDevCaps->ulBitsPixel = 16;
    pDevCaps->ulPlanes = pGdiInfo->cPlanes;
    pDevCaps->ulNumPens = pGdiInfo->ulNumColors;
    if (pDevCaps->ulNumPens != -1) pDevCaps->ulNumPens *= 5;
    pDevCaps->ulNumFonts = 0; // PDEVOBJ_cFonts(ppdev);
    pDevCaps->ulNumColors = pGdiInfo->ulNumColors;
    pDevCaps->ulRasterCaps = pGdiInfo->flRaster;
    pDevCaps->ulAspectX = pGdiInfo->ulAspectX;
    pDevCaps->ulAspectY = pGdiInfo->ulAspectY;
    pDevCaps->ulAspectXY = pGdiInfo->ulAspectXY;
    pDevCaps->ulLogPixelsX = pGdiInfo->ulLogPixelsX;
    pDevCaps->ulLogPixelsY = pGdiInfo->ulLogPixelsY;
    pDevCaps->ulSizePalette = pGdiInfo->ulNumPalReg;
    pDevCaps->ulColorRes = pGdiInfo->ulDACRed +
                           pGdiInfo->ulDACGreen +
                           pGdiInfo->ulDACBlue;
    pDevCaps->ulPhysicalWidth = pGdiInfo->szlPhysSize.cx;
    pDevCaps->ulPhysicalHeight = pGdiInfo->szlPhysSize.cy;
    pDevCaps->ulPhysicalOffsetX = pGdiInfo->ptlPhysOffset.x;
    pDevCaps->ulPhysicalOffsetY = pGdiInfo->ptlPhysOffset.y;
    pDevCaps->ulTextCaps = pGdiInfo->flTextCaps;
    pDevCaps->ulTextCaps |= (TC_SO_ABLE|TC_UA_ABLE|TC_CP_STROKE|TC_OP_STROKE|TC_OP_CHARACTER);
    if (pGdiInfo->ulTechnology != DT_PLOTTER)
        pDevCaps->ulTextCaps |= TC_VA_ABLE;
    pDevCaps->ulVRefresh = pGdiInfo->ulVRefresh;
    pDevCaps->ulDesktopHorzRes = pGdiInfo->ulHorzRes;
    pDevCaps->ulDesktopVertRes = pGdiInfo->ulVertRes;
    pDevCaps->ulBltAlignment = pGdiInfo->ulBltAlignment;
    pDevCaps->ulPanningHorzRes = pGdiInfo->ulPanningHorzRes;
    pDevCaps->ulPanningVertRes = pGdiInfo->ulPanningVertRes;
    pDevCaps->xPanningAlignment = pGdiInfo->xPanningAlignment;
    pDevCaps->yPanningAlignment = pGdiInfo->yPanningAlignment;
    pDevCaps->ulShadeBlend = pGdiInfo->flShadeBlend;
    pDevCaps->ulColorMgmtCaps = PDEVOBJ_iGetColorManagementCaps(ppdev);
}


/** Exported functions ********************************************************/

LPWSTR
APIENTRY
EngGetDriverName(IN HDEV hdev)
{
    PPDEVOBJ ppdev = (PPDEVOBJ)hdev;
    PLDEVOBJ pldev;

    if (!hdev)
        return NULL;

    pldev = ppdev->pldev;
    ASSERT(pldev);

    if (!pldev->pGdiDriverInfo)
        return NULL;

    return pldev->pGdiDriverInfo->DriverName.Buffer;
}


INT
APIENTRY
NtGdiGetDeviceCaps(
    HDC hdc,
    INT Index)
{
    PDC pdc;
    DEVCAPS devcaps;

    /* Lock the given DC */
    pdc = DC_LockDc(hdc);
    if (!pdc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    /* Get the data */
    PDEVOBJ_vGetDeviceCaps(pdc->ppdev, &devcaps);

    /* Unlock the DC */
    DC_UnlockDc(pdc);

    /* Return capability */
    switch (Index)
    {
        case DRIVERVERSION:
            return devcaps.ulVersion;

        case TECHNOLOGY:
            return devcaps.ulTechnology;

        case HORZSIZE:
            return devcaps.ulHorzSize;

        case VERTSIZE:
            return devcaps.ulVertSize;

        case HORZRES:
            return devcaps.ulHorzRes;

        case VERTRES:
            return devcaps.ulVertRes;

        case LOGPIXELSX:
            return devcaps.ulLogPixelsX;

        case LOGPIXELSY:
            return devcaps.ulLogPixelsY;

        case BITSPIXEL:
            return devcaps.ulBitsPixel;

        case PLANES:
            return devcaps.ulPlanes;

        case NUMBRUSHES:
            return -1;

        case NUMPENS:
            return devcaps.ulNumPens;

        case NUMFONTS:
            return devcaps.ulNumFonts;

        case NUMCOLORS:
            return devcaps.ulNumColors;

        case ASPECTX:
            return devcaps.ulAspectX;

        case ASPECTY:
            return devcaps.ulAspectY;

        case ASPECTXY:
            return devcaps.ulAspectXY;

        case CLIPCAPS:
            return CP_RECTANGLE;

        case SIZEPALETTE:
            return devcaps.ulSizePalette;

        case NUMRESERVED:
            return 20;

        case COLORRES:
            return devcaps.ulColorRes;

        case DESKTOPVERTRES:
            return devcaps.ulVertRes;

        case DESKTOPHORZRES:
            return devcaps.ulHorzRes;

        case BLTALIGNMENT:
            return devcaps.ulBltAlignment;

        case SHADEBLENDCAPS:
            return devcaps.ulShadeBlend;

        case COLORMGMTCAPS:
            return devcaps.ulColorMgmtCaps;

        case PHYSICALWIDTH:
            return devcaps.ulPhysicalWidth;

        case PHYSICALHEIGHT:
            return devcaps.ulPhysicalHeight;

        case PHYSICALOFFSETX:
            return devcaps.ulPhysicalOffsetX;

        case PHYSICALOFFSETY:
            return devcaps.ulPhysicalOffsetY;

        case VREFRESH:
            return devcaps.ulVRefresh;

        case RASTERCAPS:
            return devcaps.ulRasterCaps;

        case CURVECAPS:
            return (CC_CIRCLES | CC_PIE | CC_CHORD | CC_ELLIPSES | CC_WIDE |
                    CC_STYLED | CC_WIDESTYLED | CC_INTERIORS | CC_ROUNDRECT);

        case LINECAPS:
            return (LC_POLYLINE | LC_MARKER | LC_POLYMARKER | LC_WIDE |
                    LC_STYLED | LC_WIDESTYLED | LC_INTERIORS);

        case POLYGONALCAPS:
            return (PC_POLYGON | PC_RECTANGLE | PC_WINDPOLYGON | PC_SCANLINE |
                    PC_WIDE | PC_STYLED | PC_WIDESTYLED | PC_INTERIORS);

        case TEXTCAPS:
            return devcaps.ulTextCaps;

        case CAPS1:
        case PDEVICESIZE:
        case SCALINGFACTORX:
        case SCALINGFACTORY:
        default:
            return 0;
    }

    return 0;
}


BOOL
APIENTRY
NtGdiGetDeviceCapsAll(
    IN HDC hDC,
    OUT PDEVCAPS pDevCaps)
{
    PDC pdc;
    DEVCAPS devcaps;
    BOOL bResult = TRUE;

    /* Lock the given DC */
    pdc = DC_LockDc(hDC);
    if (!pdc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    /* Get the data */
    PDEVOBJ_vGetDeviceCaps(pdc->ppdev, &devcaps);

    /* Unlock the DC */
    DC_UnlockDc(pdc);

    /* Copy data to caller */
    _SEH2_TRY
    {
        ProbeForWrite(pDevCaps, sizeof(DEVCAPS), 1);
        RtlCopyMemory(pDevCaps, &devcaps, sizeof(DEVCAPS));
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastNtError(_SEH2_GetExceptionCode());
        bResult = FALSE;
    }
    _SEH2_END;

    return bResult;
}

DHPDEV
APIENTRY
NtGdiGetDhpdev(
    IN HDEV hdev)
{
    PPDEVOBJ ppdev;
    DHPDEV dhpdev = NULL;

    /* Check parameter */
    if (!hdev || (PCHAR)hdev < (PCHAR)MmSystemRangeStart)
        return NULL;

    /* Lock PDEV list */
    EngAcquireSemaphore(ghsemPDEV);

    /* Walk through the list of PDEVs */
    for (ppdev = gppdevList;  ppdev; ppdev = ppdev->ppdevNext)
    {
        /* Compare with the given HDEV */
        if (ppdev == hdev)
        {
            /* Found the PDEV! Get it's dhpdev and break */
            dhpdev = ppdev->dhpdev;
            break;
        }
    }

    /* Unlock PDEV list */
    EngReleaseSemaphore(ghsemPDEV);

    return dhpdev;
}

PSIZEL
FASTCALL
PDEVOBJ_sizl(PPDEVOBJ ppdev, PSIZEL psizl)
{
    if (ppdev->flFlags & PDEV_META_DEVICE)
    {
        psizl->cx = ppdev->ulHorzRes;
        psizl->cy = ppdev->ulVertRes;
    }
    else
    {
        psizl->cx = ppdev->gdiinfo.ulHorzRes;
        psizl->cy = ppdev->gdiinfo.ulVertRes;
    }
    return psizl;
}
