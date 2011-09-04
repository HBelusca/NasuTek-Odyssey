#pragma once

/* Internal interface */

#define NB_HATCH_STYLES  6

/*
 * The layout of this structure is taken from "Windows Graphics Programming"
 * book written by Feng Yuan.
 *
 * DON'T MODIFY THIS STRUCTURE UNLESS REALLY NEEDED AND EVEN THEN ASK ON
 * A MAILING LIST FIRST.
 */
typedef struct _BRUSH
{
  /* Header for all gdi objects in the handle table.
     Do not (re)move this. */
   BASEOBJECT    BaseObject;

   ULONG ulStyle;
   HBITMAP hbmPattern;
   HANDLE hbmClient;
   ULONG flAttrs;

   ULONG ulBrushUnique;
   BRUSH_ATTR *pBrushAttr; // Just like DC_ATTR, pointer to user data
   BRUSH_ATTR BrushAttr;   // "    "    DCOBJ, internal if pBrushAttr == Zero
   POINT ptOrigin;
   ULONG bCacheGrabbed;
   COLORREF crBack;
   COLORREF crFore;
   ULONG ulPalTime;
   ULONG ulSurfTime;
   PVOID ulRealization;
   ULONG Unknown4C[3];
   POINT ptPenWidth;
   ULONG ulPenStyle;
   DWORD *pStyle;
   ULONG dwStyleCount;
   ULONG Unknown6C;
} BRUSH, *PBRUSH;

typedef struct _EBRUSHOBJ
{
    BRUSHOBJ    BrushObject;

    COLORREF    crRealize;
    ULONG       ulRGBColor;
    PVOID       pengbrush;
    ULONG       ulSurfPalTime;
    ULONG       ulDCPalTime;
    COLORREF    crCurrentText;
    COLORREF    crCurrentBack;
    COLORADJUSTMENT *pca;
//    DWORD       dwUnknown2c;
//    DWORD       dwUnknown30;
    SURFACE *   psurfTrg;
    struct _PALETTE *   ppalSurf;
//    PALETTE *   ppalDC;
//    PALETTE *   ppal3;
//    DWORD       dwUnknown44;
    BRUSH *     pbrush;
    FLONG       flattrs;
    DWORD       ulUnique;
//    DWORD       dwUnknown54;
//    DWORD       dwUnknown58;
} EBRUSHOBJ, *PEBRUSHOBJ;

/* GDI Brush Attributes */
#define GDIBRUSH_NEED_FG_CLR            0x0001
#define GDIBRUSH_NEED_BK_CLR		0x0002 /* Background color is needed */
#define GDIBRUSH_DITHER_OK		0x0004 /* Allow color dithering */
#define GDIBRUSH_IS_SOLID		0x0010 /* Solid brush */
#define GDIBRUSH_IS_HATCH		0x0020 /* Hatch brush */
#define GDIBRUSH_IS_BITMAP		0x0040 /* DDB pattern brush */
#define GDIBRUSH_IS_DIB			0x0080 /* DIB pattern brush */
#define GDIBRUSH_IS_NULL		0x0100 /* Null/hollow brush */
#define GDIBRUSH_IS_GLOBAL		0x0200 /* Stock objects */
#define GDIBRUSH_IS_PEN			0x0400 /* Pen */
#define GDIBRUSH_IS_OLDSTYLEPEN		0x0800 /* Geometric pen */
#define GDIBRUSH_IS_DIBPALCOLORS        0x1000
#define GDIBRUSH_IS_DIBPALINDICE        0x2000
#define GDIBRUSH_IS_DEFAULTSTYLE        0x4000
#define GDIBRUSH_IS_MASKING		0x8000 /* Pattern bitmap is used as transparent mask (?) */
#define GDIBRUSH_IS_INSIDEFRAME         0x00010000
#define GDIBRUSH_CACHED_ENGINE          0x00040000
#define GDIBRUSH_CACHED_IS_SOLID	0x80000000

#define  BRUSH_AllocBrush() ((PBRUSH) GDIOBJ_AllocObj(GDIObjType_BRUSH_TYPE))
#define  BRUSH_AllocBrushWithHandle() ((PBRUSH) GDIOBJ_AllocObjWithHandle(GDI_OBJECT_TYPE_BRUSH, sizeof(BRUSH)))
#define  BRUSH_FreeBrush(pBrush) GDIOBJ_FreeObj((POBJ)pBrush, GDIObjType_BRUSH_TYPE)
#define  BRUSH_FreeBrushByHandle(hBrush) GDIOBJ_FreeObjByHandle((HGDIOBJ)hBrush, GDI_OBJECT_TYPE_BRUSH)
#define  BRUSH_ShareLockBrush(hBrush) ((PBRUSH)GDIOBJ_ShareLockObj((HGDIOBJ)hBrush, GDI_OBJECT_TYPE_BRUSH))
#define  BRUSH_ShareUnlockBrush(pBrush) GDIOBJ_vDereferenceObject((POBJ)pBrush)

INT FASTCALL BRUSH_GetObject (PBRUSH GdiObject, INT Count, LPLOGBRUSH Buffer);
BOOL NTAPI BRUSH_Cleanup(PVOID ObjectBody);

struct _DC;

VOID
NTAPI
EBRUSHOBJ_vInit(EBRUSHOBJ *pebo, PBRUSH pbrush, struct _DC *);

VOID
FASTCALL
EBRUSHOBJ_vSetSolidBrushColor(EBRUSHOBJ *pebo, COLORREF crColor);

VOID
NTAPI
EBRUSHOBJ_vUpdate(EBRUSHOBJ *pebo, PBRUSH pbrush, struct _DC *pdc);

BOOL
NTAPI
EBRUSHOBJ_bRealizeBrush(EBRUSHOBJ *pebo, BOOL bCallDriver);

VOID
NTAPI
EBRUSHOBJ_vCleanup(EBRUSHOBJ *pebo);

PVOID
NTAPI
EBRUSHOBJ_pvGetEngBrush(EBRUSHOBJ *pebo);

BOOL FASTCALL IntGdiSetBrushOwner(PBRUSH,DWORD);
BOOL FASTCALL GreSetBrushOwner(HBRUSH,DWORD);

