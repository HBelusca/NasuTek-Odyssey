#include "precomp.h"

#define NDEBUG
#include <debug.h>


#define NB_RESERVED_COLORS  20   /* number of fixed colors in system palette */

static const PALETTEENTRY sys_pal_template[NB_RESERVED_COLORS] =
{
    /* first 10 entries in the system palette */
    /* red  green blue  flags */
    { 0x00, 0x00, 0x00, 0 },
    { 0x80, 0x00, 0x00, 0 },
    { 0x00, 0x80, 0x00, 0 },
    { 0x80, 0x80, 0x00, 0 },
    { 0x00, 0x00, 0x80, 0 },
    { 0x80, 0x00, 0x80, 0 },
    { 0x00, 0x80, 0x80, 0 },
    { 0xc0, 0xc0, 0xc0, 0 },
    { 0xc0, 0xdc, 0xc0, 0 },
    { 0xa6, 0xca, 0xf0, 0 },

    /* ... c_min/2 dynamic colorcells */

    /* ... gap (for sparse palettes) */

    /* ... c_min/2 dynamic colorcells */

    { 0xff, 0xfb, 0xf0, 0 },
    { 0xa0, 0xa0, 0xa4, 0 },
    { 0x80, 0x80, 0x80, 0 },
    { 0xff, 0x00, 0x00, 0 },
    { 0x00, 0xff, 0x00, 0 },
    { 0xff, 0xff, 0x00, 0 },
    { 0x00, 0x00, 0xff, 0 },
    { 0xff, 0x00, 0xff, 0 },
    { 0x00, 0xff, 0xff, 0 },
    { 0xff, 0xff, 0xff, 0 }     /* last 10 */
};

BOOL
WINAPI
AnimatePalette(HPALETTE hpal,
               UINT iStartIndex,
               UINT cEntries,
               const PALETTEENTRY *ppe)
{
    return NtGdiDoPalette(hpal, iStartIndex, cEntries, (PALETTEENTRY*)ppe, GdiPalAnimate, TRUE);
}

HPALETTE
WINAPI
CreatePalette(CONST LOGPALETTE * plpal)
{
    return NtGdiCreatePaletteInternal((LPLOGPALETTE)plpal, plpal->palNumEntries);
}

/*
 * @implemented
 */
UINT
WINAPI
GetPaletteEntries(HPALETTE hpal,
                  UINT iStartIndex,
                  UINT cEntries,
                  LPPALETTEENTRY ppe)
{
    return NtGdiDoPalette(hpal, iStartIndex, cEntries, ppe, GdiPalGetEntries, FALSE);
}

UINT
WINAPI
SetPaletteEntries(HPALETTE hpal,
                  UINT iStartIndex,
                  UINT cEntries,
                  const PALETTEENTRY *ppe)
{
    return NtGdiDoPalette(hpal, iStartIndex, cEntries, (PALETTEENTRY*)ppe, GdiPalSetEntries, TRUE);
}

UINT
WINAPI
GetSystemPaletteEntries(HDC hDC,
                        UINT iStartIndex,
                        UINT cEntries,
                        LPPALETTEENTRY ppe)
{
    PALETTEENTRY ippe[256];

    if ((INT)cEntries >= 0)
    {
        if (GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)
        {
            return NtGdiDoPalette(hDC,
                                  iStartIndex,
                                  cEntries,
                                  ppe,
                                  GdiPalGetSystemEntries,
                                  FALSE);
        }
        else if (ppe)
        {
            RtlCopyMemory(ippe, sys_pal_template, 10 * sizeof(PALETTEENTRY));
            RtlCopyMemory(&ippe[246], &sys_pal_template[10], 10 * sizeof(PALETTEENTRY));
            RtlZeroMemory(&ippe[10], sizeof(ippe) - 20 * sizeof(PALETTEENTRY));

            if (iStartIndex < 256)
            {
                RtlCopyMemory(ppe,
                              &ippe[iStartIndex],
                              min(256 - iStartIndex, cEntries) *
                              sizeof(PALETTEENTRY));
            }
        }
    }

    return 0;
}

UINT
WINAPI
GetDIBColorTable(HDC hDC,
                 UINT iStartIndex,
                 UINT cEntries,
                 RGBQUAD *pColors)
{
    if (cEntries)
        return NtGdiDoPalette(hDC, iStartIndex, cEntries, pColors, GdiPalGetColorTable, FALSE);
    return 0;
}

/*
 * @implemented
 */
UINT
WINAPI
RealizePalette(HDC hDC) /* [in] Handle of device context */
{
#if 0
// Handle something other than a normal dc object.
    if (GDI_HANDLE_GET_TYPE(hDC) != GDI_OBJECT_TYPE_DC)
    {
        if (GDI_HANDLE_GET_TYPE(hDC) == GDI_OBJECT_TYPE_METADC)
            return MFDRV_(hDC);
        else
        {
            HPALETTE Pal = GetDCObject(hDC, GDI_OBJECT_TYPE_PALETTE);
            PLDC pLDC = GdiGetLDC((HDC) Pal);
            if ( !pLDC ) return FALSE;
            if (pLDC->iType == LDC_EMFLDC) return EMFDRV_(Pal);
            return FALSE;
        }
    }
#endif
    return UserRealizePalette(hDC);
}

/*
 * @implemented
 */
BOOL
WINAPI
ResizePalette(
    HPALETTE	hPalette,
    UINT		nEntries
)
{
    return NtGdiResizePalette(hPalette, nEntries);
}

/*
 * @implemented
 */
UINT
WINAPI
SetDIBColorTable(HDC hDC,
                 UINT iStartIndex,
                 UINT cEntries,
                 const RGBQUAD *pColors)
{
    UINT retValue=0;

    if (cEntries)
    {
        retValue = NtGdiDoPalette(hDC, iStartIndex, cEntries, (RGBQUAD*)pColors, GdiPalSetColorTable, TRUE);
    }

    return retValue;
}

/*
 * @implemented
 */
BOOL
WINAPI
UpdateColors(
    HDC	hdc
)
{
    ((PW32CLIENTINFO)NtCurrentTeb()->Win32ClientInfo)->cSpins = 0;
    return NtGdiUpdateColors(hdc);
}

/* EOF */
