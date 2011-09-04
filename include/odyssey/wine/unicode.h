/*
 * PROJECT:    Odyssey
 * LICENSE:    LGPL v2.1 or any later version
 * PURPOSE:    Map Wine's Unicode functions to native wcs* functions wherever possible
 * AUTHORS:    ?
 */

#ifndef __WINE_WINE_UNICODE_H
#define __WINE_WINE_UNICODE_H

#include <stdarg.h>
#include <stdlib.h>
#include <wchar.h>

#include <windef.h>
#include <winbase.h>
#include <winnls.h>

#ifndef WINE_UNICODE_API
#define WINE_UNICODE_API
#endif

#ifndef WINE_UNICODE_INLINE
#define WINE_UNICODE_INLINE static inline
#endif

#define memicmpW(s1,s2,n) _wcsnicmp((const wchar_t *)(s1),(const wchar_t *)(s2),(n))
#define strlenW(s) wcslen((const wchar_t *)(s))
#define strcpyW(d,s) wcscpy((wchar_t *)(d),(const wchar_t *)(s))
#define strcatW(d,s) wcscat((wchar_t *)(d),(const wchar_t *)(s))
#define strcspnW(d,s) wcscspn((wchar_t *)(d),(const wchar_t *)(s))
#define strstrW(d,s) wcsstr((const wchar_t *)(d),(const wchar_t *)(s))
#define strtolW(s,e,b) wcstol((const wchar_t *)(s),(wchar_t **)(e),(b))
#define strchrW(s,c) wcschr((const wchar_t *)(s),(wchar_t)(c))
#define strrchrW(s,c) wcsrchr((const wchar_t *)(s),(wchar_t)(c))
#define strncmpW(s1,s2,n) wcsncmp((const wchar_t *)(s1),(const wchar_t *)(s2),(n))
#define strncpyW(s1,s2,n) wcsncpy((wchar_t *)(s1),(const wchar_t *)(s2),(n))
#define strcmpW(s1,s2) wcscmp((const wchar_t *)(s1),(const wchar_t *)(s2))
#define strcmpiW(s1,s2) _wcsicmp((const wchar_t *)(s1),(const wchar_t *)(s2))
#define strncmpiW(s1,s2,n) _wcsnicmp((const wchar_t *)(s1),(const wchar_t *)(s2),(n))
#define strtoulW(s1,s2,b) wcstoul((const wchar_t *)(s1),(wchar_t **)(s2),(b))
#define strspnW(str, accept) wcsspn((const wchar_t *)(str), (const wchar_t *)(accept))
#define strpbrkW(str, accept) wcspbrk((const wchar_t *)(str), (const wchar_t *)(accept))
#define tolowerW(n) towlower((n))
#define toupperW(n) towupper((n))
#define islowerW(n) iswlower((n))
#define isupperW(n) iswupper((n))
#define isalphaW(n) iswalpha((n))
#define isalnumW(n) iswalnum((n))
#define isdigitW(n) iswdigit((n))
#define isxdigitW(n) iswxdigit((n))
#define isspaceW(n) iswspace((n))
#define iscntrlW(n) iswcntrl((n))
#define atoiW(s) _wtoi((const wchar_t *)(s))
#define atolW(s) _wtol((const wchar_t *)(s))
#define strlwrW(s) _wcslwr((wchar_t *)(s))
#define struprW(s) _wcsupr((wchar_t *)(s))
#define sprintfW swprintf
#define vsprintfW vswprintf
#define snprintfW _snwprintf
#define vsnprintfW _vsnwprintf
#define isprintW iswprint

static __inline WCHAR *memchrW( const WCHAR *ptr, WCHAR ch, size_t n )
{
    const WCHAR *end;
    for (end = ptr + n; ptr < end; ptr++) if (*ptr == ch) return (WCHAR *)ptr;
    return NULL;
}

static __inline WCHAR *memrchrW( const WCHAR *ptr, WCHAR ch, size_t n )
{
    const WCHAR *end, *ret = NULL;
    for (end = ptr + n; ptr < end; ptr++) if (*ptr == ch) ret = ptr;
    return (WCHAR *)ret;
}

#endif
