#pragma once

typedef struct _CVR // Tag Ussw
{
  WINDOWPOS   pos;
  LONG        xClientNew;
  LONG        yClientNew;
  LONG        cxClientNew;
  LONG        cyClientNew;
  RECT        rcBlt;
  LONG        dxBlt;
  LONG        dyBlt;
  UINT        fsRE;
  HRGN        hrgnVisOld;
  PTHREADINFO pti;
  HRGN        hrgnClip;
  HRGN        hrgnInterMonitor;
} CVR, *PCVR;

typedef struct _SMWP
{
  HEAD head;
  UINT bShellNotify:1;
  UINT bHandle:1;
  INT  ccvr;
  INT  ccvrAlloc;
  PCVR acvr;
} SMWP, *PSMWP;

FORCEINLINE BOOL IntPtInWindow(PWND pwnd, INT x, INT y)
{
    if(!RECTL_bPointInRect(&pwnd->rcWindow, x, y))
    {
        return FALSE;
    }

    if(!pwnd->hrgnClip || pwnd->style & WS_MINIMIZE)
    {
        return TRUE;
    }

    return NtGdiPtInRegion(pwnd->hrgnClip, 
                           x - pwnd->rcWindow.left,
                           y - pwnd->rcWindow.top);
}

UINT FASTCALL co_WinPosArrangeIconicWindows(PWND parent);
BOOL FASTCALL IntGetClientOrigin(PWND Window, LPPOINT Point);
LRESULT FASTCALL co_WinPosGetNonClientSize(PWND Window, RECTL* WindowRect, RECTL* ClientRect);
UINT FASTCALL co_WinPosGetMinMaxInfo(PWND Window, POINT* MaxSize, POINT* MaxPos, POINT* MinTrack, POINT* MaxTrack);
UINT FASTCALL co_WinPosMinMaximize(PWND WindowObject, UINT ShowFlag, RECTL* NewPos);
BOOLEAN FASTCALL co_WinPosSetWindowPos(PWND Wnd, HWND WndInsertAfter, INT x, INT y, INT cx, INT cy, UINT flags);
BOOLEAN FASTCALL co_WinPosShowWindow(PWND Window, INT Cmd);
void FASTCALL co_WinPosSendSizeMove(PWND Window);
PWND FASTCALL co_WinPosWindowFromPoint(PWND ScopeWin, POINT *WinPoint, USHORT* HitTest);
VOID FASTCALL co_WinPosActivateOtherWindow(PWND Window);
VOID FASTCALL WinPosInitInternalPos(PWND WindowObject, POINT *pt, RECTL *RestoreRect);
