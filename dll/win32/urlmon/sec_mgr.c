/*
 * Internet Security and Zone Manager
 *
 * Copyright (c) 2004 Huw D M Davies
 * Copyright 2004 Jacek Caban
 * Copyright 2009 Detlef Riekenberg
 * Copyright 2011 Thomas Mullaly for CodeWeavers
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

#include <stdio.h>

#include "urlmon_main.h"
#include "winreg.h"
#include "wininet.h"

#define NO_SHLWAPI_REG
#include "shlwapi.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(urlmon);

static const WCHAR currentlevelW[] = {'C','u','r','r','e','n','t','L','e','v','e','l',0};
static const WCHAR descriptionW[] = {'D','e','s','c','r','i','p','t','i','o','n',0};
static const WCHAR displaynameW[] = {'D','i','s','p','l','a','y','N','a','m','e',0};
static const WCHAR fileW[] = {'f','i','l','e',0};
static const WCHAR flagsW[] = {'F','l','a','g','s',0};
static const WCHAR iconW[] = {'I','c','o','n',0};
static const WCHAR minlevelW[] = {'M','i','n','L','e','v','e','l',0};
static const WCHAR recommendedlevelW[] = {'R','e','c','o','m','m','e','n','d','e','d',
                                          'L','e','v','e','l',0};
static const WCHAR wszZonesKey[] = {'S','o','f','t','w','a','r','e','\\',
                                    'M','i','c','r','o','s','o','f','t','\\',
                                    'W','i','n','d','o','w','s','\\',
                                    'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
                                    'I','n','t','e','r','n','e','t',' ','S','e','t','t','i','n','g','s','\\',
                                    'Z','o','n','e','s','\\',0};
static const WCHAR wszZoneMapDomainsKey[] = {'S','o','f','t','w','a','r','e','\\',
                                             'M','i','c','r','o','s','o','f','t','\\',
                                             'W','i','n','d','o','w','s','\\',
                                             'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
                                             'I','n','t','e','r','n','e','t',' ','S','e','t','t','i','n','g','s','\\',
                                             'Z','o','n','e','M','a','p','\\',
                                             'D','o','m','a','i','n','s',0};

/********************************************************************
 * get_string_from_reg [internal]
 *
 * helper to get a string from the reg.
 *
 */
static void get_string_from_reg(HKEY hcu, HKEY hklm, LPCWSTR name, LPWSTR out, DWORD maxlen)
{
    DWORD type = REG_SZ;
    DWORD len = maxlen * sizeof(WCHAR);
    DWORD res;

    res = RegQueryValueExW(hcu, name, NULL, &type, (LPBYTE) out, &len);

    if (res && hklm) {
        len = maxlen * sizeof(WCHAR);
        type = REG_SZ;
        res = RegQueryValueExW(hklm, name, NULL, &type, (LPBYTE) out, &len);
    }

    if (res) {
        TRACE("%s failed: %d\n", debugstr_w(name), res);
        *out = '\0';
    }
}

/********************************************************************
 * get_dword_from_reg [internal]
 *
 * helper to get a dword from the reg.
 *
 */
static void get_dword_from_reg(HKEY hcu, HKEY hklm, LPCWSTR name, LPDWORD out)
{
    DWORD type = REG_DWORD;
    DWORD len = sizeof(DWORD);
    DWORD res;

    res = RegQueryValueExW(hcu, name, NULL, &type, (LPBYTE) out, &len);

    if (res && hklm) {
        len = sizeof(DWORD);
        type = REG_DWORD;
        res = RegQueryValueExW(hklm, name, NULL, &type, (LPBYTE) out, &len);
    }

    if (res) {
        TRACE("%s failed: %d\n", debugstr_w(name), res);
        *out = 0;
    }
}

static HRESULT get_zone_from_reg(LPCWSTR schema, DWORD *zone)
{
    DWORD res, size;
    HKEY hkey;

    static const WCHAR wszZoneMapProtocolKey[] =
        {'S','o','f','t','w','a','r','e','\\',
         'M','i','c','r','o','s','o','f','t','\\',
         'W','i','n','d','o','w','s','\\',
         'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
         'I','n','t','e','r','n','e','t',' ','S','e','t','t','i','n','g','s','\\',
         'Z','o','n','e','M','a','p','\\',
         'P','r','o','t','o','c','o','l','D','e','f','a','u','l','t','s',0};

    res = RegOpenKeyW(HKEY_CURRENT_USER, wszZoneMapProtocolKey, &hkey);
    if(res != ERROR_SUCCESS) {
        ERR("Could not open key %s\n", debugstr_w(wszZoneMapProtocolKey));
        return E_UNEXPECTED;
    }

    size = sizeof(DWORD);
    res = RegQueryValueExW(hkey, schema, NULL, NULL, (PBYTE)zone, &size);
    RegCloseKey(hkey);
    if(res == ERROR_SUCCESS)
        return S_OK;

    res = RegOpenKeyW(HKEY_LOCAL_MACHINE, wszZoneMapProtocolKey, &hkey);
    if(res != ERROR_SUCCESS) {
        ERR("Could not open key %s\n", debugstr_w(wszZoneMapProtocolKey));
        return E_UNEXPECTED;
    }

    size = sizeof(DWORD);
    res = RegQueryValueExW(hkey, schema, NULL, NULL, (PBYTE)zone, &size);
    RegCloseKey(hkey);
    if(res == ERROR_SUCCESS)
        return S_OK;

    *zone = 3;
    return S_OK;
}

/********************************************************************
 * matches_domain_pattern [internal]
 *
 * Checks if the given string matches the specified domain pattern.
 *
 * This function looks for explicit wildcard domain components iff
 * they appear at the very beginning of the 'pattern' string
 *
 *  pattern = "*.google.com"
 */
static BOOL matches_domain_pattern(LPCWSTR pattern, LPCWSTR str, BOOL implicit_wildcard, LPCWSTR *matched)
{
    BOOL matches = FALSE;
    DWORD pattern_len = strlenW(pattern);
    DWORD str_len = strlenW(str);

    TRACE("(%d) Checking if %s matches %s\n", implicit_wildcard, debugstr_w(str), debugstr_w(pattern));

    *matched = NULL;
    if(str_len >= pattern_len) {
        /* Check if there's an explicit wildcard in the pattern. */
        if(pattern[0] == '*' && pattern[1] == '.') {
            /* Make sure that 'str' matches the wildcard pattern.
             *
             * Example:
             *  pattern = "*.google.com"
             *
             * So in this case 'str' would have to end with ".google.com" in order
             * to map to this pattern.
             */
            if(str_len >= pattern_len+1 && !strcmpiW(str+(str_len-pattern_len+1), pattern+1)) {
                /* Check if there's another '.' inside of the "unmatched" portion
                 * of 'str'.
                 *
                 * Example:
                 *  pattern = "*.google.com"
                 *  str     = "test.testing.google.com"
                 *
                 * The currently matched portion is ".google.com" in 'str', we need
                 * see if there's a '.' inside of the unmatched portion ("test.testing"), because
                 * if there is and 'implicit_wildcard' isn't set, then this isn't
                 * a match.
                 */
                const WCHAR *ptr;
                if(str_len > pattern_len+1 && (ptr = memrchrW(str, '.', str_len-pattern_len-2))) {
                    if(implicit_wildcard) {
                        matches = TRUE;
                        *matched = ptr+1;
                    }
                } else {
                    matches = TRUE;
                    *matched = str;
                }
            }
        } else if(implicit_wildcard && str_len > pattern_len) {
            /* When the pattern has an implicit wildcard component, it means
             * that anything goes in 'str' as long as it ends with the pattern
             * and that the beginning of the match has a '.' before it.
             *
             * Example:
             *  pattern = "google.com"
             *  str     = "www.google.com"
             *
             * Implicitly matches the pattern, where as:
             *
             *  pattern = "google.com"
             *  str     = "wwwgoogle.com"
             *
             * Doesn't match the pattern.
             */
            if(str_len > pattern_len) {
                if(str[str_len-pattern_len-1] == '.' && !strcmpiW(str+(str_len-pattern_len), pattern)) {
                    matches = TRUE;
                    *matched = str+(str_len-pattern_len);
                }
            }
        } else {
            /* The pattern doesn't have an implicit wildcard, or an explicit wildcard,
             * so 'str' has to be an exact match to the 'pattern'.
             */
            if(!strcmpiW(str, pattern)) {
                matches = TRUE;
                *matched = str;
            }
        }
    }

    if(matches)
        TRACE("Found a match: matched=%s\n", debugstr_w(*matched));
    else
        TRACE("No match found\n");

    return matches;
}

static BOOL get_zone_for_scheme(HKEY key, LPCWSTR schema, DWORD *zone)
{
    static const WCHAR wildcardW[] = {'*',0};

    DWORD res;
    DWORD size = sizeof(DWORD);
    DWORD type;

    /* See if the key contains a value for the scheme first. */
    res = RegQueryValueExW(key, schema, NULL, &type, (BYTE*)zone, &size);
    if(type != REG_DWORD)
        WARN("Unexpected value type %d for value %s, expected REG_DWORD\n", type, debugstr_w(schema));

    if(res != ERROR_SUCCESS || type != REG_DWORD) {
        /* Try to get the zone for the wildcard scheme. */
        size = sizeof(DWORD);
        res = RegQueryValueExW(key, wildcardW, NULL, &type, (BYTE*)zone, &size);
        if(type != REG_DWORD)
            WARN("Unexpected value type %d for value %s, expected REG_DWORD\n", type, debugstr_w(wildcardW));
    }

    return res == ERROR_SUCCESS && type == REG_DWORD;
}

/********************************************************************
 * search_domain_for_zone [internal]
 *
 * Searches the specified 'domain' registry key to see if 'host' maps into it, or any
 * of it's subdomain registry keys.
 *
 * Returns S_OK if a match is found, S_FALSE if no matches were found, or an error code.
 */
static HRESULT search_domain_for_zone(HKEY domains, LPCWSTR domain, DWORD domain_len, LPCWSTR schema,
                                      LPCWSTR host, DWORD host_len, DWORD *zone)
{
    BOOL found = FALSE;
    HKEY domain_key;
    DWORD res;
    LPCWSTR matched;

    if(host_len >= domain_len && matches_domain_pattern(domain, host, TRUE, &matched)) {
        res = RegOpenKeyW(domains, domain, &domain_key);
        if(res != ERROR_SUCCESS) {
            ERR("Failed to open domain key %s: %d\n", debugstr_w(domain), res);
            return E_UNEXPECTED;
        }

        if(matched == host)
            found = get_zone_for_scheme(domain_key, schema, zone);
        else {
            INT domain_offset;
            DWORD subdomain_count, subdomain_len;
            BOOL check_domain = TRUE;

            find_domain_name(domain, domain_len, &domain_offset);

            res = RegQueryInfoKeyW(domain_key, NULL, NULL, NULL, &subdomain_count, &subdomain_len,
                                   NULL, NULL, NULL, NULL, NULL, NULL);
            if(res != ERROR_SUCCESS) {
                ERR("Unable to query info for key %s: %d\n", debugstr_w(domain), res);
                RegCloseKey(domain_key);
                return E_UNEXPECTED;
            }

            if(subdomain_count) {
                WCHAR *subdomain;
                WCHAR *component;
                DWORD i;

                subdomain = heap_alloc((subdomain_len+1)*sizeof(WCHAR));
                if(!subdomain) {
                    RegCloseKey(domain_key);
                    return E_OUTOFMEMORY;
                }

                component = heap_strndupW(host, matched-host-1);
                if(!component) {
                    heap_free(subdomain);
                    RegCloseKey(domain_key);
                    return E_OUTOFMEMORY;
                }

                for(i = 0; i < subdomain_count; ++i) {
                    DWORD len = subdomain_len+1;
                    const WCHAR *sub_matched;

                    res = RegEnumKeyExW(domain_key, i, subdomain, &len, NULL, NULL, NULL, NULL);
                    if(res != ERROR_SUCCESS) {
                        heap_free(component);
                        heap_free(subdomain);
                        RegCloseKey(domain_key);
                        return E_UNEXPECTED;
                    }

                    if(matches_domain_pattern(subdomain, component, FALSE, &sub_matched)) {
                        HKEY subdomain_key;

                        res = RegOpenKeyW(domain_key, subdomain, &subdomain_key);
                        if(res != ERROR_SUCCESS) {
                            ERR("Unable to open subdomain key %s of %s: %d\n", debugstr_w(subdomain),
                                debugstr_w(domain), res);
                            heap_free(component);
                            heap_free(subdomain);
                            RegCloseKey(domain_key);
                            return E_UNEXPECTED;
                        }

                        found = get_zone_for_scheme(subdomain_key, schema, zone);
                        check_domain = FALSE;
                        RegCloseKey(subdomain_key);
                        break;
                    }
                }
                heap_free(subdomain);
                heap_free(component);
            }

            /* There's a chance that 'host' implicitly mapped into 'domain', in
             * which case we check to see if 'domain' contains zone information.
             *
             * This can only happen if 'domain' is it's own domain name.
             *  Example:
             *      "google.com" (domain name = "google.com")
             *
             *  So if:
             *      host = "www.google.com"
             *
             *  Then host would map directly into the "google.com" domain key.
             *
             * If 'domain' has more than just it's domain name, or it does not
             * have a domain name, then we don't perform the check. The reason
             * for this is that these domains don't allow implicit mappings.
             *  Example:
             *      domain = "org" (has no domain name)
             *      host   = "www.org"
             *
             *  The mapping would only happen if the "org" key had an explicit subkey
             *  called "www".
             */
            if(check_domain && !domain_offset && !strchrW(host, matched-host-1))
                found = get_zone_for_scheme(domain_key, schema, zone);
        }
        RegCloseKey(domain_key);
    }

    return found ? S_OK : S_FALSE;
}

static HRESULT search_for_domain_mapping(HKEY domains, LPCWSTR schema, LPCWSTR host, DWORD host_len, DWORD *zone)
{
    WCHAR *domain;
    DWORD domain_count, domain_len, i;
    DWORD res;
    HRESULT hres = S_FALSE;

    res = RegQueryInfoKeyW(domains, NULL, NULL, NULL, &domain_count, &domain_len,
                           NULL, NULL, NULL, NULL, NULL, NULL);
    if(res != ERROR_SUCCESS) {
        WARN("Failed to retrieve information about key\n");
        return E_UNEXPECTED;
    }

    if(!domain_count)
        return S_FALSE;

    domain = heap_alloc((domain_len+1)*sizeof(WCHAR));
    if(!domain)
        return E_OUTOFMEMORY;

    for(i = 0; i < domain_count; ++i) {
        DWORD len = domain_len+1;

        res = RegEnumKeyExW(domains, i, domain, &len, NULL, NULL, NULL, NULL);
        if(res != ERROR_SUCCESS) {
            heap_free(domain);
            return E_UNEXPECTED;
        }

        hres = search_domain_for_zone(domains, domain, len, schema, host, host_len, zone);
        if(FAILED(hres) || hres == S_OK)
            break;
    }

    heap_free(domain);
    return hres;
}

static HRESULT get_zone_from_domains(LPCWSTR url, LPCWSTR schema, DWORD *zone)
{
    HRESULT hres;
    WCHAR *host_name;
    DWORD host_len = lstrlenW(url)+1;
    DWORD res;
    HKEY domains;

    host_name = heap_alloc(host_len*sizeof(WCHAR));
    if(!host_name)
        return E_OUTOFMEMORY;

    hres = CoInternetParseUrl(url, PARSE_DOMAIN, 0, host_name, host_len, &host_len, 0);
    if(hres == S_FALSE) {
        WCHAR *tmp = heap_realloc(host_name, (host_len+1)*sizeof(WCHAR));
        if(!tmp) {
            heap_free(host_name);
            return E_OUTOFMEMORY;
        }

        host_name = tmp;
        hres = CoInternetParseUrl(url, PARSE_DOMAIN, 0, host_name, host_len+1, &host_len, 0);
    }

    /* Windows doesn't play nice with unknown scheme types when it tries
     * to check if a host name maps into any domains.
     *
     * The reason is with how CoInternetParseUrl handles unknown scheme types
     * when it's parsing the domain of a URL (IE it always returns E_FAIL).
     *
     * Windows doesn't compenstate for this and simply doesn't check if
     * the URL maps into any domains.
     */
    if(hres != S_OK) {
        heap_free(host_name);
        if(hres == E_FAIL)
            return S_FALSE;
        return hres;
    }

    /* First try CURRENT_USER. */
    res = RegOpenKeyW(HKEY_CURRENT_USER, wszZoneMapDomainsKey, &domains);
    if(res == ERROR_SUCCESS) {
        hres = search_for_domain_mapping(domains, schema, host_name, host_len, zone);
        RegCloseKey(domains);
    } else
        WARN("Failed to open HKCU's %s key\n", debugstr_w(wszZoneMapDomainsKey));

    /* If that doesn't work try LOCAL_MACHINE. */
    if(hres == S_FALSE) {
        res = RegOpenKeyW(HKEY_LOCAL_MACHINE, wszZoneMapDomainsKey, &domains);
        if(res == ERROR_SUCCESS) {
            hres = search_for_domain_mapping(domains, schema, host_name, host_len, zone);
            RegCloseKey(domains);
        } else
            WARN("Failed to open HKLM's %s key\n", debugstr_w(wszZoneMapDomainsKey));
    }

    heap_free(host_name);
    return hres;
}

static HRESULT map_url_to_zone(LPCWSTR url, DWORD *zone, LPWSTR *ret_url)
{
    LPWSTR secur_url;
    WCHAR schema[64];
    DWORD size=0;
    HRESULT hres;

    *zone = URLZONE_INVALID;

    hres = CoInternetGetSecurityUrl(url, &secur_url, PSU_SECURITY_URL_ONLY, 0);
    if(hres != S_OK) {
        size = strlenW(url)*sizeof(WCHAR);

        secur_url = heap_alloc(size);
        if(!secur_url)
            return E_OUTOFMEMORY;

        memcpy(secur_url, url, size);
    }

    hres = CoInternetParseUrl(secur_url, PARSE_SCHEMA, 0, schema, sizeof(schema)/sizeof(WCHAR), &size, 0);
    if(FAILED(hres) || !*schema) {
        heap_free(secur_url);
        return E_INVALIDARG;
    }

    /* file protocol is a special case */
    if(!strcmpW(schema, fileW)) {
        WCHAR path[MAX_PATH], root[20];
        WCHAR *ptr;

        hres = CoInternetParseUrl(secur_url, PARSE_PATH_FROM_URL, 0, path,
                sizeof(path)/sizeof(WCHAR), &size, 0);

        if(SUCCEEDED(hres) && (ptr = strchrW(path, '\\')) && ptr-path < sizeof(root)/sizeof(WCHAR)) {
            UINT type;

            memcpy(root, path, (ptr-path)*sizeof(WCHAR));
            root[ptr-path] = 0;

            type = GetDriveTypeW(root);

            switch(type) {
            case DRIVE_UNKNOWN:
            case DRIVE_NO_ROOT_DIR:
                break;
            case DRIVE_REMOVABLE:
            case DRIVE_FIXED:
            case DRIVE_CDROM:
            case DRIVE_RAMDISK:
                *zone = URLZONE_LOCAL_MACHINE;
                hres = S_OK;
                break;
            case DRIVE_REMOTE:
                *zone = URLZONE_INTERNET;
                hres = S_OK;
                break;
            default:
                FIXME("unsupported drive type %d\n", type);
            }
        }
    }

    if(*zone == URLZONE_INVALID) {
        hres = get_zone_from_domains(secur_url, schema, zone);
        if(hres == S_FALSE)
            hres = get_zone_from_reg(schema, zone);
    }

    if(FAILED(hres) || !ret_url)
        heap_free(secur_url);
    else
        *ret_url = secur_url;

    return hres;
}

static HRESULT open_zone_key(HKEY parent_key, DWORD zone, HKEY *hkey)
{
    static const WCHAR wszFormat[] = {'%','s','%','u',0};

    WCHAR key_name[sizeof(wszZonesKey)/sizeof(WCHAR)+12];
    DWORD res;

    wsprintfW(key_name, wszFormat, wszZonesKey, zone);

    res = RegOpenKeyW(parent_key, key_name, hkey);

    if(res != ERROR_SUCCESS) {
        WARN("RegOpenKey failed\n");
        return E_INVALIDARG;
    }

    return S_OK;
}

static HRESULT get_action_policy(DWORD zone, DWORD action, BYTE *policy, DWORD size, URLZONEREG zone_reg)
{
    HKEY parent_key;
    HKEY hkey;
    LONG res;
    HRESULT hres;

    switch(action) {
    case URLACTION_SCRIPT_OVERRIDE_SAFETY:
    case URLACTION_ACTIVEX_OVERRIDE_SCRIPT_SAFETY:
        *(DWORD*)policy = URLPOLICY_DISALLOW;
        return S_OK;
    }

    switch(zone_reg) {
    case URLZONEREG_DEFAULT:
    case URLZONEREG_HKCU:
        parent_key = HKEY_CURRENT_USER;
        break;
    case URLZONEREG_HKLM:
        parent_key = HKEY_LOCAL_MACHINE;
        break;
    default:
        WARN("Unknown URLZONEREG: %d\n", zone_reg);
        return E_FAIL;
    };

    hres = open_zone_key(parent_key, zone, &hkey);
    if(SUCCEEDED(hres)) {
        WCHAR action_str[16];
        DWORD len = size;

        static const WCHAR formatW[] = {'%','X',0};

        wsprintfW(action_str, formatW, action);

        res = RegQueryValueExW(hkey, action_str, NULL, NULL, policy, &len);
        if(res == ERROR_MORE_DATA) {
            hres = E_INVALIDARG;
        }else if(res == ERROR_FILE_NOT_FOUND) {
            hres = E_FAIL;
        }else if(res != ERROR_SUCCESS) {
            ERR("RegQueryValue failed: %d\n", res);
            hres = E_UNEXPECTED;
        }

        RegCloseKey(hkey);
    }

    if(FAILED(hres) && zone_reg == URLZONEREG_DEFAULT)
        return get_action_policy(zone, action, policy, size, URLZONEREG_HKLM);

    return hres;
}

/***********************************************************************
 *           InternetSecurityManager implementation
 *
 */
typedef struct {
    IInternetSecurityManager IInternetSecurityManager_iface;

    LONG ref;

    IInternetSecurityMgrSite *mgrsite;
    IInternetSecurityManager *custom_manager;
} SecManagerImpl;

static inline SecManagerImpl *impl_from_IInternetSecurityManager(IInternetSecurityManager *iface)
{
    return CONTAINING_RECORD(iface, SecManagerImpl, IInternetSecurityManager_iface);
}

static HRESULT WINAPI SecManagerImpl_QueryInterface(IInternetSecurityManager* iface,REFIID riid,void** ppvObject)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);

    TRACE("(%p)->(%s,%p)\n",This,debugstr_guid(riid),ppvObject);

    /* Perform a sanity check on the parameters.*/
    if ( (This==0) || (ppvObject==0) )
	return E_INVALIDARG;

    /* Initialize the return parameter */
    *ppvObject = 0;

    /* Compare the riid with the interface IDs implemented by this object.*/
    if (IsEqualIID(&IID_IUnknown, riid) ||
        IsEqualIID(&IID_IInternetSecurityManager, riid))
        *ppvObject = iface;

    /* Check that we obtained an interface.*/
    if (!*ppvObject) {
        WARN("not supported interface %s\n", debugstr_guid(riid));
        return E_NOINTERFACE;
    }

    /* Query Interface always increases the reference count by one when it is successful */
    IInternetSecurityManager_AddRef(iface);

    return S_OK;
}

static ULONG WINAPI SecManagerImpl_AddRef(IInternetSecurityManager* iface)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    ULONG refCount = InterlockedIncrement(&This->ref);

    TRACE("(%p) ref=%u\n", This, refCount);

    return refCount;
}

static ULONG WINAPI SecManagerImpl_Release(IInternetSecurityManager* iface)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    ULONG refCount = InterlockedDecrement(&This->ref);

    TRACE("(%p) ref=%u\n", This, refCount);

    /* destroy the object if there's no more reference on it */
    if (!refCount){
        if(This->mgrsite)
            IInternetSecurityMgrSite_Release(This->mgrsite);
        if(This->custom_manager)
            IInternetSecurityManager_Release(This->custom_manager);

        heap_free(This);

        URLMON_UnlockModule();
    }

    return refCount;
}

static HRESULT WINAPI SecManagerImpl_SetSecuritySite(IInternetSecurityManager *iface,
                                                     IInternetSecurityMgrSite *pSite)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);

    TRACE("(%p)->(%p)\n", This, pSite);

    if(This->mgrsite)
        IInternetSecurityMgrSite_Release(This->mgrsite);

    if(This->custom_manager) {
        IInternetSecurityManager_Release(This->custom_manager);
        This->custom_manager = NULL;
    }

    This->mgrsite = pSite;

    if(pSite) {
        IServiceProvider *servprov;
        HRESULT hres;

        IInternetSecurityMgrSite_AddRef(pSite);

        hres = IInternetSecurityMgrSite_QueryInterface(pSite, &IID_IServiceProvider,
                (void**)&servprov);
        if(SUCCEEDED(hres)) {
            IServiceProvider_QueryService(servprov, &SID_SInternetSecurityManager,
                    &IID_IInternetSecurityManager, (void**)&This->custom_manager);
            IServiceProvider_Release(servprov);
        }
    }

    return S_OK;
}

static HRESULT WINAPI SecManagerImpl_GetSecuritySite(IInternetSecurityManager *iface,
                                                     IInternetSecurityMgrSite **ppSite)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);

    TRACE("(%p)->(%p)\n", This, ppSite);

    if(!ppSite)
        return E_INVALIDARG;

    if(This->mgrsite)
        IInternetSecurityMgrSite_AddRef(This->mgrsite);

    *ppSite = This->mgrsite;
    return S_OK;
}

static HRESULT WINAPI SecManagerImpl_MapUrlToZone(IInternetSecurityManager *iface,
                                                  LPCWSTR pwszUrl, DWORD *pdwZone,
                                                  DWORD dwFlags)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    HRESULT hres;

    TRACE("(%p)->(%s %p %08x)\n", iface, debugstr_w(pwszUrl), pdwZone, dwFlags);

    if(This->custom_manager) {
        hres = IInternetSecurityManager_MapUrlToZone(This->custom_manager,
                pwszUrl, pdwZone, dwFlags);
        if(hres != INET_E_DEFAULT_ACTION)
            return hres;
    }

    if(!pwszUrl) {
        *pdwZone = URLZONE_INVALID;
        return E_INVALIDARG;
    }

    if(dwFlags)
        FIXME("not supported flags: %08x\n", dwFlags);

    return map_url_to_zone(pwszUrl, pdwZone, NULL);
}

static HRESULT WINAPI SecManagerImpl_GetSecurityId(IInternetSecurityManager *iface, 
        LPCWSTR pwszUrl, BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    LPWSTR url, ptr, ptr2;
    DWORD zone, len;
    HRESULT hres;

    static const WCHAR wszFile[] = {'f','i','l','e',':'};

    TRACE("(%p)->(%s %p %p %08lx)\n", iface, debugstr_w(pwszUrl), pbSecurityId,
          pcbSecurityId, dwReserved);

    if(This->custom_manager) {
        hres = IInternetSecurityManager_GetSecurityId(This->custom_manager,
                pwszUrl, pbSecurityId, pcbSecurityId, dwReserved);
        if(hres != INET_E_DEFAULT_ACTION)
            return hres;
    }

    if(!pwszUrl || !pbSecurityId || !pcbSecurityId)
        return E_INVALIDARG;

    if(dwReserved)
        FIXME("dwReserved is not supported\n");

    hres = map_url_to_zone(pwszUrl, &zone, &url);
    if(FAILED(hres))
        return hres == 0x80041001 ? E_INVALIDARG : hres;

    /* file protocol is a special case */
    if(strlenW(url) >= sizeof(wszFile)/sizeof(WCHAR)
            && !memcmp(url, wszFile, sizeof(wszFile)) && strchrW(url, '\\')) {

        static const BYTE secidFile[] = {'f','i','l','e',':'};

        heap_free(url);

        if(*pcbSecurityId < sizeof(secidFile)+sizeof(zone))
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

        memcpy(pbSecurityId, secidFile, sizeof(secidFile));
        *(DWORD*)(pbSecurityId+sizeof(secidFile)) = zone;

        *pcbSecurityId = sizeof(secidFile)+sizeof(zone);
        return S_OK;
    }

    ptr = strchrW(url, ':');
    ptr2 = ++ptr;
    while(*ptr2 == '/')
        ptr2++;
    if(ptr2 != ptr)
        memmove(ptr, ptr2, (strlenW(ptr2)+1)*sizeof(WCHAR));

    ptr = strchrW(ptr, '/');
    if(ptr)
        *ptr = 0;

    len = WideCharToMultiByte(CP_ACP, 0, url, -1, NULL, 0, NULL, NULL)-1;

    if(len+sizeof(DWORD) > *pcbSecurityId) {
        heap_free(url);
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    WideCharToMultiByte(CP_ACP, 0, url, -1, (LPSTR)pbSecurityId, len, NULL, NULL);
    heap_free(url);

    *(DWORD*)(pbSecurityId+len) = zone;

    *pcbSecurityId = len+sizeof(DWORD);

    return S_OK;
}


static HRESULT WINAPI SecManagerImpl_ProcessUrlAction(IInternetSecurityManager *iface,
                                                      LPCWSTR pwszUrl, DWORD dwAction,
                                                      BYTE *pPolicy, DWORD cbPolicy,
                                                      BYTE *pContext, DWORD cbContext,
                                                      DWORD dwFlags, DWORD dwReserved)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    DWORD zone, policy;
    HRESULT hres;

    TRACE("(%p)->(%s %08x %p %08x %p %08x %08x %08x)\n", iface, debugstr_w(pwszUrl), dwAction,
          pPolicy, cbPolicy, pContext, cbContext, dwFlags, dwReserved);

    if(This->custom_manager) {
        hres = IInternetSecurityManager_ProcessUrlAction(This->custom_manager, pwszUrl, dwAction,
                pPolicy, cbPolicy, pContext, cbContext, dwFlags, dwReserved);
        if(hres != INET_E_DEFAULT_ACTION)
            return hres;
    }

    if(dwFlags || dwReserved)
        FIXME("Unsupported arguments\n");

    if(!pwszUrl)
        return E_INVALIDARG;

    hres = map_url_to_zone(pwszUrl, &zone, NULL);
    if(FAILED(hres))
        return hres;

    hres = get_action_policy(zone, dwAction, (BYTE*)&policy, sizeof(policy), URLZONEREG_DEFAULT);
    if(FAILED(hres))
        return hres;

    TRACE("policy %x\n", policy);
    if(cbPolicy >= sizeof(DWORD))
        *(DWORD*)pPolicy = policy;

    switch(GetUrlPolicyPermissions(policy)) {
    case URLPOLICY_ALLOW:
    case URLPOLICY_CHANNEL_SOFTDIST_PRECACHE:
        return S_OK;
    case URLPOLICY_DISALLOW:
        return S_FALSE;
    case URLPOLICY_QUERY:
        FIXME("URLPOLICY_QUERY not implemented\n");
        return E_FAIL;
    default:
        FIXME("Not implemented policy %x\n", policy);
    }

    return E_FAIL;
}
                                               

static HRESULT WINAPI SecManagerImpl_QueryCustomPolicy(IInternetSecurityManager *iface,
                                                       LPCWSTR pwszUrl, REFGUID guidKey,
                                                       BYTE **ppPolicy, DWORD *pcbPolicy,
                                                       BYTE *pContext, DWORD cbContext,
                                                       DWORD dwReserved)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    HRESULT hres;

    TRACE("(%p)->(%s %s %p %p %p %08x %08x )\n", iface, debugstr_w(pwszUrl), debugstr_guid(guidKey),
          ppPolicy, pcbPolicy, pContext, cbContext, dwReserved);

    if(This->custom_manager) {
        hres = IInternetSecurityManager_QueryCustomPolicy(This->custom_manager, pwszUrl, guidKey,
                ppPolicy, pcbPolicy, pContext, cbContext, dwReserved);
        if(hres != INET_E_DEFAULT_ACTION)
            return hres;
    }

    WARN("Unknown guidKey %s\n", debugstr_guid(guidKey));
    return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
}

static HRESULT WINAPI SecManagerImpl_SetZoneMapping(IInternetSecurityManager *iface,
                                                    DWORD dwZone, LPCWSTR pwszPattern, DWORD dwFlags)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    HRESULT hres;

    TRACE("(%p)->(%08x %s %08x)\n", iface, dwZone, debugstr_w(pwszPattern),dwFlags);

    if(This->custom_manager) {
        hres = IInternetSecurityManager_SetZoneMapping(This->custom_manager, dwZone,
                pwszPattern, dwFlags);
        if(hres != INET_E_DEFAULT_ACTION)
            return hres;
    }

    FIXME("Default action is not implemented\n");
    return E_NOTIMPL;
}

static HRESULT WINAPI SecManagerImpl_GetZoneMappings(IInternetSecurityManager *iface,
        DWORD dwZone, IEnumString **ppenumString, DWORD dwFlags)
{
    SecManagerImpl *This = impl_from_IInternetSecurityManager(iface);
    HRESULT hres;

    TRACE("(%p)->(%08x %p %08x)\n", iface, dwZone, ppenumString,dwFlags);

    if(This->custom_manager) {
        hres = IInternetSecurityManager_GetZoneMappings(This->custom_manager, dwZone,
                ppenumString, dwFlags);
        if(hres != INET_E_DEFAULT_ACTION)
            return hres;
    }

    FIXME("Default action is not implemented\n");
    return E_NOTIMPL;
}

static const IInternetSecurityManagerVtbl VT_SecManagerImpl =
{
    SecManagerImpl_QueryInterface,
    SecManagerImpl_AddRef,
    SecManagerImpl_Release,
    SecManagerImpl_SetSecuritySite,
    SecManagerImpl_GetSecuritySite,
    SecManagerImpl_MapUrlToZone,
    SecManagerImpl_GetSecurityId,
    SecManagerImpl_ProcessUrlAction,
    SecManagerImpl_QueryCustomPolicy,
    SecManagerImpl_SetZoneMapping,
    SecManagerImpl_GetZoneMappings
};

HRESULT SecManagerImpl_Construct(IUnknown *pUnkOuter, LPVOID *ppobj)
{
    SecManagerImpl *This;

    TRACE("(%p,%p)\n",pUnkOuter,ppobj);
    This = heap_alloc(sizeof(*This));

    /* Initialize the virtual function table. */
    This->IInternetSecurityManager_iface.lpVtbl = &VT_SecManagerImpl;

    This->ref = 1;
    This->mgrsite = NULL;
    This->custom_manager = NULL;

    *ppobj = This;

    URLMON_LockModule();

    return S_OK;
}

/***********************************************************************
 *           InternetZoneManager implementation
 *
 */
typedef struct {
    IInternetZoneManagerEx2 IInternetZoneManagerEx2_iface;
    LONG ref;
    LPDWORD *zonemaps;
    DWORD zonemap_count;
} ZoneMgrImpl;

static inline ZoneMgrImpl *impl_from_IInternetZoneManagerEx2(IInternetZoneManagerEx2 *iface)
{
    return CONTAINING_RECORD(iface, ZoneMgrImpl, IInternetZoneManagerEx2_iface);
}


/***********************************************************************
 * build_zonemap_from_reg [internal]
 *
 * Enumerate the Zones in the Registry and return the Zones in a DWORD-array
 * The number of the Zones is returned in data[0]
 */
static LPDWORD build_zonemap_from_reg(void)
{
    WCHAR name[32];
    HKEY hkey;
    LPDWORD data = NULL;
    DWORD allocated = 6; /* space for the zonecount and Zone "0" up to Zone "4" */
    DWORD used = 0;
    DWORD res;
    DWORD len;


    res = RegOpenKeyW(HKEY_CURRENT_USER, wszZonesKey, &hkey);
    if (res)
        return NULL;

    data = heap_alloc(allocated * sizeof(DWORD));
    if (!data)
        goto cleanup;

    while (!res) {
        name[0] = '\0';
        len = sizeof(name) / sizeof(name[0]);
        res = RegEnumKeyExW(hkey, used, name, &len, NULL, NULL, NULL, NULL);

        if (!res) {
            used++;
            if (used == allocated) {
                LPDWORD new_data;

                allocated *= 2;
                new_data = heap_realloc_zero(data, allocated * sizeof(DWORD));
                if (!new_data)
                    goto cleanup;

                data = new_data;
            }
            data[used] = atoiW(name);
        }
    }
    if (used) {
        RegCloseKey(hkey);
        data[0] = used;
        return data;
    }

cleanup:
    /* something failed */
    RegCloseKey(hkey);
    heap_free(data);
    return NULL;
}

/********************************************************************
 *      IInternetZoneManager_QueryInterface
 */
static HRESULT WINAPI ZoneMgrImpl_QueryInterface(IInternetZoneManagerEx2* iface, REFIID riid, void** ppvObject)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);

    TRACE("(%p)->(%s,%p)\n", This, debugstr_guid(riid), ppvObject);

    if(!This || !ppvObject)
        return E_INVALIDARG;

    if(IsEqualIID(&IID_IUnknown, riid)) {
        TRACE("(%p)->(IID_IUnknown %p)\n", This, ppvObject);
    }else if(IsEqualIID(&IID_IInternetZoneManager, riid)) {
        TRACE("(%p)->(IID_InternetZoneManager %p)\n", This, ppvObject);
    }else if(IsEqualIID(&IID_IInternetZoneManagerEx, riid)) {
        TRACE("(%p)->(IID_InternetZoneManagerEx %p)\n", This, ppvObject);
    }else if(IsEqualIID(&IID_IInternetZoneManagerEx2, riid)) {
        TRACE("(%p)->(IID_InternetZoneManagerEx2 %p)\n", This, ppvObject);
    }
    else
    {
        FIXME("Unknown interface: %s\n", debugstr_guid(riid));
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    *ppvObject = iface;
    IInternetZoneManager_AddRef(iface);
    return S_OK;
}

/********************************************************************
 *      IInternetZoneManager_AddRef
 */
static ULONG WINAPI ZoneMgrImpl_AddRef(IInternetZoneManagerEx2* iface)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    ULONG refCount = InterlockedIncrement(&This->ref);

    TRACE("(%p)->(ref before=%u)\n",This, refCount - 1);

    return refCount;
}

/********************************************************************
 *      IInternetZoneManager_Release
 */
static ULONG WINAPI ZoneMgrImpl_Release(IInternetZoneManagerEx2* iface)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    ULONG refCount = InterlockedDecrement(&This->ref);

    TRACE("(%p)->(ref before=%u)\n",This, refCount + 1);

    if(!refCount) {
        while (This->zonemap_count) heap_free(This->zonemaps[--This->zonemap_count]);
        heap_free(This->zonemaps);
        heap_free(This);
        URLMON_UnlockModule();
    }
    
    return refCount;
}

/********************************************************************
 *      IInternetZoneManager_GetZoneAttributes
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneAttributes(IInternetZoneManagerEx2* iface,
                                                    DWORD dwZone,
                                                    ZONEATTRIBUTES* pZoneAttributes)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    HRESULT hr;
    HKEY hcu;
    HKEY hklm = NULL;

    TRACE("(%p)->(%d %p)\n", This, dwZone, pZoneAttributes);

    if (!pZoneAttributes)
        return E_INVALIDARG;

    hr = open_zone_key(HKEY_CURRENT_USER, dwZone, &hcu);
    if (FAILED(hr))
        return S_OK;  /* IE6 and older returned E_FAIL here */

    hr = open_zone_key(HKEY_LOCAL_MACHINE, dwZone, &hklm);
    if (FAILED(hr))
        TRACE("Zone %d not in HKLM\n", dwZone);

    get_string_from_reg(hcu, hklm, displaynameW, pZoneAttributes->szDisplayName, MAX_ZONE_PATH);
    get_string_from_reg(hcu, hklm, descriptionW, pZoneAttributes->szDescription, MAX_ZONE_DESCRIPTION);
    get_string_from_reg(hcu, hklm, iconW, pZoneAttributes->szIconPath, MAX_ZONE_PATH);
    get_dword_from_reg(hcu, hklm, minlevelW, &pZoneAttributes->dwTemplateMinLevel);
    get_dword_from_reg(hcu, hklm, currentlevelW, &pZoneAttributes->dwTemplateCurrentLevel);
    get_dword_from_reg(hcu, hklm, recommendedlevelW, &pZoneAttributes->dwTemplateRecommended);
    get_dword_from_reg(hcu, hklm, flagsW, &pZoneAttributes->dwFlags);

    RegCloseKey(hklm);
    RegCloseKey(hcu);
    return S_OK;
}

/********************************************************************
 *      IInternetZoneManager_SetZoneAttributes
 */
static HRESULT WINAPI ZoneMgrImpl_SetZoneAttributes(IInternetZoneManagerEx2* iface,
                                                    DWORD dwZone,
                                                    ZONEATTRIBUTES* pZoneAttributes)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    HRESULT hr;
    HKEY hcu;

    TRACE("(%p)->(%d %p)\n", This, dwZone, pZoneAttributes);

    if (!pZoneAttributes)
        return E_INVALIDARG;

    hr = open_zone_key(HKEY_CURRENT_USER, dwZone, &hcu);
    if (FAILED(hr))
        return S_OK;  /* IE6 returned E_FAIL here */

    /* cbSize is ignored */
    RegSetValueExW(hcu, displaynameW, 0, REG_SZ, (LPBYTE) pZoneAttributes->szDisplayName,
                    (lstrlenW(pZoneAttributes->szDisplayName)+1)* sizeof(WCHAR));

    RegSetValueExW(hcu, descriptionW, 0, REG_SZ, (LPBYTE) pZoneAttributes->szDescription,
                    (lstrlenW(pZoneAttributes->szDescription)+1)* sizeof(WCHAR));

    RegSetValueExW(hcu, iconW, 0, REG_SZ, (LPBYTE) pZoneAttributes->szIconPath,
                    (lstrlenW(pZoneAttributes->szIconPath)+1)* sizeof(WCHAR));

    RegSetValueExW(hcu, minlevelW, 0, REG_DWORD,
                    (const BYTE*) &pZoneAttributes->dwTemplateMinLevel, sizeof(DWORD));

    RegSetValueExW(hcu, currentlevelW, 0, REG_DWORD,
                    (const BYTE*) &pZoneAttributes->dwTemplateCurrentLevel, sizeof(DWORD));

    RegSetValueExW(hcu, recommendedlevelW, 0, REG_DWORD,
                    (const BYTE*) &pZoneAttributes->dwTemplateRecommended, sizeof(DWORD));

    RegSetValueExW(hcu, flagsW, 0, REG_DWORD, (const BYTE*) &pZoneAttributes->dwFlags, sizeof(DWORD));
    RegCloseKey(hcu);
    return S_OK;

}

/********************************************************************
 *      IInternetZoneManager_GetZoneCustomPolicy
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneCustomPolicy(IInternetZoneManagerEx2* iface,
                                                      DWORD dwZone,
                                                      REFGUID guidKey,
                                                      BYTE** ppPolicy,
                                                      DWORD* pcbPolicy,
                                                      URLZONEREG ulrZoneReg)
{
    FIXME("(%p)->(%08x %s %p %p %08x) stub\n", iface, dwZone, debugstr_guid(guidKey),
                                                    ppPolicy, pcbPolicy, ulrZoneReg);
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManager_SetZoneCustomPolicy
 */
static HRESULT WINAPI ZoneMgrImpl_SetZoneCustomPolicy(IInternetZoneManagerEx2* iface,
                                                      DWORD dwZone,
                                                      REFGUID guidKey,
                                                      BYTE* ppPolicy,
                                                      DWORD cbPolicy,
                                                      URLZONEREG ulrZoneReg)
{
    FIXME("(%p)->(%08x %s %p %08x %08x) stub\n", iface, dwZone, debugstr_guid(guidKey),
                                                    ppPolicy, cbPolicy, ulrZoneReg);
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManager_GetZoneActionPolicy
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneActionPolicy(IInternetZoneManagerEx2* iface,
        DWORD dwZone, DWORD dwAction, BYTE* pPolicy, DWORD cbPolicy, URLZONEREG urlZoneReg)
{
    TRACE("(%p)->(%d %08x %p %d %d)\n", iface, dwZone, dwAction, pPolicy,
            cbPolicy, urlZoneReg);

    if(!pPolicy)
        return E_INVALIDARG;

    return get_action_policy(dwZone, dwAction, pPolicy, cbPolicy, urlZoneReg);
}

/********************************************************************
 *      IInternetZoneManager_SetZoneActionPolicy
 */
static HRESULT WINAPI ZoneMgrImpl_SetZoneActionPolicy(IInternetZoneManagerEx2* iface,
                                                      DWORD dwZone,
                                                      DWORD dwAction,
                                                      BYTE* pPolicy,
                                                      DWORD cbPolicy,
                                                      URLZONEREG urlZoneReg)
{
    FIXME("(%p)->(%08x %08x %p %08x %08x) stub\n", iface, dwZone, dwAction, pPolicy,
                                                       cbPolicy, urlZoneReg);
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManager_PromptAction
 */
static HRESULT WINAPI ZoneMgrImpl_PromptAction(IInternetZoneManagerEx2* iface,
                                               DWORD dwAction,
                                               HWND hwndParent,
                                               LPCWSTR pwszUrl,
                                               LPCWSTR pwszText,
                                               DWORD dwPromptFlags)
{
    FIXME("%p %08x %p %s %s %08x\n", iface, dwAction, hwndParent,
          debugstr_w(pwszUrl), debugstr_w(pwszText), dwPromptFlags );
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManager_LogAction
 */
static HRESULT WINAPI ZoneMgrImpl_LogAction(IInternetZoneManagerEx2* iface,
                                            DWORD dwAction,
                                            LPCWSTR pwszUrl,
                                            LPCWSTR pwszText,
                                            DWORD dwLogFlags)
{
    FIXME("(%p)->(%08x %s %s %08x) stub\n", iface, dwAction, debugstr_w(pwszUrl),
                                              debugstr_w(pwszText), dwLogFlags);
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManager_CreateZoneEnumerator
 */
static HRESULT WINAPI ZoneMgrImpl_CreateZoneEnumerator(IInternetZoneManagerEx2* iface,
                                                       DWORD* pdwEnum,
                                                       DWORD* pdwCount,
                                                       DWORD dwFlags)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    LPDWORD * new_maps;
    LPDWORD data;
    DWORD i;

    TRACE("(%p)->(%p, %p, 0x%08x)\n", This, pdwEnum, pdwCount, dwFlags);
    if (!pdwEnum || !pdwCount || (dwFlags != 0))
        return E_INVALIDARG;

    data = build_zonemap_from_reg();
    TRACE("found %d zones\n", data ? data[0] : -1);

    if (!data)
        return E_FAIL;

    for (i = 0; i < This->zonemap_count; i++) {
        if (This->zonemaps && !This->zonemaps[i]) {
            This->zonemaps[i] = data;
            *pdwEnum = i;
            *pdwCount = data[0];
            return S_OK;
        }
    }

    if (This->zonemaps) {
        /* try to double the nr. of pointers in the array */
        new_maps = heap_realloc_zero(This->zonemaps, This->zonemap_count * 2 * sizeof(LPDWORD));
        if (new_maps)
            This->zonemap_count *= 2;
    }
    else
    {
        This->zonemap_count = 2;
        new_maps = heap_alloc_zero(This->zonemap_count * sizeof(LPDWORD));
    }

    if (!new_maps) {
        heap_free(data);
        return E_FAIL;
    }
    This->zonemaps = new_maps;
    This->zonemaps[i] = data;
    *pdwEnum = i;
    *pdwCount = data[0];
    return S_OK;
}

/********************************************************************
 *      IInternetZoneManager_GetZoneAt
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneAt(IInternetZoneManagerEx2* iface,
                                            DWORD dwEnum,
                                            DWORD dwIndex,
                                            DWORD* pdwZone)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    LPDWORD data;

    TRACE("(%p)->(0x%08x, %d, %p)\n", This, dwEnum, dwIndex, pdwZone);

    /* make sure, that dwEnum and dwIndex are in the valid range */
    if (dwEnum < This->zonemap_count) {
        if ((data = This->zonemaps[dwEnum])) {
            if (dwIndex < data[0]) {
                *pdwZone = data[dwIndex + 1];
                return S_OK;
            }
        }
    }
    return E_INVALIDARG;
}

/********************************************************************
 *      IInternetZoneManager_DestroyZoneEnumerator
 */
static HRESULT WINAPI ZoneMgrImpl_DestroyZoneEnumerator(IInternetZoneManagerEx2* iface,
                                                        DWORD dwEnum)
{
    ZoneMgrImpl* This = impl_from_IInternetZoneManagerEx2(iface);
    LPDWORD data;

    TRACE("(%p)->(0x%08x)\n", This, dwEnum);
    /* make sure, that dwEnum is valid */
    if (dwEnum < This->zonemap_count) {
        if ((data = This->zonemaps[dwEnum])) {
            This->zonemaps[dwEnum] = NULL;
            heap_free(data);
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

/********************************************************************
 *      IInternetZoneManager_CopyTemplatePoliciesToZone
 */
static HRESULT WINAPI ZoneMgrImpl_CopyTemplatePoliciesToZone(IInternetZoneManagerEx2* iface,
                                                             DWORD dwTemplate,
                                                             DWORD dwZone,
                                                             DWORD dwReserved)
{
    FIXME("(%p)->(%08x %08x %08x) stub\n", iface, dwTemplate, dwZone, dwReserved);
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManagerEx_GetZoneActionPolicyEx
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneActionPolicyEx(IInternetZoneManagerEx2* iface,
                                                        DWORD dwZone,
                                                        DWORD dwAction,
                                                        BYTE* pPolicy,
                                                        DWORD cbPolicy,
                                                        URLZONEREG urlZoneReg,
                                                        DWORD dwFlags)
{
    TRACE("(%p)->(%d, 0x%x, %p, %d, %d, 0x%x)\n", iface, dwZone,
            dwAction, pPolicy, cbPolicy, urlZoneReg, dwFlags);

    if(!pPolicy)
        return E_INVALIDARG;

    if (dwFlags)
        FIXME("dwFlags 0x%x ignored\n", dwFlags);

    return get_action_policy(dwZone, dwAction, pPolicy, cbPolicy, urlZoneReg);
}

/********************************************************************
 *      IInternetZoneManagerEx_SetZoneActionPolicyEx
 */
static HRESULT WINAPI ZoneMgrImpl_SetZoneActionPolicyEx(IInternetZoneManagerEx2* iface,
                                                        DWORD dwZone,
                                                        DWORD dwAction,
                                                        BYTE* pPolicy,
                                                        DWORD cbPolicy,
                                                        URLZONEREG urlZoneReg,
                                                        DWORD dwFlags)
{
    FIXME("(%p)->(%d, 0x%x, %p, %d, %d, 0x%x) stub\n", iface, dwZone, dwAction, pPolicy,
                                                       cbPolicy, urlZoneReg, dwFlags);
    return E_NOTIMPL;
}

/********************************************************************
 *      IInternetZoneManagerEx2_GetZoneAttributesEx
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneAttributesEx(IInternetZoneManagerEx2* iface,
                                                      DWORD dwZone,
                                                      ZONEATTRIBUTES* pZoneAttributes,
                                                      DWORD dwFlags)
{
    TRACE("(%p)->(%d, %p, 0x%x)\n", iface, dwZone, pZoneAttributes, dwFlags);

    if (dwFlags)
        FIXME("dwFlags 0x%x ignored\n", dwFlags);

    return IInternetZoneManager_GetZoneAttributes(iface, dwZone, pZoneAttributes);
}


/********************************************************************
 *      IInternetZoneManagerEx2_GetZoneSecurityState
 */
static HRESULT WINAPI ZoneMgrImpl_GetZoneSecurityState(IInternetZoneManagerEx2* iface,
                                                       DWORD dwZoneIndex,
                                                       BOOL fRespectPolicy,
                                                       LPDWORD pdwState,
                                                       BOOL *pfPolicyEncountered)
{
    FIXME("(%p)->(%d, %d, %p, %p) stub\n", iface, dwZoneIndex, fRespectPolicy,
                                           pdwState, pfPolicyEncountered);

    *pdwState = SECURITY_IE_STATE_GREEN;

    if (pfPolicyEncountered)
        *pfPolicyEncountered = FALSE;

    return S_OK;
}

/********************************************************************
 *      IInternetZoneManagerEx2_GetIESecurityState
 */
static HRESULT WINAPI ZoneMgrImpl_GetIESecurityState(IInternetZoneManagerEx2* iface,
                                                     BOOL fRespectPolicy,
                                                     LPDWORD pdwState,
                                                     BOOL *pfPolicyEncountered,
                                                     BOOL fNoCache)
{
    FIXME("(%p)->(%d, %p, %p, %d) stub\n", iface, fRespectPolicy, pdwState,
                                           pfPolicyEncountered, fNoCache);

    *pdwState = SECURITY_IE_STATE_GREEN;

    if (pfPolicyEncountered)
        *pfPolicyEncountered = FALSE;

    return S_OK;
}

/********************************************************************
 *      IInternetZoneManagerEx2_FixInsecureSettings
 */
static HRESULT WINAPI ZoneMgrImpl_FixInsecureSettings(IInternetZoneManagerEx2* iface)
{
    FIXME("(%p) stub\n", iface);
    return S_OK;
}

/********************************************************************
 *      IInternetZoneManager_Construct
 */
static const IInternetZoneManagerEx2Vtbl ZoneMgrImplVtbl = {
    ZoneMgrImpl_QueryInterface,
    ZoneMgrImpl_AddRef,
    ZoneMgrImpl_Release,
    /* IInternetZoneManager */
    ZoneMgrImpl_GetZoneAttributes,
    ZoneMgrImpl_SetZoneAttributes,
    ZoneMgrImpl_GetZoneCustomPolicy,
    ZoneMgrImpl_SetZoneCustomPolicy,
    ZoneMgrImpl_GetZoneActionPolicy,
    ZoneMgrImpl_SetZoneActionPolicy,
    ZoneMgrImpl_PromptAction,
    ZoneMgrImpl_LogAction,
    ZoneMgrImpl_CreateZoneEnumerator,
    ZoneMgrImpl_GetZoneAt,
    ZoneMgrImpl_DestroyZoneEnumerator,
    ZoneMgrImpl_CopyTemplatePoliciesToZone,
    /* IInternetZoneManagerEx */
    ZoneMgrImpl_GetZoneActionPolicyEx,
    ZoneMgrImpl_SetZoneActionPolicyEx,
    /* IInternetZoneManagerEx2 */
    ZoneMgrImpl_GetZoneAttributesEx,
    ZoneMgrImpl_GetZoneSecurityState,
    ZoneMgrImpl_GetIESecurityState,
    ZoneMgrImpl_FixInsecureSettings,
};

HRESULT ZoneMgrImpl_Construct(IUnknown *pUnkOuter, LPVOID *ppobj)
{
    ZoneMgrImpl* ret = heap_alloc_zero(sizeof(ZoneMgrImpl));

    TRACE("(%p %p)\n", pUnkOuter, ppobj);
    ret->IInternetZoneManagerEx2_iface.lpVtbl = &ZoneMgrImplVtbl;
    ret->ref = 1;
    *ppobj = (IInternetZoneManagerEx*)ret;

    URLMON_LockModule();

    return S_OK;
}

/***********************************************************************
 *           CoInternetCreateSecurityManager (URLMON.@)
 *
 */
HRESULT WINAPI CoInternetCreateSecurityManager( IServiceProvider *pSP,
    IInternetSecurityManager **ppSM, DWORD dwReserved )
{
    TRACE("%p %p %d\n", pSP, ppSM, dwReserved );

    if(pSP)
        FIXME("pSP not supported\n");

    return SecManagerImpl_Construct(NULL, (void**) ppSM);
}

/********************************************************************
 *      CoInternetCreateZoneManager (URLMON.@)
 */
HRESULT WINAPI CoInternetCreateZoneManager(IServiceProvider* pSP, IInternetZoneManager** ppZM, DWORD dwReserved)
{
    TRACE("(%p %p %x)\n", pSP, ppZM, dwReserved);
    return ZoneMgrImpl_Construct(NULL, (void**)ppZM);
}

static HRESULT parse_security_url(const WCHAR *url, PSUACTION action, WCHAR **result) {
    IInternetProtocolInfo *protocol_info;
    WCHAR *tmp, *new_url = NULL, *alloc_url = NULL;
    DWORD size, new_size;
    HRESULT hres = S_OK, parse_hres;

    while(1) {
        TRACE("parsing %s\n", debugstr_w(url));

        protocol_info = get_protocol_info(url);
        if(!protocol_info)
            break;

        size = strlenW(url)+1;
        new_url = CoTaskMemAlloc(size*sizeof(WCHAR));
        if(!new_url) {
            hres = E_OUTOFMEMORY;
            break;
        }

        new_size = 0;
        parse_hres = IInternetProtocolInfo_ParseUrl(protocol_info, url, PARSE_SECURITY_URL, 0, new_url, size, &new_size, 0);
        if(parse_hres == S_FALSE) {
            if(!new_size) {
                hres = E_UNEXPECTED;
                break;
            }

            tmp = CoTaskMemRealloc(new_url, new_size*sizeof(WCHAR));
            if(!tmp) {
                hres = E_OUTOFMEMORY;
                break;
            }
            new_url = tmp;
            parse_hres = IInternetProtocolInfo_ParseUrl(protocol_info, url, PARSE_SECURITY_URL, 0, new_url,
                    new_size, &new_size, 0);
            if(parse_hres == S_FALSE) {
                hres = E_FAIL;
                break;
            }
        }

        if(parse_hres != S_OK || !strcmpW(url, new_url))
            break;

        CoTaskMemFree(alloc_url);
        url = alloc_url = new_url;
        new_url = NULL;
    }

    CoTaskMemFree(new_url);

    if(hres != S_OK) {
        WARN("failed: %08x\n", hres);
        CoTaskMemFree(alloc_url);
        return hres;
    }

    if(action == PSU_DEFAULT && (protocol_info = get_protocol_info(url))) {
        size = strlenW(url)+1;
        new_url = CoTaskMemAlloc(size * sizeof(WCHAR));
        if(new_url) {
            new_size = 0;
            parse_hres = IInternetProtocolInfo_ParseUrl(protocol_info, url, PARSE_SECURITY_DOMAIN, 0,
                    new_url, size, &new_size, 0);
            if(parse_hres == S_FALSE) {
                if(new_size) {
                    tmp = CoTaskMemRealloc(new_url, new_size*sizeof(WCHAR));
                    if(tmp) {
                        new_url = tmp;
                        parse_hres = IInternetProtocolInfo_ParseUrl(protocol_info, url, PARSE_SECURITY_DOMAIN, 0, new_url,
                                new_size, &new_size, 0);
                        if(parse_hres == S_FALSE)
                            hres = E_FAIL;
                    }else {
                        hres = E_OUTOFMEMORY;
                    }
                }else {
                    hres = E_UNEXPECTED;
                }
            }

            if(hres == S_OK && parse_hres == S_OK) {
                CoTaskMemFree(alloc_url);
                url = alloc_url = new_url;
                new_url = NULL;
            }

            CoTaskMemFree(new_url);
        }else {
            hres = E_OUTOFMEMORY;
        }
        IInternetProtocolInfo_Release(protocol_info);
    }

    if(FAILED(hres)) {
        WARN("failed %08x\n", hres);
        CoTaskMemFree(alloc_url);
        return hres;
    }

    if(!alloc_url) {
        size = strlenW(url)+1;
        alloc_url = CoTaskMemAlloc(size * sizeof(WCHAR));
        if(!alloc_url)
            return E_OUTOFMEMORY;
        memcpy(alloc_url, url, size * sizeof(WCHAR));
    }

    *result = alloc_url;
    return S_OK;
}

/********************************************************************
 *      CoInternetGetSecurityUrl (URLMON.@)
 */
HRESULT WINAPI CoInternetGetSecurityUrl(LPCWSTR pwzUrl, LPWSTR *ppwzSecUrl, PSUACTION psuAction, DWORD dwReserved)
{
    WCHAR *secure_url;
    HRESULT hres;

    TRACE("(%p,%p,%u,%u)\n", pwzUrl, ppwzSecUrl, psuAction, dwReserved);

    hres = parse_security_url(pwzUrl, psuAction, &secure_url);
    if(FAILED(hres))
        return hres;

    if(psuAction != PSU_SECURITY_URL_ONLY) {
        PARSEDURLW parsed_url = { sizeof(parsed_url) };
        DWORD size;

        /* FIXME: Use helpers from uri.c */
        if(SUCCEEDED(ParseURLW(secure_url, &parsed_url))) {
            WCHAR *new_url;

            switch(parsed_url.nScheme) {
            case URL_SCHEME_FTP:
            case URL_SCHEME_HTTP:
            case URL_SCHEME_HTTPS:
                size = strlenW(secure_url)+1;
                new_url = CoTaskMemAlloc(size * sizeof(WCHAR));
                if(new_url)
                    hres = UrlGetPartW(secure_url, new_url, &size, URL_PART_HOSTNAME, URL_PARTFLAG_KEEPSCHEME);
                else
                    hres = E_OUTOFMEMORY;
                CoTaskMemFree(secure_url);
                if(hres != S_OK) {
                    WARN("UrlGetPart failed: %08x\n", hres);
                    CoTaskMemFree(new_url);
                    return FAILED(hres) ? hres : E_FAIL;
                }
                secure_url = new_url;
            }
        }
    }

    *ppwzSecUrl = secure_url;
    return S_OK;
}

/********************************************************************
 *      CoInternetGetSecurityUrlEx (URLMON.@)
 */
HRESULT WINAPI CoInternetGetSecurityUrlEx(IUri *pUri, IUri **ppSecUri, PSUACTION psuAction, DWORD_PTR dwReserved)
{
    URL_SCHEME scheme_type;
    BSTR secure_uri;
    WCHAR *ret_url;
    HRESULT hres;

    TRACE("(%p,%p,%u,%u)\n", pUri, ppSecUri, psuAction, (DWORD)dwReserved);

    if(!pUri || !ppSecUri)
        return E_INVALIDARG;

    hres = IUri_GetDisplayUri(pUri, &secure_uri);
    if(FAILED(hres))
        return hres;

    hres = parse_security_url(secure_uri, psuAction, &ret_url);
    SysFreeString(secure_uri);
    if(FAILED(hres))
        return hres;

    hres = CreateUri(ret_url, Uri_CREATE_ALLOW_IMPLICIT_WILDCARD_SCHEME, 0, ppSecUri);
    if(FAILED(hres)) {
        CoTaskMemFree(ret_url);
        return hres;
    }

    /* File URIs have to hierarchical. */
    hres = IUri_GetScheme(pUri, (DWORD*)&scheme_type);
    if(SUCCEEDED(hres) && scheme_type == URL_SCHEME_FILE) {
        const WCHAR *tmp = ret_url;

        /* Check and see if a "//" is after the scheme name. */
        tmp += sizeof(fileW)/sizeof(WCHAR);
        if(*tmp != '/' || *(tmp+1) != '/')
            hres = E_INVALIDARG;
    }

    if(SUCCEEDED(hres))
        hres = CreateUri(ret_url, Uri_CREATE_ALLOW_IMPLICIT_WILDCARD_SCHEME, 0, ppSecUri);
    CoTaskMemFree(ret_url);
    return hres;
}
