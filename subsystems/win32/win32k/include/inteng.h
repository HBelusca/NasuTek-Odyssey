#pragma once

typedef ULONG HCLIP;

#define ENUM_RECT_LIMIT   50

typedef struct _RECT_ENUM
{
  ULONG c;
  RECTL arcl[ENUM_RECT_LIMIT];
} RECT_ENUM;

typedef struct tagSPAN
{
  LONG Y;
  LONG X;
  ULONG Width;
} SPAN, *PSPAN;

#define R3_OPINDEX_NOOP         0xAA

#define R3_OPINDEX_BLACKNESS    0x00
#define R3_OPINDEX_NOTSRCERASE  0x11
#define R3_OPINDEX_NOTSRCCOPY   0x33
#define R3_OPINDEX_SRCERASE     0x44
#define R3_OPINDEX_DSTINVERT    0x55
#define R3_OPINDEX_PATINVERT    0x5A
#define R3_OPINDEX_SRCINVERT    0x66
#define R3_OPINDEX_SRCAND       0x88
#define R3_OPINDEX_MERGEPAINT   0xBB
#define R3_OPINDEX_MERGECOPY    0xC0
#define R3_OPINDEX_SRCCOPY      0xCC
#define R3_OPINDEX_SRCPAINT     0xEE
#define R3_OPINDEX_PATCOPY      0xF0
#define R3_OPINDEX_PATPAINT     0xFB
#define R3_OPINDEX_WHITENESS    0xFF

#define ROP2_TO_MIX(Rop2) (((Rop2) << 8) | (Rop2))

#define ROP4_FROM_INDEX(index) ((index) | ((index) << 8))

#define ROP4_USES_SOURCE(Rop4)  (((((Rop4) & 0xCC00) >> 2) != ((Rop4) & 0x3300)) || ((((Rop4) & 0xCC) >> 2) != ((Rop4) & 0x33)))
#define ROP4_USES_MASK(Rop4)    (((Rop4) & 0xFF00) != (((Rop4) & 0xff) << 8))
#define ROP4_USES_DEST(Rop4)    (((((Rop4) & 0xAA) >> 1) != ((Rop4) & 0x55)) || ((((Rop4) & 0xAA00) >> 1) != ((Rop4) & 0x5500)))
#define ROP4_USES_PATTERN(Rop4) (((((Rop4) & 0xF0) >> 4) != ((Rop4) & 0x0F)) || ((((Rop4) & 0xF000) >> 4) != ((Rop4) & 0x0F00)))

#define IS_VALID_ROP4(rop) (((rop) & 0xFFFF0000) == 0)

#define ROP4_FGND(Rop4)    ((Rop4) & 0x00FF)
#define ROP4_BKGND(Rop4)    (((Rop4) & 0xFF00) >> 8)

#define ROP4_NOOP (R3_OPINDEX_NOOP | (R3_OPINDEX_NOOP << 8))
#define ROP4_MASK (R3_OPINDEX_SRCCOPY | (R3_OPINDEX_NOOP << 8))

/* Definitions of IntEngXxx functions */

BOOL APIENTRY
IntEngLineTo(SURFOBJ *Surface,
             CLIPOBJ *Clip,
             BRUSHOBJ *Brush,
             LONG x1,
             LONG y1,
             LONG x2,
             LONG y2,
             RECTL *RectBounds,
             MIX mix);

BOOL APIENTRY
IntEngBitBlt(SURFOBJ *DestObj,
               SURFOBJ *SourceObj,
               SURFOBJ *Mask,
               CLIPOBJ *ClipRegion,
               XLATEOBJ *ColorTranslation,
               RECTL *DestRect,
               POINTL *SourcePoint,
               POINTL *MaskOrigin,
               BRUSHOBJ *Brush,
               POINTL *BrushOrigin,
               ROP4 Rop4);

BOOL APIENTRY
IntEngStretchBlt(SURFOBJ *DestObj,
                 SURFOBJ *SourceObj,
                 SURFOBJ *Mask,
                 CLIPOBJ *ClipRegion,
                 XLATEOBJ *ColorTranslation,
                 RECTL *DestRect,
                 RECTL *SourceRect,
                 POINTL *pMaskOrigin,
                 BRUSHOBJ *Brush,
                 POINTL *BrushOrigin,
                 ULONG Mode);

BOOL APIENTRY
IntEngGradientFill(SURFOBJ *psoDest,
                   CLIPOBJ *pco,
                   XLATEOBJ *pxlo,
                   TRIVERTEX *pVertex,
                   ULONG nVertex,
                   PVOID pMesh,
                   ULONG nMesh,
                   RECTL *prclExtents,
                   POINTL *pptlDitherOrg,
                   ULONG ulMode);

BOOL APIENTRY
IntEngPolyline(SURFOBJ *DestSurf,
               CLIPOBJ *Clip,
               BRUSHOBJ *Brush,
               CONST LPPOINT  pt,
               LONG dCount,
               MIX mix);

CLIPOBJ* FASTCALL
IntEngCreateClipRegion(ULONG count,
                       PRECTL pRect,
                       PRECTL rcBounds);

VOID FASTCALL
IntEngDeleteClipRegion(CLIPOBJ *ClipObj);


BOOL FASTCALL
IntEngTransparentBlt(SURFOBJ *Dest,
                     SURFOBJ *Source,
                     CLIPOBJ *Clip,
                     XLATEOBJ *ColorTranslation,
                     PRECTL DestRect,
                     PRECTL SourceRect,
                     ULONG iTransColor,
                     ULONG Reserved);

BOOL APIENTRY
IntEngPaint(IN SURFOBJ *Surface,
            IN CLIPOBJ *ClipRegion,
            IN BRUSHOBJ *Brush,
            IN POINTL *BrushOrigin,
            IN MIX Mix);

ULONG APIENTRY
IntEngSetPointerShape(
   IN SURFOBJ *pso,
   IN SURFOBJ *psoMask,
   IN SURFOBJ *psoColor,
   IN XLATEOBJ *pxlo,
   IN LONG xHot,
   IN LONG yHot,
   IN LONG x,
   IN LONG y,
   IN RECTL *prcl,
   IN FLONG fl);

BOOL APIENTRY
IntEngAlphaBlend(IN SURFOBJ *Dest,
                 IN SURFOBJ *Source,
                 IN CLIPOBJ *ClipRegion,
                 IN XLATEOBJ *ColorTranslation,
                 IN PRECTL DestRect,
                 IN PRECTL SourceRect,
                 IN BLENDOBJ *BlendObj);

BOOL APIENTRY
IntEngCopyBits(SURFOBJ *psoDest,
	    SURFOBJ *psoSource,
	    CLIPOBJ *pco,
	    XLATEOBJ *pxlo,
	    RECTL *prclDest,
	    POINTL *ptlSource);
