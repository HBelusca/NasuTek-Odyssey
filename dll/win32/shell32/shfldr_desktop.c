
/*
 *    Virtual Desktop Folder
 *
 *    Copyright 1997            Marcus Meissner
 *    Copyright 1998, 1999, 2002    Juergen Schmied
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <precomp.h>

WINE_DEFAULT_DEBUG_CHANNEL (shell);

/***********************************************************************
*     Desktopfolder implementation
*/

typedef struct {
    const IShellFolder2Vtbl *lpVtbl;
    const IPersistFolder2Vtbl *lpPF2;
    const ISFHelperVtbl       *lpvtblSFHelper;
    LONG ref;

    /* both paths are parsible from the desktop */
    LPWSTR sPathTarget;     /* complete path to target used for enumeration and ChangeNotify */
    LPITEMIDLIST pidlRoot;  /* absolute pidl */

    UINT cfShellIDList;        /* clipboardformat for IDropTarget */
    BOOL fAcceptFmt;        /* flag for pending Drop */
} IGenericSFImpl, *LPIGenericSFImpl;

WCHAR *build_paths_list(LPCWSTR wszBasePath, int cidl, LPCITEMIDLIST *pidls);
int SHELL_ConfirmMsgBox(HWND hWnd, LPWSTR lpszText, LPWSTR lpszCaption, HICON hIcon, BOOL bYesToAll);

#define _IUnknown_(This)    (IShellFolder*)&(This->lpVtbl)
#define _IShellFolder_(This)    (IShellFolder*)&(This->lpVtbl)

static const shvheader DesktopSFHeader[] = {
    {IDS_SHV_COLUMN1, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 15},
    {IDS_SHV_COLUMN2, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 10},
    {IDS_SHV_COLUMN3, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 10},
    {IDS_SHV_COLUMN4, SHCOLSTATE_TYPE_DATE | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 12},
    {IDS_SHV_COLUMN5, SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT, LVCFMT_RIGHT, 5}
};

#define DESKTOPSHELLVIEWCOLUMNS 5

/**************************************************************************
 *    ISF_Desktop_fnQueryInterface
 *
 * NOTES supports not IPersist/IPersistFolder
 */
static HRESULT WINAPI ISF_Desktop_fnQueryInterface(
                IShellFolder2 * iface, REFIID riid, LPVOID * ppvObj)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    TRACE ("(%p)->(%s,%p)\n", This, shdebugstr_guid (riid), ppvObj);

    *ppvObj = NULL;

    if (IsEqualIID (riid, &IID_IUnknown) ||
        IsEqualIID (riid, &IID_IShellFolder) ||
        IsEqualIID (riid, &IID_IShellFolder2))
    {
        *ppvObj = This;
    }
    else if (IsEqualIID (riid, &IID_IPersistFolder) ||
             IsEqualIID (riid, &IID_IPersistFolder2))
    {
        *ppvObj = (void *)&This->lpPF2;
    }
    else if (IsEqualIID(riid, &IID_ISFHelper))
    {
        *ppvObj = (void *)&This->lpvtblSFHelper;
    }
    if (*ppvObj)
    {
        IUnknown_AddRef ((IUnknown *) (*ppvObj));
        TRACE ("-- Interface: (%p)->(%p)\n", ppvObj, *ppvObj);
        return S_OK;
    }
    TRACE ("-- Interface: E_NOINTERFACE\n");
    return E_NOINTERFACE;
}

static LPIGenericSFImpl __inline impl_from_ISFHelper( ISFHelper *iface )
{
    return (IGenericSFImpl *)((char*)iface - FIELD_OFFSET(IGenericSFImpl, lpvtblSFHelper));
}

static ULONG WINAPI ISF_Desktop_fnAddRef (IShellFolder2 * iface)
{
    return 2; /* non-heap based object */
}

static ULONG WINAPI ISF_Desktop_fnRelease (IShellFolder2 * iface)
{
    return 1; /* non-heap based object */
}

/**************************************************************************
 *    ISF_Desktop_fnParseDisplayName
 *
 * NOTES
 *    "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}" and "" binds
 *    to MyComputer
 */
static HRESULT WINAPI ISF_Desktop_fnParseDisplayName (IShellFolder2 * iface,
                HWND hwndOwner, LPBC pbc, LPOLESTR lpszDisplayName,
                DWORD * pchEaten, LPITEMIDLIST * ppidl, DWORD * pdwAttributes)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    WCHAR szElement[MAX_PATH];
    LPCWSTR szNext = NULL;
    LPITEMIDLIST pidlTemp = NULL;
    HRESULT hr = S_OK;
    CLSID clsid;

    TRACE ("(%p)->(HWND=%p,%p,%p=%s,%p,pidl=%p,%p)\n",
           This, hwndOwner, pbc, lpszDisplayName, debugstr_w(lpszDisplayName),
           pchEaten, ppidl, pdwAttributes);

    if (!lpszDisplayName || !ppidl)
        return E_INVALIDARG;

    *ppidl = 0;

    if (pchEaten)
        *pchEaten = 0;        /* strange but like the original */

    if (lpszDisplayName[0] == ':' && lpszDisplayName[1] == ':')
    {
        szNext = GetNextElementW (lpszDisplayName, szElement, MAX_PATH);
        TRACE ("-- element: %s\n", debugstr_w (szElement));
        CLSIDFromString (szElement + 2, &clsid);
        pidlTemp = _ILCreateGuid (PT_GUID, &clsid);
    }
    else if (PathGetDriveNumberW (lpszDisplayName) >= 0)
    {
        /* it's a filesystem path with a drive. Let MyComputer/UnixDosFolder parse it */
#if 0
        if (UNIXFS_is_rooted_at_desktop())
            pidlTemp = _ILCreateGuid(PT_GUID, &CLSID_UnixDosFolder);
        else
#endif
            pidlTemp = _ILCreateMyComputer ();
        szNext = lpszDisplayName;
    }
    else if (PathIsUNCW(lpszDisplayName))
    {
        pidlTemp = _ILCreateNetwork();
        szNext = lpszDisplayName;
    }
    else if( (pidlTemp = SHELL32_CreatePidlFromBindCtx(pbc, lpszDisplayName)) )
    {
        *ppidl = pidlTemp;
        return S_OK;
    }
    else
    {
        /* it's a filesystem path on the desktop. Let a FSFolder parse it */

        if (*lpszDisplayName)
        {
            WCHAR szPath[MAX_PATH];
            LPWSTR pathPtr;

            /* build a complete path to create a simple pidl */
            lstrcpynW(szPath, This->sPathTarget, MAX_PATH);
            pathPtr = PathAddBackslashW(szPath);
            if (pathPtr)
            {
                lstrcpynW(pathPtr, lpszDisplayName, MAX_PATH - (pathPtr - szPath));
                hr = _ILCreateFromPathW(szPath, &pidlTemp);
            }
            else
            {
                /* should never reach here, but for completeness */
                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            }
        }
        else
            pidlTemp = _ILCreateMyComputer();

        szNext = NULL;
    }

    if (SUCCEEDED(hr) && pidlTemp)
    {
        if (szNext && *szNext)
        {
            hr = SHELL32_ParseNextElement(iface, hwndOwner, pbc,
                    &pidlTemp, (LPOLESTR) szNext, pchEaten, pdwAttributes);
        }
        else
        {
            if (pdwAttributes && *pdwAttributes)
                hr = SHELL32_GetItemAttributes(_IShellFolder_ (This),
                                               pidlTemp, pdwAttributes);
        }
    }

    *ppidl = pidlTemp;

    TRACE ("(%p)->(-- ret=0x%08x)\n", This, hr);

    return hr;
}

static const WCHAR ClassicStartMenuW[] =  {'S','O','F','T','W','A','R','E','\\',
 'M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\',
 'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','E','x','p','l','o','r','e','r',
 '\\','H','i','d','e','D','e','s','k','t','o','p','I','c','o','n','s','\\',
 'C','l','a','s','s','i','c','S','t','a','r','t','M','e','n','u','\0' };

INT
IsNamespaceExtensionHidden(WCHAR *iid)
{
    DWORD Result, dwResult;
    dwResult = sizeof(DWORD);

    if (RegGetValueW(HKEY_CURRENT_USER, /* FIXME use NewStartPanel when activated */
                     ClassicStartMenuW,
                     iid,
                     RRF_RT_DWORD,
                     NULL,
                     &Result,
                     &dwResult) != ERROR_SUCCESS)
    {
        return -1;     
    }
    
    return Result;
}

static
VOID
SetNamespaceExtensionVisibleStatus(WCHAR * iid, DWORD dwStatus)
{
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, ClassicStartMenuW, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueExW(hKey, iid, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(DWORD));
        RegCloseKey(hKey);
    }
}



/**************************************************************************
 *  CreateDesktopEnumList()
 */
static const WCHAR Desktop_NameSpaceW[] = { 'S','O','F','T','W','A','R','E',
 '\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\',
 'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\','E','x','p','l',
 'o','r','e','r','\\','D','e','s','k','t','o','p','\\','N','a','m','e','s','p',
 'a','c','e','\0' };

static BOOL CreateDesktopEnumList(IEnumIDList *list, DWORD dwFlags)
{
    BOOL ret = TRUE;
    WCHAR szPath[MAX_PATH];

    static WCHAR MyDocumentsClassString[] = L"{450D8FBA-AD25-11D0-98A8-0800361B1103}";


    TRACE("(%p)->(flags=0x%08x)\n", list, dwFlags);

    /* enumerate the root folders */
    if (dwFlags & SHCONTF_FOLDERS)
    {
        HKEY hkey;
        UINT i;
        DWORD dwResult;

        /* create the pidl for This item */
        if (IsNamespaceExtensionHidden(MyDocumentsClassString) < 1)
        {
            ret = AddToEnumList(list, _ILCreateMyDocuments());
        }
        ret = AddToEnumList(list, _ILCreateMyComputer());

        for (i = 0; i < 2; i++)
        {
            if (i == 0)
                dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, Desktop_NameSpaceW, 0, KEY_READ, &hkey);
            else 
                dwResult = RegOpenKeyExW(HKEY_CURRENT_USER, Desktop_NameSpaceW, 0, KEY_READ, &hkey);

            if (dwResult == ERROR_SUCCESS)
            {
                WCHAR iid[50];
                LPITEMIDLIST pidl;
                int i=0;

                while (ret)
                {
                    DWORD size;
                    LONG r;

                    size = sizeof (iid) / sizeof (iid[0]);
                    r = RegEnumKeyExW(hkey, i, iid, &size, 0, NULL, NULL, NULL);
                    if (ERROR_SUCCESS == r)
                    {
                        if (IsNamespaceExtensionHidden(iid) < 1)
                        {
                           pidl = _ILCreateGuidFromStrW(iid);
                           if (!HasItemWithCLSID(list, pidl))
                           {
                               ret = AddToEnumList(list, pidl);
                           }
                           else
                           {
                                SHFree(pidl);
                           }
                       }
                    }
                    else if (ERROR_NO_MORE_ITEMS == r)
                        break;
                    else
                        ret = FALSE;
                    i++;
                }
                RegCloseKey(hkey);
            }
        }
        for (i = 0; i < 2; i++)
        {
            if (i == 0)
                dwResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, ClassicStartMenuW, 0, KEY_READ, &hkey);
            else 
                dwResult = RegOpenKeyExW(HKEY_CURRENT_USER, ClassicStartMenuW, 0, KEY_READ, &hkey);

            if (dwResult == ERROR_SUCCESS)
            {
                DWORD j = 0, dwVal, Val, dwType, dwIID;
                LONG r;
                WCHAR iid[50];

                while(ret)
                {
                    dwVal = sizeof(Val);
                    dwIID = sizeof(iid) / sizeof(WCHAR);

                    r = RegEnumValueW(hkey, j++, iid, &dwIID, NULL, &dwType, (LPBYTE)&Val, &dwVal);
                    if (r == ERROR_SUCCESS)
                    {
                        if (Val == 0 && dwType == REG_DWORD)
                        {
                            LPITEMIDLIST pidl = _ILCreateGuidFromStrW(iid);
                            if (!HasItemWithCLSID(list, pidl))
                            {
                               AddToEnumList(list, pidl);
                            }
                            else
                            {
                                SHFree(pidl);
                            }
                        }
                    }
                    else if (ERROR_NO_MORE_ITEMS == r)
                        break;
                    else
                        ret = FALSE;
                }
            }

        }
    }

    /* enumerate the elements in %windir%\desktop */
    ret = ret && SHGetSpecialFolderPathW(0, szPath, CSIDL_DESKTOPDIRECTORY, FALSE);
    ret = ret && CreateFolderEnumList(list, szPath, dwFlags);

    ret = ret && SHGetSpecialFolderPathW(0, szPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
    ret = ret && CreateFolderEnumList(list, szPath, dwFlags);

    return ret;
}

/**************************************************************************
 *        ISF_Desktop_fnEnumObjects
 */
static HRESULT WINAPI ISF_Desktop_fnEnumObjects (IShellFolder2 * iface,
                HWND hwndOwner, DWORD dwFlags, LPENUMIDLIST * ppEnumIDList)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    TRACE ("(%p)->(HWND=%p flags=0x%08x pplist=%p)\n",
           This, hwndOwner, dwFlags, ppEnumIDList);

    *ppEnumIDList = IEnumIDList_Constructor();
    if (*ppEnumIDList)
        CreateDesktopEnumList(*ppEnumIDList, dwFlags);

    TRACE ("-- (%p)->(new ID List: %p)\n", This, *ppEnumIDList);

    return *ppEnumIDList ? S_OK : E_OUTOFMEMORY;
}

/**************************************************************************
 *        ISF_Desktop_fnBindToObject
 */
static HRESULT WINAPI ISF_Desktop_fnBindToObject (IShellFolder2 * iface,
                LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvOut)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    TRACE ("(%p)->(pidl=%p,%p,%s,%p)\n",
           This, pidl, pbcReserved, shdebugstr_guid (riid), ppvOut);

    return SHELL32_BindToChild( This->pidlRoot, This->sPathTarget, pidl, riid, ppvOut );
}

/**************************************************************************
 *    ISF_Desktop_fnBindToStorage
 */
static HRESULT WINAPI ISF_Desktop_fnBindToStorage (IShellFolder2 * iface,
                LPCITEMIDLIST pidl, LPBC pbcReserved, REFIID riid, LPVOID * ppvOut)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    FIXME ("(%p)->(pidl=%p,%p,%s,%p) stub\n",
           This, pidl, pbcReserved, shdebugstr_guid (riid), ppvOut);

    *ppvOut = NULL;
    return E_NOTIMPL;
}

/**************************************************************************
 *     ISF_Desktop_fnCompareIDs
 */
static HRESULT WINAPI ISF_Desktop_fnCompareIDs (IShellFolder2 * iface,
                        LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    int nReturn;

    TRACE ("(%p)->(0x%08lx,pidl1=%p,pidl2=%p)\n", This, lParam, pidl1, pidl2);
    nReturn = SHELL32_CompareIDs (_IShellFolder_ (This), lParam, pidl1, pidl2);
    TRACE ("-- %i\n", nReturn);
    return nReturn;
}

/**************************************************************************
 *    ISF_Desktop_fnCreateViewObject
 */
static HRESULT WINAPI ISF_Desktop_fnCreateViewObject (IShellFolder2 * iface,
                              HWND hwndOwner, REFIID riid, LPVOID * ppvOut)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    LPSHELLVIEW pShellView;
    HRESULT hr = E_INVALIDARG;

    TRACE ("(%p)->(hwnd=%p,%s,%p)\n",
           This, hwndOwner, shdebugstr_guid (riid), ppvOut);

    if (!ppvOut)
        return hr;

    *ppvOut = NULL;

    if (IsEqualIID (riid, &IID_IDropTarget))
    {
        WARN ("IDropTarget not implemented\n");
        hr = E_NOTIMPL;
    }
    else if (IsEqualIID (riid, &IID_IContextMenu))
    {
        WARN ("IContextMenu not implemented\n");
        hr = E_NOTIMPL;
    }
    else if (IsEqualIID (riid, &IID_IShellView))
    {
        pShellView = IShellView_Constructor ((IShellFolder *) iface);
        if (pShellView)
        {
            hr = IShellView_QueryInterface (pShellView, riid, ppvOut);
            IShellView_Release (pShellView);
        }
    }
    TRACE ("-- (%p)->(interface=%p)\n", This, ppvOut);
    return hr;
}

/**************************************************************************
 *  ISF_Desktop_fnGetAttributesOf
 */
static HRESULT WINAPI ISF_Desktop_fnGetAttributesOf (IShellFolder2 * iface,
                UINT cidl, LPCITEMIDLIST * apidl, DWORD * rgfInOut)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    HRESULT hr = S_OK;
    static const DWORD dwDesktopAttributes =
        SFGAO_STORAGE | SFGAO_HASPROPSHEET | SFGAO_STORAGEANCESTOR |
        SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER;
    static const DWORD dwMyComputerAttributes =
        SFGAO_CANRENAME | SFGAO_CANDELETE | SFGAO_HASPROPSHEET |
        SFGAO_DROPTARGET | SFGAO_FILESYSANCESTOR | SFGAO_FOLDER | SFGAO_HASSUBFOLDER;

    TRACE ("(%p)->(cidl=%d apidl=%p mask=%p (0x%08x))\n",
           This, cidl, apidl, rgfInOut, rgfInOut ? *rgfInOut : 0);

    if (!rgfInOut)
        return E_INVALIDARG;
    if (cidl && !apidl)
        return E_INVALIDARG;

    if (*rgfInOut == 0)
        *rgfInOut = ~0;

    if(cidl == 0) {
        *rgfInOut &= dwDesktopAttributes;
    } else {
        while (cidl > 0 && *apidl) {
            pdump (*apidl);
            if (_ILIsDesktop(*apidl)) {
                *rgfInOut &= dwDesktopAttributes;
            } else if (_ILIsMyComputer(*apidl)) {
                *rgfInOut &= dwMyComputerAttributes;
            } else {
                SHELL32_GetItemAttributes (_IShellFolder_ (This), *apidl, rgfInOut);
            }
            apidl++;
            cidl--;
        }
    }
    /* make sure SFGAO_VALIDATE is cleared, some apps depend on that */
    *rgfInOut &= ~SFGAO_VALIDATE;

    TRACE ("-- result=0x%08x\n", *rgfInOut);

    return hr;
}

/**************************************************************************
 *    ISF_Desktop_fnGetUIObjectOf
 *
 * PARAMETERS
 *  HWND           hwndOwner, //[in ] Parent window for any output
 *  UINT           cidl,      //[in ] array size
 *  LPCITEMIDLIST* apidl,     //[in ] simple pidl array
 *  REFIID         riid,      //[in ] Requested Interface
 *  UINT*          prgfInOut, //[   ] reserved
 *  LPVOID*        ppvObject) //[out] Resulting Interface
 *
 */
static HRESULT WINAPI ISF_Desktop_fnGetUIObjectOf (IShellFolder2 * iface,
                HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                REFIID riid, UINT * prgfInOut, LPVOID * ppvOut)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    LPITEMIDLIST pidl;
    IUnknown *pObj = NULL;
    HRESULT hr = E_INVALIDARG;

    TRACE ("(%p)->(%p,%u,apidl=%p,%s,%p,%p)\n",
       This, hwndOwner, cidl, apidl, shdebugstr_guid (riid), prgfInOut, ppvOut);

    if (!ppvOut)
        return hr;

    *ppvOut = NULL;

    if (IsEqualIID (riid, &IID_IContextMenu))
    {
        hr = CDefFolderMenu_Create2(This->pidlRoot, hwndOwner, cidl, apidl, (IShellFolder*)iface, NULL, 0, NULL, (IContextMenu**)&pObj);
    }
    else if (IsEqualIID (riid, &IID_IDataObject) && (cidl >= 1))
    {
        pObj = (LPUNKNOWN) IDataObject_Constructor( hwndOwner,
                                                  This->pidlRoot, apidl, cidl);
        hr = S_OK;
    }
    else if (IsEqualIID (riid, &IID_IExtractIconA) && (cidl == 1))
    {
        pidl = ILCombine (This->pidlRoot, apidl[0]);
        pObj = (LPUNKNOWN) IExtractIconA_Constructor (pidl);
        SHFree (pidl);
        hr = S_OK;
    }
    else if (IsEqualIID (riid, &IID_IExtractIconW) && (cidl == 1))
    {
        pidl = ILCombine (This->pidlRoot, apidl[0]);
        pObj = (LPUNKNOWN) IExtractIconW_Constructor (pidl);
        SHFree (pidl);
        hr = S_OK;
    }
    else if (IsEqualIID (riid, &IID_IDropTarget) && (cidl >= 1))
    {
        hr = IShellFolder_QueryInterface (iface,
                                          &IID_IDropTarget, (LPVOID *) & pObj);
    }
    else if ((IsEqualIID(riid,&IID_IShellLinkW) ||
              IsEqualIID(riid,&IID_IShellLinkA)) && (cidl == 1))
    {
        pidl = ILCombine (This->pidlRoot, apidl[0]);
        hr = IShellLink_ConstructFromFile(NULL, riid, pidl, (LPVOID*)&pObj);
        SHFree (pidl);
    }
    else
        hr = E_NOINTERFACE;

    if (SUCCEEDED(hr) && !pObj)
        hr = E_OUTOFMEMORY;

    *ppvOut = pObj;
    TRACE ("(%p)->hr=0x%08x\n", This, hr);
    return hr;
}

/**************************************************************************
 *    ISF_Desktop_fnGetDisplayNameOf
 *
 * NOTES
 *    special case: pidl = null gives desktop-name back
 */
static HRESULT WINAPI ISF_Desktop_fnGetDisplayNameOf (IShellFolder2 * iface,
                LPCITEMIDLIST pidl, DWORD dwFlags, LPSTRRET strRet)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    HRESULT hr = S_OK;
    LPWSTR pszPath;

    TRACE ("(%p)->(pidl=%p,0x%08x,%p)\n", This, pidl, dwFlags, strRet);
    pdump (pidl);

    if (!strRet)
        return E_INVALIDARG;

    pszPath = CoTaskMemAlloc((MAX_PATH +1) * sizeof(WCHAR));
    if (!pszPath)
        return E_OUTOFMEMORY;

    if (_ILIsDesktop (pidl))
    {
        if ((GET_SHGDN_RELATION (dwFlags) == SHGDN_NORMAL) &&
            (GET_SHGDN_FOR (dwFlags) & SHGDN_FORPARSING))
            wcscpy(pszPath, This->sPathTarget);
        else
            HCR_GetClassNameW(&CLSID_ShellDesktop, pszPath, MAX_PATH);
    }
    else if (_ILIsPidlSimple (pidl))
    {
        GUID const *clsid;

        if ((clsid = _ILGetGUIDPointer (pidl)))
        {
            if (GET_SHGDN_FOR (dwFlags) & SHGDN_FORPARSING)
            {
                int bWantsForParsing;

                /*
                 * We can only get a filesystem path from a shellfolder if the
                 *  value WantsFORPARSING in CLSID\\{...}\\shellfolder exists.
                 *
                 * Exception: The MyComputer folder doesn't have this key,
                 *   but any other filesystem backed folder it needs it.
                 */
                if (IsEqualIID (clsid, &CLSID_MyComputer))
                {
                    bWantsForParsing = TRUE;
                }
                else
                {
                    /* get the "WantsFORPARSING" flag from the registry */
                    static const WCHAR clsidW[] =
                     { 'C','L','S','I','D','\\',0 };
                    static const WCHAR shellfolderW[] =
                     { '\\','s','h','e','l','l','f','o','l','d','e','r',0 };
                    static const WCHAR wantsForParsingW[] =
                     { 'W','a','n','t','s','F','o','r','P','a','r','s','i','n',
                     'g',0 };
                    WCHAR szRegPath[100];
                    LONG r;

                    wcscpy (szRegPath, clsidW);
                    SHELL32_GUIDToStringW (clsid, &szRegPath[6]);
                    wcscat (szRegPath, shellfolderW);
                    r = SHGetValueW(HKEY_CLASSES_ROOT, szRegPath,
                                    wantsForParsingW, NULL, NULL, NULL);
                    if (r == ERROR_SUCCESS)
                        bWantsForParsing = TRUE;
                    else
                        bWantsForParsing = FALSE;
                }

                if ((GET_SHGDN_RELATION (dwFlags) == SHGDN_NORMAL) &&
                     bWantsForParsing)
                {
                    /*
                     * we need the filesystem path to the destination folder.
                     * Only the folder itself can know it
                     */
                    hr = SHELL32_GetDisplayNameOfChild (iface, pidl, dwFlags,
                                                        pszPath,
                                                        MAX_PATH);
                }
                else
                {
                    /* parsing name like ::{...} */
                    pszPath[0] = ':';
                    pszPath[1] = ':';
                    SHELL32_GUIDToStringW (clsid, &pszPath[2]);
                }
            }
            else
            {
                /* user friendly name */
                HCR_GetClassNameW (clsid, pszPath, MAX_PATH);
            }
        }
        else
        {
            int cLen = 0;

            /* file system folder or file rooted at the desktop */
            if ((GET_SHGDN_FOR(dwFlags) == SHGDN_FORPARSING) &&
                (GET_SHGDN_RELATION(dwFlags) != SHGDN_INFOLDER))
            {
                lstrcpynW(pszPath, This->sPathTarget, MAX_PATH - 1);
                PathAddBackslashW(pszPath);
                cLen = wcslen(pszPath);
            }

            _ILSimpleGetTextW(pidl, pszPath + cLen, MAX_PATH - cLen);
            if (!_ILIsFolder(pidl))
                SHELL_FS_ProcessDisplayFilename(pszPath, dwFlags);

            if (GetFileAttributes(pszPath) == INVALID_FILE_ATTRIBUTES)
            {
                /* file system folder or file rooted at the AllUsers desktop */
                if ((GET_SHGDN_FOR(dwFlags) == SHGDN_FORPARSING) &&
                    (GET_SHGDN_RELATION(dwFlags) != SHGDN_INFOLDER))
                {
                    SHGetSpecialFolderPathW(0, pszPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE);
                    PathAddBackslashW(pszPath);
                    cLen = wcslen(pszPath);
                }

                _ILSimpleGetTextW(pidl, pszPath + cLen, MAX_PATH - cLen);
                if (!_ILIsFolder(pidl))
                    SHELL_FS_ProcessDisplayFilename(pszPath, dwFlags);
            }
        }
    }
    else
    {
        /* a complex pidl, let the subfolder do the work */
        hr = SHELL32_GetDisplayNameOfChild (iface, pidl, dwFlags,
                                            pszPath, MAX_PATH);
    }

    if (SUCCEEDED(hr))
    {
        /* Win9x always returns ANSI strings, NT always returns Unicode strings */
        if (GetVersion() & 0x80000000)
        {
            strRet->uType = STRRET_CSTR;
            if (!WideCharToMultiByte(CP_ACP, 0, pszPath, -1, strRet->u.cStr, MAX_PATH,
                                     NULL, NULL))
                strRet->u.cStr[0] = '\0';
            CoTaskMemFree(pszPath);
        }
        else
        {
            strRet->uType = STRRET_WSTR;
            strRet->u.pOleStr = pszPath;
        }
    }
    else
        CoTaskMemFree(pszPath);

    TRACE ("-- (%p)->(%s,0x%08x)\n", This,
     strRet->uType == STRRET_CSTR ? strRet->u.cStr :
     debugstr_w(strRet->u.pOleStr), hr);
    return hr;
}

/**************************************************************************
 *  ISF_Desktop_fnSetNameOf
 *  Changes the name of a file object or subfolder, possibly changing its item
 *  identifier in the process.
 *
 * PARAMETERS
 *  HWND          hwndOwner,  //[in ] Owner window for output
 *  LPCITEMIDLIST pidl,       //[in ] simple pidl of item to change
 *  LPCOLESTR     lpszName,   //[in ] the items new display name
 *  DWORD         dwFlags,    //[in ] SHGNO formatting flags
 *  LPITEMIDLIST* ppidlOut)   //[out] simple pidl returned
 */
static HRESULT WINAPI ISF_Desktop_fnSetNameOf (IShellFolder2 * iface,
                HWND hwndOwner, LPCITEMIDLIST pidl,    /* simple pidl */
                LPCOLESTR lpName, DWORD dwFlags, LPITEMIDLIST * pPidlOut)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    IShellFolder2 * psf;
    HRESULT hr;
    WCHAR szSrc[MAX_PATH + 1], szDest[MAX_PATH + 1];
    LPWSTR ptr;
    BOOL bIsFolder = _ILIsFolder (ILFindLastID (pidl));

    TRACE ("(%p)->(%p,pidl=%p,%s,%u,%p)\n", This, hwndOwner, pidl,
           debugstr_w (lpName), dwFlags, pPidlOut);

    if (_ILGetGUIDPointer(pidl))
    {
        if (SUCCEEDED(IShellFolder2_BindToObject(iface, pidl, NULL, &IID_IShellFolder2, (LPVOID*)&psf)))
        {
            hr = IShellFolder2_SetNameOf(psf, hwndOwner, pidl, lpName, dwFlags, pPidlOut);
            IShellFolder2_Release(psf);
            return hr;
        }
    }

    /* build source path */
    lstrcpynW(szSrc, This->sPathTarget, MAX_PATH);
    ptr = PathAddBackslashW (szSrc);
    if (ptr)
        _ILSimpleGetTextW (pidl, ptr, MAX_PATH + 1 - (ptr - szSrc));

    /* build destination path */
    if (dwFlags == SHGDN_NORMAL || dwFlags & SHGDN_INFOLDER) {
        lstrcpynW(szDest, This->sPathTarget, MAX_PATH);
        ptr = PathAddBackslashW (szDest);
        if (ptr)
            lstrcpynW(ptr, lpName, MAX_PATH + 1 - (ptr - szDest));
    } else
        lstrcpynW(szDest, lpName, MAX_PATH);

    if(!(dwFlags & SHGDN_FORPARSING) && SHELL_FS_HideExtension(szSrc)) {
        WCHAR *ext = PathFindExtensionW(szSrc);
        if(*ext != '\0') {
            INT len = wcslen(szDest);
            lstrcpynW(szDest + len, ext, MAX_PATH - len);
        }
    }

    if (!memcmp(szSrc, szDest, (wcslen(szDest)+1) * sizeof(WCHAR)))
    {
        /* src and destination is the same */
        hr = S_OK;
        if (pPidlOut)
            hr = _ILCreateFromPathW(szDest, pPidlOut);

        return hr;
    }

    TRACE ("src=%s dest=%s\n", debugstr_w(szSrc), debugstr_w(szDest));
    if (MoveFileW (szSrc, szDest))
    {
        hr = S_OK;

        if (pPidlOut)
            hr = _ILCreateFromPathW(szDest, pPidlOut);

        SHChangeNotify (bIsFolder ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM,
         SHCNF_PATHW, szSrc, szDest);

        return hr;
    }
    return E_FAIL;
}
static HRESULT WINAPI ISF_Desktop_fnGetDefaultSearchGUID(IShellFolder2 *iface,
                GUID * pguid)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    FIXME ("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI ISF_Desktop_fnEnumSearches (IShellFolder2 *iface,
                IEnumExtraSearch ** ppenum)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    FIXME ("(%p)\n", This);
    return E_NOTIMPL;
}

static HRESULT WINAPI ISF_Desktop_fnGetDefaultColumn (IShellFolder2 * iface,
                DWORD dwRes, ULONG * pSort, ULONG * pDisplay)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    TRACE ("(%p)\n", This);

    if (pSort)
        *pSort = 0;
    if (pDisplay)
        *pDisplay = 0;

    return S_OK;
}
static HRESULT WINAPI ISF_Desktop_fnGetDefaultColumnState (
                IShellFolder2 * iface, UINT iColumn, DWORD * pcsFlags)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    TRACE ("(%p)\n", This);

    if (!pcsFlags || iColumn >= DESKTOPSHELLVIEWCOLUMNS)
    return E_INVALIDARG;

    *pcsFlags = DesktopSFHeader[iColumn].pcsFlags;

    return S_OK;
}

static HRESULT WINAPI ISF_Desktop_fnGetDetailsEx (IShellFolder2 * iface,
                LPCITEMIDLIST pidl, const SHCOLUMNID * pscid, VARIANT * pv)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    FIXME ("(%p)\n", This);

    return E_NOTIMPL;
}

static HRESULT WINAPI ISF_Desktop_fnGetDetailsOf (IShellFolder2 * iface,
                LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS * psd)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;

    HRESULT hr = S_OK;

    TRACE ("(%p)->(%p %i %p)\n", This, pidl, iColumn, psd);

    if (!psd || iColumn >= DESKTOPSHELLVIEWCOLUMNS)
        return E_INVALIDARG;

    if (!pidl)
    {
        psd->fmt = DesktopSFHeader[iColumn].fmt;
        psd->cxChar = DesktopSFHeader[iColumn].cxChar;
        psd->str.uType = STRRET_CSTR;
        LoadStringA (shell32_hInstance, DesktopSFHeader[iColumn].colnameid,
                     psd->str.u.cStr, MAX_PATH);
        return S_OK;
    }

    /* the data from the pidl */
    psd->str.uType = STRRET_CSTR;
    switch (iColumn)
    {
    case 0:        /* name */
        hr = IShellFolder_GetDisplayNameOf(iface, pidl,
                   SHGDN_NORMAL | SHGDN_INFOLDER, &psd->str);
        break;
    case 1:        /* size */
        _ILGetFileSize (pidl, psd->str.u.cStr, MAX_PATH);
        break;
    case 2:        /* type */
        _ILGetFileType (pidl, psd->str.u.cStr, MAX_PATH);
        break;
    case 3:        /* date */
        _ILGetFileDate (pidl, psd->str.u.cStr, MAX_PATH);
        break;
    case 4:        /* attributes */
        _ILGetFileAttributes (pidl, psd->str.u.cStr, MAX_PATH);
        break;
    }

    return hr;
}

static HRESULT WINAPI ISF_Desktop_fnMapColumnToSCID (
                IShellFolder2 * iface, UINT column, SHCOLUMNID * pscid)
{
    IGenericSFImpl *This = (IGenericSFImpl *)iface;
    FIXME ("(%p)\n", This);
    return E_NOTIMPL;
}

static const IShellFolder2Vtbl vt_MCFldr_ShellFolder2 =
{
    ISF_Desktop_fnQueryInterface,
    ISF_Desktop_fnAddRef,
    ISF_Desktop_fnRelease,
    ISF_Desktop_fnParseDisplayName,
    ISF_Desktop_fnEnumObjects,
    ISF_Desktop_fnBindToObject,
    ISF_Desktop_fnBindToStorage,
    ISF_Desktop_fnCompareIDs,
    ISF_Desktop_fnCreateViewObject,
    ISF_Desktop_fnGetAttributesOf,
    ISF_Desktop_fnGetUIObjectOf,
    ISF_Desktop_fnGetDisplayNameOf,
    ISF_Desktop_fnSetNameOf,
    /* ShellFolder2 */
    ISF_Desktop_fnGetDefaultSearchGUID,
    ISF_Desktop_fnEnumSearches,
    ISF_Desktop_fnGetDefaultColumn,
    ISF_Desktop_fnGetDefaultColumnState,
    ISF_Desktop_fnGetDetailsEx,
    ISF_Desktop_fnGetDetailsOf,
    ISF_Desktop_fnMapColumnToSCID
};

static LPIGenericSFImpl __inline impl_from_IPersistFolder2( IPersistFolder2 *iface )
{
    return (IGenericSFImpl *)((char*)iface - FIELD_OFFSET(IGenericSFImpl, lpPF2));
}

static HRESULT WINAPI
ISF_Desktop_PersistFolder2_fnQueryInterface (IPersistFolder2 * iface, REFIID iid,
                                       LPVOID * ppvObj)
{
    IGenericSFImpl *This = impl_from_IPersistFolder2(iface);

    TRACE ("(%p)\n", This);

    return ISF_Desktop_fnQueryInterface ((IShellFolder2*)This, iid, ppvObj);
}

static ULONG WINAPI
ISF_Desktop_PersistFolder2_fnAddRef (IPersistFolder2 * iface)
{
    IGenericSFImpl *This = impl_from_IPersistFolder2(iface);

    TRACE ("(%p)->(count=%u)\n", This, This->ref);

    return ISF_Desktop_fnAddRef((IShellFolder2*)This);
}

static ULONG WINAPI
ISF_Desktop_PersistFolder2_fnRelease (IPersistFolder2 * iface)
{
    IGenericSFImpl *This = impl_from_IPersistFolder2(iface);

    TRACE ("(%p)->(count=%u)\n", This, This->ref);

    return ISF_Desktop_fnRelease ((IShellFolder2*)This);
}

static HRESULT WINAPI
ISF_Desktop_PersistFolder2_fnGetClassID (IPersistFolder2 * iface, CLSID * lpClassId)
{
    static GUID const CLSID_Desktop = 
    { 0x00021400, 0x0000, 0x0000, {0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };

    IGenericSFImpl *This = impl_from_IPersistFolder2(iface);

    TRACE ("(%p)\n", This);

    if (!lpClassId)
        return E_POINTER;

    memcpy(lpClassId, &CLSID_Desktop, sizeof(GUID));

    return S_OK;
}
static HRESULT WINAPI
ISF_Desktop_PersistFolder2_fnInitialize (IPersistFolder2 * iface, LPCITEMIDLIST pidl)
{
    IGenericSFImpl *This = impl_from_IPersistFolder2(iface);

    TRACE ("(%p)->(%p)\n", This, pidl);

    return E_NOTIMPL;
}

static HRESULT WINAPI
ISF_Desktop_PersistFolder2_fnGetCurFolder (IPersistFolder2 * iface,
                                       LPITEMIDLIST * pidl)
{
    IGenericSFImpl *This = impl_from_IPersistFolder2(iface);

    TRACE ("(%p)->(%p)\n", This, pidl);

    if (!pidl) return E_POINTER;
    *pidl = ILClone (This->pidlRoot);
    return S_OK;
}

static const IPersistFolder2Vtbl vt_FSFldr_PersistFolder2 =
{
    ISF_Desktop_PersistFolder2_fnQueryInterface,
    ISF_Desktop_PersistFolder2_fnAddRef,
    ISF_Desktop_PersistFolder2_fnRelease,
    ISF_Desktop_PersistFolder2_fnGetClassID,
    ISF_Desktop_PersistFolder2_fnInitialize,
    ISF_Desktop_PersistFolder2_fnGetCurFolder,
};

static HRESULT WINAPI
ISF_Desktop_ISFHelper_fnQueryInterface (ISFHelper * iface, REFIID riid, LPVOID * ppvObj)
{
    IGenericSFImpl *This = impl_from_ISFHelper(iface);

    TRACE ("(%p)->(count=%u)\n", This, This->ref);

    return ISF_Desktop_fnQueryInterface ((IShellFolder2*)This, riid, ppvObj);
}

static ULONG WINAPI 
ISF_Desktop_ISFHelper_fnAddRef (ISFHelper * iface)
{
    IGenericSFImpl *This = impl_from_ISFHelper(iface);

    TRACE ("(%p)->(count=%u)\n", This, This->ref);

    return ISF_Desktop_fnAddRef((IShellFolder2*)This);
}

static ULONG WINAPI
ISF_Desktop_ISFHelper_fnRelease (ISFHelper * iface)
{
    IGenericSFImpl *This = impl_from_ISFHelper(iface);

    TRACE ("(%p)\n", This);

    return ISF_Desktop_fnRelease ((IShellFolder2*)This);
}

static HRESULT WINAPI
ISF_Desktop_ISFHelper_fnGetUniqueName (ISFHelper * iface, LPWSTR pwszName, UINT uLen)
{
    IGenericSFImpl *This = impl_from_ISFHelper(iface);
    IEnumIDList *penum;
    HRESULT hr;
    WCHAR wszText[MAX_PATH];
    WCHAR wszNewFolder[25];
    const WCHAR wszFormat[] = {'%','s',' ','%','d',0 };

    LoadStringW(shell32_hInstance, IDS_NEWFOLDER, wszNewFolder,  sizeof(wszNewFolder)/sizeof(WCHAR));

    TRACE ("(%p)(%p %u)\n", This, pwszName, uLen);

    if (uLen < sizeof(wszNewFolder)/sizeof(WCHAR) + 3)
        return E_POINTER;

    lstrcpynW (pwszName, wszNewFolder, uLen);

    hr = IShellFolder_EnumObjects ((IShellFolder2*)This, 0,
     SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &penum);
    if (penum) {
        LPITEMIDLIST pidl;
        DWORD dwFetched;
        int i = 1;

next:
        IEnumIDList_Reset (penum);
        while (S_OK == IEnumIDList_Next (penum, 1, &pidl, &dwFetched) &&
         dwFetched) {
            _ILSimpleGetTextW (pidl, wszText, MAX_PATH);
            if (0 == lstrcmpiW (wszText, pwszName)) {
                _snwprintf (pwszName, uLen, wszFormat, wszNewFolder, i++);
                if (i > 99) {
                    hr = E_FAIL;
                    break;
                }
                goto next;
            }
        }

        IEnumIDList_Release (penum);
    }
    return hr;
}

static HRESULT WINAPI
ISF_Desktop_ISFHelper_fnAddFolder (ISFHelper * iface, HWND hwnd, LPCWSTR pwszName,
                       LPITEMIDLIST * ppidlOut)
{
    IGenericSFImpl *This = impl_from_ISFHelper(iface);
    WCHAR wszNewDir[MAX_PATH];
    DWORD bRes;
    HRESULT hres = E_FAIL;

    TRACE ("(%p)(%s %p)\n", This, debugstr_w(pwszName), ppidlOut);

    wszNewDir[0] = 0;
    if (This->sPathTarget)
        lstrcpynW(wszNewDir, This->sPathTarget, MAX_PATH);
    PathAppendW(wszNewDir, pwszName);
    bRes = CreateDirectoryW (wszNewDir, NULL);
    if (bRes) 
    {
        SHChangeNotify (SHCNE_MKDIR, SHCNF_PATHW, wszNewDir, NULL);
        hres = S_OK;
        if (ppidlOut)
                hres = _ILCreateFromPathW(wszNewDir, ppidlOut);
    }

    return hres;
}
static HRESULT WINAPI
ISF_Desktop_ISFHelper_fnDeleteItems (ISFHelper * iface, UINT cidl, LPCITEMIDLIST * apidl)
{
    IGenericSFImpl *This = impl_from_ISFHelper(iface);
    UINT i;
    SHFILEOPSTRUCTW op;
    WCHAR wszPath[MAX_PATH];
    WCHAR wszCaption[50];
    WCHAR *wszPathsList;
    HRESULT ret;
    WCHAR *wszCurrentPath;
    UINT bRestoreWithDeskCpl = FALSE;
    int res;

    TRACE ("(%p)(%u %p)\n", This, cidl, apidl);
    if (cidl==0) return S_OK;

    for(i = 0; i < cidl; i++)
    {
        if (_ILIsMyComputer(apidl[i]))
            bRestoreWithDeskCpl++;
        else if (_ILIsNetHood(apidl[i]))
            bRestoreWithDeskCpl++;
        else if (_ILIsMyDocuments(apidl[i]))
            bRestoreWithDeskCpl++;
    }

    if (bRestoreWithDeskCpl)
    {
        /* FIXME use FormatMessage
         * use a similar message resource as in windows
         */
        LoadStringW(shell32_hInstance, IDS_DELETEMULTIPLE_TEXT, wszPath, sizeof(wszPath)/sizeof(WCHAR));
        wszPath[(sizeof(wszPath)/sizeof(WCHAR))-1] = 0;

        LoadStringW(shell32_hInstance, IDS_DELETEITEM_CAPTION, wszCaption, sizeof(wszCaption)/sizeof(WCHAR));
        wszCaption[(sizeof(wszCaption)/sizeof(WCHAR))-1] = 0;

        res = SHELL_ConfirmMsgBox(GetActiveWindow(), wszPath, wszCaption, NULL, cidl > 1);
        if (res == IDD_YESTOALL || res == IDYES)
        {
            for(i = 0; i < cidl; i++)
            {
                if (_ILIsMyComputer(apidl[i]))
                    SetNamespaceExtensionVisibleStatus(L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}", 0x1);
                else if (_ILIsNetHood(apidl[i]))
                    SetNamespaceExtensionVisibleStatus(L"{208D2C60-3AEA-1069-A2D7-08002B30309D}", 0x1);
                else if (_ILIsMyDocuments(apidl[i]))
                    SetNamespaceExtensionVisibleStatus(L"{450D8FBA-AD25-11D0-98A8-0800361B1103}", 0x1);
            }
        }
    }
    if (This->sPathTarget)
        lstrcpynW(wszPath, This->sPathTarget, MAX_PATH);
    else
        wszPath[0] = '\0';

    PathAddBackslashW(wszPath);
    wszPathsList = build_paths_list(wszPath, cidl, apidl);

    ZeroMemory(&op, sizeof(op));
    op.hwnd = GetActiveWindow();
    op.wFunc = FO_DELETE;
    op.pFrom = wszPathsList;
    op.fFlags = FOF_ALLOWUNDO;
    if (SHFileOperationW(&op))
    {
        WARN("SHFileOperation failed\n");
        ret = E_FAIL;
    }
    else
        ret = S_OK;

    /* we currently need to manually send the notifies */
    wszCurrentPath = wszPathsList;
    for (i = 0; i < cidl; i++)
    {
        LONG wEventId;

        if (_ILIsFolder(apidl[i]))
            wEventId = SHCNE_RMDIR;
        else if (_ILIsValue(apidl[i]))
            wEventId = SHCNE_DELETE;
        else
            continue;

        /* check if file exists */
        if (GetFileAttributesW(wszCurrentPath) == INVALID_FILE_ATTRIBUTES)
        {
            LPITEMIDLIST pidl = ILCombine(This->pidlRoot, apidl[i]);
            SHChangeNotify(wEventId, SHCNF_IDLIST, pidl, NULL);
            SHFree(pidl);
        }

        wszCurrentPath += wcslen(wszCurrentPath)+1;
    }
    HeapFree(GetProcessHeap(), 0, wszPathsList);
    return ret;
}

static HRESULT WINAPI
ISF_Desktop_ISFHelper_fnCopyItems (ISFHelper * iface, IShellFolder * pSFFrom, UINT cidl, LPCITEMIDLIST * apidl)
{
    IPersistFolder2 *ppf2 = NULL;
    WCHAR szSrcPath[MAX_PATH];
    WCHAR szTargetPath[MAX_PATH];
    SHFILEOPSTRUCTW op;
    LPITEMIDLIST pidl;
    LPWSTR pszSrc, pszTarget, pszSrcList, pszTargetList, pszFileName;
    int res, length;
    STRRET strRet;
    IGenericSFImpl *This = impl_from_ISFHelper(iface);

    TRACE ("(%p)->(%p,%u,%p)\n", This, pSFFrom, cidl, apidl);

    IShellFolder_QueryInterface (pSFFrom, &IID_IPersistFolder2, (LPVOID *) & ppf2);
    if (ppf2) 
    {
        if (FAILED(IPersistFolder2_GetCurFolder (ppf2, &pidl)))
        {
            IPersistFolder2_Release(ppf2);
            return E_FAIL;
        }
        IPersistFolder2_Release(ppf2);

        if (FAILED(IShellFolder_GetDisplayNameOf(pSFFrom, pidl, SHGDN_FORPARSING, &strRet)))
        {
            SHFree (pidl);
            return E_FAIL;
        }

        if (FAILED(StrRetToBufW(&strRet, pidl, szSrcPath, MAX_PATH)))
        {
            SHFree (pidl);
            return E_FAIL;
        }
        SHFree (pidl);

        pszSrc = PathAddBackslashW (szSrcPath);

        wcscpy(szTargetPath, This->sPathTarget);
        pszTarget = PathAddBackslashW (szTargetPath);

        pszSrcList = build_paths_list(szSrcPath, cidl, apidl);
        pszTargetList = build_paths_list(szTargetPath, cidl, apidl);

        if (!pszSrcList || !pszTargetList)
        {
            if (pszSrcList)
                HeapFree(GetProcessHeap(), 0, pszSrcList);

            if (pszTargetList)
                HeapFree(GetProcessHeap(), 0, pszTargetList);

            SHFree (pidl);
            IPersistFolder2_Release (ppf2);
            return E_OUTOFMEMORY;
        }
        ZeroMemory(&op, sizeof(op));
        if (!pszSrcList[0])
        {
            /* remove trailing backslash */
            pszSrc--;
            pszSrc[0] = L'\0';
            op.pFrom = szSrcPath;
        }
        else
        {
            op.pFrom = pszSrcList;
        }

        if (!pszTargetList[0])
        {
            /* remove trailing backslash */
            if (pszTarget - szTargetPath > 3)
            {
                pszTarget--;
                pszTarget[0] = L'\0';
            }
            else
            {
                pszTarget[1] = L'\0';
            }

            op.pTo = szTargetPath;
        }
        else
        {
            op.pTo = pszTargetList;
        }
        op.hwnd = GetActiveWindow();
        op.wFunc = FO_COPY;
        op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR;

        res = SHFileOperationW(&op);

        if (res == DE_SAMEFILE)
        {
            length = wcslen(szTargetPath);


            pszFileName = wcsrchr(pszSrcList, '\\');
            pszFileName++;

            if (LoadStringW(shell32_hInstance, IDS_COPY_OF, pszTarget, MAX_PATH - length))
            {
                wcscat(szTargetPath, L" ");
            }

            wcscat(szTargetPath, pszFileName);
            op.pTo = szTargetPath;

            res = SHFileOperationW(&op);
        }


        HeapFree(GetProcessHeap(), 0, pszSrcList);
        HeapFree(GetProcessHeap(), 0, pszTargetList);

        if (res)
            return E_FAIL;
        else
            return S_OK;
    }
    return E_FAIL;
}

static const ISFHelperVtbl vt_FSFldr_ISFHelper =
{
    ISF_Desktop_ISFHelper_fnQueryInterface,
    ISF_Desktop_ISFHelper_fnAddRef,
    ISF_Desktop_ISFHelper_fnRelease,
    ISF_Desktop_ISFHelper_fnGetUniqueName,
    ISF_Desktop_ISFHelper_fnAddFolder,
    ISF_Desktop_ISFHelper_fnDeleteItems,
    ISF_Desktop_ISFHelper_fnCopyItems
};


/**************************************************************************
 *    ISF_Desktop_Constructor
 */
HRESULT WINAPI ISF_Desktop_Constructor (
                IUnknown * pUnkOuter, REFIID riid, LPVOID * ppv)
{
    static IGenericSFImpl *cached_sf;
    WCHAR szMyPath[MAX_PATH];

    TRACE ("unkOut=%p %s\n", pUnkOuter, shdebugstr_guid (riid));

    if (!ppv)
        return E_POINTER;
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if (!cached_sf)
    {
        IGenericSFImpl *sf;

        if (!SHGetSpecialFolderPathW( 0, szMyPath, CSIDL_DESKTOPDIRECTORY, TRUE ))
            return E_UNEXPECTED;

        sf = LocalAlloc( LMEM_ZEROINIT, sizeof (IGenericSFImpl) );
        if (!sf)
            return E_OUTOFMEMORY;

        sf->ref = 1;
        sf->lpVtbl = &vt_MCFldr_ShellFolder2;
        sf->lpPF2 = &vt_FSFldr_PersistFolder2;
        sf->lpvtblSFHelper = &vt_FSFldr_ISFHelper;
        sf->pidlRoot = _ILCreateDesktop();    /* my qualified pidl */
        sf->sPathTarget = SHAlloc( (wcslen(szMyPath) + 1)*sizeof(WCHAR) );
        wcscpy( sf->sPathTarget, szMyPath );

        if (InterlockedCompareExchangePointer((void *)&cached_sf, sf, NULL) != NULL)
        {
            /* some other thread already been here */
            SHFree( sf->pidlRoot );
            SHFree( sf->sPathTarget );
            LocalFree( sf );
        }
    }

    return IUnknown_QueryInterface( _IUnknown_(cached_sf), riid, ppv );
}
