/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         shell32.dll
 * FILE:            dll/win32/shell32/stubs.c
 * PURPOSE:         shell32.dll stubs
 * PROGRAMMER:      Dmitry Chapyshev (dmitry@odyssey.org)
 * NOTES:           If you implement a function, remove it from this file
 * UPDATE HISTORY:
 *      03/02/2009  Created
 */


#include <precomp.h>

WINE_DEFAULT_DEBUG_CHANNEL(shell);

/*
 * Unimplemented
 */
HLOCAL
WINAPI
SHLocalAlloc(UINT uFlags, SIZE_T uBytes)
{
    FIXME("SHLocalAlloc() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
HLOCAL
WINAPI
SHLocalFree(HLOCAL hMem)
{
    FIXME("SHLocalFree() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
HLOCAL
WINAPI
SHLocalReAlloc(HLOCAL hMem,
               SIZE_T uBytes,
               UINT uFlags)
{
    FIXME("SHLocalReAlloc() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
LPWSTR
WINAPI
AddCommasW(DWORD dwUnknown, LPWSTR lpNumber)
{
    LPWSTR lpRetBuf = L"0";

    FIXME("AddCommasW() stub\n");
    return lpRetBuf;
}

/*
 * Unimplemented
 */
LPWSTR
WINAPI
ShortSizeFormatW(LONGLONG llNumber)
{
    FIXME("ShortSizeFormatW() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHFindComputer(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    FIXME("SHFindComputer() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHLimitInputEdit(HWND hWnd, LPVOID lpUnknown)
{
    FIXME("SHLimitInputEdit() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHLimitInputCombo(HWND hWnd, LPVOID lpUnknown)
{
    FIXME("SHLimitInputCombo() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
PathIsEqualOrSubFolder(LPWSTR lpFolder, LPWSTR lpSubFolder)
{
    FIXME("PathIsEqualOrSubFolder() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHCreateFileExtractIconW(LPCWSTR pszPath,
                         DWORD dwFileAttributes,
                         LPVOID lpUnknown1,
                         LPVOID lpUnknown2)
{
    FIXME("SHCreateFileExtractIconW() stub\n");
    return E_FAIL;
}

HRESULT
WINAPI
SHGetUnreadMailCountW(HKEY hKeyUser,
                      LPCWSTR pszMailAddress,
                      DWORD *pdwCount,
                      FILETIME *pFileTime,
                      LPCWSTR pszShellExecuteCommand,
                      int cchShellExecuteCommand)
{
    FIXME("SHGetUnreadMailCountW() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHSetUnreadMailCountW(LPCWSTR pszMailAddress,
                      DWORD dwCount,
                      LPCWSTR pszShellExecuteCommand)
{
    FIXME("SHSetUnreadMailCountW() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
VOID
WINAPI
CheckDiskSpace(VOID)
{
    FIXME("CheckDiskSpace() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
SHReValidateDarwinCache(VOID)
{
    FIXME("SHReValidateDarwinCache() stub\n");
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
CopyStreamUI(IStream *pSrc, IStream *pDst, IProgressDialog *pProgDlg)
{
    FIXME("CopyStreamUI() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
FILEDESCRIPTOR*
WINAPI
GetFileDescriptor(FILEGROUPDESCRIPTOR *pFileGroupDesc, BOOL bUnicode, INT iIndex, LPWSTR lpName)
{
    FIXME("GetFileDescriptor() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHIsTempDisplayMode(VOID)
{
    FIXME("SHIsTempDisplayMode() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
LONG
WINAPI
SHCreateSessionKey(REGSAM regSam, PHKEY phKey)
{
    FIXME("SHCreateSessionKey() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
MakeShellURLFromPathW(LPCWSTR lpPath, LPWSTR lpUrl, INT cchMax)
{
    FIXME("MakeShellURLFromPathW() stub\n");
    lpUrl = NULL;
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
MakeShellURLFromPathA(LPCSTR lpPath, LPSTR lpUrl, INT cchMax)
{
    FIXME("MakeShellURLFromPathA() stub\n");
    lpUrl = NULL;
    return FALSE;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHParseDarwinIDFromCacheW(LPCWSTR lpUnknown1, LPWSTR lpUnknown2)
{
    FIXME("SHParseDarwinIDFromCacheW() stub\n");
    lpUnknown2 = NULL;
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHMultiFileProperties(IDataObject *pDataObject, DWORD dwFlags)
{
    FIXME("SHMultiFileProperties() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHCreatePropertyBag(REFIID refIId, LPVOID *lpUnknown)
{
    /* Call SHCreatePropertyBagOnMemory() from shlwapi.dll */
    FIXME("SHCreatePropertyBag() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHCopyMonikerToTemp(IMoniker *pMoniker, LPCWSTR lpInput, LPWSTR lpOutput, INT cchMax)
{
    /* Unimplemented in XP SP3 */
    TRACE("SHCopyMonikerToTemp() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HLOCAL
WINAPI
CheckWinIniForAssocs(VOID)
{
    FIXME("CheckWinIniForAssocs() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHGetSetFolderCustomSettingsW(LPSHFOLDERCUSTOMSETTINGSW pfcs,
                              LPCWSTR pszPath,
                              DWORD dwReadWrite)
{
    FIXME("SHGetSetFolderCustomSettingsW() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHGetSetFolderCustomSettingsA(LPSHFOLDERCUSTOMSETTINGSA pfcs,
                              LPCSTR pszPath,
                              DWORD dwReadWrite)
{
    FIXME("SHGetSetFolderCustomSettingsA() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHOpenPropSheetA(LPSTR lpCaption,
                 HKEY hKeys[],
                 UINT uCount,
                 CLSID *pClsID,
                 IDataObject *pDataObject,
                 IShellBrowser *pShellBrowser,
                 LPSTR lpStartPage)
{
    FIXME("SHOpenPropSheetA() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHOpenPropSheetW(LPWSTR lpCaption,
                 HKEY hKeys[],
                 UINT uCount,
                 CLSID *pClsID,
                 IDataObject *pDataObject,
                 IShellBrowser *pShellBrowser,
                 LPWSTR lpStartPage)
{
    FIXME("SHOpenPropSheetW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
VOID
WINAPI
CDefFolderMenu_MergeMenu(HINSTANCE hInstance,
                         UINT uMainMerge,
                         UINT uPopupMerge,
                         LPQCMINFO lpQcmInfo)
{
    FIXME("CDefFolderMenu_MergeMenu() stub\n");
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
CDefFolderMenu_Create(LPITEMIDLIST pidlFolder,
                      HWND hwnd,
                      UINT uidl,
                      PCUITEMID_CHILD_ARRAY *apidl,
                      IShellFolder *psf,
                      LPFNDFMCALLBACK lpfn,
                      HKEY hProgID,
                      HKEY hBaseProgID,
                      IContextMenu **ppcm)
{
    FIXME("CDefFolderMenu_Create() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHChangeRegistrationReceive(LPVOID lpUnknown1, DWORD dwUnknown2)
{
    FIXME("SHChangeRegistrationReceive() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
VOID
WINAPI
SHWaitOp_Operate(LPVOID lpUnknown1, DWORD dwUnknown2)
{
    FIXME("SHWaitOp_Operate() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
SHChangeNotifyReceive(LONG lUnknown, UINT uUnknown, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    FIXME("SHChangeNotifyReceive() stub\n");
}

/*
 * Unimplemented
 */
INT
WINAPI
RealDriveTypeFlags(INT iDrive, BOOL bUnknown)
{
    FIXME("RealDriveTypeFlags() stub\n");
    return 1;
}

/*
 * Unimplemented
 */
LPWSTR
WINAPI
StrRStrW(LPWSTR lpSrc, LPWSTR lpLast, LPWSTR lpSearch)
{
    FIXME("StrRStrW() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
LPWSTR
WINAPI
StrRStrA(LPSTR lpSrc, LPSTR lpLast, LPSTR lpSearch)
{
    FIXME("StrRStrA() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
LONG
WINAPI
ShellHookProc(INT iCode, WPARAM wParam, LPARAM lParam)
{
    /* Unimplemented in WinXP SP3 */
    TRACE("ShellHookProc() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
VOID
WINAPI
ShellExec_RunDLL(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    FIXME("ShellExec_RunDLL() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
ShellExec_RunDLLA(HWND hwnd, HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow)
{
    FIXME("ShellExec_RunDLLA() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
ShellExec_RunDLLW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    FIXME("ShellExec_RunDLLW() stub\n");
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SheShortenPathW(LPWSTR lpPath, BOOL bShorten)
{
    FIXME("SheShortenPathW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SheShortenPathA(LPSTR lpPath, BOOL bShorten)
{
    FIXME("SheShortenPathA() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheSetCurDrive(INT iIndex)
{
    FIXME("SheSetCurDrive() stub\n");
    return 1;
}

/*
 * Unimplemented
 */
LPWSTR
WINAPI
SheRemoveQuotesW(LPWSTR lpInput)
{
    FIXME("SheRemoveQuotesW() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
LPSTR
WINAPI
SheRemoveQuotesA(LPSTR lpInput)
{
    FIXME("SheRemoveQuotesA() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheGetPathOffsetW(LPWSTR lpPath)
{
    FIXME("SheGetPathOffsetW() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SheGetDirExW(LPWSTR lpDrive,
             LPDWORD lpCurDirLen,
             LPWSTR lpCurDir)
{
    FIXME("SheGetDirExW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheGetCurDrive(VOID)
{
    FIXME("SheGetCurDrive() stub\n");
    return 1;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheFullPathW(LPWSTR lpFullName, DWORD dwPathSize, LPWSTR lpBuffer)
{
    FIXME("SheFullPathW() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheFullPathA(LPSTR lpFullName, DWORD dwPathSize, LPSTR lpBuffer)
{
    FIXME("SheFullPathA() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SheConvertPathW(LPWSTR lpCmd, LPWSTR lpFileName, UINT uCmdLen)
{
    FIXME("SheConvertPathW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheChangeDirExW(LPWSTR lpDir)
{
    FIXME("SheChangeDirExW() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
INT
WINAPI
SheChangeDirExA(LPSTR lpDir)
{
    FIXME("SheChangeDirExA() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHInvokePrinterCommandW(HWND hwnd,
                        UINT uAction,
                        LPCWSTR lpBuf1,
                        LPCWSTR lpBuf2,
                        BOOL fModal)
{
    FIXME("SHInvokePrinterCommandW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHInvokePrinterCommandA(HWND hwnd,
                        UINT uAction,
                        LPCSTR lpBuf1,
                        LPCSTR lpBuf2,
                        BOOL fModal)
{
    FIXME("SHInvokePrinterCommandA() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHCreateQueryCancelAutoPlayMoniker(IMoniker **ppmoniker)
{
    FIXME("SHCreateQueryCancelAutoPlayMoniker() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHCreateProcessAsUserW(PSHCREATEPROCESSINFOW pscpi)
{
    FIXME("SHCreateProcessAsUserW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHChangeNotifySuspendResume(BOOL bSuspend, 
                            LPITEMIDLIST pidl, 
                            BOOL bRecursive, 
                            DWORD dwReserved)
{
    FIXME("SHChangeNotifySuspendResume() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
RegenerateUserEnvironment(LPVOID *lpUnknown, BOOL bUnknown)
{
    FIXME("RegenerateUserEnvironment() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
HINSTANCE
WINAPI
RealShellExecuteExA(HWND hwnd,
                    LPCSTR lpOperation,
                    LPCSTR lpFile,
                    LPCSTR lpParameters,
                    LPCSTR lpDirectory,
                    LPSTR lpReturn,
                    LPCSTR lpTitle,
                    LPSTR lpReserved,
                    WORD nShowCmd,
                    HANDLE *lpProcess,
                    DWORD dwFlags)
{
    FIXME("RealShellExecuteExA() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
HINSTANCE
WINAPI
RealShellExecuteExW(HWND hwnd,
                    LPCWSTR lpOperation,
                    LPCWSTR lpFile,
                    LPCWSTR lpParameters,
                    LPCWSTR lpDirectory,
                    LPWSTR lpReturn,
                    LPCWSTR lpTitle,
                    LPWSTR lpReserved,
                    WORD nShowCmd,
                    HANDLE *lpProcess,
                    DWORD dwFlags)
{
    FIXME("RealShellExecuteExW() stub\n");
    return NULL;
}

/*
 * Implemented
 */
HINSTANCE
WINAPI
RealShellExecuteA(HWND hwnd,
                  LPCSTR lpOperation,
                  LPCSTR lpFile,
                  LPCSTR lpParameters,
                  LPCSTR lpDirectory,
                  LPSTR lpReturn,
                  LPCSTR lpTitle,
                  LPSTR lpReserved,
                  WORD nShowCmd,
                  HANDLE *lpProcess)
{
    return RealShellExecuteExA(hwnd,
                               lpOperation,
                               lpFile,
                               lpParameters,
                               lpDirectory,
                               lpReturn,
                               lpTitle,
                               lpReserved,
                               nShowCmd,
                               lpProcess,
                               0);
}

/*
 * Implemented
 */
HINSTANCE
WINAPI
RealShellExecuteW(HWND hwnd,
                  LPCWSTR lpOperation,
                  LPCWSTR lpFile,
                  LPCWSTR lpParameters,
                  LPCWSTR lpDirectory,
                  LPWSTR lpReturn,
                  LPCWSTR lpTitle,
                  LPWSTR lpReserved,
                  WORD nShowCmd,
                  HANDLE *lpProcess)
{
    return RealShellExecuteExW(hwnd,
                               lpOperation,
                               lpFile,
                               lpParameters,
                               lpDirectory,
                               lpReturn,
                               lpTitle,
                               lpReserved,
                               nShowCmd,
                               lpProcess,
                               0);
}

/*
 * Unimplemented
 */
VOID
WINAPI
PrintersGetCommand_RunDLL(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    FIXME("PrintersGetCommand_RunDLL() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
PrintersGetCommand_RunDLLA(HWND hwnd, HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow)
{
    FIXME("PrintersGetCommand_RunDLLA() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
PrintersGetCommand_RunDLLW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    FIXME("PrintersGetCommand_RunDLLW() stub\n");
}

/*
 * Unimplemented
 */
IShellFolderViewCB*
WINAPI
SHGetShellFolderViewCB(HWND hwnd)
{
    FIXME("SHGetShellFolderViewCB() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
INT
WINAPI
SHLookupIconIndexA(LPCSTR lpName, INT iIndex, UINT uFlags)
{
    FIXME("SHLookupIconIndexA() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
INT
WINAPI
SHLookupIconIndexW(LPCWSTR lpName, INT iIndex, UINT uFlags)
{
    FIXME("SHLookupIconIndexW() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
HANDLE
WINAPI
PifMgr_OpenProperties(LPWSTR lpAppPath, LPWSTR lpPifPath, UINT hInfIndex, UINT uUnknown)
{
    FIXME("PifMgr_OpenProperties() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
INT
WINAPI
PifMgr_GetProperties(HANDLE hHandle, LPWSTR lpName, LPVOID lpUnknown, INT iUnknown, UINT uUnknown)
{
    FIXME("PifMgr_GetProperties() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
INT
WINAPI
PifMgr_SetProperties(HANDLE hHandle, LPWSTR lpName, LPVOID lpUnknown, INT iUnknown, UINT uUnknown)
{
    FIXME("PifMgr_SetProperties() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHStartNetConnectionDialogA(HWND hwnd,
                            LPCSTR pszRemoteName,
                            DWORD dwType)
{
    FIXME("SHStartNetConnectionDialogA() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHStartNetConnectionDialogW(HWND hwnd,
                            LPCWSTR pszRemoteName,
                            DWORD dwType)
{
    FIXME("SHStartNetConnectionDialogW() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HANDLE
WINAPI
PifMgr_CloseProperties(HANDLE hHandle, UINT uUnknown)
{
    FIXME("PifMgr_CloseProperties() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
DAD_DragEnterEx2(HWND hwndTarget,
                 POINT ptStart,
                 IDataObject *pdtObject)
{
    FIXME("DAD_DragEnterEx2() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
IsSuspendAllowed(VOID)
{
    FIXME("IsSuspendAllowed() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
UINT
WINAPI
SHGetNetResource(LPVOID lpUnknown1, UINT iIndex, LPVOID lpUnknown2, UINT cchMax)
{
    FIXME("SHGetNetResource() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
DragQueryInfo(HDROP hDrop, DRAGINFO *pDragInfo)
{
    FIXME("DragQueryInfo() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
LPVOID
WINAPI
DDECreatePostNotify(LPVOID lpUnknown)
{
    FIXME("DDECreatePostNotify() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHIsBadInterfacePtr(LPVOID pv, UINT ucb)
{
    FIXME("SHIsBadInterfacePtr() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
Activate_RunDLL(DWORD dwProcessId, LPVOID lpUnused1, LPVOID lpUnused2, LPVOID lpUnused3)
{
    FIXME("Activate_RunDLL() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
VOID
WINAPI
AppCompat_RunDLLW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    FIXME("AppCompat_RunDLLW() stub\n");
}

/*
 * Unimplemented
 */
VOID
WINAPI
Control_RunDLLAsUserW(HWND hwnd, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    FIXME("Control_RunDLLAsUserW() stub\n");
}

/*
 * Unimplemented
 */
UINT
WINAPI
DragQueryFileAorW(HDROP hDrop, UINT iIndex, LPWSTR lpFile, UINT ucb, BOOL bUnicode, BOOL bShorten)
{
    FIXME("DragQueryFileAorW() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
DWORD
WINAPI
SHNetConnectionDialog(HWND hwndOwner,
                      LPCWSTR lpstrRemoteName,
                      DWORD dwType)
{
    FIXME("SHNetConnectionDialog() stub\n");
    return ERROR_INVALID_PARAMETER;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
DAD_SetDragImageFromListView(HWND hwnd, POINT pt)
{
    FIXME("DAD_SetDragImageFromListView() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
void
WINAPI
SHHandleDiskFull(HWND hwndOwner, UINT uDrive)
{
    FIXME("SHHandleDiskFull() stub\n");
}

/*
 * Unimplemented
 */
BOOL
WINAPI
ILGetPseudoNameW(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, LPWSTR szStr, INT iUnknown)
{
    /* Unimplemented in WinXP SP3 */
    TRACE("ILGetPseudoNameW() stub\n");
    *szStr = 0;
    return FALSE;
}

/*
 * Unimplemented
 */
VOID
WINAPI
SHGlobalDefect(DWORD dwUnknown)
{
    /* Unimplemented in WinXP SP3 */
    TRACE("SHGlobalDefect() stub\n");
}

/*
 * Unimplemented
 */
LPITEMIDLIST
WINAPI
Printers_GetPidl(LPCITEMIDLIST pidl, LPCWSTR lpName)
{
    FIXME("Printers_GetPidl() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
INT
WINAPI
Int64ToString(LONGLONG llInt64,
              LPWSTR lpOut,
              UINT uSize,
              BOOL bUseFormat,
              NUMBERFMT *pNumberFormat,
              DWORD dwNumberFlags)
{
    FIXME("Int64ToString() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
INT
WINAPI
LargeIntegerToString(LARGE_INTEGER *pLargeInt,
                     LPWSTR lpOut,
                     UINT uSize,
                     BOOL bUseFormat,
                     NUMBERFMT *pNumberFormat,
                     DWORD dwNumberFlags)
{
    FIXME("LargeIntegerToString() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
LONG
WINAPI
Printers_AddPrinterPropPages(LPVOID lpUnknown1, LPVOID lpUnknown2)
{
    FIXME("Printers_AddPrinterPropPages() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
WORD
WINAPI
ExtractIconResInfoA(HANDLE hHandle,
                    LPSTR lpFile,
                    WORD wIndex,
                    LPWORD lpSize,
                    LPHANDLE lpIcon)
{
    FIXME("ExtractIconResInfoA() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
WORD
WINAPI
ExtractIconResInfoW(HANDLE hHandle,
                    LPWSTR lpFile,
                    WORD wIndex,
                    LPWORD lpSize,
                    LPHANDLE lpIcon)
{
    FIXME("ExtractIconResInfoW() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
DWORD
WINAPI
ExtractVersionResource16W(LPWSTR lpName, LPHANDLE lpHandle)
{
    FIXME("ExtractVersionResource16W() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL*
WINAPI
FindExeDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    FIXME("FindExeDlgProc() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
HANDLE
WINAPI
InternalExtractIconListW(HANDLE hHandle,
                         LPWSTR lpFileName,
                         LPINT lpCount)
{
    FIXME("InternalExtractIconListW() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
HANDLE
WINAPI
InternalExtractIconListA(HANDLE hHandle,
                         LPSTR lpFileName,
                         LPINT lpCount)
{
    FIXME("InternalExtractIconListA() stub\n");
    return NULL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
FirstUserLogon(LPWSTR lpUnknown1, LPWSTR lpUnknown2)
{
    FIXME("FirstUserLogon() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHSetFolderPathA(int csidl,
                 HANDLE hToken,
                 DWORD dwFlags,
                 LPCSTR pszPath)
{
    FIXME("SHSetFolderPathA() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHSetFolderPathW(int csidl,
                 HANDLE hToken,
                 DWORD dwFlags,
                 LPCWSTR pszPath)
{
    FIXME("SHSetFolderPathW() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHGetUserPicturePathW(LPCWSTR lpPath, int csidl, LPVOID lpUnknown)
{
    FIXME("SHGetUserPicturePathW() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
HRESULT
WINAPI
SHSetUserPicturePathW(LPCWSTR lpPath, int csidl, LPVOID lpUnknown)
{
    FIXME("SHGetUserPicturePathA() stub\n");
    return E_FAIL;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHOpenEffectiveToken(LPVOID Token)
{
    FIXME("SHOpenEffectiveToken() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHTestTokenPrivilegeW(HANDLE hToken, LPDWORD ReturnLength)
{
    FIXME("SHTestTokenPrivilegeW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHShouldShowWizards(LPVOID lpUnknown)
{
    FIXME("SHShouldShowWizards() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
PathIsSlowW(LPCWSTR pszFile, DWORD dwFileAttr)
{
    FIXME("PathIsSlowW() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
PathIsSlowA(LPCSTR pszFile, DWORD dwFileAttr)
{
    FIXME("PathIsSlowA() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
DWORD
WINAPI
SHGetUserDisplayName(LPWSTR lpName, PULONG puSize)
{
    FIXME("SHGetUserDisplayName() stub\n");
    wcscpy(lpName, L"UserName");
    return ERROR_SUCCESS;
}

/*
 * Unimplemented
 */
DWORD
WINAPI
SHGetProcessDword(DWORD dwUnknown1, DWORD dwUnknown2)
{
    /* Unimplemented in WinXP SP3 */
    TRACE("SHGetProcessDword() stub\n");
    return 0;
}

/*
 * Unimplemented
 */
BOOL
WINAPI
SHTestTokenMembership(HANDLE TokenHandle, PSID SidToCheck)
{
    FIXME("SHTestTokenMembership() stub\n");
    return FALSE;
}

/*
 * Unimplemented
 */
LPVOID
WINAPI
SHGetUserSessionId(HANDLE hHandle)
{
    FIXME("SHGetUserSessionId() stub\n");
    return NULL;
}
