/*
 * Odyssey User32 Library
 * - Window non-client area management
 *
 * Copyright (C) 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/* INCLUDES *******************************************************************/

#include <user32.h>

#include <wine/debug.h>

#define HAS_DLGFRAME(Style, ExStyle) \
            (((ExStyle) & WS_EX_DLGMODALFRAME) || \
            (((Style) & WS_DLGFRAME) && (!((Style) & (WS_THICKFRAME | WS_MINIMIZE)))))

#define HAS_THICKFRAME(Style, ExStyle) \
            (((Style) & WS_THICKFRAME) && !((Style) & WS_MINIMIZE) && \
            (!(((Style) & (WS_DLGFRAME | WS_BORDER)) == WS_DLGFRAME)))

#define HAS_THINFRAME(Style, ExStyle) \
            (((Style) & (WS_BORDER | WS_MINIMIZE)) || (!((Style) & (WS_CHILD | WS_POPUP))))

#define HASSIZEGRIP(Style, ExStyle, ParentStyle, WindowRect, ParentClientRect) \
            ((!(Style & WS_CHILD) && (Style & WS_THICKFRAME) && !(Style & WS_MAXIMIZE))  || \
             ((Style & WS_CHILD) && (ParentStyle & WS_THICKFRAME) && !(ParentStyle & WS_MAXIMIZE) && \
             (WindowRect.right - WindowRect.left == ParentClientRect.right) && \
             (WindowRect.bottom - WindowRect.top == ParentClientRect.bottom)))

#ifndef STATE_SYSTEM_OFFSCREEN
#define STATE_SYSTEM_OFFSCREEN	0x00010000
#endif

/*
 * FIXME: This should be moved to a header
 */
VOID
IntDrawScrollBar(HWND hWnd, HDC hDC, INT nBar);
DWORD
IntScrollHitTest(HWND hWnd, INT nBar, POINT pt, BOOL bDragging);

BOOL WINAPI GdiGradientFill(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG);

extern ATOM AtomInternalPos;

/* PRIVATE FUNCTIONS **********************************************************/

BOOL
IntIsScrollBarVisible(HWND hWnd, INT hBar)
{
  SCROLLBARINFO sbi;
  sbi.cbSize = sizeof(SCROLLBARINFO);
  if(!NtUserGetScrollBarInfo(hWnd, hBar, &sbi))
    return FALSE;

  return !(sbi.rgstate[0] & STATE_SYSTEM_OFFSCREEN);
}

BOOL
UserHasWindowEdge(DWORD Style, DWORD ExStyle)
{
   if (Style & WS_MINIMIZE)
      return TRUE;
   if (ExStyle & WS_EX_DLGMODALFRAME)
      return TRUE;
   if (ExStyle & WS_EX_STATICEDGE)
      return FALSE;
   if (Style & WS_THICKFRAME)
      return TRUE;
   Style &= WS_CAPTION;
   if (Style == WS_DLGFRAME || Style == WS_CAPTION)
      return TRUE;
   return FALSE;
}

VOID
UserGetWindowBorders(DWORD Style, DWORD ExStyle, SIZE *Size, BOOL WithClient)
{
   DWORD Border = 0;

   if (UserHasWindowEdge(Style, ExStyle))
      Border += 2;
   else if (ExStyle & WS_EX_STATICEDGE)
      Border += 1;
   if ((ExStyle & WS_EX_CLIENTEDGE) && WithClient)
      Border += 2;
   if (Style & WS_CAPTION || ExStyle & WS_EX_DLGMODALFRAME)
      Border ++;
   Size->cx = Size->cy = Border;
   if ((Style & WS_THICKFRAME) && !(Style & WS_MINIMIZE))
   {
      Size->cx += GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXDLGFRAME);
      Size->cy += GetSystemMetrics(SM_CYFRAME) - GetSystemMetrics(SM_CYDLGFRAME);
   }
   Size->cx *= GetSystemMetrics(SM_CXBORDER);
   Size->cy *= GetSystemMetrics(SM_CYBORDER);
}

BOOL
UserHasMenu(HWND hWnd, ULONG Style)
{
   return (!(Style & WS_CHILD) && GetMenu(hWnd) != 0);
}

HICON
UserGetWindowIcon(HWND hwnd)
{
   HICON hIcon = 0;

   SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL2, 0, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);

   if (!hIcon)
      SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);

   if (!hIcon)
      SendMessageTimeout(hwnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG, 1000, (PDWORD_PTR)&hIcon);

   if (!hIcon)
      hIcon = (HICON)GetClassLongPtr(hwnd, GCL_HICONSM);

   if (!hIcon)
      hIcon = (HICON)GetClassLongPtr(hwnd, GCL_HICON);

   return hIcon;
}

BOOL
UserDrawSysMenuButton(HWND hWnd, HDC hDC, LPRECT Rect, BOOL Down)
{
   HICON WindowIcon;

   if ((WindowIcon = UserGetWindowIcon(hWnd)))
   {
      return DrawIconEx(hDC, Rect->left + 2, Rect->top + 2, WindowIcon,
                        GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                        0, NULL, DI_NORMAL);
   }

   return FALSE;
}

/*
 * FIXME:
 * - Cache bitmaps, then just bitblt instead of calling DFC() (and
 *   wasting precious CPU cycles) every time
 * - Center the buttons verticaly in the rect
 */
VOID
UserDrawCaptionButton(HWND hWnd, LPRECT Rect, DWORD Style, DWORD ExStyle, HDC hDC, BOOL bDown, ULONG Type)
{
   RECT TempRect;

   if (!(Style & WS_SYSMENU))
   {
      return;
   }

   TempRect = *Rect;

   switch (Type)
   {
      case DFCS_CAPTIONMIN:
      {
         if (ExStyle & WS_EX_TOOLWINDOW)
            return; /* ToolWindows don't have min/max buttons */

         if (Style & WS_SYSMENU)
             TempRect.right -= GetSystemMetrics(SM_CXSIZE) + 1;
         if (Style & (WS_MAXIMIZEBOX | WS_MINIMIZEBOX))
             TempRect.right -= GetSystemMetrics(SM_CXSIZE) - 2;
         TempRect.left = TempRect.right - GetSystemMetrics(SM_CXSIZE) + 1;
         TempRect.bottom = TempRect.top + GetSystemMetrics(SM_CYSIZE) - 2;
         TempRect.top += 2;
         TempRect.right -= 1;

         DrawFrameControl(hDC, &TempRect, DFC_CAPTION,
                          ((Style & WS_MINIMIZE) ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMIN) |
                          (bDown ? DFCS_PUSHED : 0) |
                          ((Style & WS_MINIMIZEBOX) ? 0 : DFCS_INACTIVE));
         break;
      }
      case DFCS_CAPTIONMAX:
      {
         if (ExStyle & WS_EX_TOOLWINDOW)
             return; /* ToolWindows don't have min/max buttons */

         if (Style & WS_SYSMENU)
             TempRect.right -= GetSystemMetrics(SM_CXSIZE) + 1;
         TempRect.left = TempRect.right - GetSystemMetrics(SM_CXSIZE) + 1;
         TempRect.bottom = TempRect.top + GetSystemMetrics(SM_CYSIZE) - 2;
         TempRect.top += 2;
         TempRect.right -= 1;

         DrawFrameControl(hDC, &TempRect, DFC_CAPTION,
                          ((Style & WS_MAXIMIZE) ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMAX) |
                          (bDown ? DFCS_PUSHED : 0) |
                          ((Style & WS_MAXIMIZEBOX) ? 0 : DFCS_INACTIVE));
         break;
      }
      case DFCS_CAPTIONCLOSE:
      {
          HMENU hSysMenu = GetSystemMenu(hWnd, FALSE);
          UINT MenuState = GetMenuState(hSysMenu, SC_CLOSE, MF_BYCOMMAND); /* in case of error MenuState==0xFFFFFFFF */
          
         /* FIXME: A tool window has a smaller Close button */

         if (ExStyle & WS_EX_TOOLWINDOW)
         {
            TempRect.left = TempRect.right - GetSystemMetrics(SM_CXSMSIZE);
            TempRect.bottom = TempRect.top + GetSystemMetrics(SM_CYSMSIZE) - 2;
         }
         else
         {
            TempRect.left = TempRect.right - GetSystemMetrics(SM_CXSIZE);
            TempRect.bottom = TempRect.top + GetSystemMetrics(SM_CYSIZE) - 2;
         }
         TempRect.top += 2;
         TempRect.right -= 2;

         DrawFrameControl(hDC, &TempRect, DFC_CAPTION,
                          (DFCS_CAPTIONCLOSE | (bDown ? DFCS_PUSHED : 0) |
                          ((!(MenuState & (MF_GRAYED|MF_DISABLED)) && !(GetClassLong(hWnd, GCL_STYLE) & CS_NOCLOSE)) ? 0 : DFCS_INACTIVE)));
         break;
      }
   }
}

VOID
UserDrawCaptionButtonWnd(HWND hWnd, HDC hDC, BOOL bDown, ULONG Type)
{
   RECT WindowRect;
   SIZE WindowBorder;
   DWORD Style, ExStyle;

   GetWindowRect(hWnd, &WindowRect);
   WindowRect.right -= WindowRect.left;
   WindowRect.bottom -= WindowRect.top;
   WindowRect.left = WindowRect.top = 0;
   Style = GetWindowLongPtrW(hWnd, GWL_STYLE);
   ExStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
   UserGetWindowBorders(Style, ExStyle, &WindowBorder, FALSE);
   InflateRect(&WindowRect, -WindowBorder.cx, -WindowBorder.cy);
   UserDrawCaptionButton(hWnd, &WindowRect, Style, ExStyle, hDC, bDown, Type);
}

// Note from Wine:
/* MSDN docs are pretty idiotic here, they say app CAN use clipRgn in
   the call to GetDCEx implying that it is allowed not to use it either.
   However, the suggested GetDCEx(    , DCX_WINDOW | DCX_INTERSECTRGN)
   will cause clipRgn to be deleted after ReleaseDC().
   Now, how is the "system" supposed to tell what happened?
 */
/*
 * FIXME:
 * - Drawing of WS_BORDER after scrollbars
 * - Correct drawing of size-box
 */
LRESULT
DefWndNCPaint(HWND hWnd, HRGN hRgn, BOOL Active)
{
   HDC hDC;
   DWORD Style, ExStyle;
   HWND Parent;
   RECT ClientRect, WindowRect, CurrentRect, TempRect;

   if (!IsWindowVisible(hWnd))
      return 0;

   Style = GetWindowLongPtrW(hWnd, GWL_STYLE);

   hDC = GetDCEx(hWnd, hRgn, DCX_WINDOW | DCX_INTERSECTRGN | DCX_USESTYLE | DCX_KEEPCLIPRGN);
   if (hDC == 0)
   {
      return 0;
   }

   Parent = GetParent(hWnd);
   ExStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);
   if (Active == -1)
   {
      if (ExStyle & WS_EX_MDICHILD)
      {
         Active = IsChild(GetForegroundWindow(), hWnd);
         if (Active)
            Active = (hWnd == (HWND)SendMessageW(Parent, WM_MDIGETACTIVE, 0, 0));
      }
      else
      {
         Active = (GetForegroundWindow() == hWnd);
      }
   }
   GetWindowRect(hWnd, &WindowRect);
   GetClientRect(hWnd, &ClientRect);

   CurrentRect.top = CurrentRect.left = 0;
   CurrentRect.right = WindowRect.right - WindowRect.left;
   CurrentRect.bottom = WindowRect.bottom - WindowRect.top;

   /* Draw outer edge */
   if (UserHasWindowEdge(Style, ExStyle))
   {
      DrawEdge(hDC, &CurrentRect, EDGE_RAISED, BF_RECT | BF_ADJUST);
   } else
   if (ExStyle & WS_EX_STATICEDGE)
   {
#if 0
      DrawEdge(hDC, &CurrentRect, BDR_SUNKENINNER, BF_RECT | BF_ADJUST | BF_FLAT);
#else
      SelectObject(hDC, GetSysColorBrush(COLOR_BTNSHADOW));
      PatBlt(hDC, CurrentRect.left, CurrentRect.top, CurrentRect.right - CurrentRect.left, 1, PATCOPY);
      PatBlt(hDC, CurrentRect.left, CurrentRect.top, 1, CurrentRect.bottom - CurrentRect.top, PATCOPY);

      SelectObject(hDC, GetSysColorBrush(COLOR_BTNHIGHLIGHT));
      PatBlt(hDC, CurrentRect.left, CurrentRect.bottom - 1, CurrentRect.right - CurrentRect.left, 1, PATCOPY);
      PatBlt(hDC, CurrentRect.right - 1, CurrentRect.top, 1, CurrentRect.bottom - CurrentRect.top, PATCOPY);

      InflateRect(&CurrentRect, -1, -1);
#endif
   }

   /* Firstly the "thick" frame */
   if ((Style & WS_THICKFRAME) && !(Style & WS_MINIMIZE))
   {
      DWORD Width =
         (GetSystemMetrics(SM_CXFRAME) - GetSystemMetrics(SM_CXDLGFRAME)) *
         GetSystemMetrics(SM_CXBORDER);
      DWORD Height =
         (GetSystemMetrics(SM_CYFRAME) - GetSystemMetrics(SM_CYDLGFRAME)) *
         GetSystemMetrics(SM_CYBORDER);

      SelectObject(hDC, GetSysColorBrush(Active ? COLOR_ACTIVEBORDER :
         COLOR_INACTIVEBORDER));

      /* Draw frame */
      PatBlt(hDC, CurrentRect.left, CurrentRect.top, CurrentRect.right - CurrentRect.left, Height, PATCOPY);
      PatBlt(hDC, CurrentRect.left, CurrentRect.top, Width, CurrentRect.bottom - CurrentRect.top, PATCOPY);
#ifdef __ODYSSEY__
      PatBlt(hDC, CurrentRect.left, CurrentRect.bottom - 1, CurrentRect.right - CurrentRect.left, -Height, PATCOPY);
      PatBlt(hDC, CurrentRect.right - 1, CurrentRect.top, -Width, CurrentRect.bottom - CurrentRect.top, PATCOPY);
#else
      PatBlt(hDC, CurrentRect.left, CurrentRect.bottom, CurrentRect.right - CurrentRect.left, -Height, PATCOPY);
      PatBlt(hDC, CurrentRect.right, CurrentRect.top, -Width, CurrentRect.bottom - CurrentRect.top, PATCOPY);
#endif

      InflateRect(&CurrentRect, -Width, -Height);
   }

   /* Now the other bit of the frame */
   if (Style & (WS_DLGFRAME | WS_BORDER) || ExStyle & WS_EX_DLGMODALFRAME)
   {
      DWORD Width = GetSystemMetrics(SM_CXBORDER);
      DWORD Height = GetSystemMetrics(SM_CYBORDER);

      SelectObject(hDC, GetSysColorBrush(
         (ExStyle & (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE)) ? COLOR_3DFACE :
         (ExStyle & WS_EX_STATICEDGE) ? COLOR_WINDOWFRAME :
         (Style & (WS_DLGFRAME | WS_THICKFRAME)) ? COLOR_3DFACE :
         COLOR_WINDOWFRAME));

      /* Draw frame */
      PatBlt(hDC, CurrentRect.left, CurrentRect.top, CurrentRect.right - CurrentRect.left, Height, PATCOPY);
      PatBlt(hDC, CurrentRect.left, CurrentRect.top, Width, CurrentRect.bottom - CurrentRect.top, PATCOPY);
#ifdef __ODYSSEY__
      PatBlt(hDC, CurrentRect.left, CurrentRect.bottom - 1, CurrentRect.right - CurrentRect.left, -Height, PATCOPY);
      PatBlt(hDC, CurrentRect.right - 1, CurrentRect.top, -Width, CurrentRect.bottom - CurrentRect.top, PATCOPY);
#else
      PatBlt(hDC, CurrentRect.left, CurrentRect.bottom, CurrentRect.right - CurrentRect.left, -Height, PATCOPY);
      PatBlt(hDC, CurrentRect.right, CurrentRect.top, -Width, CurrentRect.bottom - CurrentRect.top, PATCOPY);
#endif

      InflateRect(&CurrentRect, -Width, -Height);
   }

   /* Draw caption */
   if ((Style & WS_CAPTION) == WS_CAPTION)
   {
      DWORD CaptionFlags = DC_ICON | DC_TEXT | DC_BUTTONS;
      HPEN PreviousPen;
      BOOL Gradient = FALSE;

      if(SystemParametersInfoW(SPI_GETGRADIENTCAPTIONS, 0, &Gradient, 0) && Gradient)
      {
        CaptionFlags |= DC_GRADIENT;
      }

      TempRect = CurrentRect;

      if (Active)
      {
         CaptionFlags |= DC_ACTIVE;
      }

      if (ExStyle & WS_EX_TOOLWINDOW)
      {
         CaptionFlags |= DC_SMALLCAP;
         TempRect.bottom = TempRect.top + GetSystemMetrics(SM_CYSMCAPTION) - 1;
         CurrentRect.top += GetSystemMetrics(SM_CYSMCAPTION);
      }
      else
      {
         TempRect.bottom = TempRect.top + GetSystemMetrics(SM_CYCAPTION) - 1;
         CurrentRect.top += GetSystemMetrics(SM_CYCAPTION);
      }

      NtUserDrawCaption(hWnd, hDC, &TempRect, CaptionFlags);

      /* Draw buttons */
      if (Style & WS_SYSMENU)
      {
         UserDrawCaptionButton(hWnd, &TempRect, Style, ExStyle, hDC, FALSE, DFCS_CAPTIONCLOSE);
         if ((Style & (WS_MAXIMIZEBOX | WS_MINIMIZEBOX)) && !(ExStyle & WS_EX_TOOLWINDOW))
         {
            UserDrawCaptionButton(hWnd, &TempRect, Style, ExStyle, hDC, FALSE, DFCS_CAPTIONMIN);
            UserDrawCaptionButton(hWnd, &TempRect, Style, ExStyle, hDC, FALSE, DFCS_CAPTIONMAX);
         }
      }
      if(!(Style & WS_MINIMIZE))
      {
        /* Line under caption */
        PreviousPen = SelectObject(hDC, GetStockObject(DC_PEN));
        SetDCPenColor(hDC, GetSysColor(
           ((ExStyle & (WS_EX_STATICEDGE | WS_EX_CLIENTEDGE |
                        WS_EX_DLGMODALFRAME)) == WS_EX_STATICEDGE) ?
           COLOR_WINDOWFRAME : COLOR_3DFACE));
        MoveToEx(hDC, TempRect.left, TempRect.bottom, NULL);
        LineTo(hDC, TempRect.right, TempRect.bottom);
        SelectObject(hDC, PreviousPen);
      }
   }

   if(!(Style & WS_MINIMIZE))
   {
     HMENU menu = GetMenu(hWnd);
     /* Draw menu bar */
     if (menu && !(Style & WS_CHILD))
     {
        TempRect = CurrentRect;
        TempRect.bottom = TempRect.top + (UINT)NtUserxSetMenuBarHeight(menu, 0);
        CurrentRect.top += MenuDrawMenuBar(hDC, &TempRect, hWnd, FALSE);
     }

     if (ExStyle & WS_EX_CLIENTEDGE)
     {
        DrawEdge(hDC, &CurrentRect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
     }

     /* Draw the scrollbars */
     if ((Style & WS_VSCROLL) && (Style & WS_HSCROLL) &&
         IntIsScrollBarVisible(hWnd, OBJID_VSCROLL) && IntIsScrollBarVisible(hWnd, OBJID_HSCROLL))
     {
        RECT ParentClientRect;

        TempRect = CurrentRect;
        if (ExStyle & WS_EX_LEFTSCROLLBAR)
           TempRect.right = TempRect.left + GetSystemMetrics(SM_CXVSCROLL);
        else
           TempRect.left = TempRect.right - GetSystemMetrics(SM_CXVSCROLL);
        TempRect.top = TempRect.bottom - GetSystemMetrics(SM_CYHSCROLL);
        FillRect(hDC, &TempRect, GetSysColorBrush(COLOR_BTNFACE));
        /* FIXME: Correct drawing of size-box with WS_EX_LEFTSCROLLBAR */
        if(Parent)
          GetClientRect(Parent, &ParentClientRect);
        if (HASSIZEGRIP(Style, ExStyle, GetWindowLongPtrW(Parent, GWL_STYLE), WindowRect, ParentClientRect))
        {
           DrawFrameControl(hDC, &TempRect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
        }
        IntDrawScrollBar(hWnd, hDC, SB_VERT);
        IntDrawScrollBar(hWnd, hDC, SB_HORZ);
     }
     else
     {
        if (Style & WS_VSCROLL && IntIsScrollBarVisible(hWnd, OBJID_VSCROLL))
           IntDrawScrollBar(hWnd, hDC, SB_VERT);
        else if (Style & WS_HSCROLL && IntIsScrollBarVisible(hWnd, OBJID_HSCROLL))
           IntDrawScrollBar(hWnd, hDC, SB_HORZ);
     }
   }

   ReleaseDC(hWnd, hDC);
   if (hRgn != HRGN_WINDOW)
      DeleteObject(hRgn); // We use DCX_KEEPCLIPRGN

   return 0;
}

LRESULT
DefWndNCCalcSize(HWND hWnd, BOOL CalcSizeStruct, RECT *Rect)
{
   LRESULT Result = 0;
   DWORD Style = GetClassLongPtrW(hWnd, GCL_STYLE);
   DWORD ExStyle;
   SIZE WindowBorders;
   RECT OrigRect;

   if (Rect == NULL)
   {
      return Result;
   }
   OrigRect = *Rect;

   if (CalcSizeStruct)
   {
      if (Style & CS_VREDRAW)
      {
         Result |= WVR_VREDRAW;
      }
      if (Style & CS_HREDRAW)
      {
         Result |= WVR_HREDRAW;
      }
      Result |= WVR_VALIDRECTS;
   }

   Style = GetWindowLongPtrW(hWnd, GWL_STYLE);
   ExStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);

   if (!(Style & WS_MINIMIZE))
   {
      HMENU menu = GetMenu(hWnd);

      if (UserHasWindowEdge(Style, ExStyle))
      {
         UserGetWindowBorders(Style, ExStyle, &WindowBorders, FALSE);
         InflateRect(Rect, -WindowBorders.cx, -WindowBorders.cy);
      } else
      if ((ExStyle & WS_EX_STATICEDGE) || (Style & WS_BORDER))
      {
         InflateRect(Rect, -1, -1);
      }

      if ((Style & WS_CAPTION) == WS_CAPTION)
      {
         if (ExStyle & WS_EX_TOOLWINDOW)
            Rect->top += GetSystemMetrics(SM_CYSMCAPTION);
         else
            Rect->top += GetSystemMetrics(SM_CYCAPTION);
      }

      if (menu && !(Style & WS_CHILD))
      {
         HDC hDC = GetWindowDC(hWnd);
         if(hDC)
         {
           RECT CliRect = *Rect;
           CliRect.bottom -= OrigRect.top;
           CliRect.right -= OrigRect.left;
           CliRect.left -= OrigRect.left;
           CliRect.top -= OrigRect.top;
           Rect->top += MenuDrawMenuBar(hDC, &CliRect, hWnd, TRUE);
           ReleaseDC(hWnd, hDC);
         }
      }

      if (ExStyle & WS_EX_CLIENTEDGE)
      {
         InflateRect(Rect, -2 * GetSystemMetrics(SM_CXBORDER),
            -2 * GetSystemMetrics(SM_CYBORDER));
      }

      if(Style & (WS_VSCROLL | WS_HSCROLL))
      {
        SCROLLBARINFO sbi;
        SETSCROLLBARINFO ssbi;

        sbi.cbSize = sizeof(SCROLLBARINFO);
        if((Style & WS_VSCROLL) && NtUserGetScrollBarInfo(hWnd, OBJID_VSCROLL, &sbi))
        {
          int i;
          LONG sx = Rect->right;

          sx -= GetSystemMetrics(SM_CXVSCROLL);
          for(i = 0; i <= CCHILDREN_SCROLLBAR; i++)
            ssbi.rgstate[i] = sbi.rgstate[i];
          if(sx <= Rect->left)
            ssbi.rgstate[0] |= STATE_SYSTEM_OFFSCREEN;
          else
            ssbi.rgstate[0] &= ~STATE_SYSTEM_OFFSCREEN;
          NtUserSetScrollBarInfo(hWnd, OBJID_VSCROLL, &ssbi);
          if(ssbi.rgstate[0] & STATE_SYSTEM_OFFSCREEN)
            Style &= ~WS_VSCROLL;
        }
        else
          Style &= ~WS_VSCROLL;

        if((Style & WS_HSCROLL) && NtUserGetScrollBarInfo(hWnd, OBJID_HSCROLL, &sbi))
        {
          int i;
          LONG sy = Rect->bottom;

          sy -= GetSystemMetrics(SM_CYHSCROLL);
          for(i = 0; i <= CCHILDREN_SCROLLBAR; i++)
            ssbi.rgstate[i] = sbi.rgstate[i];
          if(sy <= Rect->top)
            ssbi.rgstate[0] |= STATE_SYSTEM_OFFSCREEN;
          else
            ssbi.rgstate[0] &= ~STATE_SYSTEM_OFFSCREEN;
          NtUserSetScrollBarInfo(hWnd, OBJID_HSCROLL, &ssbi);
          if(ssbi.rgstate[0] & STATE_SYSTEM_OFFSCREEN)
            Style &= ~WS_HSCROLL;
        }
        else
          Style &= ~WS_HSCROLL;
      }

      if ((Style & WS_VSCROLL) && (Style & WS_HSCROLL))
      {
         if ((ExStyle & WS_EX_LEFTSCROLLBAR) != 0)
            Rect->left += GetSystemMetrics(SM_CXVSCROLL);
         else
            Rect->right -= GetSystemMetrics(SM_CXVSCROLL);
         Rect->bottom -= GetSystemMetrics(SM_CYHSCROLL);
      }
      else
      {
         if (Style & WS_VSCROLL)
         {
            if ((ExStyle & WS_EX_LEFTSCROLLBAR) != 0)
               Rect->left += GetSystemMetrics(SM_CXVSCROLL);
            else
               Rect->right -= GetSystemMetrics(SM_CXVSCROLL);
         }
         else if (Style & WS_HSCROLL)
            Rect->bottom -= GetSystemMetrics(SM_CYHSCROLL);
      }
      if (Rect->top > Rect->bottom)
         Rect->bottom = Rect->top;
      if (Rect->left > Rect->right)
         Rect->right = Rect->left;
   }
   else
   {
      Rect->right = Rect->left;
      Rect->bottom = Rect->top;
   }

   return Result;
}

LRESULT
DefWndNCActivate(HWND hWnd, WPARAM wParam)
{
   DefWndNCPaint(hWnd, HRGN_WINDOW, wParam);
   return TRUE;
}

/*
 * FIXME:
 * - Check the scrollbar handling
 */
LRESULT
DefWndNCHitTest(HWND hWnd, POINT Point)
{
   RECT WindowRect, ClientRect, OrigWndRect;
   POINT ClientPoint;
   SIZE WindowBorders;
   DWORD Style = GetWindowLongPtrW(hWnd, GWL_STYLE);
   DWORD ExStyle = GetWindowLongPtrW(hWnd, GWL_EXSTYLE);

   GetWindowRect(hWnd, &WindowRect);
   if (!PtInRect(&WindowRect, Point))
   {
      return HTNOWHERE;
   }
   OrigWndRect = WindowRect;

   if (UserHasWindowEdge(Style, ExStyle))
   {
      LONG XSize, YSize;

      UserGetWindowBorders(Style, ExStyle, &WindowBorders, FALSE);
      InflateRect(&WindowRect, -WindowBorders.cx, -WindowBorders.cy);
      XSize = GetSystemMetrics(SM_CXSIZE) * GetSystemMetrics(SM_CXBORDER);
      YSize = GetSystemMetrics(SM_CYSIZE) * GetSystemMetrics(SM_CYBORDER);
      if (!PtInRect(&WindowRect, Point))
      {
         BOOL ThickFrame;

         ThickFrame = (Style & WS_THICKFRAME);
         if (Point.y < WindowRect.top)
         {
            if(Style & WS_MINIMIZE)
              return HTCAPTION;
            if(!ThickFrame)
              return HTBORDER;
            if (Point.x < (WindowRect.left + XSize))
               return HTTOPLEFT;
            if (Point.x >= (WindowRect.right - XSize))
               return HTTOPRIGHT;
            return HTTOP;
         }
         if (Point.y >= WindowRect.bottom)
         {
            if(Style & WS_MINIMIZE)
              return HTCAPTION;
            if(!ThickFrame)
              return HTBORDER;
            if (Point.x < (WindowRect.left + XSize))
               return HTBOTTOMLEFT;
            if (Point.x >= (WindowRect.right - XSize))
               return HTBOTTOMRIGHT;
            return HTBOTTOM;
         }
         if (Point.x < WindowRect.left)
         {
            if(Style & WS_MINIMIZE)
              return HTCAPTION;
            if(!ThickFrame)
              return HTBORDER;
            if (Point.y < (WindowRect.top + YSize))
               return HTTOPLEFT;
            if (Point.y >= (WindowRect.bottom - YSize))
               return HTBOTTOMLEFT;
            return HTLEFT;
         }
         if (Point.x >= WindowRect.right)
         {
            if(Style & WS_MINIMIZE)
              return HTCAPTION;
            if(!ThickFrame)
              return HTBORDER;
            if (Point.y < (WindowRect.top + YSize))
               return HTTOPRIGHT;
            if (Point.y >= (WindowRect.bottom - YSize))
               return HTBOTTOMRIGHT;
            return HTRIGHT;
         }
      }
   }
   else
   {
      if (ExStyle & WS_EX_STATICEDGE)
         InflateRect(&WindowRect,
            -GetSystemMetrics(SM_CXBORDER),
            -GetSystemMetrics(SM_CYBORDER));
      if (!PtInRect(&WindowRect, Point))
         return HTBORDER;
   }

   if ((Style & WS_CAPTION) == WS_CAPTION)
   {
      if (ExStyle & WS_EX_TOOLWINDOW)
         WindowRect.top += GetSystemMetrics(SM_CYSMCAPTION);
      else
         WindowRect.top += GetSystemMetrics(SM_CYCAPTION);
      if (!PtInRect(&WindowRect, Point))
      {
         if (Style & WS_SYSMENU)
         {
            if (ExStyle & WS_EX_TOOLWINDOW)
            {
               WindowRect.right -= GetSystemMetrics(SM_CXSMSIZE);
            }
            else
            {
               if(!(ExStyle & WS_EX_DLGMODALFRAME))
                  WindowRect.left += GetSystemMetrics(SM_CXSIZE);
               WindowRect.right -= GetSystemMetrics(SM_CXSIZE);
            }
         }
         if (Point.x < WindowRect.left)
            return HTSYSMENU;
         if (WindowRect.right <= Point.x)
            return HTCLOSE;
         if (Style & WS_MAXIMIZEBOX || Style & WS_MINIMIZEBOX)
            WindowRect.right -= GetSystemMetrics(SM_CXSIZE);
         if (Point.x >= WindowRect.right)
            return HTMAXBUTTON;
         if (Style & WS_MINIMIZEBOX)
            WindowRect.right -= GetSystemMetrics(SM_CXSIZE);
         if (Point.x >= WindowRect.right)
            return HTMINBUTTON;
         return HTCAPTION;
      }
   }

   if(!(Style & WS_MINIMIZE))
   {
     ClientPoint = Point;
     ScreenToClient(hWnd, &ClientPoint);
     GetClientRect(hWnd, &ClientRect);

     if (PtInRect(&ClientRect, ClientPoint))
     {
        return HTCLIENT;
     }

     if (GetMenu(hWnd) && !(Style & WS_CHILD))
     {
        if (Point.x > 0 && Point.x < WindowRect.right && ClientPoint.y < 0)
           return HTMENU;
     }

     if (ExStyle & WS_EX_CLIENTEDGE)
     {
        InflateRect(&WindowRect, -2 * GetSystemMetrics(SM_CXBORDER),
           -2 * GetSystemMetrics(SM_CYBORDER));
     }

     if ((Style & WS_VSCROLL) && (Style & WS_HSCROLL) &&
         (WindowRect.bottom - WindowRect.top) > GetSystemMetrics(SM_CYHSCROLL))
     {
        RECT ParentRect, TempRect = WindowRect, TempRect2 = WindowRect;
        HWND Parent = GetParent(hWnd);

        TempRect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
        if ((ExStyle & WS_EX_LEFTSCROLLBAR) != 0)
           TempRect.right = TempRect.left + GetSystemMetrics(SM_CXVSCROLL);
        else
           TempRect.left = TempRect.right - GetSystemMetrics(SM_CXVSCROLL);
        if (PtInRect(&TempRect, Point))
           return HTVSCROLL;

        TempRect2.top = TempRect2.bottom - GetSystemMetrics(SM_CYHSCROLL);
        if ((ExStyle & WS_EX_LEFTSCROLLBAR) != 0)
           TempRect2.left += GetSystemMetrics(SM_CXVSCROLL);
        else
           TempRect2.right -= GetSystemMetrics(SM_CXVSCROLL);
        if (PtInRect(&TempRect2, Point))
           return HTHSCROLL;

        TempRect.top = TempRect2.top;
        TempRect.bottom = TempRect2.bottom;
        if(Parent)
          GetClientRect(Parent, &ParentRect);
        if (PtInRect(&TempRect, Point) && HASSIZEGRIP(Style, ExStyle,
            GetWindowLongPtrW(Parent, GWL_STYLE), OrigWndRect, ParentRect))
        {
           if ((ExStyle & WS_EX_LEFTSCROLLBAR) != 0)
              return HTBOTTOMLEFT;
           else
              return HTBOTTOMRIGHT;
        }
     }
     else
     {
        if (Style & WS_VSCROLL)
        {
           RECT TempRect = WindowRect;

           if ((ExStyle & WS_EX_LEFTSCROLLBAR) != 0)
              TempRect.right = TempRect.left + GetSystemMetrics(SM_CXVSCROLL);
           else
              TempRect.left = TempRect.right - GetSystemMetrics(SM_CXVSCROLL);
           if (PtInRect(&TempRect, Point))
              return HTVSCROLL;
        } else
        if (Style & WS_HSCROLL)
        {
           RECT TempRect = WindowRect;
           TempRect.top = TempRect.bottom - GetSystemMetrics(SM_CYHSCROLL);
           if (PtInRect(&TempRect, Point))
              return HTHSCROLL;
        }
     }
   }

   return HTNOWHERE;
}

VOID
DefWndDoButton(HWND hWnd, WPARAM wParam)
{
   MSG Msg;
   HDC WindowDC;
   BOOL Pressed = TRUE, OldState;
   WPARAM SCMsg;
   HMENU hSysMenu;
   ULONG ButtonType;
   DWORD Style;
   UINT MenuState;

   Style = GetWindowLongPtrW(hWnd, GWL_STYLE);
   switch (wParam)
   {
      case HTCLOSE:
         hSysMenu = GetSystemMenu(hWnd, FALSE);
         MenuState = GetMenuState(hSysMenu, SC_CLOSE, MF_BYCOMMAND); /* in case of error MenuState==0xFFFFFFFF */
         if (!(Style & WS_SYSMENU) || (MenuState & (MF_GRAYED|MF_DISABLED)) || (GetClassLongPtrW(hWnd, GCL_STYLE) & CS_NOCLOSE))
            return;
         ButtonType = DFCS_CAPTIONCLOSE;
         SCMsg = SC_CLOSE;
         break;
      case HTMINBUTTON:
         if (!(Style & WS_MINIMIZEBOX))
            return;
         ButtonType = DFCS_CAPTIONMIN;
         SCMsg = ((Style & WS_MINIMIZE) ? SC_RESTORE : SC_MINIMIZE);
         break;
      case HTMAXBUTTON:
         if (!(Style & WS_MAXIMIZEBOX))
            return;
         ButtonType = DFCS_CAPTIONMAX;
         SCMsg = ((Style & WS_MAXIMIZE) ? SC_RESTORE : SC_MAXIMIZE);
         break;

      default:
         ASSERT(FALSE);
         return;
   }

   /*
    * FIXME: Not sure where to do this, but we must flush the pending
    * window updates when someone clicks on the close button and at
    * the same time the window is overlapped with another one. This
    * looks like a good place for now...
    */
   UpdateWindow(hWnd);

   WindowDC = GetWindowDC(hWnd);
   UserDrawCaptionButtonWnd(hWnd, WindowDC, TRUE, ButtonType);

   SetCapture(hWnd);

   for (;;)
   {
      if (GetMessageW(&Msg, 0, WM_MOUSEFIRST, WM_MOUSELAST) <= 0)
         break;

      if (Msg.message == WM_LBUTTONUP)
         break;

      if (Msg.message != WM_MOUSEMOVE)
         continue;

      OldState = Pressed;
      Pressed = (DefWndNCHitTest(hWnd, Msg.pt) == wParam);
      if (Pressed != OldState)
         UserDrawCaptionButtonWnd(hWnd, WindowDC, Pressed, ButtonType);
   }

   if (Pressed)
      UserDrawCaptionButtonWnd(hWnd, WindowDC, FALSE, ButtonType);
   ReleaseCapture();
   ReleaseDC(hWnd, WindowDC);
   if (Pressed)
      SendMessageW(hWnd, WM_SYSCOMMAND, SCMsg, 0);
}


LRESULT
DefWndNCLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
        case HTCAPTION:
        {
	        HWND hTopWnd = GetAncestor(hWnd, GA_ROOT);
	        if (SetActiveWindow(hTopWnd) || GetActiveWindow() == hTopWnd)
	        {
	            SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE + HTCAPTION, lParam);
	        }
	        break;
        }
        case HTSYSMENU:
        {
	  if (GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_SYSMENU)
            {
	      SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOUSEMENU + HTSYSMENU,
			   lParam);
	    }
	  break;
        }
        case HTMENU:
        {
            SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOUSEMENU + HTMENU, lParam);
            break;
        }
        case HTHSCROLL:
        {
            SendMessageW(hWnd, WM_SYSCOMMAND, SC_HSCROLL + HTHSCROLL, lParam);
            break;
        }
        case HTVSCROLL:
        {
            SendMessageW(hWnd, WM_SYSCOMMAND, SC_VSCROLL + HTVSCROLL, lParam);
            break;
        }
        case HTMINBUTTON:
        case HTMAXBUTTON:
        case HTCLOSE:
        {
          DefWndDoButton(hWnd, wParam);
          break;
        }
        case HTLEFT:
        case HTRIGHT:
        case HTTOP:
        case HTBOTTOM:
        case HTTOPLEFT:
        case HTTOPRIGHT:
        case HTBOTTOMLEFT:
        case HTBOTTOMRIGHT:
        {
            SendMessageW(hWnd, WM_SYSCOMMAND, SC_SIZE + wParam - (HTLEFT - WMSZ_LEFT), lParam);
            break;
        }
    }
    return(0);
}


LRESULT
DefWndNCLButtonDblClk(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
  ULONG Style;

  Style = GetWindowLongPtrW(hWnd, GWL_STYLE);
  switch(wParam)
  {
    case HTCAPTION:
    {
      /* Maximize/Restore the window */
      if((Style & WS_CAPTION) == WS_CAPTION && (Style & WS_MAXIMIZEBOX))
      {
        SendMessageW(hWnd, WM_SYSCOMMAND, ((Style & (WS_MINIMIZE | WS_MAXIMIZE)) ? SC_RESTORE : SC_MAXIMIZE), 0);
      }
      break;
    }
    case HTSYSMENU:
    {
      SendMessageW(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
      break;
    }
    default:
      return DefWndNCLButtonDown(hWnd, wParam, lParam);
  }
  return(0);
}

VOID
DefWndTrackScrollBar(HWND hWnd, WPARAM wParam, POINT Point)
{
   INT ScrollBar;

   if ((wParam & 0xfff0) == SC_HSCROLL)
   {
      if ((wParam & 0x0f) != HTHSCROLL)
         return;
      ScrollBar = SB_HORZ;
   }
   else
   {
      if ((wParam & 0x0f) != HTVSCROLL)
         return;
      ScrollBar = SB_VERT;
   }

   /* FIXME */
}

/* PUBLIC FUNCTIONS ***********************************************************/

BOOL WINAPI
RealAdjustWindowRectEx(LPRECT lpRect,
                       DWORD dwStyle,
                       BOOL bMenu,
                       DWORD dwExStyle)
{
   SIZE BorderSize;

   if (bMenu)
   {
      lpRect->top -= GetSystemMetrics(SM_CYMENU);
   }
   if ((dwStyle & WS_CAPTION) == WS_CAPTION)
   {
      if (dwExStyle & WS_EX_TOOLWINDOW)
         lpRect->top -= GetSystemMetrics(SM_CYSMCAPTION);
      else
         lpRect->top -= GetSystemMetrics(SM_CYCAPTION);
   }
   UserGetWindowBorders(dwStyle, dwExStyle, &BorderSize, TRUE);
   InflateRect(
      lpRect,
      BorderSize.cx,
      BorderSize.cy);

   return TRUE;
}

/*
 * @implemented
 */
BOOL WINAPI
AdjustWindowRectEx(LPRECT lpRect,
		   DWORD dwStyle,
		   BOOL bMenu,
		   DWORD dwExStyle)
{
   BOOL Hook, Ret = FALSE;

   LoadUserApiHook();

   Hook = BeginIfHookedUserApiHook();

     /* Bypass SEH and go direct. */
   if (!Hook) return RealAdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);

   _SEH2_TRY
   {
      Ret = guah.AdjustWindowRectEx(lpRect, dwStyle, bMenu, dwExStyle);
   }
   _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
   {
   }
   _SEH2_END;

   EndUserApiHook();

   return Ret;
}


/*
 * @implemented
 */
BOOL WINAPI
AdjustWindowRect(LPRECT lpRect,
		 DWORD dwStyle,
		 BOOL bMenu)
{
   return AdjustWindowRectEx(lpRect, dwStyle, bMenu, 0);
}

// Enabling this will cause captions to draw smoother, but slower:
#define DOUBLE_BUFFER_CAPTION

/*
 * @implemented
 */
BOOL WINAPI
DrawCaption(HWND hWnd, HDC hDC, LPCRECT lprc, UINT uFlags)
{
   BOOL Hook, Ret = FALSE;

   LoadUserApiHook();

   Hook = BeginIfHookedUserApiHook();

   /* Bypass SEH and go direct. */
   if (!Hook) return NtUserDrawCaption(hWnd, hDC, lprc, uFlags);

   _SEH2_TRY
   {
      Ret = guah.DrawCaption(hWnd, hDC, lprc, uFlags);
   }
   _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
   {
   }
   _SEH2_END;

   EndUserApiHook();

   return Ret;
}

/*
 * @implemented
 */
BOOL
WINAPI
DrawCaptionTempW(
		 HWND        hWnd,
		 HDC         hDC,
		 const RECT *rect,
		 HFONT       hFont,
		 HICON       hIcon,
		 LPCWSTR     str,
		 UINT        uFlags
		 )
{
   UNICODE_STRING Text = {0};
   RtlInitUnicodeString(&Text, str);
   return NtUserDrawCaptionTemp(hWnd, hDC, rect, hFont, hIcon, &Text, uFlags);
}

/*
 * @implemented
 */
BOOL
WINAPI
DrawCaptionTempA(
		 HWND        hwnd,
		 HDC         hdc,
		 const RECT *rect,
		 HFONT       hFont,
		 HICON       hIcon,
		 LPCSTR      str,
		 UINT        uFlags
		 )
{
  LPWSTR strW;
  INT len;
  BOOL ret = FALSE;

  if (!(uFlags & DC_TEXT) || !str)
    return DrawCaptionTempW(hwnd, hdc, rect, hFont, hIcon, NULL, uFlags);

  len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
  if ((strW = HeapAlloc(GetProcessHeap(), 0, len * sizeof(WCHAR))))
  {
    MultiByteToWideChar(CP_ACP, 0, str, -1, strW, len );
    ret = DrawCaptionTempW(hwnd, hdc, rect, hFont, hIcon, strW, uFlags);
    HeapFree(GetProcessHeap(), 0, strW);
  }
  return ret;
}

/***********************************************************************
 *           NcGetInsideRect
 *
 * Get the 'inside' rectangle of a window, i.e. the whole window rectangle
 * but without the borders (if any).
 * The rectangle is in window coordinates (for drawing with GetWindowDC()).
 */
static void FASTCALL
NcGetInsideRect(HWND Wnd, RECT *Rect)
{
  DWORD Style;
  DWORD ExStyle;

  GetWindowRect(Wnd, Rect);
  Rect->right = Rect->right - Rect->left;
  Rect->left = 0;
  Rect->bottom = Rect->bottom - Rect->top;
  Rect->top = 0;

  Style = GetWindowLongPtrW(Wnd, GWL_STYLE);
  if (0 != (Style & WS_ICONIC))
    {
      return;
    }

  /* Remove frame from rectangle */
  ExStyle = GetWindowLongPtrW(Wnd, GWL_EXSTYLE);
  if (HAS_THICKFRAME(Style, ExStyle))
    {
      InflateRect(Rect, - GetSystemMetrics(SM_CXFRAME), - GetSystemMetrics(SM_CYFRAME));
    }
  else if (HAS_DLGFRAME(Style, ExStyle))
    {
      InflateRect(Rect, - GetSystemMetrics(SM_CXDLGFRAME), - GetSystemMetrics(SM_CYDLGFRAME));
    }
  else if (HAS_THINFRAME(Style, ExStyle))
    {
      InflateRect(Rect, - GetSystemMetrics(SM_CXBORDER), - GetSystemMetrics(SM_CYBORDER));
    }

  /* We have additional border information if the window
   * is a child (but not an MDI child) */
  if (0 != (Style & WS_CHILD)
      && 0 == (ExStyle & WS_EX_MDICHILD))
    {
      if (0 != (ExStyle & WS_EX_CLIENTEDGE))
        {
          InflateRect(Rect, - GetSystemMetrics(SM_CXEDGE), - GetSystemMetrics(SM_CYEDGE));
        }
      if (0 != (ExStyle & WS_EX_STATICEDGE))
        {
          InflateRect(Rect, - GetSystemMetrics(SM_CXBORDER), - GetSystemMetrics(SM_CYBORDER));
        }
    }
}

/***********************************************************************
 *           NcGetSysPopupPos
 */
void FASTCALL
NcGetSysPopupPos(HWND Wnd, RECT *Rect)
{
  RECT WindowRect;

  if (IsIconic(Wnd))
    {
      GetWindowRect(Wnd, Rect);
    }
  else
    {
      NcGetInsideRect(Wnd, Rect);
      GetWindowRect(Wnd, &WindowRect);
      OffsetRect(Rect, WindowRect.left, WindowRect.top);
      if (0 != (GetWindowLongPtrW(Wnd, GWL_STYLE) & WS_CHILD))
        {
          ClientToScreen(GetParent(Wnd), (POINT *) Rect);
        }
      Rect->right = Rect->left + GetSystemMetrics(SM_CYCAPTION) - 1;
      Rect->bottom = Rect->top + GetSystemMetrics(SM_CYCAPTION) - 1;
    }
}
