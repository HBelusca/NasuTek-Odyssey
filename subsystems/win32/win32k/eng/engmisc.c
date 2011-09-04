/*
 *  Odyssey W32 Subsystem
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/* $Id: engmisc.c 49275 2010-10-25 17:36:27Z tkreuzer $ */

#include <win32k.h>

#define NDEBUG
#include <debug.h>

BOOL APIENTRY
IntEngEnter(PINTENG_ENTER_LEAVE EnterLeave,
            SURFOBJ *psoDest,
            RECTL *DestRect,
            BOOL ReadOnly,
            POINTL *Translate,
            SURFOBJ **ppsoOutput)
{
  LONG Exchange;
  SIZEL BitmapSize;
  POINTL SrcPoint;
  LONG Width;
  RECTL ClippedDestRect;

  /* Normalize */
  if (DestRect->right < DestRect->left)
    {
    Exchange = DestRect->left;
    DestRect->left = DestRect->right;
    DestRect->right = Exchange;
    }
  if (DestRect->bottom < DestRect->top)
    {
    Exchange = DestRect->top;
    DestRect->top = DestRect->bottom;
    DestRect->bottom = Exchange;
    }

  if (NULL != psoDest && STYPE_BITMAP != psoDest->iType &&
      (NULL == psoDest->pvScan0 || 0 == psoDest->lDelta))
    {
    /* Driver needs to support DrvCopyBits, else we can't do anything */
    SURFACE *psurfDest = CONTAINING_RECORD(psoDest, SURFACE, SurfObj);
    if (!(psurfDest->flags & HOOK_COPYBITS))
    {
      return FALSE;
    }

    /* Allocate a temporary bitmap */
    BitmapSize.cx = DestRect->right - DestRect->left;
    BitmapSize.cy = DestRect->bottom - DestRect->top;
    Width = WIDTH_BYTES_ALIGN32(BitmapSize.cx, BitsPerFormat(psoDest->iBitmapFormat));
    EnterLeave->OutputBitmap = EngCreateBitmap(BitmapSize, Width,
                                               psoDest->iBitmapFormat,
                                               BMF_TOPDOWN | BMF_NOZEROINIT, NULL);

    if (!EnterLeave->OutputBitmap)
      {
      DPRINT1("EngCreateBitmap() failed\n");
      return FALSE;
      }

    *ppsoOutput = EngLockSurface((HSURF)EnterLeave->OutputBitmap);
    if (*ppsoOutput == NULL)
    {
      EngDeleteSurface((HSURF)EnterLeave->OutputBitmap);
      return FALSE;
    }

    EnterLeave->DestRect.left = 0;
    EnterLeave->DestRect.top = 0;
    EnterLeave->DestRect.right = BitmapSize.cx;
    EnterLeave->DestRect.bottom = BitmapSize.cy;
    SrcPoint.x = DestRect->left;
    SrcPoint.y = DestRect->top;
    ClippedDestRect = EnterLeave->DestRect;
    if (SrcPoint.x < 0)
      {
        ClippedDestRect.left -= SrcPoint.x;
        SrcPoint.x = 0;
      }
    if (psoDest->sizlBitmap.cx < SrcPoint.x + ClippedDestRect.right - ClippedDestRect.left)
      {
        ClippedDestRect.right = ClippedDestRect.left + psoDest->sizlBitmap.cx - SrcPoint.x;
      }
    if (SrcPoint.y < 0)
      {
        ClippedDestRect.top -= SrcPoint.y;
        SrcPoint.y = 0;
      }
    if (psoDest->sizlBitmap.cy < SrcPoint.y + ClippedDestRect.bottom - ClippedDestRect.top)
      {
        ClippedDestRect.bottom = ClippedDestRect.top + psoDest->sizlBitmap.cy - SrcPoint.y;
      }
    EnterLeave->TrivialClipObj = EngCreateClip();
    if (EnterLeave->TrivialClipObj == NULL)
    {
      EngUnlockSurface(*ppsoOutput);
      EngDeleteSurface((HSURF)EnterLeave->OutputBitmap);
      return FALSE;
    }
    EnterLeave->TrivialClipObj->iDComplexity = DC_TRIVIAL;
    if (ClippedDestRect.left < (*ppsoOutput)->sizlBitmap.cx &&
        0 <= ClippedDestRect.right &&
        SrcPoint.x < psoDest->sizlBitmap.cx &&
        ClippedDestRect.top <= (*ppsoOutput)->sizlBitmap.cy &&
        0 <= ClippedDestRect.bottom &&
        SrcPoint.y < psoDest->sizlBitmap.cy &&
        ! GDIDEVFUNCS(psoDest).CopyBits(
                                        *ppsoOutput, psoDest,
                                        EnterLeave->TrivialClipObj, NULL,
                                        &ClippedDestRect, &SrcPoint))
      {
          EngDeleteClip(EnterLeave->TrivialClipObj);
          EngUnlockSurface(*ppsoOutput);
          EngDeleteSurface((HSURF)EnterLeave->OutputBitmap);
          return FALSE;
      }
    EnterLeave->DestRect.left = DestRect->left;
    EnterLeave->DestRect.top = DestRect->top;
    EnterLeave->DestRect.right = DestRect->right;
    EnterLeave->DestRect.bottom = DestRect->bottom;
    Translate->x = - DestRect->left;
    Translate->y = - DestRect->top;
    }
  else
    {
    Translate->x = 0;
    Translate->y = 0;
    *ppsoOutput = psoDest;
    }

  if (NULL != *ppsoOutput)
  {
    SURFACE* psurfOutput = CONTAINING_RECORD(*ppsoOutput, SURFACE, SurfObj);
    if (0 != (psurfOutput->flags & HOOK_SYNCHRONIZE))
    {
      if (NULL != GDIDEVFUNCS(*ppsoOutput).SynchronizeSurface)
        {
          GDIDEVFUNCS(*ppsoOutput).SynchronizeSurface(*ppsoOutput, DestRect, 0);
        }
      else if (STYPE_BITMAP == (*ppsoOutput)->iType
               && NULL != GDIDEVFUNCS(*ppsoOutput).Synchronize)
        {
          GDIDEVFUNCS(*ppsoOutput).Synchronize((*ppsoOutput)->dhpdev, DestRect);
        }
    }
  }
  else return FALSE;

  EnterLeave->DestObj = psoDest;
  EnterLeave->OutputObj = *ppsoOutput;
  EnterLeave->ReadOnly = ReadOnly;

  return TRUE;
}

BOOL APIENTRY
IntEngLeave(PINTENG_ENTER_LEAVE EnterLeave)
{
  POINTL SrcPoint;
  BOOL Result = TRUE;

  if (EnterLeave->OutputObj != EnterLeave->DestObj && NULL != EnterLeave->OutputObj)
    {
    if (! EnterLeave->ReadOnly)
      {
      SrcPoint.x = 0;
      SrcPoint.y = 0;
      if (EnterLeave->DestRect.left < 0)
        {
          SrcPoint.x = - EnterLeave->DestRect.left;
          EnterLeave->DestRect.left = 0;
        }
      if (EnterLeave->DestObj->sizlBitmap.cx < EnterLeave->DestRect.right)
        {
          EnterLeave->DestRect.right = EnterLeave->DestObj->sizlBitmap.cx;
        }
      if (EnterLeave->DestRect.top < 0)
        {
          SrcPoint.y = - EnterLeave->DestRect.top;
          EnterLeave->DestRect.top = 0;
        }
      if (EnterLeave->DestObj->sizlBitmap.cy < EnterLeave->DestRect.bottom)
        {
          EnterLeave->DestRect.bottom = EnterLeave->DestObj->sizlBitmap.cy;
        }
      if (SrcPoint.x < EnterLeave->OutputObj->sizlBitmap.cx &&
          EnterLeave->DestRect.left <= EnterLeave->DestRect.right &&
          EnterLeave->DestRect.left < EnterLeave->DestObj->sizlBitmap.cx &&
          SrcPoint.y < EnterLeave->OutputObj->sizlBitmap.cy &&
          EnterLeave->DestRect.top <= EnterLeave->DestRect.bottom &&
          EnterLeave->DestRect.top < EnterLeave->DestObj->sizlBitmap.cy)
        {
          Result = GDIDEVFUNCS(EnterLeave->DestObj).CopyBits(
                                                 EnterLeave->DestObj,
                                                 EnterLeave->OutputObj,
                                                 EnterLeave->TrivialClipObj, NULL,
                                                 &EnterLeave->DestRect, &SrcPoint);
        }
      else
        {
          Result = TRUE;
        }
      }
    EngUnlockSurface(EnterLeave->OutputObj);
    EngDeleteSurface((HSURF)EnterLeave->OutputBitmap);
    EngDeleteClip(EnterLeave->TrivialClipObj);
    }
  else
    {
    Result = TRUE;
    }

  return Result;
}

HANDLE APIENTRY
EngGetProcessHandle(VOID)
{
  /* http://www.osr.com/ddk/graphics/gdifncs_3tif.htm
     In Windows 2000 and later, the EngGetProcessHandle function always returns NULL.
     FIXME - what does NT4 return? */
  return NULL;
}

VOID
APIENTRY
EngGetCurrentCodePage(OUT PUSHORT OemCodePage,
                      OUT PUSHORT AnsiCodePage)
{
    /* Forward to kernel */
    RtlGetDefaultCodePage(AnsiCodePage, OemCodePage);
}

BOOL
APIENTRY
EngQuerySystemAttribute(
   IN ENG_SYSTEM_ATTRIBUTE CapNum,
   OUT PDWORD pCapability)
{
    SYSTEM_BASIC_INFORMATION sbi;
    SYSTEM_PROCESSOR_INFORMATION spi;

    switch (CapNum)
    {
        case EngNumberOfProcessors:
            NtQuerySystemInformation(SystemBasicInformation,
                                     &sbi,
                                     sizeof(SYSTEM_BASIC_INFORMATION),
                                     NULL);
            *pCapability = sbi.NumberOfProcessors;
            return TRUE;

        case EngProcessorFeature:
            NtQuerySystemInformation(SystemProcessorInformation,
                                     &spi,
                                     sizeof(SYSTEM_PROCESSOR_INFORMATION),
                                     NULL);
            *pCapability = spi.ProcessorFeatureBits;
            return TRUE;

        default:
            break;
    }

    return FALSE;
}

ULONGLONG
APIENTRY
EngGetTickCount(VOID)
{
    ULONG Multiplier;
    LARGE_INTEGER TickCount;

    /* Get the multiplier and current tick count */
    KeQueryTickCount(&TickCount);
    Multiplier = SharedUserData->TickCountMultiplier;

    /* Convert to milliseconds and return */
    return (Int64ShrlMod32(UInt32x32To64(Multiplier, TickCount.LowPart), 24) +
            (Multiplier * (TickCount.HighPart << 8)));
}




/* EOF */
