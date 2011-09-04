/*
 * Locale support
 *
 * Copyright 1995 Martin von Loewis
 * Copyright 1998 David Lee Lambert
 * Copyright 2000 Julio C√©sar G√°zquez
 * Copyright 2002 Alexandre Julliard for CodeWeavers
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

//#include "config.h"
//#include "wine/port.h"

#include <assert.h>
#include <locale.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "ntstatus.h"
#define WIN32_NO_STATUS
#include "windef.h"
#include "winbase.h"
#include "winuser.h"  /* for RT_STRINGW */
#include "winternl.h"
#include "wine/unicode.h"
#include "winnls.h"
#include "winerror.h"
#include "winver.h"
#include "wine/debug.h"

#include "lcformat_private.h"
#define REG_SZ 1
extern int wine_fold_string(int flags, const WCHAR *src, int srclen, WCHAR *dst, int dstlen);
extern int wine_get_sortkey(int flags, const WCHAR *src, int srclen, char *dst, int dstlen);
extern int wine_compare_string(int flags, const WCHAR *str1, int len1, const WCHAR *str2, int len2);
static inline unsigned short get_char_typeW( WCHAR ch )
{
    extern const unsigned short wine_wctype_table[];
    return wine_wctype_table[wine_wctype_table[ch >> 8] + (ch & 0xff)];
}

#define HeapAlloc RtlAllocateHeap
#define HeapReAlloc RtlReAllocateHeap
#define HeapFree RtlFreeHeap
WINE_DEFAULT_DEBUG_CHANNEL(nls);

extern HMODULE kernel32_handle;

#define LOCALE_LOCALEINFOFLAGSMASK (LOCALE_NOUSEROVERRIDE|LOCALE_USE_CP_ACP|\
                                    LOCALE_RETURN_NUMBER|LOCALE_RETURN_GENITIVE_NAMES)

static const WCHAR szNlsKeyName[] = {
    'M','a','c','h','i','n','e','\\','S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\','N','l','s','\0'
};

static const WCHAR szLocaleKeyName[] = {
    'M','a','c','h','i','n','e','\\','S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\','N','l','s','\\','L','o','c','a','l','e',0
};

static const WCHAR szLangGroupsKeyName[] = {
    'M','a','c','h','i','n','e','\\','S','y','s','t','e','m','\\',
    'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
    'C','o','n','t','r','o','l','\\','N','l','s','\\',
    'L','a','n','g','u','a','g','e',' ','G','r','o','u','p','s',0
};

/* Charset to codepage map, sorted by name. */
static const struct charset_entry
{
    const char *charset_name;
    UINT        codepage;
} charset_names[] =
{
    { "BIG5", 950 },
    { "CP1250", 1250 },
    { "CP1251", 1251 },
    { "CP1252", 1252 },
    { "CP1253", 1253 },
    { "CP1254", 1254 },
    { "CP1255", 1255 },
    { "CP1256", 1256 },
    { "CP1257", 1257 },
    { "CP1258", 1258 },
    { "CP932", 932 },
    { "CP936", 936 },
    { "CP949", 949 },
    { "CP950", 950 },
    { "EUCJP", 20932 },
    { "GB2312", 936 },
    { "IBM037", 37 },
    { "IBM1026", 1026 },
    { "IBM424", 424 },
    { "IBM437", 437 },
    { "IBM500", 500 },
    { "IBM850", 850 },
    { "IBM852", 852 },
    { "IBM855", 855 },
    { "IBM857", 857 },
    { "IBM860", 860 },
    { "IBM861", 861 },
    { "IBM862", 862 },
    { "IBM863", 863 },
    { "IBM864", 864 },
    { "IBM865", 865 },
    { "IBM866", 866 },
    { "IBM869", 869 },
    { "IBM874", 874 },
    { "IBM875", 875 },
    { "ISO88591", 28591 },
    { "ISO885910", 28600 },
    { "ISO885913", 28603 },
    { "ISO885914", 28604 },
    { "ISO885915", 28605 },
    { "ISO885916", 28606 },
    { "ISO88592", 28592 },
    { "ISO88593", 28593 },
    { "ISO88594", 28594 },
    { "ISO88595", 28595 },
    { "ISO88596", 28596 },
    { "ISO88597", 28597 },
    { "ISO88598", 28598 },
    { "ISO88599", 28599 },
    { "KOI8R", 20866 },
    { "KOI8U", 21866 },
    { "UTF8", CP_UTF8 }
};


struct locale_name
{
    WCHAR  win_name[128];   /* Windows name ("en-US") */
    WCHAR  lang[128];       /* language ("en") (note: buffer contains the other strings too) */
    WCHAR *country;         /* country ("US") */
    WCHAR *charset;         /* charset ("UTF-8") for Unix format only */
    WCHAR *script;          /* script ("Latn") for Windows format only */
    WCHAR *modifier;        /* modifier or sort order */
    LCID   lcid;            /* corresponding LCID */
    int    matches;         /* number of elements matching LCID (0..4) */
    UINT   codepage;        /* codepage corresponding to charset */
};

/* locale ids corresponding to the various Unix locale parameters */
static LCID lcid_LC_COLLATE;
static LCID lcid_LC_CTYPE;
static LCID lcid_LC_MONETARY;
static LCID lcid_LC_NUMERIC;
static LCID lcid_LC_TIME;
static LCID lcid_LC_PAPER;
static LCID lcid_LC_MEASUREMENT;
static LCID lcid_LC_TELEPHONE;

/* Copy Ascii string to Unicode without using codepages */
static inline void strcpynAtoW( WCHAR *dst, const char *src, size_t n )
{
    while (n > 1 && *src)
    {
        *dst++ = (unsigned char)*src++;
        n--;
    }
    if (n) *dst = 0;
}


/***********************************************************************
 *		get_lcid_codepage
 *
 * Retrieve the ANSI codepage for a given locale.
 */
static inline UINT get_lcid_codepage( LCID lcid )
{
    UINT ret;
    if (!GetLocaleInfoW( lcid, LOCALE_IDEFAULTANSICODEPAGE|LOCALE_RETURN_NUMBER, (WCHAR *)&ret,
                         sizeof(ret)/sizeof(WCHAR) )) ret = 0;
    return ret;
}

/***********************************************************************
 *           convert_default_lcid
 *
 * Get the default LCID to use for a given lctype in GetLocaleInfo.
 */
static LCID convert_default_lcid( LCID lcid, LCTYPE lctype )
{
    if (lcid == LOCALE_SYSTEM_DEFAULT ||
        lcid == LOCALE_USER_DEFAULT ||
        lcid == LOCALE_NEUTRAL)
    {
        LCID default_id = 0;

        switch(lctype & 0xffff)
        {
        case LOCALE_SSORTNAME:
            default_id = lcid_LC_COLLATE;
            break;

        case LOCALE_FONTSIGNATURE:
        case LOCALE_IDEFAULTANSICODEPAGE:
        case LOCALE_IDEFAULTCODEPAGE:
        case LOCALE_IDEFAULTEBCDICCODEPAGE:
        case LOCALE_IDEFAULTMACCODEPAGE:
        case LOCALE_IDEFAULTUNIXCODEPAGE:
            default_id = lcid_LC_CTYPE;
            break;

        case LOCALE_ICURRDIGITS:
        case LOCALE_ICURRENCY:
        case LOCALE_IINTLCURRDIGITS:
        case LOCALE_INEGCURR:
        case LOCALE_INEGSEPBYSPACE:
        case LOCALE_INEGSIGNPOSN:
        case LOCALE_INEGSYMPRECEDES:
        case LOCALE_IPOSSEPBYSPACE:
        case LOCALE_IPOSSIGNPOSN:
        case LOCALE_IPOSSYMPRECEDES:
        case LOCALE_SCURRENCY:
        case LOCALE_SINTLSYMBOL:
        case LOCALE_SMONDECIMALSEP:
        case LOCALE_SMONGROUPING:
        case LOCALE_SMONTHOUSANDSEP:
        case LOCALE_SNATIVECURRNAME:
            default_id = lcid_LC_MONETARY;
            break;

        case LOCALE_IDIGITS:
        case LOCALE_IDIGITSUBSTITUTION:
        case LOCALE_ILZERO:
        case LOCALE_INEGNUMBER:
        case LOCALE_SDECIMAL:
        case LOCALE_SGROUPING:
        //case LOCALE_SNAN:
        case LOCALE_SNATIVEDIGITS:
        case LOCALE_SNEGATIVESIGN:
        //case LOCALE_SNEGINFINITY:
        //case LOCALE_SPOSINFINITY:
        case LOCALE_SPOSITIVESIGN:
        case LOCALE_STHOUSAND:
            default_id = lcid_LC_NUMERIC;
            break;

        case LOCALE_ICALENDARTYPE:
        case LOCALE_ICENTURY:
        case LOCALE_IDATE:
        case LOCALE_IDAYLZERO:
        case LOCALE_IFIRSTDAYOFWEEK:
        case LOCALE_IFIRSTWEEKOFYEAR:
        case LOCALE_ILDATE:
        case LOCALE_IMONLZERO:
        case LOCALE_IOPTIONALCALENDAR:
        case LOCALE_ITIME:
        case LOCALE_ITIMEMARKPOSN:
        case LOCALE_ITLZERO:
        case LOCALE_S1159:
        case LOCALE_S2359:
        case LOCALE_SABBREVDAYNAME1:
        case LOCALE_SABBREVDAYNAME2:
        case LOCALE_SABBREVDAYNAME3:
        case LOCALE_SABBREVDAYNAME4:
        case LOCALE_SABBREVDAYNAME5:
        case LOCALE_SABBREVDAYNAME6:
        case LOCALE_SABBREVDAYNAME7:
        case LOCALE_SABBREVMONTHNAME1:
        case LOCALE_SABBREVMONTHNAME2:
        case LOCALE_SABBREVMONTHNAME3:
        case LOCALE_SABBREVMONTHNAME4:
        case LOCALE_SABBREVMONTHNAME5:
        case LOCALE_SABBREVMONTHNAME6:
        case LOCALE_SABBREVMONTHNAME7:
        case LOCALE_SABBREVMONTHNAME8:
        case LOCALE_SABBREVMONTHNAME9:
        case LOCALE_SABBREVMONTHNAME10:
        case LOCALE_SABBREVMONTHNAME11:
        case LOCALE_SABBREVMONTHNAME12:
        case LOCALE_SABBREVMONTHNAME13:
        case LOCALE_SDATE:
        case LOCALE_SDAYNAME1:
        case LOCALE_SDAYNAME2:
        case LOCALE_SDAYNAME3:
        case LOCALE_SDAYNAME4:
        case LOCALE_SDAYNAME5:
        case LOCALE_SDAYNAME6:
        case LOCALE_SDAYNAME7:
        //case LOCALE_SDURATION:
        case LOCALE_SLONGDATE:
        case LOCALE_SMONTHNAME1:
        case LOCALE_SMONTHNAME2:
        case LOCALE_SMONTHNAME3:
        case LOCALE_SMONTHNAME4:
        case LOCALE_SMONTHNAME5:
        case LOCALE_SMONTHNAME6:
        case LOCALE_SMONTHNAME7:
        case LOCALE_SMONTHNAME8:
        case LOCALE_SMONTHNAME9:
        case LOCALE_SMONTHNAME10:
        case LOCALE_SMONTHNAME11:
        case LOCALE_SMONTHNAME12:
        case LOCALE_SMONTHNAME13:
        case LOCALE_SSHORTDATE:
        //case LOCALE_SSHORTESTDAYNAME1:
        //case LOCALE_SSHORTESTDAYNAME2:
        //case LOCALE_SSHORTESTDAYNAME3:
        //case LOCALE_SSHORTESTDAYNAME4:
        //case LOCALE_SSHORTESTDAYNAME5:
        //case LOCALE_SSHORTESTDAYNAME6:
        //case LOCALE_SSHORTESTDAYNAME7:
        case LOCALE_STIME:
        case LOCALE_STIMEFORMAT:
        case LOCALE_SYEARMONTH:
            default_id = lcid_LC_TIME;
            break;

        case LOCALE_IPAPERSIZE:
            default_id = lcid_LC_PAPER;
            break;

        case LOCALE_IMEASURE:
            default_id = lcid_LC_MEASUREMENT;
            break;

        case LOCALE_ICOUNTRY:
            default_id = lcid_LC_TELEPHONE;
            break;
        }
        if (default_id) lcid = default_id;
    }
    return ConvertDefaultLocale( lcid );
}

/***********************************************************************
 *           is_genitive_name_supported
 *
 * Determine could LCTYPE basically support genitive name form or not.
 */
static BOOL is_genitive_name_supported( LCTYPE lctype )
{
    switch(lctype & 0xffff)
    {
    case LOCALE_SMONTHNAME1:
    case LOCALE_SMONTHNAME2:
    case LOCALE_SMONTHNAME3:
    case LOCALE_SMONTHNAME4:
    case LOCALE_SMONTHNAME5:
    case LOCALE_SMONTHNAME6:
    case LOCALE_SMONTHNAME7:
    case LOCALE_SMONTHNAME8:
    case LOCALE_SMONTHNAME9:
    case LOCALE_SMONTHNAME10:
    case LOCALE_SMONTHNAME11:
    case LOCALE_SMONTHNAME12:
    case LOCALE_SMONTHNAME13:
         return TRUE;
    default:
         return FALSE;
    }
}

/***********************************************************************
 *		create_registry_key
 *
 * Create the Control Panel\\International registry key.
 */
static inline HANDLE create_registry_key(void)
{
    static const WCHAR cplW[] = {'C','o','n','t','r','o','l',' ','P','a','n','e','l',0};
    static const WCHAR intlW[] = {'I','n','t','e','r','n','a','t','i','o','n','a','l',0};
    OBJECT_ATTRIBUTES attr;
    UNICODE_STRING nameW;
    HANDLE cpl_key, hkey = 0;

    if (RtlOpenCurrentUser( KEY_ALL_ACCESS, &hkey ) != STATUS_SUCCESS) return 0;

    attr.Length = sizeof(attr);
    attr.RootDirectory = hkey;
    attr.ObjectName = &nameW;
    attr.Attributes = 0;
    attr.SecurityDescriptor = NULL;
    attr.SecurityQualityOfService = NULL;
    RtlInitUnicodeString( &nameW, cplW );

    if (!NtCreateKey( &cpl_key, KEY_ALL_ACCESS, &attr, 0, NULL, 0, NULL ))
    {
        NtClose( attr.RootDirectory );
        attr.RootDirectory = cpl_key;
        RtlInitUnicodeString( &nameW, intlW );
        if (NtCreateKey( &hkey, KEY_ALL_ACCESS, &attr, 0, NULL, 0, NULL )) hkey = 0;
    }
    NtClose( attr.RootDirectory );
    return hkey;
}

/***********************************************************************
 *		GetUserDefaultLangID (KERNEL32.@)
 *
 * Get the default language Id for the current user.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LANGID of the default language for the current user.
 */
LANGID WINAPI GetUserDefaultLangID(void)
{
    return LANGIDFROMLCID(GetUserDefaultLCID());
}


/***********************************************************************
 *		GetSystemDefaultLangID (KERNEL32.@)
 *
 * Get the default language Id for the system.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LANGID of the default language for the system.
 */
LANGID WINAPI GetSystemDefaultLangID(void)
{
    return LANGIDFROMLCID(GetSystemDefaultLCID());
}


/***********************************************************************
 *		GetUserDefaultLCID (KERNEL32.@)
 *
 * Get the default locale Id for the current user.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LCID of the default locale for the current user.
 */
LCID WINAPI GetUserDefaultLCID(void)
{
    LCID lcid;
    NtQueryDefaultLocale( TRUE, &lcid );
    return lcid;
}


/***********************************************************************
 *		GetSystemDefaultLCID (KERNEL32.@)
 *
 * Get the default locale Id for the system.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LCID of the default locale for the system.
 */
LCID WINAPI GetSystemDefaultLCID(void)
{
    LCID lcid;
    NtQueryDefaultLocale( FALSE, &lcid );
    return lcid;
}


/***********************************************************************
 *		GetUserDefaultUILanguage (KERNEL32.@)
 *
 * Get the default user interface language Id for the current user.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LANGID of the default UI language for the current user.
 */
LANGID WINAPI GetUserDefaultUILanguage(void)
{
    LANGID lang;
    NtQueryDefaultUILanguage( &lang );
    return lang;
}


/***********************************************************************
 *		GetSystemDefaultUILanguage (KERNEL32.@)
 *
 * Get the default user interface language Id for the system.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The current LANGID of the default UI language for the system. This is
 *  typically the same language used during the installation process.
 */
LANGID WINAPI GetSystemDefaultUILanguage(void)
{
    LANGID lang;
    NtQueryInstallUILanguage( &lang );
    return lang;
}

/******************************************************************************
 *		get_locale_value_name
 *
 * Gets the registry value name for a given lctype.
 */
static const WCHAR *get_locale_value_name( DWORD lctype )
{
    static const WCHAR iCalendarTypeW[] = {'i','C','a','l','e','n','d','a','r','T','y','p','e',0};
    static const WCHAR iCountryW[] = {'i','C','o','u','n','t','r','y',0};
    static const WCHAR iCurrDigitsW[] = {'i','C','u','r','r','D','i','g','i','t','s',0};
    static const WCHAR iCurrencyW[] = {'i','C','u','r','r','e','n','c','y',0};
    static const WCHAR iDateW[] = {'i','D','a','t','e',0};
    static const WCHAR iDigitsW[] = {'i','D','i','g','i','t','s',0};
    static const WCHAR iFirstDayOfWeekW[] = {'i','F','i','r','s','t','D','a','y','O','f','W','e','e','k',0};
    static const WCHAR iFirstWeekOfYearW[] = {'i','F','i','r','s','t','W','e','e','k','O','f','Y','e','a','r',0};
    static const WCHAR iLDateW[] = {'i','L','D','a','t','e',0};
    static const WCHAR iLZeroW[] = {'i','L','Z','e','r','o',0};
    static const WCHAR iMeasureW[] = {'i','M','e','a','s','u','r','e',0};
    static const WCHAR iNegCurrW[] = {'i','N','e','g','C','u','r','r',0};
    static const WCHAR iNegNumberW[] = {'i','N','e','g','N','u','m','b','e','r',0};
    static const WCHAR iPaperSizeW[] = {'i','P','a','p','e','r','S','i','z','e',0};
    static const WCHAR iTLZeroW[] = {'i','T','L','Z','e','r','o',0};
    static const WCHAR iTimePrefixW[] = {'i','T','i','m','e','P','r','e','f','i','x',0};
    static const WCHAR iTimeW[] = {'i','T','i','m','e',0};
    static const WCHAR s1159W[] = {'s','1','1','5','9',0};
    static const WCHAR s2359W[] = {'s','2','3','5','9',0};
    static const WCHAR sCountryW[] = {'s','C','o','u','n','t','r','y',0};
    static const WCHAR sCurrencyW[] = {'s','C','u','r','r','e','n','c','y',0};
    static const WCHAR sDateW[] = {'s','D','a','t','e',0};
    static const WCHAR sDecimalW[] = {'s','D','e','c','i','m','a','l',0};
    static const WCHAR sGroupingW[] = {'s','G','r','o','u','p','i','n','g',0};
    static const WCHAR sLanguageW[] = {'s','L','a','n','g','u','a','g','e',0};
    static const WCHAR sListW[] = {'s','L','i','s','t',0};
    static const WCHAR sLongDateW[] = {'s','L','o','n','g','D','a','t','e',0};
    static const WCHAR sMonDecimalSepW[] = {'s','M','o','n','D','e','c','i','m','a','l','S','e','p',0};
    static const WCHAR sMonGroupingW[] = {'s','M','o','n','G','r','o','u','p','i','n','g',0};
    static const WCHAR sMonThousandSepW[] = {'s','M','o','n','T','h','o','u','s','a','n','d','S','e','p',0};
    static const WCHAR sNativeDigitsW[] = {'s','N','a','t','i','v','e','D','i','g','i','t','s',0};
    static const WCHAR sNegativeSignW[] = {'s','N','e','g','a','t','i','v','e','S','i','g','n',0};
    static const WCHAR sPositiveSignW[] = {'s','P','o','s','i','t','i','v','e','S','i','g','n',0};
    static const WCHAR sShortDateW[] = {'s','S','h','o','r','t','D','a','t','e',0};
    static const WCHAR sThousandW[] = {'s','T','h','o','u','s','a','n','d',0};
    static const WCHAR sTimeFormatW[] = {'s','T','i','m','e','F','o','r','m','a','t',0};
    static const WCHAR sTimeW[] = {'s','T','i','m','e',0};
    static const WCHAR sYearMonthW[] = {'s','Y','e','a','r','M','o','n','t','h',0};
    static const WCHAR NumShapeW[] = {'N','u','m','s','h','a','p','e',0};

    switch (lctype)
    {
    /* These values are used by SetLocaleInfo and GetLocaleInfo, and
     * the values are stored in the registry, confirmed under Windows.
     */
    case LOCALE_ICALENDARTYPE:    return iCalendarTypeW;
    case LOCALE_ICURRDIGITS:      return iCurrDigitsW;
    case LOCALE_ICURRENCY:        return iCurrencyW;
    case LOCALE_IDIGITS:          return iDigitsW;
    case LOCALE_IFIRSTDAYOFWEEK:  return iFirstDayOfWeekW;
    case LOCALE_IFIRSTWEEKOFYEAR: return iFirstWeekOfYearW;
    case LOCALE_ILZERO:           return iLZeroW;
    case LOCALE_IMEASURE:         return iMeasureW;
    case LOCALE_INEGCURR:         return iNegCurrW;
    case LOCALE_INEGNUMBER:       return iNegNumberW;
    case LOCALE_IPAPERSIZE:       return iPaperSizeW;
    case LOCALE_ITIME:            return iTimeW;
    case LOCALE_S1159:            return s1159W;
    case LOCALE_S2359:            return s2359W;
    case LOCALE_SCURRENCY:        return sCurrencyW;
    case LOCALE_SDATE:            return sDateW;
    case LOCALE_SDECIMAL:         return sDecimalW;
    case LOCALE_SGROUPING:        return sGroupingW;
    case LOCALE_SLIST:            return sListW;
    case LOCALE_SLONGDATE:        return sLongDateW;
    case LOCALE_SMONDECIMALSEP:   return sMonDecimalSepW;
    case LOCALE_SMONGROUPING:     return sMonGroupingW;
    case LOCALE_SMONTHOUSANDSEP:  return sMonThousandSepW;
    case LOCALE_SNEGATIVESIGN:    return sNegativeSignW;
    case LOCALE_SPOSITIVESIGN:    return sPositiveSignW;
    case LOCALE_SSHORTDATE:       return sShortDateW;
    case LOCALE_STHOUSAND:        return sThousandW;
    case LOCALE_STIME:            return sTimeW;
    case LOCALE_STIMEFORMAT:      return sTimeFormatW;
    case LOCALE_SYEARMONTH:       return sYearMonthW;

    /* The following are not listed under MSDN as supported,
     * but seem to be used and also stored in the registry.
     */
    case LOCALE_ICOUNTRY:         return iCountryW;
    case LOCALE_IDATE:            return iDateW;
    case LOCALE_ILDATE:           return iLDateW;
    case LOCALE_ITLZERO:          return iTLZeroW;
    case LOCALE_SCOUNTRY:         return sCountryW;
    case LOCALE_SABBREVLANGNAME:  return sLanguageW;

    /* The following are used in XP and later */
    case LOCALE_IDIGITSUBSTITUTION: return NumShapeW;
    case LOCALE_SNATIVEDIGITS:      return sNativeDigitsW;
    case LOCALE_ITIMEMARKPOSN:      return iTimePrefixW;
    }
    return NULL;
}


/******************************************************************************
 *		get_registry_locale_info
 *
 * Retrieve user-modified locale info from the registry.
 * Return length, 0 on error, -1 if not found.
 */
static INT get_registry_locale_info( LPCWSTR value, LPWSTR buffer, INT len )
{
    DWORD size;
    INT ret;
    HANDLE hkey;
    NTSTATUS status;
    UNICODE_STRING nameW;
    KEY_VALUE_PARTIAL_INFORMATION *info;
    static const int info_size = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

    if (!(hkey = create_registry_key())) return -1;

    RtlInitUnicodeString( &nameW, value );
    size = info_size + len * sizeof(WCHAR);

    if (!(info = HeapAlloc( GetProcessHeap(), 0, size )))
    {
        NtClose( hkey );
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return 0;
    }

    status = NtQueryValueKey( hkey, &nameW, KeyValuePartialInformation, info, size, &size );

    if (!status)
    {
        ret = (size - info_size) / sizeof(WCHAR);
        /* append terminating null if needed */
        if (!ret || ((WCHAR *)info->Data)[ret-1])
        {
            if (ret < len || !buffer) ret++;
            else
            {
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                ret = 0;
            }
        }
        if (ret && buffer)
        {
            memcpy( buffer, info->Data, (ret-1) * sizeof(WCHAR) );
            buffer[ret-1] = 0;
        }
    }
    else if (status == STATUS_BUFFER_OVERFLOW && !buffer)
    {
        ret = (size - info_size) / sizeof(WCHAR) + 1;
    }
    else if (status == STATUS_OBJECT_NAME_NOT_FOUND)
    {
        ret = -1;
    }
    else
    {
        SetLastError( RtlNtStatusToDosError(status) );
        ret = 0;
    }
    NtClose( hkey );
    HeapFree( GetProcessHeap(), 0, info );
    return ret;
}


/******************************************************************************
 *		GetLocaleInfoA (KERNEL32.@)
 *
 * Get information about an aspect of a locale.
 *
 * PARAMS
 *  lcid   [I] LCID of the locale
 *  lctype [I] LCTYPE_ flags from "winnls.h"
 *  buffer [O] Destination for the information
 *  len    [I] Length of buffer in characters
 *
 * RETURNS
 *  Success: The size of the data requested. If buffer is non-NULL, it is filled
 *           with the information.
 *  Failure: 0. Use GetLastError() to determine the cause.
 *
 * NOTES
 *  - LOCALE_NEUTRAL is equal to LOCALE_SYSTEM_DEFAULT
 *  - The string returned is NUL terminated, except for LOCALE_FONTSIGNATURE,
 *    which is a bit string.
 */
INT WINAPI GetLocaleInfoA( LCID lcid, LCTYPE lctype, LPSTR buffer, INT len )
{
    WCHAR *bufferW;
    INT lenW, ret;

    TRACE( "(lcid=0x%x,lctype=0x%x,%p,%d)\n", lcid, lctype, buffer, len );

    if (len < 0 || (len && !buffer))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }
    if (lctype & LOCALE_RETURN_GENITIVE_NAMES )
    {
        SetLastError( ERROR_INVALID_FLAGS );
        return 0;
    }

    if (!len) buffer = NULL;

    if (!(lenW = GetLocaleInfoW( lcid, lctype, NULL, 0 ))) return 0;

    if (!(bufferW = HeapAlloc( GetProcessHeap(), 0, lenW * sizeof(WCHAR) )))
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return 0;
    }
    if ((ret = GetLocaleInfoW( lcid, lctype, bufferW, lenW )))
    {
        if ((lctype & LOCALE_RETURN_NUMBER) ||
            ((lctype & ~LOCALE_LOCALEINFOFLAGSMASK) == LOCALE_FONTSIGNATURE))
        {
            /* it's not an ASCII string, just bytes */
            ret *= sizeof(WCHAR);
            if (buffer)
            {
                if (ret <= len) memcpy( buffer, bufferW, ret );
                else
                {
                    SetLastError( ERROR_INSUFFICIENT_BUFFER );
                    ret = 0;
                }
            }
        }
        else
        {
            UINT codepage = CP_ACP;
            if (!(lctype & LOCALE_USE_CP_ACP)) codepage = get_lcid_codepage( lcid );
            ret = WideCharToMultiByte( codepage, 0, bufferW, ret, buffer, len, NULL, NULL );
        }
    }
    HeapFree( GetProcessHeap(), 0, bufferW );
    return ret;
}


/******************************************************************************
 *		GetLocaleInfoW (KERNEL32.@)
 *
 * See GetLocaleInfoA.
 */
INT WINAPI GetLocaleInfoW( LCID lcid, LCTYPE lctype, LPWSTR buffer, INT len )
{
    LANGID lang_id;
    HRSRC hrsrc;
    HGLOBAL hmem;
    INT ret;
    UINT lcflags;
    const WCHAR *p;
    unsigned int i;

    if (len < 0 || (len && !buffer))
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return 0;
    }
    if (lctype & LOCALE_RETURN_GENITIVE_NAMES &&
       !is_genitive_name_supported( lctype ))
    {
        SetLastError( ERROR_INVALID_FLAGS );
        return 0;
    }

    if (!len) buffer = NULL;

    lcid = convert_default_lcid( lcid, lctype );

    lcflags = lctype & LOCALE_LOCALEINFOFLAGSMASK;
    lctype &= 0xffff;

    TRACE( "(lcid=0x%x,lctype=0x%x,%p,%d)\n", lcid, lctype, buffer, len );

    /* first check for overrides in the registry */

    if (!(lcflags & LOCALE_NOUSEROVERRIDE) &&
        lcid == convert_default_lcid( LOCALE_USER_DEFAULT, lctype ))
    {
        const WCHAR *value = get_locale_value_name(lctype);

        if (value)
        {
            if (lcflags & LOCALE_RETURN_NUMBER)
            {
                WCHAR tmp[16];
                ret = get_registry_locale_info( value, tmp, sizeof(tmp)/sizeof(WCHAR) );
                if (ret > 0)
                {
                    WCHAR *end;
                    UINT number = strtolW( tmp, &end, 10 );
                    if (*end)  /* invalid number */
                    {
                        SetLastError( ERROR_INVALID_FLAGS );
                        return 0;
                    }
                    ret = sizeof(UINT)/sizeof(WCHAR);
                    if (!buffer) return ret;
                    if (ret > len)
                    {
                        SetLastError( ERROR_INSUFFICIENT_BUFFER );
                        return 0;
                    }
                    memcpy( buffer, &number, sizeof(number) );
                }
            }
            else ret = get_registry_locale_info( value, buffer, len );

            if (ret != -1) return ret;
        }
    }

    /* now load it from kernel resources */

    lang_id = LANGIDFROMLCID( lcid );

    /* replace SUBLANG_NEUTRAL by SUBLANG_DEFAULT */
    if (SUBLANGID(lang_id) == SUBLANG_NEUTRAL)
        lang_id = MAKELANGID(PRIMARYLANGID(lang_id), SUBLANG_DEFAULT);

    if (!(hrsrc = FindResourceExW( kernel32_handle, (LPWSTR)RT_STRING,
                                   ULongToPtr((lctype >> 4) + 1), lang_id )))
    {
        SetLastError( ERROR_INVALID_FLAGS );  /* no such lctype */
        return 0;
    }
    if (!(hmem = LoadResource( kernel32_handle, hrsrc )))
        return 0;

    p = LockResource( hmem );
    for (i = 0; i < (lctype & 0x0f); i++) p += *p + 1;

    if (lcflags & LOCALE_RETURN_NUMBER) ret = sizeof(UINT)/sizeof(WCHAR);
    else if (is_genitive_name_supported( lctype ) && *p)
    {
        /* genitive form's stored after a null separator from a nominative */
        for (i = 1; i <= *p; i++) if (!p[i]) break;

        if (i <= *p && (lcflags & LOCALE_RETURN_GENITIVE_NAMES))
        {
            ret = *p - i + 1;
            p += i;
        }
        else ret = i;
    }
    else
        ret = (lctype == LOCALE_FONTSIGNATURE) ? *p : *p + 1;

    if (!buffer) return ret;

    if (ret > len)
    {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return 0;
    }

    if (lcflags & LOCALE_RETURN_NUMBER)
    {
        UINT number;
        WCHAR *end, *tmp = HeapAlloc( GetProcessHeap(), 0, (*p + 1) * sizeof(WCHAR) );
        if (!tmp) return 0;
        memcpy( tmp, p + 1, *p * sizeof(WCHAR) );
        tmp[*p] = 0;
        number = strtolW( tmp, &end, 10 );
        if (!*end)
            memcpy( buffer, &number, sizeof(number) );
        else  /* invalid number */
        {
            SetLastError( ERROR_INVALID_FLAGS );
            ret = 0;
        }
        HeapFree( GetProcessHeap(), 0, tmp );

        TRACE( "(lcid=0x%x,lctype=0x%x,%p,%d) returning number %d\n",
               lcid, lctype, buffer, len, number );
    }
    else
    {
        memcpy( buffer, p + 1, ret * sizeof(WCHAR) );
        if (lctype != LOCALE_FONTSIGNATURE) buffer[ret-1] = 0;

        TRACE( "(lcid=0x%x,lctype=0x%x,%p,%d) returning %d %s\n",
               lcid, lctype, buffer, len, ret, debugstr_w(buffer) );
    }
    return ret;
}


/******************************************************************************
 *		SetLocaleInfoA	[KERNEL32.@]
 *
 * Set information about an aspect of a locale.
 *
 * PARAMS
 *  lcid   [I] LCID of the locale
 *  lctype [I] LCTYPE_ flags from "winnls.h"
 *  data   [I] Information to set
 *
 * RETURNS
 *  Success: TRUE. The information given will be returned by GetLocaleInfoA()
 *           whenever it is called without LOCALE_NOUSEROVERRIDE.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 *
 * NOTES
 *  - Values are only be set for the current user locale; the system locale
 *  settings cannot be changed.
 *  - Any settings changed by this call are lost when the locale is changed by
 *  the control panel (in Wine, this happens every time you change LANG).
 *  - The native implementation of this function does not check that lcid matches
 *  the current user locale, and simply sets the new values. Wine warns you in
 *  this case, but behaves the same.
 */
BOOL WINAPI SetLocaleInfoA(LCID lcid, LCTYPE lctype, LPCSTR data)
{
    UINT codepage = CP_ACP;
    WCHAR *strW;
    DWORD len;
    BOOL ret;

    if (!(lctype & LOCALE_USE_CP_ACP)) codepage = get_lcid_codepage( lcid );

    if (!data)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    len = MultiByteToWideChar( codepage, 0, data, -1, NULL, 0 );
    if (!(strW = HeapAlloc( GetProcessHeap(), 0, len * sizeof(WCHAR) )))
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }
    MultiByteToWideChar( codepage, 0, data, -1, strW, len );
    ret = SetLocaleInfoW( lcid, lctype, strW );
    HeapFree( GetProcessHeap(), 0, strW );
    return ret;
}


/******************************************************************************
 *		SetLocaleInfoW	(KERNEL32.@)
 *
 * See SetLocaleInfoA.
 */
BOOL WINAPI SetLocaleInfoW( LCID lcid, LCTYPE lctype, LPCWSTR data )
{
    const WCHAR *value;
    static const WCHAR intlW[] = {'i','n','t','l',0 };
    UNICODE_STRING valueW;
    NTSTATUS status;
    HANDLE hkey;

    lctype &= 0xffff;
    value = get_locale_value_name( lctype );

    if (!data || !value)
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (lctype == LOCALE_IDATE || lctype == LOCALE_ILDATE)
    {
        SetLastError( ERROR_INVALID_FLAGS );
        return FALSE;
    }

    TRACE("setting %x (%s) to %s\n", lctype, debugstr_w(value), debugstr_w(data) );

    /* FIXME: should check that data to set is sane */

    /* FIXME: profile functions should map to registry */
    WriteProfileStringW( intlW, value, data );

    if (!(hkey = create_registry_key())) return FALSE;
    RtlInitUnicodeString( &valueW, value );
    status = NtSetValueKey( hkey, &valueW, 0, REG_SZ, data, (strlenW(data)+1)*sizeof(WCHAR) );

    if (lctype == LOCALE_SSHORTDATE || lctype == LOCALE_SLONGDATE)
    {
      /* Set I-value from S value */
      WCHAR *lpD, *lpM, *lpY;
      WCHAR szBuff[2];

      lpD = strrchrW(data, 'd');
      lpM = strrchrW(data, 'M');
      lpY = strrchrW(data, 'y');

      if (lpD <= lpM)
      {
        szBuff[0] = '1'; /* D-M-Y */
      }
      else
      {
        if (lpY <= lpM)
          szBuff[0] = '2'; /* Y-M-D */
        else
          szBuff[0] = '0'; /* M-D-Y */
      }

      szBuff[1] = '\0';

      if (lctype == LOCALE_SSHORTDATE)
        lctype = LOCALE_IDATE;
      else
        lctype = LOCALE_ILDATE;

      value = get_locale_value_name( lctype );

      WriteProfileStringW( intlW, value, szBuff );

      RtlInitUnicodeString( &valueW, value );
      status = NtSetValueKey( hkey, &valueW, 0, REG_SZ, szBuff, sizeof(szBuff) );
    }

    NtClose( hkey );

    if (status) SetLastError( RtlNtStatusToDosError(status) );
    return !status;
}

/***********************************************************************
 *           GetThreadLocale    (KERNEL32.@)
 *
 * Get the current threads locale.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  The LCID currently associated with the calling thread.
 */
LCID WINAPI GetThreadLocale(void)
{
    LCID ret = NtCurrentTeb()->CurrentLocale;
    if (!ret) NtCurrentTeb()->CurrentLocale = ret = GetUserDefaultLCID();
    return ret;
}

/**********************************************************************
 *           SetThreadLocale    (KERNEL32.@)
 *
 * Set the current threads locale.
 *
 * PARAMS
 *  lcid [I] LCID of the locale to set
 *
 * RETURNS
 *  Success: TRUE. The threads locale is set to lcid.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI SetThreadLocale( LCID lcid )
{
    TRACE("(0x%04X)\n", lcid);

    lcid = ConvertDefaultLocale(lcid);

    if (lcid != GetThreadLocale())
    {
        if (!IsValidLocale(lcid, LCID_SUPPORTED))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        NtCurrentTeb()->CurrentLocale = lcid;
    }
    return TRUE;
}

/******************************************************************************
 *		ConvertDefaultLocale (KERNEL32.@)
 *
 * Convert a default locale identifier into a real identifier.
 *
 * PARAMS
 *  lcid [I] LCID identifier of the locale to convert
 *
 * RETURNS
 *  lcid unchanged, if not a default locale or its sublanguage is
 *   not SUBLANG_NEUTRAL.
 *  GetSystemDefaultLCID(), if lcid == LOCALE_SYSTEM_DEFAULT.
 *  GetUserDefaultLCID(), if lcid == LOCALE_USER_DEFAULT or LOCALE_NEUTRAL.
 *  Otherwise, lcid with sublanguage changed to SUBLANG_DEFAULT.
 */
LCID WINAPI ConvertDefaultLocale( LCID lcid )
{
    LANGID langid;

    switch (lcid)
    {
    case LOCALE_SYSTEM_DEFAULT:
        lcid = GetSystemDefaultLCID();
        break;
    case LOCALE_USER_DEFAULT:
    case LOCALE_NEUTRAL:
        lcid = GetUserDefaultLCID();
        break;
    default:
        /* Replace SUBLANG_NEUTRAL with SUBLANG_DEFAULT */
        langid = LANGIDFROMLCID(lcid);
        if (SUBLANGID(langid) == SUBLANG_NEUTRAL)
        {
          langid = MAKELANGID(PRIMARYLANGID(langid), SUBLANG_DEFAULT);
          lcid = MAKELCID(langid, SORTIDFROMLCID(lcid));
        }
    }
    return lcid;
}


/******************************************************************************
 *           IsValidLocale   (KERNEL32.@)
 *
 * Determine if a locale is valid.
 *
 * PARAMS
 *  lcid  [I] LCID of the locale to check
 *  flags [I] LCID_SUPPORTED = Valid, LCID_INSTALLED = Valid and installed on the system
 *
 * RETURNS
 *  TRUE,  if lcid is valid,
 *  FALSE, otherwise.
 *
 * NOTES
 *  Wine does not currently make the distinction between supported and installed. All
 *  languages supported are installed by default.
 */
BOOL WINAPI IsValidLocale( LCID lcid, DWORD flags )
{
    /* check if language is registered in the kernel32 resources */
    return FindResourceExW( kernel32_handle, (LPWSTR)RT_STRING,
                            (LPCWSTR)LOCALE_ILANGUAGE, LANGIDFROMLCID(lcid)) != 0;
}


static BOOL CALLBACK enum_lang_proc_a( HMODULE hModule, LPCSTR type,
                                       LPCSTR name, WORD LangID, LONG_PTR lParam )
{
    LOCALE_ENUMPROCA lpfnLocaleEnum = (LOCALE_ENUMPROCA)lParam;
    char buf[20];

    sprintf(buf, "%08x", (UINT)LangID);
    return lpfnLocaleEnum( buf );
}

static BOOL CALLBACK enum_lang_proc_w( HMODULE hModule, LPCWSTR type,
                                       LPCWSTR name, WORD LangID, LONG_PTR lParam )
{
    static const WCHAR formatW[] = {'%','0','8','x',0};
    LOCALE_ENUMPROCW lpfnLocaleEnum = (LOCALE_ENUMPROCW)lParam;
    WCHAR buf[20];
    sprintfW( buf, formatW, (UINT)LangID );
    return lpfnLocaleEnum( buf );
}

/******************************************************************************
 *           EnumSystemLocalesA  (KERNEL32.@)
 *
 * Call a users function for each locale available on the system.
 *
 * PARAMS
 *  lpfnLocaleEnum [I] Callback function to call for each locale
 *  dwFlags        [I] LOCALE_SUPPORTED=All supported, LOCALE_INSTALLED=Installed only
 *
 * RETURNS
 *  Success: TRUE.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI EnumSystemLocalesA( LOCALE_ENUMPROCA lpfnLocaleEnum, DWORD dwFlags )
{
    TRACE("(%p,%08x)\n", lpfnLocaleEnum, dwFlags);
    EnumResourceLanguagesA( kernel32_handle, (LPSTR)RT_STRING,
                            (LPCSTR)LOCALE_ILANGUAGE, enum_lang_proc_a,
                            (LONG_PTR)lpfnLocaleEnum);
    return TRUE;
}


/******************************************************************************
 *           EnumSystemLocalesW  (KERNEL32.@)
 *
 * See EnumSystemLocalesA.
 */
BOOL WINAPI EnumSystemLocalesW( LOCALE_ENUMPROCW lpfnLocaleEnum, DWORD dwFlags )
{
    TRACE("(%p,%08x)\n", lpfnLocaleEnum, dwFlags);
    EnumResourceLanguagesW( kernel32_handle, (LPWSTR)RT_STRING,
                            (LPCWSTR)LOCALE_ILANGUAGE, enum_lang_proc_w,
                            (LONG_PTR)lpfnLocaleEnum);
    return TRUE;
}

/***********************************************************************
 *           VerLanguageNameA  (KERNEL32.@)
 *
 * Get the name of a language.
 *
 * PARAMS
 *  wLang  [I] LANGID of the language
 *  szLang [O] Destination for the language name
 *
 * RETURNS
 *  Success: The size of the language name. If szLang is non-NULL, it is filled
 *           with the name.
 *  Failure: 0. Use GetLastError() to determine the cause.
 *
 */
DWORD WINAPI VerLanguageNameA( DWORD wLang, LPSTR szLang, DWORD nSize )
{
    return GetLocaleInfoA( MAKELCID(wLang, SORT_DEFAULT), LOCALE_SENGLANGUAGE, szLang, nSize );
}


/***********************************************************************
 *           VerLanguageNameW  (KERNEL32.@)
 *
 * See VerLanguageNameA.
 */
DWORD WINAPI VerLanguageNameW( DWORD wLang, LPWSTR szLang, DWORD nSize )
{
    return GetLocaleInfoW( MAKELCID(wLang, SORT_DEFAULT), LOCALE_SENGLANGUAGE, szLang, nSize );
}

/******************************************************************************
 *           GetStringTypeW    (KERNEL32.@)
 *
 * See GetStringTypeA.
 */
BOOL WINAPI GetStringTypeW( DWORD type, LPCWSTR src, INT count, LPWORD chartype )
{
    static const unsigned char type2_map[16] =
    {
        C2_NOTAPPLICABLE,      /* unassigned */
        C2_LEFTTORIGHT,        /* L */
        C2_RIGHTTOLEFT,        /* R */
        C2_EUROPENUMBER,       /* EN */
        C2_EUROPESEPARATOR,    /* ES */
        C2_EUROPETERMINATOR,   /* ET */
        C2_ARABICNUMBER,       /* AN */
        C2_COMMONSEPARATOR,    /* CS */
        C2_BLOCKSEPARATOR,     /* B */
        C2_SEGMENTSEPARATOR,   /* S */
        C2_WHITESPACE,         /* WS */
        C2_OTHERNEUTRAL,       /* ON */
        C2_RIGHTTOLEFT,        /* AL */
        C2_NOTAPPLICABLE,      /* NSM */
        C2_NOTAPPLICABLE,      /* BN */
        C2_OTHERNEUTRAL        /* LRE, LRO, RLE, RLO, PDF */
    };

    if (count == -1) count = strlenW(src) + 1;
    switch(type)
    {
    case CT_CTYPE1:
        while (count--) *chartype++ = get_char_typeW( *src++ ) & 0xfff;
        break;
    case CT_CTYPE2:
        while (count--) *chartype++ = type2_map[get_char_typeW( *src++ ) >> 12];
        break;
    case CT_CTYPE3:
    {
        WARN("CT_CTYPE3: semi-stub.\n");
        while (count--)
        {
            int c = *src;
            WORD type1, type3 = 0; /* C3_NOTAPPLICABLE */

            type1 = get_char_typeW( *src++ ) & 0xfff;
            /* try to construct type3 from type1 */
            if(type1 & C1_SPACE) type3 |= C3_SYMBOL;
            if(type1 & C1_ALPHA) type3 |= C3_ALPHA;
            if ((c>=0x30A0)&&(c<=0x30FF)) type3 |= C3_KATAKANA;
            if ((c>=0x3040)&&(c<=0x309F)) type3 |= C3_HIRAGANA;
            if ((c>=0x4E00)&&(c<=0x9FAF)) type3 |= C3_IDEOGRAPH;
            if ((c>=0x0600)&&(c<=0x06FF)) type3 |= C3_KASHIDA;
            if ((c>=0x3000)&&(c<=0x303F)) type3 |= C3_SYMBOL;

            if ((c>=0xFF00)&&(c<=0xFF60)) type3 |= C3_FULLWIDTH;
            if ((c>=0xFF00)&&(c<=0xFF20)) type3 |= C3_SYMBOL;
            if ((c>=0xFF3B)&&(c<=0xFF40)) type3 |= C3_SYMBOL;
            if ((c>=0xFF5B)&&(c<=0xFF60)) type3 |= C3_SYMBOL;
            if ((c>=0xFF21)&&(c<=0xFF3A)) type3 |= C3_ALPHA;
            if ((c>=0xFF41)&&(c<=0xFF5A)) type3 |= C3_ALPHA;
            if ((c>=0xFFE0)&&(c<=0xFFE6)) type3 |= C3_FULLWIDTH;
            if ((c>=0xFFE0)&&(c<=0xFFE6)) type3 |= C3_SYMBOL;

            if ((c>=0xFF61)&&(c<=0xFFDC)) type3 |= C3_HALFWIDTH;
            if ((c>=0xFF61)&&(c<=0xFF64)) type3 |= C3_SYMBOL;
            if ((c>=0xFF65)&&(c<=0xFF9F)) type3 |= C3_KATAKANA;
            if ((c>=0xFF65)&&(c<=0xFF9F)) type3 |= C3_ALPHA;
            if ((c>=0xFFE8)&&(c<=0xFFEE)) type3 |= C3_HALFWIDTH;
            if ((c>=0xFFE8)&&(c<=0xFFEE)) type3 |= C3_SYMBOL;
            *chartype++ = type3;
        }
        break;
    }
    default:
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }
    return TRUE;
}


/******************************************************************************
 *           GetStringTypeExW    (KERNEL32.@)
 *
 * See GetStringTypeExA.
 */
BOOL WINAPI GetStringTypeExW( LCID locale, DWORD type, LPCWSTR src, INT count, LPWORD chartype )
{
    /* locale is ignored for Unicode */
    return GetStringTypeW( type, src, count, chartype );
}


/******************************************************************************
 *           GetStringTypeA    (KERNEL32.@)
 *
 * Get characteristics of the characters making up a string.
 *
 * PARAMS
 *  locale   [I] Locale Id for the string
 *  type     [I] CT_CTYPE1 = classification, CT_CTYPE2 = directionality, CT_CTYPE3 = typographic info
 *  src      [I] String to analyse
 *  count    [I] Length of src in chars, or -1 if src is NUL terminated
 *  chartype [O] Destination for the calculated characteristics
 *
 * RETURNS
 *  Success: TRUE. chartype is filled with the requested characteristics of each char
 *           in src.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI GetStringTypeA( LCID locale, DWORD type, LPCSTR src, INT count, LPWORD chartype )
{
    UINT cp;
    INT countW;
    LPWSTR srcW;
    BOOL ret = FALSE;

    if(count == -1) count = strlen(src) + 1;

    if (!(cp = get_lcid_codepage( locale )))
    {
        FIXME("For locale %04x using current ANSI code page\n", locale);
        cp = GetACP();
    }

    countW = MultiByteToWideChar(cp, 0, src, count, NULL, 0);
    if((srcW = HeapAlloc(GetProcessHeap(), 0, countW * sizeof(WCHAR))))
    {
        MultiByteToWideChar(cp, 0, src, count, srcW, countW);
    /*
     * NOTE: the target buffer has 1 word for each CHARACTER in the source
     * string, with multibyte characters there maybe be more bytes in count
     * than character space in the buffer!
     */
        ret = GetStringTypeW(type, srcW, countW, chartype);
        HeapFree(GetProcessHeap(), 0, srcW);
    }
    return ret;
}

/******************************************************************************
 *           GetStringTypeExA    (KERNEL32.@)
 *
 * Get characteristics of the characters making up a string.
 *
 * PARAMS
 *  locale   [I] Locale Id for the string
 *  type     [I] CT_CTYPE1 = classification, CT_CTYPE2 = directionality, CT_CTYPE3 = typographic info
 *  src      [I] String to analyse
 *  count    [I] Length of src in chars, or -1 if src is NUL terminated
 *  chartype [O] Destination for the calculated characteristics
 *
 * RETURNS
 *  Success: TRUE. chartype is filled with the requested characteristics of each char
 *           in src.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI GetStringTypeExA( LCID locale, DWORD type, LPCSTR src, INT count, LPWORD chartype )
{
    return GetStringTypeA(locale, type, src, count, chartype);
}


/*************************************************************************
 *           LCMapStringW    (KERNEL32.@)
 *
 * See LCMapStringA.
 */
INT WINAPI LCMapStringW(LCID lcid, DWORD flags, LPCWSTR src, INT srclen,
                        LPWSTR dst, INT dstlen)
{
    LPWSTR dst_ptr;

    if (!src || !srclen || dstlen < 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    /* mutually exclusive flags */
    if ((flags & (LCMAP_LOWERCASE | LCMAP_UPPERCASE)) == (LCMAP_LOWERCASE | LCMAP_UPPERCASE) ||
        (flags & (LCMAP_HIRAGANA | LCMAP_KATAKANA)) == (LCMAP_HIRAGANA | LCMAP_KATAKANA) ||
        (flags & (LCMAP_HALFWIDTH | LCMAP_FULLWIDTH)) == (LCMAP_HALFWIDTH | LCMAP_FULLWIDTH) ||
        (flags & (LCMAP_TRADITIONAL_CHINESE | LCMAP_SIMPLIFIED_CHINESE)) == (LCMAP_TRADITIONAL_CHINESE | LCMAP_SIMPLIFIED_CHINESE))
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return 0;
    }

    if (!dstlen) dst = NULL;

    lcid = ConvertDefaultLocale(lcid);

    if (flags & LCMAP_SORTKEY)
    {
        INT ret;
        if (src == dst)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return 0;
        }

        if (srclen < 0) srclen = strlenW(src);

        TRACE("(0x%04x,0x%08x,%s,%d,%p,%d)\n",
              lcid, flags, debugstr_wn(src, srclen), srclen, dst, dstlen);

        ret = wine_get_sortkey(flags, src, srclen, (char *)dst, dstlen);
        if (ret == 0)
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        else
            ret++;
        return ret;
    }

    /* SORT_STRINGSORT must be used exclusively with LCMAP_SORTKEY */
    if (flags & SORT_STRINGSORT)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return 0;
    }

    if (srclen < 0) srclen = strlenW(src) + 1;

    TRACE("(0x%04x,0x%08x,%s,%d,%p,%d)\n",
          lcid, flags, debugstr_wn(src, srclen), srclen, dst, dstlen);

    if (!dst) /* return required string length */
    {
        INT len;

        for (len = 0; srclen; src++, srclen--)
        {
            WCHAR wch = *src;
            /* tests show that win2k just ignores NORM_IGNORENONSPACE,
             * and skips white space and punctuation characters for
             * NORM_IGNORESYMBOLS.
             */
            if ((flags & NORM_IGNORESYMBOLS) && (get_char_typeW(wch) & (C1_PUNCT | C1_SPACE)))
                continue;
            len++;
        }
        return len;
    }

    if (flags & LCMAP_UPPERCASE)
    {
        for (dst_ptr = dst; srclen && dstlen; src++, srclen--)
        {
            WCHAR wch = *src;
            if ((flags & NORM_IGNORESYMBOLS) && (get_char_typeW(wch) & (C1_PUNCT | C1_SPACE)))
                continue;
            *dst_ptr++ = toupperW(wch);
            dstlen--;
        }
    }
    else if (flags & LCMAP_LOWERCASE)
    {
        for (dst_ptr = dst; srclen && dstlen; src++, srclen--)
        {
            WCHAR wch = *src;
            if ((flags & NORM_IGNORESYMBOLS) && (get_char_typeW(wch) & (C1_PUNCT | C1_SPACE)))
                continue;
            *dst_ptr++ = tolowerW(wch);
            dstlen--;
        }
    }
    else
    {
        if (src == dst)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return 0;
        }
        for (dst_ptr = dst; srclen && dstlen; src++, srclen--)
        {
            WCHAR wch = *src;
            if ((flags & NORM_IGNORESYMBOLS) && (get_char_typeW(wch) & (C1_PUNCT | C1_SPACE)))
                continue;
            *dst_ptr++ = wch;
            dstlen--;
        }
    }

    if (srclen)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

    return dst_ptr - dst;
}

/*************************************************************************
 *           LCMapStringA    (KERNEL32.@)
 *
 * Map characters in a locale sensitive string.
 *
 * PARAMS
 *  lcid   [I] LCID for the conversion.
 *  flags  [I] Flags controlling the mapping (LCMAP_ constants from "winnls.h").
 *  src    [I] String to map
 *  srclen [I] Length of src in chars, or -1 if src is NUL terminated
 *  dst    [O] Destination for mapped string
 *  dstlen [I] Length of dst in characters
 *
 * RETURNS
 *  Success: The length of the mapped string in dst, including the NUL terminator.
 *  Failure: 0. Use GetLastError() to determine the cause.
 */
INT WINAPI LCMapStringA(LCID lcid, DWORD flags, LPCSTR src, INT srclen,
                        LPSTR dst, INT dstlen)
{
    WCHAR *bufW = NtCurrentTeb()->StaticUnicodeBuffer;
    LPWSTR srcW, dstW;
    INT ret = 0, srclenW, dstlenW;
    UINT locale_cp = CP_ACP;

    if (!src || !srclen || dstlen < 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!(flags & LOCALE_USE_CP_ACP)) locale_cp = get_lcid_codepage( lcid );

    srclenW = MultiByteToWideChar(locale_cp, 0, src, srclen, bufW, 260);
    if (srclenW)
        srcW = bufW;
    else
    {
        srclenW = MultiByteToWideChar(locale_cp, 0, src, srclen, NULL, 0);
        srcW = HeapAlloc(GetProcessHeap(), 0, srclenW * sizeof(WCHAR));
        if (!srcW)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        MultiByteToWideChar(locale_cp, 0, src, srclen, srcW, srclenW);
    }

    if (flags & LCMAP_SORTKEY)
    {
        if (src == dst)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            goto map_string_exit;
        }
        ret = wine_get_sortkey(flags, srcW, srclenW, dst, dstlen);
        if (ret == 0)
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        else
            ret++;
        goto map_string_exit;
    }

    if (flags & SORT_STRINGSORT)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        goto map_string_exit;
    }

    dstlenW = LCMapStringW(lcid, flags, srcW, srclenW, NULL, 0);
    if (!dstlenW)
        goto map_string_exit;

    dstW = HeapAlloc(GetProcessHeap(), 0, dstlenW * sizeof(WCHAR));
    if (!dstW)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto map_string_exit;
    }

    LCMapStringW(lcid, flags, srcW, srclenW, dstW, dstlenW);
    ret = WideCharToMultiByte(locale_cp, 0, dstW, dstlenW, dst, dstlen, NULL, NULL);
    HeapFree(GetProcessHeap(), 0, dstW);

map_string_exit:
    if (srcW != bufW) HeapFree(GetProcessHeap(), 0, srcW);
    return ret;
}

/*************************************************************************
 *           FoldStringA    (KERNEL32.@)
 *
 * Map characters in a string.
 *
 * PARAMS
 *  dwFlags [I] Flags controlling chars to map (MAP_ constants from "winnls.h")
 *  src     [I] String to map
 *  srclen  [I] Length of src, or -1 if src is NUL terminated
 *  dst     [O] Destination for mapped string
 *  dstlen  [I] Length of dst, or 0 to find the required length for the mapped string
 *
 * RETURNS
 *  Success: The length of the string written to dst, including the terminating NUL. If
 *           dstlen is 0, the value returned is the same, but nothing is written to dst,
 *           and dst may be NULL.
 *  Failure: 0. Use GetLastError() to determine the cause.
 */
INT WINAPI FoldStringA(DWORD dwFlags, LPCSTR src, INT srclen,
                       LPSTR dst, INT dstlen)
{
    INT ret = 0, srclenW = 0;
    WCHAR *srcW = NULL, *dstW = NULL;

    if (!src || !srclen || dstlen < 0 || (dstlen && !dst) || src == dst)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    srclenW = MultiByteToWideChar(CP_ACP, dwFlags & MAP_COMPOSITE ? MB_COMPOSITE : 0,
                                  src, srclen, NULL, 0);
    srcW = HeapAlloc(GetProcessHeap(), 0, srclenW * sizeof(WCHAR));

    if (!srcW)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto FoldStringA_exit;
    }

    MultiByteToWideChar(CP_ACP, dwFlags & MAP_COMPOSITE ? MB_COMPOSITE : 0,
                        src, srclen, srcW, srclenW);

    dwFlags = (dwFlags & ~MAP_PRECOMPOSED) | MAP_FOLDCZONE;

    ret = FoldStringW(dwFlags, srcW, srclenW, NULL, 0);
    if (ret && dstlen)
    {
        dstW = HeapAlloc(GetProcessHeap(), 0, ret * sizeof(WCHAR));

        if (!dstW)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto FoldStringA_exit;
        }

        ret = FoldStringW(dwFlags, srcW, srclenW, dstW, ret);
        if (!WideCharToMultiByte(CP_ACP, 0, dstW, ret, dst, dstlen, NULL, NULL))
        {
            ret = 0;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    HeapFree(GetProcessHeap(), 0, dstW);

FoldStringA_exit:
    HeapFree(GetProcessHeap(), 0, srcW);
    return ret;
}

/*************************************************************************
 *           FoldStringW    (KERNEL32.@)
 *
 * See FoldStringA.
 */
INT WINAPI FoldStringW(DWORD dwFlags, LPCWSTR src, INT srclen,
                       LPWSTR dst, INT dstlen)
{
    int ret;

    switch (dwFlags & (MAP_COMPOSITE|MAP_PRECOMPOSED|MAP_EXPAND_LIGATURES))
    {
    case 0:
        if (dwFlags)
          break;
        /* Fall through for dwFlags == 0 */
    case MAP_PRECOMPOSED|MAP_COMPOSITE:
    case MAP_PRECOMPOSED|MAP_EXPAND_LIGATURES:
    case MAP_COMPOSITE|MAP_EXPAND_LIGATURES:
        SetLastError(ERROR_INVALID_FLAGS);
        return 0;
    }

    if (!src || !srclen || dstlen < 0 || (dstlen && !dst) || src == dst)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    ret = wine_fold_string(dwFlags, src, srclen, dst, dstlen);
    if (!ret)
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return ret;
}

/******************************************************************************
 *           CompareStringW    (KERNEL32.@)
 *
 * See CompareStringA.
 */
INT WINAPI CompareStringW(LCID lcid, DWORD style,
                          LPCWSTR str1, INT len1, LPCWSTR str2, INT len2)
{
    INT ret;

    if (!str1 || !str2)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if( style & ~(NORM_IGNORECASE|NORM_IGNORENONSPACE|NORM_IGNORESYMBOLS|
        SORT_STRINGSORT|NORM_IGNOREKANATYPE|NORM_IGNOREWIDTH|LOCALE_USE_CP_ACP|0x10000000) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return 0;
    }

    /* this style is related to diacritics in Arabic, Japanese, and Hebrew */
    if (style & 0x10000000)
        WARN("Ignoring unknown style 0x10000000\n");

    if (len1 < 0) len1 = strlenW(str1);
    if (len2 < 0) len2 = strlenW(str2);

    ret = wine_compare_string(style, str1, len1, str2, len2);

    if (ret) /* need to translate result */
        return (ret < 0) ? CSTR_LESS_THAN : CSTR_GREATER_THAN;
    return CSTR_EQUAL;
}

/******************************************************************************
 *           CompareStringA    (KERNEL32.@)
 *
 * Compare two locale sensitive strings.
 *
 * PARAMS
 *  lcid  [I] LCID for the comparison
 *  style [I] Flags for the comparison (NORM_ constants from "winnls.h").
 *  str1  [I] First string to compare
 *  len1  [I] Length of str1, or -1 if str1 is NUL terminated
 *  str2  [I] Second string to compare
 *  len2  [I] Length of str2, or -1 if str2 is NUL terminated
 *
 * RETURNS
 *  Success: CSTR_LESS_THAN, CSTR_EQUAL or CSTR_GREATER_THAN depending on whether
 *           str1 is less than, equal to or greater than str2 respectively.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
INT WINAPI CompareStringA(LCID lcid, DWORD style,
                          LPCSTR str1, INT len1, LPCSTR str2, INT len2)
{
    WCHAR *buf1W = NtCurrentTeb()->StaticUnicodeBuffer;
    WCHAR *buf2W = buf1W + 130;
    LPWSTR str1W, str2W;
    INT len1W, len2W, ret;
    UINT locale_cp = CP_ACP;

    if (!str1 || !str2)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    if (len1 < 0) len1 = strlen(str1);
    if (len2 < 0) len2 = strlen(str2);

    if (!(style & LOCALE_USE_CP_ACP)) locale_cp = get_lcid_codepage( lcid );

    len1W = MultiByteToWideChar(locale_cp, 0, str1, len1, buf1W, 130);
    if (len1W)
        str1W = buf1W;
    else
    {
        len1W = MultiByteToWideChar(locale_cp, 0, str1, len1, NULL, 0);
        str1W = HeapAlloc(GetProcessHeap(), 0, len1W * sizeof(WCHAR));
        if (!str1W)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        MultiByteToWideChar(locale_cp, 0, str1, len1, str1W, len1W);
    }
    len2W = MultiByteToWideChar(locale_cp, 0, str2, len2, buf2W, 130);
    if (len2W)
        str2W = buf2W;
    else
    {
        len2W = MultiByteToWideChar(locale_cp, 0, str2, len2, NULL, 0);
        str2W = HeapAlloc(GetProcessHeap(), 0, len2W * sizeof(WCHAR));
        if (!str2W)
        {
            if (str1W != buf1W) HeapFree(GetProcessHeap(), 0, str1W);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return 0;
        }
        MultiByteToWideChar(locale_cp, 0, str2, len2, str2W, len2W);
    }

    ret = CompareStringW(lcid, style, str1W, len1W, str2W, len2W);

    if (str1W != buf1W) HeapFree(GetProcessHeap(), 0, str1W);
    if (str2W != buf2W) HeapFree(GetProcessHeap(), 0, str2W);
    return ret;
}

static HANDLE NLS_RegOpenKey(HANDLE hRootKey, LPCWSTR szKeyName)
{
    UNICODE_STRING keyName;
    OBJECT_ATTRIBUTES attr;
    HANDLE hkey;

    RtlInitUnicodeString( &keyName, szKeyName );
    InitializeObjectAttributes(&attr, &keyName, 0, hRootKey, NULL);

    if (NtOpenKey( &hkey, KEY_READ, &attr ) != STATUS_SUCCESS)
        hkey = 0;

    return hkey;
}

static BOOL NLS_RegEnumSubKey(HANDLE hKey, UINT ulIndex, LPWSTR szKeyName,
                              ULONG keyNameSize)
{
    BYTE buffer[80];
    KEY_BASIC_INFORMATION *info = (KEY_BASIC_INFORMATION *)buffer;
    DWORD dwLen;

    if (NtEnumerateKey( hKey, ulIndex, KeyBasicInformation, buffer,
                        sizeof(buffer), &dwLen) != STATUS_SUCCESS ||
        info->NameLength > keyNameSize)
    {
        return FALSE;
    }

    TRACE("info->Name %s info->NameLength %d\n", debugstr_w(info->Name), info->NameLength);

    memcpy( szKeyName, info->Name, info->NameLength);
    szKeyName[info->NameLength / sizeof(WCHAR)] = '\0';

    TRACE("returning %s\n", debugstr_w(szKeyName));
    return TRUE;
}

static BOOL NLS_RegEnumValue(HANDLE hKey, UINT ulIndex,
                             LPWSTR szValueName, ULONG valueNameSize,
                             LPWSTR szValueData, ULONG valueDataSize)
{
    BYTE buffer[80];
    KEY_VALUE_FULL_INFORMATION *info = (KEY_VALUE_FULL_INFORMATION *)buffer;
    DWORD dwLen;

    if (NtEnumerateValueKey( hKey, ulIndex, KeyValueFullInformation,
        buffer, sizeof(buffer), &dwLen ) != STATUS_SUCCESS ||
        info->NameLength > valueNameSize ||
        info->DataLength > valueDataSize)
    {
        return FALSE;
    }

    TRACE("info->Name %s info->DataLength %d\n", debugstr_w(info->Name), info->DataLength);

    memcpy( szValueName, info->Name, info->NameLength);
    szValueName[info->NameLength / sizeof(WCHAR)] = '\0';
    memcpy( szValueData, buffer + info->DataOffset, info->DataLength );
    szValueData[info->DataLength / sizeof(WCHAR)] = '\0';

    TRACE("returning %s %s\n", debugstr_w(szValueName), debugstr_w(szValueData));
    return TRUE;
}

static BOOL NLS_RegGetDword(HANDLE hKey, LPCWSTR szValueName, DWORD *lpVal)
{
    BYTE buffer[128];
    const KEY_VALUE_PARTIAL_INFORMATION *info = (KEY_VALUE_PARTIAL_INFORMATION *)buffer;
    DWORD dwSize = sizeof(buffer);
    UNICODE_STRING valueName;

    RtlInitUnicodeString( &valueName, szValueName );

    TRACE("%p, %s\n", hKey, debugstr_w(szValueName));
    if (NtQueryValueKey( hKey, &valueName, KeyValuePartialInformation,
                         buffer, dwSize, &dwSize ) == STATUS_SUCCESS &&
        info->DataLength == sizeof(DWORD))
    {
        memcpy(lpVal, info->Data, sizeof(DWORD));
        return TRUE;
    }

    return FALSE;
}

static BOOL NLS_GetLanguageGroupName(LGRPID lgrpid, LPWSTR szName, ULONG nameSize)
{
    LANGID  langId;
    LPCWSTR szResourceName = MAKEINTRESOURCEW(((lgrpid + 0x2000) >> 4) + 1);
    HRSRC   hResource;
    BOOL    bRet = FALSE;

    /* FIXME: Is it correct to use the system default langid? */
    langId = GetSystemDefaultLangID();

    if (SUBLANGID(langId) == SUBLANG_NEUTRAL)
        langId = MAKELANGID( PRIMARYLANGID(langId), SUBLANG_DEFAULT );

    hResource = FindResourceExW( kernel32_handle, (LPWSTR)RT_STRING, szResourceName, langId );

    if (hResource)
    {
        HGLOBAL hResDir = LoadResource( kernel32_handle, hResource );

        if (hResDir)
        {
            ULONG   iResourceIndex = lgrpid & 0xf;
            LPCWSTR lpResEntry = LockResource( hResDir );
            ULONG   i;

            for (i = 0; i < iResourceIndex; i++)
                lpResEntry += *lpResEntry + 1;

            if (*lpResEntry < nameSize)
            {
                memcpy( szName, lpResEntry + 1, *lpResEntry * sizeof(WCHAR) );
                szName[*lpResEntry] = '\0';
                bRet = TRUE;
            }

        }
        FreeResource( hResource );
    }
    return bRet;
}

/* Registry keys for NLS related information */

static const WCHAR szCountryListName[] = {
    'M','a','c','h','i','n','e','\\','S','o','f','t','w','a','r','e','\\',
    'M','i','c','r','o','s','o','f','t','\\','W','i','n','d','o','w','s','\\',
    'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
    'T','e','l','e','p','h','o','n','y','\\',
    'C','o','u','n','t','r','y',' ','L','i','s','t','\0'
};


/* Callback function ptrs for EnumSystemLanguageGroupsA/W */
typedef struct
{
  LANGUAGEGROUP_ENUMPROCA procA;
  LANGUAGEGROUP_ENUMPROCW procW;
  DWORD    dwFlags;
  LONG_PTR lParam;
} ENUMLANGUAGEGROUP_CALLBACKS;

/* Internal implementation of EnumSystemLanguageGroupsA/W */
static BOOL NLS_EnumSystemLanguageGroups(ENUMLANGUAGEGROUP_CALLBACKS *lpProcs)
{
    WCHAR szNumber[10], szValue[4];
    HANDLE hKey;
    BOOL bContinue = TRUE;
    ULONG ulIndex = 0;

    if (!lpProcs)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    switch (lpProcs->dwFlags)
    {
    case 0:
        /* Default to LGRPID_INSTALLED */
        lpProcs->dwFlags = LGRPID_INSTALLED;
        /* Fall through... */
    case LGRPID_INSTALLED:
    case LGRPID_SUPPORTED:
        break;
    default:
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    hKey = NLS_RegOpenKey( 0, szLangGroupsKeyName );

    if (!hKey)
        FIXME("NLS registry key not found. Please apply the default registry file 'wine.inf'\n");

    while (bContinue)
    {
        if (NLS_RegEnumValue( hKey, ulIndex, szNumber, sizeof(szNumber),
                              szValue, sizeof(szValue) ))
        {
            BOOL bInstalled = szValue[0] == '1' ? TRUE : FALSE;
            LGRPID lgrpid = strtoulW( szNumber, NULL, 16 );

            TRACE("grpid %s (%sinstalled)\n", debugstr_w(szNumber),
                   bInstalled ? "" : "not ");

            if (lpProcs->dwFlags == LGRPID_SUPPORTED || bInstalled)
            {
                WCHAR szGrpName[48];

                if (!NLS_GetLanguageGroupName( lgrpid, szGrpName, sizeof(szGrpName) / sizeof(WCHAR) ))
                    szGrpName[0] = '\0';

                if (lpProcs->procW)
                    bContinue = lpProcs->procW( lgrpid, szNumber, szGrpName, lpProcs->dwFlags,
                                                lpProcs->lParam );
                else
                {
                    char szNumberA[sizeof(szNumber)/sizeof(WCHAR)];
                    char szGrpNameA[48];

                    /* FIXME: MSDN doesn't say which code page the W->A translation uses,
                     *        or whether the language names are ever localised. Assume CP_ACP.
                     */

                    WideCharToMultiByte(CP_ACP, 0, szNumber, -1, szNumberA, sizeof(szNumberA), 0, 0);
                    WideCharToMultiByte(CP_ACP, 0, szGrpName, -1, szGrpNameA, sizeof(szGrpNameA), 0, 0);

                    bContinue = lpProcs->procA( lgrpid, szNumberA, szGrpNameA, lpProcs->dwFlags,
                                                lpProcs->lParam );
                }
            }

            ulIndex++;
        }
        else
            bContinue = FALSE;

        if (!bContinue)
            break;
    }

    if (hKey)
        NtClose( hKey );

    return TRUE;
}

/******************************************************************************
 *           EnumSystemLanguageGroupsA    (KERNEL32.@)
 *
 * Call a users function for each language group available on the system.
 *
 * PARAMS
 *  pLangGrpEnumProc [I] Callback function to call for each language group
 *  dwFlags          [I] LGRPID_SUPPORTED=All Supported, LGRPID_INSTALLED=Installed only
 *  lParam           [I] User parameter to pass to pLangGrpEnumProc
 *
 * RETURNS
 *  Success: TRUE.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI EnumSystemLanguageGroupsA(LANGUAGEGROUP_ENUMPROCA pLangGrpEnumProc,
                                      DWORD dwFlags, LONG_PTR lParam)
{
    ENUMLANGUAGEGROUP_CALLBACKS procs;

    TRACE("(%p,0x%08X,0x%08lX)\n", pLangGrpEnumProc, dwFlags, lParam);

    procs.procA = pLangGrpEnumProc;
    procs.procW = NULL;
    procs.dwFlags = dwFlags;
    procs.lParam = lParam;

    return NLS_EnumSystemLanguageGroups( pLangGrpEnumProc ? &procs : NULL);
}

/******************************************************************************
 *           EnumSystemLanguageGroupsW    (KERNEL32.@)
 *
 * See EnumSystemLanguageGroupsA.
 */
BOOL WINAPI EnumSystemLanguageGroupsW(LANGUAGEGROUP_ENUMPROCW pLangGrpEnumProc,
                                      DWORD dwFlags, LONG_PTR lParam)
{
    ENUMLANGUAGEGROUP_CALLBACKS procs;

    TRACE("(%p,0x%08X,0x%08lX)\n", pLangGrpEnumProc, dwFlags, lParam);

    procs.procA = NULL;
    procs.procW = pLangGrpEnumProc;
    procs.dwFlags = dwFlags;
    procs.lParam = lParam;

    return NLS_EnumSystemLanguageGroups( pLangGrpEnumProc ? &procs : NULL);
}

/******************************************************************************
 *           IsValidLanguageGroup    (KERNEL32.@)
 *
 * Determine if a language group is supported and/or installed.
 *
 * PARAMS
 *  lgrpid  [I] Language Group Id (LGRPID_ values from "winnls.h")
 *  dwFlags [I] LGRPID_SUPPORTED=Supported, LGRPID_INSTALLED=Installed
 *
 * RETURNS
 *  TRUE, if lgrpid is supported and/or installed, according to dwFlags.
 *  FALSE otherwise.
 */
BOOL WINAPI IsValidLanguageGroup(LGRPID lgrpid, DWORD dwFlags)
{
    static const WCHAR szFormat[] = { '%','x','\0' };
    WCHAR szValueName[16], szValue[2];
    BOOL bSupported = FALSE, bInstalled = FALSE;
    HANDLE hKey;


    switch (dwFlags)
    {
    case LGRPID_INSTALLED:
    case LGRPID_SUPPORTED:

        hKey = NLS_RegOpenKey( 0, szLangGroupsKeyName );

        sprintfW( szValueName, szFormat, lgrpid );

        if (NLS_RegGetDword( hKey, szValueName, (LPDWORD)szValue ))
        {
            bSupported = TRUE;

            if (szValue[0] == '1')
                bInstalled = TRUE;
        }

        if (hKey)
            NtClose( hKey );

        break;
    }

    if ((dwFlags == LGRPID_SUPPORTED && bSupported) ||
        (dwFlags == LGRPID_INSTALLED && bInstalled))
        return TRUE;

    return FALSE;
}

/* Callback function ptrs for EnumLanguageGrouplocalesA/W */
typedef struct
{
  LANGGROUPLOCALE_ENUMPROCA procA;
  LANGGROUPLOCALE_ENUMPROCW procW;
  DWORD    dwFlags;
  LGRPID   lgrpid;
  LONG_PTR lParam;
} ENUMLANGUAGEGROUPLOCALE_CALLBACKS;

/* Internal implementation of EnumLanguageGrouplocalesA/W */
static BOOL NLS_EnumLanguageGroupLocales(ENUMLANGUAGEGROUPLOCALE_CALLBACKS *lpProcs)
{
    static const WCHAR szAlternateSortsKeyName[] = {
      'A','l','t','e','r','n','a','t','e',' ','S','o','r','t','s','\0'
    };
    WCHAR szNumber[10], szValue[4];
    HANDLE hKey;
    BOOL bContinue = TRUE, bAlternate = FALSE;
    LGRPID lgrpid;
    ULONG ulIndex = 1;  /* Ignore default entry of 1st key */

    if (!lpProcs || !lpProcs->lgrpid || lpProcs->lgrpid > LGRPID_ARMENIAN)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (lpProcs->dwFlags)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    hKey = NLS_RegOpenKey( 0, szLocaleKeyName );

    if (!hKey)
        WARN("NLS registry key not found. Please apply the default registry file 'wine.inf'\n");

    while (bContinue)
    {
        if (NLS_RegEnumValue( hKey, ulIndex, szNumber, sizeof(szNumber),
                              szValue, sizeof(szValue) ))
        {
            lgrpid = strtoulW( szValue, NULL, 16 );

            TRACE("lcid %s, grpid %d (%smatched)\n", debugstr_w(szNumber),
                   lgrpid, lgrpid == lpProcs->lgrpid ? "" : "not ");

            if (lgrpid == lpProcs->lgrpid)
            {
                LCID lcid;

                lcid = strtoulW( szNumber, NULL, 16 );

                /* FIXME: native returns extra text for a few (17/150) locales, e.g:
                 * '00000437          ;Georgian'
                 * At present we only pass the LCID string.
                 */

                if (lpProcs->procW)
                    bContinue = lpProcs->procW( lgrpid, lcid, szNumber, lpProcs->lParam );
                else
                {
                    char szNumberA[sizeof(szNumber)/sizeof(WCHAR)];

                    WideCharToMultiByte(CP_ACP, 0, szNumber, -1, szNumberA, sizeof(szNumberA), 0, 0);

                    bContinue = lpProcs->procA( lgrpid, lcid, szNumberA, lpProcs->lParam );
                }
            }

            ulIndex++;
        }
        else
        {
            /* Finished enumerating this key */
            if (!bAlternate)
            {
                /* Enumerate alternate sorts also */
                hKey = NLS_RegOpenKey( hKey, szAlternateSortsKeyName );
                bAlternate = TRUE;
                ulIndex = 0;
            }
            else
                bContinue = FALSE; /* Finished both keys */
        }

        if (!bContinue)
            break;
    }

    if (hKey)
        NtClose( hKey );

    return TRUE;
}

/******************************************************************************
 *           EnumLanguageGroupLocalesA    (KERNEL32.@)
 *
 * Call a users function for every locale in a language group available on the system.
 *
 * PARAMS
 *  pLangGrpLcEnumProc [I] Callback function to call for each locale
 *  lgrpid             [I] Language group (LGRPID_ values from "winnls.h")
 *  dwFlags            [I] Reserved, set to 0
 *  lParam             [I] User parameter to pass to pLangGrpLcEnumProc
 *
 * RETURNS
 *  Success: TRUE.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI EnumLanguageGroupLocalesA(LANGGROUPLOCALE_ENUMPROCA pLangGrpLcEnumProc,
                                      LGRPID lgrpid, DWORD dwFlags, LONG_PTR lParam)
{
    ENUMLANGUAGEGROUPLOCALE_CALLBACKS callbacks;

    TRACE("(%p,0x%08X,0x%08X,0x%08lX)\n", pLangGrpLcEnumProc, lgrpid, dwFlags, lParam);

    callbacks.procA   = pLangGrpLcEnumProc;
    callbacks.procW   = NULL;
    callbacks.dwFlags = dwFlags;
    callbacks.lgrpid  = lgrpid;
    callbacks.lParam  = lParam;

    return NLS_EnumLanguageGroupLocales( pLangGrpLcEnumProc ? &callbacks : NULL );
}

/******************************************************************************
 *           EnumLanguageGroupLocalesW    (KERNEL32.@)
 *
 * See EnumLanguageGroupLocalesA.
 */
BOOL WINAPI EnumLanguageGroupLocalesW(LANGGROUPLOCALE_ENUMPROCW pLangGrpLcEnumProc,
                                      LGRPID lgrpid, DWORD dwFlags, LONG_PTR lParam)
{
    ENUMLANGUAGEGROUPLOCALE_CALLBACKS callbacks;

    TRACE("(%p,0x%08X,0x%08X,0x%08lX)\n", pLangGrpLcEnumProc, lgrpid, dwFlags, lParam);

    callbacks.procA   = NULL;
    callbacks.procW   = pLangGrpLcEnumProc;
    callbacks.dwFlags = dwFlags;
    callbacks.lgrpid  = lgrpid;
    callbacks.lParam  = lParam;

    return NLS_EnumLanguageGroupLocales( pLangGrpLcEnumProc ? &callbacks : NULL );
}

/* Callback function ptrs for EnumSystemCodePagesA/W */
typedef struct
{
  CODEPAGE_ENUMPROCA procA;
  CODEPAGE_ENUMPROCW procW;
  DWORD    dwFlags;
} ENUMSYSTEMCODEPAGES_CALLBACKS;

/* Internal implementation of EnumSystemCodePagesA/W */
static BOOL NLS_EnumSystemCodePages(ENUMSYSTEMCODEPAGES_CALLBACKS *lpProcs)
{
    WCHAR szNumber[5 + 1], szValue[MAX_PATH];
    HANDLE hKey;
    BOOL bContinue = TRUE;
    ULONG ulIndex = 0;

    if (!lpProcs)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    switch (lpProcs->dwFlags)
    {
        case CP_INSTALLED:
        case CP_SUPPORTED:
            break;
        default:
            SetLastError(ERROR_INVALID_FLAGS);
            return FALSE;
    }

    hKey = NLS_RegOpenKey(0, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\NLS\\CodePage");
    if (!hKey)
    {
        WARN("NLS_RegOpenKey() failed\n");
        return FALSE;
    }

    while (bContinue)
    {
        if (NLS_RegEnumValue(hKey, ulIndex, szNumber, sizeof(szNumber),
                             szValue, sizeof(szValue)))
        {
            if ((lpProcs->dwFlags == CP_SUPPORTED)||
                ((lpProcs->dwFlags == CP_INSTALLED)&&(wcslen(szValue) > 2)))
            {
                if (lpProcs->procW)
                {
                    bContinue = lpProcs->procW(szNumber);
                }
                else
                {
                    char szNumberA[sizeof(szNumber)/sizeof(WCHAR)];

                    WideCharToMultiByte(CP_ACP, 0, szNumber, -1, szNumberA, sizeof(szNumberA), 0, 0);
                    bContinue = lpProcs->procA(szNumberA);
                }
            }

            ulIndex++;

        } else bContinue = FALSE;

        if (!bContinue)
            break;
    }

    if (hKey)
        NtClose(hKey);

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
EnumSystemCodePagesW (
    CODEPAGE_ENUMPROCW  lpCodePageEnumProc,
    DWORD               dwFlags
    )
{
    ENUMSYSTEMCODEPAGES_CALLBACKS procs;

    TRACE("(%p,0x%08X,0x%08lX)\n", lpCodePageEnumProc, dwFlags);

    procs.procA = NULL;
    procs.procW = lpCodePageEnumProc;
    procs.dwFlags = dwFlags;

    return NLS_EnumSystemCodePages(lpCodePageEnumProc ? &procs : NULL);
}


/*
 * @implemented
 */
BOOL
WINAPI
EnumSystemCodePagesA (
    CODEPAGE_ENUMPROCA lpCodePageEnumProc,
    DWORD              dwFlags
    )
{
    ENUMSYSTEMCODEPAGES_CALLBACKS procs;

    TRACE("(%p,0x%08X,0x%08lX)\n", lpCodePageEnumProc, dwFlags);

    procs.procA = lpCodePageEnumProc;
    procs.procW = NULL;
    procs.dwFlags = dwFlags;

    return NLS_EnumSystemCodePages(lpCodePageEnumProc ? &procs : NULL);
}
/******************************************************************************
 *           EnumSystemGeoID    (KERNEL32.@)
 *
 * Call a users function for every location available on the system.
 *
 * PARAMS
 *  geoclass     [I] Type of information desired (SYSGEOTYPE enum from "winnls.h")
 *  reserved     [I] Reserved, set to 0
 *  pGeoEnumProc [I] Callback function to call for each location
 *
 * RETURNS
 *  Success: TRUE.
 *  Failure: FALSE. Use GetLastError() to determine the cause.
 */
BOOL WINAPI EnumSystemGeoID(GEOCLASS geoclass, GEOID reserved, GEO_ENUMPROC pGeoEnumProc)
{
    static const WCHAR szCountryCodeValueName[] = {
      'C','o','u','n','t','r','y','C','o','d','e','\0'
    };
    WCHAR szNumber[10];
    HANDLE hKey;
    ULONG ulIndex = 0;

    TRACE("(0x%08X,0x%08X,%p)\n", geoclass, reserved, pGeoEnumProc);

    if (geoclass != GEOCLASS_NATION || reserved || !pGeoEnumProc)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    hKey = NLS_RegOpenKey( 0, szCountryListName );

    while (NLS_RegEnumSubKey( hKey, ulIndex, szNumber, sizeof(szNumber) ))
    {
        BOOL bContinue = TRUE;
        DWORD dwGeoId;
        HANDLE hSubKey = NLS_RegOpenKey( hKey, szNumber );

        if (hSubKey)
        {
            if (NLS_RegGetDword( hSubKey, szCountryCodeValueName, &dwGeoId ))
            {
                TRACE("Got geoid %d\n", dwGeoId);

                if (!pGeoEnumProc( dwGeoId ))
                    bContinue = FALSE;
            }

            NtClose( hSubKey );
        }

        if (!bContinue)
            break;

        ulIndex++;
    }

    if (hKey)
        NtClose( hKey );

    return TRUE;
}

/******************************************************************************
 *           InvalidateNLSCache           (KERNEL32.@)
 *
 * Invalidate the cache of NLS values.
 *
 * PARAMS
 *  None.
 *
 * RETURNS
 *  Success: TRUE.
 *  Failure: FALSE.
 */
BOOL WINAPI InvalidateNLSCache(void)
{
  FIXME("() stub\n");
  return FALSE;
}

/******************************************************************************
 *           GetUserGeoID (KERNEL32.@)
 */
GEOID WINAPI GetUserGeoID( GEOCLASS GeoClass )
{
    GEOID ret = GEOID_NOT_AVAILABLE;
    static const WCHAR geoW[] = {'G','e','o',0};
    static const WCHAR nationW[] = {'N','a','t','i','o','n',0};
    WCHAR bufferW[40], *end;
    DWORD count;
    HANDLE hkey, hSubkey = 0;
    UNICODE_STRING keyW;
    const KEY_VALUE_PARTIAL_INFORMATION *info = (KEY_VALUE_PARTIAL_INFORMATION *)bufferW;
    RtlInitUnicodeString( &keyW, nationW );
    count = sizeof(bufferW);

    if(!(hkey = create_registry_key())) return ret;

    switch( GeoClass ){
    case GEOCLASS_NATION:
        if ((hSubkey = NLS_RegOpenKey(hkey, geoW)))
        {
            if((NtQueryValueKey(hSubkey, &keyW, KeyValuePartialInformation,
                                bufferW, count, &count) == STATUS_SUCCESS ) && info->DataLength)
                ret = strtolW((LPCWSTR)info->Data, &end, 10);
        }
        break;
    case GEOCLASS_REGION:
        FIXME("GEOCLASS_REGION not handled yet\n");
        break;
    }

    NtClose(hkey);
    if (hSubkey) NtClose(hSubkey);
    return ret;
}

/******************************************************************************
 *           SetUserGeoID (KERNEL32.@)
 */
BOOL WINAPI SetUserGeoID( GEOID GeoID )
{
    static const WCHAR geoW[] = {'G','e','o',0};
    static const WCHAR nationW[] = {'N','a','t','i','o','n',0};
    static const WCHAR formatW[] = {'%','i',0};
    UNICODE_STRING nameW,keyW;
    WCHAR bufferW[10];
    OBJECT_ATTRIBUTES attr;
    HANDLE hkey;

    if(!(hkey = create_registry_key())) return FALSE;

    attr.Length = sizeof(attr);
    attr.RootDirectory = hkey;
    attr.ObjectName = &nameW;
    attr.Attributes = 0;
    attr.SecurityDescriptor = NULL;
    attr.SecurityQualityOfService = NULL;
    RtlInitUnicodeString( &nameW, geoW );
    RtlInitUnicodeString( &keyW, nationW );

    if (NtCreateKey( &hkey, KEY_ALL_ACCESS, &attr, 0, NULL, 0, NULL ) != STATUS_SUCCESS)

    {
        NtClose(attr.RootDirectory);
        return FALSE;
    }

    sprintfW(bufferW, formatW, GeoID);
    NtSetValueKey(hkey, &keyW, 0, REG_SZ, bufferW, (strlenW(bufferW) + 1) * sizeof(WCHAR));
    NtClose(attr.RootDirectory);
    NtClose(hkey);
    return TRUE;
}

typedef struct
{
    union
    {
        UILANGUAGE_ENUMPROCA procA;
        UILANGUAGE_ENUMPROCW procW;
    } u;
    DWORD flags;
    LONG_PTR param;
} ENUM_UILANG_CALLBACK;

static BOOL CALLBACK enum_uilang_proc_a( HMODULE hModule, LPCSTR type,
                                         LPCSTR name, WORD LangID, LONG_PTR lParam )
{
    ENUM_UILANG_CALLBACK *enum_uilang = (ENUM_UILANG_CALLBACK *)lParam;
    char buf[20];

    sprintf(buf, "%08x", (UINT)LangID);
    return enum_uilang->u.procA( buf, enum_uilang->param );
}

static BOOL CALLBACK enum_uilang_proc_w( HMODULE hModule, LPCWSTR type,
                                         LPCWSTR name, WORD LangID, LONG_PTR lParam )
{
    static const WCHAR formatW[] = {'%','0','8','x',0};
    ENUM_UILANG_CALLBACK *enum_uilang = (ENUM_UILANG_CALLBACK *)lParam;
    WCHAR buf[20];

    sprintfW( buf, formatW, (UINT)LangID );
    return enum_uilang->u.procW( buf, enum_uilang->param );
}

/******************************************************************************
 *           EnumUILanguagesA (KERNEL32.@)
 */
BOOL WINAPI EnumUILanguagesA(UILANGUAGE_ENUMPROCA pUILangEnumProc, DWORD dwFlags, LONG_PTR lParam)
{
    ENUM_UILANG_CALLBACK enum_uilang;

    TRACE("%p, %x, %lx\n", pUILangEnumProc, dwFlags, lParam);

    if(!pUILangEnumProc) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
    if(dwFlags) {
	SetLastError(ERROR_INVALID_FLAGS);
	return FALSE;
    }

    enum_uilang.u.procA = pUILangEnumProc;
    enum_uilang.flags = dwFlags;
    enum_uilang.param = lParam;

    EnumResourceLanguagesA( kernel32_handle, (LPCSTR)RT_STRING,
                            (LPCSTR)LOCALE_ILANGUAGE, enum_uilang_proc_a,
                            (LONG_PTR)&enum_uilang);
    return TRUE;
}

/******************************************************************************
 *           EnumUILanguagesW (KERNEL32.@)
 */
BOOL WINAPI EnumUILanguagesW(UILANGUAGE_ENUMPROCW pUILangEnumProc, DWORD dwFlags, LONG_PTR lParam)
{
    ENUM_UILANG_CALLBACK enum_uilang;

    TRACE("%p, %x, %lx\n", pUILangEnumProc, dwFlags, lParam);


    if(!pUILangEnumProc) {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }
    if(dwFlags) {
	SetLastError(ERROR_INVALID_FLAGS);
	return FALSE;
    }

    enum_uilang.u.procW = pUILangEnumProc;
    enum_uilang.flags = dwFlags;
    enum_uilang.param = lParam;

    EnumResourceLanguagesW( kernel32_handle, (LPCWSTR)RT_STRING,
                            (LPCWSTR)LOCALE_ILANGUAGE, enum_uilang_proc_w,
                            (LONG_PTR)&enum_uilang);
    return TRUE;
}

INT WINAPI GetGeoInfoW(GEOID GeoId, GEOTYPE GeoType, LPWSTR lpGeoData, 
                int cchData, LANGID language)
{
    FIXME("%d %d %p %d %d\n", GeoId, GeoType, lpGeoData, cchData, language);
    return 0;
}

INT WINAPI GetGeoInfoA(GEOID GeoId, GEOTYPE GeoType, LPSTR lpGeoData, 
                int cchData, LANGID language)
{
    FIXME("%d %d %p %d %d\n", GeoId, GeoType, lpGeoData, cchData, language);
    return 0;
}
