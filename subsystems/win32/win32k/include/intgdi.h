#pragma once

/* Brush functions */

extern HDC hSystemBM;
extern HSEMAPHORE hsemDriverMgmt;

HBRUSH APIENTRY
IntGdiCreatePatternBrush(
   HBITMAP hBitmap);

HBRUSH APIENTRY
IntGdiCreateSolidBrush(
   COLORREF Color);

HBRUSH APIENTRY
IntGdiCreateNullBrush(VOID);

VOID FASTCALL
IntGdiSetSolidBrushColor(HBRUSH hBrush, COLORREF Color);

/* Line functions */

BOOL FASTCALL
IntGdiLineTo(DC  *dc,
             int XEnd,
             int YEnd);

BOOL FASTCALL
IntGdiMoveToEx(DC      *dc,
               int     X,
               int     Y,
               LPPOINT Point,
               BOOL    BypassPath);

BOOL FASTCALL
IntGdiPolyBezier(DC      *dc,
                 LPPOINT pt,
                 DWORD   Count);

BOOL FASTCALL
IntGdiPolyline(DC      *dc,
               LPPOINT pt,
               int     Count);

BOOL FASTCALL
IntGdiPolyBezierTo(DC      *dc,
                   LPPOINT pt,
                   DWORD   Count);

BOOL FASTCALL
IntGdiPolyPolyline(DC      *dc,
                   LPPOINT pt,
                   PULONG PolyPoints,
                   DWORD   Count);

BOOL FASTCALL
IntGdiPolylineTo(DC      *dc,
                 LPPOINT pt,
                 DWORD   Count);

/* Shape functions */

BOOL
NTAPI
GreGradientFill(
    HDC hdc,
    PTRIVERTEX pVertex,
    ULONG nVertex,
    PVOID pMesh,
    ULONG nMesh,
    ULONG ulMode);

/* DC functions */

HDC FASTCALL
IntGdiCreateDC(PUNICODE_STRING Driver,
               PUNICODE_STRING Device,
               PVOID pUMdhpdev,
               CONST PDEVMODEW InitData,
               BOOL CreateAsIC);

/* Coord functions */

BOOL FASTCALL
IntGdiCombineTransform(LPXFORM XFormResult,
                       LPXFORM xform1,
                       LPXFORM xform2);

/* Stock objects */

VOID FASTCALL
IntSetSysColors(UINT nColors, CONST INT *Elements, CONST COLORREF *Colors);

HGDIOBJ FASTCALL
IntGetSysColorBrush(INT Object);

DWORD FASTCALL
IntGetSysColor(INT nIndex);

/* Other Stuff */

HBITMAP
FASTCALL
IntCreateCompatibleBitmap(PDC Dc,
                          INT Width,
                          INT Height);

WORD APIENTRY IntGdiSetHookFlags(HDC hDC, WORD Flags);

UINT APIENTRY IntSetDIBColorTable(HDC hDC, UINT StartIndex, UINT Entries, CONST RGBQUAD *Colors);

UINT APIENTRY IntGetDIBColorTable(HDC hDC, UINT StartIndex, UINT Entries, RGBQUAD *Colors);

UINT APIENTRY
IntGetPaletteEntries(HPALETTE  hpal,
                     UINT  StartIndex,
                     UINT  Entries,
                     LPPALETTEENTRY  pe);

UINT APIENTRY
IntGetSystemPaletteEntries(HDC  hDC,
                           UINT  StartIndex,
                           UINT  Entries,
                           LPPALETTEENTRY  pe);

VOID  FASTCALL CreateStockObjects (VOID);
VOID  FASTCALL CreateSysColorObjects (VOID);

PPOINT FASTCALL GDI_Bezier (const POINT *Points, INT count, PINT nPtsOut);
