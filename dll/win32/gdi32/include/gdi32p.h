/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey System Libraries
 * FILE:            lib/gdi32/include/gdi32p.h
 * PURPOSE:         User-Mode Win32 GDI Library Private Header
 * PROGRAMMER:      Alex Ionescu (alex@relsoft.net)
 */


/* DATA **********************************************************************/

extern PGDI_TABLE_ENTRY GdiHandleTable;
extern PGDI_SHARED_HANDLE_TABLE GdiSharedHandleTable;
extern HANDLE hProcessHeap;
extern HANDLE CurrentProcessId;
extern DWORD GDI_BatchLimit;
extern PDEVCAPS GdiDevCaps;
extern BOOL gbLpk;          // Global bool LanguagePack
extern HANDLE ghSpooler;
extern RTL_CRITICAL_SECTION semLocal;

typedef INT
(CALLBACK* EMFPLAYPROC)(
    HDC hdc,
    INT iFunction,
    HANDLE hPageQuery
);

/* DEFINES *******************************************************************/

#define HANDLE_LIST_INC 20

#define METAFILE_MEMORY 1
#define METAFILE_DISK   2

#define SAPCALLBACKDELAY 244

/* MACRO ********************************************************************/

#define ROP_USES_SOURCE(Rop)   (((Rop) << 2 ^ Rop) & 0xCC0000)

/* TYPES *********************************************************************/

// Based on wmfapi.h and Wine.
typedef struct tagMETAFILEDC
{
    PVOID       pvMetaBuffer;
    HANDLE      hFile;
    DWORD       Size;
    DWORD       dwWritten;
    METAHEADER  mh;
    WORD        reserved;
    HLOCAL      MFObjList;
    HPEN        hPen;
    HBRUSH      hBrush;
    HDC         hDc;
    HGDIOBJ     hMetaDc;
    HPALETTE    hPalette;
    HFONT       hFont;
    HBITMAP     hBitmap;
    HRGN        hRegion;
    HGDIOBJ     hMetafile;
    HGDIOBJ     hMemDc;
    HPEN        hExtPen;
    HGDIOBJ     hEnhMetaDc;
    HGDIOBJ     hEnhMetaFile;
    HCOLORSPACE hColorSpace;
    WCHAR       Filename[MAX_PATH+2];
} METAFILEDC,*PMETAFILEDC;

// Metafile Entry handle
typedef struct tagMF_ENTRY
{
    LIST_ENTRY   List;
    HGDIOBJ      hmDC;             // Handle return from NtGdiCreateClientObj.
    PMETAFILEDC pmfDC;
} MF_ENTRY, *PMF_ENTRY;

typedef struct tagENHMETAFILE
{
    PVOID      pvMetaBuffer;
    HANDLE     hFile;      /* Handle for disk based MetaFile */
    DWORD      Size;
    INT        iType;
    PENHMETAHEADER emf;
    UINT       handles_size, cur_handles;
    HGDIOBJ   *handles;
    INT        horzres, vertres;
    INT        horzsize, vertsize;
    INT        logpixelsx, logpixelsy;
    INT        bitspixel;
    INT        textcaps;
    INT        rastercaps;
    INT        technology;
    INT        planes;
} ENHMETAFILE,*PENHMETAFILE;


#define PDEV_UMPD_ID  0xFEDCBA98
// UMPDEV flags
#define UMPDEV_NO_ESCAPE      0x0002
#define UMPDEV_SUPPORT_ESCAPE 0x0004
typedef struct _UMPDEV
{
    DWORD           Sig;            // Init with PDEV_UMPD_ID
    struct _UMPDEV *pumpdNext;
    PDRIVER_INFO_5W pdi5Info;
    HMODULE         hModule;
    DWORD           dwFlags;
    DWORD           dwDriverAttributes;
    DWORD           dwConfigVersion; // Number of times the configuration
    // file for this driver has been upgraded
    // or downgraded since the last spooler restart.
    DWORD           dwDriverCount;   // After init should be 2
    DWORD           WOW64_UMPDev;
    DWORD           WOW64_hMod;
    DWORD           Unknown;
    PVOID           apfn[INDEX_LAST]; // Print Driver pfn
} UMPDEV, *PUMPDEV;

#define LOCALFONT_COUNT 10
typedef struct _LOCALFONT
{
    FONT_ATTR  lfa[LOCALFONT_COUNT];
} LOCALFONT, *PLOCALFONT;

// sdk/winspool.h
typedef BOOL (WINAPI *ABORTPRINTER) (HANDLE);
typedef BOOL (WINAPI *CLOSEPRINTER) (HANDLE);
typedef BOOL (WINAPI *CLOSESPOOLFILEHANDLE) (HANDLE, HANDLE); // W2k8
typedef HANDLE (WINAPI *COMMITSPOOLDATA) (HANDLE,HANDLE,DWORD); // W2k8
typedef LONG (WINAPI *DOCUMENTPROPERTIESW) (HWND,HANDLE,LPWSTR,PDEVMODEW,PDEVMODEW,DWORD);
typedef BOOL (WINAPI *ENDDOCPRINTER) (HANDLE);
typedef BOOL (WINAPI *ENDPAGEPRINTER) (HANDLE);
typedef BOOL (WINAPI *GETPRINTERW) (HANDLE,DWORD,LPBYTE,DWORD,LPDWORD);
typedef BOOL (WINAPI *GETPRINTERDRIVERW) (HANDLE,LPWSTR,DWORD,LPBYTE,DWORD,LPDWORD);
typedef HANDLE (WINAPI *GETSPOOLFILEHANDLE) (HANDLE); // W2k8
typedef BOOL (WINAPI *ISVALIDDEVMODEW) (PDEVMODEW,size_t);
typedef BOOL (WINAPI *OPENPRINTERW) (LPWSTR,PHANDLE,LPPRINTER_DEFAULTSW);
typedef BOOL (WINAPI *READPRINTER) (HANDLE,PVOID,DWORD,PDWORD);
typedef BOOL (WINAPI *RESETPRINTERW) (HANDLE,LPPRINTER_DEFAULTSW);
typedef LPWSTR (WINAPI *STARTDOCDLGW) (HANDLE,DOCINFOW *);
typedef DWORD (WINAPI *STARTDOCPRINTERW) (HANDLE,DWORD,PBYTE);
typedef BOOL (WINAPI *STARTPAGEPRINTER) (HANDLE);
// ddk/winsplp.h
typedef BOOL (WINAPI *SEEKPRINTER) (HANDLE,LARGE_INTEGER,PLARGE_INTEGER,DWORD,BOOL);
typedef BOOL (WINAPI *SPLREADPRINTER) (HANDLE,LPBYTE *,DWORD);
// Same as ddk/winsplp.h DriverUnloadComplete?
typedef BOOL (WINAPI *SPLDRIVERUNLOADCOMPLETE) (LPWSTR);
// Driver support:
// DrvDocumentEvent api/winddiui.h not W2k8 DocumentEventAW
typedef INT (WINAPI *DOCUMENTEVENT) (HANDLE,HDC,INT,ULONG,PVOID,ULONG,PVOID);
// DrvQueryColorProfile
typedef BOOL (WINAPI *QUERYCOLORPROFILE) (HANDLE,PDEVMODEW,ULONG,VOID*,ULONG,FLONG);
// Unknown:
typedef DWORD (WINAPI *QUERYSPOOLMODE) (HANDLE,DWORD,DWORD);
typedef DWORD (WINAPI *QUERYREMOTEFONTS) (DWORD,DWORD,DWORD);

extern CLOSEPRINTER fpClosePrinter;
extern OPENPRINTERW fpOpenPrinterW;

/* FUNCTIONS *****************************************************************/

PVOID
HEAP_alloc(DWORD len);

NTSTATUS
HEAP_strdupA2W(
    LPWSTR* ppszW,
    LPCSTR lpszA
);

VOID
HEAP_free(LPVOID memory);

VOID
FASTCALL
FONT_TextMetricWToA(
    const TEXTMETRICW *ptmW,
    LPTEXTMETRICA ptmA
);

VOID
FASTCALL
NewTextMetricW2A(
    NEWTEXTMETRICA *tma,
    NEWTEXTMETRICW *tmw
);

VOID
FASTCALL
NewTextMetricExW2A(
    NEWTEXTMETRICEXA *tma,
    NEWTEXTMETRICEXW *tmw
);

BOOL
FASTCALL
DeleteRegion( HRGN );

BOOL
GdiIsHandleValid(HGDIOBJ hGdiObj);

BOOL
GdiGetHandleUserData(
    HGDIOBJ hGdiObj,
    DWORD ObjectType,
    PVOID *UserData
);

PLDC
FASTCALL
GdiGetLDC(HDC hDC);

HGDIOBJ
WINAPI
GdiFixUpHandle(HGDIOBJ hGO);

BOOL
WINAPI
CalculateColorTableSize(
    CONST BITMAPINFOHEADER *BitmapInfoHeader,
    UINT *ColorSpec,
    UINT *ColorTableSize
);

LPBITMAPINFO
WINAPI
ConvertBitmapInfo(
    CONST BITMAPINFO *BitmapInfo,
    UINT ColorSpec,
    UINT *BitmapInfoSize,
    BOOL FollowedByData
);

DWORD
WINAPI
GetAndSetDCDWord( HDC, INT, DWORD, DWORD, DWORD, DWORD );

DWORD
WINAPI
GetDCDWord( HDC, INT, DWORD);

HGDIOBJ
WINAPI
GetDCObject( HDC, INT);

VOID
NTAPI
LogFontA2W(
    LPLOGFONTW pW,
    CONST LOGFONTA *pA
);

VOID
NTAPI
LogFontW2A(
    LPLOGFONTA pA,
    CONST LOGFONTW *pW
);

VOID
WINAPI
EnumLogFontExW2A(
    LPENUMLOGFONTEXA fontA,
    CONST ENUMLOGFONTEXW *fontW );

/* FIXME: Put in some public header */
UINT
WINAPI
UserRealizePalette(HDC hDC);

int
WINAPI
GdiAddFontResourceW(LPCWSTR lpszFilename,FLONG fl,DESIGNVECTOR *pdv);

VOID
WINAPI
GdiSetLastError( DWORD dwErrCode );

DWORD WINAPI GdiGetCodePage(HDC);

int
WINAPI
GdiGetBitmapBitsSize(BITMAPINFO *lpbmi);

VOID GdiSAPCallback(PLDC pldc);
HGDIOBJ FASTCALL hGetPEBHandle(HANDLECACHETYPE,COLORREF);

int FASTCALL DocumentEventEx(PVOID,HANDLE,HDC,int,ULONG,PVOID,ULONG,PVOID);
BOOL FASTCALL EndPagePrinterEx(PVOID,HANDLE);
BOOL FASTCALL LoadTheSpoolerDrv(VOID);

FORCEINLINE
PVOID
GdiAllocBatchCommand(
    HDC hdc,
    USHORT Cmd)
{
    PTEB pTeb;
    ULONG ulSize;
    PGDIBATCHHDR pHdr;

    /* Get a pointer to the TEB */
    pTeb = NtCurrentTeb();

    /* Check if we have a valid environment */
    if (!pTeb || !pTeb->Win32ThreadInfo) return NULL;

    /* Do we use a DC? */
    if (hdc)
    {
        /* If the batch DC is NULL, we set this one as the new one */
        if (!pTeb->GdiTebBatch.HDC) pTeb->GdiTebBatch.HDC = hdc;

        /* If not, check if the batch DC equal to our DC */
        else if (pTeb->GdiTebBatch.HDC != hdc) return NULL;
    }

    /* Get the size of the entry */
    switch(Cmd)
    {
    case GdiBCPatBlt:
        ulSize = 0;
        break;
    case GdiBCPolyPatBlt:
        ulSize = 0;
        break;
    case GdiBCTextOut:
        ulSize = 0;
        break;
    case GdiBCExtTextOut:
        ulSize = 0;
        break;
    case GdiBCSetBrushOrg:
        ulSize = 0;
        break;
    case GdiBCExtSelClipRgn:
        ulSize = 0;
        break;
    case GdiBCSelObj:
        ulSize = sizeof(GDIBSOBJECT);
        break;
    case GdiBCDelRgn:
        ulSize = sizeof(GDIBSOBJECT);
        break;
    case GdiBCDelObj:
        ulSize = sizeof(GDIBSOBJECT);
        break;
    default:
        return NULL;
    }

    /* Unsupported operation */
    if (ulSize == 0) return NULL;

    /* Check if the buffer is full */
    if ((pTeb->GdiBatchCount >= GDI_BatchLimit) ||
            ((pTeb->GdiTebBatch.Offset + ulSize) > GDIBATCHBUFSIZE))
    {
        /* Call win32k, the kernel will call NtGdiFlushUserBatch to flush
           the current batch */
        NtGdiFlush();
    }

    /* Get the head of the entry */
    pHdr = (PVOID)((PUCHAR)pTeb->GdiTebBatch.Buffer + pTeb->GdiTebBatch.Offset);

    /* Update Offset and batch count */
    pTeb->GdiTebBatch.Offset += ulSize;
    pTeb->GdiBatchCount++;

    /* Fill in the core fields */
    pHdr->Cmd = Cmd;
    pHdr->Size = ulSize;

    return pHdr;
}

FORCEINLINE
PDC_ATTR
GdiGetDcAttr(HDC hdc)
{
    PDC_ATTR pdcattr;

    if (!GdiGetHandleUserData((HGDIOBJ)hdc, GDI_OBJECT_TYPE_DC, (PVOID*)&pdcattr)) return NULL;
    return pdcattr;
}

#ifdef _M_IX86
FLOATL FASTCALL EFtoF(EFLOAT_S * efp);
#define FOtoF(pfo) EFtoF((EFLOAT_S*)pfo)
#else
#define FOtoF(pfo) (*(pfo))
#endif

/* This is an inlined version of lrintf. */
FORCEINLINE
int
_lrintf(float f)
{
#if defined(_M_IX86) && defined(__GNUC__)
    int ret;
    __asm__ __volatile__ ("fistpl %0" : "=m" (ret) : "t" (f) : "st");
    return ret;
#elif defined(_M_IX86) && defined(_MSC_VER)
    int ret;
    __asm
    {
        fld f;
        fistp ret;
    }
#else
    /* slow, but portable */
    return (int)(f >= 0 ? f+0.5 : f-0.5);
#endif
}

/* EOF */
