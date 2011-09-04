/*
 *	self-registerable dll functions for shell32.dll
*
 * Copyright (C) 2003 John K. Hohm
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

const GUID CLSID_AdminFolderShortcut     = {0xD20EA4E1, 0x3957, 0x11D2, {0xA4, 0x0B, 0x0C, 0x50, 0x20, 0x52, 0x41, 0x53}};
const GUID CLSID_StartMenu               = {0x4622AD11, 0xFF23, 0x11D0, {0x8D, 0x34, 0x00, 0xA0, 0xC9, 0x0F, 0x27, 0x19}};
const GUID CLSID_MenuBandSite            = {0xE13EF4E4, 0xD2F2, 0x11d0, {0x98, 0x16, 0x00, 0xC0, 0x4F, 0xD9, 0x19, 0x72}};
const GUID CLSID_OpenWith                = {0x09799AFB, 0xAD67, 0x11d1, {0xAB, 0xCD, 0x00, 0xC0, 0x4F, 0xC3, 0x09, 0x36}};
const GUID CLSID_UnixFolder              = {0xcc702eb2, 0x7dc5, 0x11d9, {0xc6, 0x87, 0x00, 0x04, 0x23, 0x8a, 0x01, 0xcd}};
const GUID CLSID_UnixDosFolder           = {0x9d20aae8, 0x0625, 0x44b0, {0x9c, 0xa7, 0x71, 0x88, 0x9c, 0x22, 0x54, 0xd9}};
const GUID CLSID_FontsFolderShortcut     = {0xD20EA4E1, 0x3957, 0x11D2, {0xA4, 0x0B, 0x0C, 0x50, 0x20, 0x52, 0x41,0x52}};
const GUID SHELL32_AdvtShortcutProduct   = {0x9db1186f, 0x40df, 0x11d1, {0xaa, 0x8c, 0x00, 0xc0, 0x4f, 0xb6, 0x78, 0x63}};
const GUID SHELL32_AdvtShortcutComponent = {0x9db1186e, 0x40df, 0x11d1, {0xaa, 0x8c, 0x00, 0xc0, 0x4f, 0xb6, 0x78, 0x63}};

#ifdef _MSC_VER
extern const GUID CLSID_ShellFolderViewOC;
#endif


WINE_DEFAULT_DEBUG_CHANNEL(shell);

/*
 * Near the bottom of this file are the exported DllRegisterServer and
 * DllUnregisterServer, which make all this worthwhile.
 */

/***********************************************************************
 *		interface for self-registering
 */
struct regsvr_interface
{
    IID const *iid;		/* NULL for end of list */
    LPCSTR name;		/* can be NULL to omit */
    IID const *base_iid;	/* can be NULL to omit */
    int num_methods;		/* can be <0 to omit */
    CLSID const *ps_clsid;	/* can be NULL to omit */
    CLSID const *ps_clsid32;	/* can be NULL to omit */
};

static HRESULT register_interfaces(struct regsvr_interface const *list);
static HRESULT unregister_interfaces(struct regsvr_interface const *list);

struct regsvr_coclass
{
    CLSID const *clsid;		/* NULL for end of list */
    LPCSTR name;		/* can be NULL to omit */
    UINT idName;                /* can be 0 to omit */
    LPCSTR ips;			/* can be NULL to omit */
    LPCSTR ips32;		/* can be NULL to omit */
    LPCSTR ips32_tmodel;	/* can be NULL to omit */
    DWORD flags;
    DWORD dwAttributes;
    DWORD dwCallForAttributes;
    LPCSTR clsid_str;		/* can be NULL to omit */
    LPCSTR progid;		/* can be NULL to omit */
    UINT idDefaultIcon;         /* can be 0 to omit */
//    CLSID const *clsid_menu; /* can be NULL to omit */
};

/* flags for regsvr_coclass.flags */
#define SHELLEX_MAYCHANGEDEFAULTMENU  0x00000001
#define SHELLFOLDER_WANTSFORPARSING   0x00000002
#define SHELLFOLDER_ATTRIBUTES        0x00000004
#define SHELLFOLDER_CALLFORATTRIBUTES 0x00000008
#define SHELLFOLDER_WANTSFORDISPLAY   0x00000010
#define SHELLFOLDER_HIDEASDELETEPERUSER 0x00000020

static HRESULT register_coclasses(struct regsvr_coclass const *list);
static HRESULT unregister_coclasses(struct regsvr_coclass const *list);

struct regsvr_namespace
{
    CLSID const *clsid; /* CLSID of the namespace extension. NULL for end of list */
    LPCWSTR parent;     /* Mount point (MyComputer, Desktop, ..). */
    LPCWSTR value;      /* Display name of the extension. */
};

static HRESULT register_namespace_extensions(struct regsvr_namespace const *list);
static HRESULT unregister_namespace_extensions(struct regsvr_namespace const *list);

/***********************************************************************
 *		static string constants
 */
static WCHAR const interface_keyname[10] = {
    'I', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e', 0 };
static WCHAR const base_ifa_keyname[14] = {
    'B', 'a', 's', 'e', 'I', 'n', 't', 'e', 'r', 'f', 'a', 'c',
    'e', 0 };
static WCHAR const num_methods_keyname[11] = {
    'N', 'u', 'm', 'M', 'e', 't', 'h', 'o', 'd', 's', 0 };
static WCHAR const ps_clsid_keyname[15] = {
    'P', 'r', 'o', 'x', 'y', 'S', 't', 'u', 'b', 'C', 'l', 's',
    'i', 'd', 0 };
static WCHAR const ps_clsid32_keyname[17] = {
    'P', 'r', 'o', 'x', 'y', 'S', 't', 'u', 'b', 'C', 'l', 's',
    'i', 'd', '3', '2', 0 };
static WCHAR const clsid_keyname[6] = {
    'C', 'L', 'S', 'I', 'D', 0 };
static WCHAR const ips_keyname[13] = {
    'I', 'n', 'P', 'r', 'o', 'c', 'S', 'e', 'r', 'v', 'e', 'r',
    0 };
static WCHAR const ips32_keyname[15] = {
    'I', 'n', 'P', 'r', 'o', 'c', 'S', 'e', 'r', 'v', 'e', 'r',
    '3', '2', 0 };
static WCHAR const progid_keyname[7] = {
    'P', 'r', 'o', 'g', 'I', 'D', 0 };
static WCHAR const shellex_keyname[8] = {
    's', 'h', 'e', 'l', 'l', 'e', 'x', 0 };
static WCHAR const shellfolder_keyname[12] = {
    'S', 'h', 'e', 'l', 'l', 'F', 'o', 'l', 'd', 'e', 'r', 0 };
static WCHAR const mcdm_keyname[21] = {
    'M', 'a', 'y', 'C', 'h', 'a', 'n', 'g', 'e', 'D', 'e', 'f',
    'a', 'u', 'l', 't', 'M', 'e', 'n', 'u', 0 };
static WCHAR const defaulticon_keyname[] = {
    'D','e','f','a','u','l','t','I','c','o','n',0};
static WCHAR const contextmenu_keyname[] = { 'C', 'o', 'n', 't', 'e', 'x', 't', 'M', 'e', 'n', 'u', 'H', 'a', 'n', 'd', 'l', 'e', 'r', 's', 0 };
static char const tmodel_valuename[] = "ThreadingModel";
static char const wfparsing_valuename[] = "WantsFORPARSING";
static char const wfdisplay_valuename[] = "WantsFORDISPLAY";
static char const hideasdeleteperuser_valuename[] = "HideAsDeletePerUser";
static char const attributes_valuename[] = "Attributes";
static char const cfattributes_valuename[] = "CallForAttributes";
static char const localized_valuename[] = "LocalizedString";

/***********************************************************************
 *		static helper functions
 */
static LONG register_key_guid(HKEY base, WCHAR const *name, GUID const *guid);
static LONG register_key_defvalueW(HKEY base, WCHAR const *name,
				   WCHAR const *value);
static LONG register_key_defvalueA(HKEY base, WCHAR const *name,
				   char const *value);

/***********************************************************************
 *		register_interfaces
 */
static HRESULT register_interfaces(struct regsvr_interface const *list)
{
    LONG res = ERROR_SUCCESS;
    HKEY interface_key;

    res = RegCreateKeyExW(HKEY_CLASSES_ROOT, interface_keyname, 0, NULL, 0,
			  KEY_READ | KEY_WRITE, NULL, &interface_key, NULL);
    if (res != ERROR_SUCCESS) goto error_return;

    for (; res == ERROR_SUCCESS && list->iid; ++list) {
	WCHAR buf[39];
	HKEY iid_key;

	StringFromGUID2(list->iid, buf, 39);
	res = RegCreateKeyExW(interface_key, buf, 0, NULL, 0,
			      KEY_READ | KEY_WRITE, NULL, &iid_key, NULL);
	if (res != ERROR_SUCCESS) goto error_close_interface_key;

	if (list->name) {
	    res = RegSetValueExA(iid_key, NULL, 0, REG_SZ,
				 (CONST BYTE*)(list->name),
				 strlen(list->name) + 1);
	    if (res != ERROR_SUCCESS) goto error_close_iid_key;
	}

	if (list->base_iid) {
	    res = register_key_guid(iid_key, base_ifa_keyname, list->base_iid);
	    if (res != ERROR_SUCCESS) goto error_close_iid_key;
	}

	if (0 <= list->num_methods) {
	    static WCHAR const fmt[3] = { '%', 'd', 0 };
	    HKEY key;

	    res = RegCreateKeyExW(iid_key, num_methods_keyname, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL, &key, NULL);
	    if (res != ERROR_SUCCESS) goto error_close_iid_key;

	    swprintf(buf, fmt, list->num_methods);
	    res = RegSetValueExW(key, NULL, 0, REG_SZ,
				 (CONST BYTE*)buf,
				 (wcslen(buf) + 1) * sizeof(WCHAR));
	    RegCloseKey(key);

	    if (res != ERROR_SUCCESS) goto error_close_iid_key;
	}

	if (list->ps_clsid) {
	    res = register_key_guid(iid_key, ps_clsid_keyname, list->ps_clsid);
	    if (res != ERROR_SUCCESS) goto error_close_iid_key;
	}

	if (list->ps_clsid32) {
	    res = register_key_guid(iid_key, ps_clsid32_keyname, list->ps_clsid32);
	    if (res != ERROR_SUCCESS) goto error_close_iid_key;
	}

    error_close_iid_key:
	RegCloseKey(iid_key);
    }

error_close_interface_key:
    RegCloseKey(interface_key);
error_return:
    return res != ERROR_SUCCESS ? HRESULT_FROM_WIN32(res) : S_OK;
}

/***********************************************************************
 *		unregister_interfaces
 */
static HRESULT unregister_interfaces(struct regsvr_interface const *list)
{
    LONG res = ERROR_SUCCESS;
    HKEY interface_key;

    res = RegOpenKeyExW(HKEY_CLASSES_ROOT, interface_keyname, 0,
			KEY_READ | KEY_WRITE, &interface_key);
    if (res == ERROR_FILE_NOT_FOUND) return S_OK;
    if (res != ERROR_SUCCESS) goto error_return;

    for (; res == ERROR_SUCCESS && list->iid; ++list) {
	WCHAR buf[39];

	StringFromGUID2(list->iid, buf, 39);
	res = RegDeleteTreeW(interface_key, buf);
	if (res == ERROR_FILE_NOT_FOUND) res = ERROR_SUCCESS;
    }

    RegCloseKey(interface_key);
error_return:
    return res != ERROR_SUCCESS ? HRESULT_FROM_WIN32(res) : S_OK;
}

/***********************************************************************
 *		register_coclasses
 */
static HRESULT register_coclasses(struct regsvr_coclass const *list)
{
    LONG res = ERROR_SUCCESS;
    HKEY coclass_key;

    res = RegCreateKeyExW(HKEY_CLASSES_ROOT, clsid_keyname, 0, NULL, 0,
			  KEY_READ | KEY_WRITE, NULL, &coclass_key, NULL);
    if (res != ERROR_SUCCESS) goto error_return;

    for (; res == ERROR_SUCCESS && list->clsid; ++list) {
	WCHAR buf[39];
	HKEY clsid_key;

	StringFromGUID2(list->clsid, buf, 39);
	res = RegCreateKeyExW(coclass_key, buf, 0, NULL, 0,
			      KEY_READ | KEY_WRITE, NULL, &clsid_key, NULL);
	if (res != ERROR_SUCCESS) goto error_close_coclass_key;

	if (list->name) {
	    res = RegSetValueExA(clsid_key, NULL, 0, REG_SZ,
				 (CONST BYTE*)(list->name),
				 strlen(list->name) + 1);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	}

        if (list->idName) {
            char buffer[64];
            sprintf(buffer, "@shell32.dll,-%u", list->idName);
            res = RegSetValueExA(clsid_key, localized_valuename, 0, REG_SZ,
                                 (CONST BYTE*)(buffer), strlen(buffer)+1);
            if (res != ERROR_SUCCESS) goto error_close_clsid_key;
        }

        if (list->idDefaultIcon) {
            HKEY icon_key;
            char buffer[64];

            res = RegCreateKeyExW(clsid_key, defaulticon_keyname, 0, NULL, 0,
                        KEY_READ | KEY_WRITE, NULL, &icon_key, NULL);
            if (res != ERROR_SUCCESS) goto error_close_clsid_key;

            sprintf(buffer, "shell32.dll,-%u", list->idDefaultIcon);
            res = RegSetValueExA(icon_key, NULL, 0, REG_SZ,
                                 (CONST BYTE*)(buffer), strlen(buffer)+1);
            RegCloseKey(icon_key);
            if (res != ERROR_SUCCESS) goto error_close_clsid_key;
        }

	if (list->ips) {
	    res = register_key_defvalueA(clsid_key, ips_keyname, list->ips);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	}

	if (list->ips32) {
	    HKEY ips32_key;

	    res = RegCreateKeyExW(clsid_key, ips32_keyname, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL,
				  &ips32_key, NULL);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;

	    res = RegSetValueExA(ips32_key, NULL, 0, REG_SZ,
				 (CONST BYTE*)list->ips32,
				 lstrlenA(list->ips32) + 1);
	    if (res == ERROR_SUCCESS && list->ips32_tmodel)
		res = RegSetValueExA(ips32_key, tmodel_valuename, 0, REG_SZ,
				     (CONST BYTE*)list->ips32_tmodel,
				     strlen(list->ips32_tmodel) + 1);
	    RegCloseKey(ips32_key);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	}

	if (list->flags & SHELLEX_MAYCHANGEDEFAULTMENU) {
	    HKEY shellex_key, mcdm_key;

	    res = RegCreateKeyExW(clsid_key, shellex_keyname, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL,
				  &shellex_key, NULL);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	    res = RegCreateKeyExW(shellex_key, mcdm_keyname, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL,
				  &mcdm_key, NULL);
	    RegCloseKey(shellex_key);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	    RegCloseKey(mcdm_key);
	}

	if (list->flags &
		(SHELLFOLDER_WANTSFORPARSING|SHELLFOLDER_ATTRIBUTES|SHELLFOLDER_CALLFORATTRIBUTES|SHELLFOLDER_WANTSFORDISPLAY|SHELLFOLDER_HIDEASDELETEPERUSER))
	{
	    HKEY shellfolder_key;

	    res = RegCreateKeyExW(clsid_key, shellfolder_keyname, 0, NULL, 0,
			     	  KEY_READ | KEY_WRITE, NULL,
				  &shellfolder_key, NULL);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	    if (list->flags & SHELLFOLDER_WANTSFORPARSING)
		res = RegSetValueExA(shellfolder_key, wfparsing_valuename, 0, REG_SZ, (const BYTE *)"", 1);
	    if (list->flags & SHELLFOLDER_ATTRIBUTES)
		res = RegSetValueExA(shellfolder_key, attributes_valuename, 0, REG_DWORD,
				     (const BYTE *)&list->dwAttributes, sizeof(DWORD));
	    if (list->flags & SHELLFOLDER_CALLFORATTRIBUTES)
		res = RegSetValueExA(shellfolder_key, cfattributes_valuename, 0, REG_DWORD,
				     (const BYTE *)&list->dwCallForAttributes, sizeof(DWORD));
        if (list->flags & SHELLFOLDER_WANTSFORDISPLAY)
		res = RegSetValueExA(shellfolder_key, wfdisplay_valuename, 0, REG_SZ, (const BYTE *)"", 1);
        if (list->flags & SHELLFOLDER_HIDEASDELETEPERUSER)
		res = RegSetValueExA(shellfolder_key, hideasdeleteperuser_valuename, 0, REG_SZ, (const BYTE *)"", 1);
        RegCloseKey(shellfolder_key);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	}

	if (list->clsid_str) {
	    res = register_key_defvalueA(clsid_key, clsid_keyname,
					 list->clsid_str);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	}

	if (list->progid) {
	    HKEY progid_key;

	    res = register_key_defvalueA(clsid_key, progid_keyname,
					 list->progid);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;

	    res = RegCreateKeyExA(HKEY_CLASSES_ROOT, list->progid, 0,
				  NULL, 0, KEY_READ | KEY_WRITE, NULL,
				  &progid_key, NULL);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;

	    res = register_key_defvalueW(progid_key, clsid_keyname, buf);
	    RegCloseKey(progid_key);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	}
    if (IsEqualIID(list->clsid, &CLSID_RecycleBin)) {//if (list->clsid_menu) {
        HKEY shellex_key, cmenu_key, menuhandler_key;
	    res = RegCreateKeyExW(clsid_key, shellex_keyname, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL,
				  &shellex_key, NULL);
	    if (res != ERROR_SUCCESS) goto error_close_clsid_key;
	    res = RegCreateKeyExW(shellex_key, contextmenu_keyname, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL,
				  &cmenu_key, NULL);
        if (res != ERROR_SUCCESS) {
            RegCloseKey(shellex_key);
            goto error_close_clsid_key;
        }

	    StringFromGUID2(list->clsid, buf, 39); //clsid_menu
	    res = RegCreateKeyExW(cmenu_key, buf, 0, NULL, 0,
				  KEY_READ | KEY_WRITE, NULL,
				  &menuhandler_key, NULL);
        RegCloseKey(menuhandler_key);
        RegCloseKey(cmenu_key);
        RegCloseKey(shellex_key);
    }

    error_close_clsid_key:
	RegCloseKey(clsid_key);
    }

error_close_coclass_key:
    RegCloseKey(coclass_key);
error_return:
    return res != ERROR_SUCCESS ? HRESULT_FROM_WIN32(res) : S_OK;
}

/***********************************************************************
 *		unregister_coclasses
 */
static HRESULT unregister_coclasses(struct regsvr_coclass const *list)
{
    LONG res = ERROR_SUCCESS;
    HKEY coclass_key;

    res = RegOpenKeyExW(HKEY_CLASSES_ROOT, clsid_keyname, 0,
			KEY_READ | KEY_WRITE, &coclass_key);
    if (res == ERROR_FILE_NOT_FOUND) return S_OK;
    if (res != ERROR_SUCCESS) goto error_return;

    for (; res == ERROR_SUCCESS && list->clsid; ++list) {
	WCHAR buf[39];

	StringFromGUID2(list->clsid, buf, 39);
	res = RegDeleteTreeW(coclass_key, buf);
	if (res == ERROR_FILE_NOT_FOUND) res = ERROR_SUCCESS;
	if (res != ERROR_SUCCESS) goto error_close_coclass_key;

	if (list->progid) {
	    res = RegDeleteTreeA(HKEY_CLASSES_ROOT, list->progid);
	    if (res == ERROR_FILE_NOT_FOUND) res = ERROR_SUCCESS;
	    if (res != ERROR_SUCCESS) goto error_close_coclass_key;
	}
    }

error_close_coclass_key:
    RegCloseKey(coclass_key);
error_return:
    return res != ERROR_SUCCESS ? HRESULT_FROM_WIN32(res) : S_OK;
}

/**********************************************************************
 * register_namespace_extensions
 */
static WCHAR *get_namespace_key(struct regsvr_namespace const *list) {
    static const WCHAR wszExplorerKey[] = {
        'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\',
        'W','i','n','d','o','w','s','\\','C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
        'E','x','p','l','o','r','e','r','\\',0 };
    static const WCHAR wszNamespace[] = { '\\','N','a','m','e','s','p','a','c','e','\\',0 };
    WCHAR *pwszKey, *pwszCLSID;

    pwszKey = HeapAlloc(GetProcessHeap(), 0, sizeof(wszExplorerKey)+sizeof(wszNamespace)+
                                             sizeof(WCHAR)*(wcslen(list->parent)+CHARS_IN_GUID));
    if (!pwszKey)
        return NULL;

    wcscpy(pwszKey, wszExplorerKey);
    wcscat(pwszKey, list->parent);
    wcscat(pwszKey, wszNamespace);
    if (FAILED(StringFromCLSID(list->clsid, &pwszCLSID))) {
        HeapFree(GetProcessHeap(), 0, pwszKey);
        return NULL;
    }
    wcscat(pwszKey, pwszCLSID);
    CoTaskMemFree(pwszCLSID);

    return pwszKey;
}

static HRESULT register_namespace_extensions(struct regsvr_namespace const *list) {
    WCHAR *pwszKey;
    HKEY hKey;

    for (; list->clsid; list++) {
        pwszKey = get_namespace_key(list);

        /* Create the key and set the value. */
        if (pwszKey && ERROR_SUCCESS ==
            RegCreateKeyExW(HKEY_LOCAL_MACHINE, pwszKey, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL))
        {
            RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE *)list->value, sizeof(WCHAR)*(wcslen(list->value)+1));
            RegCloseKey(hKey);
        }

        HeapFree(GetProcessHeap(), 0, pwszKey);
    }
    return S_OK;
}

static HRESULT unregister_namespace_extensions(struct regsvr_namespace const *list) {
    WCHAR *pwszKey;

    for (; list->clsid; list++) {
        pwszKey = get_namespace_key(list);
        RegDeleteKeyW(HKEY_LOCAL_MACHINE, pwszKey);
        HeapFree(GetProcessHeap(), 0, pwszKey);
    }
    return S_OK;
}

/***********************************************************************
 *		regsvr_key_guid
 */
static LONG register_key_guid(HKEY base, WCHAR const *name, GUID const *guid)
{
    WCHAR buf[39];

    StringFromGUID2(guid, buf, 39);
    return register_key_defvalueW(base, name, buf);
}

/***********************************************************************
 *		regsvr_key_defvalueW
 */
static LONG register_key_defvalueW(
    HKEY base,
    WCHAR const *name,
    WCHAR const *value)
{
    LONG res;
    HKEY key;

    res = RegCreateKeyExW(base, name, 0, NULL, 0,
			  KEY_READ | KEY_WRITE, NULL, &key, NULL);
    if (res != ERROR_SUCCESS) return res;
    res = RegSetValueExW(key, NULL, 0, REG_SZ, (CONST BYTE*)value,
			 (wcslen(value) + 1) * sizeof(WCHAR));
    RegCloseKey(key);
    return res;
}

/***********************************************************************
 *		regsvr_key_defvalueA
 */
static LONG register_key_defvalueA(
    HKEY base,
    WCHAR const *name,
    char const *value)
{
    LONG res;
    HKEY key;

    res = RegCreateKeyExW(base, name, 0, NULL, 0,
			  KEY_READ | KEY_WRITE, NULL, &key, NULL);
    if (res != ERROR_SUCCESS) return res;
    res = RegSetValueExA(key, NULL, 0, REG_SZ, (CONST BYTE*)value,
			 lstrlenA(value) + 1);
    RegCloseKey(key);
    return res;
}

/***********************************************************************
 *		coclass list
 */
static GUID const CLSID_Desktop = {
    0x00021400, 0x0000, 0x0000, {0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };

static GUID const CLSID_Shortcut = {
    0x00021401, 0x0000, 0x0000, {0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46} };

static struct regsvr_coclass const coclass_list[] = {
    {   &CLSID_Desktop,
	"Desktop",
	IDS_DESKTOP,
	NULL,
	"shell32.dll",
	"Apartment"
    },
    {  &CLSID_ControlPanel,
       "Shell Control Panel Folder",
       IDS_CONTROLPANEL,
       NULL,
       "shell32.dll",
       "Apartment",
       SHELLFOLDER_WANTSFORDISPLAY|SHELLFOLDER_ATTRIBUTES|SHELLFOLDER_HIDEASDELETEPERUSER,
	   SFGAO_FOLDER|SFGAO_HASSUBFOLDER,
	   0,
	   NULL,
	   NULL,
	   IDI_SHELL_CONTROL_PANEL1
    },
    {   &CLSID_DragDropHelper,
        "Shell Drag and Drop Helper",
	0,
        NULL,
        "shell32.dll",
        "Apartment"
    },
    {   &CLSID_Printers,
        "Printers & Fax",
        IDS_PRINTERS,
        NULL,
        "shell32.dll",
        "Apartment",
        SHELLFOLDER_ATTRIBUTES,
        SFGAO_FOLDER,
        0,
        NULL,
        NULL,
        IDI_SHELL_PRINTERS_FOLDER
    },
    {   &CLSID_MyComputer,
        "My Computer",
        IDS_MYCOMPUTER,
        NULL,
        "shell32.dll",
        "Apartment"
    },
    {   &CLSID_NetworkPlaces,
        "My Network Places",
        IDS_NETWORKPLACE,
        NULL,
        "shell32.dll",
        "Apartment",
        SHELLFOLDER_ATTRIBUTES|SHELLFOLDER_CALLFORATTRIBUTES,
        SFGAO_FOLDER|SFGAO_HASPROPSHEET,
        0,
        NULL,
        NULL,
        IDI_SHELL_MY_NETWORK_PLACES
    },
    {  &CLSID_FontsFolderShortcut,
       "Fonts",
       IDS_FONTS,
       NULL,
       "shell32.dll", 
       "Apartment",
       SHELLFOLDER_ATTRIBUTES,
       SFGAO_FOLDER,
       0,
       NULL,
       NULL,
       IDI_SHELL_FONTS_FOLDER
    },
    {  &CLSID_AdminFolderShortcut,
       "Administrative Tools",
       IDS_ADMINISTRATIVETOOLS,
       NULL,
       "shell32.dll", 
       "Apartment",
       SHELLFOLDER_ATTRIBUTES,
       SFGAO_FOLDER,
       0,
       NULL,
       NULL,
       IDI_SHELL_ADMINTOOLS //FIXME
    },
    {   &CLSID_Shortcut,
	"Shortcut",
	0,
	NULL,
	"shell32.dll",
	"Apartment",
	SHELLEX_MAYCHANGEDEFAULTMENU
    },
    {   &CLSID_AutoComplete,
	"AutoComplete",
	0,
	NULL,
	"shell32.dll",
	"Apartment",
    },
    {	&CLSID_FolderShortcut,
	"Foldershortcut",
	0,
	NULL,
	"shell32.dll",
	"Apartment",
	SHELLFOLDER_ATTRIBUTES|SHELLFOLDER_CALLFORATTRIBUTES,
	SFGAO_FILESYSTEM|SFGAO_FOLDER|SFGAO_LINK,
	SFGAO_HASSUBFOLDER|SFGAO_FILESYSTEM|SFGAO_FOLDER|SFGAO_FILESYSANCESTOR
    },
    {	&CLSID_MyDocuments,
	"My Documents",
	IDS_PERSONAL,
	NULL,
	"shell32.dll",
	"Apartment",
	SHELLFOLDER_WANTSFORPARSING|SHELLFOLDER_ATTRIBUTES|SHELLFOLDER_CALLFORATTRIBUTES,
	SFGAO_FILESYSANCESTOR|SFGAO_FOLDER|SFGAO_HASSUBFOLDER,
	SFGAO_FILESYSTEM
    },
    {	&CLSID_RecycleBin,
	"Trash",
	IDS_RECYCLEBIN_FOLDER_NAME,
	NULL,
	"shell32.dll",
	"Apartment",
	SHELLFOLDER_ATTRIBUTES|SHELLFOLDER_CALLFORATTRIBUTES,
	SFGAO_FOLDER|SFGAO_DROPTARGET|SFGAO_HASPROPSHEET,
	0,
	NULL,
	NULL,
	IDI_SHELL_FULL_RECYCLE_BIN
//    &CLSID_RecycleBin
    },
    {   &CLSID_ShellFSFolder,
	"Shell File System Folder",
	0,
	NULL,
	"shell32.dll",
	"Apartment"
    },
    {   &CLSID_ShellFolderViewOC,
	"Microsoft Shell Folder View Router",
	0,
	NULL,
	"shell32.dll",
	"Apartment"
    },
    {   &CLSID_StartMenu,
	"Start Menu",
	0,
	NULL,
	"shell32.dll",
	"Apartment"
    },
    {   &CLSID_MenuBandSite,
	"Menu Site",
	0,
	NULL,
	"shell32.dll",
	"Apartment"
    },
    { NULL }			/* list terminator */
};

/***********************************************************************
 *		interface list
 */

static struct regsvr_interface const interface_list[] = {
    { NULL }			/* list terminator */
};

/***********************************************************************
 *              namespace extensions list
 */
static const WCHAR wszDesktop[] = { 'D','e','s','k','t','o','p',0 };
static const WCHAR wszSlash[] = { '/', 0 };
static const WCHAR wszMyDocuments[] = { 'M','y',' ','D','o','c','u','m','e','n','t','s', 0 };
static const WCHAR wszRecycleBin[] = { 'T','r','a','s','h', 0 };
static const WCHAR wszMyComputer[] = { 'M','y','C','o','m','p','u','t','e','r',0 };
static const WCHAR wszControlPanel[] = { 'C','o','n','t','r','o','l','P','a','n','e','l',0 };
static const WCHAR wszFolderOptions[] = { 'F','o','l','d','e','r',' ','O','p','t','i','o','n','s',0 };
static const WCHAR wszNethoodFolder[] = { 'N','e','t','h','o','o','d',' ','f','o','l','d','e','r',0};
static const WCHAR wszPrinters[] = { 'P','r','i','n','t','e','r','s',0 };
static const WCHAR wszFonts[] = { 'F','o','n','t','s',0 };
static const WCHAR wszAdminTools[] = { 'A','d','m','i','n','T','o','o','l','s',0 };
const GUID CLSID_FolderOptions = { 0x6DFD7C5C, 0x2451, 0x11d3, {0xa2,0x99,0x00,0xC0,0x4F,0x8e,0xf6,0xaf} };

static struct regsvr_namespace const namespace_extensions_list[] = {
#if 0
    {
        &CLSID_UnixDosFolder,
        wszDesktop,
        wszSlash
    },
#endif
    {
        &CLSID_MyDocuments,
        wszDesktop,
        wszMyDocuments
    },
    {
        &CLSID_NetworkPlaces,
        wszDesktop,
        wszNethoodFolder
    },
    {
        &CLSID_RecycleBin,
        wszDesktop,
        wszRecycleBin
    },
    {
        &CLSID_ControlPanel,
        wszMyComputer,
        wszControlPanel
    },
    {
        &CLSID_FolderOptions,
        wszControlPanel,
        wszFolderOptions
    },
    {
        &CLSID_FontsFolderShortcut,
        wszControlPanel,
        wszFonts
    },
    {
        &CLSID_Printers,
        wszControlPanel,
        wszPrinters
    },
    {
        &CLSID_AdminFolderShortcut,
        wszControlPanel,
        wszAdminTools
    },
    { NULL }
};

/***********************************************************************
 *		DllRegisterServer (SHELL32.@)
 */
HRESULT WINAPI DllRegisterServer(void)
{
    HRESULT hr;

    TRACE("\n");

    hr = register_coclasses(coclass_list);
    if (SUCCEEDED(hr))
	hr = register_interfaces(interface_list);
    if (SUCCEEDED(hr))
	hr = SHELL_RegisterShellFolders();
    if (SUCCEEDED(hr))
        hr = register_namespace_extensions(namespace_extensions_list);
    return hr;
}

/***********************************************************************
 *		DllUnregisterServer (SHELL32.@)
 */
HRESULT WINAPI DllUnregisterServer(void)
{
    HRESULT hr;

    TRACE("\n");

    hr = unregister_coclasses(coclass_list);
    if (SUCCEEDED(hr))
	hr = unregister_interfaces(interface_list);
    if (SUCCEEDED(hr))
        hr = unregister_namespace_extensions(namespace_extensions_list);
    return hr;
}
