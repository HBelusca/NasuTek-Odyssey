/*
 * PROJECT:         Odyssey win32 kernel mode subsystem
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            subsystems/win32/win32k/objects/font.c
 * PURPOSE:         Font
 * PROGRAMMER:
 */

/** Includes ******************************************************************/

#include <win32k.h>

#define NDEBUG
#include <debug.h>

DWORD FASTCALL GreGetGlyphIndicesW(HDC,LPWSTR,INT,LPWORD,DWORD,DWORD);

/** Internal ******************************************************************/

DWORD
FASTCALL
GreGetKerningPairs(
    HDC hDC,
    ULONG NumPairs,
    LPKERNINGPAIR krnpair)
{
  PDC dc;
  PDC_ATTR pdcattr;
  PTEXTOBJ TextObj;
  PFONTGDI FontGDI;
  DWORD Count;
  KERNINGPAIR *pKP;

  dc = DC_LockDc(hDC);
  if (!dc)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }

  pdcattr = dc->pdcattr;
  TextObj = RealizeFontInit(pdcattr->hlfntNew);
  DC_UnlockDc(dc);

  if (!TextObj)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }

  FontGDI = ObjToGDI(TextObj->Font, FONT);
  TEXTOBJ_UnlockText(TextObj);

  Count = ftGdiGetKerningPairs(FontGDI,0,NULL);

  if ( Count && krnpair )
  {
     if (Count > NumPairs)
     {
        EngSetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
     }
     pKP = ExAllocatePoolWithTag(PagedPool, Count * sizeof(KERNINGPAIR), GDITAG_TEXT);
     if (!pKP)
     {
        EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
     }
     ftGdiGetKerningPairs(FontGDI,Count,pKP);

     RtlCopyMemory(krnpair, pKP, Count * sizeof(KERNINGPAIR));

     ExFreePoolWithTag(pKP,GDITAG_TEXT);
  }
  return Count;
}

/*
 
  It is recommended that an application use the GetFontLanguageInfo function
  to determine whether the GCP_DIACRITIC, GCP_DBCS, GCP_USEKERNING, GCP_LIGATE,
  GCP_REORDER, GCP_GLYPHSHAPE, and GCP_KASHIDA values are valid for the
  currently selected font. If not valid, GetCharacterPlacement ignores the
  value.

  M$ must use a preset "compiled in" support for each language based releases.
  Odyssey uses FreeType, this will need to be supported. ATM this is hard coded
  for GCPCLASS_LATIN!

 */
#if 0
DWORD
FASTCALL
GreGetCharacterPlacementW(
    HDC hdc,
    LPWSTR pwsz,
    INT nCount,
    INT nMaxExtent,
    LPGCP_RESULTSW pgcpw,
    DWORD dwFlags)
{
  GCP_RESULTSW gcpwSave;
  UINT i, nSet, cSet;
  INT *tmpDxCaretPos;
  LONG Cx;
  SIZE Size = {0,0};
 
  DPRINT1("GreGCPW Start\n");

   if (!pgcpw)
   {
      if (GreGetTextExtentW( hdc, pwsz, nCount, &Size, 1))
         return MAKELONG(Size.cx, Size.cy);
      return 0;
   }

  DPRINT1("GreGCPW 1\n");

  RtlCopyMemory(&gcpwSave, pgcpw, sizeof(GCP_RESULTSW));

  cSet = nSet = nCount;

  if ( nCount > gcpwSave.nGlyphs ) cSet = gcpwSave.nGlyphs;

  /* GCP_JUSTIFY may only be used in conjunction with GCP_MAXEXTENT. */
  if ( dwFlags & GCP_JUSTIFY) dwFlags |= GCP_MAXEXTENT;

  if ( !gcpwSave.lpDx && gcpwSave.lpCaretPos )
     tmpDxCaretPos = gcpwSave.lpCaretPos;
  else
     tmpDxCaretPos = gcpwSave.lpDx;  

  if ( !GreGetTextExtentExW( hdc,
                             pwsz,
                             cSet,
                             nMaxExtent,
                            ((dwFlags & GCP_MAXEXTENT) ? (PULONG) &cSet : NULL),
                            (PULONG) tmpDxCaretPos,
                             &Size,
                             0) )
  {
     return 0;
  }

  DPRINT1("GreGCPW 2\n");

  nSet = cSet;

  if ( tmpDxCaretPos && nSet > 0)
  {  
      for (i = (nSet - 1); i > 0; i--)
      {
          tmpDxCaretPos[i] -= tmpDxCaretPos[i - 1];
      }
  }

  if ( !(dwFlags & GCP_MAXEXTENT) || nSet )
  {
     if ( (dwFlags & GCP_USEKERNING) &&
           ( gcpwSave.lpDx ||
             gcpwSave.lpCaretPos ) &&
           nSet >= 2 )
     {
        DWORD Count;
        LPKERNINGPAIR pKP;
        
        Count = GreGetKerningPairs( hdc, 0, NULL);
        if (Count)
        {
           pKP = ExAllocatePoolWithTag(PagedPool, Count * sizeof(KERNINGPAIR), GDITAG_TEXT);
           if (pKP)
           {
              if ( GreGetKerningPairs( hdc, Count, pKP) != Count)
              {
                 ExFreePoolWithTag( pKP, GDITAG_TEXT);
                 return 0;
              }

              if ( (ULONG_PTR)(pKP) < ((ULONG_PTR)(pKP) + (ULONG_PTR)(Count * sizeof(KERNINGPAIR))) )
              {
                 DPRINT1("We Need to Do Something HERE!\n");
              }

              ExFreePoolWithTag( pKP, GDITAG_TEXT);

              if ( dwFlags & GCP_MAXEXTENT )
              {
                 if ( Size.cx > nMaxExtent )
                 {
                    for (Cx = Size.cx; nSet > 0; nSet--)
                    {
                        Cx -= tmpDxCaretPos[nSet - 1];
                        Size.cx = Cx;
                        if ( Cx <= nMaxExtent ) break;
                    }
                 }
                 if ( !nSet )
                 {
                    pgcpw->nGlyphs = 0;
                    pgcpw->nMaxFit = 0;
                    return 0;
                 }
              }
           }
        }
     }

     if ( (dwFlags & GCP_JUSTIFY) &&
           ( gcpwSave.lpDx ||
             gcpwSave.lpCaretPos ) &&
           nSet )
     {
         DPRINT1("We Need to Do Something HERE 2!\n");
     }

     if ( gcpwSave.lpDx && gcpwSave.lpCaretPos )
        RtlCopyMemory( gcpwSave.lpCaretPos, gcpwSave.lpDx, nSet * sizeof(LONG));

     if ( gcpwSave.lpCaretPos )
     {
        int pos = 0;
        i = 0;
        if ( nSet > 0 )
        {
           do
           {
              Cx = gcpwSave.lpCaretPos[i];
              gcpwSave.lpCaretPos[i] = pos;
              pos += Cx;
              ++i;
           }
           while ( i < nSet );
        }
     }

     if ( gcpwSave.lpOutString )
        RtlCopyMemory(gcpwSave.lpOutString, pwsz,  nSet * sizeof(WCHAR));

     if ( gcpwSave.lpClass )
        RtlFillMemory(gcpwSave.lpClass, nSet, GCPCLASS_LATIN);

     if ( gcpwSave.lpOrder )
     {
        for (i = 0; i < nSet; i++)
           gcpwSave.lpOrder[i] = i;
     }

     if ( gcpwSave.lpGlyphs )
     {
        if ( GreGetGlyphIndicesW( hdc, pwsz, nSet, gcpwSave.lpGlyphs, 0, 0) == GDI_ERROR )
        {
           nSet = 0;
           Size.cx = 0;
           Size.cy = 0;
        }
     }
     pgcpw->nGlyphs = nSet;
     pgcpw->nMaxFit = nSet;
  }
  DPRINT1("GreGCPW Exit\n");
  return MAKELONG(Size.cx, Size.cy);
}
#endif

INT
FASTCALL
FontGetObject(PTEXTOBJ TFont, INT Count, PVOID Buffer)
{
  if( Buffer == NULL ) return sizeof(LOGFONTW);

  switch (Count)
  {
     case sizeof(ENUMLOGFONTEXDVW):
        RtlCopyMemory( (LPENUMLOGFONTEXDVW) Buffer,
                                            &TFont->logfont,
                                            sizeof(ENUMLOGFONTEXDVW));
        break;
     case sizeof(ENUMLOGFONTEXW):
        RtlCopyMemory( (LPENUMLOGFONTEXW) Buffer,
                                          &TFont->logfont.elfEnumLogfontEx,
                                          sizeof(ENUMLOGFONTEXW));
        break;

     case sizeof(EXTLOGFONTW):
     case sizeof(ENUMLOGFONTW):
        RtlCopyMemory((LPENUMLOGFONTW) Buffer,
                                    &TFont->logfont.elfEnumLogfontEx.elfLogFont,
                                       sizeof(ENUMLOGFONTW));
        break;

     case sizeof(LOGFONTW):
        RtlCopyMemory((LPLOGFONTW) Buffer,
                                   &TFont->logfont.elfEnumLogfontEx.elfLogFont,
                                   sizeof(LOGFONTW));
        break;

     default:
        EngSetLastError(ERROR_BUFFER_OVERFLOW);
        return 0;
  }
  return Count;
}

DWORD
FASTCALL
IntGetCharDimensions(HDC hdc, PTEXTMETRICW ptm, PDWORD height)
{
  PDC pdc;
  PDC_ATTR pdcattr;
  PTEXTOBJ TextObj;
  SIZE sz;
  TMW_INTERNAL tmwi;
  BOOL Good;

  static const WCHAR alphabet[] = {
        'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q',
        'r','s','t','u','v','w','x','y','z','A','B','C','D','E','F','G','H',
        'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',0};

  if(!ftGdiGetTextMetricsW(hdc, &tmwi)) return 0;

  pdc = DC_LockDc(hdc);

  if (!pdc) return 0;

  pdcattr = pdc->pdcattr;

  TextObj = RealizeFontInit(pdcattr->hlfntNew);
  if ( !TextObj )
  {
     DC_UnlockDc(pdc);
     return 0;
  }
  Good = TextIntGetTextExtentPoint(pdc, TextObj, alphabet, 52, 0, NULL, 0, &sz, 0);
  TEXTOBJ_UnlockText(TextObj);
  DC_UnlockDc(pdc);

  if (!Good) return 0;
  if (ptm) *ptm = tmwi.TextMetric;
  if (height) *height = tmwi.TextMetric.tmHeight;

  return (sz.cx / 26 + 1) / 2;
}


DWORD
FASTCALL
IntGetFontLanguageInfo(PDC Dc)
{
  PDC_ATTR pdcattr;
  FONTSIGNATURE fontsig;
  static const DWORD GCP_DBCS_MASK=0x003F0000,
		GCP_DIACRITIC_MASK=0x00000000,
		FLI_GLYPHS_MASK=0x00000000,
		GCP_GLYPHSHAPE_MASK=0x00000040,
		GCP_KASHIDA_MASK=0x00000000,
		GCP_LIGATE_MASK=0x00000000,
		GCP_USEKERNING_MASK=0x00000000,
		GCP_REORDER_MASK=0x00000060;

  DWORD result=0;

  ftGdiGetTextCharsetInfo( Dc, &fontsig, 0 );

 /* We detect each flag we return using a bitmask on the Codepage Bitfields */
  if( (fontsig.fsCsb[0]&GCP_DBCS_MASK)!=0 )
		result|=GCP_DBCS;

  if( (fontsig.fsCsb[0]&GCP_DIACRITIC_MASK)!=0 )
		result|=GCP_DIACRITIC;

  if( (fontsig.fsCsb[0]&FLI_GLYPHS_MASK)!=0 )
		result|=FLI_GLYPHS;

  if( (fontsig.fsCsb[0]&GCP_GLYPHSHAPE_MASK)!=0 )
		result|=GCP_GLYPHSHAPE;

  if( (fontsig.fsCsb[0]&GCP_KASHIDA_MASK)!=0 )
		result|=GCP_KASHIDA;

  if( (fontsig.fsCsb[0]&GCP_LIGATE_MASK)!=0 )
		result|=GCP_LIGATE;

  if( (fontsig.fsCsb[0]&GCP_USEKERNING_MASK)!=0 )
		result|=GCP_USEKERNING;

  pdcattr = Dc->pdcattr;

  /* this might need a test for a HEBREW- or ARABIC_CHARSET as well */
  if ( pdcattr->lTextAlign & TA_RTLREADING )
     if( (fontsig.fsCsb[0]&GCP_REORDER_MASK)!=0 )
                    result|=GCP_REORDER;

  return result;
}

PTEXTOBJ
FASTCALL
RealizeFontInit(HFONT hFont)
{
  NTSTATUS Status = STATUS_SUCCESS;
  PTEXTOBJ pTextObj;

  pTextObj = TEXTOBJ_LockText(hFont);

  if ( pTextObj && !(pTextObj->fl & TEXTOBJECT_INIT))
  {
     Status = TextIntRealizeFont(hFont, pTextObj);
     if (!NT_SUCCESS(Status))
     {
        TEXTOBJ_UnlockText(pTextObj);
        return NULL;
     }
  }
  return pTextObj;
}

HFONT
FASTCALL
GreSelectFont( HDC hDC, HFONT hFont)
{
    PDC pdc;
    PDC_ATTR pdcattr;
    PTEXTOBJ pOrgFnt, pNewFnt = NULL;
    HFONT hOrgFont = NULL;

    if (!hDC || !hFont) return NULL;

    pdc = DC_LockDc(hDC);
    if (!pdc)
    {
        return NULL;
    }

    if (NT_SUCCESS(TextIntRealizeFont((HFONT)hFont,NULL)))
    {
       /* LFONTOBJ use share and locking. */
       pNewFnt = TEXTOBJ_LockText(hFont);
       pdcattr = pdc->pdcattr;
       pOrgFnt = pdc->dclevel.plfnt;
       if (pOrgFnt)
       {
          hOrgFont = pOrgFnt->BaseObject.hHmgr;
       }
       else
       {
          hOrgFont = pdcattr->hlfntNew;
       }
       pdc->dclevel.plfnt = pNewFnt;
       pdc->hlfntCur = hFont;
       pdcattr->hlfntNew = hFont;
       pdcattr->ulDirty_ |= DIRTY_CHARSET;
       pdcattr->ulDirty_ &= ~SLOW_WIDTHS;
    }

    if (pNewFnt) TEXTOBJ_UnlockText(pNewFnt);
    DC_UnlockDc(pdc);
    return hOrgFont;
}

/** Functions ******************************************************************/

INT
APIENTRY
NtGdiAddFontResourceW(
    IN WCHAR *pwszFiles,
    IN ULONG cwc,
    IN ULONG cFiles,
    IN FLONG fl,
    IN DWORD dwPidTid,
    IN OPTIONAL DESIGNVECTOR *pdv)
{
  UNICODE_STRING SafeFileName;
  PWSTR src;
  NTSTATUS Status;
  int Ret;

  /* FIXME - Protect with SEH? */
  RtlInitUnicodeString(&SafeFileName, pwszFiles);

  /* Reserve for prepending '\??\' */
  SafeFileName.Length += 4 * sizeof(WCHAR);
  SafeFileName.MaximumLength += 4 * sizeof(WCHAR);

  src = SafeFileName.Buffer;
  SafeFileName.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool, SafeFileName.MaximumLength, TAG_STRING);
  if(!SafeFileName.Buffer)
  {
    EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return 0;
  }

  /* Prepend '\??\' */
  RtlCopyMemory(SafeFileName.Buffer, L"\\??\\", 4 * sizeof(WCHAR));

  Status = MmCopyFromCaller(SafeFileName.Buffer + 4, src, SafeFileName.MaximumLength - (4 * sizeof(WCHAR)));
  if(!NT_SUCCESS(Status))
  {
    ExFreePoolWithTag(SafeFileName.Buffer, TAG_STRING);
    SetLastNtError(Status);
    return 0;
  }

  Ret = IntGdiAddFontResource(&SafeFileName, (DWORD)fl);

  ExFreePoolWithTag(SafeFileName.Buffer, TAG_STRING);
  return Ret;
}

 /*
 * @unimplemented
 */
DWORD
APIENTRY
NtGdiGetCharacterPlacementW(
    IN HDC hdc,
    IN LPWSTR pwsz,
    IN INT nCount,
    IN INT nMaxExtent,
    IN OUT LPGCP_RESULTSW pgcpw,
    IN DWORD dwFlags)
{
    UNIMPLEMENTED;
    return 0;
#if 0
    return GreGetCharacterPlacementW( hdc,
                                      pwsz,
                                      nCount,
                                      nMaxExtent,
                                      pgcpw,
                                      dwFlags);
#endif
}

DWORD
APIENTRY
NtGdiGetFontData(
   HDC hDC,
   DWORD Table,
   DWORD Offset,
   LPVOID Buffer,
   DWORD Size)
{
  PDC Dc;
  PDC_ATTR pdcattr;
  HFONT hFont;
  PTEXTOBJ TextObj;
  PFONTGDI FontGdi;
  DWORD Result = GDI_ERROR;
  NTSTATUS Status = STATUS_SUCCESS;

  if (Buffer && Size)
  {
     _SEH2_TRY
     {
         ProbeForRead(Buffer, Size, 1);
     }
     _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
     {
         Status = _SEH2_GetExceptionCode();
     }
     _SEH2_END
  }

  if (!NT_SUCCESS(Status)) return Result;

  Dc = DC_LockDc(hDC);
  if (Dc == NULL)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return GDI_ERROR;
  }
  pdcattr = Dc->pdcattr;

  hFont = pdcattr->hlfntNew;
  TextObj = RealizeFontInit(hFont);
  DC_UnlockDc(Dc);

  if (TextObj == NULL)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return GDI_ERROR;
  }

  FontGdi = ObjToGDI(TextObj->Font, FONT);

  Result = ftGdiGetFontData(FontGdi, Table, Offset, Buffer, Size);

  TEXTOBJ_UnlockText(TextObj);

  return Result;
}

 /*
 * @implemented
 */
DWORD
APIENTRY
NtGdiGetFontUnicodeRanges(
    IN HDC hdc,
    OUT OPTIONAL LPGLYPHSET pgs)
{
  PDC pDc;
  PDC_ATTR pdcattr;
  HFONT hFont;
  PTEXTOBJ TextObj;
  PFONTGDI FontGdi;
  DWORD Size = 0;
  PGLYPHSET pgsSafe;
  NTSTATUS Status = STATUS_SUCCESS;

  pDc = DC_LockDc(hdc);
  if (!pDc)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }

  pdcattr = pDc->pdcattr;

  hFont = pdcattr->hlfntNew;
  TextObj = RealizeFontInit(hFont);

  if ( TextObj == NULL)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     goto Exit;
  }
  FontGdi = ObjToGDI(TextObj->Font, FONT);

  Size = ftGetFontUnicodeRanges( FontGdi, NULL);

  if (Size && pgs)
  {
     pgsSafe = ExAllocatePoolWithTag(PagedPool, Size, GDITAG_TEXT);
     if (!pgsSafe)
     {
        EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        Size = 0;
        goto Exit;
     }

     Size = ftGetFontUnicodeRanges( FontGdi, pgsSafe);

     if (Size)
     {
        _SEH2_TRY
        {
            ProbeForWrite(pgs, Size, 1);
            RtlCopyMemory(pgs, pgsSafe, Size);
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
           Status = _SEH2_GetExceptionCode();
        }
        _SEH2_END

        if (!NT_SUCCESS(Status)) Size = 0;
     }
     ExFreePoolWithTag(pgsSafe, GDITAG_TEXT);
  }
Exit:
  TEXTOBJ_UnlockText(TextObj);
  DC_UnlockDc(pDc);
  return Size;
}

ULONG
APIENTRY
NtGdiGetGlyphOutline(
    IN HDC hdc,
    IN WCHAR wch,
    IN UINT iFormat,
    OUT LPGLYPHMETRICS pgm,
    IN ULONG cjBuf,
    OUT OPTIONAL PVOID UnsafeBuf,
    IN LPMAT2 pmat2,
    IN BOOL bIgnoreRotation)
{
  ULONG Ret = GDI_ERROR;
  PDC dc;
  PVOID pvBuf = NULL;
  GLYPHMETRICS gm;
  NTSTATUS Status = STATUS_SUCCESS;

  dc = DC_LockDc(hdc);
  if (!dc)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return GDI_ERROR;
  }

  if (UnsafeBuf && cjBuf)
  {
     pvBuf = ExAllocatePoolWithTag(PagedPool, cjBuf, GDITAG_TEXT);
     if (!pvBuf)
     {
        EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto Exit;
     }
  }

  Ret = ftGdiGetGlyphOutline( dc,
                             wch,
                         iFormat,
                             pgm ? &gm : NULL,
                           cjBuf,
                           pvBuf,
                           pmat2,
                 bIgnoreRotation);

  if (pvBuf)
  {
     _SEH2_TRY
     {
         ProbeForWrite(UnsafeBuf, cjBuf, 1);
         RtlCopyMemory(UnsafeBuf, pvBuf, cjBuf);
     }
     _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
     {
         Status = _SEH2_GetExceptionCode();
     }
     _SEH2_END

     ExFreePoolWithTag(pvBuf, GDITAG_TEXT);
  }

  if (pgm)
  {
     _SEH2_TRY
     {
         ProbeForWrite(pgm, sizeof(GLYPHMETRICS), 1);
         RtlCopyMemory(pgm, &gm, sizeof(GLYPHMETRICS));
     }
     _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
     {
         Status = _SEH2_GetExceptionCode();
     }
     _SEH2_END
  }

  if (! NT_SUCCESS(Status))
  {
     EngSetLastError(ERROR_INVALID_PARAMETER);
     Ret = GDI_ERROR;
  }

Exit:
  DC_UnlockDc(dc);
  return Ret;
}

DWORD
APIENTRY
NtGdiGetKerningPairs(HDC  hDC,
                     ULONG  NumPairs,
                     LPKERNINGPAIR  krnpair)
{
  PDC dc;
  PDC_ATTR pdcattr;
  PTEXTOBJ TextObj;
  PFONTGDI FontGDI;
  DWORD Count;
  KERNINGPAIR *pKP;
  NTSTATUS Status = STATUS_SUCCESS;

  dc = DC_LockDc(hDC);
  if (!dc)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }

  pdcattr = dc->pdcattr;
  TextObj = RealizeFontInit(pdcattr->hlfntNew);
  DC_UnlockDc(dc);

  if (!TextObj)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }

  FontGDI = ObjToGDI(TextObj->Font, FONT);
  TEXTOBJ_UnlockText(TextObj);

  Count = ftGdiGetKerningPairs(FontGDI,0,NULL);

  if ( Count && krnpair )
  {
     if (Count > NumPairs)
     {
        EngSetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
     }
     pKP = ExAllocatePoolWithTag(PagedPool, Count * sizeof(KERNINGPAIR), GDITAG_TEXT);
     if (!pKP)
     {
        EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
     }
     ftGdiGetKerningPairs(FontGDI,Count,pKP);
     _SEH2_TRY
     {
        ProbeForWrite(krnpair, Count * sizeof(KERNINGPAIR), 1);
        RtlCopyMemory(krnpair, pKP, Count * sizeof(KERNINGPAIR));
     }
     _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
     {
        Status = _SEH2_GetExceptionCode();
     }
     _SEH2_END
     if (!NT_SUCCESS(Status))
     {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        Count = 0;
     }
     ExFreePoolWithTag(pKP,GDITAG_TEXT);
  }
  return Count;
}

/*
 From "Undocumented Windows 2000 Secrets" Appendix B, Table B-2, page
 472, this is NtGdiGetOutlineTextMetricsInternalW.
 */
ULONG
APIENTRY
NtGdiGetOutlineTextMetricsInternalW (HDC  hDC,
                                   ULONG  Data,
                      OUTLINETEXTMETRICW  *otm,
                                   TMDIFF *Tmd)
{
  PDC dc;
  PDC_ATTR pdcattr;
  PTEXTOBJ TextObj;
  PFONTGDI FontGDI;
  HFONT hFont = 0;
  ULONG Size;
  OUTLINETEXTMETRICW *potm;
  NTSTATUS Status = STATUS_SUCCESS;

  dc = DC_LockDc(hDC);
  if (!dc)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }
  pdcattr = dc->pdcattr;
  hFont = pdcattr->hlfntNew;
  TextObj = RealizeFontInit(hFont);
  DC_UnlockDc(dc);
  if (!TextObj)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }
  FontGDI = ObjToGDI(TextObj->Font, FONT);
  TEXTOBJ_UnlockText(TextObj);
  Size = IntGetOutlineTextMetrics(FontGDI, 0, NULL);
  if (!otm) return Size;
  if (Size > Data)
  {
      EngSetLastError(ERROR_INSUFFICIENT_BUFFER);
      return 0;
  }
  potm = ExAllocatePoolWithTag(PagedPool, Size, GDITAG_TEXT);
  if (!potm)
  {
      EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return 0;
  }
  IntGetOutlineTextMetrics(FontGDI, Size, potm);
  if (otm)
  {
     _SEH2_TRY
     {
         ProbeForWrite(otm, Size, 1);
         RtlCopyMemory(otm, potm, Size);
     }
     _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
     {
         Status = _SEH2_GetExceptionCode();
     }
     _SEH2_END

     if (!NT_SUCCESS(Status))
     {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        Size = 0;
     }
  }
  ExFreePoolWithTag(potm,GDITAG_TEXT);
  return Size;
}

W32KAPI
BOOL
APIENTRY
NtGdiGetFontResourceInfoInternalW(
    IN LPWSTR   pwszFiles,
    IN ULONG    cwc,
    IN ULONG    cFiles,
    IN UINT     cjIn,
    OUT LPDWORD pdwBytes,
    OUT LPVOID  pvBuf,
    IN DWORD    dwType)
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD dwBytes;
    UNICODE_STRING SafeFileNames;
    BOOL bRet = FALSE;
    ULONG cbStringSize;

    union
    {
        LOGFONTW logfontw;
        WCHAR FullName[LF_FULLFACESIZE];
    } Buffer;

    /* FIXME: handle cFiles > 0 */

    /* Check for valid dwType values
       dwType == 4 seems to be handled by gdi32 only */
    if (dwType == 4 || dwType > 5)
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /* Allocate a safe unicode string buffer */
    cbStringSize = cwc * sizeof(WCHAR);
    SafeFileNames.MaximumLength = SafeFileNames.Length = cbStringSize - sizeof(WCHAR);
    SafeFileNames.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                 cbStringSize,
                                                 'RTSU');
    if (!SafeFileNames.Buffer)
    {
        EngSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    /* Check buffers and copy pwszFiles to safe unicode string */
    _SEH2_TRY
    {
        ProbeForRead(pwszFiles, cbStringSize, 1);
        ProbeForWrite(pdwBytes, sizeof(DWORD), 1);
        ProbeForWrite(pvBuf, cjIn, 1);

        RtlCopyMemory(SafeFileNames.Buffer, pwszFiles, cbStringSize);
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = _SEH2_GetExceptionCode();
    }
    _SEH2_END

    if(!NT_SUCCESS(Status))
    {
        SetLastNtError(Status);
        /* Free the string buffer for the safe filename */
        ExFreePoolWithTag(SafeFileNames.Buffer,'RTSU');
        return FALSE;
    }

    /* Do the actual call */
    bRet = IntGdiGetFontResourceInfo(&SafeFileNames, &Buffer, &dwBytes, dwType);

    /* Check if succeeded and the buffer is big enough */
    if (bRet && cjIn >= dwBytes)
    {
        /* Copy the data back to caller */
        _SEH2_TRY
        {
            /* Buffers are already probed */
            RtlCopyMemory(pvBuf, &Buffer, dwBytes);
            *pdwBytes = dwBytes;
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = _SEH2_GetExceptionCode();
        }
        _SEH2_END

        if(!NT_SUCCESS(Status))
        {
            SetLastNtError(Status);
            bRet = FALSE;
        }
    }

    /* Free the string for the safe filenames */
    ExFreePoolWithTag(SafeFileNames.Buffer,'RTSU');

    return bRet;
}

 /*
 * @unimplemented
 */
BOOL
APIENTRY
NtGdiGetRealizationInfo(
    IN HDC hdc,
    OUT PREALIZATION_INFO pri,
    IN HFONT hf)
{
  PDC pDc;
  PTEXTOBJ pTextObj;
  PFONTGDI pFontGdi;
  PDC_ATTR pdcattr;
  BOOL Ret = FALSE;
  INT i = 0;
  REALIZATION_INFO ri;

  pDc = DC_LockDc(hdc);
  if (!pDc)
  {
     EngSetLastError(ERROR_INVALID_HANDLE);
     return 0;
  }
  pdcattr = pDc->pdcattr;
  pTextObj = RealizeFontInit(pdcattr->hlfntNew);
  pFontGdi = ObjToGDI(pTextObj->Font, FONT);
  TEXTOBJ_UnlockText(pTextObj);
  DC_UnlockDc(pDc);

  Ret = ftGdiRealizationInfo(pFontGdi, &ri);
  if (Ret)
  {
     if (pri)
     {
        NTSTATUS Status = STATUS_SUCCESS;
        _SEH2_TRY
        {
            ProbeForWrite(pri, sizeof(REALIZATION_INFO), 1);
            RtlCopyMemory(pri, &ri, sizeof(REALIZATION_INFO));
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = _SEH2_GetExceptionCode();
        }
        _SEH2_END

        if(!NT_SUCCESS(Status))
        {
            SetLastNtError(Status);
            return FALSE;
        }
     }
     do
     {
        if (GdiHandleTable->cfPublic[i].hf == hf)
        {
           GdiHandleTable->cfPublic[i].iTechnology = ri.iTechnology;
           GdiHandleTable->cfPublic[i].iUniq = ri.iUniq;
           GdiHandleTable->cfPublic[i].dwUnknown = ri.dwUnknown;
           GdiHandleTable->cfPublic[i].dwCFCount = GdiHandleTable->dwCFCount;
           GdiHandleTable->cfPublic[i].fl |= CFONT_REALIZATION;
        }
        i++;
     }
     while ( i < GDI_CFONT_MAX );
  }
  return Ret;
}

HFONT
APIENTRY
NtGdiHfontCreate(
  IN PENUMLOGFONTEXDVW pelfw,
  IN ULONG cjElfw,
  IN LFTYPE lft,
  IN FLONG  fl,
  IN PVOID pvCliData )
{
  ENUMLOGFONTEXDVW SafeLogfont;
  HFONT hNewFont;
  PTEXTOBJ TextObj;
  NTSTATUS Status = STATUS_SUCCESS;

  /* Silence GCC warnings */
  SafeLogfont.elfEnumLogfontEx.elfLogFont.lfEscapement = 0;
  SafeLogfont.elfEnumLogfontEx.elfLogFont.lfOrientation = 0;

  if (!pelfw)
  {
      return NULL;
  }

  _SEH2_TRY
  {
      ProbeForRead(pelfw, sizeof(ENUMLOGFONTEXDVW), 1);
      RtlCopyMemory(&SafeLogfont, pelfw, sizeof(ENUMLOGFONTEXDVW));
  }
  _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
  {
      Status = _SEH2_GetExceptionCode();
  }
  _SEH2_END

  if (!NT_SUCCESS(Status))
  {
      return NULL;
  }

  TextObj = TEXTOBJ_AllocTextWithHandle();
  if (!TextObj)
  {
      return NULL;
  }
  hNewFont = TextObj->BaseObject.hHmgr;

  TextObj->lft = lft;
  TextObj->fl  = fl;
  RtlCopyMemory (&TextObj->logfont, &SafeLogfont, sizeof(ENUMLOGFONTEXDVW));

  if (SafeLogfont.elfEnumLogfontEx.elfLogFont.lfEscapement !=
      SafeLogfont.elfEnumLogfontEx.elfLogFont.lfOrientation)
  {
    /* this should really depend on whether GM_ADVANCED is set */
    TextObj->logfont.elfEnumLogfontEx.elfLogFont.lfOrientation =
    TextObj->logfont.elfEnumLogfontEx.elfLogFont.lfEscapement;
  }
  TEXTOBJ_UnlockText(TextObj);

  if (pvCliData && hNewFont)
  {
    // FIXME: use GDIOBJ_InsertUserData
    KeEnterCriticalRegion();
    {
       INT Index = GDI_HANDLE_GET_INDEX((HGDIOBJ)hNewFont);
       PGDI_TABLE_ENTRY Entry = &GdiHandleTable->Entries[Index];
       Entry->UserData = pvCliData;
    }
    KeLeaveCriticalRegion();
  }

  return hNewFont;
}

/*
 * @implemented
 */
HFONT
APIENTRY
NtGdiSelectFont(
    IN HDC hDC,
    IN HFONT hFont)
{
    return GreSelectFont(hDC, hFont);
}


/* EOF */
