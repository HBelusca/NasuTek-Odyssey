#include <win32k.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define NDEBUG
#include <debug.h>

/*
 * a couple macros to fill a single pixel or a line
 */
#define PUTPIXEL(x,y,BrushInst)        \
  ret = ret && IntEngLineTo(&psurf->SurfObj, \
       dc->rosdc.CombinedClip,                         \
       &BrushInst.BrushObject,                   \
       x, y, (x)+1, y,                           \
       &RectBounds,                              \
       ROP2_TO_MIX(pdcattr->jROP2));

#define PUTLINE(x1,y1,x2,y2,BrushInst) \
  ret = ret && IntEngLineTo(&psurf->SurfObj, \
       dc->rosdc.CombinedClip,                         \
       &BrushInst.BrushObject,                   \
       x1, y1, x2, y2,                           \
       &RectBounds,                              \
       ROP2_TO_MIX(pdcattr->jROP2));

#define Rsin(d) ((d) == 0.0 ? 0.0 : ((d) == 90.0 ? 1.0 : sin(d*M_PI/180.0)))
#define Rcos(d) ((d) == 0.0 ? 1.0 : ((d) == 90.0 ? 0.0 : cos(d*M_PI/180.0)))

BOOL FASTCALL IntFillArc( PDC dc, INT XLeft, INT YLeft, INT Width, INT Height, double StartArc, double EndArc, ARCTYPE arctype);
BOOL FASTCALL IntDrawArc( PDC dc, INT XLeft, INT YLeft, INT Width, INT Height, double StartArc, double EndArc, ARCTYPE arctype, PBRUSH pbrush);

static
BOOL
FASTCALL
IntArc( DC *dc,
        int  Left,
        int  Top,
        int  Right,
        int  Bottom,
        int  XRadialStart,
        int  YRadialStart,
        int  XRadialEnd,
        int  YRadialEnd,
        ARCTYPE arctype)
{
    PDC_ATTR pdcattr;
    RECTL RectBounds, RectSEpts;
    PBRUSH pbrPen;
    SURFACE *psurf;
    BOOL ret = TRUE;
    LONG PenWidth, PenOrigWidth;
    double AngleStart, AngleEnd;
    LONG RadiusX, RadiusY, CenterX, CenterY;
    LONG SfCx, SfCy, EfCx, EfCy;

    if (Right < Left)
    {
       INT tmp = Right; Right = Left; Left = tmp;
    }
    if (Bottom < Top)
    {
       INT tmp = Bottom; Bottom = Top; Top = tmp;
    }
    if ((Left == Right) ||
        (Top == Bottom) ||
        (((arctype != GdiTypeArc) || (arctype != GdiTypeArcTo)) &&
        ((Right - Left == 1) ||
        (Bottom - Top == 1))))
       return TRUE;

    pdcattr = dc->pdcattr;

    pbrPen = PEN_ShareLockPen(pdcattr->hpen);
    if (!pbrPen)
    {
        DPRINT1("Arc Fail 1\n");
        EngSetLastError(ERROR_INTERNAL_ERROR);
        return FALSE;
    }

    PenOrigWidth = PenWidth = pbrPen->ptPenWidth.x;
    if (pbrPen->ulPenStyle == PS_NULL) PenWidth = 0;

    if (pbrPen->ulPenStyle == PS_INSIDEFRAME)
    {
       if (2*PenWidth > (Right - Left)) PenWidth = (Right -Left + 1)/2;
       if (2*PenWidth > (Bottom - Top)) PenWidth = (Bottom -Top + 1)/2;
       Left   += PenWidth / 2;
       Right  -= (PenWidth - 1) / 2;
       Top    += PenWidth / 2;
       Bottom -= (PenWidth - 1) / 2;
    }

    if (!PenWidth) PenWidth = 1;
    pbrPen->ptPenWidth.x = PenWidth;

    RectBounds.left   = Left;
    RectBounds.right  = Right;
    RectBounds.top    = Top;
    RectBounds.bottom = Bottom;

    RectSEpts.left   = XRadialStart;
    RectSEpts.top    = YRadialStart;
    RectSEpts.right  = XRadialEnd;
    RectSEpts.bottom = YRadialEnd;

    IntLPtoDP(dc, (LPPOINT)&RectBounds, 2);
    IntLPtoDP(dc, (LPPOINT)&RectSEpts, 2);

    RectBounds.left   += dc->ptlDCOrig.x;
    RectBounds.right  += dc->ptlDCOrig.x;
    RectBounds.top    += dc->ptlDCOrig.y;
    RectBounds.bottom += dc->ptlDCOrig.y;

    RectSEpts.left    += dc->ptlDCOrig.x;
    RectSEpts.top     += dc->ptlDCOrig.y;
    RectSEpts.right   += dc->ptlDCOrig.x;
    RectSEpts.bottom  += dc->ptlDCOrig.y;

    DPRINT("1: StartX: %d, StartY: %d, EndX: %d, EndY: %d\n",
               RectSEpts.left,RectSEpts.top,RectSEpts.right,RectSEpts.bottom);

    DPRINT("1: Left: %d, Top: %d, Right: %d, Bottom: %d\n",
               RectBounds.left,RectBounds.top,RectBounds.right,RectBounds.bottom);

    RadiusX = max((RectBounds.right - RectBounds.left) / 2, 1);
    RadiusY = max((RectBounds.bottom - RectBounds.top) / 2, 1);
    CenterX = (RectBounds.right + RectBounds.left) / 2;
    CenterY = (RectBounds.bottom + RectBounds.top) / 2;
    AngleEnd   = atan2((RectSEpts.bottom - CenterY), RectSEpts.right - CenterX)*(360.0/(M_PI*2));
    AngleStart = atan2((RectSEpts.top - CenterY), RectSEpts.left - CenterX)*(360.0/(M_PI*2));

    SfCx = (Rcos(AngleStart) * RadiusX);
    SfCy = (Rsin(AngleStart) * RadiusY);
    EfCx = (Rcos(AngleEnd) * RadiusX);
    EfCy = (Rsin(AngleEnd) * RadiusY);

    if ((arctype == GdiTypePie) || (arctype == GdiTypeChord))
    {
        ret = IntFillArc( dc,
              RectBounds.left,
              RectBounds.top,
              abs(RectBounds.right-RectBounds.left), // Width
              abs(RectBounds.bottom-RectBounds.top), // Height
              AngleStart,
              AngleEnd,
              arctype);
    }

    ret = IntDrawArc( dc,
              RectBounds.left,
              RectBounds.top,
              abs(RectBounds.right-RectBounds.left), // Width
              abs(RectBounds.bottom-RectBounds.top), // Height
              AngleStart,
              AngleEnd,
              arctype,
              pbrPen);

    psurf = dc->dclevel.pSurface;
    if (NULL == psurf)
    {
        DPRINT1("Arc Fail 2\n");
        PEN_ShareUnlockPen(pbrPen);
        EngSetLastError(ERROR_INTERNAL_ERROR);
        return FALSE;
    }

    if (arctype == GdiTypePie)
    {
       PUTLINE(CenterX, CenterY, SfCx + CenterX, SfCy + CenterY, dc->eboLine);
       PUTLINE(EfCx + CenterX, EfCy + CenterY, CenterX, CenterY, dc->eboLine);
    }
    if (arctype == GdiTypeChord)
        PUTLINE(EfCx + CenterX, EfCy + CenterY, SfCx + CenterX, SfCy + CenterY, dc->eboLine);

    pbrPen->ptPenWidth.x = PenOrigWidth;
    PEN_ShareUnlockPen(pbrPen);
    DPRINT("IntArc Exit.\n");
    return ret;
}


BOOL FASTCALL
IntGdiArcInternal(
          ARCTYPE arctype,
          DC  *dc,
          int LeftRect,
          int TopRect,
          int RightRect,
          int BottomRect,
          int XStartArc,
          int YStartArc,
          int XEndArc,
          int YEndArc)
{
  BOOL Ret;
  PDC_ATTR pdcattr;

  DPRINT("StartX: %d, StartY: %d, EndX: %d, EndY: %d\n",
           XStartArc,YStartArc,XEndArc,YEndArc);
  DPRINT("Left: %d, Top: %d, Right: %d, Bottom: %d\n",
           LeftRect,TopRect,RightRect,BottomRect);

  if ((LeftRect == RightRect) || (TopRect == BottomRect)) return TRUE;

  if (PATH_IsPathOpen(dc->dclevel))
  {
     return PATH_Arc( dc,
                LeftRect,
                 TopRect,
               RightRect,
              BottomRect,
               XStartArc,
               YStartArc,
                 XEndArc,
                 YEndArc,
                 arctype);
  }

  pdcattr = dc->pdcattr;

  if (arctype == GdiTypeArcTo)
  {
    if (dc->dclevel.flPath & DCPATH_CLOCKWISE)
       IntGdiLineTo(dc, XEndArc, YEndArc);
    else
       IntGdiLineTo(dc, XStartArc, YStartArc);
  }

  Ret = IntArc( dc,
          LeftRect,
           TopRect,
         RightRect,
        BottomRect,
         XStartArc,
         YStartArc,
           XEndArc,
           YEndArc,
           arctype);

  if (arctype == GdiTypeArcTo)
  {
     if (dc->dclevel.flPath & DCPATH_CLOCKWISE)
       IntGdiMoveToEx(dc, XStartArc, YStartArc, NULL, TRUE);
     else
       IntGdiMoveToEx(dc, XEndArc, YEndArc, NULL, TRUE);
  }
  return Ret;
}

BOOL
FASTCALL
IntGdiAngleArc( PDC pDC,
                  INT x,
                  INT y,
         DWORD dwRadius,
      FLOAT eStartAngle,
      FLOAT eSweepAngle)
{
  INT  x1, y1, x2, y2, arcdir;
  BOOL result;

  /* Calculate the end point */
  x2 = x + (INT)(cos(((eStartAngle+eSweepAngle)/360)*(M_PI*2)) * dwRadius);
  y2 = y - (INT)(sin(((eStartAngle+eSweepAngle)/360)*(M_PI*2)) * dwRadius);

  x1 = x + (INT)(cos((eStartAngle/360)*(M_PI*2)) * dwRadius);
  y1 = y - (INT)(sin((eStartAngle/360)*(M_PI*2)) * dwRadius);

  arcdir = pDC->dclevel.flPath & DCPATH_CLOCKWISE;
  if (eSweepAngle >= 0)
     pDC->dclevel.flPath &= ~DCPATH_CLOCKWISE;
  else
     pDC->dclevel.flPath |= DCPATH_CLOCKWISE;

  result = IntGdiArcInternal( GdiTypeArcTo,
                                       pDC,
                                x-dwRadius,
                                y-dwRadius,
                                x+dwRadius,
                                y+dwRadius,
                                        x1,
                                        y1,
                                        x2,
                                        y2 );

  pDC->dclevel.flPath |= (arcdir & DCPATH_CLOCKWISE);

  if (result)
  {
     IntGdiMoveToEx(pDC, x2, y2, NULL, TRUE);
  }
  return result;
}

/* FUNCTIONS *****************************************************************/

BOOL
APIENTRY
NtGdiAngleArc(
    IN HDC hDC,
    IN INT x,
    IN INT y,
    IN DWORD dwRadius,
    IN DWORD dwStartAngle,
    IN DWORD dwSweepAngle)
{
  DC *pDC;
  BOOL Ret = FALSE;
  gxf_long worker, worker1;

  pDC = DC_LockDc (hDC);
  if(!pDC)
  {
    EngSetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }
  if (pDC->dctype == DC_TYPE_INFO)
  {
    DC_UnlockDc(pDC);
    /* Yes, Windows really returns TRUE in this case */
    return TRUE;
  }
  worker.l  = dwStartAngle;
  worker1.l = dwSweepAngle;
  DC_vPrepareDCsForBlit(pDC, pDC->rosdc.CombinedClip->rclBounds,
                           NULL, pDC->rosdc.CombinedClip->rclBounds);
  if (pDC->pdcattr->ulDirty_ & (DIRTY_FILL | DC_BRUSH_DIRTY))
    DC_vUpdateFillBrush(pDC);
  if (pDC->pdcattr->ulDirty_ & (DIRTY_LINE | DC_PEN_DIRTY))
    DC_vUpdateLineBrush(pDC);
  Ret = IntGdiAngleArc( pDC, x, y, dwRadius, worker.f, worker1.f);
  DC_vFinishBlit(pDC, NULL);
  DC_UnlockDc( pDC );
  return Ret;
}

BOOL
APIENTRY
NtGdiArcInternal(
        ARCTYPE arctype,
        HDC  hDC,
        int  LeftRect,
        int  TopRect,
        int  RightRect,
        int  BottomRect,
        int  XStartArc,
        int  YStartArc,
        int  XEndArc,
        int  YEndArc)
{
  DC *dc;
  BOOL Ret;

  dc = DC_LockDc (hDC);
  if(!dc)
  {
    EngSetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }
  if (dc->dctype == DC_TYPE_INFO)
  {
    DC_UnlockDc(dc);
    /* Yes, Windows really returns TRUE in this case */
    return TRUE;
  }

  DC_vPrepareDCsForBlit(dc, dc->rosdc.CombinedClip->rclBounds,
                            NULL, dc->rosdc.CombinedClip->rclBounds);

  if (dc->pdcattr->ulDirty_ & (DIRTY_FILL | DC_BRUSH_DIRTY))
    DC_vUpdateFillBrush(dc);

  if (dc->pdcattr->ulDirty_ & (DIRTY_LINE | DC_PEN_DIRTY))
    DC_vUpdateLineBrush(dc);

  Ret = IntGdiArcInternal(
                  arctype,
                  dc,
                  LeftRect,
                  TopRect,
                  RightRect,
                  BottomRect,
                  XStartArc,
                  YStartArc,
                  XEndArc,
                  YEndArc);

  DC_vFinishBlit(dc, NULL);
  DC_UnlockDc( dc );
  return Ret;
}

