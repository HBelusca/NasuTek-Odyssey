/*
 *	AutoComplete interfaces implementation.
 *
 *	Copyright 2004	Maxime Belleng� <maxime.bellenge@laposte.net>
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

/*
  Implemented:
  - ACO_AUTOAPPEND style
  - ACO_AUTOSUGGEST style
  - ACO_UPDOWNKEYDROPSLIST style

  - Handle pwzsRegKeyPath and pwszQuickComplete in Init

  TODO:
  - implement ACO_SEARCH style
  - implement ACO_FILTERPREFIXES style
  - implement ACO_USETAB style
  - implement ACO_RTLREADING style

 */

#include <precomp.h>


WINE_DEFAULT_DEBUG_CHANNEL(shell);

typedef struct
{
    const IAutoCompleteVtbl  *lpVtbl;
    const IAutoComplete2Vtbl *lpvtblAutoComplete2;
    LONG ref;
    BOOL  enabled;
    HWND hwndEdit;
    HWND hwndListBox;
    WNDPROC wpOrigEditProc;
    WNDPROC wpOrigLBoxProc;
    WCHAR *txtbackup;
    WCHAR *quickComplete;
    IEnumString *enumstr;
    AUTOCOMPLETEOPTIONS options;
} IAutoCompleteImpl;

static const IAutoCompleteVtbl acvt;
static const IAutoComplete2Vtbl ac2vt;

static IAutoCompleteImpl * impl_from_IAutoComplete2( IAutoComplete2 *iface )
{
    return (IAutoCompleteImpl *)((char*)iface - FIELD_OFFSET(IAutoCompleteImpl, lpvtblAutoComplete2));
}


/*
  converts This to an interface pointer
*/
#define _IUnknown_(This) (IUnknown*)&(This->lpVtbl)
#define _IAutoComplete2_(This)  (IAutoComplete2*)&(This->lpvtblAutoComplete2)

static LRESULT APIENTRY ACEditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static LRESULT APIENTRY ACLBoxSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/**************************************************************************
 *  IAutoComplete_Constructor
 */
HRESULT WINAPI IAutoComplete_Constructor(IUnknown * pUnkOuter, REFIID riid, LPVOID * ppv)
{
    IAutoCompleteImpl *lpac;

    if (pUnkOuter && !IsEqualIID (riid, &IID_IUnknown))
	return CLASS_E_NOAGGREGATION;

    lpac = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IAutoCompleteImpl));
    if (!lpac)
	return E_OUTOFMEMORY;

    lpac->ref = 1;
    lpac->lpVtbl = &acvt;
    lpac->lpvtblAutoComplete2 = &ac2vt;
    lpac->enabled = TRUE;
    lpac->enumstr = NULL;
    lpac->options = ACO_AUTOAPPEND;
    lpac->wpOrigEditProc = NULL;
    lpac->hwndListBox = NULL;
    lpac->txtbackup = NULL;
    lpac->quickComplete = NULL;

    if (!SUCCEEDED (IUnknown_QueryInterface (_IUnknown_ (lpac), riid, ppv))) {
	IUnknown_Release (_IUnknown_ (lpac));
	return E_NOINTERFACE;
    }

    TRACE("-- (%p)->\n",lpac);

    return S_OK;
}

/**************************************************************************
 *  AutoComplete_QueryInterface
 */
static HRESULT WINAPI IAutoComplete_fnQueryInterface(
    IAutoComplete * iface,
    REFIID riid,
    LPVOID *ppvObj)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)iface;

    TRACE("(%p)->(\n\tIID:\t%s,%p)\n", This, shdebugstr_guid(riid), ppvObj);
    *ppvObj = NULL;

    if(IsEqualIID(riid, &IID_IUnknown))
    {
	*ppvObj = This;
    }
    else if(IsEqualIID(riid, &IID_IAutoComplete))
    {
	*ppvObj = (IAutoComplete*)This;
    }
    else if(IsEqualIID(riid, &IID_IAutoComplete2))
    {
	*ppvObj = _IAutoComplete2_ (This);
    }

    if (*ppvObj)
    {
	IAutoComplete_AddRef((IAutoComplete*)*ppvObj);
	TRACE("-- Interface: (%p)->(%p)\n", ppvObj, *ppvObj);
	return S_OK;
    }
    TRACE("-- Interface: E_NOINTERFACE\n");
    return E_NOINTERFACE;
}

/******************************************************************************
 * IAutoComplete_fnAddRef
 */
static ULONG WINAPI IAutoComplete_fnAddRef(
	IAutoComplete * iface)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)iface;
    ULONG refCount = InterlockedIncrement(&This->ref);

    TRACE("(%p)->(%u)\n", This, refCount - 1);

    return refCount;
}

/******************************************************************************
 * IAutoComplete_fnRelease
 */
static ULONG WINAPI IAutoComplete_fnRelease(
	IAutoComplete * iface)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)iface;
    ULONG refCount = InterlockedDecrement(&This->ref);

    TRACE("(%p)->(%u)\n", This, refCount + 1);

    if (!refCount) {
	TRACE(" destroying IAutoComplete(%p)\n",This);
        HeapFree(GetProcessHeap(), 0, This->quickComplete);
        HeapFree(GetProcessHeap(), 0, This->txtbackup);
	if (This->hwndListBox)
	    DestroyWindow(This->hwndListBox);
	if (This->enumstr)
	    IEnumString_Release(This->enumstr);
	HeapFree(GetProcessHeap(), 0, This);
    }
    return refCount;
}

/******************************************************************************
 * IAutoComplete_fnEnable
 */
static HRESULT WINAPI IAutoComplete_fnEnable(
    IAutoComplete * iface,
    BOOL fEnable)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)iface;

    HRESULT hr = S_OK;

    TRACE("(%p)->(%s)\n", This, (fEnable)?"true":"false");

    This->enabled = fEnable;

    return hr;
}

/******************************************************************************
 * IAutoComplete_fnInit
 */
static HRESULT WINAPI IAutoComplete_fnInit(
    IAutoComplete * iface,
    HWND hwndEdit,
    IUnknown *punkACL,
    LPCOLESTR pwzsRegKeyPath,
    LPCOLESTR pwszQuickComplete)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)iface;
    static const WCHAR lbName[] = {'L','i','s','t','B','o','x',0};

    TRACE("(%p)->(0x%08lx, %p, %s, %s)\n",
	  This, hwndEdit, punkACL, debugstr_w(pwzsRegKeyPath), debugstr_w(pwszQuickComplete));

    if (This->options & ACO_AUTOSUGGEST) TRACE(" ACO_AUTOSUGGEST\n");
    if (This->options & ACO_AUTOAPPEND) TRACE(" ACO_AUTOAPPEND\n");
    if (This->options & ACO_SEARCH) FIXME(" ACO_SEARCH not supported\n");
    if (This->options & ACO_FILTERPREFIXES) FIXME(" ACO_FILTERPREFIXES not supported\n");
    if (This->options & ACO_USETAB) FIXME(" ACO_USETAB not supported\n");
    if (This->options & ACO_UPDOWNKEYDROPSLIST) TRACE(" ACO_UPDOWNKEYDROPSLIST\n");
    if (This->options & ACO_RTLREADING) FIXME(" ACO_RTLREADING not supported\n");

    This->hwndEdit = hwndEdit;

    if (!SUCCEEDED (IUnknown_QueryInterface (punkACL, &IID_IEnumString, (LPVOID*)&This->enumstr))) {
	TRACE("No IEnumString interface\n");
	return  E_NOINTERFACE;
    }

    This->wpOrigEditProc = (WNDPROC) SetWindowLongPtrW( hwndEdit, GWLP_WNDPROC, (LONG_PTR) ACEditSubclassProc);
    SetWindowLongPtrW( hwndEdit, GWLP_USERDATA, (LONG_PTR)This);

    if (This->options & ACO_AUTOSUGGEST) {
	HWND hwndParent;

	hwndParent = GetParent(This->hwndEdit);

	/* FIXME : The listbox should be resizable with the mouse. WS_THICKFRAME looks ugly */
	This->hwndListBox = CreateWindowExW(0, lbName, NULL,
					    WS_BORDER | WS_CHILD | WS_VSCROLL | LBS_HASSTRINGS | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
					    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					    hwndParent, NULL,
					    (HINSTANCE)GetWindowLongPtrW( hwndParent, GWLP_HINSTANCE ), NULL);

	if (This->hwndListBox) {
	    This->wpOrigLBoxProc = (WNDPROC) SetWindowLongPtrW( This->hwndListBox, GWLP_WNDPROC, (LONG_PTR) ACLBoxSubclassProc);
	    SetWindowLongPtrW( This->hwndListBox, GWLP_USERDATA, (LONG_PTR)This);
	}
    }

    if (pwzsRegKeyPath) {
	WCHAR *key;
	WCHAR result[MAX_PATH];
	WCHAR *value;
	HKEY hKey = 0;
	LONG res;
	LONG len;

	/* pwszRegKeyPath contains the key as well as the value, so we split */
	key = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (wcslen(pwzsRegKeyPath)+1)*sizeof(WCHAR));
	wcscpy(key, pwzsRegKeyPath);
	value = strrchrW(key, '\\');
	*value = 0;
	value++;
	/* Now value contains the value and buffer the key */
	res = RegOpenKeyExW(HKEY_CURRENT_USER, key, 0, KEY_READ, &hKey);
	if (res != ERROR_SUCCESS) {
	    /* if the key is not found, MSDN states we must seek in HKEY_LOCAL_MACHINE */
	    res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hKey);
	}
	if (res == ERROR_SUCCESS) {
	    res = RegQueryValueW(hKey, value, result, &len);
	    if (res == ERROR_SUCCESS) {
		This->quickComplete = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, len*sizeof(WCHAR));
		wcscpy(This->quickComplete, result);
	    }
	    RegCloseKey(hKey);
	}
	HeapFree(GetProcessHeap(), 0, key);
    }

    if ((pwszQuickComplete) && (!This->quickComplete)) {
	This->quickComplete = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (wcslen(pwszQuickComplete)+1)*sizeof(WCHAR));
	wcscpy(This->quickComplete, pwszQuickComplete);
    }

    return S_OK;
}

/**************************************************************************
 *  IAutoComplete_fnVTable
 */
static const IAutoCompleteVtbl acvt =
{
    IAutoComplete_fnQueryInterface,
    IAutoComplete_fnAddRef,
    IAutoComplete_fnRelease,
    IAutoComplete_fnInit,
    IAutoComplete_fnEnable,
};

/**************************************************************************
 *  AutoComplete2_QueryInterface
 */
static HRESULT WINAPI IAutoComplete2_fnQueryInterface(
    IAutoComplete2 * iface,
    REFIID riid,
    LPVOID *ppvObj)
{
    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE ("(%p)->(%s,%p)\n", This, shdebugstr_guid (riid), ppvObj);

    return IAutoComplete_QueryInterface((IAutoComplete*)This, riid, ppvObj);
}

/******************************************************************************
 * IAutoComplete2_fnAddRef
 */
static ULONG WINAPI IAutoComplete2_fnAddRef(
	IAutoComplete2 * iface)
{
    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE ("(%p)->(count=%u)\n", This, This->ref);

    return IAutoComplete2_AddRef((IAutoComplete*)This);
}

/******************************************************************************
 * IAutoComplete2_fnRelease
 */
static ULONG WINAPI IAutoComplete2_fnRelease(
	IAutoComplete2 * iface)
{
    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE ("(%p)->(count=%u)\n", This, This->ref);

    return IAutoComplete_Release((IAutoComplete*)This);
}

/******************************************************************************
 * IAutoComplete2_fnEnable
 */
static HRESULT WINAPI IAutoComplete2_fnEnable(
    IAutoComplete2 * iface,
    BOOL fEnable)
{
    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE ("(%p)->(%s)\n", This, (fEnable)?"true":"false");

    return IAutoComplete_Enable((IAutoComplete*)This, fEnable);
}

/******************************************************************************
 * IAutoComplete2_fnInit
 */
static HRESULT WINAPI IAutoComplete2_fnInit(
    IAutoComplete2 * iface,
    HWND hwndEdit,
    IUnknown *punkACL,
    LPCOLESTR pwzsRegKeyPath,
    LPCOLESTR pwszQuickComplete)
{
    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE("(%p)\n", This);

    return IAutoComplete_Init((IAutoComplete*)This, hwndEdit, punkACL, pwzsRegKeyPath, pwszQuickComplete);
}

/**************************************************************************
 *  IAutoComplete_fnGetOptions
 */
static HRESULT WINAPI IAutoComplete2_fnGetOptions(
    IAutoComplete2 * iface,
    DWORD *pdwFlag)
{
    HRESULT hr = S_OK;

    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE("(%p) -> (%p)\n", This, pdwFlag);

    *pdwFlag = This->options;

    return hr;
}

/**************************************************************************
 *  IAutoComplete_fnSetOptions
 */
static HRESULT WINAPI IAutoComplete2_fnSetOptions(
    IAutoComplete2 * iface,
    DWORD dwFlag)
{
    HRESULT hr = S_OK;

    IAutoCompleteImpl *This = impl_from_IAutoComplete2(iface);

    TRACE("(%p) -> (0x%x)\n", This, dwFlag);

    This->options = dwFlag;

    return hr;
}

/**************************************************************************
 *  IAutoComplete2_fnVTable
 */
static const IAutoComplete2Vtbl ac2vt =
{
    IAutoComplete2_fnQueryInterface,
    IAutoComplete2_fnAddRef,
    IAutoComplete2_fnRelease,
    IAutoComplete2_fnInit,
    IAutoComplete2_fnEnable,
    /* IAutoComplete2 */
    IAutoComplete2_fnSetOptions,
    IAutoComplete2_fnGetOptions,
};

/*
  Window procedure for autocompletion
 */
static LRESULT APIENTRY ACEditSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    LPOLESTR strs;
    HRESULT hr;
    WCHAR hwndText[255];
    WCHAR *hwndQCText;
    RECT r;
    BOOL control, filled, displayall = FALSE;
    int cpt, height, sel;

    if (!This->enabled) return CallWindowProcW(This->wpOrigEditProc, hwnd, uMsg, wParam, lParam);

    switch (uMsg)
    {
	case CB_SHOWDROPDOWN:
	    ShowWindow(This->hwndListBox, SW_HIDE);
	    break;
	case WM_KILLFOCUS:
	    if ((This->options & ACO_AUTOSUGGEST) &&
		((HWND)wParam != This->hwndListBox))
	    {
		ShowWindow(This->hwndListBox, SW_HIDE);
	    }
	    break;
	case WM_KEYUP:

	    GetWindowTextW( hwnd, (LPWSTR)hwndText, 255);

	    switch(wParam) {
		case VK_RETURN:
		    /* If quickComplete is set and control is pressed, replace the string */
		    control = GetKeyState(VK_CONTROL) & 0x8000;
		    if (control && This->quickComplete) {
			hwndQCText = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
						       (wcslen(This->quickComplete)+wcslen(hwndText))*sizeof(WCHAR));
			sel = swprintf(hwndQCText, This->quickComplete, hwndText);
			SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)hwndQCText);
			SendMessageW(hwnd, EM_SETSEL, 0, sel);
			HeapFree(GetProcessHeap(), 0, hwndQCText);
		    }

		    ShowWindow(This->hwndListBox, SW_HIDE);
		    return 0;
		case VK_LEFT:
		case VK_RIGHT:
		    return 0;
		case VK_UP:
		case VK_DOWN:
		    /* Two cases here :
		       - if the listbox is not visible, displays it
		       with all the entries if the style ACO_UPDOWNKEYDROPSLIST
		       is present but does not select anything.
		       - if the listbox is visible, change the selection
		    */
		    if ( (This->options & (ACO_AUTOSUGGEST | ACO_UPDOWNKEYDROPSLIST))
			 && (!IsWindowVisible(This->hwndListBox) && (! *hwndText)) )
		    {
			 /* We must display all the entries */
			 displayall = TRUE;
		    } else {
			if (IsWindowVisible(This->hwndListBox)) {
			    int count;

			    count = SendMessageW(This->hwndListBox, LB_GETCOUNT, 0, 0);
			    /* Change the selection */
			    sel = SendMessageW(This->hwndListBox, LB_GETCURSEL, 0, 0);
			    if (wParam == VK_UP)
				sel = ((sel-1)<0)?count-1:sel-1;
			    else
				sel = ((sel+1)>= count)?-1:sel+1;
			    SendMessageW(This->hwndListBox, LB_SETCURSEL, sel, 0);
			    if (sel != -1) {
				WCHAR *msg;
				int len;

				len = SendMessageW(This->hwndListBox, LB_GETTEXTLEN, sel, (LPARAM)NULL);
				msg = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len+1)*sizeof(WCHAR));
				SendMessageW(This->hwndListBox, LB_GETTEXT, sel, (LPARAM)msg);
				SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)msg);
				SendMessageW(hwnd, EM_SETSEL, wcslen(msg), wcslen(msg));
				HeapFree(GetProcessHeap(), 0, msg);
			    } else {
				SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)This->txtbackup);
				SendMessageW(hwnd, EM_SETSEL, wcslen(This->txtbackup), wcslen(This->txtbackup));
			    }
			}
			return 0;
		    }
		    break;
		case VK_BACK:
		case VK_DELETE:
		    if ((! *hwndText) && (This->options & ACO_AUTOSUGGEST)) {
			ShowWindow(This->hwndListBox, SW_HIDE);
			return CallWindowProcW(This->wpOrigEditProc, hwnd, uMsg, wParam, lParam);
		    }
		    if (This->options & ACO_AUTOAPPEND) {
			DWORD b;
			SendMessageW(hwnd, EM_GETSEL, (WPARAM)&b, (LPARAM)NULL);
			if (b>1) {
			    hwndText[b-1] = '\0';
			} else {
			    hwndText[0] = '\0';
			    SetWindowTextW(hwnd, hwndText);
			}
		    }
		    break;
		default:
		    ;
	    }

	    SendMessageW(This->hwndListBox, LB_RESETCONTENT, 0, 0);

	    HeapFree(GetProcessHeap(), 0, This->txtbackup);
	    This->txtbackup = HeapAlloc(GetProcessHeap(),
						 HEAP_ZERO_MEMORY, (wcslen(hwndText)+1)*sizeof(WCHAR));
	    wcscpy(This->txtbackup, hwndText);

	    /* Returns if there is no text to search and we doesn't want to display all the entries */
	    if ((!displayall) && (! *hwndText) )
		break;

	    IEnumString_Reset(This->enumstr);
	    filled = FALSE;
	    for(cpt = 0;;) {
		hr = IEnumString_Next(This->enumstr, 1, &strs, NULL);
		if (hr != S_OK)
		    break;

		if ((LPWSTR)strstrW(strs, hwndText) == strs) {

		    if (This->options & ACO_AUTOAPPEND) {
			SetWindowTextW(hwnd, strs);
			SendMessageW(hwnd, EM_SETSEL, wcslen(hwndText), wcslen(strs));
			break;
		    }

		    if (This->options & ACO_AUTOSUGGEST) {
			SendMessageW(This->hwndListBox, LB_ADDSTRING, 0, (LPARAM)strs);
			filled = TRUE;
			cpt++;
		    }
		}
	    }

	    if (This->options & ACO_AUTOSUGGEST) {
		if (filled) {
		    height = SendMessageW(This->hwndListBox, LB_GETITEMHEIGHT, 0, 0);
		    SendMessageW(This->hwndListBox, LB_CARETOFF, 0, 0);
		    GetWindowRect(hwnd, &r);
		    SetParent(This->hwndListBox, HWND_DESKTOP);
		    /* It seems that Windows XP displays 7 lines at most
		       and otherwise displays a vertical scroll bar */
		    SetWindowPos(This->hwndListBox, HWND_TOP,
				 r.left, r.bottom + 1, r.right - r.left, min(height * 7, height*(cpt+1)),
				 SWP_SHOWWINDOW );
		} else {
		    ShowWindow(This->hwndListBox, SW_HIDE);
		}
	    }

	    break;
	default:
	    return CallWindowProcW(This->wpOrigEditProc, hwnd, uMsg, wParam, lParam);

    }

    return 0;
}

static LRESULT APIENTRY ACLBoxSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    IAutoCompleteImpl *This = (IAutoCompleteImpl *)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
    WCHAR *msg;
    int sel, len;

    switch (uMsg) {
	case WM_MOUSEMOVE:
	    sel = SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, lParam);
	    SendMessageW(hwnd, LB_SETCURSEL, (WPARAM)sel, (LPARAM)0);
	    break;
	case WM_LBUTTONDOWN:
	    sel = SendMessageW(hwnd, LB_GETCURSEL, 0, 0);
	    if (sel < 0)
		break;
	    len = SendMessageW(This->hwndListBox, LB_GETTEXTLEN, sel, 0);
	    msg = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (len+1)*sizeof(WCHAR));
	    SendMessageW(hwnd, LB_GETTEXT, sel, (LPARAM)msg);
	    SendMessageW(This->hwndEdit, WM_SETTEXT, 0, (LPARAM)msg);
	    SendMessageW(This->hwndEdit, EM_SETSEL, 0, wcslen(msg));
	    ShowWindow(hwnd, SW_HIDE);
	    HeapFree(GetProcessHeap(), 0, msg);
	    break;
	default:
	    return CallWindowProcW(This->wpOrigLBoxProc, hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
