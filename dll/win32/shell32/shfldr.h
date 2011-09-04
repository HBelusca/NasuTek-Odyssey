
/*
 *	Virtual Folder
 *	common definitions
 *
 *	Copyright 1997			Marcus Meissner
 *	Copyright 1998, 1999, 2002	Juergen Schmied
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

#define CHARS_IN_GUID 39

typedef struct {
    int colnameid;
    int pcsFlags;
    int fmt;
    int cxChar;
} shvheader;

#define GET_SHGDN_FOR(dwFlags)         ((DWORD)dwFlags & (DWORD)0x0000FF00)
#define GET_SHGDN_RELATION(dwFlags)    ((DWORD)dwFlags & (DWORD)0x000000FF)

BOOL SHELL32_GetCustomFolderAttribute (LPCITEMIDLIST pidl, LPCWSTR pwszHeading, LPCWSTR pwszAttribute, LPWSTR pwszValue, DWORD cchValue);

LPCWSTR GetNextElementW (LPCWSTR pszNext, LPWSTR pszOut, DWORD dwOut);
HRESULT SHELL32_ParseNextElement (IShellFolder2 * psf, HWND hwndOwner, LPBC pbc, LPITEMIDLIST * pidlInOut,
				  LPOLESTR szNext, DWORD * pEaten, DWORD * pdwAttributes);
HRESULT SHELL32_GetItemAttributes (IShellFolder * psf, LPCITEMIDLIST pidl, LPDWORD pdwAttributes);
HRESULT SHELL32_GetDisplayNameOfChild (IShellFolder2 * psf, LPCITEMIDLIST pidl, DWORD dwFlags, LPWSTR szOut,
				       DWORD dwOutLen);

HRESULT SHELL32_BindToChild (LPCITEMIDLIST pidlRoot,
			     LPCWSTR pathRoot, LPCITEMIDLIST pidlComplete, REFIID riid, LPVOID * ppvOut);

HRESULT SHELL32_CompareIDs (IShellFolder * iface, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
LPITEMIDLIST SHELL32_CreatePidlFromBindCtx(IBindCtx *pbc, LPCWSTR path);

static int __inline SHELL32_GUIDToStringA (REFGUID guid, LPSTR str)
{
    return sprintf(str, "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
            guid->Data1, guid->Data2, guid->Data3,
            guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
            guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

static int __inline SHELL32_GUIDToStringW (REFGUID guid, LPWSTR str)
{
    static const WCHAR fmtW[] =
     { '{','%','0','8','l','x','-','%','0','4','x','-','%','0','4','x','-',
     '%','0','2','x','%','0','2','x','-',
     '%','0','2','x','%','0','2','x','%','0','2','x','%','0','2','x',
     '%','0','2','x','%','0','2','x','}',0 };
    return swprintf(str, fmtW,
            guid->Data1, guid->Data2, guid->Data3,
            guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
            guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
}

void SHELL_FS_ProcessDisplayFilename(LPWSTR szPath, DWORD dwFlags);
BOOL SHELL_FS_HideExtension(LPWSTR pwszPath);
