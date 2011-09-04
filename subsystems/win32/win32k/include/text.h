#pragma once

#define TAG_FINF        'FNIF'
//
// EXSTROBJ flags.
//
#define TO_MEM_ALLOCATED    0x0001
#define TO_ALL_PTRS_VALID   0x0002
#define TO_VALID            0x0004
#define TO_ESC_NOT_ORIENT   0x0008
#define TO_PWSZ_ALLOCATED   0x0010
#define TSIM_UNDERLINE1     0x0020
#define TSIM_UNDERLINE2     0x0040
#define TSIM_STRIKEOUT      0x0080
#define TO_HIGHRESTEXT      0x0100
#define TO_BITMAPS          0x0200
#define TO_PARTITION_INIT   0x0400
#define TO_ALLOC_FACENAME   0x0800
#define TO_SYS_PARTITION    0x1000
//
// Extended STROBJ
//
typedef struct _STRGDI
{
  STROBJ    StrObj; // Text string object header.
  FLONG     flTO;
  INT       cgposCopied;
  INT       cgposPositionsEnumerated;
  PVOID     prfo;  // PRFONT -> PFONTGDI
  PGLYPHPOS pgpos;
  POINTFIX  ptfxRef;
  POINTFIX  ptfxUpdate;
  POINTFIX  ptfxEscapement;
  RECTFX    rcfx;
  FIX       fxExtent;
  FIX       fxExtra;
  FIX       fxBreakExtra;
  DWORD     dwCodePage;
  INT       cExtraRects;
  RECTL     arclExtra[3];
  RECTL     rclBackGroundSave;
  PWCHAR    pwcPartition;
  PLONG     plPartition;
  PLONG     plNext;
  PGLYPHPOS pgpNext;
  PLONG     plCurrentFont;
  POINTL    ptlBaseLineAdjust;
  INT       cTTSysGlyphs;
  INT       cSysGlyphs;
  INT       cDefGlyphs;
  INT       cNumFaceNameGlyphs;
  PVOID     pacFaceNameGlyphs;
  ULONG     acFaceNameGlyphs[8];
} STRGDI, *PSTRGDI;

#define TEXTOBJECT_INIT 0x00010000

/* GDI logical font object */
typedef struct
{
  /* Header for all gdi objects in the handle table.
     Do not (re)move this. */
   BASEOBJECT    BaseObject;
   LFTYPE        lft;
   FLONG         fl;
   FONTOBJ      *Font;
   WCHAR         FullName[LF_FULLFACESIZE];
   WCHAR         Style[LF_FACESIZE];
   WCHAR         FaceName[LF_FACESIZE];
   DWORD         dwOffsetEndArray;
// Fixed:
   ENUMLOGFONTEXDVW logfont;
} TEXTOBJ, *PTEXTOBJ;

/*  Internal interface  */

#define  TEXTOBJ_AllocText()       ((PTEXTOBJ) GDIOBJ_AllocObj(GDIObjType_LFONT_TYPE))
#define  TEXTOBJ_AllocTextWithHandle() ((PTEXTOBJ) GDIOBJ_AllocObjWithHandle(GDI_OBJECT_TYPE_FONT, sizeof(TEXTOBJ)))
#define  TEXTOBJ_FreeText(pBMObj)  GDIOBJ_FreeObj((POBJ) pBMObj, GDILoObjType_LO_FONT_TYPE)
#define  TEXTOBJ_FreeTextByHandle(hBMObj)  GDIOBJ_FreeObj((HGDIOBJ) hBMObj, GDI_OBJECT_TYPE_FONT)
#define  TEXTOBJ_LockText(hBMObj) ((PTEXTOBJ) GDIOBJ_LockObject((HGDIOBJ) hBMObj, GDIObjType_LFONT_TYPE))
#define  TEXTOBJ_UnlockText(pBMObj) GDIOBJ_vUnlockObject ((POBJ)pBMObj)

PTEXTOBJ FASTCALL RealizeFontInit(HFONT);
NTSTATUS FASTCALL TextIntRealizeFont(HFONT,PTEXTOBJ);
NTSTATUS FASTCALL TextIntCreateFontIndirect(CONST LPLOGFONTW lf, HFONT *NewFont);
BOOL FASTCALL InitFontSupport(VOID);
BOOL FASTCALL IntIsFontRenderingEnabled(VOID);
BOOL FASTCALL IntIsFontRenderingEnabled(VOID);
VOID FASTCALL IntEnableFontRendering(BOOL Enable);
INT FASTCALL FontGetObject(PTEXTOBJ TextObj, INT Count, PVOID Buffer);
VOID FASTCALL IntLoadSystemFonts(VOID);
INT FASTCALL IntGdiAddFontResource(PUNICODE_STRING FileName, DWORD Characteristics);
ULONG FASTCALL ftGdiGetGlyphOutline(PDC,WCHAR,UINT,LPGLYPHMETRICS,ULONG,PVOID,LPMAT2,BOOL);
INT FASTCALL IntGetOutlineTextMetrics(PFONTGDI,UINT,OUTLINETEXTMETRICW *);
BOOL FASTCALL ftGdiGetRasterizerCaps(LPRASTERIZER_STATUS);
BOOL FASTCALL TextIntGetTextExtentPoint(PDC,PTEXTOBJ,LPCWSTR,INT,ULONG,LPINT,LPINT,LPSIZE,FLONG);
BOOL FASTCALL ftGdiGetTextMetricsW(HDC,PTMW_INTERNAL);
DWORD FASTCALL IntGetFontLanguageInfo(PDC);
INT FASTCALL ftGdiGetTextCharsetInfo(PDC,PFONTSIGNATURE,DWORD);
DWORD FASTCALL ftGetFontUnicodeRanges(PFONTGDI, PGLYPHSET);
DWORD FASTCALL ftGdiGetFontData(PFONTGDI,DWORD,DWORD,PVOID,DWORD);
BOOL FASTCALL IntGdiGetFontResourceInfo(PUNICODE_STRING,PVOID,DWORD*,DWORD);
BOOL FASTCALL ftGdiRealizationInfo(PFONTGDI,PREALIZATION_INFO);
DWORD FASTCALL ftGdiGetKerningPairs(PFONTGDI,DWORD,LPKERNINGPAIR);
BOOL NTAPI GreExtTextOutW(IN HDC,IN INT,IN INT,IN UINT,IN OPTIONAL RECTL*,
    IN LPWSTR, IN INT, IN OPTIONAL LPINT, IN DWORD);
DWORD FASTCALL IntGetCharDimensions(HDC, PTEXTMETRICW, PDWORD);
BOOL FASTCALL GreGetTextExtentW(HDC,LPWSTR,INT,LPSIZE,UINT);
BOOL FASTCALL GreGetTextExtentExW(HDC,LPWSTR,ULONG,ULONG,PULONG,PULONG,LPSIZE,FLONG);

#define IntLockProcessPrivateFonts(W32Process) \
  ExEnterCriticalRegionAndAcquireFastMutexUnsafe(&W32Process->PrivateFontListLock)

#define IntUnLockProcessPrivateFonts(W32Process) \
  ExReleaseFastMutexUnsafeAndLeaveCriticalRegion(&W32Process->PrivateFontListLock)

#define IntLockGlobalFonts \
  ExEnterCriticalRegionAndAcquireFastMutexUnsafe(&FontListLock)

#define IntUnLockGlobalFonts \
  ExReleaseFastMutexUnsafeAndLeaveCriticalRegion(&FontListLock)

#define IntLockFreeType \
  ExEnterCriticalRegionAndAcquireFastMutexUnsafe(&FreeTypeLock)

#define IntUnLockFreeType \
  ExReleaseFastMutexUnsafeAndLeaveCriticalRegion(&FreeTypeLock)
