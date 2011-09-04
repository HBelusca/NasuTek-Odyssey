/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey System Libraries
 * FILE:            lib/dnsapi/precomp.h
 * PURPOSE:         Win32 DNS API Libary Header
 * PROGRAMMER:      Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/

#define WIN32_NO_STATUS
#define NTOS_MODE_USER

/* PSDK/NDK Headers */
#include <windows.h>
#include <winerror.h>
#include <windns.h>
#include <ndk/rtlfuncs.h>

/* Internal DNSAPI Headers */
#include <internal/windns.h>

static inline LPWSTR dns_strdup_uw( const char *str )
{
    LPWSTR ret = NULL;
    if (str)
    {
        DWORD len = MultiByteToWideChar( CP_UTF8, 0, str, -1, NULL, 0 );
        if ((ret = HeapAlloc(GetProcessHeap(),0,( len * sizeof(WCHAR) ))))
            MultiByteToWideChar( CP_UTF8, 0, str, -1, ret, len );
    }
    return ret;
}

static inline LPWSTR dns_strdup_aw( LPCSTR str )
{
    LPWSTR ret = NULL;
    if (str)
    {
        DWORD len = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, 0 );
        if ((ret = HeapAlloc(GetProcessHeap(), 0, ( len * sizeof(WCHAR) ))))
            MultiByteToWideChar( CP_ACP, 0, str, -1, ret, len );
    }
    return ret;
}

static inline LPSTR dns_strdup_a( LPCSTR src )
{
    LPSTR dst;

    if (!src) return NULL;
    dst = HeapAlloc(GetProcessHeap(), 0, (lstrlenA( src ) + 1) * sizeof(char) );
    if (dst) lstrcpyA( dst, src );
    return dst;
}

static inline char *dns_strdup_u( const char *src )
{
    char *dst;

    if (!src) return NULL;
    dst = HeapAlloc(GetProcessHeap(), 0, (strlen( src ) + 1) * sizeof(char) );
    if (dst) strcpy( dst, src );
    return dst;
}

static inline LPWSTR dns_strdup_w( LPCWSTR src )
{
    LPWSTR dst;

    if (!src) return NULL;
    dst = HeapAlloc(GetProcessHeap(), 0, (lstrlenW( src ) + 1) * sizeof(WCHAR) );
    if (dst) lstrcpyW( dst, src );
    return dst;
}

static inline LPSTR dns_strdup_wa( LPCWSTR str )
{
    LPSTR ret = NULL;
    if (str)
    {
        DWORD len = WideCharToMultiByte( CP_ACP, 0, str, -1, NULL, 0, NULL, NULL );
        if ((ret = HeapAlloc(GetProcessHeap(), 0, len )))
            WideCharToMultiByte( CP_ACP, 0, str, -1, ret, len, NULL, NULL );
    }
    return ret;
}

static inline char *dns_strdup_wu( LPCWSTR str )
{
    LPSTR ret = NULL;
    if (str)
    {
        DWORD len = WideCharToMultiByte( CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL );
        if ((ret = HeapAlloc(GetProcessHeap(), 0, len )))
            WideCharToMultiByte( CP_UTF8, 0, str, -1, ret, len, NULL, NULL );
    }
    return ret;
}

static inline char *dns_strdup_au( LPCSTR src )
{
    char *dst = NULL;
    LPWSTR ret = dns_strdup_aw( src );

    if (ret)
    {
        dst = dns_strdup_wu( ret );
        HeapFree( GetProcessHeap(), 0, ret );
    }
    return dst;
}

static inline LPSTR dns_strdup_ua( const char *src )
{
    LPSTR dst = NULL;
    LPWSTR ret = dns_strdup_uw( src );

    if (ret)
    {
        dst = dns_strdup_wa( ret );
        HeapFree( GetProcessHeap(), 0, ret );
    }
    return dst;
}
