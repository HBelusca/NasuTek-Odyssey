/*
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey user32.dll
 * FILE:            dll/win32/user32/windows/defwnd.c
 * PURPOSE:         Window management
 * PROGRAMMER:      Casper S. Hornstrup (chorns@users.sourceforge.net)
 * UPDATE HISTORY:
 *      06-06-2001  CSH  Created
 */

/* INCLUDES ******************************************************************/

#include <user32.h>

#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(user32);

LRESULT DefWndNCPaint(HWND hWnd, HRGN hRgn, BOOL Active);
LRESULT DefWndNCCalcSize(HWND hWnd, BOOL CalcSizeStruct, RECT *Rect);
LRESULT DefWndNCActivate(HWND hWnd, WPARAM wParam);
LRESULT DefWndNCHitTest(HWND hWnd, POINT Point);
LRESULT DefWndNCLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT DefWndNCLButtonDblClk(HWND hWnd, WPARAM wParam, LPARAM lParam);
void FASTCALL MenuInitSysMenuPopup(HMENU Menu, DWORD Style, DWORD ClsStyle, LONG HitTest );
void MENU_EndMenu( HWND );

/* GLOBALS *******************************************************************/

static short iF10Key = 0;
static short iMenuSysKey = 0;

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
DWORD WINAPI
GetSysColor(int nIndex)
{
  if(nIndex >= 0 && nIndex < NUM_SYSCOLORS)
  {
    return gpsi->argbSystem[nIndex];
  }

  SetLastError(ERROR_INVALID_PARAMETER);
  return 0;
}

/*
 * @implemented
 */
HBRUSH WINAPI
GetSysColorBrush(int nIndex)
{
  if(nIndex >= 0 && nIndex < NUM_SYSCOLORS)
  {
    return gpsi->ahbrSystem[nIndex];
  }

  SetLastError(ERROR_INVALID_PARAMETER);
  return NULL;
}

/*
 * @implemented
 */
BOOL
WINAPI
SetSysColors(
  int cElements,
  CONST INT *lpaElements,
  CONST COLORREF *lpaRgbValues)
{
  return NtUserSetSysColors(cElements, lpaElements, lpaRgbValues, 0);
}

BOOL
FASTCALL
DefSetText(HWND hWnd, PCWSTR String, BOOL Ansi)
{
  BOOL Ret;
  LARGE_STRING lsString;

  if ( String )
  {
     if ( Ansi )
        RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&lsString, (PCSZ)String, 0);
     else
        RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&lsString, String, 0);
  }
  Ret = NtUserDefSetText(hWnd, (String ? &lsString : NULL));

  if (Ret)
     IntNotifyWinEvent(EVENT_OBJECT_NAMECHANGE, hWnd, OBJID_WINDOW, CHILDID_SELF, 0);

  return Ret;
}

void
UserGetInsideRectNC(PWND Wnd, RECT *rect)
{
    ULONG Style;
    ULONG ExStyle;

    Style = Wnd->style;
    ExStyle = Wnd->ExStyle;

    rect->top    = rect->left = 0;
    rect->right  = Wnd->rcWindow.right - Wnd->rcWindow.left;
    rect->bottom = Wnd->rcWindow.bottom - Wnd->rcWindow.top;

    if (Style & WS_ICONIC)
    {
        return;
    }

    /* Remove frame from rectangle */
    if (UserHasThickFrameStyle(Style, ExStyle ))
    {
        InflateRect(rect, -GetSystemMetrics(SM_CXFRAME),
                    -GetSystemMetrics(SM_CYFRAME));
    }
    else
    {
        if (UserHasDlgFrameStyle(Style, ExStyle ))
        {
            InflateRect(rect, -GetSystemMetrics(SM_CXDLGFRAME),
                        -GetSystemMetrics(SM_CYDLGFRAME));
            /* FIXME: this isn't in NC_AdjustRect? why not? */
            if (ExStyle & WS_EX_DLGMODALFRAME)
	            InflateRect( rect, -1, 0 );
        }
        else
        {
            if (UserHasThinFrameStyle(Style, ExStyle))
            {
                InflateRect(rect, -GetSystemMetrics(SM_CXBORDER),
                            -GetSystemMetrics(SM_CYBORDER));
            }
        }
    }
}


VOID
DefWndSetRedraw(HWND hWnd, WPARAM wParam)
{
    LONG Style = GetWindowLongPtr(hWnd, GWL_STYLE);
    /* Content can be redrawn after a change. */
    if (wParam)
    {
       if (!(Style & WS_VISIBLE)) /* Not Visible */
       {
          SetWindowLongPtr(hWnd, GWL_STYLE, WS_VISIBLE);
       }
    }
    else /* Content cannot be redrawn after a change. */
    {
       if (Style & WS_VISIBLE) /* Visible */
       {
            RedrawWindow( hWnd, NULL, 0, RDW_ALLCHILDREN | RDW_VALIDATE );
            Style &= ~WS_VISIBLE;
            SetWindowLongPtr(hWnd, GWL_STYLE, Style); /* clear bits */
       }
    }
    return;
}


LRESULT
DefWndHandleSetCursor(HWND hWnd, WPARAM wParam, LPARAM lParam, ULONG Style)
{
  /* Not for child windows. */
  if (hWnd != (HWND)wParam)
    {
      return(0);
    }

  switch((INT_PTR) LOWORD(lParam))
    {
    case HTERROR:
      {
	WORD Msg = HIWORD(lParam);
	if (Msg == WM_LBUTTONDOWN || Msg == WM_MBUTTONDOWN ||
	    Msg == WM_RBUTTONDOWN || Msg == WM_XBUTTONDOWN)
	  {
	    MessageBeep(0);
	  }
	break;
      }

    case HTCLIENT:
      {
	HICON hCursor = (HICON)GetClassLongPtrW(hWnd, GCL_HCURSOR);
	if (hCursor)
	  {
	    SetCursor(hCursor);
	    return(TRUE);
	  }
	return(FALSE);
      }

    case HTLEFT:
    case HTRIGHT:
      {
        if (Style & WS_MAXIMIZE)
        {
          break;
        }
	return((LRESULT)SetCursor(LoadCursorW(0, IDC_SIZEWE)));
      }

    case HTTOP:
    case HTBOTTOM:
      {
        if (Style & WS_MAXIMIZE)
        {
          break;
        }
	return((LRESULT)SetCursor(LoadCursorW(0, IDC_SIZENS)));
      }

    case HTTOPLEFT:
    case HTBOTTOMRIGHT:
      {
        if (Style & WS_MAXIMIZE)
        {
          break;
        }
	return((LRESULT)SetCursor(LoadCursorW(0, IDC_SIZENWSE)));
      }

    case HTBOTTOMLEFT:
    case HTTOPRIGHT:
      {
        if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_MAXIMIZE)
        {
          break;
        }
	return((LRESULT)SetCursor(LoadCursorW(0, IDC_SIZENESW)));
      }
    }
  return((LRESULT)SetCursor(LoadCursorW(0, IDC_ARROW)));
}

static LONG
DefWndStartSizeMove(HWND hWnd, PWND Wnd, WPARAM wParam, POINT *capturePoint)
{
  LONG hittest = 0;
  POINT pt;
  MSG msg;
  RECT rectWindow;
  ULONG Style = Wnd->style;

  rectWindow = Wnd->rcWindow;

  if ((wParam & 0xfff0) == SC_MOVE)
    {
      /* Move pointer at the center of the caption */
      RECT rect;
      UserGetInsideRectNC(Wnd, &rect);
      if (Style & WS_SYSMENU)
	rect.left += GetSystemMetrics(SM_CXSIZE) + 1;
      if (Style & WS_MINIMIZEBOX)
	rect.right -= GetSystemMetrics(SM_CXSIZE) + 1;
      if (Style & WS_MAXIMIZEBOX)
	rect.right -= GetSystemMetrics(SM_CXSIZE) + 1;
      pt.x = rectWindow.left + (rect.right - rect.left) / 2;
      pt.y = rectWindow.top + rect.top + GetSystemMetrics(SM_CYSIZE)/2;
      hittest = HTCAPTION;
      *capturePoint = pt;
    }
  else  /* SC_SIZE */
    {
      pt.x = pt.y = 0;
      while(!hittest)
	{
          if (!GetMessageW(&msg, NULL, 0, 0)) break; //return 0;
          if (CallMsgFilterW( &msg, MSGF_SIZE )) continue;

	  switch(msg.message)
	    {
	    case WM_MOUSEMOVE:
	      hittest = DefWndNCHitTest(hWnd, msg.pt);
	      if ((hittest < HTLEFT) || (hittest > HTBOTTOMRIGHT))
		hittest = 0;
	      break;

	    case WM_LBUTTONUP:
	      return 0;

	    case WM_KEYDOWN:
	      switch(msg.wParam)
		{
		case VK_UP:
		  hittest = HTTOP;
		  pt.x =(rectWindow.left+rectWindow.right)/2;
		  pt.y = rectWindow.top + GetSystemMetrics(SM_CYFRAME) / 2;
		  break;
		case VK_DOWN:
		  hittest = HTBOTTOM;
		  pt.x =(rectWindow.left+rectWindow.right)/2;
		  pt.y = rectWindow.bottom - GetSystemMetrics(SM_CYFRAME) / 2;
		  break;
		case VK_LEFT:
		  hittest = HTLEFT;
		  pt.x = rectWindow.left + GetSystemMetrics(SM_CXFRAME) / 2;
		  pt.y =(rectWindow.top+rectWindow.bottom)/2;
		  break;
		case VK_RIGHT:
		  hittest = HTRIGHT;
		  pt.x = rectWindow.right - GetSystemMetrics(SM_CXFRAME) / 2;
		  pt.y =(rectWindow.top+rectWindow.bottom)/2;
		  break;
		case VK_RETURN:
		case VK_ESCAPE:
		  return 0;
		}
            default:
              TranslateMessage( &msg );
              DispatchMessageW( &msg );
              break;
	    }
	}
      *capturePoint = pt;
    }
    SetCursorPos( pt.x, pt.y );
    DefWndHandleSetCursor(hWnd, (WPARAM)hWnd, MAKELONG(hittest, WM_MOUSEMOVE), Style);
    return hittest;
}

#define ON_LEFT_BORDER(hit) \
 (((hit) == HTLEFT) || ((hit) == HTTOPLEFT) || ((hit) == HTBOTTOMLEFT))
#define ON_RIGHT_BORDER(hit) \
 (((hit) == HTRIGHT) || ((hit) == HTTOPRIGHT) || ((hit) == HTBOTTOMRIGHT))
#define ON_TOP_BORDER(hit) \
 (((hit) == HTTOP) || ((hit) == HTTOPLEFT) || ((hit) == HTTOPRIGHT))
#define ON_BOTTOM_BORDER(hit) \
 (((hit) == HTBOTTOM) || ((hit) == HTBOTTOMLEFT) || ((hit) == HTBOTTOMRIGHT))

static VOID
UserDrawWindowFrame(HDC hdc, const RECT *rect,
		    ULONG width, ULONG height)
{
  static HBRUSH hDraggingRectBrush = NULL;
  HBRUSH hbrush;

  if(!hDraggingRectBrush)
  {
    static HBITMAP hDraggingPattern = NULL;
    const DWORD Pattern[4] = {0x5555AAAA, 0x5555AAAA, 0x5555AAAA, 0x5555AAAA};

    hDraggingPattern = CreateBitmap(8, 8, 1, 1, Pattern);
    hDraggingRectBrush = CreatePatternBrush(hDraggingPattern);
  }

  hbrush = SelectObject( hdc, hDraggingRectBrush );
  PatBlt( hdc, rect->left, rect->top,
	  rect->right - rect->left - width, height, PATINVERT );
  PatBlt( hdc, rect->left, rect->top + height, width,
	  rect->bottom - rect->top - height, PATINVERT );
  PatBlt( hdc, rect->left + width, rect->bottom - 1,
	  rect->right - rect->left - width, -height, PATINVERT );
  PatBlt( hdc, rect->right - 1, rect->top, -width,
	  rect->bottom - rect->top - height, PATINVERT );
  SelectObject( hdc, hbrush );
}

static VOID
UserDrawMovingFrame(HDC hdc, RECT *rect, BOOL thickframe)
{
  if(thickframe)
  {
    UserDrawWindowFrame(hdc, rect, GetSystemMetrics(SM_CXFRAME), GetSystemMetrics(SM_CYFRAME));
  }
  else
  {
    UserDrawWindowFrame(hdc, rect, 1, 1);
  }
}

static VOID
DefWndDoSizeMove(HWND hwnd, WORD wParam)
{
  HRGN DesktopRgn;
  MSG msg;
  RECT sizingRect, mouseRect, origRect, clipRect, unmodRect;
  HDC hdc;
  LONG hittest = (LONG)(wParam & 0x0f);
  HCURSOR hDragCursor = 0, hOldCursor = 0;
  POINT minTrack, maxTrack;
  POINT capturePoint, pt;
  ULONG Style, ExStyle;
  BOOL thickframe;
  BOOL iconic;
  BOOL moved = FALSE;
  DWORD dwPoint = GetMessagePos();
  BOOL DragFullWindows = FALSE;
  HWND hWndParent = NULL;
  PWND Wnd;

  Wnd = ValidateHwnd(hwnd);
  if (!Wnd)
      return;

  Style = Wnd->style;
  ExStyle = Wnd->ExStyle;
  iconic = (Style & WS_MINIMIZE) != 0;

  SystemParametersInfoA(SPI_GETDRAGFULLWINDOWS, 0, &DragFullWindows, 0);

  pt.x = GET_X_LPARAM(dwPoint);
  pt.y = GET_Y_LPARAM(dwPoint);
  capturePoint = pt;

  if ((Style & WS_MAXIMIZE) || !IsWindowVisible(hwnd))
    {
      return;
    }

  thickframe = UserHasThickFrameStyle(Style, ExStyle) && !(Style & WS_MINIMIZE);
  if ((wParam & 0xfff0) == SC_MOVE)
    {
      if (!hittest)
	{
	  hittest = DefWndStartSizeMove(hwnd, Wnd, wParam, &capturePoint);
	}
      if (!hittest)
	{
	  return;
	}
    }
  else  /* SC_SIZE */
    {
      if (!thickframe)
	{
	  return;
	}
      if (hittest && ((wParam & 0xfff0) != SC_MOUSEMENU))
	{
          hittest += (HTLEFT - WMSZ_LEFT);
	}
      else
	{
	  SetCapture(hwnd);
	  hittest = DefWndStartSizeMove(hwnd, Wnd, wParam, &capturePoint);
	  if (!hittest)
	    {
	      ReleaseCapture();
	      return;
	    }
	}
    }

  /* Get min/max info */

  WinPosGetMinMaxInfo(hwnd, NULL, NULL, &minTrack, &maxTrack);
  sizingRect = Wnd->rcWindow;
  if (Style & WS_CHILD)
    {
      hWndParent = GetParent(hwnd);
      MapWindowPoints( 0, hWndParent, (LPPOINT)&sizingRect, 2 );
      unmodRect = sizingRect;
      GetClientRect(hWndParent, &mouseRect );
      clipRect = mouseRect;
      MapWindowPoints(hWndParent, HWND_DESKTOP, (LPPOINT)&clipRect, 2);
    }
  else
    {
      if(!(ExStyle & WS_EX_TOPMOST))
      {
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &clipRect, 0);
        mouseRect = clipRect;
      }
      else
      {
        SetRect(&mouseRect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
        clipRect = mouseRect;
      }
      unmodRect = sizingRect;
    }
  ClipCursor(&clipRect);

  origRect = sizingRect;
  if (ON_LEFT_BORDER(hittest))
    {
      mouseRect.left  = max( mouseRect.left, sizingRect.right-maxTrack.x );
      mouseRect.right = min( mouseRect.right, sizingRect.right-minTrack.x );
    }
  else if (ON_RIGHT_BORDER(hittest))
    {
      mouseRect.left  = max( mouseRect.left, sizingRect.left+minTrack.x );
      mouseRect.right = min( mouseRect.right, sizingRect.left+maxTrack.x );
    }
  if (ON_TOP_BORDER(hittest))
    {
      mouseRect.top    = max( mouseRect.top, sizingRect.bottom-maxTrack.y );
      mouseRect.bottom = min( mouseRect.bottom,sizingRect.bottom-minTrack.y);
    }
  else if (ON_BOTTOM_BORDER(hittest))
    {
      mouseRect.top    = max( mouseRect.top, sizingRect.top+minTrack.y );
      mouseRect.bottom = min( mouseRect.bottom, sizingRect.top+maxTrack.y );
    }
  if (Style & WS_CHILD)
    {
      MapWindowPoints( hWndParent, 0, (LPPOINT)&mouseRect, 2 );
    }

  SendMessageA( hwnd, WM_ENTERSIZEMOVE, 0, 0 );
  NtUserxSetGUIThreadHandle(MSQ_STATE_MOVESIZE, hwnd);
  if (GetCapture() != hwnd) SetCapture( hwnd );

  if (Style & WS_CHILD)
    {
      /* Retrieve a default cache DC (without using the window style) */
      hdc = GetDCEx(hWndParent, 0, DCX_CACHE);
      DesktopRgn = NULL;
    }
  else
    {
      hdc = GetDC( 0 );
      DesktopRgn = CreateRectRgnIndirect(&clipRect);
    }

  SelectObject(hdc, DesktopRgn);

  if( iconic ) /* create a cursor for dragging */
    {
      HICON hIcon = (HICON)GetClassLongPtrW(hwnd, GCL_HICON);
      if(!hIcon) hIcon = (HICON)SendMessageW( hwnd, WM_QUERYDRAGICON, 0, 0L);
      if( hIcon ) hDragCursor = CursorIconToCursor( hIcon, TRUE );
      if( !hDragCursor ) iconic = FALSE;
    }

  /* invert frame if WIN31_LOOK to indicate mouse click on caption */
  if( !iconic && !DragFullWindows)
    {
      UserDrawMovingFrame( hdc, &sizingRect, thickframe);
    }

  for(;;)
    {
      int dx = 0, dy = 0;

      if (!GetMessageW(&msg, 0, 0, 0)) break;
      if (CallMsgFilterW( &msg, MSGF_SIZE )) continue;

      /* Exit on button-up, Return, or Esc */
      if ((msg.message == WM_LBUTTONUP) ||
	  ((msg.message == WM_KEYDOWN) &&
	   ((msg.wParam == VK_RETURN) || (msg.wParam == VK_ESCAPE)))) break;

      if (msg.message == WM_PAINT)
        {
	  if(!iconic && !DragFullWindows) UserDrawMovingFrame( hdc, &sizingRect, thickframe );
	  UpdateWindow( msg.hwnd );
	  if(!iconic && !DragFullWindows) UserDrawMovingFrame( hdc, &sizingRect, thickframe );
	  continue;
        }

      if ((msg.message != WM_KEYDOWN) && (msg.message != WM_MOUSEMOVE))
      {
         TranslateMessage( &msg );
         DispatchMessageW( &msg );
         continue;  /* We are not interested in other messages */
      }

      pt = msg.pt;

      if (msg.message == WM_KEYDOWN) switch(msg.wParam)
	{
	case VK_UP:    pt.y -= 8; break;
	case VK_DOWN:  pt.y += 8; break;
	case VK_LEFT:  pt.x -= 8; break;
	case VK_RIGHT: pt.x += 8; break;
	}

      pt.x = max( pt.x, mouseRect.left );
      pt.x = min( pt.x, mouseRect.right );
      pt.y = max( pt.y, mouseRect.top );
      pt.y = min( pt.y, mouseRect.bottom );

      dx = pt.x - capturePoint.x;
      dy = pt.y - capturePoint.y;

      if (dx || dy)
	{
	  if( !moved )
	    {
	      moved = TRUE;

		if( iconic ) /* ok, no system popup tracking */
		  {
		    hOldCursor = SetCursor(hDragCursor);
		    ShowCursor( TRUE );
		  }
	    }

	  if (msg.message == WM_KEYDOWN) SetCursorPos( pt.x, pt.y );
	  else
	    {
	      RECT newRect = unmodRect;
	      WPARAM wpSizingHit = 0;

	      if (hittest == HTCAPTION) OffsetRect( &newRect, dx, dy );
	      if (ON_LEFT_BORDER(hittest)) newRect.left += dx;
	      else if (ON_RIGHT_BORDER(hittest)) newRect.right += dx;
	      if (ON_TOP_BORDER(hittest)) newRect.top += dy;
	      else if (ON_BOTTOM_BORDER(hittest)) newRect.bottom += dy;
	      if(!iconic && !DragFullWindows) UserDrawMovingFrame( hdc, &sizingRect, thickframe );
	      capturePoint = pt;

	      /* determine the hit location */
	      if (hittest >= HTLEFT && hittest <= HTBOTTOMRIGHT)
		wpSizingHit = WMSZ_LEFT + (hittest - HTLEFT);
	      unmodRect	= newRect;
	      SendMessageA( hwnd, WM_SIZING, wpSizingHit, (LPARAM)&newRect );

	      if (!iconic)
		{
		  if(!DragFullWindows)
		    UserDrawMovingFrame( hdc, &newRect, thickframe );
		  else {
		    /* To avoid any deadlocks, all the locks on the windows
		       structures must be suspended before the SetWindowPos */
		    SetWindowPos( hwnd, 0, newRect.left, newRect.top,
				  newRect.right - newRect.left,
				  newRect.bottom - newRect.top,
				  ( hittest == HTCAPTION ) ? SWP_NOSIZE : 0 );
		  }
		}
	      sizingRect = newRect;
	    }
	}
    }

  ReleaseCapture();
  ClipCursor(NULL);
  if( iconic )
    {
      if( moved ) /* restore cursors, show icon title later on */
	{
	  ShowCursor( FALSE );
	  SetCursor( hOldCursor );
	}
      DestroyCursor( hDragCursor );
    }
  else if(!DragFullWindows)
      UserDrawMovingFrame( hdc, &sizingRect, thickframe );

  if (Style & WS_CHILD)
    ReleaseDC( hWndParent, hdc );
  else
  {
    ReleaseDC( 0, hdc );
    if(DesktopRgn)
    {
      DeleteObject(DesktopRgn);
    }
  }

  if (ISITHOOKED(WH_CBT))
  {
      LRESULT lResult;
      NtUserMessageCall( hwnd, WM_CBT, HCBT_MOVESIZE, (LPARAM)&sizingRect, (ULONG_PTR)&lResult, FNID_DEFWINDOWPROC, FALSE);
      if (lResult) moved = FALSE;
  }

  NtUserxSetGUIThreadHandle(MSQ_STATE_MOVESIZE, NULL);
  SendMessageA( hwnd, WM_EXITSIZEMOVE, 0, 0 );
  SendMessageA( hwnd, WM_SETVISIBLE, !IsIconic(hwnd), 0L);

  /* window moved or resized */
  if (moved)
    {
      /* if the moving/resizing isn't canceled call SetWindowPos
       * with the new position or the new size of the window
       */
      if (!((msg.message == WM_KEYDOWN) && (msg.wParam == VK_ESCAPE)) )
        {
	  /* NOTE: SWP_NOACTIVATE prevents document window activation in Word 6 */
	  if(!DragFullWindows)
	    SetWindowPos( hwnd, 0, sizingRect.left, sizingRect.top,
			  sizingRect.right - sizingRect.left,
			  sizingRect.bottom - sizingRect.top,
			  ( hittest == HTCAPTION ) ? SWP_NOSIZE : 0 );
        }
      else { /* restore previous size/position */
	if(DragFullWindows)
	  SetWindowPos( hwnd, 0, origRect.left, origRect.top,
			origRect.right - origRect.left,
			origRect.bottom - origRect.top,
			( hittest == HTCAPTION ) ? SWP_NOSIZE : 0 );
      }
    }

  if( IsWindow(hwnd) )
    if( Style & WS_MINIMIZE )
      {
	/* Single click brings up the system menu when iconized */

	if( !moved )
	  {
	    if( Style & WS_SYSMENU )
	      SendMessageA( hwnd, WM_SYSCOMMAND,
			    SC_MOUSEMENU + HTSYSMENU, MAKELONG(pt.x,pt.y));
	  }
      }
}


/***********************************************************************
 *           DefWndTrackScrollBar
 *
 * Track a mouse button press on the horizontal or vertical scroll-bar.
 */
static  VOID
DefWndTrackScrollBar(HWND Wnd, WPARAM wParam, POINT Pt)
{
  INT ScrollBar;

  if (SC_HSCROLL == (wParam & 0xfff0))
    {
      if (HTHSCROLL != (wParam & 0x0f))
        {
          return;
        }
      ScrollBar = SB_HORZ;
    }
  else  /* SC_VSCROLL */
    {
      if (HTVSCROLL != (wParam & 0x0f))
        {
          return;
        }
      ScrollBar = SB_VERT;
    }
  ScrollTrackScrollBar(Wnd, ScrollBar, Pt );
}

LRESULT WINAPI DoAppSwitch( WPARAM wParam, LPARAM lParam);

LRESULT
DefWndHandleSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
  WINDOWPLACEMENT wp;
  POINT Pt;
  LRESULT lResult;

  if (!IsWindowEnabled( hWnd )) return 0;

  if (ISITHOOKED(WH_CBT))
  {
     NtUserMessageCall( hWnd, WM_SYSCOMMAND, wParam, lParam, (ULONG_PTR)&lResult, FNID_DEFWINDOWPROC, FALSE);
     if (lResult) return 0;
  }

  switch (wParam & 0xfff0)
    {
      case SC_MOVE:
      case SC_SIZE:
	DefWndDoSizeMove(hWnd, wParam);
	break;
      case SC_MINIMIZE:
        wp.length = sizeof(WINDOWPLACEMENT);
        if(GetWindowPlacement(hWnd, &wp))
        {
          wp.showCmd = SW_MINIMIZE;
          SetWindowPlacement(hWnd, &wp);
        }
        break;
      case SC_MAXIMIZE:
        wp.length = sizeof(WINDOWPLACEMENT);
        if(GetWindowPlacement(hWnd, &wp))
        {
          wp.showCmd = SW_MAXIMIZE;
          SetWindowPlacement(hWnd, &wp);
        }
        break;
      case SC_RESTORE:
        wp.length = sizeof(WINDOWPLACEMENT);
        if(GetWindowPlacement(hWnd, &wp))
        {
          wp.showCmd = SW_RESTORE;
          SetWindowPlacement(hWnd, &wp);
        }
        break;
      case SC_CLOSE:
        return SendMessageW(hWnd, WM_CLOSE, 0, 0);
//      case SC_DEFAULT:
      case SC_MOUSEMENU:
        {
          Pt.x = (short)LOWORD(lParam);
          Pt.y = (short)HIWORD(lParam);
          MenuTrackMouseMenuBar(hWnd, wParam & 0x000f, Pt);
        }
	break;
      case SC_KEYMENU:
        MenuTrackKbdMenuBar(hWnd, wParam, (WCHAR)lParam);
	break;
      case SC_VSCROLL:
      case SC_HSCROLL:
        {
          Pt.x = (short)LOWORD(lParam);
          Pt.y = (short)HIWORD(lParam);
          DefWndTrackScrollBar(hWnd, wParam, Pt);
        }
	break;

      case SC_SCREENSAVE:
        NtUserMessageCall( hWnd, WM_SYSCOMMAND, wParam, lParam, (ULONG_PTR)&lResult, FNID_DEFWINDOWPROC, FALSE);
        break;

      case SC_NEXTWINDOW:
      case SC_PREVWINDOW:
        DoAppSwitch( wParam, lParam);
        break;

      case SC_HOTKEY:
        {
           HWND hwnd, hWndLastActive;
           PWND pWnd;

           hwnd = (HWND)lParam;
           pWnd = ValidateHwnd(hwnd);
           if (pWnd)
           {
              hWndLastActive = GetLastActivePopup(hwnd);
              if (hWndLastActive)
              {
                 hwnd = hWndLastActive;
                 pWnd = ValidateHwnd(hwnd);
              }
              SetForegroundWindow(hwnd);
              if (pWnd->style & WS_MINIMIZE)
              {
                 PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
              }
           }
        }
        break;

      default:
        FIXME("Unimplemented DefWndHandleSysCommand wParam 0x%x\n",wParam);
        break;
    }

  return(0);
}

LRESULT
DefWndHandleWindowPosChanging(HWND hWnd, WINDOWPOS* Pos)
{
    POINT maxTrack, minTrack;
    LONG style = GetWindowLongPtrA(hWnd, GWL_STYLE);

    if (Pos->flags & SWP_NOSIZE) return 0;
    if ((style & WS_THICKFRAME) || ((style & (WS_POPUP | WS_CHILD)) == 0))
    {
        WinPosGetMinMaxInfo(hWnd, NULL, NULL, &minTrack, &maxTrack);
        Pos->cx = min(Pos->cx, maxTrack.x);
        Pos->cy = min(Pos->cy, maxTrack.y);
        if (!(style & WS_MINIMIZE))
        {
            if (Pos->cx < minTrack.x) Pos->cx = minTrack.x;
            if (Pos->cy < minTrack.y) Pos->cy = minTrack.y;
        }
    }
    else
    {
        Pos->cx = max(Pos->cx, 0);
        Pos->cy = max(Pos->cy, 0);
    }
    return 0;
}

LRESULT
DefWndHandleWindowPosChanged(HWND hWnd, WINDOWPOS* Pos)
{
  RECT Rect;

  GetClientRect(hWnd, &Rect);
  MapWindowPoints(hWnd, (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD ?
                         GetParent(hWnd) : NULL), (LPPOINT) &Rect, 2);

  if (! (Pos->flags & SWP_NOCLIENTMOVE))
    {
      SendMessageW(hWnd, WM_MOVE, 0, MAKELONG(Rect.left, Rect.top));
    }

  if (! (Pos->flags & SWP_NOCLIENTSIZE))
    {
      WPARAM wp = SIZE_RESTORED;
      if (IsZoomed(hWnd))
        {
          wp = SIZE_MAXIMIZED;
        }
      else if (IsIconic(hWnd))
        {
          wp = SIZE_MINIMIZED;
        }
      SendMessageW(hWnd, WM_SIZE, wp,
                   MAKELONG(Rect.right - Rect.left, Rect.bottom - Rect.top));
    }

  return 0;
}

/***********************************************************************
 *           DefWndControlColor
 *
 * Default colors for control painting.
 */
HBRUSH
DefWndControlColor(HDC hDC, UINT ctlType)
{
  if (ctlType == CTLCOLOR_SCROLLBAR)
  {
      HBRUSH hb = GetSysColorBrush(COLOR_SCROLLBAR);
      COLORREF bk = GetSysColor(COLOR_3DHILIGHT);
      SetTextColor(hDC, GetSysColor(COLOR_3DFACE));
      SetBkColor(hDC, bk);

      /* if COLOR_WINDOW happens to be the same as COLOR_3DHILIGHT
       * we better use 0x55aa bitmap brush to make scrollbar's background
       * look different from the window background.
       */
      if ( bk == GetSysColor(COLOR_WINDOW))
          return gpsi->hbrGray;

      UnrealizeObject( hb );
      return hb;
  }

  SetTextColor(hDC, GetSysColor(COLOR_WINDOWTEXT));

  if ((ctlType == CTLCOLOR_EDIT) || (ctlType == CTLCOLOR_LISTBOX))
  {
      SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
  }
  else
  {
      SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
      return GetSysColorBrush(COLOR_3DFACE);
  }

  return GetSysColorBrush(COLOR_WINDOW);
}

static void DefWndPrint( HWND hwnd, HDC hdc, ULONG uFlags)
{
  /*
   * Visibility flag.
   */
  if ( (uFlags & PRF_CHECKVISIBLE) &&
       !IsWindowVisible(hwnd) )
      return;

  /*
   * Unimplemented flags.
   */
  if ( (uFlags & PRF_CHILDREN) ||
       (uFlags & PRF_OWNED)    ||
       (uFlags & PRF_NONCLIENT) )
  {
    FIXME("WM_PRINT message with unsupported flags\n");
  }

  /*
   * Background
   */
  if ( uFlags & PRF_ERASEBKGND)
    SendMessageW(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);

  /*
   * Client area
   */
  if ( uFlags & PRF_CLIENT)
    SendMessageW(hwnd, WM_PRINTCLIENT, (WPARAM)hdc, uFlags);
}

static BOOL CALLBACK
UserSendUiUpdateMsg(HWND hwnd, LPARAM lParam)
{
    SendMessageW(hwnd, WM_UPDATEUISTATE, (WPARAM)lParam, 0);
    return TRUE;
}


VOID FASTCALL
DefWndScreenshot(HWND hWnd)
{
    RECT rect;
    HDC hdc;
    INT w;
    INT h;
    HBITMAP hbitmap;
    HDC hdc2;

    OpenClipboard(hWnd);
    EmptyClipboard();

    hdc = GetWindowDC(hWnd);
    GetWindowRect(hWnd, &rect);
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;

    hbitmap = CreateCompatibleBitmap(hdc, w, h);
    hdc2 = CreateCompatibleDC(hdc);
    SelectObject(hdc2, hbitmap);

    BitBlt(hdc2, 0, 0, w, h,
           hdc, 0, 0,
           SRCCOPY);

    SetClipboardData(CF_BITMAP, hbitmap);

    ReleaseDC(hWnd, hdc);
    ReleaseDC(hWnd, hdc2);

    CloseClipboard();

}



LRESULT WINAPI
User32DefWindowProc(HWND hWnd,
		    UINT Msg,
		    WPARAM wParam,
		    LPARAM lParam,
		    BOOL bUnicode)
{
    switch (Msg)
    {
	case WM_NCPAINT:
	{
            return DefWndNCPaint(hWnd, (HRGN)wParam, -1);
        }

        case WM_NCCALCSIZE:
        {
            return DefWndNCCalcSize(hWnd, (BOOL)wParam, (RECT*)lParam);
        }

        case WM_POPUPSYSTEMMENU:
        {
            /* This is an undocumented message used by the windows taskbar to
               display the system menu of windows that belong to other processes. */
            HMENU menu = GetSystemMenu(hWnd, FALSE);

            if (menu)
                TrackPopupMenu(menu, TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
                               LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);
            return 0;
        }

        case WM_NCACTIVATE:
        {
            return DefWndNCActivate(hWnd, wParam);
        }

        case WM_NCHITTEST:
        {
            POINT Point;
            Point.x = GET_X_LPARAM(lParam);
            Point.y = GET_Y_LPARAM(lParam);
            return (DefWndNCHitTest(hWnd, Point));
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
            iF10Key = iMenuSysKey = 0;
            break;

        case WM_NCLBUTTONDOWN:
        {
            return (DefWndNCLButtonDown(hWnd, wParam, lParam));
        }

        case WM_LBUTTONDBLCLK:
            return (DefWndNCLButtonDblClk(hWnd, HTCLIENT, lParam));

        case WM_NCLBUTTONDBLCLK:
        {
            return (DefWndNCLButtonDblClk(hWnd, wParam, lParam));
        }

        case WM_WINDOWPOSCHANGING:
        {
            return (DefWndHandleWindowPosChanging(hWnd, (WINDOWPOS*)lParam));
        }

        case WM_WINDOWPOSCHANGED:
        {
            return (DefWndHandleWindowPosChanged(hWnd, (WINDOWPOS*)lParam));
        }

        case WM_NCRBUTTONDOWN:
        {
            /* in Windows, capture is taken when right-clicking on the caption bar */
            if (wParam == HTCAPTION)
            {
                SetCapture(hWnd);
            }
            break;
        }

        case WM_RBUTTONUP:
        {
            POINT Pt;
            if (hWnd == GetCapture())
            {
                ReleaseCapture();
            }
            Pt.x = GET_X_LPARAM(lParam);
            Pt.y = GET_Y_LPARAM(lParam);
            ClientToScreen(hWnd, &Pt);
            lParam = MAKELPARAM(Pt.x, Pt.y);
            if (bUnicode)
            {
                SendMessageW(hWnd, WM_CONTEXTMENU, (WPARAM)hWnd, lParam);
            }
            else
            {
                SendMessageA(hWnd, WM_CONTEXTMENU, (WPARAM)hWnd, lParam);
            }
            break;
        }

        case WM_NCRBUTTONUP:
          /*
           * FIXME : we must NOT send WM_CONTEXTMENU on a WM_NCRBUTTONUP (checked
           * in Windows), but what _should_ we do? According to MSDN :
           * "If it is appropriate to do so, the system sends the WM_SYSCOMMAND
           * message to the window". When is it appropriate?
           */
            break;

        case WM_CONTEXTMENU:
        {
            if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD)
            {
                if (bUnicode)
                {
                    SendMessageW(GetParent(hWnd), Msg, wParam, lParam);
                }
                else
                {
                    SendMessageA(GetParent(hWnd), WM_CONTEXTMENU, wParam, lParam);
                }
            }
            else
            {
                POINT Pt;
                LONG_PTR Style;
                LONG HitCode;

                Style = GetWindowLongPtrW(hWnd, GWL_STYLE);

                Pt.x = GET_X_LPARAM(lParam);
                Pt.y = GET_Y_LPARAM(lParam);
                if (Style & WS_CHILD)
                {
                    ScreenToClient(GetParent(hWnd), &Pt);
                }

                HitCode = DefWndNCHitTest(hWnd, Pt);

                if (HitCode == HTCAPTION || HitCode == HTSYSMENU)
                {
                    HMENU SystemMenu;
                    UINT Flags;

                    if((SystemMenu = GetSystemMenu(hWnd, FALSE)))
                    {
                      MenuInitSysMenuPopup(SystemMenu, GetWindowLongPtrW(hWnd, GWL_STYLE),
                                           GetClassLongPtrW(hWnd, GCL_STYLE), HitCode);

                      if(HitCode == HTCAPTION)
                        Flags = TPM_LEFTBUTTON | TPM_RIGHTBUTTON;
                      else
                        Flags = TPM_LEFTBUTTON;

                      TrackPopupMenu(SystemMenu, Flags,
                                     Pt.x, Pt.y, 0, hWnd, NULL);
                    }
                }
	    }
            break;
        }

        case WM_PRINT:
        {
            DefWndPrint(hWnd, (HDC)wParam, lParam);
            return (0);
        }

        case WM_SYSCOLORCHANGE:
        {
            /* force to redraw non-client area */
            DefWndNCPaint(hWnd, HRGN_WINDOW, -1);
            /* Use InvalidateRect to redraw client area, enable
             * erase to redraw all subcontrols otherwise send the
             * WM_SYSCOLORCHANGE to child windows/controls is required
             */
            InvalidateRect(hWnd,NULL,TRUE);
            return (0);
        }

        case WM_PAINTICON:
        case WM_PAINT:
        {
            PAINTSTRUCT Ps;
            HDC hDC = BeginPaint(hWnd, &Ps);
            if (hDC)
            {
                HICON hIcon;

                if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_MINIMIZE &&
                    (hIcon = (HICON)GetClassLongPtrW(hWnd, GCL_HICON)) != NULL)
                {
                    RECT ClientRect;
                    INT x, y;
                    GetClientRect(hWnd, &ClientRect);
                    x = (ClientRect.right - ClientRect.left -
                         GetSystemMetrics(SM_CXICON)) / 2;
                    y = (ClientRect.bottom - ClientRect.top -
                         GetSystemMetrics(SM_CYICON)) / 2;
                    DrawIcon(hDC, x, y, hIcon);
                }
                EndPaint(hWnd, &Ps);
            }
            return (0);
        }

        case WM_SYNCPAINT:
        {
            HRGN hRgn;
            hRgn = CreateRectRgn(0, 0, 0, 0);
            if (GetUpdateRgn(hWnd, hRgn, FALSE) != NULLREGION)
            {
                RedrawWindow(hWnd, NULL, hRgn,
			        RDW_ERASENOW | RDW_ERASE | RDW_FRAME |
                    RDW_ALLCHILDREN);
            }
            DeleteObject(hRgn);
            return (0);
        }

        case WM_SETREDRAW:
        {
            LONG_PTR Style = GetWindowLongPtrW(hWnd, GWL_STYLE);
            if (wParam) SetWindowLongPtr(hWnd, GWL_STYLE, Style | WS_VISIBLE);
            else
            {
                RedrawWindow(hWnd, NULL, 0, RDW_ALLCHILDREN | RDW_VALIDATE);
                Style &= ~WS_VISIBLE;
                SetWindowLongPtr(hWnd, GWL_STYLE, Style);
            }
            return (0);
        }

        case WM_CLOSE:
        {
            DestroyWindow(hWnd);
            return (0);
        }

        case WM_MOUSEACTIVATE:
        {
            if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD)
            {
                LONG Ret;
                if (bUnicode)
                {
                    Ret = SendMessageW(GetParent(hWnd), WM_MOUSEACTIVATE,
                                       wParam, lParam);
                }
                else
                {
                    Ret = SendMessageA(GetParent(hWnd), WM_MOUSEACTIVATE,
                                       wParam, lParam);
                }
                if (Ret)
                {
                    return (Ret);
                }
            }
            return ((LOWORD(lParam) >= HTCLIENT) ? MA_ACTIVATE : MA_NOACTIVATE);
        }

        case WM_ACTIVATE:
        {
            /* Check if the window is minimized. */
            if (LOWORD(wParam) != WA_INACTIVE &&
                !(GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_MINIMIZE))
            {
                SetFocus(hWnd);
            }
            break;
        }

        case WM_MOUSEWHEEL:
        {
            if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CHILD)
            {
                if (bUnicode)
                {
                    return (SendMessageW(GetParent(hWnd), WM_MOUSEWHEEL,
                                         wParam, lParam));
                }
                else
                {
                    return (SendMessageA(GetParent(hWnd), WM_MOUSEWHEEL,
                                         wParam, lParam));
                }
            }
            break;
        }

        case WM_ERASEBKGND:
        case WM_ICONERASEBKGND:
        {
            RECT Rect;
            HBRUSH hBrush = (HBRUSH)GetClassLongPtrW(hWnd, GCL_HBRBACKGROUND);

            if (NULL == hBrush)
            {
                return 0;
            }
            if (GetClassLongPtrW(hWnd, GCL_STYLE) & CS_PARENTDC)
            {
                /* can't use GetClipBox with a parent DC or we fill the whole parent */
                GetClientRect(hWnd, &Rect);
                DPtoLP((HDC)wParam, (LPPOINT)&Rect, 2);
            }
            else
            {
                GetClipBox((HDC)wParam, &Rect);
            }
            FillRect((HDC)wParam, &Rect, hBrush);
            return (1);
        }

        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORSCROLLBAR:
	    return (LRESULT) DefWndControlColor((HDC)wParam, Msg - WM_CTLCOLORMSGBOX);

        case WM_CTLCOLOR:
            return (LRESULT) DefWndControlColor((HDC)wParam, HIWORD(lParam));

        case WM_SETCURSOR:
        {
            LONG_PTR Style = GetWindowLongPtrW(hWnd, GWL_STYLE);

            if (Style & WS_CHILD)
            {
                /* with the exception of the border around a resizable wnd,
                 * give the parent first chance to set the cursor */
                if (LOWORD(lParam) < HTLEFT || LOWORD(lParam) > HTBOTTOMRIGHT)
                {
                    HWND parent = GetParent( hWnd );
                    if (bUnicode)
                    {
                       if (parent != GetDesktopWindow() &&
                           SendMessageW( parent, WM_SETCURSOR, wParam, lParam))
                          return TRUE;
                    }
                    else
                    {
                       if (parent != GetDesktopWindow() &&                    
                           SendMessageA( parent, WM_SETCURSOR, wParam, lParam))
                          return TRUE;
                    }
                }
            }
            return (DefWndHandleSetCursor(hWnd, wParam, lParam, Style));
        }

        case WM_SYSCOMMAND:
            return (DefWndHandleSysCommand(hWnd, wParam, lParam));

        case WM_KEYDOWN:
            if(wParam == VK_F10) iF10Key = VK_F10;
            break;

        case WM_SYSKEYDOWN:
        {
            if (HIWORD(lParam) & KF_ALTDOWN)
            {   /* Previous state, if the key was down before this message,
                   this is a cheap way to ignore autorepeat keys. */
                if ( !(HIWORD(lParam) & KF_REPEAT) )
                {
                   if ( ( wParam == VK_MENU  ||
                          wParam == VK_LMENU ||
                          wParam == VK_RMENU ) && !iMenuSysKey )
                       iMenuSysKey = 1;
                   else
                       iMenuSysKey = 0;
                }

                iF10Key = 0;

                if (wParam == VK_F4) /* Try to close the window */
                {
                   HWND top = GetAncestor(hWnd, GA_ROOT);
                   if (!(GetClassLongPtrW(top, GCL_STYLE) & CS_NOCLOSE))
                      PostMessageW(top, WM_SYSCOMMAND, SC_CLOSE, 0);
                }
                else if (wParam == VK_SNAPSHOT) // Alt-VK_SNAPSHOT?
                {
                   HWND hwnd = hWnd;
                   while (GetParent(hwnd) != NULL)
                   {
                       hwnd = GetParent(hwnd);
                   }
                   DefWndScreenshot(hwnd);
                }
                else if ( wParam == VK_ESCAPE || wParam == VK_TAB ) // Alt-Tab/ESC Alt-Shift-Tab/ESC
                {
                   WPARAM wParamTmp;
                   HWND Active = GetActiveWindow(); // Noticed MDI problem.
                   if (!Active)
                   {
                      FIXME("WM_SYSKEYDOWN VK_ESCAPE no active\n");
                      break;
                   }
                   wParamTmp = GetKeyState(VK_SHIFT) & 0x8000 ? SC_PREVWINDOW : SC_NEXTWINDOW;
                   SendMessageW( Active, WM_SYSCOMMAND, wParamTmp, wParam );
                }
            }
            else if( wParam == VK_F10 )
            {
                if (GetKeyState(VK_SHIFT) & 0x8000)
                    SendMessageW( hWnd, WM_CONTEXTMENU, (WPARAM)hWnd, MAKELPARAM(-1, -1) );
                iF10Key = 1;
            }
            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
           /* Press and release F10 or ALT */
            if (((wParam == VK_MENU || wParam == VK_LMENU || wParam == VK_RMENU)
                 && iMenuSysKey) || ((wParam == VK_F10) && iF10Key))
                SendMessageW( GetAncestor( hWnd, GA_ROOT ), WM_SYSCOMMAND, SC_KEYMENU, 0L );
            iMenuSysKey = iF10Key = 0;
            break;
        }

        case WM_SYSCHAR:
        {
            iMenuSysKey = 0;
            if (wParam == VK_RETURN && IsIconic(hWnd))
            {
                PostMessageW( hWnd, WM_SYSCOMMAND, SC_RESTORE, 0L );
                break;
            }
            if ((HIWORD(lParam) & KF_ALTDOWN) && wParam)
            {
                if (wParam == VK_TAB || wParam == VK_ESCAPE) break;
                if (wParam == VK_SPACE && (GetWindowLongPtrW( hWnd, GWL_STYLE ) & WS_CHILD))
                    SendMessageW( GetParent(hWnd), Msg, wParam, lParam );
                else
                    SendMessageW( hWnd, WM_SYSCOMMAND, SC_KEYMENU, wParam );
            }
            else /* check for Ctrl-Esc */
                if (wParam != VK_ESCAPE) MessageBeep(0);
            break;
        }

        case WM_CANCELMODE:
        {
            iMenuSysKey = 0;
            /* FIXME: Check for a desktop. */
            //if (!(GetWindowLongPtrW( hWnd, GWL_STYLE ) & WS_CHILD)) EndMenu();
            MENU_EndMenu( hWnd );
            if (GetCapture() == hWnd)
            {
                ReleaseCapture();
            }
            break;
        }

        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
            return (-1);
/*
        case WM_DROPOBJECT:
            return DRAG_FILE;
*/
        case WM_QUERYDROPOBJECT:
        {
            if (GetWindowLongPtrW(hWnd, GWL_EXSTYLE) & WS_EX_ACCEPTFILES)
            {
                return(1);
            }
            break;
        }

        case WM_QUERYDRAGICON:
        {
            UINT Len;
            HICON hIcon;

            hIcon = (HICON)GetClassLongPtrW(hWnd, GCL_HICON);
            if (hIcon)
            {
                return ((LRESULT)hIcon);
            }
            for (Len = 1; Len < 64; Len++)
            {
                if ((hIcon = LoadIconW(NULL, MAKEINTRESOURCEW(Len))) != NULL)
                {
                    return((LRESULT)hIcon);
                }
            }
            return ((LRESULT)LoadIconW(0, IDI_APPLICATION));
        }

        case WM_ISACTIVEICON:
        {
           PWND pWnd;
           BOOL isai;
           pWnd = ValidateHwnd(hWnd);
           if (!pWnd) return 0;
           isai = (pWnd->state & WNDS_ACTIVEFRAME) != 0;
           return isai;
        }

        case WM_NOTIFYFORMAT:
        {
            if (lParam == NF_QUERY)
                return IsWindowUnicode(hWnd) ? NFR_UNICODE : NFR_ANSI;
            break;
        }

        case WM_SETICON:
        {
           INT Index = (wParam != 0) ? GCL_HICON : GCL_HICONSM;
           HICON hOldIcon = (HICON)GetClassLongPtrW(hWnd, Index);
           SetClassLongPtrW(hWnd, Index, lParam);
           SetWindowPos(hWnd, 0, 0, 0, 0, 0,
		       SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE |
		       SWP_NOACTIVATE | SWP_NOZORDER);
           return ((LRESULT)hOldIcon);
        }

        case WM_GETICON:
        {
            INT Index = (wParam == ICON_BIG) ? GCL_HICON : GCL_HICONSM;
            return (GetClassLongPtrW(hWnd, Index));
        }

        case WM_HELP:
        {
            if (bUnicode)
            {
                SendMessageW(GetParent(hWnd), Msg, wParam, lParam);
            }
            else
            {
                SendMessageA(GetParent(hWnd), Msg, wParam, lParam);
            }
            break;
        }

        case WM_SYSTIMER:
        {
          THRDCARETINFO CaretInfo;
          switch(wParam)
          {
            case 0xffff: /* Caret timer */
              /* switch showing byte in win32k and get information about the caret */
              if(NtUserxSwitchCaretShowing(&CaretInfo) && (CaretInfo.hWnd == hWnd))
              {
                DrawCaret(hWnd, &CaretInfo);
              }
              break;
          }
          break;
        }

        case WM_QUERYOPEN:
        case WM_QUERYENDSESSION:
        {
            return (1);
        }

        case WM_INPUTLANGCHANGEREQUEST:
        {
            HKL NewHkl;

            if(wParam & INPUTLANGCHANGE_BACKWARD
               && wParam & INPUTLANGCHANGE_FORWARD)
            {
                return FALSE;
            }

            //FIXME: What to do with INPUTLANGCHANGE_SYSCHARSET ?

            if(wParam & INPUTLANGCHANGE_BACKWARD) NewHkl = (HKL) HKL_PREV;
            else if(wParam & INPUTLANGCHANGE_FORWARD) NewHkl = (HKL) HKL_NEXT;
            else NewHkl = (HKL) lParam;

            NtUserActivateKeyboardLayout(NewHkl, 0);

            return TRUE;
        }

        case WM_INPUTLANGCHANGE:
        {
            int count = 0;
            HWND *win_array = WIN_ListChildren( hWnd );

            if (!win_array)
                break;
            while (win_array[count])
                SendMessageW( win_array[count++], WM_INPUTLANGCHANGE, wParam, lParam);
            HeapFree(GetProcessHeap(),0,win_array);
            break;
        }

        case WM_QUERYUISTATE:
        {
            LRESULT Ret = 0;
            PWND Wnd = ValidateHwnd(hWnd);
            if (Wnd != NULL)
            {
                if (Wnd->HideFocus)
                    Ret |= UISF_HIDEFOCUS;
                if (Wnd->HideAccel)
                    Ret |= UISF_HIDEACCEL;
            }
            return Ret;
        }

        case WM_CHANGEUISTATE:
        {
            BOOL AlwaysShowCues = FALSE;
            WORD Action = LOWORD(wParam);
            WORD Flags = HIWORD(wParam);
            PWND Wnd;

            SystemParametersInfoW(SPI_GETKEYBOARDCUES, 0, &AlwaysShowCues, 0);
            if (AlwaysShowCues)
                break;

            Wnd= ValidateHwnd(hWnd);
            if (!Wnd || lParam != 0)
                break;

            if (Flags & ~(UISF_HIDEFOCUS | UISF_HIDEACCEL | UISF_ACTIVE))
                break;

            if (Flags & UISF_ACTIVE)
            {
                WARN("WM_CHANGEUISTATE does not yet support UISF_ACTIVE!\n");
            }

            if (Action == UIS_INITIALIZE)
            {
                PDESKTOPINFO Desk = GetThreadDesktopInfo();
                if (Desk == NULL)
                    break;

                Action = Desk->LastInputWasKbd ? UIS_CLEAR : UIS_SET;
                Flags = UISF_HIDEFOCUS | UISF_HIDEACCEL;

                /* We need to update wParam in case we need to send out messages */
                wParam = MAKEWPARAM(Action, Flags);
            }

            switch (Action)
            {
                case UIS_SET:
                    /* See if we actually need to change something */
                    if ((Flags & UISF_HIDEFOCUS) && !Wnd->HideFocus)
                        break;
                    if ((Flags & UISF_HIDEACCEL) && !Wnd->HideAccel)
                        break;

                    /* Don't need to do anything... */
                    return 0;

                case UIS_CLEAR:
                    /* See if we actually need to change something */
                    if ((Flags & UISF_HIDEFOCUS) && Wnd->HideFocus)
                        break;
                    if ((Flags & UISF_HIDEACCEL) && Wnd->HideAccel)
                        break;

                    /* Don't need to do anything... */
                    return 0;

                default:
                    WARN("WM_CHANGEUISTATE: Unsupported Action 0x%x\n", Action);
                    break;
            }

            if ((Wnd->style & WS_CHILD) && Wnd->spwndParent != NULL)
            {
                /* We're a child window and we need to pass this message down until
                   we reach the root */
                hWnd = UserHMGetHandle((PWND)DesktopPtrToUser(Wnd->spwndParent));
            }
            else
            {
                /* We're a top level window, we need to change the UI state */
                Msg = WM_UPDATEUISTATE;
            }

            if (bUnicode)
                return SendMessageW(hWnd, Msg, wParam, lParam);
            else
                return SendMessageA(hWnd, Msg, wParam, lParam);
        }

        case WM_UPDATEUISTATE:
        {
            BOOL Change = TRUE;
            BOOL AlwaysShowCues = FALSE;
            WORD Action = LOWORD(wParam);
            WORD Flags = HIWORD(wParam);
            PWND Wnd;

            SystemParametersInfoW(SPI_GETKEYBOARDCUES, 0, &AlwaysShowCues, 0);
            if (AlwaysShowCues)
                break;

            Wnd = ValidateHwnd(hWnd);
            if (!Wnd || lParam != 0)
                break;

            if (Flags & ~(UISF_HIDEFOCUS | UISF_HIDEACCEL | UISF_ACTIVE))
                break;

            if (Flags & UISF_ACTIVE)
            {
                WARN("WM_UPDATEUISTATE does not yet support UISF_ACTIVE!\n");
            }

            if (Action == UIS_INITIALIZE)
            {
                PDESKTOPINFO Desk = GetThreadDesktopInfo();
                if (Desk == NULL)
                    break;

                Action = Desk->LastInputWasKbd ? UIS_CLEAR : UIS_SET;
                Flags = UISF_HIDEFOCUS | UISF_HIDEACCEL;

                /* We need to update wParam for broadcasting the update */
                wParam = MAKEWPARAM(Action, Flags);
            }

            switch (Action)
            {
                case UIS_SET:
                    /* See if we actually need to change something */
                    if ((Flags & UISF_HIDEFOCUS) && !Wnd->HideFocus)
                        break;
                    if ((Flags & UISF_HIDEACCEL) && !Wnd->HideAccel)
                        break;

                    /* Don't need to do anything... */
                    Change = FALSE;
                    break;

                case UIS_CLEAR:
                    /* See if we actually need to change something */
                    if ((Flags & UISF_HIDEFOCUS) && Wnd->HideFocus)
                        break;
                    if ((Flags & UISF_HIDEACCEL) && Wnd->HideAccel)
                        break;

                    /* Don't need to do anything... */
                    Change = FALSE;
                    break;

                default:
                    WARN("WM_UPDATEUISTATE: Unsupported Action 0x%x\n", Action);
                    return 0;
            }

            /* Pack the information and call win32k */
            if (Change)
            {
                if (!NtUserxUpdateUiState(hWnd, Flags | ((DWORD)Action << 3)))
                    break;
            }

            /* Always broadcast the update to all children */
            EnumChildWindows(hWnd,
                             UserSendUiUpdateMsg,
                             (LPARAM)wParam);

            break;
        }

/* Move to win32k !*/
        case WM_SHOWWINDOW:
            if (!lParam) break; // Call when it is necessary.
        case WM_CLIENTSHUTDOWN:
        case WM_GETHOTKEY:
        case WM_SETHOTKEY:
        {
            LRESULT lResult;
            NtUserMessageCall( hWnd, Msg, wParam, lParam, (ULONG_PTR)&lResult, FNID_DEFWINDOWPROC, !bUnicode);
            return lResult;
        }
    }
    return 0;
}


/*
 * helpers for calling IMM32 (from Wine 10/22/2008)
 *
 * WM_IME_* messages are generated only by IMM32,
 * so I assume imm32 is already LoadLibrary-ed.
 */
static HWND
DefWndImmGetDefaultIMEWnd(HWND hwnd)
{
    HINSTANCE hInstIMM = GetModuleHandleW(L"imm32\0");
    HWND (WINAPI *pFunc)(HWND);
    HWND hwndRet = 0;

    if (!hInstIMM)
    {
        ERR("cannot get IMM32 handle\n");
        return 0;
    }

    pFunc = (void*) GetProcAddress(hInstIMM, "ImmGetDefaultIMEWnd");
    if (pFunc != NULL)
        hwndRet = (*pFunc)(hwnd);

    return hwndRet;
}


static BOOL
DefWndImmIsUIMessageA(HWND hwndIME, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInstIMM = GetModuleHandleW(L"imm32\0");
    BOOL (WINAPI *pFunc)(HWND,UINT,WPARAM,LPARAM);
    BOOL fRet = FALSE;

    if (!hInstIMM)
    {
        ERR("cannot get IMM32 handle\n");
        return FALSE;
    }

    pFunc = (void*) GetProcAddress(hInstIMM, "ImmIsUIMessageA");
    if (pFunc != NULL)
        fRet = (*pFunc)(hwndIME, msg, wParam, lParam);

    return fRet;
}


static BOOL
DefWndImmIsUIMessageW(HWND hwndIME, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HINSTANCE hInstIMM = GetModuleHandleW(L"imm32\0");
    BOOL (WINAPI *pFunc)(HWND,UINT,WPARAM,LPARAM);
    BOOL fRet = FALSE;

    if (!hInstIMM)
    {
        ERR("cannot get IMM32 handle\n");
        return FALSE;
    }

    pFunc = (void*) GetProcAddress(hInstIMM, "ImmIsUIMessageW");
    if (pFunc != NULL)
        fRet = (*pFunc)(hwndIME, msg, wParam, lParam);

    return fRet;
}


LRESULT WINAPI
RealDefWindowProcA(HWND hWnd,
                   UINT Msg,
                   WPARAM wParam,
                   LPARAM lParam)
{
    LRESULT Result = 0;
    PWND Wnd;

    SPY_EnterMessage(SPY_DEFWNDPROC, hWnd, Msg, wParam, lParam);
    switch (Msg)
    {
        case WM_NCCREATE:
        {
            if (lParam)
            {
                LPCREATESTRUCTA cs = (LPCREATESTRUCTA)lParam;
                /* check for string, as static icons, bitmaps (SS_ICON, SS_BITMAP)
                 * may have child window IDs instead of window name */
                if (HIWORD(cs->lpszName))
                {
                    DefSetText(hWnd, (PCWSTR)cs->lpszName, TRUE);
                }
                Result = 1;
            }
            break;
        }

        case WM_GETTEXTLENGTH:
        {
            PWSTR buf;
            ULONG len;

            Wnd = ValidateHwnd(hWnd);
            if (Wnd != NULL && Wnd->strName.Length != 0)
            {
                buf = DesktopPtrToUser(Wnd->strName.Buffer);
                if (buf != NULL &&
                    NT_SUCCESS(RtlUnicodeToMultiByteSize(&len,
                                                         buf,
                                                         Wnd->strName.Length)))
                {
                    Result = (LRESULT) len;
                }
            }
            else Result = 0L;

            break;
        }

        case WM_GETTEXT:
        {
            PWSTR buf = NULL;
            PSTR outbuf = (PSTR)lParam;
            UINT copy;

            Wnd = ValidateHwnd(hWnd);
            if (Wnd != NULL && wParam != 0)
            {
                if (Wnd->strName.Buffer != NULL)
                    buf = DesktopPtrToUser(Wnd->strName.Buffer);
                else
                    outbuf[0] = L'\0';

                if (buf != NULL)
                {
                    if (Wnd->strName.Length != 0)
                    {
                        copy = min(Wnd->strName.Length / sizeof(WCHAR), wParam - 1);
                        Result = WideCharToMultiByte(CP_ACP,
                                                     0,
                                                     buf,
                                                     copy,
                                                     outbuf,
                                                     wParam,
                                                     NULL,
                                                     NULL);
                        outbuf[Result] = '\0';
                    }
                    else
                        outbuf[0] = '\0';
                }
            }
            break;
        }

        case WM_SETTEXT:
        {
            DefSetText(hWnd, (PCWSTR)lParam, TRUE);

            if ((GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION)
            {
                DefWndNCPaint(hWnd, HRGN_WINDOW, -1);
            }
            Result = 1;
            break;
        }

        case WM_IME_KEYDOWN:
        {
            Result = PostMessageA(hWnd, WM_KEYDOWN, wParam, lParam);
            break;
        }

        case WM_IME_KEYUP:
        {
            Result = PostMessageA(hWnd, WM_KEYUP, wParam, lParam);
            break;
        }

        case WM_IME_CHAR:
        {
            if (HIBYTE(wParam))
                PostMessageA(hWnd, WM_CHAR, HIBYTE(wParam), lParam);
            PostMessageA(hWnd, WM_CHAR, LOBYTE(wParam), lParam);
            break;
        }

        case WM_IME_STARTCOMPOSITION:
        case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_SELECT:
        case WM_IME_NOTIFY:
        {
            HWND hwndIME;

            hwndIME = DefWndImmGetDefaultIMEWnd(hWnd);
            if (hwndIME)
                Result = SendMessageA(hwndIME, Msg, wParam, lParam);
            break;
        }

        case WM_IME_SETCONTEXT:
        {
            HWND hwndIME;

            hwndIME = DefWndImmGetDefaultIMEWnd(hWnd);
            if (hwndIME)
                Result = DefWndImmIsUIMessageA(hwndIME, Msg, wParam, lParam);
            break;
        }

        /* fall through */
        default:
            Result = User32DefWindowProc(hWnd, Msg, wParam, lParam, FALSE);
    }

    SPY_ExitMessage(SPY_RESULT_DEFWND, hWnd, Msg, Result, wParam, lParam);
    return Result;
}


LRESULT WINAPI
RealDefWindowProcW(HWND hWnd,
                   UINT Msg,
                   WPARAM wParam,
                   LPARAM lParam)
{
    LRESULT Result = 0;
    PWND Wnd;

    SPY_EnterMessage(SPY_DEFWNDPROC, hWnd, Msg, wParam, lParam);
    switch (Msg)
    {
        case WM_NCCREATE:
        {
            if (lParam)
            {
                LPCREATESTRUCTW cs = (LPCREATESTRUCTW)lParam;
                /* check for string, as static icons, bitmaps (SS_ICON, SS_BITMAP)
                 * may have child window IDs instead of window name */
                if (HIWORD(cs->lpszName))
                {
                    DefSetText(hWnd, cs->lpszName, FALSE);
                }
                Result = 1;
            }
            break;
        }

        case WM_GETTEXTLENGTH:
        {
            PWSTR buf;
            ULONG len;

            Wnd = ValidateHwnd(hWnd);
            if (Wnd != NULL && Wnd->strName.Length != 0)
            {
                buf = DesktopPtrToUser(Wnd->strName.Buffer);
                if (buf != NULL &&
                    NT_SUCCESS(RtlUnicodeToMultiByteSize(&len,
                                                         buf,
                                                         Wnd->strName.Length)))
                {
                    Result = (LRESULT) (Wnd->strName.Length / sizeof(WCHAR));
                }
            }
            else Result = 0L;

            break;
        }

        case WM_GETTEXT:
        {
            PWSTR buf = NULL;
            PWSTR outbuf = (PWSTR)lParam;

            Wnd = ValidateHwnd(hWnd);
            if (Wnd != NULL && wParam != 0)
            {
                if (Wnd->strName.Buffer != NULL)
                    buf = DesktopPtrToUser(Wnd->strName.Buffer);
                else
                    outbuf[0] = L'\0';

                if (buf != NULL)
                {
                    if (Wnd->strName.Length != 0)
                    {
                        Result = min(Wnd->strName.Length / sizeof(WCHAR), wParam - 1);
                        RtlCopyMemory(outbuf,
                                      buf,
                                      Result * sizeof(WCHAR));
                        outbuf[Result] = L'\0';
                    }
                    else
                        outbuf[0] = L'\0';
                }
            }
            break;
        }

        case WM_SETTEXT:
        {
            DefSetText(hWnd, (PCWSTR)lParam, FALSE);

            if ((GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_CAPTION) == WS_CAPTION)
            {
                DefWndNCPaint(hWnd, HRGN_WINDOW, -1);
            }
            Result = 1;
            break;
        }

        case WM_IME_CHAR:
        {
            PostMessageW(hWnd, WM_CHAR, wParam, lParam);
            Result = 0;
            break;
        }

        case WM_IME_KEYDOWN:
        {
            Result = PostMessageW(hWnd, WM_KEYDOWN, wParam, lParam);
            break;
        }

        case WM_IME_KEYUP:
        {
            Result = PostMessageW(hWnd, WM_KEYUP, wParam, lParam);
            break;
        }

        case WM_IME_STARTCOMPOSITION:
        case WM_IME_COMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_SELECT:
        case WM_IME_NOTIFY:
        {
            HWND hwndIME;

            hwndIME = DefWndImmGetDefaultIMEWnd(hWnd);
            if (hwndIME)
                Result = SendMessageW(hwndIME, Msg, wParam, lParam);
            break;
        }

        case WM_IME_SETCONTEXT:
        {
            HWND hwndIME;

            hwndIME = DefWndImmGetDefaultIMEWnd(hWnd);
            if (hwndIME)
                Result = DefWndImmIsUIMessageW(hwndIME, Msg, wParam, lParam);
            break;
        }

        default:
            Result = User32DefWindowProc(hWnd, Msg, wParam, lParam, TRUE);
    }
    SPY_ExitMessage(SPY_RESULT_DEFWND, hWnd, Msg, Result, wParam, lParam);

    return Result;
}

LRESULT WINAPI
DefWindowProcA(HWND hWnd,
	       UINT Msg,
	       WPARAM wParam,
	       LPARAM lParam)
{
   BOOL Hook, msgOverride = FALSE;
   LRESULT Result = 0;

   LoadUserApiHook();

   Hook = BeginIfHookedUserApiHook();
   if (Hook)
      msgOverride = IsMsgOverride(Msg, &guah.DefWndProcArray);

   /* Bypass SEH and go direct. */
   if (!Hook || !msgOverride)
      return RealDefWindowProcA(hWnd, Msg, wParam, lParam);

   _SEH2_TRY
   {
      Result = guah.DefWindowProcA(hWnd, Msg, wParam, lParam);
   }
   _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
   {
   }
   _SEH2_END;

   EndUserApiHook();

   return Result;
}

LRESULT WINAPI
DefWindowProcW(HWND hWnd,
	       UINT Msg,
	       WPARAM wParam,
	       LPARAM lParam)
{
   BOOL Hook, msgOverride = FALSE;
   LRESULT Result = 0;

   LoadUserApiHook();

   Hook = BeginIfHookedUserApiHook();
   if (Hook)
      msgOverride = IsMsgOverride(Msg, &guah.DefWndProcArray);

   /* Bypass SEH and go direct. */
   if (!Hook || !msgOverride)
      return RealDefWindowProcW(hWnd, Msg, wParam, lParam);

   _SEH2_TRY
   {
      Result = guah.DefWindowProcW(hWnd, Msg, wParam, lParam);
   }
   _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
   {
   }
   _SEH2_END;

   EndUserApiHook();

   return Result;
}
