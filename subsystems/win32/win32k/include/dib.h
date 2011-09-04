#pragma once

INT FASTCALL DIB_BitmapInfoSize (const BITMAPINFO * info, WORD coloruse);
HBITMAP APIENTRY DIB_CreateDIBSection (PDC dc, CONST BITMAPINFO *bmi, UINT usage, LPVOID *bits, HANDLE section, DWORD offset, DWORD ovr_pitch);
int FASTCALL DIB_GetBitmapInfo( const BITMAPINFOHEADER *header, LONG *width,
                       LONG *height, WORD *planes, WORD *bpp, DWORD *compr, DWORD *size );
INT APIENTRY DIB_GetDIBImageBytes (INT  width, INT height, INT depth);
HPALETTE FASTCALL DIB_MapPaletteColors(PPALETTE ppal, CONST BITMAPINFO* lpbmi);
HPALETTE FASTCALL BuildDIBPalette (CONST BITMAPINFO *bmi);
BITMAPINFO* FASTCALL DIB_ConvertBitmapInfo(CONST BITMAPINFO* bmi, DWORD Usage);
VOID FASTCALL DIB_FreeConvertedBitmapInfo(BITMAPINFO* converted, BITMAPINFO* orig);
