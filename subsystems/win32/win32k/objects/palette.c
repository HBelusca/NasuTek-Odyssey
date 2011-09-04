/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey kernel
 * PURPOSE:           GDI Palette Functions
 * FILE:              subsys/win32k/eng/palette.c
 * PROGRAMERS:        Jason Filby
 *                    Timo Kreuzer
 */

#include <win32k.h>

#define NDEBUG
#include <debug.h>

static UINT SystemPaletteUse = SYSPAL_NOSTATIC;  /* the program need save the pallete and restore it */

PALETTE gpalRGB, gpalBGR, gpalMono, gpalRGB555, gpalRGB565, *gppalDefault;
PPALETTE appalSurfaceDefault[11];

const PALETTEENTRY g_sysPalTemplate[NB_RESERVED_COLORS] =
{
  // first 10 entries in the system palette
  // red  green blue  flags
  { 0x00, 0x00, 0x00, PC_SYS_USED },
  { 0x80, 0x00, 0x00, PC_SYS_USED },
  { 0x00, 0x80, 0x00, PC_SYS_USED },
  { 0x80, 0x80, 0x00, PC_SYS_USED },
  { 0x00, 0x00, 0x80, PC_SYS_USED },
  { 0x80, 0x00, 0x80, PC_SYS_USED },
  { 0x00, 0x80, 0x80, PC_SYS_USED },
  { 0xc0, 0xc0, 0xc0, PC_SYS_USED },
  { 0xc0, 0xdc, 0xc0, PC_SYS_USED },
  { 0xa6, 0xca, 0xf0, PC_SYS_USED },

  // ... c_min/2 dynamic colorcells
  // ... gap (for sparse palettes)
  // ... c_min/2 dynamic colorcells

  { 0xff, 0xfb, 0xf0, PC_SYS_USED },
  { 0xa0, 0xa0, 0xa4, PC_SYS_USED },
  { 0x80, 0x80, 0x80, PC_SYS_USED },
  { 0xff, 0x00, 0x00, PC_SYS_USED },
  { 0x00, 0xff, 0x00, PC_SYS_USED },
  { 0xff, 0xff, 0x00, PC_SYS_USED },
  { 0x00, 0x00, 0xff, PC_SYS_USED },
  { 0xff, 0x00, 0xff, PC_SYS_USED },
  { 0x00, 0xff, 0xff, PC_SYS_USED },
  { 0xff, 0xff, 0xff, PC_SYS_USED }     // last 10
};

unsigned short GetNumberOfBits(unsigned int dwMask)
{
   unsigned short wBits;
   for (wBits = 0; dwMask; dwMask = dwMask & (dwMask - 1))
      wBits++;
   return wBits;
}

// Create the system palette
INIT_FUNCTION
NTSTATUS
NTAPI
InitPaletteImpl()
{
    int i;
    HPALETTE hpalette;
    PLOGPALETTE palPtr;

    // create default palette (20 system colors)
    palPtr = ExAllocatePoolWithTag(PagedPool,
                                   sizeof(LOGPALETTE) +
                                       (NB_RESERVED_COLORS * sizeof(PALETTEENTRY)),
                                   TAG_PALETTE);
    if (!palPtr) return STATUS_NO_MEMORY;

    palPtr->palVersion = 0x300;
    palPtr->palNumEntries = NB_RESERVED_COLORS;
    for (i=0; i<NB_RESERVED_COLORS; i++)
    {
        palPtr->palPalEntry[i].peRed = g_sysPalTemplate[i].peRed;
        palPtr->palPalEntry[i].peGreen = g_sysPalTemplate[i].peGreen;
        palPtr->palPalEntry[i].peBlue = g_sysPalTemplate[i].peBlue;
        palPtr->palPalEntry[i].peFlags = 0;
    }

    hpalette = NtGdiCreatePaletteInternal(palPtr,NB_RESERVED_COLORS);
    ExFreePoolWithTag(palPtr, TAG_PALETTE);

    /*  palette_size = visual->map_entries; */

    gpalRGB.flFlags = PAL_RGB;
    gpalRGB.RedMask = RGB(0xFF, 0x00, 0x00);
    gpalRGB.GreenMask = RGB(0x00, 0xFF, 0x00);
    gpalRGB.BlueMask = RGB(0x00, 0x00, 0xFF);
    gpalRGB.BaseObject.ulShareCount = 1;
    gpalRGB.BaseObject.BaseFlags = 0 ;

    gpalBGR.flFlags = PAL_BGR;
    gpalBGR.RedMask = RGB(0x00, 0x00, 0xFF);
    gpalBGR.GreenMask = RGB(0x00, 0xFF, 0x00);
    gpalBGR.BlueMask = RGB(0xFF, 0x00, 0x00);
    gpalBGR.BaseObject.ulShareCount = 1;
    gpalBGR.BaseObject.BaseFlags = 0 ;

    gpalRGB555.flFlags = PAL_RGB16_555 | PAL_BITFIELDS;
    gpalRGB555.RedMask = 0x7C00;
    gpalRGB555.GreenMask = 0x3E0;
    gpalRGB555.BlueMask = 0x1F;
    gpalRGB555.BaseObject.ulShareCount = 1;
    gpalRGB555.BaseObject.BaseFlags = 0 ;

    gpalRGB565.flFlags = PAL_RGB16_565 | PAL_BITFIELDS;
    gpalRGB565.RedMask = 0xF800;
    gpalRGB565.GreenMask = 0x7E0;
    gpalRGB565.BlueMask = 0x1F;
    gpalRGB565.BaseObject.ulShareCount = 1;
    gpalRGB565.BaseObject.BaseFlags = 0 ;

    memset(&gpalMono, 0, sizeof(PALETTE));
    gpalMono.flFlags = PAL_MONOCHROME;
    gpalMono.BaseObject.ulShareCount = 1;
    gpalMono.BaseObject.BaseFlags = 0 ;

    /* Initialize default surface palettes */
    gppalDefault = PALETTE_ShareLockPalette(hpalette);
    appalSurfaceDefault[BMF_1BPP] = &gpalMono;
    appalSurfaceDefault[BMF_4BPP] = gppalDefault;
    appalSurfaceDefault[BMF_8BPP] = gppalDefault;
    appalSurfaceDefault[BMF_16BPP] = &gpalRGB565;
    appalSurfaceDefault[BMF_24BPP] = &gpalBGR;
    appalSurfaceDefault[BMF_32BPP] = &gpalBGR;
    appalSurfaceDefault[BMF_4RLE] = gppalDefault;
    appalSurfaceDefault[BMF_8RLE] = gppalDefault;
    appalSurfaceDefault[BMF_JPEG] = &gpalRGB;
    appalSurfaceDefault[BMF_PNG] = &gpalRGB;

    return STATUS_SUCCESS;
}

VOID FASTCALL PALETTE_ValidateFlags(PALETTEENTRY* lpPalE, INT size)
{
    int i = 0;
    for (; i<size ; i++)
        lpPalE[i].peFlags = PC_SYS_USED | (lpPalE[i].peFlags & 0x07);
}

PPALETTE
NTAPI
PALETTE_AllocPalette2(ULONG Mode,
                     ULONG NumColors,
                     ULONG *Colors,
                     ULONG Red,
                     ULONG Green,
                     ULONG Blue)
{
    PPALETTE PalGDI;

    PalGDI = (PPALETTE)GDIOBJ_AllocateObject(GDIObjType_PAL_TYPE,
                                             sizeof(PALETTE),
                                             BASEFLAG_LOOKASIDE);
    if (!PalGDI)
    {
        DPRINT1("Could not allocate a palette.\n");
        return NULL;
    }

    PalGDI->Self = PalGDI->BaseObject.hHmgr;
    PalGDI->flFlags = Mode;

    if (NULL != Colors)
    {
        PalGDI->IndexedColors = ExAllocatePoolWithTag(PagedPool,
                                                      sizeof(PALETTEENTRY) * NumColors,
                                                      TAG_PALETTE);
        if (NULL == PalGDI->IndexedColors)
        {
            GDIOBJ_vDeleteObject(&PalGDI->BaseObject);
            return NULL;
        }
        RtlCopyMemory(PalGDI->IndexedColors, Colors, sizeof(PALETTEENTRY) * NumColors);
    }

    if (Mode & PAL_INDEXED)
    {
        PalGDI->NumColors = NumColors;
    }
    else if (Mode & PAL_BITFIELDS)
    {
        PalGDI->RedMask = Red;
        PalGDI->GreenMask = Green;
        PalGDI->BlueMask = Blue;

        if (Red == 0x7c00 && Green == 0x3E0 && Blue == 0x1F)
            PalGDI->flFlags |= PAL_RGB16_555;
        else if (Red == 0xF800 && Green == 0x7E0 && Blue == 0x1F)
            PalGDI->flFlags |= PAL_RGB16_565;
        else if (Red == 0xFF0000 && Green == 0xFF00 && Blue == 0xFF)
            PalGDI->flFlags |= PAL_BGR;
        else if (Red == 0xFF && Green == 0xFF00 && Blue == 0xFF0000)
            PalGDI->flFlags |= PAL_RGB;
    }

    return PalGDI;
}

HPALETTE
FASTCALL
PALETTE_AllocPalette(ULONG Mode,
                     ULONG NumColors,
                     ULONG *Colors,
                     ULONG Red,
                     ULONG Green,
                     ULONG Blue)
{
    PPALETTE ppal;
    HPALETTE hpal;

    ppal = PALETTE_AllocPalette2(Mode, NumColors, Colors, Red, Green, Blue);
    if (!ppal) return NULL;

    hpal = GDIOBJ_hInsertObject(&ppal->BaseObject, GDI_OBJ_HMGR_POWNED);
    if (!hpal)
    {
        DPRINT1("Could not insert palette into handle table.\n");
        GDIOBJ_vFreeObject(&ppal->BaseObject);
        return NULL;
    }

    PALETTE_UnlockPalette(ppal);

    return hpal;
}

HPALETTE
FASTCALL
PALETTE_AllocPaletteIndexedRGB(ULONG NumColors,
                               CONST RGBQUAD *Colors)
{
    HPALETTE NewPalette;
    PPALETTE PalGDI;
    UINT i;

    PalGDI = (PPALETTE)GDIOBJ_AllocateObject(GDIObjType_PAL_TYPE,
                                           sizeof(PALETTE),
                                           BASEFLAG_LOOKASIDE);
    if (!PalGDI)
    {
        DPRINT1("Could not allocate a palette.\n");
        return NULL;
    }

    if (!GDIOBJ_hInsertObject(&PalGDI->BaseObject, GDI_OBJ_HMGR_POWNED))
    {
        DPRINT1("Could not insert palette into handle table.\n");
        GDIOBJ_vFreeObject(&PalGDI->BaseObject);
        return NULL;
    }

    NewPalette = PalGDI->BaseObject.hHmgr;

    PalGDI->Self = NewPalette;
    PalGDI->flFlags = PAL_INDEXED;

    PalGDI->IndexedColors = ExAllocatePoolWithTag(PagedPool,
                                                  sizeof(PALETTEENTRY) * NumColors,
                                                  TAG_PALETTE);
    if (NULL == PalGDI->IndexedColors)
    {
        GDIOBJ_vDeleteObject(&PalGDI->BaseObject);
        return NULL;
    }

    for (i = 0; i < NumColors; i++)
    {
        PalGDI->IndexedColors[i].peRed = Colors[i].rgbRed;
        PalGDI->IndexedColors[i].peGreen = Colors[i].rgbGreen;
        PalGDI->IndexedColors[i].peBlue = Colors[i].rgbBlue;
        PalGDI->IndexedColors[i].peFlags = 0;
    }

    PalGDI->NumColors = NumColors;

    PALETTE_UnlockPalette(PalGDI);

    return NewPalette;
}

BOOL NTAPI
PALETTE_Cleanup(PVOID ObjectBody)
{
    PPALETTE pPal = (PPALETTE)ObjectBody;
    if (NULL != pPal->IndexedColors)
    {
        ExFreePoolWithTag(pPal->IndexedColors, TAG_PALETTE);
    }

    return TRUE;
}

INT FASTCALL
PALETTE_GetObject(PPALETTE ppal, INT cbCount, LPLOGBRUSH lpBuffer)
{
    if (!lpBuffer)
    {
        return sizeof(WORD);
    }

    if ((UINT)cbCount < sizeof(WORD)) return 0;
    *((WORD*)lpBuffer) = (WORD)ppal->NumColors;
    return sizeof(WORD);
}

ULONG
NTAPI
PALETTE_ulGetNearestPaletteIndex(PALETTE* ppal, ULONG iColor)
{
    ULONG ulDiff, ulColorDiff, ulMinimalDiff = 0xFFFFFF;
    ULONG i, ulBestIndex = 0;
    PALETTEENTRY peColor = *(PPALETTEENTRY)&iColor;

    /* Loop all palette entries, break on exact match */
    for (i = 0; i < ppal->NumColors && ulMinimalDiff != 0; i++)
    {
        /* Calculate distance in the color cube */
        ulDiff = peColor.peRed - ppal->IndexedColors[i].peRed;
        ulColorDiff = ulDiff * ulDiff;
        ulDiff = peColor.peGreen - ppal->IndexedColors[i].peGreen;
        ulColorDiff += ulDiff * ulDiff;
        ulDiff = peColor.peBlue - ppal->IndexedColors[i].peBlue;
        ulColorDiff += ulDiff * ulDiff;

        /* Check for a better match */
        if (ulColorDiff < ulMinimalDiff)
        {
            ulBestIndex = i;
            ulMinimalDiff = ulColorDiff;
        }
    }

    return ulBestIndex;
}

ULONG
NTAPI
PALETTE_ulGetNearestBitFieldsIndex(PALETTE* ppal, ULONG ulColor)
{
    ULONG ulNewColor;

    // FIXME: HACK, should be stored already
    ppal->ulRedShift = CalculateShift(RGB(0xff,0,0), ppal->RedMask);
    ppal->ulGreenShift = CalculateShift(RGB(0,0xff,0), ppal->GreenMask);
    ppal->ulBlueShift = CalculateShift(RGB(0,0,0xff), ppal->BlueMask);

    ulNewColor = _rotl(ulColor, ppal->ulRedShift) & ppal->RedMask;
    ulNewColor |= _rotl(ulColor, ppal->ulGreenShift) & ppal->GreenMask;
    ulNewColor |= _rotl(ulColor, ppal->ulBlueShift) & ppal->BlueMask;

   return ulNewColor;
}

ULONG
NTAPI
PALETTE_ulGetNearestIndex(PALETTE* ppal, ULONG ulColor)
{
    if (ppal->flFlags & PAL_INDEXED) // use fl & PALINDEXED
        return PALETTE_ulGetNearestPaletteIndex(ppal, ulColor);
    else
        return PALETTE_ulGetNearestBitFieldsIndex(ppal, ulColor);
}

VOID
NTAPI
PALETTE_vGetBitMasks(PPALETTE ppal, PULONG pulColors)
{
    ASSERT(pulColors);

    if (ppal->flFlags & PAL_INDEXED || ppal->flFlags & PAL_RGB)
    {
        pulColors[0] = RGB(0xFF, 0x00, 0x00);
        pulColors[1] = RGB(0x00, 0xFF, 0x00);
        pulColors[2] = RGB(0x00, 0x00, 0xFF);
    }
    else if (ppal->flFlags & PAL_BGR)
    {
        pulColors[0] = RGB(0x00, 0x00, 0xFF);
        pulColors[1] = RGB(0x00, 0xFF, 0x00);
        pulColors[2] = RGB(0xFF, 0x00, 0x00);
    }
    else if (ppal->flFlags & PAL_BITFIELDS)
    {
        pulColors[0] = ppal->RedMask;
        pulColors[1] = ppal->GreenMask;
        pulColors[2] = ppal->BlueMask;
    }
}

VOID
FASTCALL
ColorCorrection(PPALETTE PalGDI, PPALETTEENTRY PaletteEntry, ULONG Colors)
{
    PPDEVOBJ ppdev = (PPDEVOBJ)PalGDI->hPDev;

    if (!ppdev) return;

    if (ppdev->flFlags & PDEV_GAMMARAMP_TABLE)
    {
        INT i;
        PGAMMARAMP GammaRamp = (PGAMMARAMP)ppdev->pvGammaRamp;
        for ( i = 0; i < Colors; i++)
        {
            PaletteEntry[i].peRed   += GammaRamp->Red[i];
            PaletteEntry[i].peGreen += GammaRamp->Green[i];
            PaletteEntry[i].peBlue  += GammaRamp->Blue[i];
        }
    }
    return;
}

/** Display Driver Interface **************************************************/

/*
 * @implemented
 */
HPALETTE
APIENTRY
EngCreatePalette(
    ULONG iMode,
    ULONG cColors,
    ULONG *pulColors,
    ULONG flRed,
    ULONG flGreen,
    ULONG flBlue)
{
    PPALETTE ppal;
    HPALETTE hpal;

    ppal = PALETTE_AllocPalette2(iMode, cColors, pulColors, flRed, flGreen, flBlue);
    if (!ppal) return NULL;

    hpal = GDIOBJ_hInsertObject(&ppal->BaseObject, GDI_OBJ_HMGR_PUBLIC);
    if (!hpal)
    {
        DPRINT1("Could not insert palette into handle table.\n");
        GDIOBJ_vFreeObject(&ppal->BaseObject);
        return NULL;
    }

    PALETTE_UnlockPalette(ppal);
    return hpal;
}

/*
 * @implemented
 */
BOOL
APIENTRY
EngDeletePalette(IN HPALETTE hpal)
{
    PPALETTE ppal;

    ppal = PALETTE_ShareLockPalette(hpal);
    if (!ppal) return FALSE;

    GDIOBJ_vDeleteObject(&ppal->BaseObject);

    return TRUE;
}

/*
 * @implemented
 */
ULONG
APIENTRY
PALOBJ_cGetColors(PALOBJ *PalObj, ULONG Start, ULONG Colors, ULONG *PaletteEntry)
{
    PALETTE *PalGDI;

    PalGDI = (PALETTE*)PalObj;
   /* PalGDI = (PALETTE*)AccessInternalObjectFromUserObject(PalObj); */

    if (Start >= PalGDI->NumColors)
        return 0;

    Colors = min(Colors, PalGDI->NumColors - Start);

    /* NOTE: PaletteEntry ULONGs are in the same order as PALETTEENTRY. */
    RtlCopyMemory(PaletteEntry, PalGDI->IndexedColors + Start, sizeof(ULONG) * Colors);

    if (PalGDI->flFlags & PAL_GAMMACORRECTION)
        ColorCorrection(PalGDI, (PPALETTEENTRY)PaletteEntry, Colors);

    return Colors;
}


/** Systemcall Interface ******************************************************/

/*
 * @implemented
 */
HPALETTE APIENTRY
NtGdiCreatePaletteInternal ( IN LPLOGPALETTE pLogPal, IN UINT cEntries )
{
    PPALETTE PalGDI;
    HPALETTE NewPalette;

    pLogPal->palNumEntries = cEntries;
    NewPalette = PALETTE_AllocPalette( PAL_INDEXED,
                                       cEntries,
                                       (PULONG)pLogPal->palPalEntry,
                                       0, 0, 0);

    if (NewPalette == NULL)
    {
        return NULL;
    }

    PalGDI = (PPALETTE) PALETTE_ShareLockPalette(NewPalette);
    if (PalGDI != NULL)
    {
        PALETTE_ValidateFlags(PalGDI->IndexedColors, PalGDI->NumColors);
        PALETTE_ShareUnlockPalette(PalGDI);
    }
    else
    {
        /* FIXME - Handle PalGDI == NULL!!!! */
        DPRINT1("PalGDI is NULL\n");
    }
  return NewPalette;
}

HPALETTE APIENTRY NtGdiCreateHalftonePalette(HDC  hDC)
{
    int i, r, g, b;
    struct {
        WORD Version;
        WORD NumberOfEntries;
        PALETTEENTRY aEntries[256];
        } Palette;

    Palette.Version = 0x300;
    Palette.NumberOfEntries = 256;
    if (IntGetSystemPaletteEntries(hDC, 0, 256, Palette.aEntries) == 0)
    {
        /* from wine, more that 256 color math */
        Palette.NumberOfEntries = 20;
        for (i = 0; i < Palette.NumberOfEntries; i++)
        {
            Palette.aEntries[i].peRed=0xff;
            Palette.aEntries[i].peGreen=0xff;
            Palette.aEntries[i].peBlue=0xff;
            Palette.aEntries[i].peFlags=0x00;
        }

        Palette.aEntries[0].peRed=0x00;
        Palette.aEntries[0].peBlue=0x00;
        Palette.aEntries[0].peGreen=0x00;

        /* the first 6 */
        for (i=1; i <= 6; i++)
        {
            Palette.aEntries[i].peRed=(i%2)?0x80:0;
            Palette.aEntries[i].peGreen=(i==2)?0x80:(i==3)?0x80:(i==6)?0x80:0;
            Palette.aEntries[i].peBlue=(i>3)?0x80:0;
        }

        for (i=7;  i <= 12; i++)
        {
            switch(i)
            {
                case 7:
                    Palette.aEntries[i].peRed=0xc0;
                    Palette.aEntries[i].peBlue=0xc0;
                    Palette.aEntries[i].peGreen=0xc0;
                    break;
                case 8:
                    Palette.aEntries[i].peRed=0xc0;
                    Palette.aEntries[i].peGreen=0xdc;
                    Palette.aEntries[i].peBlue=0xc0;
                    break;
                case 9:
                    Palette.aEntries[i].peRed=0xa6;
                    Palette.aEntries[i].peGreen=0xca;
                    Palette.aEntries[i].peBlue=0xf0;
                    break;
                case 10:
                    Palette.aEntries[i].peRed=0xff;
                    Palette.aEntries[i].peGreen=0xfb;
                    Palette.aEntries[i].peBlue=0xf0;
                    break;
                case 11:
                    Palette.aEntries[i].peRed=0xa0;
                    Palette.aEntries[i].peGreen=0xa0;
                    Palette.aEntries[i].peBlue=0xa4;
                    break;
            case 12:
                Palette.aEntries[i].peRed=0x80;
                Palette.aEntries[i].peGreen=0x80;
                Palette.aEntries[i].peBlue=0x80;
            }
        }

        for (i=13; i <= 18; i++)
        {
            Palette.aEntries[i].peRed=(i%2)?0xff:0;
            Palette.aEntries[i].peGreen=(i==14)?0xff:(i==15)?0xff:(i==18)?0xff:0;
            Palette.aEntries[i].peBlue=(i>15)?0xff:0x00;
        }
    }
    else
    {
        /* 256 color table */
        for (r = 0; r < 6; r++)
            for (g = 0; g < 6; g++)
                for (b = 0; b < 6; b++)
                {
                    i = r + g*6 + b*36 + 10;
                    Palette.aEntries[i].peRed = r * 51;
                    Palette.aEntries[i].peGreen = g * 51;
                    Palette.aEntries[i].peBlue = b * 51;
                }

        for (i = 216; i < 246; i++)
        {
            int v = (i - 216) << 3;
            Palette.aEntries[i].peRed = v;
            Palette.aEntries[i].peGreen = v;
            Palette.aEntries[i].peBlue = v;
        }
    }

   return NtGdiCreatePaletteInternal((LOGPALETTE *)&Palette, Palette.NumberOfEntries);
}

BOOL
APIENTRY
NtGdiResizePalette(
    HPALETTE hpal,
    UINT Entries)
{
/*  PALOBJ *palPtr = (PALOBJ*)AccessUserObject(hPal);
  UINT cPrevEnt, prevVer;
  INT prevsize, size = sizeof(LOGPALETTE) + (cEntries - 1) * sizeof(PALETTEENTRY);
  XLATEOBJ *XlateObj = NULL;

  if(!palPtr) return FALSE;
  cPrevEnt = palPtr->logpalette->palNumEntries;
  prevVer = palPtr->logpalette->palVersion;
  prevsize = sizeof(LOGPALETTE) + (cPrevEnt - 1) * sizeof(PALETTEENTRY) + sizeof(int*) + sizeof(GDIOBJHDR);
  size += sizeof(int*) + sizeof(GDIOBJHDR);
  XlateObj = palPtr->logicalToSystem;

  if (!(palPtr = GDI_ReallocObject(size, hPal, palPtr))) return FALSE;

  if(XlateObj)
  {
    XLATEOBJ *NewXlateObj = (int*) HeapReAlloc(GetProcessHeap(), 0, XlateObj, cEntries * sizeof(int));
    if(NewXlateObj == NULL)
    {
      ERR("Can not resize logicalToSystem -- out of memory!");
      GDI_ReleaseObj( hPal );
      return FALSE;
    }
    palPtr->logicalToSystem = NewXlateObj;
  }

  if(cEntries > cPrevEnt)
  {
    if(XlateObj) memset(palPtr->logicalToSystem + cPrevEnt, 0, (cEntries - cPrevEnt)*sizeof(int));
    memset( (BYTE*)palPtr + prevsize, 0, size - prevsize );
    PALETTE_ValidateFlags((PALETTEENTRY*)((BYTE*)palPtr + prevsize), cEntries - cPrevEnt );
  }
  palPtr->logpalette->palNumEntries = cEntries;
  palPtr->logpalette->palVersion = prevVer;
//    GDI_ReleaseObj( hPal );
  return TRUE; */

  UNIMPLEMENTED;
  return FALSE;
}

BOOL
APIENTRY
NtGdiGetColorAdjustment(
    HDC hdc,
    LPCOLORADJUSTMENT pca)
{
    UNIMPLEMENTED;
    return FALSE;
}

BOOL
APIENTRY
NtGdiSetColorAdjustment(
    HDC hdc,
    LPCOLORADJUSTMENT pca)
{
    UNIMPLEMENTED;
    return FALSE;
}

COLORREF APIENTRY NtGdiGetNearestColor(HDC hDC, COLORREF Color)
{
   COLORREF nearest = CLR_INVALID;
   PDC dc;
   PPALETTE palGDI;
   LONG RBits, GBits, BBits;

   dc = DC_LockDc(hDC);
   if (NULL != dc)
   {
      HPALETTE hpal = dc->dclevel.hpal;
      palGDI = PALETTE_ShareLockPalette(hpal);
      if (!palGDI)
      {
         DC_UnlockDc(dc);
         return nearest;
      }

      if (palGDI->flFlags & PAL_INDEXED)
      {
         ULONG index;
         index = PALETTE_ulGetNearestPaletteIndex(palGDI, Color);
         nearest = PALETTE_ulGetRGBColorFromIndex(palGDI, index);
      }
      else if (palGDI->flFlags & PAL_RGB || palGDI->flFlags & PAL_BGR)
      {
         nearest = Color;
      }
      else if (palGDI->flFlags & PAL_BITFIELDS)
      {
         RBits = 8 - GetNumberOfBits(palGDI->RedMask);
         GBits = 8 - GetNumberOfBits(palGDI->GreenMask);
         BBits = 8 - GetNumberOfBits(palGDI->BlueMask);
         nearest = RGB(
            (GetRValue(Color) >> RBits) << RBits,
            (GetGValue(Color) >> GBits) << GBits,
            (GetBValue(Color) >> BBits) << BBits);
      }
      PALETTE_ShareUnlockPalette(palGDI);
      DC_UnlockDc(dc);
   }

   return nearest;
}

UINT
APIENTRY
NtGdiGetNearestPaletteIndex(
    HPALETTE hpal,
    COLORREF crColor)
{
    PPALETTE ppal = PALETTE_ShareLockPalette(hpal);
    UINT index  = 0;

    if (ppal)
    {
        if (ppal->flFlags & PAL_INDEXED)
        {
            /* Return closest match for the given RGB color */
            index = PALETTE_ulGetNearestPaletteIndex(ppal, crColor);
        }
        // else SetLastError ?
        PALETTE_ShareUnlockPalette(ppal);
    }

    return index;
}

UINT
FASTCALL
IntGdiRealizePalette(HDC hDC)
{
    UINT i, realize = 0;
    PDC pdc;
    PALETTE *ppalSurf, *ppalDC;

    pdc = DC_LockDc(hDC);
    if(!pdc)
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    ppalSurf = pdc->dclevel.pSurface->ppal;
    ppalDC = pdc->dclevel.ppal;

    if(!(ppalSurf->flFlags & PAL_INDEXED))
    {
        // FIXME : set error?
        goto cleanup;
    }

    ASSERT(ppalDC->flFlags & PAL_INDEXED);

    // FIXME : should we resize ppalSurf if it's too small?
    realize = (ppalDC->NumColors < ppalSurf->NumColors) ? ppalDC->NumColors : ppalSurf->NumColors;

    for(i=0; i<realize; i++)
    {
        InterlockedExchange((LONG*)&ppalSurf->IndexedColors[i], *(LONG*)&ppalDC->IndexedColors[i]);
    }

cleanup:
    DC_UnlockDc(pdc);
    return realize;
}

UINT APIENTRY
IntAnimatePalette(HPALETTE hPal,
                  UINT StartIndex,
                  UINT NumEntries,
                  CONST PPALETTEENTRY PaletteColors)
{
    UINT ret = 0;

    if( hPal != NtGdiGetStockObject(DEFAULT_PALETTE) )
    {
        PPALETTE palPtr;
        UINT pal_entries;
        HDC hDC;
        PDC dc;
        PWND Wnd;
        const PALETTEENTRY *pptr = PaletteColors;

        palPtr = PALETTE_ShareLockPalette(hPal);
        if (!palPtr) return FALSE;

        pal_entries = palPtr->NumColors;
        if (StartIndex >= pal_entries)
        {
            PALETTE_ShareUnlockPalette(palPtr);
            return FALSE;
        }
        if (StartIndex+NumEntries > pal_entries) NumEntries = pal_entries - StartIndex;

        for (NumEntries += StartIndex; StartIndex < NumEntries; StartIndex++, pptr++)
        {
            /* According to MSDN, only animate PC_RESERVED colours */
            if (palPtr->IndexedColors[StartIndex].peFlags & PC_RESERVED)
            {
                memcpy( &palPtr->IndexedColors[StartIndex], pptr,
                        sizeof(PALETTEENTRY) );
                ret++;
                PALETTE_ValidateFlags(&palPtr->IndexedColors[StartIndex], 1);
            }
        }

        PALETTE_ShareUnlockPalette(palPtr);

        /* Immediately apply the new palette if current window uses it */
        Wnd = UserGetDesktopWindow();
        hDC =  UserGetWindowDC(Wnd);
        dc = DC_LockDc(hDC);
        if (NULL != dc)
        {
            if (dc->dclevel.hpal == hPal)
            {
                DC_UnlockDc(dc);
                IntGdiRealizePalette(hDC);
            }
            else
                DC_UnlockDc(dc);
        }
        UserReleaseDC(Wnd,hDC, FALSE);
    }
    return ret;
}

UINT APIENTRY
IntGetPaletteEntries(
    HPALETTE hpal,
    UINT StartIndex,
    UINT  Entries,
    LPPALETTEENTRY  pe)
{
    PPALETTE palGDI;
    UINT numEntries;

    palGDI = (PPALETTE) PALETTE_ShareLockPalette(hpal);
    if (NULL == palGDI)
    {
        return 0;
    }

    numEntries = palGDI->NumColors;
    if (NULL != pe)
    {
        if (numEntries < StartIndex + Entries)
        {
            Entries = numEntries - StartIndex;
        }
        if (numEntries <= StartIndex)
        {
            PALETTE_ShareUnlockPalette(palGDI);
            return 0;
        }
        memcpy(pe, palGDI->IndexedColors + StartIndex, Entries * sizeof(PALETTEENTRY));
    }
    else
    {
        Entries = numEntries;
    }

    PALETTE_ShareUnlockPalette(palGDI);
    return Entries;
}

UINT APIENTRY
IntGetSystemPaletteEntries(HDC  hDC,
                           UINT  StartIndex,
                           UINT  Entries,
                           LPPALETTEENTRY  pe)
{
    PPALETTE palGDI = NULL;
    PDC dc = NULL;
    UINT EntriesSize = 0;
    UINT Ret = 0;

    if (Entries == 0)
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (pe != NULL)
    {
        EntriesSize = Entries * sizeof(pe[0]);
        if (Entries != EntriesSize / sizeof(pe[0]))
        {
            /* Integer overflow! */
            EngSetLastError(ERROR_INVALID_PARAMETER);
            return 0;
        }
    }

    if (!(dc = DC_LockDc(hDC)))
    {
        EngSetLastError(ERROR_INVALID_HANDLE);
        return 0;
    }

    palGDI = PALETTE_ShareLockPalette(dc->dclevel.hpal);
    if (palGDI != NULL)
    {
        if (pe != NULL)
        {
            if (StartIndex >= palGDI->NumColors)
                Entries = 0;
            else if (Entries > palGDI->NumColors - StartIndex)
                Entries = palGDI->NumColors - StartIndex;

            memcpy(pe,
                   palGDI->IndexedColors + StartIndex,
                   Entries * sizeof(pe[0]));

            Ret = Entries;
        }
        else
        {
            Ret = dc->ppdev->gdiinfo.ulNumPalReg;
        }
    }

    if (palGDI != NULL)
        PALETTE_ShareUnlockPalette(palGDI);

    if (dc != NULL)
        DC_UnlockDc(dc);

    return Ret;
}

UINT
APIENTRY
IntSetPaletteEntries(
    HPALETTE  hpal,
    UINT  Start,
    UINT  Entries,
    CONST LPPALETTEENTRY pe)
{
    PPALETTE palGDI;
    WORD numEntries;

    if ((UINT)hpal & GDI_HANDLE_STOCK_MASK)
    {
    	return 0;
    }

    palGDI = PALETTE_ShareLockPalette(hpal);
    if (!palGDI) return 0;

    numEntries = palGDI->NumColors;
    if (Start >= numEntries)
    {
        PALETTE_ShareUnlockPalette(palGDI);
        return 0;
    }
    if (numEntries < Start + Entries)
    {
        Entries = numEntries - Start;
    }
    memcpy(palGDI->IndexedColors + Start, pe, Entries * sizeof(PALETTEENTRY));
    PALETTE_ShareUnlockPalette(palGDI);

    return Entries;
}

W32KAPI
LONG
APIENTRY
NtGdiDoPalette(
    IN HGDIOBJ hObj,
    IN WORD iStart,
    IN WORD cEntries,
    IN LPVOID pUnsafeEntries,
    IN DWORD iFunc,
    IN BOOL bInbound)
{
	LONG ret;
	LPVOID pEntries = NULL;

	/* FIXME: Handle bInbound correctly */

	if (bInbound &&
	    (pUnsafeEntries == NULL || cEntries == 0))
	{
		return 0;
	}

	if (pUnsafeEntries)
	{
		pEntries = ExAllocatePoolWithTag(PagedPool, cEntries * sizeof(PALETTEENTRY), TAG_PALETTE);
		if (!pEntries)
			return 0;
		if (bInbound)
		{
			_SEH2_TRY
			{
				ProbeForRead(pUnsafeEntries, cEntries * sizeof(PALETTEENTRY), 1);
				memcpy(pEntries, pUnsafeEntries, cEntries * sizeof(PALETTEENTRY));
			}
			_SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
			{
				ExFreePoolWithTag(pEntries, TAG_PALETTE);
				_SEH2_YIELD(return 0);
			}
			_SEH2_END
		}
	}

	ret = 0;
	switch(iFunc)
	{
		case GdiPalAnimate:
			if (pEntries)
				ret = IntAnimatePalette((HPALETTE)hObj, iStart, cEntries, (CONST PPALETTEENTRY)pEntries);
			break;

		case GdiPalSetEntries:
			if (pEntries)
				ret = IntSetPaletteEntries((HPALETTE)hObj, iStart, cEntries, (CONST LPPALETTEENTRY)pEntries);
			break;

		case GdiPalGetEntries:
			ret = IntGetPaletteEntries((HPALETTE)hObj, iStart, cEntries, (LPPALETTEENTRY)pEntries);
			break;

		case GdiPalGetSystemEntries:
			ret = IntGetSystemPaletteEntries((HDC)hObj, iStart, cEntries, (LPPALETTEENTRY)pEntries);
			break;

		case GdiPalSetColorTable:
			if (pEntries)
				ret = IntSetDIBColorTable((HDC)hObj, iStart, cEntries, (RGBQUAD*)pEntries);
			break;

		case GdiPalGetColorTable:
			if (pEntries)
				ret = IntGetDIBColorTable((HDC)hObj, iStart, cEntries, (RGBQUAD*)pEntries);
			break;
	}

	if (pEntries)
	{
		if (!bInbound)
		{
			_SEH2_TRY
			{
				ProbeForWrite(pUnsafeEntries, cEntries * sizeof(PALETTEENTRY), 1);
				memcpy(pUnsafeEntries, pEntries, cEntries * sizeof(PALETTEENTRY));
			}
			_SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
			{
				ret = 0;
			}
			_SEH2_END
		}
		ExFreePoolWithTag(pEntries, TAG_PALETTE);
	}

	return ret;
}

UINT APIENTRY
NtGdiSetSystemPaletteUse(HDC hDC, UINT Usage)
{
    UINT old = SystemPaletteUse;

    /* Device doesn't support colour palettes */
    if (!(NtGdiGetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE)) {
        return SYSPAL_ERROR;
    }

    switch (Usage)
	{
		case SYSPAL_NOSTATIC:
        case SYSPAL_NOSTATIC256:
        case SYSPAL_STATIC:
				SystemPaletteUse = Usage;
				break;

        default:
				old=SYSPAL_ERROR;
				break;
	}

 return old;
}

UINT
APIENTRY
NtGdiGetSystemPaletteUse(HDC hDC)
{
    return SystemPaletteUse;
}

BOOL
APIENTRY
NtGdiUpdateColors(HDC hDC)
{
   PWND Wnd;
   BOOL calledFromUser, ret;
   USER_REFERENCE_ENTRY Ref;

   calledFromUser = UserIsEntered();

   if (!calledFromUser){
      UserEnterExclusive();
   }

   Wnd = UserGetWindowObject(IntWindowFromDC(hDC));
   if (Wnd == NULL)
   {
      EngSetLastError(ERROR_INVALID_WINDOW_HANDLE);

      if (!calledFromUser){
         UserLeave();
      }

      return FALSE;
   }

   UserRefObjectCo(Wnd, &Ref);
   ret = co_UserRedrawWindow(Wnd, NULL, 0, RDW_INVALIDATE);
   UserDerefObjectCo(Wnd);

   if (!calledFromUser){
      UserLeave();
   }

   return ret;
}

BOOL
APIENTRY
NtGdiUnrealizeObject(HGDIOBJ hgdiobj)
{
   BOOL Ret = FALSE;
   PPALETTE palGDI;

   if ( !hgdiobj ||
        ((UINT)hgdiobj & GDI_HANDLE_STOCK_MASK) ||
        !GDI_HANDLE_IS_TYPE(hgdiobj, GDI_OBJECT_TYPE_PALETTE) )
      return Ret;

   palGDI = PALETTE_ShareLockPalette(hgdiobj);
   if (!palGDI) return FALSE;

   // FIXME!!
   // Need to do something!!!
   // Zero out Current and Old Translated pointers?
   //
   Ret = TRUE;
   PALETTE_ShareUnlockPalette(palGDI);
   return Ret;
}


/* EOF */
