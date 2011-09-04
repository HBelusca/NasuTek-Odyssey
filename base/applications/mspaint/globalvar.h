/*
 * PROJECT:     PAINT for Odyssey
 * LICENSE:     LGPL
 * FILE:        base/applications/paint/globalvar.h
 * PURPOSE:     Declaring global variables for later initialization
 * PROGRAMMERS: Benedikt Freisen
 */
 
/* INCLUDES *********************************************************/

#include <windows.h>
#include "definitions.h"

/* VARIABLES declared in main.c *************************************/

extern HDC hDrawingDC;
extern HDC hSelDC;
extern int *bmAddress;
extern BITMAPINFO bitmapinfo;
extern int imgXRes;
extern int imgYRes;

extern HBITMAP hBms[HISTORYSIZE];
extern int currInd;
extern int undoSteps;
extern int redoSteps;
extern BOOL imageSaved;

extern LONG startX;
extern LONG startY;
extern LONG lastX;
extern LONG lastY;
extern int lineWidth;
extern int shapeStyle;
extern int brushStyle;
extern int activeTool;
extern int airBrushWidth;
extern int rubberRadius;
extern int transpBg;
extern int zoom;
extern int rectSel_src[4];
extern int rectSel_dest[4];
extern HWND hSelection;
extern HWND hImageArea;
extern HBITMAP hSelBm;
extern HBITMAP hSelMask;

extern int palColors[28];

extern int fgColor;
extern int bgColor;

extern HWND hStatusBar;
extern HWND hScrollbox;
extern HWND hMainWnd;
extern HWND hPalWin;
extern HWND hToolBoxContainer;
extern HWND hToolSettings;
extern HWND hTrackbarZoom;
extern CHOOSECOLOR choosecolor;
extern OPENFILENAME ofn;
extern OPENFILENAME sfn;
extern HICON hNontranspIcon;
extern HICON hTranspIcon;

extern HCURSOR hCurFill;
extern HCURSOR hCurColor;
extern HCURSOR hCurZoom;
extern HCURSOR hCurPen;
extern HCURSOR hCurAirbrush;

extern HWND hScrlClient;

extern HWND hToolBtn[16];

extern HINSTANCE hProgInstance;
    
extern TCHAR filename[256];
extern TCHAR filepathname[1000];
extern BOOL isAFile;
extern int fileSize;
extern int fileHPPM;
extern int fileVPPM;
extern SYSTEMTIME fileTime;

extern BOOL showGrid;
extern BOOL showMiniature;

extern HWND hwndMiniature;

extern HWND hSizeboxLeftTop;
extern HWND hSizeboxCenterTop;
extern HWND hSizeboxRightTop;
extern HWND hSizeboxLeftCenter;
extern HWND hSizeboxRightCenter;
extern HWND hSizeboxLeftBottom;
extern HWND hSizeboxCenterBottom;
extern HWND hSizeboxRightBottom;

/* VARIABLES declared in mouse.c ************************************/

extern POINT pointStack[256];
extern short pointSP;
extern POINT *ptStack;
extern int ptSP;
