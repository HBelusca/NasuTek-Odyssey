/*
 * PROJECT:     PAINT for Odyssey
 * LICENSE:     LGPL
 * FILE:        base/applications/paint/dib.h
 * PURPOSE:     Some DIB related functions
 * PROGRAMMERS: Benedikt Freisen
 */

HBITMAP CreateDIBWithProperties(int width, int height);

int GetDIBWidth(HBITMAP hbm);

int GetDIBHeight(HBITMAP hbm);

void SaveDIBToFile(HBITMAP hBitmap, LPTSTR FileName, HDC hDC, LPSYSTEMTIME time, int *size, int hRes,
                   int vRes);

void LoadDIBFromFile(HBITMAP *hBitmap, LPTSTR name, LPSYSTEMTIME time, int *size, int *hRes, int *vRes);
