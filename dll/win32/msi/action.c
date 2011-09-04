/*
 * Implementation of the Microsoft Installer (msi.dll)
 *
 * Copyright 2004,2005 Aric Stewart for CodeWeavers
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

#include <stdarg.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "winreg.h"
#include "winsvc.h"
#include "odbcinst.h"
#include "wine/debug.h"
#include "msidefs.h"
#include "msipriv.h"
#include "winuser.h"
#include "shlobj.h"
#include "objbase.h"
#include "mscoree.h"
#include "shlwapi.h"
#include "wine/unicode.h"
#include "winver.h"

#define REG_PROGRESS_VALUE 13200
#define COMPONENT_PROGRESS_VALUE 24000

WINE_DEFAULT_DEBUG_CHANNEL(msi);

/*
 * consts and values used
 */
static const WCHAR c_colon[] = {'C',':','\\',0};

static const WCHAR szCreateFolders[] =
    {'C','r','e','a','t','e','F','o','l','d','e','r','s',0};
static const WCHAR szCostFinalize[] =
    {'C','o','s','t','F','i','n','a','l','i','z','e',0};
static const WCHAR szWriteRegistryValues[] =
    {'W','r','i','t','e','R','e','g','i','s','t','r','y','V','a','l','u','e','s',0};
static const WCHAR szCostInitialize[] =
    {'C','o','s','t','I','n','i','t','i','a','l','i','z','e',0};
static const WCHAR szFileCost[] = 
    {'F','i','l','e','C','o','s','t',0};
static const WCHAR szInstallInitialize[] = 
    {'I','n','s','t','a','l','l','I','n','i','t','i','a','l','i','z','e',0};
static const WCHAR szInstallValidate[] = 
    {'I','n','s','t','a','l','l','V','a','l','i','d','a','t','e',0};
static const WCHAR szLaunchConditions[] = 
    {'L','a','u','n','c','h','C','o','n','d','i','t','i','o','n','s',0};
static const WCHAR szProcessComponents[] = 
    {'P','r','o','c','e','s','s','C','o','m','p','o','n','e','n','t','s',0};
static const WCHAR szRegisterTypeLibraries[] = 
    {'R','e','g','i','s','t','e','r','T','y','p','e','L','i','b','r','a','r','i','e','s',0};
static const WCHAR szCreateShortcuts[] = 
    {'C','r','e','a','t','e','S','h','o','r','t','c','u','t','s',0};
static const WCHAR szPublishProduct[] = 
    {'P','u','b','l','i','s','h','P','r','o','d','u','c','t',0};
static const WCHAR szWriteIniValues[] = 
    {'W','r','i','t','e','I','n','i','V','a','l','u','e','s',0};
static const WCHAR szSelfRegModules[] = 
    {'S','e','l','f','R','e','g','M','o','d','u','l','e','s',0};
static const WCHAR szPublishFeatures[] = 
    {'P','u','b','l','i','s','h','F','e','a','t','u','r','e','s',0};
static const WCHAR szRegisterProduct[] = 
    {'R','e','g','i','s','t','e','r','P','r','o','d','u','c','t',0};
static const WCHAR szInstallExecute[] = 
    {'I','n','s','t','a','l','l','E','x','e','c','u','t','e',0};
static const WCHAR szInstallExecuteAgain[] = 
    {'I','n','s','t','a','l','l','E','x','e','c','u','t','e','A','g','a','i','n',0};
static const WCHAR szInstallFinalize[] = 
    {'I','n','s','t','a','l','l','F','i','n','a','l','i','z','e',0};
static const WCHAR szForceReboot[] = 
    {'F','o','r','c','e','R','e','b','o','o','t',0};
static const WCHAR szResolveSource[] =
    {'R','e','s','o','l','v','e','S','o','u','r','c','e',0};
static const WCHAR szAllocateRegistrySpace[] = 
    {'A','l','l','o','c','a','t','e','R','e','g','i','s','t','r','y','S','p','a','c','e',0};
static const WCHAR szBindImage[] = 
    {'B','i','n','d','I','m','a','g','e',0};
static const WCHAR szDeleteServices[] = 
    {'D','e','l','e','t','e','S','e','r','v','i','c','e','s',0};
static const WCHAR szDisableRollback[] = 
    {'D','i','s','a','b','l','e','R','o','l','l','b','a','c','k',0};
static const WCHAR szExecuteAction[] = 
    {'E','x','e','c','u','t','e','A','c','t','i','o','n',0};
static const WCHAR szInstallAdminPackage[] = 
    {'I','n','s','t','a','l','l','A','d','m','i','n','P','a','c','k','a','g','e',0};
static const WCHAR szInstallSFPCatalogFile[] = 
    {'I','n','s','t','a','l','l','S','F','P','C','a','t','a','l','o','g','F','i','l','e',0};
static const WCHAR szIsolateComponents[] = 
    {'I','s','o','l','a','t','e','C','o','m','p','o','n','e','n','t','s',0};
static const WCHAR szMigrateFeatureStates[] =
    {'M','i','g','r','a','t','e','F','e','a','t','u','r','e','S','t','a','t','e','s',0};
static const WCHAR szMsiUnpublishAssemblies[] = 
    {'M','s','i','U','n','p','u','b','l','i','s','h','A','s','s','e','m','b','l','i','e','s',0};
static const WCHAR szInstallODBC[] = 
    {'I','n','s','t','a','l','l','O','D','B','C',0};
static const WCHAR szInstallServices[] = 
    {'I','n','s','t','a','l','l','S','e','r','v','i','c','e','s',0};
static const WCHAR szPatchFiles[] =
    {'P','a','t','c','h','F','i','l','e','s',0};
static const WCHAR szPublishComponents[] = 
    {'P','u','b','l','i','s','h','C','o','m','p','o','n','e','n','t','s',0};
static const WCHAR szRegisterComPlus[] =
    {'R','e','g','i','s','t','e','r','C','o','m','P','l','u','s',0};
static const WCHAR szRegisterUser[] =
    {'R','e','g','i','s','t','e','r','U','s','e','r',0};
static const WCHAR szRemoveEnvironmentStrings[] =
    {'R','e','m','o','v','e','E','n','v','i','r','o','n','m','e','n','t','S','t','r','i','n','g','s',0};
static const WCHAR szRemoveExistingProducts[] =
    {'R','e','m','o','v','e','E','x','i','s','t','i','n','g','P','r','o','d','u','c','t','s',0};
static const WCHAR szRemoveFolders[] =
    {'R','e','m','o','v','e','F','o','l','d','e','r','s',0};
static const WCHAR szRemoveIniValues[] =
    {'R','e','m','o','v','e','I','n','i','V','a','l','u','e','s',0};
static const WCHAR szRemoveODBC[] =
    {'R','e','m','o','v','e','O','D','B','C',0};
static const WCHAR szRemoveRegistryValues[] =
    {'R','e','m','o','v','e','R','e','g','i','s','t','r','y','V','a','l','u','e','s',0};
static const WCHAR szRemoveShortcuts[] =
    {'R','e','m','o','v','e','S','h','o','r','t','c','u','t','s',0};
static const WCHAR szRMCCPSearch[] =
    {'R','M','C','C','P','S','e','a','r','c','h',0};
static const WCHAR szScheduleReboot[] =
    {'S','c','h','e','d','u','l','e','R','e','b','o','o','t',0};
static const WCHAR szSelfUnregModules[] =
    {'S','e','l','f','U','n','r','e','g','M','o','d','u','l','e','s',0};
static const WCHAR szSetODBCFolders[] =
    {'S','e','t','O','D','B','C','F','o','l','d','e','r','s',0};
static const WCHAR szStartServices[] =
    {'S','t','a','r','t','S','e','r','v','i','c','e','s',0};
static const WCHAR szStopServices[] =
    {'S','t','o','p','S','e','r','v','i','c','e','s',0};
static const WCHAR szUnpublishComponents[] =
    {'U','n','p','u','b','l','i','s','h', 'C','o','m','p','o','n','e','n','t','s',0};
static const WCHAR szUnpublishFeatures[] =
    {'U','n','p','u','b','l','i','s','h','F','e','a','t','u','r','e','s',0};
static const WCHAR szUnregisterComPlus[] =
    {'U','n','r','e','g','i','s','t','e','r','C','o','m','P','l','u','s',0};
static const WCHAR szUnregisterTypeLibraries[] =
    {'U','n','r','e','g','i','s','t','e','r','T','y','p','e','L','i','b','r','a','r','i','e','s',0};
static const WCHAR szValidateProductID[] =
    {'V','a','l','i','d','a','t','e','P','r','o','d','u','c','t','I','D',0};
static const WCHAR szWriteEnvironmentStrings[] =
    {'W','r','i','t','e','E','n','v','i','r','o','n','m','e','n','t','S','t','r','i','n','g','s',0};

static void ui_actionstart(MSIPACKAGE *package, LPCWSTR action)
{
    static const WCHAR Query_t[] = 
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','A','c','t','i','o', 'n','T','e','x','t','`',' ',
         'W','H','E','R','E', ' ','`','A','c','t','i','o','n','`',' ','=', 
         ' ','\'','%','s','\'',0};
    MSIRECORD * row;

    row = MSI_QueryGetRecord( package->db, Query_t, action );
    if (!row)
        return;
    MSI_ProcessMessage(package, INSTALLMESSAGE_ACTIONSTART, row);
    msiobj_release(&row->hdr);
}

static void ui_actioninfo(MSIPACKAGE *package, LPCWSTR action, BOOL start, 
                          UINT rc)
{
    MSIRECORD * row;
    static const WCHAR template_s[]=
        {'A','c','t','i','o','n',' ','s','t','a','r','t',' ','%','s',':',' ',
         '%','s', '.',0};
    static const WCHAR template_e[]=
        {'A','c','t','i','o','n',' ','e','n','d','e','d',' ','%','s',':',' ',
         '%','s', '.',' ','R','e','t','u','r','n',' ','v','a','l','u','e',' ',
         '%','i','.',0};
    static const WCHAR format[] = 
        {'H','H','\'',':','\'','m','m','\'',':','\'','s','s',0};
    WCHAR message[1024];
    WCHAR timet[0x100];

    GetTimeFormatW(LOCALE_USER_DEFAULT, 0, NULL, format, timet, 0x100);
    if (start)
        sprintfW(message,template_s,timet,action);
    else
        sprintfW(message,template_e,timet,action,rc);
    
    row = MSI_CreateRecord(1);
    MSI_RecordSetStringW(row,1,message);
 
    MSI_ProcessMessage(package, INSTALLMESSAGE_INFO, row);
    msiobj_release(&row->hdr);
}

enum parse_state
{
    state_whitespace,
    state_token,
    state_quote
};

static int parse_prop( const WCHAR *str, WCHAR *value, int *quotes )
{
    enum parse_state state = state_quote;
    const WCHAR *p;
    WCHAR *out = value;
    int ignore, in_quotes = 0, count = 0, len = 0;

    for (p = str; *p; p++)
    {
        ignore = 0;
        switch (state)
        {
        case state_whitespace:
            switch (*p)
            {
            case ' ':
                if (!count) goto done;
                in_quotes = 1;
                ignore = 1;
                break;
            case '"':
                state = state_quote;
                if (in_quotes) count--;
                else count++;
                break;
            default:
                state = state_token;
                if (!count) in_quotes = 0;
                else in_quotes = 1;
                len++;
                break;
            }
            break;

        case state_token:
            switch (*p)
            {
            case '"':
                state = state_quote;
                if (in_quotes) count--;
                else count++;
                break;
            case ' ':
                state = state_whitespace;
                if (!count) goto done;
                in_quotes = 1;
                break;
            default:
                if (!count) in_quotes = 0;
                else in_quotes = 1;
                len++;
                break;
            }
            break;

        case state_quote:
            switch (*p)
            {
            case '"':
                if (in_quotes) count--;
                else count++;
                break;
            case ' ':
                state = state_whitespace;
                if (!count || !len) goto done;
                in_quotes = 1;
                break;
            default:
                state = state_token;
                if (!count) in_quotes = 0;
                else in_quotes = 1;
                len++;
                break;
            }
            break;

        default: break;
        }
        if (!ignore) *out++ = *p;
    }

done:
    if (!len) *value = 0;
    else *out = 0;

    *quotes = count;
    return p - str;
}

static void remove_quotes( WCHAR *str )
{
    WCHAR *p = str;
    int len = strlenW( str );

    while ((p = strchrW( p, '"' )))
    {
        memmove( p, p + 1, (len - (p - str)) * sizeof(WCHAR) );
        p++;
    }
}

UINT msi_parse_command_line( MSIPACKAGE *package, LPCWSTR szCommandLine,
                             BOOL preserve_case )
{
    LPCWSTR ptr, ptr2;
    int quotes;
    DWORD len;
    WCHAR *prop, *val;
    UINT r;

    if (!szCommandLine)
        return ERROR_SUCCESS;

    ptr = szCommandLine;
    while (*ptr)
    {
        while (*ptr == ' ') ptr++;
        if (!*ptr) break;

        ptr2 = strchrW( ptr, '=' );
        if (!ptr2) return ERROR_INVALID_COMMAND_LINE;
 
        len = ptr2 - ptr;
        if (!len) return ERROR_INVALID_COMMAND_LINE;

        prop = msi_alloc( (len + 1) * sizeof(WCHAR) );
        memcpy( prop, ptr, len * sizeof(WCHAR) );
        prop[len] = 0;
        if (!preserve_case) struprW( prop );

        ptr2++;
        while (*ptr2 == ' ') ptr2++;

        quotes = 0;
        val = msi_alloc( (strlenW( ptr2 ) + 1) * sizeof(WCHAR) );
        len = parse_prop( ptr2, val, &quotes );
        if (quotes % 2)
        {
            WARN("unbalanced quotes\n");
            msi_free( val );
            msi_free( prop );
            return ERROR_INVALID_COMMAND_LINE;
        }
        remove_quotes( val );
        TRACE("Found commandline property %s = %s\n", debugstr_w(prop), debugstr_w(val));

        r = msi_set_property( package->db, prop, val );
        if (r == ERROR_SUCCESS && !strcmpW( prop, cszSourceDir ))
            msi_reset_folders( package, TRUE );

        msi_free( val );
        msi_free( prop );

        ptr = ptr2 + len;
    }

    return ERROR_SUCCESS;
}

WCHAR **msi_split_string( const WCHAR *str, WCHAR sep )
{
    LPCWSTR pc;
    LPWSTR p, *ret = NULL;
    UINT count = 0;

    if (!str)
        return ret;

    /* count the number of substrings */
    for ( pc = str, count = 0; pc; count++ )
    {
        pc = strchrW( pc, sep );
        if (pc)
            pc++;
    }

    /* allocate space for an array of substring pointers and the substrings */
    ret = msi_alloc( (count+1) * sizeof (LPWSTR) +
                     (lstrlenW(str)+1) * sizeof(WCHAR) );
    if (!ret)
        return ret;

    /* copy the string and set the pointers */
    p = (LPWSTR) &ret[count+1];
    lstrcpyW( p, str );
    for( count = 0; (ret[count] = p); count++ )
    {
        p = strchrW( p, sep );
        if (p)
            *p++ = 0;
    }

    return ret;
}

static UINT msi_check_transform_applicable( MSIPACKAGE *package, IStorage *patch )
{
    static const WCHAR szSystemLanguageID[] =
        { 'S','y','s','t','e','m','L','a','n','g','u','a','g','e','I','D',0 };

    LPWSTR prod_code, patch_product, langid = NULL, template = NULL;
    UINT ret = ERROR_FUNCTION_FAILED;

    prod_code = msi_dup_property( package->db, szProductCode );
    patch_product = msi_get_suminfo_product( patch );

    TRACE("db = %s patch = %s\n", debugstr_w(prod_code), debugstr_w(patch_product));

    if ( strstrW( patch_product, prod_code ) )
    {
        MSISUMMARYINFO *si;
        const WCHAR *p;

        si = MSI_GetSummaryInformationW( patch, 0 );
        if (!si)
        {
            ERR("no summary information!\n");
            goto end;
        }

        template = msi_suminfo_dup_string( si, PID_TEMPLATE );
        if (!template)
        {
            ERR("no template property!\n");
            msiobj_release( &si->hdr );
            goto end;
        }

        if (!template[0])
        {
            ret = ERROR_SUCCESS;
            msiobj_release( &si->hdr );
            goto end;
        }

        langid = msi_dup_property( package->db, szSystemLanguageID );
        if (!langid)
        {
            msiobj_release( &si->hdr );
            goto end;
        }

        p = strchrW( template, ';' );
        if (p && (!strcmpW( p + 1, langid ) || !strcmpW( p + 1, szZero )))
        {
            TRACE("applicable transform\n");
            ret = ERROR_SUCCESS;
        }

        /* FIXME: check platform */

        msiobj_release( &si->hdr );
    }

end:
    msi_free( patch_product );
    msi_free( prod_code );
    msi_free( template );
    msi_free( langid );

    return ret;
}

static UINT msi_apply_substorage_transform( MSIPACKAGE *package,
                                 MSIDATABASE *patch_db, LPCWSTR name )
{
    UINT ret = ERROR_FUNCTION_FAILED;
    IStorage *stg = NULL;
    HRESULT r;

    TRACE("%p %s\n", package, debugstr_w(name) );

    if (*name++ != ':')
    {
        ERR("expected a colon in %s\n", debugstr_w(name));
        return ERROR_FUNCTION_FAILED;
    }

    r = IStorage_OpenStorage( patch_db->storage, name, NULL, STGM_SHARE_EXCLUSIVE, NULL, 0, &stg );
    if (SUCCEEDED(r))
    {
        ret = msi_check_transform_applicable( package, stg );
        if (ret == ERROR_SUCCESS)
            msi_table_apply_transform( package->db, stg );
        else
            TRACE("substorage transform %s wasn't applicable\n", debugstr_w(name));
        IStorage_Release( stg );
    }
    else
        ERR("failed to open substorage %s\n", debugstr_w(name));

    return ERROR_SUCCESS;
}

UINT msi_check_patch_applicable( MSIPACKAGE *package, MSISUMMARYINFO *si )
{
    LPWSTR guid_list, *guids, product_code;
    UINT i, ret = ERROR_FUNCTION_FAILED;

    product_code = msi_dup_property( package->db, szProductCode );
    if (!product_code)
    {
        /* FIXME: the property ProductCode should be written into the DB somewhere */
        ERR("no product code to check\n");
        return ERROR_SUCCESS;
    }

    guid_list = msi_suminfo_dup_string( si, PID_TEMPLATE );
    guids = msi_split_string( guid_list, ';' );
    for ( i = 0; guids[i] && ret != ERROR_SUCCESS; i++ )
    {
        if (!strcmpW( guids[i], product_code ))
            ret = ERROR_SUCCESS;
    }
    msi_free( guids );
    msi_free( guid_list );
    msi_free( product_code );

    return ret;
}

static UINT msi_set_media_source_prop(MSIPACKAGE *package)
{
    MSIQUERY *view;
    MSIRECORD *rec = NULL;
    LPWSTR patch;
    LPCWSTR prop;
    UINT r;

    static const WCHAR query[] = {'S','E','L','E','C','T',' ',
        '`','S','o','u','r','c','e','`',' ','F','R','O','M',' ',
        '`','M','e','d','i','a','`',' ','W','H','E','R','E',' ',
        '`','S','o','u','r','c','e','`',' ','I','S',' ',
        'N','O','T',' ','N','U','L','L',0};

    r = MSI_DatabaseOpenViewW(package->db, query, &view);
    if (r != ERROR_SUCCESS)
        return r;

    r = MSI_ViewExecute(view, 0);
    if (r != ERROR_SUCCESS)
        goto done;

    if (MSI_ViewFetch(view, &rec) == ERROR_SUCCESS)
    {
        prop = MSI_RecordGetString(rec, 1);
        patch = msi_dup_property(package->db, szPatch);
        msi_set_property(package->db, prop, patch);
        msi_free(patch);
    }

done:
    if (rec) msiobj_release(&rec->hdr);
    msiobj_release(&view->hdr);

    return r;
}

UINT msi_parse_patch_summary( MSISUMMARYINFO *si, MSIPATCHINFO **patch )
{
    MSIPATCHINFO *pi;
    UINT r = ERROR_SUCCESS;
    WCHAR *p;

    pi = msi_alloc_zero( sizeof(MSIPATCHINFO) );
    if (!pi)
        return ERROR_OUTOFMEMORY;

    pi->patchcode = msi_suminfo_dup_string( si, PID_REVNUMBER );
    if (!pi->patchcode)
    {
        msi_free( pi );
        return ERROR_OUTOFMEMORY;
    }

    p = pi->patchcode;
    if (*p != '{')
    {
        msi_free( pi->patchcode );
        msi_free( pi );
        return ERROR_PATCH_PACKAGE_INVALID;
    }

    p = strchrW( p + 1, '}' );
    if (!p)
    {
        msi_free( pi->patchcode );
        msi_free( pi );
        return ERROR_PATCH_PACKAGE_INVALID;
    }

    if (p[1])
    {
        FIXME("patch obsoletes %s\n", debugstr_w(p + 1));
        p[1] = 0;
    }

    TRACE("patch code %s\n", debugstr_w(pi->patchcode));

    pi->transforms = msi_suminfo_dup_string( si, PID_LASTAUTHOR );
    if (!pi->transforms)
    {
        msi_free( pi->patchcode );
        msi_free( pi );
        return ERROR_OUTOFMEMORY;
    }

    *patch = pi;
    return r;
}

UINT msi_apply_patch_db( MSIPACKAGE *package, MSIDATABASE *patch_db, MSIPATCHINFO *patch )
{
    UINT i, r = ERROR_SUCCESS;
    WCHAR **substorage;

    /* apply substorage transforms */
    substorage = msi_split_string( patch->transforms, ';' );
    for (i = 0; substorage && substorage[i] && r == ERROR_SUCCESS; i++)
        r = msi_apply_substorage_transform( package, patch_db, substorage[i] );

    msi_free( substorage );
    if (r != ERROR_SUCCESS)
        return r;

    msi_set_media_source_prop( package );

    /*
     * There might be a CAB file in the patch package,
     * so append it to the list of storages to search for streams.
     */
    append_storage_to_db( package->db, patch_db->storage );

    patch->state = MSIPATCHSTATE_APPLIED;
    list_add_tail( &package->patches, &patch->entry );
    return ERROR_SUCCESS;
}

static UINT msi_apply_patch_package( MSIPACKAGE *package, LPCWSTR file )
{
    static const WCHAR dotmsp[] = {'.','m','s','p',0};
    MSIDATABASE *patch_db = NULL;
    WCHAR localfile[MAX_PATH];
    MSISUMMARYINFO *si;
    MSIPATCHINFO *patch = NULL;
    UINT r = ERROR_SUCCESS;

    TRACE("%p %s\n", package, debugstr_w( file ) );

    r = MSI_OpenDatabaseW( file, MSIDBOPEN_READONLY + MSIDBOPEN_PATCHFILE, &patch_db );
    if ( r != ERROR_SUCCESS )
    {
        ERR("failed to open patch collection %s\n", debugstr_w( file ) );
        return r;
    }

    si = MSI_GetSummaryInformationW( patch_db->storage, 0 );
    if (!si)
    {
        msiobj_release( &patch_db->hdr );
        return ERROR_FUNCTION_FAILED;
    }

    r = msi_check_patch_applicable( package, si );
    if (r != ERROR_SUCCESS)
    {
        TRACE("patch not applicable\n");
        r = ERROR_SUCCESS;
        goto done;
    }

    r = msi_parse_patch_summary( si, &patch );
    if ( r != ERROR_SUCCESS )
        goto done;

    r = msi_get_local_package_name( localfile, dotmsp );
    if ( r != ERROR_SUCCESS )
        goto done;

    TRACE("copying to local package %s\n", debugstr_w(localfile));

    if (!CopyFileW( file, localfile, FALSE ))
    {
        ERR("Unable to copy package (%s -> %s) (error %u)\n",
            debugstr_w(file), debugstr_w(localfile), GetLastError());
        r = GetLastError();
        goto done;
    }
    patch->localfile = strdupW( localfile );

    r = msi_apply_patch_db( package, patch_db, patch );
    if ( r != ERROR_SUCCESS )
        WARN("patch failed to apply %u\n", r);

done:
    msiobj_release( &si->hdr );
    msiobj_release( &patch_db->hdr );
    if (patch && r != ERROR_SUCCESS)
    {
        if (patch->localfile)
            DeleteFileW( patch->localfile );

        msi_free( patch->patchcode );
        msi_free( patch->transforms );
        msi_free( patch->localfile );
        msi_free( patch );
    }
    return r;
}

/* get the PATCH property, and apply all the patches it specifies */
static UINT msi_apply_patches( MSIPACKAGE *package )
{
    LPWSTR patch_list, *patches;
    UINT i, r = ERROR_SUCCESS;

    patch_list = msi_dup_property( package->db, szPatch );

    TRACE("patches to be applied: %s\n", debugstr_w( patch_list ) );

    patches = msi_split_string( patch_list, ';' );
    for( i=0; patches && patches[i] && r == ERROR_SUCCESS; i++ )
        r = msi_apply_patch_package( package, patches[i] );

    msi_free( patches );
    msi_free( patch_list );

    return r;
}

static UINT msi_apply_transforms( MSIPACKAGE *package )
{
    static const WCHAR szTransforms[] = {
        'T','R','A','N','S','F','O','R','M','S',0 };
    LPWSTR xform_list, *xforms;
    UINT i, r = ERROR_SUCCESS;

    xform_list = msi_dup_property( package->db, szTransforms );
    xforms = msi_split_string( xform_list, ';' );

    for( i=0; xforms && xforms[i] && r == ERROR_SUCCESS; i++ )
    {
        if (xforms[i][0] == ':')
            r = msi_apply_substorage_transform( package, package->db, xforms[i] );
        else
        {
            WCHAR *transform;

            if (!PathIsRelativeW( xforms[i] )) transform = xforms[i];
            else
            {
                WCHAR *p = strrchrW( package->PackagePath, '\\' );
                DWORD len = p - package->PackagePath + 1;

                if (!(transform = msi_alloc( (len + strlenW( xforms[i] ) + 1) * sizeof(WCHAR)) ))
                {
                    msi_free( xforms );
                    msi_free( xform_list );
                    return ERROR_OUTOFMEMORY;
                }
                memcpy( transform, package->PackagePath, len * sizeof(WCHAR) );
                memcpy( transform + len, xforms[i], (strlenW( xforms[i] ) + 1) * sizeof(WCHAR) );
            }
            r = MSI_DatabaseApplyTransformW( package->db, transform, 0 );
            if (transform != xforms[i]) msi_free( transform );
        }
    }

    msi_free( xforms );
    msi_free( xform_list );

    return r;
}

static BOOL ui_sequence_exists( MSIPACKAGE *package )
{
    MSIQUERY *view;
    UINT rc;

    static const WCHAR ExecSeqQuery [] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','I','n','s','t','a','l','l',
         'U','I','S','e','q','u','e','n','c','e','`',
         ' ','W','H','E','R','E',' ',
         '`','S','e','q','u','e','n','c','e','`',' ',
         '>',' ','0',' ','O','R','D','E','R',' ','B','Y',' ',
         '`','S','e','q','u','e','n','c','e','`',0};

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc == ERROR_SUCCESS)
    {
        msiobj_release(&view->hdr);
        return TRUE;
    }

    return FALSE;
}

UINT msi_set_sourcedir_props(MSIPACKAGE *package, BOOL replace)
{
    LPWSTR source, check;

    if (msi_get_property_int( package->db, szInstalled, 0 ))
    {
        HKEY hkey;

        MSIREG_OpenInstallProps( package->ProductCode, package->Context, NULL, &hkey, FALSE );
        source = msi_reg_get_val_str( hkey, INSTALLPROPERTY_INSTALLSOURCEW );
        RegCloseKey( hkey );
    }
    else
    {
        LPWSTR p, db;
        DWORD len;

        db = msi_dup_property( package->db, szOriginalDatabase );
        if (!db)
            return ERROR_OUTOFMEMORY;

        p = strrchrW( db, '\\' );
        if (!p)
        {
            p = strrchrW( db, '/' );
            if (!p)
            {
                msi_free(db);
                return ERROR_SUCCESS;
            }
        }

        len = p - db + 2;
        source = msi_alloc( len * sizeof(WCHAR) );
        lstrcpynW( source, db, len );
        msi_free( db );
    }

    check = msi_dup_property( package->db, cszSourceDir );
    if (!check || replace)
    {
        UINT r = msi_set_property( package->db, cszSourceDir, source );
        if (r == ERROR_SUCCESS)
            msi_reset_folders( package, TRUE );
    }
    msi_free( check );

    check = msi_dup_property( package->db, cszSOURCEDIR );
    if (!check || replace)
        msi_set_property( package->db, cszSOURCEDIR, source );

    msi_free( check );
    msi_free( source );

    return ERROR_SUCCESS;
}

static BOOL needs_ui_sequence(MSIPACKAGE *package)
{
    INT level = msi_get_property_int(package->db, szUILevel, 0);
    return (level & INSTALLUILEVEL_MASK) >= INSTALLUILEVEL_REDUCED;
}

UINT msi_set_context(MSIPACKAGE *package)
{
    int num;

    package->Context = MSIINSTALLCONTEXT_USERUNMANAGED;

    num = msi_get_property_int(package->db, szAllUsers, 0);
    if (num == 1 || num == 2)
        package->Context = MSIINSTALLCONTEXT_MACHINE;

    return ERROR_SUCCESS;
}

static UINT ITERATE_Actions(MSIRECORD *row, LPVOID param)
{
    UINT rc;
    LPCWSTR cond, action;
    MSIPACKAGE *package = param;

    action = MSI_RecordGetString(row,1);
    if (!action)
    {
        ERR("Error is retrieving action name\n");
        return ERROR_FUNCTION_FAILED;
    }

    /* check conditions */
    cond = MSI_RecordGetString(row,2);

    /* this is a hack to skip errors in the condition code */
    if (MSI_EvaluateConditionW(package, cond) == MSICONDITION_FALSE)
    {
        TRACE("Skipping action: %s (condition is false)\n", debugstr_w(action));
        return ERROR_SUCCESS;
    }

    if (needs_ui_sequence(package))
        rc = ACTION_PerformUIAction(package, action, -1);
    else
        rc = ACTION_PerformAction(package, action, -1);

    msi_dialog_check_messages( NULL );

    if (package->CurrentInstallState != ERROR_SUCCESS)
        rc = package->CurrentInstallState;

    if (rc == ERROR_FUNCTION_NOT_CALLED)
        rc = ERROR_SUCCESS;

    if (rc != ERROR_SUCCESS)
        ERR("Execution halted, action %s returned %i\n", debugstr_w(action), rc);

    return rc;
}

UINT MSI_Sequence( MSIPACKAGE *package, LPCWSTR szTable, INT iSequenceMode )
{
    MSIQUERY * view;
    UINT r;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','%','s','`',
         ' ','W','H','E','R','E',' ', 
         '`','S','e','q','u','e','n','c','e','`',' ',
         '>',' ','0',' ','O','R','D','E','R',' ','B','Y',' ',
         '`','S','e','q','u','e','n','c','e','`',0};

    TRACE("%p %s %i\n", package, debugstr_w(szTable), iSequenceMode );

    r = MSI_OpenQuery( package->db, &view, query, szTable );
    if (r == ERROR_SUCCESS)
    {
        r = MSI_IterateRecords( view, NULL, ITERATE_Actions, package );
        msiobj_release(&view->hdr);
    }

    return r;
}

static UINT ACTION_ProcessExecSequence(MSIPACKAGE *package, BOOL UIran)
{
    MSIQUERY * view;
    UINT rc;
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ', 'F','R','O','M',' ',
         '`','I','n','s','t','a','l','l','E','x','e','c','u','t','e',
         'S','e','q','u','e','n','c','e','`',' ', 'W','H','E','R','E',' ',
         '`','S','e','q','u','e','n','c','e','`',' ', '>',' ','%','i',' ',
         'O','R','D','E','R',' ', 'B','Y',' ',
         '`','S','e','q','u','e','n','c','e','`',0 };
    static const WCHAR IVQuery[] =
        {'S','E','L','E','C','T',' ','`','S','e','q','u','e','n','c','e','`',
         ' ', 'F','R','O','M',' ','`','I','n','s','t','a','l','l',
         'E','x','e','c','u','t','e','S','e','q','u','e','n','c','e','`',' ',
         'W','H','E','R','E',' ','`','A','c','t','i','o','n','`',' ','=',
         ' ','\'', 'I','n','s','t','a','l','l',
         'V','a','l','i','d','a','t','e','\'', 0};
    INT seq = 0;

    if (package->script->ExecuteSequenceRun)
    {
        TRACE("Execute Sequence already Run\n");
        return ERROR_SUCCESS;
    }

    package->script->ExecuteSequenceRun = TRUE;

    /* get the sequence number */
    if (UIran)
    {
        MSIRECORD *row = MSI_QueryGetRecord(package->db, IVQuery);
        if( !row )
            return ERROR_FUNCTION_FAILED;
        seq = MSI_RecordGetInteger(row,1);
        msiobj_release(&row->hdr);
    }

    rc = MSI_OpenQuery(package->db, &view, ExecSeqQuery, seq);
    if (rc == ERROR_SUCCESS)
    {
        TRACE("Running the actions\n");

        msi_set_property(package->db, cszSourceDir, NULL);

        rc = MSI_IterateRecords(view, NULL, ITERATE_Actions, package);
        msiobj_release(&view->hdr);
    }

    return rc;
}

static UINT ACTION_ProcessUISequence(MSIPACKAGE *package)
{
    MSIQUERY * view;
    UINT rc;
    static const WCHAR ExecSeqQuery [] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','I','n','s','t','a','l','l',
         'U','I','S','e','q','u','e','n','c','e','`',
         ' ','W','H','E','R','E',' ', 
         '`','S','e','q','u','e','n','c','e','`',' ',
         '>',' ','0',' ','O','R','D','E','R',' ','B','Y',' ',
         '`','S','e','q','u','e','n','c','e','`',0};

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc == ERROR_SUCCESS)
    {
        TRACE("Running the actions\n"); 

        rc = MSI_IterateRecords(view, NULL, ITERATE_Actions, package);
        msiobj_release(&view->hdr);
    }

    return rc;
}

/********************************************************
 * ACTION helper functions and functions that perform the actions
 *******************************************************/
static BOOL ACTION_HandleCustomAction( MSIPACKAGE* package, LPCWSTR action,
                                       UINT* rc, UINT script, BOOL force )
{
    BOOL ret=FALSE;
    UINT arc;

    arc = ACTION_CustomAction(package, action, script, force);

    if (arc != ERROR_CALL_NOT_IMPLEMENTED)
    {
        *rc = arc;
        ret = TRUE;
    }
    return ret;
}

/*
 * Actual Action Handlers
 */

static UINT ITERATE_CreateFolders(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPCWSTR dir, component;
    LPWSTR full_path;
    MSIRECORD *uirow;
    MSIFOLDER *folder;
    MSICOMPONENT *comp;

    component = MSI_RecordGetString(row, 2);
    if (!component)
        return ERROR_SUCCESS;

    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    dir = MSI_RecordGetString(row,1);
    if (!dir)
    {
        ERR("Unable to get folder id\n");
        return ERROR_SUCCESS;
    }

    uirow = MSI_CreateRecord(1);
    MSI_RecordSetStringW(uirow, 1, dir);
    ui_actiondata(package, szCreateFolders, uirow);
    msiobj_release(&uirow->hdr);

    full_path = resolve_target_folder( package, dir, FALSE, TRUE, &folder );
    if (!full_path)
    {
        ERR("Unable to resolve folder id %s\n",debugstr_w(dir));
        return ERROR_SUCCESS;
    }

    TRACE("Folder is %s\n",debugstr_w(full_path));

    if (folder->State == 0)
        create_full_pathW(full_path);

    folder->State = 3;

    msi_free(full_path);
    return ERROR_SUCCESS;
}

static UINT ACTION_CreateFolders(MSIPACKAGE *package)
{
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','C','r','e','a','t','e','F','o','l','d','e','r','`',0};
    UINT rc;
    MSIQUERY *view;

    /* create all the empty folders specified in the CreateFolder table */
    rc = MSI_DatabaseOpenViewW(package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_CreateFolders, package);
    msiobj_release(&view->hdr);

    return rc;
}

static UINT ITERATE_RemoveFolders( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR dir, component;
    LPWSTR full_path;
    MSIRECORD *uirow;
    MSIFOLDER *folder;
    MSICOMPONENT *comp;

    component = MSI_RecordGetString(row, 2);
    if (!component)
        return ERROR_SUCCESS;

    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    dir = MSI_RecordGetString( row, 1 );
    if (!dir)
    {
        ERR("Unable to get folder id\n");
        return ERROR_SUCCESS;
    }

    full_path = resolve_target_folder( package, dir, FALSE, TRUE, &folder );
    if (!full_path)
    {
        ERR("Unable to resolve folder id %s\n", debugstr_w(dir));
        return ERROR_SUCCESS;
    }

    TRACE("folder is %s\n", debugstr_w(full_path));

    uirow = MSI_CreateRecord( 1 );
    MSI_RecordSetStringW( uirow, 1, dir );
    ui_actiondata( package, szRemoveFolders, uirow );
    msiobj_release( &uirow->hdr );

    RemoveDirectoryW( full_path );
    folder->State = 0;

    msi_free( full_path );
    return ERROR_SUCCESS;
}

static UINT ACTION_RemoveFolders( MSIPACKAGE *package )
{
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','C','r','e','a','t','e','F','o','l','d','e','r','`',0};

    MSIQUERY *view;
    UINT rc;

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveFolders, package );
    msiobj_release( &view->hdr );

    return rc;
}

static UINT load_component( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;

    comp = msi_alloc_zero( sizeof(MSICOMPONENT) );
    if (!comp)
        return ERROR_FUNCTION_FAILED;

    list_add_tail( &package->components, &comp->entry );

    /* fill in the data */
    comp->Component = msi_dup_record_field( row, 1 );

    TRACE("Loading Component %s\n", debugstr_w(comp->Component));

    comp->ComponentId = msi_dup_record_field( row, 2 );
    comp->Directory = msi_dup_record_field( row, 3 );
    comp->Attributes = MSI_RecordGetInteger(row,4);
    comp->Condition = msi_dup_record_field( row, 5 );
    comp->KeyPath = msi_dup_record_field( row, 6 );

    comp->Installed = INSTALLSTATE_UNKNOWN;
    comp->Action = INSTALLSTATE_UNKNOWN;
    comp->ActionRequest = INSTALLSTATE_UNKNOWN;

    comp->assembly = load_assembly( package, comp );
    return ERROR_SUCCESS;
}

static UINT load_all_components( MSIPACKAGE *package )
{
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R', 'O','M',' ', 
         '`','C','o','m','p','o','n','e','n','t','`',0 };
    MSIQUERY *view;
    UINT r;

    if (!list_empty(&package->components))
        return ERROR_SUCCESS;

    r = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (r != ERROR_SUCCESS)
        return r;

    r = MSI_IterateRecords(view, NULL, load_component, package);
    msiobj_release(&view->hdr);
    return r;
}

typedef struct {
    MSIPACKAGE *package;
    MSIFEATURE *feature;
} _ilfs;

static UINT add_feature_component( MSIFEATURE *feature, MSICOMPONENT *comp )
{
    ComponentList *cl;

    cl = msi_alloc( sizeof (*cl) );
    if ( !cl )
        return ERROR_NOT_ENOUGH_MEMORY;
    cl->component = comp;
    list_add_tail( &feature->Components, &cl->entry );

    return ERROR_SUCCESS;
}

static UINT add_feature_child( MSIFEATURE *parent, MSIFEATURE *child )
{
    FeatureList *fl;

    fl = msi_alloc( sizeof(*fl) );
    if ( !fl )
        return ERROR_NOT_ENOUGH_MEMORY;
    fl->feature = child;
    list_add_tail( &parent->Children, &fl->entry );

    return ERROR_SUCCESS;
}

static UINT iterate_load_featurecomponents(MSIRECORD *row, LPVOID param)
{
    _ilfs* ilfs = param;
    LPCWSTR component;
    MSICOMPONENT *comp;

    component = MSI_RecordGetString(row,1);

    /* check to see if the component is already loaded */
    comp = get_loaded_component( ilfs->package, component );
    if (!comp)
    {
        ERR("unknown component %s\n", debugstr_w(component));
        return ERROR_FUNCTION_FAILED;
    }

    add_feature_component( ilfs->feature, comp );
    comp->Enabled = TRUE;

    return ERROR_SUCCESS;
}

static MSIFEATURE *find_feature_by_name( MSIPACKAGE *package, LPCWSTR name )
{
    MSIFEATURE *feature;

    if ( !name )
        return NULL;

    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        if ( !strcmpW( feature->Feature, name ) )
            return feature;
    }

    return NULL;
}

static UINT load_feature(MSIRECORD * row, LPVOID param)
{
    MSIPACKAGE* package = param;
    MSIFEATURE* feature;
    static const WCHAR Query1[] = 
        {'S','E','L','E','C','T',' ',
         '`','C','o','m','p','o','n','e','n','t','_','`',
         ' ','F','R','O','M',' ','`','F','e','a','t','u','r','e',
         'C','o','m','p','o','n','e','n','t','s','`',' ',
         'W','H','E','R','E',' ',
         '`','F','e', 'a','t','u','r','e','_','`',' ','=','\'','%','s','\'',0};
    MSIQUERY * view;
    UINT    rc;
    _ilfs ilfs;

    /* fill in the data */

    feature = msi_alloc_zero( sizeof (MSIFEATURE) );
    if (!feature)
        return ERROR_NOT_ENOUGH_MEMORY;

    list_init( &feature->Children );
    list_init( &feature->Components );
    
    feature->Feature = msi_dup_record_field( row, 1 );

    TRACE("Loading feature %s\n",debugstr_w(feature->Feature));

    feature->Feature_Parent = msi_dup_record_field( row, 2 );
    feature->Title = msi_dup_record_field( row, 3 );
    feature->Description = msi_dup_record_field( row, 4 );

    if (!MSI_RecordIsNull(row,5))
        feature->Display = MSI_RecordGetInteger(row,5);
  
    feature->Level= MSI_RecordGetInteger(row,6);
    feature->Directory = msi_dup_record_field( row, 7 );
    feature->Attributes = MSI_RecordGetInteger(row,8);

    feature->Installed = INSTALLSTATE_UNKNOWN;
    feature->Action = INSTALLSTATE_UNKNOWN;
    feature->ActionRequest = INSTALLSTATE_UNKNOWN;

    list_add_tail( &package->features, &feature->entry );

    /* load feature components */

    rc = MSI_OpenQuery( package->db, &view, Query1, feature->Feature );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    ilfs.package = package;
    ilfs.feature = feature;

    MSI_IterateRecords(view, NULL, iterate_load_featurecomponents , &ilfs);
    msiobj_release(&view->hdr);

    return ERROR_SUCCESS;
}

static UINT find_feature_children(MSIRECORD * row, LPVOID param)
{
    MSIPACKAGE* package = param;
    MSIFEATURE *parent, *child;

    child = find_feature_by_name( package, MSI_RecordGetString( row, 1 ) );
    if (!child)
        return ERROR_FUNCTION_FAILED;

    if (!child->Feature_Parent)
        return ERROR_SUCCESS;

    parent = find_feature_by_name( package, child->Feature_Parent );
    if (!parent)
        return ERROR_FUNCTION_FAILED;

    add_feature_child( parent, child );
    return ERROR_SUCCESS;
}

static UINT load_all_features( MSIPACKAGE *package )
{
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ', 'F','R','O','M',' ',
        '`','F','e','a','t','u','r','e','`',' ','O','R','D','E','R',
        ' ','B','Y',' ','`','D','i','s','p','l','a','y','`',0};
    MSIQUERY *view;
    UINT r;

    if (!list_empty(&package->features))
        return ERROR_SUCCESS;
 
    r = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (r != ERROR_SUCCESS)
        return r;

    r = MSI_IterateRecords( view, NULL, load_feature, package );
    if (r != ERROR_SUCCESS)
        return r;

    r = MSI_IterateRecords( view, NULL, find_feature_children, package );
    msiobj_release( &view->hdr );

    return r;
}

static LPWSTR folder_split_path(LPWSTR p, WCHAR ch)
{
    if (!p)
        return p;
    p = strchrW(p, ch);
    if (!p)
        return p;
    *p = 0;
    return p+1;
}

static UINT load_file_hash(MSIPACKAGE *package, MSIFILE *file)
{
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ', 'F','R','O','M',' ',
        '`','M','s','i','F','i','l','e','H','a','s','h','`',' ',
        'W','H','E','R','E',' ','`','F','i','l','e','_','`',' ','=',' ','\'','%','s','\'',0};
    MSIQUERY *view = NULL;
    MSIRECORD *row = NULL;
    UINT r;

    TRACE("%s\n", debugstr_w(file->File));

    r = MSI_OpenQuery(package->db, &view, query, file->File);
    if (r != ERROR_SUCCESS)
        goto done;

    r = MSI_ViewExecute(view, NULL);
    if (r != ERROR_SUCCESS)
        goto done;

    r = MSI_ViewFetch(view, &row);
    if (r != ERROR_SUCCESS)
        goto done;

    file->hash.dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
    file->hash.dwData[0] = MSI_RecordGetInteger(row, 3);
    file->hash.dwData[1] = MSI_RecordGetInteger(row, 4);
    file->hash.dwData[2] = MSI_RecordGetInteger(row, 5);
    file->hash.dwData[3] = MSI_RecordGetInteger(row, 6);

done:
    if (view) msiobj_release(&view->hdr);
    if (row) msiobj_release(&row->hdr);
    return r;
}

static UINT load_file_disk_id( MSIPACKAGE *package, MSIFILE *file )
{
    MSIRECORD *row;
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','`','D','i','s','k','I','d','`',' ', 'F','R','O','M',' ',
        '`','M','e','d','i','a','`',' ','W','H','E','R','E',' ',
        '`','L','a','s','t','S','e','q','u','e','n','c','e','`',' ','>','=',' ','%','i',0};

    row = MSI_QueryGetRecord( package->db, query, file->Sequence );
    if (!row)
    {
        WARN("query failed\n");
        return ERROR_FUNCTION_FAILED;
    }

    file->disk_id = MSI_RecordGetInteger( row, 1 );
    msiobj_release( &row->hdr );
    return ERROR_SUCCESS;
}

static UINT load_file(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE* package = param;
    LPCWSTR component;
    MSIFILE *file;

    /* fill in the data */

    file = msi_alloc_zero( sizeof (MSIFILE) );
    if (!file)
        return ERROR_NOT_ENOUGH_MEMORY;
 
    file->File = msi_dup_record_field( row, 1 );

    component = MSI_RecordGetString( row, 2 );
    file->Component = get_loaded_component( package, component );

    if (!file->Component)
    {
        WARN("Component not found: %s\n", debugstr_w(component));
        msi_free(file->File);
        msi_free(file);
        return ERROR_SUCCESS;
    }

    file->FileName = msi_dup_record_field( row, 3 );
    reduce_to_longfilename( file->FileName );

    file->ShortName = msi_dup_record_field( row, 3 );
    file->LongName = strdupW( folder_split_path(file->ShortName, '|'));
    
    file->FileSize = MSI_RecordGetInteger( row, 4 );
    file->Version = msi_dup_record_field( row, 5 );
    file->Language = msi_dup_record_field( row, 6 );
    file->Attributes = MSI_RecordGetInteger( row, 7 );
    file->Sequence = MSI_RecordGetInteger( row, 8 );

    file->state = msifs_invalid;

    /* if the compressed bits are not set in the file attributes,
     * then read the information from the package word count property
     */
    if (package->WordCount & msidbSumInfoSourceTypeAdminImage)
    {
        file->IsCompressed = FALSE;
    }
    else if (file->Attributes &
             (msidbFileAttributesCompressed | msidbFileAttributesPatchAdded))
    {
        file->IsCompressed = TRUE;
    }
    else if (file->Attributes & msidbFileAttributesNoncompressed)
    {
        file->IsCompressed = FALSE;
    }
    else
    {
        file->IsCompressed = package->WordCount & msidbSumInfoSourceTypeCompressed;
    }

    load_file_hash(package, file);
    load_file_disk_id(package, file);

    TRACE("File Loaded (%s)\n",debugstr_w(file->File));  

    list_add_tail( &package->files, &file->entry );
 
    return ERROR_SUCCESS;
}

static UINT load_all_files(MSIPACKAGE *package)
{
    MSIQUERY * view;
    UINT rc;
    static const WCHAR Query[] =
        {'S','E','L','E','C','T',' ','*',' ', 'F','R','O','M',' ',
         '`','F','i','l','e','`',' ', 'O','R','D','E','R',' ','B','Y',' ',
         '`','S','e','q','u','e','n','c','e','`', 0};

    if (!list_empty(&package->files))
        return ERROR_SUCCESS;

    rc = MSI_DatabaseOpenViewW(package->db, Query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, load_file, package);
    msiobj_release(&view->hdr);

    return ERROR_SUCCESS;
}

static UINT load_folder( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    static WCHAR szEmpty[] = { 0 };
    LPWSTR p, tgt_short, tgt_long, src_short, src_long;
    MSIFOLDER *folder;

    folder = msi_alloc_zero( sizeof (MSIFOLDER) );
    if (!folder)
        return ERROR_NOT_ENOUGH_MEMORY;

    folder->Directory = msi_dup_record_field( row, 1 );

    TRACE("%s\n", debugstr_w(folder->Directory));

    p = msi_dup_record_field(row, 3);

    /* split src and target dir */
    tgt_short = p;
    src_short = folder_split_path( p, ':' );

    /* split the long and short paths */
    tgt_long = folder_split_path( tgt_short, '|' );
    src_long = folder_split_path( src_short, '|' );

    /* check for no-op dirs */
    if (tgt_short && !strcmpW( szDot, tgt_short ))
        tgt_short = szEmpty;
    if (src_short && !strcmpW( szDot, src_short ))
        src_short = szEmpty;

    if (!tgt_long)
        tgt_long = tgt_short;

    if (!src_short) {
        src_short = tgt_short;
        src_long = tgt_long;
    }

    if (!src_long)
        src_long = src_short;

    /* FIXME: use the target short path too */
    folder->TargetDefault = strdupW(tgt_long);
    folder->SourceShortPath = strdupW(src_short);
    folder->SourceLongPath = strdupW(src_long);
    msi_free(p);

    TRACE("TargetDefault = %s\n",debugstr_w( folder->TargetDefault ));
    TRACE("SourceLong = %s\n", debugstr_w( folder->SourceLongPath ));
    TRACE("SourceShort = %s\n", debugstr_w( folder->SourceShortPath ));

    folder->Parent = msi_dup_record_field( row, 2 );

    folder->Property = msi_dup_property( package->db, folder->Directory );

    list_add_tail( &package->folders, &folder->entry );

    TRACE("returning %p\n", folder);

    return ERROR_SUCCESS;
}

static UINT load_all_folders( MSIPACKAGE *package )
{
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R', 'O','M',' ',
         '`','D','i','r','e','c','t','o','r','y','`',0 };
    MSIQUERY *view;
    UINT r;

    if (!list_empty(&package->folders))
        return ERROR_SUCCESS;

    r = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (r != ERROR_SUCCESS)
        return r;

    r = MSI_IterateRecords(view, NULL, load_folder, package);
    msiobj_release(&view->hdr);
    return r;
}

/*
 * I am not doing any of the costing functionality yet.
 * Mostly looking at doing the Component and Feature loading
 *
 * The native MSI does A LOT of modification to tables here. Mostly adding
 * a lot of temporary columns to the Feature and Component tables.
 *
 *    note: Native msi also tracks the short filename. But I am only going to
 *          track the long ones.  Also looking at this directory table
 *          it appears that the directory table does not get the parents
 *          resolved base on property only based on their entries in the
 *          directory table.
 */
static UINT ACTION_CostInitialize(MSIPACKAGE *package)
{
    static const WCHAR szCosting[] =
        {'C','o','s','t','i','n','g','C','o','m','p','l','e','t','e',0 };

    msi_set_property( package->db, szCosting, szZero );
    msi_set_property( package->db, cszRootDrive, c_colon );

    load_all_folders( package );
    load_all_components( package );
    load_all_features( package );
    load_all_files( package );

    return ERROR_SUCCESS;
}

static UINT execute_script(MSIPACKAGE *package, UINT script )
{
    UINT i;
    UINT rc = ERROR_SUCCESS;

    TRACE("Executing Script %i\n",script);

    if (!package->script)
    {
        ERR("no script!\n");
        return ERROR_FUNCTION_FAILED;
    }

    for (i = 0; i < package->script->ActionCount[script]; i++)
    {
        LPWSTR action;
        action = package->script->Actions[script][i];
        ui_actionstart(package, action);
        TRACE("Executing Action (%s)\n",debugstr_w(action));
        rc = ACTION_PerformAction(package, action, script);
        if (rc != ERROR_SUCCESS)
            break;
    }
    msi_free_action_script(package, script);
    return rc;
}

static UINT ACTION_FileCost(MSIPACKAGE *package)
{
    return ERROR_SUCCESS;
}

static void ACTION_GetComponentInstallStates(MSIPACKAGE *package)
{
    MSICOMPONENT *comp;
    UINT r;

    LIST_FOR_EACH_ENTRY(comp, &package->components, MSICOMPONENT, entry)
    {
        if (!comp->ComponentId) continue;

        r = MsiQueryComponentStateW( package->ProductCode, NULL,
                                     MSIINSTALLCONTEXT_USERMANAGED, comp->ComponentId,
                                     &comp->Installed );
        if (r != ERROR_SUCCESS)
            r = MsiQueryComponentStateW( package->ProductCode, NULL,
                                         MSIINSTALLCONTEXT_USERUNMANAGED, comp->ComponentId,
                                         &comp->Installed );
        if (r != ERROR_SUCCESS)
            r = MsiQueryComponentStateW( package->ProductCode, NULL,
                                         MSIINSTALLCONTEXT_MACHINE, comp->ComponentId,
                                         &comp->Installed );
        if (r != ERROR_SUCCESS)
            comp->Installed = INSTALLSTATE_ABSENT;
    }
}

static void ACTION_GetFeatureInstallStates(MSIPACKAGE *package)
{
    MSIFEATURE *feature;

    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        INSTALLSTATE state = MsiQueryFeatureStateW( package->ProductCode, feature->Feature );

        if (state == INSTALLSTATE_UNKNOWN || state == INSTALLSTATE_INVALIDARG)
            feature->Installed = INSTALLSTATE_ABSENT;
        else
            feature->Installed = state;
    }
}

static inline BOOL is_feature_selected( MSIFEATURE *feature, INT level )
{
    return (feature->Level > 0 && feature->Level <= level);
}

static BOOL process_state_property(MSIPACKAGE* package, int level,
                                   LPCWSTR property, INSTALLSTATE state)
{
    LPWSTR override;
    MSIFEATURE *feature;

    override = msi_dup_property( package->db, property );
    if (!override)
        return FALSE;

    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        if (strcmpW( property, szRemove ) && !is_feature_selected( feature, level ))
            continue;

        if (!strcmpW(property, szReinstall)) state = feature->Installed;

        if (!strcmpiW( override, szAll ))
        {
            if (feature->Installed != state)
            {
                feature->Action = state;
                feature->ActionRequest = state;
            }
        }
        else
        {
            LPWSTR ptr = override;
            LPWSTR ptr2 = strchrW(override,',');

            while (ptr)
            {
                int len = ptr2 - ptr;

                if ((ptr2 && strlenW(feature->Feature) == len && !strncmpW(ptr, feature->Feature, len))
                    || (!ptr2 && !strcmpW(ptr, feature->Feature)))
                {
                    if (feature->Installed != state)
                    {
                        feature->Action = state;
                        feature->ActionRequest = state;
                    }
                    break;
                }
                if (ptr2)
                {
                    ptr=ptr2+1;
                    ptr2 = strchrW(ptr,',');
                }
                else
                    break;
            }
        }
    }
    msi_free(override);
    return TRUE;
}

static BOOL process_overrides( MSIPACKAGE *package, int level )
{
    static const WCHAR szAddLocal[] =
        {'A','D','D','L','O','C','A','L',0};
    static const WCHAR szAddSource[] =
        {'A','D','D','S','O','U','R','C','E',0};
    static const WCHAR szAdvertise[] =
        {'A','D','V','E','R','T','I','S','E',0};
    BOOL ret = FALSE;

    /* all these activation/deactivation things happen in order and things
     * later on the list override things earlier on the list.
     *
     *  0  INSTALLLEVEL processing
     *  1  ADDLOCAL
     *  2  REMOVE
     *  3  ADDSOURCE
     *  4  ADDDEFAULT
     *  5  REINSTALL
     *  6  ADVERTISE
     *  7  COMPADDLOCAL
     *  8  COMPADDSOURCE
     *  9  FILEADDLOCAL
     * 10  FILEADDSOURCE
     * 11  FILEADDDEFAULT
     */
    ret |= process_state_property( package, level, szAddLocal, INSTALLSTATE_LOCAL );
    ret |= process_state_property( package, level, szRemove, INSTALLSTATE_ABSENT );
    ret |= process_state_property( package, level, szAddSource, INSTALLSTATE_SOURCE );
    ret |= process_state_property( package, level, szReinstall, INSTALLSTATE_UNKNOWN );
    ret |= process_state_property( package, level, szAdvertise, INSTALLSTATE_ADVERTISED );

    if (ret)
        msi_set_property( package->db, szPreselected, szOne );

    return ret;
}

UINT MSI_SetFeatureStates(MSIPACKAGE *package)
{
    int level;
    static const WCHAR szlevel[] =
        {'I','N','S','T','A','L','L','L','E','V','E','L',0};
    MSICOMPONENT* component;
    MSIFEATURE *feature;

    TRACE("Checking Install Level\n");

    level = msi_get_property_int(package->db, szlevel, 1);

    if (!msi_get_property_int( package->db, szPreselected, 0 ))
    {
        LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
        {
            if (!is_feature_selected( feature, level )) continue;

            if (feature->ActionRequest == INSTALLSTATE_UNKNOWN)
            {
                if (feature->Attributes & msidbFeatureAttributesFavorSource)
                {
                    feature->Action = INSTALLSTATE_SOURCE;
                    feature->ActionRequest = INSTALLSTATE_SOURCE;
                }
                else if (feature->Attributes & msidbFeatureAttributesFavorAdvertise)
                {
                    feature->Action = INSTALLSTATE_ADVERTISED;
                    feature->ActionRequest = INSTALLSTATE_ADVERTISED;
                }
                else
                {
                    feature->Action = INSTALLSTATE_LOCAL;
                    feature->ActionRequest = INSTALLSTATE_LOCAL;
                }
            }
        }

        /* disable child features of unselected parent features */
        LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
        {
            FeatureList *fl;

            if (is_feature_selected( feature, level )) continue;

            LIST_FOR_EACH_ENTRY( fl, &feature->Children, FeatureList, entry )
            {
                fl->feature->Action = INSTALLSTATE_UNKNOWN;
                fl->feature->ActionRequest = INSTALLSTATE_UNKNOWN;
            }
        }
    }
    else /* preselected */
    {
        LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
        {
            if (!is_feature_selected( feature, level )) continue;

            if (feature->ActionRequest == INSTALLSTATE_UNKNOWN)
            {
                if (feature->Installed == INSTALLSTATE_ABSENT)
                {
                    feature->Action = INSTALLSTATE_UNKNOWN;
                    feature->ActionRequest = INSTALLSTATE_UNKNOWN;
                }
                else
                {
                    feature->Action = feature->Installed;
                    feature->ActionRequest = feature->Installed;
                }
            }
        }
    }

    /* now we want to set component state based based on feature state */
    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        ComponentList *cl;

        TRACE("Examining Feature %s (Level %d Installed %d Request %d Action %d)\n",
              debugstr_w(feature->Feature), feature->Level, feature->Installed,
              feature->ActionRequest, feature->Action);

        if (!is_feature_selected( feature, level )) continue;

        /* features with components that have compressed files are made local */
        LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
        {
            if (cl->component->ForceLocalState &&
                feature->ActionRequest == INSTALLSTATE_SOURCE)
            {
                feature->Action = INSTALLSTATE_LOCAL;
                feature->ActionRequest = INSTALLSTATE_LOCAL;
                break;
            }
        }

        LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
        {
            component = cl->component;

            switch (feature->ActionRequest)
            {
            case INSTALLSTATE_ABSENT:
                component->anyAbsent = 1;
                break;
            case INSTALLSTATE_ADVERTISED:
                component->hasAdvertiseFeature = 1;
                break;
            case INSTALLSTATE_SOURCE:
                component->hasSourceFeature = 1;
                break;
            case INSTALLSTATE_LOCAL:
                component->hasLocalFeature = 1;
                break;
            case INSTALLSTATE_DEFAULT:
                if (feature->Attributes & msidbFeatureAttributesFavorAdvertise)
                    component->hasAdvertiseFeature = 1;
                else if (feature->Attributes & msidbFeatureAttributesFavorSource)
                    component->hasSourceFeature = 1;
                else
                    component->hasLocalFeature = 1;
                break;
            default:
                break;
            }
        }
    }

    LIST_FOR_EACH_ENTRY( component, &package->components, MSICOMPONENT, entry )
    {
        /* check if it's local or source */
        if (!(component->Attributes & msidbComponentAttributesOptional) &&
             (component->hasLocalFeature || component->hasSourceFeature))
        {
            if ((component->Attributes & msidbComponentAttributesSourceOnly) &&
                 !component->ForceLocalState)
            {
                component->Action = INSTALLSTATE_SOURCE;
                component->ActionRequest = INSTALLSTATE_SOURCE;
            }
            else
            {
                component->Action = INSTALLSTATE_LOCAL;
                component->ActionRequest = INSTALLSTATE_LOCAL;
            }
            continue;
        }

        /* if any feature is local, the component must be local too */
        if (component->hasLocalFeature)
        {
            component->Action = INSTALLSTATE_LOCAL;
            component->ActionRequest = INSTALLSTATE_LOCAL;
            continue;
        }
        if (component->hasSourceFeature)
        {
            component->Action = INSTALLSTATE_SOURCE;
            component->ActionRequest = INSTALLSTATE_SOURCE;
            continue;
        }
        if (component->hasAdvertiseFeature)
        {
            component->Action = INSTALLSTATE_ADVERTISED;
            component->ActionRequest = INSTALLSTATE_ADVERTISED;
            continue;
        }
        TRACE("nobody wants component %s\n", debugstr_w(component->Component));
        if (component->anyAbsent &&
            (component->Installed == INSTALLSTATE_LOCAL || component->Installed == INSTALLSTATE_SOURCE))
        {
            component->Action = INSTALLSTATE_ABSENT;
            component->ActionRequest = INSTALLSTATE_ABSENT;
        }
    }

    LIST_FOR_EACH_ENTRY( component, &package->components, MSICOMPONENT, entry )
    {
        if (component->ActionRequest == INSTALLSTATE_DEFAULT)
        {
            TRACE("%s was default, setting to local\n", debugstr_w(component->Component));
            component->Action = INSTALLSTATE_LOCAL;
            component->ActionRequest = INSTALLSTATE_LOCAL;
        }

        if (component->ActionRequest == INSTALLSTATE_SOURCE &&
            component->Installed == INSTALLSTATE_SOURCE &&
            component->hasSourceFeature)
        {
            component->Action = INSTALLSTATE_UNKNOWN;
            component->ActionRequest = INSTALLSTATE_UNKNOWN;
        }

        TRACE("Result: Component %s (Installed %d Request %d Action %d)\n",
              debugstr_w(component->Component), component->Installed, component->ActionRequest, component->Action);
    }

    return ERROR_SUCCESS;
}

static UINT ITERATE_CostFinalizeDirectories(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPCWSTR name;
    LPWSTR path;
    MSIFOLDER *f;

    name = MSI_RecordGetString(row,1);

    f = get_loaded_folder(package, name);
    if (!f) return ERROR_SUCCESS;

    /* reset the ResolvedTarget */
    msi_free(f->ResolvedTarget);
    f->ResolvedTarget = NULL;

    TRACE("directory %s ...\n", debugstr_w(name));
    path = resolve_target_folder( package, name, TRUE, TRUE, NULL );
    TRACE("resolves to %s\n", debugstr_w(path));
    msi_free(path);

    return ERROR_SUCCESS;
}

static UINT ITERATE_CostFinalizeConditions(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPCWSTR name;
    MSIFEATURE *feature;

    name = MSI_RecordGetString( row, 1 );

    feature = get_loaded_feature( package, name );
    if (!feature)
        ERR("FAILED to find loaded feature %s\n",debugstr_w(name));
    else
    {
        LPCWSTR Condition;
        Condition = MSI_RecordGetString(row,3);

        if (MSI_EvaluateConditionW(package,Condition) == MSICONDITION_TRUE)
        {
            int level = MSI_RecordGetInteger(row,2);
            TRACE("Resetting feature %s to level %i\n", debugstr_w(name), level);
            feature->Level = level;
        }
    }
    return ERROR_SUCCESS;
}

VS_FIXEDFILEINFO *msi_get_disk_file_version( LPCWSTR filename )
{
    static const WCHAR name[] = {'\\',0};
    VS_FIXEDFILEINFO *ptr, *ret;
    LPVOID version;
    DWORD versize, handle;
    UINT sz;

    TRACE("%s\n", debugstr_w(filename));

    versize = GetFileVersionInfoSizeW( filename, &handle );
    if (!versize)
        return NULL;

    version = msi_alloc( versize );
    if (!version)
        return NULL;

    GetFileVersionInfoW( filename, 0, versize, version );

    if (!VerQueryValueW( version, name, (LPVOID *)&ptr, &sz ))
    {
        msi_free( version );
        return NULL;
    }

    ret = msi_alloc( sz );
    memcpy( ret, ptr, sz );

    msi_free( version );
    return ret;
}

int msi_compare_file_versions( VS_FIXEDFILEINFO *fi, const WCHAR *version )
{
    DWORD ms, ls;

    msi_parse_version_string( version, &ms, &ls );

    if (fi->dwFileVersionMS > ms) return 1;
    else if (fi->dwFileVersionMS < ms) return -1;
    else if (fi->dwFileVersionLS > ls) return 1;
    else if (fi->dwFileVersionLS < ls) return -1;
    return 0;
}

int msi_compare_font_versions( const WCHAR *ver1, const WCHAR *ver2 )
{
    DWORD ms1, ms2;

    msi_parse_version_string( ver1, &ms1, NULL );
    msi_parse_version_string( ver2, &ms2, NULL );

    if (ms1 > ms2) return 1;
    else if (ms1 < ms2) return -1;
    return 0;
}

DWORD msi_get_disk_file_size( LPCWSTR filename )
{
    HANDLE file;
    DWORD size;

    TRACE("%s\n", debugstr_w(filename));

    file = CreateFileW( filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
    if (file == INVALID_HANDLE_VALUE)
        return INVALID_FILE_SIZE;

    size = GetFileSize( file, NULL );
    CloseHandle( file );
    return size;
}

BOOL msi_file_hash_matches( MSIFILE *file )
{
    UINT r;
    MSIFILEHASHINFO hash;

    hash.dwFileHashInfoSize = sizeof(MSIFILEHASHINFO);
    r = MsiGetFileHashW( file->TargetPath, 0, &hash );
    if (r != ERROR_SUCCESS)
        return FALSE;

    return !memcmp( &hash, &file->hash, sizeof(MSIFILEHASHINFO) );
}

static WCHAR *get_temp_dir( void )
{
    static UINT id;
    WCHAR tmp[MAX_PATH], dir[MAX_PATH];

    GetTempPathW( MAX_PATH, tmp );
    for (;;)
    {
        if (!GetTempFileNameW( tmp, szMsi, ++id, dir )) return NULL;
        if (CreateDirectoryW( dir, NULL )) break;
    }
    return strdupW( dir );
}

static void set_target_path( MSIPACKAGE *package, MSIFILE *file )
{
    MSIASSEMBLY *assembly = file->Component->assembly;

    TRACE("file %s is named %s\n", debugstr_w(file->File), debugstr_w(file->FileName));

    msi_free( file->TargetPath );
    if (assembly && !assembly->application)
    {
        if (!assembly->tempdir) assembly->tempdir = get_temp_dir();
        file->TargetPath = build_directory_name( 2, assembly->tempdir, file->FileName );
        track_tempfile( package, file->TargetPath );
    }
    else
    {
        WCHAR *dir = resolve_target_folder( package, file->Component->Directory, FALSE, TRUE, NULL );
        file->TargetPath = build_directory_name( 2, dir, file->FileName );
        msi_free( dir );
    }

    TRACE("resolves to %s\n", debugstr_w(file->TargetPath));
}

static UINT calculate_file_cost( MSIPACKAGE *package )
{
    VS_FIXEDFILEINFO *file_version;
    WCHAR *font_version;
    MSIFILE *file;

    LIST_FOR_EACH_ENTRY( file, &package->files, MSIFILE, entry )
    {
        MSICOMPONENT *comp = file->Component;
        DWORD file_size;

        if (!comp->Enabled) continue;

        if (file->IsCompressed)
            comp->ForceLocalState = TRUE;

        set_target_path( package, file );

        if ((comp->assembly && !comp->assembly->installed) ||
            GetFileAttributesW(file->TargetPath) == INVALID_FILE_ATTRIBUTES)
        {
            comp->Cost += file->FileSize;
            continue;
        }
        file_size = msi_get_disk_file_size( file->TargetPath );

        if (file->Version)
        {
            if ((file_version = msi_get_disk_file_version( file->TargetPath )))
            {
                if (msi_compare_file_versions( file_version, file->Version ) < 0)
                {
                    comp->Cost += file->FileSize - file_size;
                }
                msi_free( file_version );
                continue;
            }
            else if ((font_version = font_version_from_file( file->TargetPath )))
            {
                if (msi_compare_font_versions( font_version, file->Version ) < 0)
                {
                    comp->Cost += file->FileSize - file_size;
                }
                msi_free( font_version );
                continue;
            }
        }
        if (file_size != file->FileSize)
        {
            comp->Cost += file->FileSize - file_size;
        }
    }
    return ERROR_SUCCESS;
}

/*
 * A lot is done in this function aside from just the costing.
 * The costing needs to be implemented at some point but for now I am going
 * to focus on the directory building
 *
 */
static UINT ACTION_CostFinalize(MSIPACKAGE *package)
{
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','D','i','r','e','c','t','o','r','y','`',0};
    static const WCHAR ConditionQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','C','o','n','d','i','t','i','o','n','`',0};
    static const WCHAR szCosting[] =
        {'C','o','s','t','i','n','g','C','o','m','p','l','e','t','e',0 };
    static const WCHAR szlevel[] =
        {'I','N','S','T','A','L','L','L','E','V','E','L',0};
    static const WCHAR szOutOfDiskSpace[] =
        {'O','u','t','O','f','D','i','s','k','S','p','a','c','e',0};
    MSICOMPONENT *comp;
    UINT rc = ERROR_SUCCESS;
    MSIQUERY * view;
    LPWSTR level;

    TRACE("Building Directory properties\n");

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc == ERROR_SUCCESS)
    {
        rc = MSI_IterateRecords(view, NULL, ITERATE_CostFinalizeDirectories,
                        package);
        msiobj_release(&view->hdr);
    }

    TRACE("Evaluating component conditions\n");
    LIST_FOR_EACH_ENTRY( comp, &package->components, MSICOMPONENT, entry )
    {
        if (MSI_EvaluateConditionW( package, comp->Condition ) == MSICONDITION_FALSE)
        {
            TRACE("Disabling component %s\n", debugstr_w(comp->Component));
            comp->Enabled = FALSE;
        }
        else
            comp->Enabled = TRUE;
    }

    /* read components states from the registry */
    ACTION_GetComponentInstallStates(package);
    ACTION_GetFeatureInstallStates(package);

    if (!process_overrides( package, msi_get_property_int( package->db, szlevel, 1 ) ))
    {
        TRACE("Evaluating feature conditions\n");

        rc = MSI_DatabaseOpenViewW( package->db, ConditionQuery, &view );
        if (rc == ERROR_SUCCESS)
        {
            rc = MSI_IterateRecords( view, NULL, ITERATE_CostFinalizeConditions, package );
            msiobj_release( &view->hdr );
        }
    }

    TRACE("Calculating file cost\n");
    calculate_file_cost( package );

    msi_set_property( package->db, szCosting, szOne );
    /* set default run level if not set */
    level = msi_dup_property( package->db, szlevel );
    if (!level)
        msi_set_property( package->db, szlevel, szOne );
    msi_free(level);

    /* FIXME: check volume disk space */
    msi_set_property( package->db, szOutOfDiskSpace, szZero );

    return MSI_SetFeatureStates(package);
}

/* OK this value is "interpreted" and then formatted based on the 
   first few characters */
static LPSTR parse_value(MSIPACKAGE *package, LPCWSTR value, DWORD *type, 
                         DWORD *size)
{
    LPSTR data = NULL;

    if (value[0]=='#' && value[1]!='#' && value[1]!='%')
    {
        if (value[1]=='x')
        {
            LPWSTR ptr;
            CHAR byte[5];
            LPWSTR deformated = NULL;
            int count;

            deformat_string(package, &value[2], &deformated);

            /* binary value type */
            ptr = deformated;
            *type = REG_BINARY;
            if (strlenW(ptr)%2)
                *size = (strlenW(ptr)/2)+1;
            else
                *size = strlenW(ptr)/2;

            data = msi_alloc(*size);

            byte[0] = '0'; 
            byte[1] = 'x'; 
            byte[4] = 0; 
            count = 0;
            /* if uneven pad with a zero in front */
            if (strlenW(ptr)%2)
            {
                byte[2]= '0';
                byte[3]= *ptr;
                ptr++;
                data[count] = (BYTE)strtol(byte,NULL,0);
                count ++;
                TRACE("Uneven byte count\n");
            }
            while (*ptr)
            {
                byte[2]= *ptr;
                ptr++;
                byte[3]= *ptr;
                ptr++;
                data[count] = (BYTE)strtol(byte,NULL,0);
                count ++;
            }
            msi_free(deformated);

            TRACE("Data %i bytes(%i)\n",*size,count);
        }
        else
        {
            LPWSTR deformated;
            LPWSTR p;
            DWORD d = 0;
            deformat_string(package, &value[1], &deformated);

            *type=REG_DWORD; 
            *size = sizeof(DWORD);
            data = msi_alloc(*size);
            p = deformated;
            if (*p == '-')
                p++;
            while (*p)
            {
                if ( (*p < '0') || (*p > '9') )
                    break;
                d *= 10;
                d += (*p - '0');
                p++;
            }
            if (deformated[0] == '-')
                d = -d;
            *(LPDWORD)data = d;
            TRACE("DWORD %i\n",*(LPDWORD)data);

            msi_free(deformated);
        }
    }
    else
    {
        static const WCHAR szMulti[] = {'[','~',']',0};
        LPCWSTR ptr;
        *type=REG_SZ;

        if (value[0]=='#')
        {
            if (value[1]=='%')
            {
                ptr = &value[2];
                *type=REG_EXPAND_SZ;
            }
            else
                ptr = &value[1];
         }
         else
            ptr=value;

        if (strstrW(value, szMulti))
            *type = REG_MULTI_SZ;

        /* remove initial delimiter */
        if (!strncmpW(value, szMulti, 3))
            ptr = value + 3;

        *size = deformat_string(package, ptr,(LPWSTR*)&data);

        /* add double NULL terminator */
        if (*type == REG_MULTI_SZ)
        {
            *size += 2 * sizeof(WCHAR); /* two NULL terminators */
            data = msi_realloc_zero(data, *size);
        }
    }
    return data;
}

static const WCHAR *get_root_key( MSIPACKAGE *package, INT root, HKEY *root_key )
{
    const WCHAR *ret;

    switch (root)
    {
    case -1:
        if (msi_get_property_int( package->db, szAllUsers, 0 ))
        {
            *root_key = HKEY_LOCAL_MACHINE;
            ret = szHLM;
        }
        else
        {
            *root_key = HKEY_CURRENT_USER;
            ret = szHCU;
        }
        break;
    case 0:
        *root_key = HKEY_CLASSES_ROOT;
        ret = szHCR;
        break;
    case 1:
        *root_key = HKEY_CURRENT_USER;
        ret = szHCU;
        break;
    case 2:
        *root_key = HKEY_LOCAL_MACHINE;
        ret = szHLM;
        break;
    case 3:
        *root_key = HKEY_USERS;
        ret = szHU;
        break;
    default:
        ERR("Unknown root %i\n", root);
        return NULL;
    }

    return ret;
}

static WCHAR *get_keypath( MSIPACKAGE *package, HKEY root, const WCHAR *path )
{
    static const WCHAR prefixW[] = {'S','O','F','T','W','A','R','E','\\'};
    static const UINT len = sizeof(prefixW) / sizeof(prefixW[0]);

    if (is_64bit && package->platform == PLATFORM_INTEL &&
        root == HKEY_LOCAL_MACHINE && !strncmpiW( path, prefixW, len ))
    {
        UINT size;
        WCHAR *path_32node;

        size = (strlenW( path ) + strlenW( szWow6432Node ) + 1) * sizeof(WCHAR);
        path_32node = msi_alloc( size );
        if (!path_32node)
            return NULL;

        memcpy( path_32node, path, len * sizeof(WCHAR) );
        path_32node[len] = 0;
        strcatW( path_32node, szWow6432Node );
        strcatW( path_32node, szBackSlash );
        strcatW( path_32node, path + len );
        return path_32node;
    }

    return strdupW( path );
}

static UINT ITERATE_WriteRegistryValues(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPSTR value_data = NULL;
    HKEY  root_key, hkey;
    DWORD type,size;
    LPWSTR deformated, uikey, keypath;
    LPCWSTR szRoot, component, name, key, value;
    MSICOMPONENT *comp;
    MSIRECORD * uirow;
    INT   root;
    BOOL check_first = FALSE;
    UINT rc;

    ui_progress(package,2,0,0,0);

    component = MSI_RecordGetString(row, 6);
    comp = get_loaded_component(package,component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    name = MSI_RecordGetString(row, 4);
    if( MSI_RecordIsNull(row,5) && name )
    {
        /* null values can have special meanings */
        if (name[0]=='-' && name[1] == 0)
                return ERROR_SUCCESS;
        else if ((name[0]=='+' && name[1] == 0) || 
                 (name[0] == '*' && name[1] == 0))
                name = NULL;
        check_first = TRUE;
    }

    root = MSI_RecordGetInteger(row,2);
    key = MSI_RecordGetString(row, 3);

    szRoot = get_root_key( package, root, &root_key );
    if (!szRoot)
        return ERROR_SUCCESS;

    deformat_string(package, key , &deformated);
    size = strlenW(deformated) + strlenW(szRoot) + 1;
    uikey = msi_alloc(size*sizeof(WCHAR));
    strcpyW(uikey,szRoot);
    strcatW(uikey,deformated);

    keypath = get_keypath( package, root_key, deformated );
    msi_free( deformated );
    if (RegCreateKeyW( root_key, keypath, &hkey ))
    {
        ERR("Could not create key %s\n", debugstr_w(keypath));
        msi_free(uikey);
        msi_free(keypath);
        return ERROR_SUCCESS;
    }

    value = MSI_RecordGetString(row,5);
    if (value)
        value_data = parse_value(package, value, &type, &size); 
    else
    {
        value_data = (LPSTR)strdupW(szEmpty);
        size = sizeof(szEmpty);
        type = REG_SZ;
    }

    deformat_string(package, name, &deformated);

    if (!check_first)
    {
        TRACE("Setting value %s of %s\n",debugstr_w(deformated),
                        debugstr_w(uikey));
        RegSetValueExW(hkey, deformated, 0, type, (LPBYTE)value_data, size);
    }
    else
    {
        DWORD sz = 0;
        rc = RegQueryValueExW(hkey, deformated, NULL, NULL, NULL, &sz);
        if (rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA)
        {
            TRACE("value %s of %s checked already exists\n",
                            debugstr_w(deformated), debugstr_w(uikey));
        }
        else
        {
            TRACE("Checked and setting value %s of %s\n",
                            debugstr_w(deformated), debugstr_w(uikey));
            if (deformated || size)
                RegSetValueExW(hkey, deformated, 0, type, (LPBYTE) value_data, size);
        }
    }
    RegCloseKey(hkey);

    uirow = MSI_CreateRecord(3);
    MSI_RecordSetStringW(uirow,2,deformated);
    MSI_RecordSetStringW(uirow,1,uikey);
    if (type == REG_SZ || type == REG_EXPAND_SZ)
        MSI_RecordSetStringW(uirow,3,(LPWSTR)value_data);
    ui_actiondata(package,szWriteRegistryValues,uirow);
    msiobj_release( &uirow->hdr );

    msi_free(value_data);
    msi_free(deformated);
    msi_free(uikey);
    msi_free(keypath);

    return ERROR_SUCCESS;
}

static UINT ACTION_WriteRegistryValues(MSIPACKAGE *package)
{
    UINT rc;
    MSIQUERY * view;
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','R','e','g','i','s','t','r','y','`',0 };

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    /* increment progress bar each time action data is sent */
    ui_progress(package,1,REG_PROGRESS_VALUE,1,0);

    rc = MSI_IterateRecords(view, NULL, ITERATE_WriteRegistryValues, package);

    msiobj_release(&view->hdr);
    return rc;
}

static void delete_reg_key_or_value( HKEY hkey_root, LPCWSTR key, LPCWSTR value, BOOL delete_key )
{
    LONG res;
    HKEY hkey;
    DWORD num_subkeys, num_values;

    if (delete_key)
    {
        if ((res = RegDeleteTreeW( hkey_root, key )))
        {
            TRACE("Failed to delete key %s (%d)\n", debugstr_w(key), res);
        }
        return;
    }

    if (!(res = RegOpenKeyW( hkey_root, key, &hkey )))
    {
        if ((res = RegDeleteValueW( hkey, value )))
        {
            TRACE("Failed to delete value %s (%d)\n", debugstr_w(value), res);
        }
        res = RegQueryInfoKeyW( hkey, NULL, NULL, NULL, &num_subkeys, NULL, NULL, &num_values,
                                NULL, NULL, NULL, NULL );
        RegCloseKey( hkey );
        if (!res && !num_subkeys && !num_values)
        {
            TRACE("Removing empty key %s\n", debugstr_w(key));
            RegDeleteKeyW( hkey_root, key );
        }
        return;
    }
    TRACE("Failed to open key %s (%d)\n", debugstr_w(key), res);
}


static UINT ITERATE_RemoveRegistryValuesOnUninstall( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR component, name, key_str, root_key_str;
    LPWSTR deformated_key, deformated_name, ui_key_str, keypath;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    BOOL delete_key = FALSE;
    HKEY hkey_root;
    UINT size;
    INT root;

    ui_progress( package, 2, 0, 0, 0 );

    component = MSI_RecordGetString( row, 6 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    name = MSI_RecordGetString( row, 4 );
    if (MSI_RecordIsNull( row, 5 ) && name )
    {
        if (name[0] == '+' && !name[1])
            return ERROR_SUCCESS;
        else if ((name[0] == '-' && !name[1]) || (name[0] == '*' && !name[1]))
        {
            delete_key = TRUE;
            name = NULL;
        }
    }

    root = MSI_RecordGetInteger( row, 2 );
    key_str = MSI_RecordGetString( row, 3 );

    root_key_str = get_root_key( package, root, &hkey_root );
    if (!root_key_str)
        return ERROR_SUCCESS;

    deformat_string( package, key_str, &deformated_key );
    size = strlenW( deformated_key ) + strlenW( root_key_str ) + 1;
    ui_key_str = msi_alloc( size * sizeof(WCHAR) );
    strcpyW( ui_key_str, root_key_str );
    strcatW( ui_key_str, deformated_key );

    deformat_string( package, name, &deformated_name );

    keypath = get_keypath( package, hkey_root, deformated_key );
    msi_free( deformated_key );
    delete_reg_key_or_value( hkey_root, keypath, deformated_name, delete_key );
    msi_free( keypath );

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, ui_key_str );
    MSI_RecordSetStringW( uirow, 2, deformated_name );

    ui_actiondata( package, szRemoveRegistryValues, uirow );
    msiobj_release( &uirow->hdr );

    msi_free( ui_key_str );
    msi_free( deformated_name );
    return ERROR_SUCCESS;
}

static UINT ITERATE_RemoveRegistryValuesOnInstall( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR component, name, key_str, root_key_str;
    LPWSTR deformated_key, deformated_name, ui_key_str, keypath;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    BOOL delete_key = FALSE;
    HKEY hkey_root;
    UINT size;
    INT root;

    ui_progress( package, 2, 0, 0, 0 );

    component = MSI_RecordGetString( row, 5 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    if ((name = MSI_RecordGetString( row, 4 )))
    {
        if (name[0] == '-' && !name[1])
        {
            delete_key = TRUE;
            name = NULL;
        }
    }

    root = MSI_RecordGetInteger( row, 2 );
    key_str = MSI_RecordGetString( row, 3 );

    root_key_str = get_root_key( package, root, &hkey_root );
    if (!root_key_str)
        return ERROR_SUCCESS;

    deformat_string( package, key_str, &deformated_key );
    size = strlenW( deformated_key ) + strlenW( root_key_str ) + 1;
    ui_key_str = msi_alloc( size * sizeof(WCHAR) );
    strcpyW( ui_key_str, root_key_str );
    strcatW( ui_key_str, deformated_key );

    deformat_string( package, name, &deformated_name );

    keypath = get_keypath( package, hkey_root, deformated_key );
    msi_free( deformated_key );
    delete_reg_key_or_value( hkey_root, keypath, deformated_name, delete_key );
    msi_free( keypath );

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, ui_key_str );
    MSI_RecordSetStringW( uirow, 2, deformated_name );

    ui_actiondata( package, szRemoveRegistryValues, uirow );
    msiobj_release( &uirow->hdr );

    msi_free( ui_key_str );
    msi_free( deformated_name );
    return ERROR_SUCCESS;
}

static UINT ACTION_RemoveRegistryValues( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR registry_query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','R','e','g','i','s','t','r','y','`',0 };
    static const WCHAR remove_registry_query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','R','e','m','o','v','e','R','e','g','i','s','t','r','y','`',0 };

    /* increment progress bar each time action data is sent */
    ui_progress( package, 1, REG_PROGRESS_VALUE, 1, 0 );

    rc = MSI_DatabaseOpenViewW( package->db, registry_query, &view );
    if (rc == ERROR_SUCCESS)
    {
        rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveRegistryValuesOnUninstall, package );
        msiobj_release( &view->hdr );
        if (rc != ERROR_SUCCESS)
            return rc;
    }

    rc = MSI_DatabaseOpenViewW( package->db, remove_registry_query, &view );
    if (rc == ERROR_SUCCESS)
    {
        rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveRegistryValuesOnInstall, package );
        msiobj_release( &view->hdr );
        if (rc != ERROR_SUCCESS)
            return rc;
    }

    return ERROR_SUCCESS;
}

static UINT ACTION_InstallInitialize(MSIPACKAGE *package)
{
    package->script->CurrentlyScripting = TRUE;

    return ERROR_SUCCESS;
}


static UINT ACTION_InstallValidate(MSIPACKAGE *package)
{
    MSICOMPONENT *comp;
    DWORD progress = 0;
    DWORD total = 0;
    static const WCHAR q1[]=
        {'S','E','L','E','C','T',' ','*',' ', 'F','R','O','M',' ',
         '`','R','e','g','i','s','t','r','y','`',0};
    UINT rc;
    MSIQUERY * view;
    MSIFEATURE *feature;
    MSIFILE *file;

    TRACE("InstallValidate\n");

    rc = MSI_DatabaseOpenViewW(package->db, q1, &view);
    if (rc == ERROR_SUCCESS)
    {
        MSI_IterateRecords( view, &progress, NULL, package );
        msiobj_release( &view->hdr );
        total += progress * REG_PROGRESS_VALUE;
    }

    LIST_FOR_EACH_ENTRY( comp, &package->components, MSICOMPONENT, entry )
        total += COMPONENT_PROGRESS_VALUE;

    LIST_FOR_EACH_ENTRY( file, &package->files, MSIFILE, entry )
        total += file->FileSize;

    ui_progress(package,0,total,0,0);

    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        TRACE("Feature: %s Installed %d Request %d Action %d\n",
              debugstr_w(feature->Feature), feature->Installed,
              feature->ActionRequest, feature->Action);
    }
    
    return ERROR_SUCCESS;
}

static UINT ITERATE_LaunchConditions(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE* package = param;
    LPCWSTR cond = NULL; 
    LPCWSTR message = NULL;
    UINT r;

    static const WCHAR title[]=
        {'I','n','s','t','a','l','l',' ','F','a', 'i','l','e','d',0};

    cond = MSI_RecordGetString(row,1);

    r = MSI_EvaluateConditionW(package,cond);
    if (r == MSICONDITION_FALSE)
    {
        if ((gUILevel & INSTALLUILEVEL_MASK) != INSTALLUILEVEL_NONE)
        {
            LPWSTR deformated;
            message = MSI_RecordGetString(row,2);
            deformat_string(package,message,&deformated);
            MessageBoxW(NULL,deformated,title,MB_OK);
            msi_free(deformated);
        }

        return ERROR_INSTALL_FAILURE;
    }

    return ERROR_SUCCESS;
}

static UINT ACTION_LaunchConditions(MSIPACKAGE *package)
{
    UINT rc;
    MSIQUERY * view = NULL;
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','L','a','u','n','c','h','C','o','n','d','i','t','i','o','n','`',0};

    TRACE("Checking launch conditions\n");

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_LaunchConditions, package);
    msiobj_release(&view->hdr);

    return rc;
}

static LPWSTR resolve_keypath( MSIPACKAGE* package, MSICOMPONENT *cmp )
{

    if (!cmp->KeyPath)
        return resolve_target_folder( package, cmp->Directory, FALSE, TRUE, NULL );

    if (cmp->Attributes & msidbComponentAttributesRegistryKeyPath)
    {
        MSIRECORD * row = 0;
        UINT root,len;
        LPWSTR deformated,buffer,deformated_name;
        LPCWSTR key,name;
        static const WCHAR ExecSeqQuery[] =
            {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
             '`','R','e','g','i','s','t','r','y','`',' ',
             'W','H','E','R','E',' ', '`','R','e','g','i','s','t','r','y','`',
             ' ','=',' ' ,'\'','%','s','\'',0 };
        static const WCHAR fmt[]={'%','0','2','i',':','\\','%','s','\\',0};
        static const WCHAR fmt2[]=
            {'%','0','2','i',':','\\','%','s','\\','%','s',0};

        row = MSI_QueryGetRecord(package->db, ExecSeqQuery,cmp->KeyPath);
        if (!row)
            return NULL;

        root = MSI_RecordGetInteger(row,2);
        key = MSI_RecordGetString(row, 3);
        name = MSI_RecordGetString(row, 4);
        deformat_string(package, key , &deformated);
        deformat_string(package, name, &deformated_name);

        len = strlenW(deformated) + 6;
        if (deformated_name)
            len+=strlenW(deformated_name);

        buffer = msi_alloc( len *sizeof(WCHAR));

        if (deformated_name)
            sprintfW(buffer,fmt2,root,deformated,deformated_name);
        else
            sprintfW(buffer,fmt,root,deformated);

        msi_free(deformated);
        msi_free(deformated_name);
        msiobj_release(&row->hdr);

        return buffer;
    }
    else if (cmp->Attributes & msidbComponentAttributesODBCDataSource)
    {
        FIXME("UNIMPLEMENTED keypath as ODBC Source\n");
        return NULL;
    }
    else
    {
        MSIFILE *file = get_loaded_file( package, cmp->KeyPath );

        if (file)
            return strdupW( file->TargetPath );
    }
    return NULL;
}

static HKEY openSharedDLLsKey(void)
{
    HKEY hkey=0;
    static const WCHAR path[] =
        {'S','o','f','t','w','a','r','e','\\',
         'M','i','c','r','o','s','o','f','t','\\',
         'W','i','n','d','o','w','s','\\',
         'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
         'S','h','a','r','e','d','D','L','L','s',0};

    RegCreateKeyW(HKEY_LOCAL_MACHINE,path,&hkey);
    return hkey;
}

static UINT ACTION_GetSharedDLLsCount(LPCWSTR dll)
{
    HKEY hkey;
    DWORD count=0;
    DWORD type;
    DWORD sz = sizeof(count);
    DWORD rc;
    
    hkey = openSharedDLLsKey();
    rc = RegQueryValueExW(hkey, dll, NULL, &type, (LPBYTE)&count, &sz);
    if (rc != ERROR_SUCCESS)
        count = 0;
    RegCloseKey(hkey);
    return count;
}

static UINT ACTION_WriteSharedDLLsCount(LPCWSTR path, UINT count)
{
    HKEY hkey;

    hkey = openSharedDLLsKey();
    if (count > 0)
        msi_reg_set_val_dword( hkey, path, count );
    else
        RegDeleteValueW(hkey,path);
    RegCloseKey(hkey);
    return count;
}

static void ACTION_RefCountComponent( MSIPACKAGE* package, MSICOMPONENT *comp )
{
    MSIFEATURE *feature;
    INT count = 0;
    BOOL write = FALSE;

    /* only refcount DLLs */
    if (comp->KeyPath == NULL || 
        comp->assembly ||
        comp->Attributes & msidbComponentAttributesRegistryKeyPath || 
        comp->Attributes & msidbComponentAttributesODBCDataSource)
        write = FALSE;
    else
    {
        count = ACTION_GetSharedDLLsCount( comp->FullKeypath);
        write = (count > 0);

        if (comp->Attributes & msidbComponentAttributesSharedDllRefCount)
            write = TRUE;
    }

    /* increment counts */
    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        ComponentList *cl;

        if (feature->ActionRequest != INSTALLSTATE_LOCAL)
            continue;

        LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
        {
            if ( cl->component == comp )
                count++;
        }
    }

    /* decrement counts */
    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        ComponentList *cl;

        if (feature->ActionRequest != INSTALLSTATE_ABSENT)
            continue;

        LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
        {
            if ( cl->component == comp )
                count--;
        }
    }

    /* ref count all the files in the component */
    if (write)
    {
        MSIFILE *file;

        LIST_FOR_EACH_ENTRY( file, &package->files, MSIFILE, entry )
        {
            if (file->Component == comp)
                ACTION_WriteSharedDLLsCount( file->TargetPath, count );
        }
    }
    
    /* add a count for permanent */
    if (comp->Attributes & msidbComponentAttributesPermanent)
        count ++;
    
    comp->RefCount = count;

    if (write)
        ACTION_WriteSharedDLLsCount( comp->FullKeypath, comp->RefCount );
}

static UINT ACTION_ProcessComponents(MSIPACKAGE *package)
{
    WCHAR squished_pc[GUID_SIZE];
    WCHAR squished_cc[GUID_SIZE];
    UINT rc;
    MSICOMPONENT *comp;
    HKEY hkey;

    TRACE("\n");

    squash_guid(package->ProductCode,squished_pc);
    ui_progress(package,1,COMPONENT_PROGRESS_VALUE,1,0);

    msi_set_sourcedir_props(package, FALSE);

    LIST_FOR_EACH_ENTRY( comp, &package->components, MSICOMPONENT, entry )
    {
        MSIRECORD * uirow;

        ui_progress(package,2,0,0,0);
        if (!comp->ComponentId)
            continue;

        squash_guid(comp->ComponentId,squished_cc);

        msi_free(comp->FullKeypath);
        if (comp->assembly)
        {
            const WCHAR prefixW[] = {'<','\\',0};
            DWORD len = strlenW( prefixW ) + strlenW( comp->assembly->display_name );

            comp->FullKeypath = msi_alloc( (len + 1) * sizeof(WCHAR) );
            if (comp->FullKeypath)
            {
                strcpyW( comp->FullKeypath, prefixW );
                strcatW( comp->FullKeypath, comp->assembly->display_name );
            }
        }
        else comp->FullKeypath = resolve_keypath( package, comp );

        ACTION_RefCountComponent( package, comp );

        TRACE("Component %s (%s), Keypath=%s, RefCount=%i Request=%u\n",
                            debugstr_w(comp->Component),
                            debugstr_w(squished_cc),
                            debugstr_w(comp->FullKeypath),
                            comp->RefCount,
                            comp->ActionRequest);

        if (comp->ActionRequest == INSTALLSTATE_LOCAL ||
            comp->ActionRequest == INSTALLSTATE_SOURCE)
        {
            if (package->Context == MSIINSTALLCONTEXT_MACHINE)
                rc = MSIREG_OpenUserDataComponentKey(comp->ComponentId, szLocalSid, &hkey, TRUE);
            else
                rc = MSIREG_OpenUserDataComponentKey(comp->ComponentId, NULL, &hkey, TRUE);

            if (rc != ERROR_SUCCESS)
                continue;

            if (comp->Attributes & msidbComponentAttributesPermanent)
            {
                static const WCHAR szPermKey[] =
                    { '0','0','0','0','0','0','0','0','0','0','0','0',
                      '0','0','0','0','0','0','0','0','0','0','0','0',
                      '0','0','0','0','0','0','0','0',0 };

                msi_reg_set_val_str(hkey, szPermKey, comp->FullKeypath);
            }

            if (comp->ActionRequest == INSTALLSTATE_LOCAL)
                msi_reg_set_val_str(hkey, squished_pc, comp->FullKeypath);
            else
            {
                MSIFILE *file;
                MSIRECORD *row;
                LPWSTR ptr, ptr2;
                WCHAR source[MAX_PATH];
                WCHAR base[MAX_PATH];
                LPWSTR sourcepath;

                static const WCHAR fmt[] = {'%','0','2','d','\\',0};
                static const WCHAR query[] = {
                    'S','E','L','E','C','T',' ','*',' ', 'F','R','O','M',' ',
                    '`','M','e','d','i','a','`',' ','W','H','E','R','E',' ',
                    '`','L','a','s','t','S','e','q','u','e','n','c','e','`',' ',
                    '>','=',' ','%','i',' ','O','R','D','E','R',' ','B','Y',' ',
                    '`','D','i','s','k','I','d','`',0};

                if (!comp->KeyPath || !(file = get_loaded_file(package, comp->KeyPath)))
                    continue;

                row = MSI_QueryGetRecord(package->db, query, file->Sequence);
                sprintfW(source, fmt, MSI_RecordGetInteger(row, 1));
                ptr2 = strrchrW(source, '\\') + 1;
                msiobj_release(&row->hdr);

                lstrcpyW(base, package->PackagePath);
                ptr = strrchrW(base, '\\');
                *(ptr + 1) = '\0';

                sourcepath = resolve_file_source(package, file);
                ptr = sourcepath + lstrlenW(base);
                lstrcpyW(ptr2, ptr);
                msi_free(sourcepath);

                msi_reg_set_val_str(hkey, squished_pc, source);
            }
            RegCloseKey(hkey);
        }
        else if (comp->ActionRequest == INSTALLSTATE_ABSENT)
        {
            if (package->Context == MSIINSTALLCONTEXT_MACHINE)
                MSIREG_DeleteUserDataComponentKey(comp->ComponentId, szLocalSid);
            else
                MSIREG_DeleteUserDataComponentKey(comp->ComponentId, NULL);
        }
        comp->Action = comp->ActionRequest;

        /* UI stuff */
        uirow = MSI_CreateRecord(3);
        MSI_RecordSetStringW(uirow,1,package->ProductCode);
        MSI_RecordSetStringW(uirow,2,comp->ComponentId);
        MSI_RecordSetStringW(uirow,3,comp->FullKeypath);
        ui_actiondata(package,szProcessComponents,uirow);
        msiobj_release( &uirow->hdr );
    }

    return ERROR_SUCCESS;
}

typedef struct {
    CLSID       clsid;
    LPWSTR      source;

    LPWSTR      path;
    ITypeLib    *ptLib;
} typelib_struct;

static BOOL CALLBACK Typelib_EnumResNameProc( HMODULE hModule, LPCWSTR lpszType, 
                                       LPWSTR lpszName, LONG_PTR lParam)
{
    TLIBATTR *attr;
    typelib_struct *tl_struct = (typelib_struct*) lParam;
    static const WCHAR fmt[] = {'%','s','\\','%','i',0};
    int sz; 
    HRESULT res;

    if (!IS_INTRESOURCE(lpszName))
    {
        ERR("Not Int Resource Name %s\n",debugstr_w(lpszName));
        return TRUE;
    }

    sz = strlenW(tl_struct->source)+4;
    sz *= sizeof(WCHAR);

    if ((INT_PTR)lpszName == 1)
        tl_struct->path = strdupW(tl_struct->source);
    else
    {
        tl_struct->path = msi_alloc(sz);
        sprintfW(tl_struct->path,fmt,tl_struct->source, lpszName);
    }

    TRACE("trying %s\n", debugstr_w(tl_struct->path));
    res = LoadTypeLib(tl_struct->path,&tl_struct->ptLib);
    if (FAILED(res))
    {
        msi_free(tl_struct->path);
        tl_struct->path = NULL;

        return TRUE;
    }

    ITypeLib_GetLibAttr(tl_struct->ptLib, &attr);
    if (IsEqualGUID(&(tl_struct->clsid),&(attr->guid)))
    {
        ITypeLib_ReleaseTLibAttr(tl_struct->ptLib, attr);
        return FALSE;
    }

    msi_free(tl_struct->path);
    tl_struct->path = NULL;

    ITypeLib_ReleaseTLibAttr(tl_struct->ptLib, attr);
    ITypeLib_Release(tl_struct->ptLib);

    return TRUE;
}

static UINT ITERATE_RegisterTypeLibraries(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE* package = param;
    LPCWSTR component;
    MSICOMPONENT *comp;
    MSIFILE *file;
    typelib_struct tl_struct;
    ITypeLib *tlib;
    HMODULE module;
    HRESULT hr;

    component = MSI_RecordGetString(row,3);
    comp = get_loaded_component(package,component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    if (!comp->KeyPath || !(file = get_loaded_file( package, comp->KeyPath )))
    {
        TRACE("component has no key path\n");
        return ERROR_SUCCESS;
    }
    ui_actiondata( package, szRegisterTypeLibraries, row );

    module = LoadLibraryExW( file->TargetPath, NULL, LOAD_LIBRARY_AS_DATAFILE );
    if (module)
    {
        LPCWSTR guid;
        guid = MSI_RecordGetString(row,1);
        CLSIDFromString((LPCWSTR)guid, &tl_struct.clsid);
        tl_struct.source = strdupW( file->TargetPath );
        tl_struct.path = NULL;

        EnumResourceNamesW(module, szTYPELIB, Typelib_EnumResNameProc,
                        (LONG_PTR)&tl_struct);

        if (tl_struct.path)
        {
            LPWSTR help = NULL;
            LPCWSTR helpid;
            HRESULT res;

            helpid = MSI_RecordGetString(row,6);

            if (helpid) help = resolve_target_folder( package, helpid, FALSE, TRUE, NULL );
            res = RegisterTypeLib(tl_struct.ptLib,tl_struct.path,help);
            msi_free(help);

            if (FAILED(res))
                ERR("Failed to register type library %s\n",
                        debugstr_w(tl_struct.path));
            else
                TRACE("Registered %s\n", debugstr_w(tl_struct.path));

            ITypeLib_Release(tl_struct.ptLib);
            msi_free(tl_struct.path);
        }
        else
            ERR("Failed to load type library %s\n",
                    debugstr_w(tl_struct.source));

        FreeLibrary(module);
        msi_free(tl_struct.source);
    }
    else
    {
        hr = LoadTypeLibEx(file->TargetPath, REGKIND_REGISTER, &tlib);
        if (FAILED(hr))
        {
            ERR("Failed to load type library: %08x\n", hr);
            return ERROR_INSTALL_FAILURE;
        }

        ITypeLib_Release(tlib);
    }

    return ERROR_SUCCESS;
}

static UINT ACTION_RegisterTypeLibraries(MSIPACKAGE *package)
{
    /* 
     * OK this is a bit confusing.. I am given a _Component key and I believe
     * that the file that is being registered as a type library is the "key file
     * of that component" which I interpret to mean "The file in the KeyPath of
     * that component".
     */
    UINT rc;
    MSIQUERY * view;
    static const WCHAR Query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','T','y','p','e','L','i','b','`',0};

    rc = MSI_DatabaseOpenViewW(package->db, Query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_RegisterTypeLibraries, package);
    msiobj_release(&view->hdr);
    return rc;
}

static UINT ITERATE_UnregisterTypeLibraries( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR component, guid;
    MSICOMPONENT *comp;
    GUID libid;
    UINT version;
    LCID language;
    SYSKIND syskind;
    HRESULT hr;

    component = MSI_RecordGetString( row, 3 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    ui_actiondata( package, szUnregisterTypeLibraries, row );

    guid = MSI_RecordGetString( row, 1 );
    CLSIDFromString( (LPCWSTR)guid, &libid );
    version = MSI_RecordGetInteger( row, 4 );
    language = MSI_RecordGetInteger( row, 2 );

#ifdef _WIN64
    syskind = SYS_WIN64;
#else
    syskind = SYS_WIN32;
#endif

    hr = UnRegisterTypeLib( &libid, (version >> 8) & 0xffff, version & 0xff, language, syskind );
    if (FAILED(hr))
    {
        WARN("Failed to unregister typelib: %08x\n", hr);
    }

    return ERROR_SUCCESS;
}

static UINT ACTION_UnregisterTypeLibraries( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','T','y','p','e','L','i','b','`',0};

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_UnregisterTypeLibraries, package );
    msiobj_release( &view->hdr );
    return rc;
}

static WCHAR *get_link_file( MSIPACKAGE *package, MSIRECORD *row )
{
    static const WCHAR szlnk[] = {'.','l','n','k',0};
    LPCWSTR directory, extension;
    LPWSTR link_folder, link_file, filename;

    directory = MSI_RecordGetString( row, 2 );
    link_folder = resolve_target_folder( package, directory, FALSE, TRUE, NULL );

    /* may be needed because of a bug somewhere else */
    create_full_pathW( link_folder );

    filename = msi_dup_record_field( row, 3 );
    reduce_to_longfilename( filename );

    extension = strchrW( filename, '.' );
    if (!extension || strcmpiW( extension, szlnk ))
    {
        int len = strlenW( filename );
        filename = msi_realloc( filename, len * sizeof(WCHAR) + sizeof(szlnk) );
        memcpy( filename + len, szlnk, sizeof(szlnk) );
    }
    link_file = build_directory_name( 2, link_folder, filename );
    msi_free( link_folder );
    msi_free( filename );

    return link_file;
}

static UINT ITERATE_CreateShortcuts(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPWSTR link_file, deformated, path;
    LPCWSTR component, target;
    MSICOMPONENT *comp;
    IShellLinkW *sl = NULL;
    IPersistFile *pf = NULL;
    HRESULT res;

    component = MSI_RecordGetString(row, 4);
    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    ui_actiondata(package,szCreateShortcuts,row);

    res = CoCreateInstance( &CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                    &IID_IShellLinkW, (LPVOID *) &sl );

    if (FAILED( res ))
    {
        ERR("CLSID_ShellLink not available\n");
        goto err;
    }

    res = IShellLinkW_QueryInterface( sl, &IID_IPersistFile,(LPVOID*) &pf );
    if (FAILED( res ))
    {
        ERR("QueryInterface(IID_IPersistFile) failed\n");
        goto err;
    }

    target = MSI_RecordGetString(row, 5);
    if (strchrW(target, '['))
    {
        deformat_string(package, target, &deformated);
        IShellLinkW_SetPath(sl,deformated);
        msi_free(deformated);
    }
    else
    {
        FIXME("poorly handled shortcut format, advertised shortcut\n");
        IShellLinkW_SetPath(sl,comp->FullKeypath);
    }

    if (!MSI_RecordIsNull(row,6))
    {
        LPCWSTR arguments = MSI_RecordGetString(row, 6);
        deformat_string(package, arguments, &deformated);
        IShellLinkW_SetArguments(sl,deformated);
        msi_free(deformated);
    }

    if (!MSI_RecordIsNull(row,7))
    {
        LPCWSTR description = MSI_RecordGetString(row, 7);
        IShellLinkW_SetDescription(sl, description);
    }

    if (!MSI_RecordIsNull(row,8))
        IShellLinkW_SetHotkey(sl,MSI_RecordGetInteger(row,8));

    if (!MSI_RecordIsNull(row,9))
    {
        INT index; 
        LPCWSTR icon = MSI_RecordGetString(row, 9);

        path = build_icon_path(package, icon);
        index = MSI_RecordGetInteger(row,10);

        /* no value means 0 */
        if (index == MSI_NULL_INTEGER)
            index = 0;

        IShellLinkW_SetIconLocation(sl, path, index);
        msi_free(path);
    }

    if (!MSI_RecordIsNull(row,11))
        IShellLinkW_SetShowCmd(sl,MSI_RecordGetInteger(row,11));

    if (!MSI_RecordIsNull(row,12))
    {
        LPCWSTR wkdir = MSI_RecordGetString(row, 12);
        path = resolve_target_folder( package, wkdir, FALSE, TRUE, NULL );
        if (path)
            IShellLinkW_SetWorkingDirectory(sl, path);
        msi_free(path);
    }

    link_file = get_link_file(package, row);

    TRACE("Writing shortcut to %s\n", debugstr_w(link_file));
    IPersistFile_Save(pf, link_file, FALSE);

    msi_free(link_file);

err:
    if (pf)
        IPersistFile_Release( pf );
    if (sl)
        IShellLinkW_Release( sl );

    return ERROR_SUCCESS;
}

static UINT ACTION_CreateShortcuts(MSIPACKAGE *package)
{
    UINT rc;
    HRESULT res;
    MSIQUERY * view;
    static const WCHAR Query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','S','h','o','r','t','c','u','t','`',0};

    rc = MSI_DatabaseOpenViewW(package->db, Query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    res = CoInitialize( NULL );

    rc = MSI_IterateRecords(view, NULL, ITERATE_CreateShortcuts, package);
    msiobj_release(&view->hdr);

    if (SUCCEEDED(res))
        CoUninitialize();

    return rc;
}

static UINT ITERATE_RemoveShortcuts( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPWSTR link_file;
    LPCWSTR component;
    MSICOMPONENT *comp;

    component = MSI_RecordGetString( row, 4 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    ui_actiondata( package, szRemoveShortcuts, row );

    link_file = get_link_file( package, row );

    TRACE("Removing shortcut file %s\n", debugstr_w( link_file ));
    if (!DeleteFileW( link_file ))
    {
        WARN("Failed to remove shortcut file %u\n", GetLastError());
    }
    msi_free( link_file );

    return ERROR_SUCCESS;
}

static UINT ACTION_RemoveShortcuts( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','S','h','o','r','t','c','u','t','`',0};

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveShortcuts, package );
    msiobj_release( &view->hdr );

    return rc;
}

static UINT ITERATE_PublishIcon(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE* package = param;
    HANDLE the_file;
    LPWSTR FilePath;
    LPCWSTR FileName;
    CHAR buffer[1024];
    DWORD sz;
    UINT rc;

    FileName = MSI_RecordGetString(row,1);
    if (!FileName)
    {
        ERR("Unable to get FileName\n");
        return ERROR_SUCCESS;
    }

    FilePath = build_icon_path(package,FileName);

    TRACE("Creating icon file at %s\n",debugstr_w(FilePath));

    the_file = CreateFileW(FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);

    if (the_file == INVALID_HANDLE_VALUE)
    {
        ERR("Unable to create file %s\n",debugstr_w(FilePath));
        msi_free(FilePath);
        return ERROR_SUCCESS;
    }

    do 
    {
        DWORD write;
        sz = 1024;
        rc = MSI_RecordReadStream(row,2,buffer,&sz);
        if (rc != ERROR_SUCCESS)
        {
            ERR("Failed to get stream\n");
            CloseHandle(the_file);  
            DeleteFileW(FilePath);
            break;
        }
        WriteFile(the_file,buffer,sz,&write,NULL);
    } while (sz == 1024);

    msi_free(FilePath);
    CloseHandle(the_file);

    return ERROR_SUCCESS;
}

static UINT msi_publish_icons(MSIPACKAGE *package)
{
    UINT r;
    MSIQUERY *view;

    static const WCHAR query[]= {
        'S','E','L','E','C','T',' ','*',' ',
        'F','R','O','M',' ','`','I','c','o','n','`',0};

    r = MSI_DatabaseOpenViewW(package->db, query, &view);
    if (r == ERROR_SUCCESS)
    {
        MSI_IterateRecords(view, NULL, ITERATE_PublishIcon, package);
        msiobj_release(&view->hdr);
    }

    return ERROR_SUCCESS;
}

static UINT msi_publish_sourcelist(MSIPACKAGE *package, HKEY hkey)
{
    UINT r;
    HKEY source;
    LPWSTR buffer;
    MSIMEDIADISK *disk;
    MSISOURCELISTINFO *info;

    r = RegCreateKeyW(hkey, szSourceList, &source);
    if (r != ERROR_SUCCESS)
        return r;

    RegCloseKey(source);

    buffer = strrchrW(package->PackagePath, '\\') + 1;
    r = MsiSourceListSetInfoW(package->ProductCode, NULL,
                              package->Context, MSICODE_PRODUCT,
                              INSTALLPROPERTY_PACKAGENAMEW, buffer);
    if (r != ERROR_SUCCESS)
        return r;

    r = MsiSourceListSetInfoW(package->ProductCode, NULL,
                              package->Context, MSICODE_PRODUCT,
                              INSTALLPROPERTY_MEDIAPACKAGEPATHW, szEmpty);
    if (r != ERROR_SUCCESS)
        return r;

    r = MsiSourceListSetInfoW(package->ProductCode, NULL,
                              package->Context, MSICODE_PRODUCT,
                              INSTALLPROPERTY_DISKPROMPTW, szEmpty);
    if (r != ERROR_SUCCESS)
        return r;

    LIST_FOR_EACH_ENTRY(info, &package->sourcelist_info, MSISOURCELISTINFO, entry)
    {
        if (!strcmpW( info->property, INSTALLPROPERTY_LASTUSEDSOURCEW ))
            msi_set_last_used_source(package->ProductCode, NULL, info->context,
                                     info->options, info->value);
        else
            MsiSourceListSetInfoW(package->ProductCode, NULL,
                                  info->context, info->options,
                                  info->property, info->value);
    }

    LIST_FOR_EACH_ENTRY(disk, &package->sourcelist_media, MSIMEDIADISK, entry)
    {
        MsiSourceListAddMediaDiskW(package->ProductCode, NULL,
                                   disk->context, disk->options,
                                   disk->disk_id, disk->volume_label, disk->disk_prompt);
    }

    return ERROR_SUCCESS;
}

static UINT msi_publish_product_properties(MSIPACKAGE *package, HKEY hkey)
{
    MSIHANDLE hdb, suminfo;
    WCHAR guids[MAX_PATH];
    WCHAR packcode[SQUISH_GUID_SIZE];
    LPWSTR buffer;
    LPWSTR ptr;
    DWORD langid;
    DWORD size;
    UINT r;

    static const WCHAR szProductLanguage[] =
        {'P','r','o','d','u','c','t','L','a','n','g','u','a','g','e',0};
    static const WCHAR szARPProductIcon[] =
        {'A','R','P','P','R','O','D','U','C','T','I','C','O','N',0};
    static const WCHAR szProductVersion[] =
        {'P','r','o','d','u','c','t','V','e','r','s','i','o','n',0};
    static const WCHAR szAssignment[] =
        {'A','s','s','i','g','n','m','e','n','t',0};
    static const WCHAR szAdvertiseFlags[] =
        {'A','d','v','e','r','t','i','s','e','F','l','a','g','s',0};
    static const WCHAR szClients[] =
        {'C','l','i','e','n','t','s',0};
    static const WCHAR szColon[] = {':',0};

    buffer = msi_dup_property(package->db, INSTALLPROPERTY_PRODUCTNAMEW);
    msi_reg_set_val_str(hkey, INSTALLPROPERTY_PRODUCTNAMEW, buffer);
    msi_free(buffer);

    langid = msi_get_property_int(package->db, szProductLanguage, 0);
    msi_reg_set_val_dword(hkey, INSTALLPROPERTY_LANGUAGEW, langid);

    /* FIXME */
    msi_reg_set_val_dword(hkey, INSTALLPROPERTY_AUTHORIZED_LUA_APPW, 0);

    buffer = msi_dup_property(package->db, szARPProductIcon);
    if (buffer)
    {
        LPWSTR path = build_icon_path(package,buffer);
        msi_reg_set_val_str(hkey, INSTALLPROPERTY_PRODUCTICONW, path);
        msi_free(path);
        msi_free(buffer);
    }

    buffer = msi_dup_property(package->db, szProductVersion);
    if (buffer)
    {
        DWORD verdword = msi_version_str_to_dword(buffer);
        msi_reg_set_val_dword(hkey, INSTALLPROPERTY_VERSIONW, verdword);
        msi_free(buffer);
    }

    msi_reg_set_val_dword(hkey, szAssignment, 0);
    msi_reg_set_val_dword(hkey, szAdvertiseFlags, 0x184);
    msi_reg_set_val_dword(hkey, INSTALLPROPERTY_INSTANCETYPEW, 0);
    msi_reg_set_val_str(hkey, szClients, szColon);

    hdb = alloc_msihandle(&package->db->hdr);
    if (!hdb)
        return ERROR_NOT_ENOUGH_MEMORY;

    r = MsiGetSummaryInformationW(hdb, NULL, 0, &suminfo);
    MsiCloseHandle(hdb);
    if (r != ERROR_SUCCESS)
        goto done;

    size = MAX_PATH;
    r = MsiSummaryInfoGetPropertyW(suminfo, PID_REVNUMBER, NULL, NULL,
                                   NULL, guids, &size);
    if (r != ERROR_SUCCESS)
        goto done;

    ptr = strchrW(guids, ';');
    if (ptr) *ptr = 0;
    squash_guid(guids, packcode);
    msi_reg_set_val_str(hkey, INSTALLPROPERTY_PACKAGECODEW, packcode);

done:
    MsiCloseHandle(suminfo);
    return ERROR_SUCCESS;
}

static UINT msi_publish_upgrade_code(MSIPACKAGE *package)
{
    UINT r;
    HKEY hkey;
    LPWSTR upgrade;
    WCHAR squashed_pc[SQUISH_GUID_SIZE];

    upgrade = msi_dup_property(package->db, szUpgradeCode);
    if (!upgrade)
        return ERROR_SUCCESS;

    if (package->Context == MSIINSTALLCONTEXT_MACHINE)
    {
        r = MSIREG_OpenClassesUpgradeCodesKey(upgrade, &hkey, TRUE);
        if (r != ERROR_SUCCESS)
            goto done;
    }
    else
    {
        r = MSIREG_OpenUserUpgradeCodesKey(upgrade, &hkey, TRUE);
        if (r != ERROR_SUCCESS)
            goto done;
    }

    squash_guid(package->ProductCode, squashed_pc);
    msi_reg_set_val_str(hkey, squashed_pc, NULL);

    RegCloseKey(hkey);

done:
    msi_free(upgrade);
    return r;
}

static BOOL msi_check_publish(MSIPACKAGE *package)
{
    MSIFEATURE *feature;

    LIST_FOR_EACH_ENTRY(feature, &package->features, MSIFEATURE, entry)
    {
        if (feature->ActionRequest == INSTALLSTATE_LOCAL)
            return TRUE;
    }

    return FALSE;
}

static BOOL msi_check_unpublish(MSIPACKAGE *package)
{
    MSIFEATURE *feature;

    LIST_FOR_EACH_ENTRY(feature, &package->features, MSIFEATURE, entry)
    {
        if (feature->ActionRequest != INSTALLSTATE_ABSENT)
            return FALSE;
    }

    return TRUE;
}

static UINT msi_publish_patches( MSIPACKAGE *package )
{
    static const WCHAR szAllPatches[] = {'A','l','l','P','a','t','c','h','e','s',0};
    WCHAR patch_squashed[GUID_SIZE];
    HKEY patches_key = NULL, product_patches_key = NULL, product_key;
    LONG res;
    MSIPATCHINFO *patch;
    UINT r;
    WCHAR *p, *all_patches = NULL;
    DWORD len = 0;

    r = MSIREG_OpenProductKey( package->ProductCode, NULL, package->Context, &product_key, TRUE );
    if (r != ERROR_SUCCESS)
        return ERROR_FUNCTION_FAILED;

    res = RegCreateKeyExW( product_key, szPatches, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &patches_key, NULL );
    if (res != ERROR_SUCCESS)
    {
        r = ERROR_FUNCTION_FAILED;
        goto done;
    }

    r = MSIREG_OpenUserDataProductPatchesKey( package->ProductCode, package->Context, &product_patches_key, TRUE );
    if (r != ERROR_SUCCESS)
        goto done;

    LIST_FOR_EACH_ENTRY( patch, &package->patches, MSIPATCHINFO, entry )
    {
        squash_guid( patch->patchcode, patch_squashed );
        len += strlenW( patch_squashed ) + 1;
    }

    p = all_patches = msi_alloc( (len + 1) * sizeof(WCHAR) );
    if (!all_patches)
        goto done;

    LIST_FOR_EACH_ENTRY( patch, &package->patches, MSIPATCHINFO, entry )
    {
        HKEY patch_key;

        squash_guid( patch->patchcode, p );
        p += strlenW( p ) + 1;

        res = RegSetValueExW( patches_key, patch_squashed, 0, REG_SZ,
                              (const BYTE *)patch->transforms,
                              (strlenW(patch->transforms) + 1) * sizeof(WCHAR) );
        if (res != ERROR_SUCCESS)
            goto done;

        r = MSIREG_OpenUserDataPatchKey( patch->patchcode, package->Context, &patch_key, TRUE );
        if (r != ERROR_SUCCESS)
            goto done;

        res = RegSetValueExW( patch_key, szLocalPackage, 0, REG_SZ,
                              (const BYTE *)patch->localfile,
                              (strlenW(patch->localfile) + 1) * sizeof(WCHAR) );
        RegCloseKey( patch_key );
        if (res != ERROR_SUCCESS)
            goto done;

        res = RegCreateKeyExW( product_patches_key, patch_squashed, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &patch_key, NULL );
        if (res != ERROR_SUCCESS)
            goto done;

        res = RegSetValueExW( patch_key, szState, 0, REG_DWORD, (const BYTE *)&patch->state, sizeof(patch->state) );
        RegCloseKey( patch_key );
        if (res != ERROR_SUCCESS)
            goto done;
    }

    all_patches[len] = 0;
    res = RegSetValueExW( patches_key, szPatches, 0, REG_MULTI_SZ,
                          (const BYTE *)all_patches, (len + 1) * sizeof(WCHAR) );
    if (res != ERROR_SUCCESS)
        goto done;

    res = RegSetValueExW( product_patches_key, szAllPatches, 0, REG_MULTI_SZ,
                          (const BYTE *)all_patches, (len + 1) * sizeof(WCHAR) );
    if (res != ERROR_SUCCESS)
        r = ERROR_FUNCTION_FAILED;

done:
    RegCloseKey( product_patches_key );
    RegCloseKey( patches_key );
    RegCloseKey( product_key );
    msi_free( all_patches );
    return r;
}

/*
 * 99% of the work done here is only done for 
 * advertised installs. However this is where the
 * Icon table is processed and written out
 * so that is what I am going to do here.
 */
static UINT ACTION_PublishProduct(MSIPACKAGE *package)
{
    UINT rc;
    HKEY hukey = NULL, hudkey = NULL;
    MSIRECORD *uirow;

    if (!list_empty(&package->patches))
    {
        rc = msi_publish_patches(package);
        if (rc != ERROR_SUCCESS)
            goto end;
    }

    /* FIXME: also need to publish if the product is in advertise mode */
    if (!msi_check_publish(package))
        return ERROR_SUCCESS;

    rc = MSIREG_OpenProductKey(package->ProductCode, NULL, package->Context,
                               &hukey, TRUE);
    if (rc != ERROR_SUCCESS)
        goto end;

    rc = MSIREG_OpenUserDataProductKey(package->ProductCode, package->Context,
                                       NULL, &hudkey, TRUE);
    if (rc != ERROR_SUCCESS)
        goto end;

    rc = msi_publish_upgrade_code(package);
    if (rc != ERROR_SUCCESS)
        goto end;

    rc = msi_publish_product_properties(package, hukey);
    if (rc != ERROR_SUCCESS)
        goto end;

    rc = msi_publish_sourcelist(package, hukey);
    if (rc != ERROR_SUCCESS)
        goto end;

    rc = msi_publish_icons(package);

end:
    uirow = MSI_CreateRecord( 1 );
    MSI_RecordSetStringW( uirow, 1, package->ProductCode );
    ui_actiondata( package, szPublishProduct, uirow );
    msiobj_release( &uirow->hdr );

    RegCloseKey(hukey);
    RegCloseKey(hudkey);

    return rc;
}

static WCHAR *get_ini_file_name( MSIPACKAGE *package, MSIRECORD *row )
{
    WCHAR *filename, *ptr, *folder, *ret;
    const WCHAR *dirprop;

    filename = msi_dup_record_field( row, 2 );
    if (filename && (ptr = strchrW( filename, '|' )))
        ptr++;
    else
        ptr = filename;

    dirprop = MSI_RecordGetString( row, 3 );
    if (dirprop)
    {
        folder = resolve_target_folder( package, dirprop, FALSE, TRUE, NULL );
        if (!folder)
            folder = msi_dup_property( package->db, dirprop );
    }
    else
        folder = msi_dup_property( package->db, szWindowsFolder );

    if (!folder)
    {
        ERR("Unable to resolve folder %s\n", debugstr_w(dirprop));
        msi_free( filename );
        return NULL;
    }

    ret = build_directory_name( 2, folder, ptr );

    msi_free( filename );
    msi_free( folder );
    return ret;
}

static UINT ITERATE_WriteIniValues(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPCWSTR component, section, key, value, identifier;
    LPWSTR deformated_section, deformated_key, deformated_value, fullname;
    MSIRECORD * uirow;
    INT action;
    MSICOMPONENT *comp;

    component = MSI_RecordGetString(row, 8);
    comp = get_loaded_component(package,component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    identifier = MSI_RecordGetString(row,1); 
    section = MSI_RecordGetString(row,4);
    key = MSI_RecordGetString(row,5);
    value = MSI_RecordGetString(row,6);
    action = MSI_RecordGetInteger(row,7);

    deformat_string(package,section,&deformated_section);
    deformat_string(package,key,&deformated_key);
    deformat_string(package,value,&deformated_value);

    fullname = get_ini_file_name(package, row);

    if (action == 0)
    {
        TRACE("Adding value %s to section %s in %s\n",
                debugstr_w(deformated_key), debugstr_w(deformated_section),
                debugstr_w(fullname));
        WritePrivateProfileStringW(deformated_section, deformated_key,
                                   deformated_value, fullname);
    }
    else if (action == 1)
    {
        WCHAR returned[10];
        GetPrivateProfileStringW(deformated_section, deformated_key, NULL,
                                 returned, 10, fullname);
        if (returned[0] == 0)
        {
            TRACE("Adding value %s to section %s in %s\n",
                    debugstr_w(deformated_key), debugstr_w(deformated_section),
                    debugstr_w(fullname));

            WritePrivateProfileStringW(deformated_section, deformated_key,
                                       deformated_value, fullname);
        }
    }
    else if (action == 3)
        FIXME("Append to existing section not yet implemented\n");

    uirow = MSI_CreateRecord(4);
    MSI_RecordSetStringW(uirow,1,identifier);
    MSI_RecordSetStringW(uirow,2,deformated_section);
    MSI_RecordSetStringW(uirow,3,deformated_key);
    MSI_RecordSetStringW(uirow,4,deformated_value);
    ui_actiondata(package,szWriteIniValues,uirow);
    msiobj_release( &uirow->hdr );

    msi_free(fullname);
    msi_free(deformated_key);
    msi_free(deformated_value);
    msi_free(deformated_section);
    return ERROR_SUCCESS;
}

static UINT ACTION_WriteIniValues(MSIPACKAGE *package)
{
    UINT rc;
    MSIQUERY * view;
    static const WCHAR ExecSeqQuery[] = 
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','I','n','i','F','i','l','e','`',0};

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
    {
        TRACE("no IniFile table\n");
        return ERROR_SUCCESS;
    }

    rc = MSI_IterateRecords(view, NULL, ITERATE_WriteIniValues, package);
    msiobj_release(&view->hdr);
    return rc;
}

static UINT ITERATE_RemoveIniValuesOnUninstall( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR component, section, key, value, identifier;
    LPWSTR deformated_section, deformated_key, deformated_value, filename;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    INT action;

    component = MSI_RecordGetString( row, 8 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    identifier = MSI_RecordGetString( row, 1 );
    section = MSI_RecordGetString( row, 4 );
    key = MSI_RecordGetString( row, 5 );
    value = MSI_RecordGetString( row, 6 );
    action = MSI_RecordGetInteger( row, 7 );

    deformat_string( package, section, &deformated_section );
    deformat_string( package, key, &deformated_key );
    deformat_string( package, value, &deformated_value );

    if (action == msidbIniFileActionAddLine || action == msidbIniFileActionCreateLine)
    {
        filename = get_ini_file_name( package, row );

        TRACE("Removing key %s from section %s in %s\n",
               debugstr_w(deformated_key), debugstr_w(deformated_section), debugstr_w(filename));

        if (!WritePrivateProfileStringW( deformated_section, deformated_key, NULL, filename ))
        {
            WARN("Unable to remove key %u\n", GetLastError());
        }
        msi_free( filename );
    }
    else
        FIXME("Unsupported action %d\n", action);


    uirow = MSI_CreateRecord( 4 );
    MSI_RecordSetStringW( uirow, 1, identifier );
    MSI_RecordSetStringW( uirow, 2, deformated_section );
    MSI_RecordSetStringW( uirow, 3, deformated_key );
    MSI_RecordSetStringW( uirow, 4, deformated_value );
    ui_actiondata( package, szRemoveIniValues, uirow );
    msiobj_release( &uirow->hdr );

    msi_free( deformated_key );
    msi_free( deformated_value );
    msi_free( deformated_section );
    return ERROR_SUCCESS;
}

static UINT ITERATE_RemoveIniValuesOnInstall( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR component, section, key, value, identifier;
    LPWSTR deformated_section, deformated_key, deformated_value, filename;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    INT action;

    component = MSI_RecordGetString( row, 8 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    identifier = MSI_RecordGetString( row, 1 );
    section = MSI_RecordGetString( row, 4 );
    key = MSI_RecordGetString( row, 5 );
    value = MSI_RecordGetString( row, 6 );
    action = MSI_RecordGetInteger( row, 7 );

    deformat_string( package, section, &deformated_section );
    deformat_string( package, key, &deformated_key );
    deformat_string( package, value, &deformated_value );

    if (action == msidbIniFileActionRemoveLine)
    {
        filename = get_ini_file_name( package, row );

        TRACE("Removing key %s from section %s in %s\n",
               debugstr_w(deformated_key), debugstr_w(deformated_section), debugstr_w(filename));

        if (!WritePrivateProfileStringW( deformated_section, deformated_key, NULL, filename ))
        {
            WARN("Unable to remove key %u\n", GetLastError());
        }
        msi_free( filename );
    }
    else
        FIXME("Unsupported action %d\n", action);

    uirow = MSI_CreateRecord( 4 );
    MSI_RecordSetStringW( uirow, 1, identifier );
    MSI_RecordSetStringW( uirow, 2, deformated_section );
    MSI_RecordSetStringW( uirow, 3, deformated_key );
    MSI_RecordSetStringW( uirow, 4, deformated_value );
    ui_actiondata( package, szRemoveIniValues, uirow );
    msiobj_release( &uirow->hdr );

    msi_free( deformated_key );
    msi_free( deformated_value );
    msi_free( deformated_section );
    return ERROR_SUCCESS;
}

static UINT ACTION_RemoveIniValues( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','I','n','i','F','i','l','e','`',0};
    static const WCHAR remove_query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','R','e','m','o','v','e','I','n','i','F','i','l','e','`',0};

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc == ERROR_SUCCESS)
    {
        rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveIniValuesOnUninstall, package );
        msiobj_release( &view->hdr );
        if (rc != ERROR_SUCCESS)
            return rc;
    }

    rc = MSI_DatabaseOpenViewW( package->db, remove_query, &view );
    if (rc == ERROR_SUCCESS)
    {
        rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveIniValuesOnInstall, package );
        msiobj_release( &view->hdr );
        if (rc != ERROR_SUCCESS)
            return rc;
    }

    return ERROR_SUCCESS;
}

static void register_dll( const WCHAR *dll, BOOL unregister )
{
    HMODULE hmod;

    hmod = LoadLibraryExW( dll, 0, LOAD_WITH_ALTERED_SEARCH_PATH );
    if (hmod)
    {
        HRESULT (WINAPI *func_ptr)( void );
        const char *func = unregister ? "DllUnregisterServer" : "DllRegisterServer";

        func_ptr = (void *)GetProcAddress( hmod, func );
        if (func_ptr)
        {
            HRESULT hr = func_ptr();
            if (FAILED( hr ))
                WARN("failed to register dll 0x%08x\n", hr);
        }
        else
            WARN("entry point %s not found\n", func);
        FreeLibrary( hmod );
        return;
    }
    WARN("failed to load library %u\n", GetLastError());
}

static UINT ITERATE_SelfRegModules(MSIRECORD *row, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPCWSTR filename;
    MSIFILE *file;
    MSIRECORD *uirow;

    filename = MSI_RecordGetString(row,1);
    file = get_loaded_file( package, filename );

    if (!file)
    {
        ERR("Unable to find file id %s\n",debugstr_w(filename));
        return ERROR_SUCCESS;
    }

    TRACE("Registering %s\n", debugstr_w( file->TargetPath ));

    register_dll( file->TargetPath, FALSE );

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, filename );
    MSI_RecordSetStringW( uirow, 2, file->Component->Directory );
    ui_actiondata( package, szSelfRegModules, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_SelfRegModules(MSIPACKAGE *package)
{
    UINT rc;
    MSIQUERY * view;
    static const WCHAR ExecSeqQuery[] = 
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','S','e','l','f','R','e','g','`',0};

    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
    {
        TRACE("no SelfReg table\n");
        return ERROR_SUCCESS;
    }

    MSI_IterateRecords(view, NULL, ITERATE_SelfRegModules, package);
    msiobj_release(&view->hdr);

    return ERROR_SUCCESS;
}

static UINT ITERATE_SelfUnregModules( MSIRECORD *row, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR filename;
    MSIFILE *file;
    MSIRECORD *uirow;

    filename = MSI_RecordGetString( row, 1 );
    file = get_loaded_file( package, filename );

    if (!file)
    {
        ERR("Unable to find file id %s\n", debugstr_w(filename));
        return ERROR_SUCCESS;
    }

    TRACE("Unregistering %s\n", debugstr_w( file->TargetPath ));

    register_dll( file->TargetPath, TRUE );

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, filename );
    MSI_RecordSetStringW( uirow, 2, file->Component->Directory );
    ui_actiondata( package, szSelfUnregModules, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_SelfUnregModules( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','S','e','l','f','R','e','g','`',0};

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
    {
        TRACE("no SelfReg table\n");
        return ERROR_SUCCESS;
    }

    MSI_IterateRecords( view, NULL, ITERATE_SelfUnregModules, package );
    msiobj_release( &view->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_PublishFeatures(MSIPACKAGE *package)
{
    MSIFEATURE *feature;
    UINT rc;
    HKEY hkey = NULL, userdata = NULL;

    if (!msi_check_publish(package))
        return ERROR_SUCCESS;

    rc = MSIREG_OpenFeaturesKey(package->ProductCode, package->Context,
                                &hkey, TRUE);
    if (rc != ERROR_SUCCESS)
        goto end;

    rc = MSIREG_OpenUserDataFeaturesKey(package->ProductCode, package->Context,
                                        &userdata, TRUE);
    if (rc != ERROR_SUCCESS)
        goto end;

    /* here the guids are base 85 encoded */
    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        ComponentList *cl;
        LPWSTR data = NULL;
        GUID clsid;
        INT size;
        BOOL absent = FALSE;
        MSIRECORD *uirow;

        if (feature->ActionRequest != INSTALLSTATE_LOCAL &&
            feature->ActionRequest != INSTALLSTATE_SOURCE &&
            feature->ActionRequest != INSTALLSTATE_ADVERTISED) absent = TRUE;

        size = 1;
        LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
        {
            size += 21;
        }
        if (feature->Feature_Parent)
            size += strlenW( feature->Feature_Parent )+2;

        data = msi_alloc(size * sizeof(WCHAR));

        data[0] = 0;
        LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
        {
            MSICOMPONENT* component = cl->component;
            WCHAR buf[21];

            buf[0] = 0;
            if (component->ComponentId)
            {
                TRACE("From %s\n",debugstr_w(component->ComponentId));
                CLSIDFromString(component->ComponentId, &clsid);
                encode_base85_guid(&clsid,buf);
                TRACE("to %s\n",debugstr_w(buf));
                strcatW(data,buf);
            }
        }

        if (feature->Feature_Parent)
        {
            static const WCHAR sep[] = {'\2',0};
            strcatW(data,sep);
            strcatW(data,feature->Feature_Parent);
        }

        msi_reg_set_val_str( userdata, feature->Feature, data );
        msi_free(data);

        size = 0;
        if (feature->Feature_Parent)
            size = strlenW(feature->Feature_Parent)*sizeof(WCHAR);
        if (!absent)
        {
            size += sizeof(WCHAR);
            RegSetValueExW(hkey,feature->Feature,0,REG_SZ,
                           (const BYTE*)(feature->Feature_Parent ? feature->Feature_Parent : szEmpty),size);
        }
        else
        {
            size += 2*sizeof(WCHAR);
            data = msi_alloc(size);
            data[0] = 0x6;
            data[1] = 0;
            if (feature->Feature_Parent)
                strcpyW( &data[1], feature->Feature_Parent );
            RegSetValueExW(hkey,feature->Feature,0,REG_SZ,
                       (LPBYTE)data,size);
            msi_free(data);
        }

        /* the UI chunk */
        uirow = MSI_CreateRecord( 1 );
        MSI_RecordSetStringW( uirow, 1, feature->Feature );
        ui_actiondata( package, szPublishFeatures, uirow);
        msiobj_release( &uirow->hdr );
        /* FIXME: call ui_progress? */
    }

end:
    RegCloseKey(hkey);
    RegCloseKey(userdata);
    return rc;
}

static UINT msi_unpublish_feature(MSIPACKAGE *package, MSIFEATURE *feature)
{
    UINT r;
    HKEY hkey;
    MSIRECORD *uirow;

    TRACE("unpublishing feature %s\n", debugstr_w(feature->Feature));

    r = MSIREG_OpenFeaturesKey(package->ProductCode, package->Context,
                               &hkey, FALSE);
    if (r == ERROR_SUCCESS)
    {
        RegDeleteValueW(hkey, feature->Feature);
        RegCloseKey(hkey);
    }

    r = MSIREG_OpenUserDataFeaturesKey(package->ProductCode, package->Context,
                                       &hkey, FALSE);
    if (r == ERROR_SUCCESS)
    {
        RegDeleteValueW(hkey, feature->Feature);
        RegCloseKey(hkey);
    }

    uirow = MSI_CreateRecord( 1 );
    MSI_RecordSetStringW( uirow, 1, feature->Feature );
    ui_actiondata( package, szUnpublishFeatures, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_UnpublishFeatures(MSIPACKAGE *package)
{
    MSIFEATURE *feature;

    if (!msi_check_unpublish(package))
        return ERROR_SUCCESS;

    LIST_FOR_EACH_ENTRY(feature, &package->features, MSIFEATURE, entry)
    {
        msi_unpublish_feature(package, feature);
    }

    return ERROR_SUCCESS;
}

static UINT msi_publish_install_properties(MSIPACKAGE *package, HKEY hkey)
{
    SYSTEMTIME systime;
    DWORD size, langid;
    WCHAR date[9], *val, *buffer;
    const WCHAR *prop, *key;

    static const WCHAR date_fmt[] = {'%','i','%','0','2','i','%','0','2','i',0};
    static const WCHAR szWindowsInstaller[] =
        {'W','i','n','d','o','w','s','I','n','s','t','a','l','l','e','r',0};
    static const WCHAR modpath_fmt[] =
        {'M','s','i','E','x','e','c','.','e','x','e',' ',
         '/','I','[','P','r','o','d','u','c','t','C','o','d','e',']',0};
    static const WCHAR szModifyPath[] =
        {'M','o','d','i','f','y','P','a','t','h',0};
    static const WCHAR szUninstallString[] =
        {'U','n','i','n','s','t','a','l','l','S','t','r','i','n','g',0};
    static const WCHAR szEstimatedSize[] =
        {'E','s','t','i','m','a','t','e','d','S','i','z','e',0};
    static const WCHAR szProductLanguage[] =
        {'P','r','o','d','u','c','t','L','a','n','g','u','a','g','e',0};
    static const WCHAR szProductVersion[] =
        {'P','r','o','d','u','c','t','V','e','r','s','i','o','n',0};
    static const WCHAR szDisplayVersion[] =
        {'D','i','s','p','l','a','y','V','e','r','s','i','o','n',0};
    static const WCHAR szInstallSource[] =
        {'I','n','s','t','a','l','l','S','o','u','r','c','e',0};
    static const WCHAR szARPAUTHORIZEDCDFPREFIX[] =
        {'A','R','P','A','U','T','H','O','R','I','Z','E','D','C','D','F','P','R','E','F','I','X',0};
    static const WCHAR szAuthorizedCDFPrefix[] =
        {'A','u','t','h','o','r','i','z','e','d','C','D','F','P','r','e','f','i','x',0};
    static const WCHAR szARPCONTACT[] =
        {'A','R','P','C','O','N','T','A','C','T',0};
    static const WCHAR szContact[] =
        {'C','o','n','t','a','c','t',0};
    static const WCHAR szARPCOMMENTS[] =
        {'A','R','P','C','O','M','M','E','N','T','S',0};
    static const WCHAR szComments[] =
        {'C','o','m','m','e','n','t','s',0};
    static const WCHAR szProductName[] =
        {'P','r','o','d','u','c','t','N','a','m','e',0};
    static const WCHAR szDisplayName[] =
        {'D','i','s','p','l','a','y','N','a','m','e',0};
    static const WCHAR szARPHELPLINK[] =
        {'A','R','P','H','E','L','P','L','I','N','K',0};
    static const WCHAR szHelpLink[] =
        {'H','e','l','p','L','i','n','k',0};
    static const WCHAR szARPHELPTELEPHONE[] =
        {'A','R','P','H','E','L','P','T','E','L','E','P','H','O','N','E',0};
    static const WCHAR szHelpTelephone[] =
        {'H','e','l','p','T','e','l','e','p','h','o','n','e',0};
    static const WCHAR szARPINSTALLLOCATION[] =
        {'A','R','P','I','N','S','T','A','L','L','L','O','C','A','T','I','O','N',0};
    static const WCHAR szInstallLocation[] =
        {'I','n','s','t','a','l','l','L','o','c','a','t','i','o','n',0};
    static const WCHAR szManufacturer[] =
        {'M','a','n','u','f','a','c','t','u','r','e','r',0};
    static const WCHAR szPublisher[] =
        {'P','u','b','l','i','s','h','e','r',0};
    static const WCHAR szARPREADME[] =
        {'A','R','P','R','E','A','D','M','E',0};
    static const WCHAR szReadme[] =
        {'R','e','a','d','M','e',0};
    static const WCHAR szARPSIZE[] =
        {'A','R','P','S','I','Z','E',0};
    static const WCHAR szSize[] =
        {'S','i','z','e',0};
    static const WCHAR szARPURLINFOABOUT[] =
        {'A','R','P','U','R','L','I','N','F','O','A','B','O','U','T',0};
    static const WCHAR szURLInfoAbout[] =
        {'U','R','L','I','n','f','o','A','b','o','u','t',0};
    static const WCHAR szARPURLUPDATEINFO[] =
        {'A','R','P','U','R','L','U','P','D','A','T','E','I','N','F','O',0};
    static const WCHAR szURLUpdateInfo[] =
        {'U','R','L','U','p','d','a','t','e','I','n','f','o',0};

    static const WCHAR *propval[] = {
        szARPAUTHORIZEDCDFPREFIX, szAuthorizedCDFPrefix,
        szARPCONTACT,             szContact,
        szARPCOMMENTS,            szComments,
        szProductName,            szDisplayName,
        szARPHELPLINK,            szHelpLink,
        szARPHELPTELEPHONE,       szHelpTelephone,
        szARPINSTALLLOCATION,     szInstallLocation,
        cszSourceDir,             szInstallSource,
        szManufacturer,           szPublisher,
        szARPREADME,              szReadme,
        szARPSIZE,                szSize,
        szARPURLINFOABOUT,        szURLInfoAbout,
        szARPURLUPDATEINFO,       szURLUpdateInfo,
        NULL
    };
    const WCHAR **p = propval;

    while (*p)
    {
        prop = *p++;
        key = *p++;
        val = msi_dup_property(package->db, prop);
        msi_reg_set_val_str(hkey, key, val);
        msi_free(val);
    }

    msi_reg_set_val_dword(hkey, szWindowsInstaller, 1);

    size = deformat_string(package, modpath_fmt, &buffer);
    RegSetValueExW(hkey, szModifyPath, 0, REG_EXPAND_SZ, (LPBYTE)buffer, size);
    RegSetValueExW(hkey, szUninstallString, 0, REG_EXPAND_SZ, (LPBYTE)buffer, size);
    msi_free(buffer);

    /* FIXME: Write real Estimated Size when we have it */
    msi_reg_set_val_dword(hkey, szEstimatedSize, 0);

    GetLocalTime(&systime);
    sprintfW(date, date_fmt, systime.wYear, systime.wMonth, systime.wDay);
    msi_reg_set_val_str(hkey, INSTALLPROPERTY_INSTALLDATEW, date);

    langid = msi_get_property_int(package->db, szProductLanguage, 0);
    msi_reg_set_val_dword(hkey, INSTALLPROPERTY_LANGUAGEW, langid);

    buffer = msi_dup_property(package->db, szProductVersion);
    msi_reg_set_val_str(hkey, szDisplayVersion, buffer);
    if (buffer)
    {
        DWORD verdword = msi_version_str_to_dword(buffer);

        msi_reg_set_val_dword(hkey, INSTALLPROPERTY_VERSIONW, verdword);
        msi_reg_set_val_dword(hkey, INSTALLPROPERTY_VERSIONMAJORW, verdword >> 24);
        msi_reg_set_val_dword(hkey, INSTALLPROPERTY_VERSIONMINORW, (verdword >> 16) & 0xFF);
        msi_free(buffer);
    }

    return ERROR_SUCCESS;
}

static UINT ACTION_RegisterProduct(MSIPACKAGE *package)
{
    WCHAR squashed_pc[SQUISH_GUID_SIZE];
    MSIRECORD *uirow;
    LPWSTR upgrade_code;
    HKEY hkey, props;
    HKEY upgrade;
    UINT rc;

    /* FIXME: also need to publish if the product is in advertise mode */
    if (!msi_check_publish(package))
        return ERROR_SUCCESS;

    rc = MSIREG_OpenUninstallKey(package, &hkey, TRUE);
    if (rc != ERROR_SUCCESS)
        return rc;

    rc = MSIREG_OpenInstallProps(package->ProductCode, package->Context,
                                 NULL, &props, TRUE);
    if (rc != ERROR_SUCCESS)
        goto done;

    msi_reg_set_val_str( props, INSTALLPROPERTY_LOCALPACKAGEW, package->db->localfile );
    msi_free( package->db->localfile );
    package->db->localfile = NULL;

    rc = msi_publish_install_properties(package, hkey);
    if (rc != ERROR_SUCCESS)
        goto done;

    rc = msi_publish_install_properties(package, props);
    if (rc != ERROR_SUCCESS)
        goto done;

    upgrade_code = msi_dup_property(package->db, szUpgradeCode);
    if (upgrade_code)
    {
        MSIREG_OpenUpgradeCodesKey(upgrade_code, &upgrade, TRUE);
        squash_guid(package->ProductCode, squashed_pc);
        msi_reg_set_val_str(upgrade, squashed_pc, NULL);
        RegCloseKey(upgrade);
        msi_free(upgrade_code);
    }

done:
    uirow = MSI_CreateRecord( 1 );
    MSI_RecordSetStringW( uirow, 1, package->ProductCode );
    ui_actiondata( package, szRegisterProduct, uirow );
    msiobj_release( &uirow->hdr );

    RegCloseKey(hkey);
    return ERROR_SUCCESS;
}

static UINT ACTION_InstallExecute(MSIPACKAGE *package)
{
    return execute_script(package,INSTALL_SCRIPT);
}

static UINT msi_unpublish_product(MSIPACKAGE *package, WCHAR *remove)
{
    WCHAR *upgrade, **features;
    BOOL full_uninstall = TRUE;
    MSIFEATURE *feature;
    MSIPATCHINFO *patch;

    static const WCHAR szUpgradeCode[] =
        {'U','p','g','r','a','d','e','C','o','d','e',0};

    features = msi_split_string(remove, ',');
    if (!features)
    {
        ERR("REMOVE feature list is empty!\n");
        return ERROR_FUNCTION_FAILED;
    }

    if (!strcmpW( features[0], szAll ))
        full_uninstall = TRUE;
    else
    {
        LIST_FOR_EACH_ENTRY(feature, &package->features, MSIFEATURE, entry)
        {
            if (feature->Action != INSTALLSTATE_ABSENT)
                full_uninstall = FALSE;
        }
    }
    msi_free(features);

    if (!full_uninstall)
        return ERROR_SUCCESS;

    MSIREG_DeleteProductKey(package->ProductCode);
    MSIREG_DeleteUserDataProductKey(package->ProductCode);
    MSIREG_DeleteUninstallKey(package);

    MSIREG_DeleteLocalClassesProductKey(package->ProductCode);
    MSIREG_DeleteLocalClassesFeaturesKey(package->ProductCode);
    MSIREG_DeleteUserProductKey(package->ProductCode);
    MSIREG_DeleteUserFeaturesKey(package->ProductCode);

    upgrade = msi_dup_property(package->db, szUpgradeCode);
    if (upgrade)
    {
        MSIREG_DeleteUserUpgradeCodesKey(upgrade);
        MSIREG_DeleteClassesUpgradeCodesKey(upgrade);
        msi_free(upgrade);
    }

    LIST_FOR_EACH_ENTRY(patch, &package->patches, MSIPATCHINFO, entry)
    {
        MSIREG_DeleteUserDataPatchKey(patch->patchcode, package->Context);
    }

    return ERROR_SUCCESS;
}

static UINT ACTION_InstallFinalize(MSIPACKAGE *package)
{
    UINT rc;
    WCHAR *remove;

    /* turn off scheduling */
    package->script->CurrentlyScripting= FALSE;

    /* first do the same as an InstallExecute */
    rc = ACTION_InstallExecute(package);
    if (rc != ERROR_SUCCESS)
        return rc;

    /* then handle Commit Actions */
    rc = execute_script(package,COMMIT_SCRIPT);
    if (rc != ERROR_SUCCESS)
        return rc;

    remove = msi_dup_property(package->db, szRemove);
    if (remove)
        rc = msi_unpublish_product(package, remove);

    msi_free(remove);
    return rc;
}

UINT ACTION_ForceReboot(MSIPACKAGE *package)
{
    static const WCHAR RunOnce[] = {
    'S','o','f','t','w','a','r','e','\\',
    'M','i','c','r','o','s','o','f','t','\\',
    'W','i','n','d','o','w','s','\\',
    'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
    'R','u','n','O','n','c','e',0};
    static const WCHAR InstallRunOnce[] = {
    'S','o','f','t','w','a','r','e','\\',
    'M','i','c','r','o','s','o','f','t','\\',
    'W','i','n','d','o','w','s','\\',
    'C','u','r','r','e','n','t','V','e','r','s','i','o','n','\\',
    'I','n','s','t','a','l','l','e','r','\\',
    'R','u','n','O','n','c','e','E','n','t','r','i','e','s',0};

    static const WCHAR msiexec_fmt[] = {
    '%','s',
    '\\','M','s','i','E','x','e','c','.','e','x','e',' ','/','@',' ',
    '\"','%','s','\"',0};
    static const WCHAR install_fmt[] = {
    '/','I',' ','\"','%','s','\"',' ',
    'A','F','T','E','R','R','E','B','O','O','T','=','1',' ',
    'R','U','N','O','N','C','E','E','N','T','R','Y','=','\"','%','s','\"',0};
    WCHAR buffer[256], sysdir[MAX_PATH];
    HKEY hkey;
    WCHAR squished_pc[100];

    squash_guid(package->ProductCode,squished_pc);

    GetSystemDirectoryW(sysdir, sizeof(sysdir)/sizeof(sysdir[0]));
    RegCreateKeyW(HKEY_LOCAL_MACHINE,RunOnce,&hkey);
    snprintfW(buffer,sizeof(buffer)/sizeof(buffer[0]),msiexec_fmt,sysdir,
     squished_pc);

    msi_reg_set_val_str( hkey, squished_pc, buffer );
    RegCloseKey(hkey);

    TRACE("Reboot command %s\n",debugstr_w(buffer));

    RegCreateKeyW(HKEY_LOCAL_MACHINE,InstallRunOnce,&hkey);
    sprintfW(buffer,install_fmt,package->ProductCode,squished_pc);

    msi_reg_set_val_str( hkey, squished_pc, buffer );
    RegCloseKey(hkey);

    return ERROR_INSTALL_SUSPEND;
}

static UINT ACTION_ResolveSource(MSIPACKAGE* package)
{
    DWORD attrib;
    UINT rc;

    /*
     * We are currently doing what should be done here in the top level Install
     * however for Administrative and uninstalls this step will be needed
     */
    if (!package->PackagePath)
        return ERROR_SUCCESS;

    msi_set_sourcedir_props(package, TRUE);

    attrib = GetFileAttributesW(package->db->path);
    if (attrib == INVALID_FILE_ATTRIBUTES)
    {
        LPWSTR prompt;
        LPWSTR msg;
        DWORD size = 0;

        rc = MsiSourceListGetInfoW(package->ProductCode, NULL, 
                package->Context, MSICODE_PRODUCT,
                INSTALLPROPERTY_DISKPROMPTW,NULL,&size);
        if (rc == ERROR_MORE_DATA)
        {
            prompt = msi_alloc(size * sizeof(WCHAR));
            MsiSourceListGetInfoW(package->ProductCode, NULL, 
                    package->Context, MSICODE_PRODUCT,
                    INSTALLPROPERTY_DISKPROMPTW,prompt,&size);
        }
        else
            prompt = strdupW(package->db->path);

        msg = generate_error_string(package,1302,1,prompt);
        while(attrib == INVALID_FILE_ATTRIBUTES)
        {
            rc = MessageBoxW(NULL,msg,NULL,MB_OKCANCEL);
            if (rc == IDCANCEL)
            {
                rc = ERROR_INSTALL_USEREXIT;
                break;
            }
            attrib = GetFileAttributesW(package->db->path);
        }
        msi_free(prompt);
        rc = ERROR_SUCCESS;
    }
    else
        return ERROR_SUCCESS;

    return rc;
}

static UINT ACTION_RegisterUser(MSIPACKAGE *package)
{
    HKEY hkey = 0;
    LPWSTR buffer, productid = NULL;
    UINT i, rc = ERROR_SUCCESS;
    MSIRECORD *uirow;

    static const WCHAR szPropKeys[][80] = 
    {
        {'P','r','o','d','u','c','t','I','D',0},
        {'U','S','E','R','N','A','M','E',0},
        {'C','O','M','P','A','N','Y','N','A','M','E',0},
        {0},
    };

    static const WCHAR szRegKeys[][80] = 
    {
        {'P','r','o','d','u','c','t','I','D',0},
        {'R','e','g','O','w','n','e','r',0},
        {'R','e','g','C','o','m','p','a','n','y',0},
        {0},
    };

    if (msi_check_unpublish(package))
    {
        MSIREG_DeleteUserDataProductKey(package->ProductCode);
        goto end;
    }

    productid = msi_dup_property( package->db, INSTALLPROPERTY_PRODUCTIDW );
    if (!productid)
        goto end;

    rc = MSIREG_OpenInstallProps(package->ProductCode, package->Context,
                                 NULL, &hkey, TRUE);
    if (rc != ERROR_SUCCESS)
        goto end;

    for( i = 0; szPropKeys[i][0]; i++ )
    {
        buffer = msi_dup_property( package->db, szPropKeys[i] );
        msi_reg_set_val_str( hkey, szRegKeys[i], buffer );
        msi_free( buffer );
    }

end:
    uirow = MSI_CreateRecord( 1 );
    MSI_RecordSetStringW( uirow, 1, productid );
    ui_actiondata( package, szRegisterUser, uirow );
    msiobj_release( &uirow->hdr );

    msi_free(productid);
    RegCloseKey(hkey);
    return rc;
}


static UINT ACTION_ExecuteAction(MSIPACKAGE *package)
{
    UINT rc;

    package->script->InWhatSequence |= SEQUENCE_EXEC;
    rc = ACTION_ProcessExecSequence(package,FALSE);
    return rc;
}


static UINT ITERATE_PublishComponent(MSIRECORD *rec, LPVOID param)
{
    MSIPACKAGE *package = param;
    LPCWSTR compgroupid, component, feature, qualifier, text;
    LPWSTR advertise = NULL, output = NULL, existing = NULL, p, q;
    HKEY hkey = NULL;
    UINT rc;
    MSICOMPONENT *comp;
    MSIFEATURE *feat;
    DWORD sz;
    MSIRECORD *uirow;
    int len;

    feature = MSI_RecordGetString(rec, 5);
    feat = get_loaded_feature(package, feature);
    if (!feat)
        return ERROR_SUCCESS;

    if (feat->ActionRequest != INSTALLSTATE_LOCAL &&
        feat->ActionRequest != INSTALLSTATE_SOURCE &&
        feat->ActionRequest != INSTALLSTATE_ADVERTISED)
    {
        TRACE("Feature %s not scheduled for installation\n", debugstr_w(feature));
        feat->Action = feat->Installed;
        return ERROR_SUCCESS;
    }

    component = MSI_RecordGetString(rec, 3);
    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    compgroupid = MSI_RecordGetString(rec,1);
    qualifier = MSI_RecordGetString(rec,2);

    rc = MSIREG_OpenUserComponentsKey(compgroupid, &hkey, TRUE);
    if (rc != ERROR_SUCCESS)
        goto end;

    advertise = create_component_advertise_string( package, comp, feature );
    text = MSI_RecordGetString( rec, 4 );
    if (text)
    {
        p = msi_alloc( (strlenW( advertise ) + strlenW( text ) + 1) * sizeof(WCHAR) );
        strcpyW( p, advertise );
        strcatW( p, text );
        msi_free( advertise );
        advertise = p;
    }
    existing = msi_reg_get_val_str( hkey, qualifier );

    sz = strlenW( advertise ) + 1;
    if (existing)
    {
        for (p = existing; *p; p += len)
        {
            len = strlenW( p ) + 1;
            if (strcmpW( advertise, p )) sz += len;
        }
    }
    if (!(output = msi_alloc( (sz + 1) * sizeof(WCHAR) )))
    {
        rc = ERROR_OUTOFMEMORY;
        goto end;
    }
    q = output;
    if (existing)
    {
        for (p = existing; *p; p += len)
        {
            len = strlenW( p ) + 1;
            if (strcmpW( advertise, p ))
            {
                memcpy( q, p, len * sizeof(WCHAR) );
                q += len;
            }
        }
    }
    strcpyW( q, advertise );
    q[strlenW( q ) + 1] = 0;

    msi_reg_set_val_multi_str( hkey, qualifier, output );
    
end:
    RegCloseKey(hkey);
    msi_free( output );
    msi_free( advertise );
    msi_free( existing );

    /* the UI chunk */
    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, compgroupid );
    MSI_RecordSetStringW( uirow, 2, qualifier);
    ui_actiondata( package, szPublishComponents, uirow);
    msiobj_release( &uirow->hdr );
    /* FIXME: call ui_progress? */

    return rc;
}

/*
 * At present I am ignorning the advertised components part of this and only
 * focusing on the qualified component sets
 */
static UINT ACTION_PublishComponents(MSIPACKAGE *package)
{
    UINT rc;
    MSIQUERY * view;
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','P','u','b','l','i','s','h',
         'C','o','m','p','o','n','e','n','t','`',0};
    
    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_PublishComponent, package);
    msiobj_release(&view->hdr);

    return rc;
}

static UINT ITERATE_UnpublishComponent( MSIRECORD *rec, LPVOID param )
{
    static const WCHAR szInstallerComponents[] = {
        'S','o','f','t','w','a','r','e','\\',
        'M','i','c','r','o','s','o','f','t','\\',
        'I','n','s','t','a','l','l','e','r','\\',
        'C','o','m','p','o','n','e','n','t','s','\\',0};

    MSIPACKAGE *package = param;
    LPCWSTR compgroupid, component, feature, qualifier;
    MSICOMPONENT *comp;
    MSIFEATURE *feat;
    MSIRECORD *uirow;
    WCHAR squashed[GUID_SIZE], keypath[MAX_PATH];
    LONG res;

    feature = MSI_RecordGetString( rec, 5 );
    feat = get_loaded_feature( package, feature );
    if (!feat)
        return ERROR_SUCCESS;

    if (feat->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Feature %s not scheduled for removal\n", debugstr_w(feature));
        feat->Action = feat->Installed;
        return ERROR_SUCCESS;
    }

    component = MSI_RecordGetString( rec, 3 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    compgroupid = MSI_RecordGetString( rec, 1 );
    qualifier = MSI_RecordGetString( rec, 2 );

    squash_guid( compgroupid, squashed );
    strcpyW( keypath, szInstallerComponents );
    strcatW( keypath, squashed );

    res = RegDeleteKeyW( HKEY_CURRENT_USER, keypath );
    if (res != ERROR_SUCCESS)
    {
        WARN("Unable to delete component key %d\n", res);
    }

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, compgroupid );
    MSI_RecordSetStringW( uirow, 2, qualifier );
    ui_actiondata( package, szUnpublishComponents, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_UnpublishComponents( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','P','u','b','l','i','s','h',
         'C','o','m','p','o','n','e','n','t','`',0};

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_UnpublishComponent, package );
    msiobj_release( &view->hdr );

    return rc;
}

static UINT ITERATE_InstallService(MSIRECORD *rec, LPVOID param)
{
    MSIPACKAGE *package = param;
    MSIRECORD *row;
    MSIFILE *file;
    SC_HANDLE hscm, service = NULL;
    LPCWSTR comp, key;
    LPWSTR name = NULL, disp = NULL, load_order = NULL, serv_name = NULL;
    LPWSTR depends = NULL, pass = NULL, args = NULL, image_path = NULL;
    DWORD serv_type, start_type, err_control;
    SERVICE_DESCRIPTIONW sd = {NULL};

    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R', 'O','M',' ',
         '`','C','o','m','p','o','n','e','n','t','`',' ',
         'W','H','E','R','E',' ',
         '`','C','o','m','p','o','n','e','n','t','`',' ',
         '=','\'','%','s','\'',0};

    hscm = OpenSCManagerW(NULL, SERVICES_ACTIVE_DATABASEW, GENERIC_WRITE);
    if (!hscm)
    {
        ERR("Failed to open the SC Manager!\n");
        goto done;
    }

    comp = MSI_RecordGetString( rec, 12 );
    if (!get_loaded_component( package, comp ))
        goto done;

    start_type = MSI_RecordGetInteger(rec, 5);
    if (start_type == SERVICE_BOOT_START || start_type == SERVICE_SYSTEM_START)
        goto done;

    deformat_string(package, MSI_RecordGetString(rec, 2), &name);
    deformat_string(package, MSI_RecordGetString(rec, 3), &disp);
    serv_type = MSI_RecordGetInteger(rec, 4);
    err_control = MSI_RecordGetInteger(rec, 6);
    deformat_string(package, MSI_RecordGetString(rec, 7), &load_order);
    deformat_string(package, MSI_RecordGetString(rec, 8), &depends);
    deformat_string(package, MSI_RecordGetString(rec, 9), &serv_name);
    deformat_string(package, MSI_RecordGetString(rec, 10), &pass);
    deformat_string(package, MSI_RecordGetString(rec, 11), &args);
    deformat_string(package, MSI_RecordGetString(rec, 13), &sd.lpDescription);

    /* fetch the service path */
    row = MSI_QueryGetRecord(package->db, query, comp);
    if (!row)
    {
        ERR("Control query failed!\n");
        goto done;
    }
    key = MSI_RecordGetString(row, 6);

    file = get_loaded_file(package, key);
    msiobj_release(&row->hdr);
    if (!file)
    {
        ERR("Failed to load the service file\n");
        goto done;
    }

    if (!args || !args[0]) image_path = file->TargetPath;
    else
    {
        int len = strlenW(file->TargetPath) + strlenW(args) + 2;
        if (!(image_path = msi_alloc(len * sizeof(WCHAR))))
            return ERROR_OUTOFMEMORY;

        strcpyW(image_path, file->TargetPath);
        strcatW(image_path, szSpace);
        strcatW(image_path, args);
    }
    service = CreateServiceW(hscm, name, disp, GENERIC_ALL, serv_type,
                             start_type, err_control, image_path, load_order,
                             NULL, depends, serv_name, pass);

    if (!service)
    {
        if (GetLastError() != ERROR_SERVICE_EXISTS)
            ERR("Failed to create service %s: %d\n", debugstr_w(name), GetLastError());
    }
    else if (sd.lpDescription)
    {
        if (!ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &sd))
            WARN("failed to set service description %u\n", GetLastError());
    }

    if (image_path != file->TargetPath) msi_free(image_path);
done:
    CloseServiceHandle(service);
    CloseServiceHandle(hscm);
    msi_free(name);
    msi_free(disp);
    msi_free(sd.lpDescription);
    msi_free(load_order);
    msi_free(serv_name);
    msi_free(pass);
    msi_free(depends);
    msi_free(args);

    return ERROR_SUCCESS;
}

static UINT ACTION_InstallServices( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY * view;
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         'S','e','r','v','i','c','e','I','n','s','t','a','l','l',0};
    
    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_InstallService, package);
    msiobj_release(&view->hdr);

    return rc;
}

/* converts arg1[~]arg2[~]arg3 to a list of ptrs to the strings */
static LPCWSTR *msi_service_args_to_vector(LPWSTR args, DWORD *numargs)
{
    LPCWSTR *vector, *temp_vector;
    LPWSTR p, q;
    DWORD sep_len;

    static const WCHAR separator[] = {'[','~',']',0};

    *numargs = 0;
    sep_len = sizeof(separator) / sizeof(WCHAR) - 1;

    if (!args)
        return NULL;

    vector = msi_alloc(sizeof(LPWSTR));
    if (!vector)
        return NULL;

    p = args;
    do
    {
        (*numargs)++;
        vector[*numargs - 1] = p;

        if ((q = strstrW(p, separator)))
        {
            *q = '\0';

            temp_vector = msi_realloc(vector, (*numargs + 1) * sizeof(LPWSTR));
            if (!temp_vector)
            {
                msi_free(vector);
                return NULL;
            }
            vector = temp_vector;

            p = q + sep_len;
        }
    } while (q);

    return vector;
}

static UINT ITERATE_StartService(MSIRECORD *rec, LPVOID param)
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    SC_HANDLE scm = NULL, service = NULL;
    LPCWSTR component, *vector = NULL;
    LPWSTR name, args, display_name = NULL;
    DWORD event, numargs, len;
    UINT r = ERROR_FUNCTION_FAILED;

    component = MSI_RecordGetString(rec, 6);
    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    deformat_string(package, MSI_RecordGetString(rec, 2), &name);
    deformat_string(package, MSI_RecordGetString(rec, 4), &args);
    event = MSI_RecordGetInteger(rec, 3);

    if (!(event & msidbServiceControlEventStart))
    {
        r = ERROR_SUCCESS;
        goto done;
    }

    scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm)
    {
        ERR("Failed to open the service control manager\n");
        goto done;
    }

    len = 0;
    if (!GetServiceDisplayNameW( scm, name, NULL, &len ) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        if ((display_name = msi_alloc( ++len * sizeof(WCHAR ))))
            GetServiceDisplayNameW( scm, name, display_name, &len );
    }

    service = OpenServiceW(scm, name, SERVICE_START);
    if (!service)
    {
        ERR("Failed to open service %s (%u)\n", debugstr_w(name), GetLastError());
        goto done;
    }

    vector = msi_service_args_to_vector(args, &numargs);

    if (!StartServiceW(service, numargs, vector) &&
        GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
    {
        ERR("Failed to start service %s (%u)\n", debugstr_w(name), GetLastError());
        goto done;
    }

    r = ERROR_SUCCESS;

done:
    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, display_name );
    MSI_RecordSetStringW( uirow, 2, name );
    ui_actiondata( package, szStartServices, uirow );
    msiobj_release( &uirow->hdr );

    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    msi_free(name);
    msi_free(args);
    msi_free(vector);
    msi_free(display_name);
    return r;
}

static UINT ACTION_StartServices( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;

    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'S','e','r','v','i','c','e','C','o','n','t','r','o','l',0 };

    rc = MSI_DatabaseOpenViewW(package->db, query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_StartService, package);
    msiobj_release(&view->hdr);

    return rc;
}

static BOOL stop_service_dependents(SC_HANDLE scm, SC_HANDLE service)
{
    DWORD i, needed, count;
    ENUM_SERVICE_STATUSW *dependencies;
    SERVICE_STATUS ss;
    SC_HANDLE depserv;

    if (EnumDependentServicesW(service, SERVICE_ACTIVE, NULL,
                               0, &needed, &count))
        return TRUE;

    if (GetLastError() != ERROR_MORE_DATA)
        return FALSE;

    dependencies = msi_alloc(needed);
    if (!dependencies)
        return FALSE;

    if (!EnumDependentServicesW(service, SERVICE_ACTIVE, dependencies,
                                needed, &needed, &count))
        goto error;

    for (i = 0; i < count; i++)
    {
        depserv = OpenServiceW(scm, dependencies[i].lpServiceName,
                               SERVICE_STOP | SERVICE_QUERY_STATUS);
        if (!depserv)
            goto error;

        if (!ControlService(depserv, SERVICE_CONTROL_STOP, &ss))
            goto error;
    }

    return TRUE;

error:
    msi_free(dependencies);
    return FALSE;
}

static UINT stop_service( LPCWSTR name )
{
    SC_HANDLE scm = NULL, service = NULL;
    SERVICE_STATUS status;
    SERVICE_STATUS_PROCESS ssp;
    DWORD needed;

    scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!scm)
    {
        WARN("Failed to open the SCM: %d\n", GetLastError());
        goto done;
    }

    service = OpenServiceW(scm, name,
                           SERVICE_STOP |
                           SERVICE_QUERY_STATUS |
                           SERVICE_ENUMERATE_DEPENDENTS);
    if (!service)
    {
        WARN("Failed to open service (%s): %d\n", debugstr_w(name), GetLastError());
        goto done;
    }

    if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                              sizeof(SERVICE_STATUS_PROCESS), &needed))
    {
        WARN("Failed to query service status (%s): %d\n", debugstr_w(name), GetLastError());
        goto done;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED)
        goto done;

    stop_service_dependents(scm, service);

    if (!ControlService(service, SERVICE_CONTROL_STOP, &status))
        WARN("Failed to stop service (%s): %d\n", debugstr_w(name), GetLastError());

done:
    CloseServiceHandle(service);
    CloseServiceHandle(scm);

    return ERROR_SUCCESS;
}

static UINT ITERATE_StopService( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    LPCWSTR component;
    LPWSTR name = NULL, display_name = NULL;
    DWORD event, len;
    SC_HANDLE scm;

    event = MSI_RecordGetInteger( rec, 3 );
    if (!(event & msidbServiceControlEventStop))
        return ERROR_SUCCESS;

    component = MSI_RecordGetString( rec, 6 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    scm = OpenSCManagerW( NULL, NULL, SC_MANAGER_CONNECT );
    if (!scm)
    {
        ERR("Failed to open the service control manager\n");
        goto done;
    }

    len = 0;
    if (!GetServiceDisplayNameW( scm, name, NULL, &len ) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        if ((display_name = msi_alloc( ++len * sizeof(WCHAR ))))
            GetServiceDisplayNameW( scm, name, display_name, &len );
    }
    CloseServiceHandle( scm );

    deformat_string( package, MSI_RecordGetString( rec, 2 ), &name );
    stop_service( name );

done:
    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, display_name );
    MSI_RecordSetStringW( uirow, 2, name );
    ui_actiondata( package, szStopServices, uirow );
    msiobj_release( &uirow->hdr );

    msi_free( name );
    msi_free( display_name );
    return ERROR_SUCCESS;
}

static UINT ACTION_StopServices( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;

    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'S','e','r','v','i','c','e','C','o','n','t','r','o','l',0 };

    rc = MSI_DatabaseOpenViewW(package->db, query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_StopService, package);
    msiobj_release(&view->hdr);

    return rc;
}

static UINT ITERATE_DeleteService( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    LPCWSTR component;
    LPWSTR name = NULL, display_name = NULL;
    DWORD event, len;
    SC_HANDLE scm = NULL, service = NULL;

    event = MSI_RecordGetInteger( rec, 3 );
    if (!(event & msidbServiceControlEventDelete))
        return ERROR_SUCCESS;

    component = MSI_RecordGetString(rec, 6);
    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    deformat_string( package, MSI_RecordGetString(rec, 2), &name );
    stop_service( name );

    scm = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (!scm)
    {
        WARN("Failed to open the SCM: %d\n", GetLastError());
        goto done;
    }

    len = 0;
    if (!GetServiceDisplayNameW( scm, name, NULL, &len ) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        if ((display_name = msi_alloc( ++len * sizeof(WCHAR ))))
            GetServiceDisplayNameW( scm, name, display_name, &len );
    }

    service = OpenServiceW( scm, name, DELETE );
    if (!service)
    {
        WARN("Failed to open service (%s): %u\n", debugstr_w(name), GetLastError());
        goto done;
    }

    if (!DeleteService( service ))
        WARN("Failed to delete service (%s): %u\n", debugstr_w(name), GetLastError());

done:
    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, display_name );
    MSI_RecordSetStringW( uirow, 2, name );
    ui_actiondata( package, szDeleteServices, uirow );
    msiobj_release( &uirow->hdr );

    CloseServiceHandle( service );
    CloseServiceHandle( scm );
    msi_free( name );
    msi_free( display_name );

    return ERROR_SUCCESS;
}

static UINT ACTION_DeleteServices( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;

    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'S','e','r','v','i','c','e','C','o','n','t','r','o','l',0 };

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_DeleteService, package );
    msiobj_release( &view->hdr );

    return rc;
}

static UINT ITERATE_InstallODBCDriver( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPWSTR driver, driver_path, ptr;
    WCHAR outpath[MAX_PATH];
    MSIFILE *driver_file = NULL, *setup_file = NULL;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    LPCWSTR desc, file_key, component;
    DWORD len, usage;
    UINT r = ERROR_SUCCESS;

    static const WCHAR driver_fmt[] = {
        'D','r','i','v','e','r','=','%','s',0};
    static const WCHAR setup_fmt[] = {
        'S','e','t','u','p','=','%','s',0};
    static const WCHAR usage_fmt[] = {
        'F','i','l','e','U','s','a','g','e','=','1',0};

    component = MSI_RecordGetString( rec, 2 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    desc = MSI_RecordGetString(rec, 3);

    file_key = MSI_RecordGetString( rec, 4 );
    if (file_key) driver_file = get_loaded_file( package, file_key );

    file_key = MSI_RecordGetString( rec, 5 );
    if (file_key) setup_file = get_loaded_file( package, file_key );

    if (!driver_file)
    {
        ERR("ODBC Driver entry not found!\n");
        return ERROR_FUNCTION_FAILED;
    }

    len = lstrlenW(desc) + lstrlenW(driver_fmt) + lstrlenW(driver_file->FileName);
    if (setup_file)
        len += lstrlenW(setup_fmt) + lstrlenW(setup_file->FileName);
    len += lstrlenW(usage_fmt) + 2; /* \0\0 */

    driver = msi_alloc(len * sizeof(WCHAR));
    if (!driver)
        return ERROR_OUTOFMEMORY;

    ptr = driver;
    lstrcpyW(ptr, desc);
    ptr += lstrlenW(ptr) + 1;

    len = sprintfW(ptr, driver_fmt, driver_file->FileName);
    ptr += len + 1;

    if (setup_file)
    {
        len = sprintfW(ptr, setup_fmt, setup_file->FileName);
        ptr += len + 1;
    }

    lstrcpyW(ptr, usage_fmt);
    ptr += lstrlenW(ptr) + 1;
    *ptr = '\0';

    driver_path = strdupW(driver_file->TargetPath);
    ptr = strrchrW(driver_path, '\\');
    if (ptr) *ptr = '\0';

    if (!SQLInstallDriverExW(driver, driver_path, outpath, MAX_PATH,
                             NULL, ODBC_INSTALL_COMPLETE, &usage))
    {
        ERR("Failed to install SQL driver!\n");
        r = ERROR_FUNCTION_FAILED;
    }

    uirow = MSI_CreateRecord( 5 );
    MSI_RecordSetStringW( uirow, 1, desc );
    MSI_RecordSetStringW( uirow, 2, MSI_RecordGetString(rec, 2) );
    MSI_RecordSetStringW( uirow, 3, driver_file->Component->Directory );
    ui_actiondata( package, szInstallODBC, uirow );
    msiobj_release( &uirow->hdr );

    msi_free(driver);
    msi_free(driver_path);

    return r;
}

static UINT ITERATE_InstallODBCTranslator( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPWSTR translator, translator_path, ptr;
    WCHAR outpath[MAX_PATH];
    MSIFILE *translator_file = NULL, *setup_file = NULL;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    LPCWSTR desc, file_key, component;
    DWORD len, usage;
    UINT r = ERROR_SUCCESS;

    static const WCHAR translator_fmt[] = {
        'T','r','a','n','s','l','a','t','o','r','=','%','s',0};
    static const WCHAR setup_fmt[] = {
        'S','e','t','u','p','=','%','s',0};

    component = MSI_RecordGetString( rec, 2 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    desc = MSI_RecordGetString(rec, 3);

    file_key = MSI_RecordGetString( rec, 4 );
    if (file_key) translator_file = get_loaded_file( package, file_key );

    file_key = MSI_RecordGetString( rec, 5 );
    if (file_key) setup_file = get_loaded_file( package, file_key );

    if (!translator_file)
    {
        ERR("ODBC Translator entry not found!\n");
        return ERROR_FUNCTION_FAILED;
    }

    len = lstrlenW(desc) + lstrlenW(translator_fmt) + lstrlenW(translator_file->FileName) + 2; /* \0\0 */
    if (setup_file)
        len += lstrlenW(setup_fmt) + lstrlenW(setup_file->FileName);

    translator = msi_alloc(len * sizeof(WCHAR));
    if (!translator)
        return ERROR_OUTOFMEMORY;

    ptr = translator;
    lstrcpyW(ptr, desc);
    ptr += lstrlenW(ptr) + 1;

    len = sprintfW(ptr, translator_fmt, translator_file->FileName);
    ptr += len + 1;

    if (setup_file)
    {
        len = sprintfW(ptr, setup_fmt, setup_file->FileName);
        ptr += len + 1;
    }
    *ptr = '\0';

    translator_path = strdupW(translator_file->TargetPath);
    ptr = strrchrW(translator_path, '\\');
    if (ptr) *ptr = '\0';

    if (!SQLInstallTranslatorExW(translator, translator_path, outpath, MAX_PATH,
                                 NULL, ODBC_INSTALL_COMPLETE, &usage))
    {
        ERR("Failed to install SQL translator!\n");
        r = ERROR_FUNCTION_FAILED;
    }

    uirow = MSI_CreateRecord( 5 );
    MSI_RecordSetStringW( uirow, 1, desc );
    MSI_RecordSetStringW( uirow, 2, MSI_RecordGetString(rec, 2) );
    MSI_RecordSetStringW( uirow, 3, translator_file->Component->Directory );
    ui_actiondata( package, szInstallODBC, uirow );
    msiobj_release( &uirow->hdr );

    msi_free(translator);
    msi_free(translator_path);

    return r;
}

static UINT ITERATE_InstallODBCDataSource( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    LPWSTR attrs;
    LPCWSTR desc, driver, component;
    WORD request = ODBC_ADD_SYS_DSN;
    INT registration;
    DWORD len;
    UINT r = ERROR_SUCCESS;
    MSIRECORD *uirow;

    static const WCHAR attrs_fmt[] = {
        'D','S','N','=','%','s',0 };

    component = MSI_RecordGetString( rec, 2 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    desc = MSI_RecordGetString(rec, 3);
    driver = MSI_RecordGetString(rec, 4);
    registration = MSI_RecordGetInteger(rec, 5);

    if (registration == msidbODBCDataSourceRegistrationPerMachine) request = ODBC_ADD_SYS_DSN;
    else if (registration == msidbODBCDataSourceRegistrationPerUser) request = ODBC_ADD_DSN;

    len = lstrlenW(attrs_fmt) + lstrlenW(desc) + 2; /* \0\0 */
    attrs = msi_alloc(len * sizeof(WCHAR));
    if (!attrs)
        return ERROR_OUTOFMEMORY;

    len = sprintfW(attrs, attrs_fmt, desc);
    attrs[len + 1] = 0;

    if (!SQLConfigDataSourceW(NULL, request, driver, attrs))
    {
        ERR("Failed to install SQL data source!\n");
        r = ERROR_FUNCTION_FAILED;
    }

    uirow = MSI_CreateRecord( 5 );
    MSI_RecordSetStringW( uirow, 1, desc );
    MSI_RecordSetStringW( uirow, 2, MSI_RecordGetString(rec, 2) );
    MSI_RecordSetInteger( uirow, 3, request );
    ui_actiondata( package, szInstallODBC, uirow );
    msiobj_release( &uirow->hdr );

    msi_free(attrs);

    return r;
}

static UINT ACTION_InstallODBC( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;

    static const WCHAR driver_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','D','r','i','v','e','r',0 };

    static const WCHAR translator_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','T','r','a','n','s','l','a','t','o','r',0 };

    static const WCHAR source_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','D','a','t','a','S','o','u','r','c','e',0 };

    rc = MSI_DatabaseOpenViewW(package->db, driver_query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_InstallODBCDriver, package);
    msiobj_release(&view->hdr);

    rc = MSI_DatabaseOpenViewW(package->db, translator_query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_InstallODBCTranslator, package);
    msiobj_release(&view->hdr);

    rc = MSI_DatabaseOpenViewW(package->db, source_query, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_InstallODBCDataSource, package);
    msiobj_release(&view->hdr);

    return rc;
}

static UINT ITERATE_RemoveODBCDriver( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    DWORD usage;
    LPCWSTR desc, component;

    component = MSI_RecordGetString( rec, 2 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    desc = MSI_RecordGetString( rec, 3 );
    if (!SQLRemoveDriverW( desc, FALSE, &usage ))
    {
        WARN("Failed to remove ODBC driver\n");
    }
    else if (!usage)
    {
        FIXME("Usage count reached 0\n");
    }

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, desc );
    MSI_RecordSetStringW( uirow, 2, MSI_RecordGetString(rec, 2) );
    ui_actiondata( package, szRemoveODBC, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ITERATE_RemoveODBCTranslator( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    DWORD usage;
    LPCWSTR desc, component;

    component = MSI_RecordGetString( rec, 2 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    desc = MSI_RecordGetString( rec, 3 );
    if (!SQLRemoveTranslatorW( desc, &usage ))
    {
        WARN("Failed to remove ODBC translator\n");
    }
    else if (!usage)
    {
        FIXME("Usage count reached 0\n");
    }

    uirow = MSI_CreateRecord( 2 );
    MSI_RecordSetStringW( uirow, 1, desc );
    MSI_RecordSetStringW( uirow, 2, MSI_RecordGetString(rec, 2) );
    ui_actiondata( package, szRemoveODBC, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ITERATE_RemoveODBCDataSource( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    LPWSTR attrs;
    LPCWSTR desc, driver, component;
    WORD request = ODBC_REMOVE_SYS_DSN;
    INT registration;
    DWORD len;

    static const WCHAR attrs_fmt[] = {
        'D','S','N','=','%','s',0 };

    component = MSI_RecordGetString( rec, 2 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    desc = MSI_RecordGetString( rec, 3 );
    driver = MSI_RecordGetString( rec, 4 );
    registration = MSI_RecordGetInteger( rec, 5 );

    if (registration == msidbODBCDataSourceRegistrationPerMachine) request = ODBC_REMOVE_SYS_DSN;
    else if (registration == msidbODBCDataSourceRegistrationPerUser) request = ODBC_REMOVE_DSN;

    len = strlenW( attrs_fmt ) + strlenW( desc ) + 2; /* \0\0 */
    attrs = msi_alloc( len * sizeof(WCHAR) );
    if (!attrs)
        return ERROR_OUTOFMEMORY;

    FIXME("Use ODBCSourceAttribute table\n");

    len = sprintfW( attrs, attrs_fmt, desc );
    attrs[len + 1] = 0;

    if (!SQLConfigDataSourceW( NULL, request, driver, attrs ))
    {
        WARN("Failed to remove ODBC data source\n");
    }
    msi_free( attrs );

    uirow = MSI_CreateRecord( 3 );
    MSI_RecordSetStringW( uirow, 1, desc );
    MSI_RecordSetStringW( uirow, 2, MSI_RecordGetString(rec, 2) );
    MSI_RecordSetInteger( uirow, 3, request );
    ui_actiondata( package, szRemoveODBC, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_RemoveODBC( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;

    static const WCHAR driver_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','D','r','i','v','e','r',0 };

    static const WCHAR translator_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','T','r','a','n','s','l','a','t','o','r',0 };

    static const WCHAR source_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','D','a','t','a','S','o','u','r','c','e',0 };

    rc = MSI_DatabaseOpenViewW( package->db, driver_query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveODBCDriver, package );
    msiobj_release( &view->hdr );

    rc = MSI_DatabaseOpenViewW( package->db, translator_query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveODBCTranslator, package );
    msiobj_release( &view->hdr );

    rc = MSI_DatabaseOpenViewW( package->db, source_query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveODBCDataSource, package );
    msiobj_release( &view->hdr );

    return rc;
}

#define ENV_ACT_SETALWAYS   0x1
#define ENV_ACT_SETABSENT   0x2
#define ENV_ACT_REMOVE      0x4
#define ENV_ACT_REMOVEMATCH 0x8

#define ENV_MOD_MACHINE     0x20000000
#define ENV_MOD_APPEND      0x40000000
#define ENV_MOD_PREFIX      0x80000000
#define ENV_MOD_MASK        0xC0000000

#define check_flag_combo(x, y) ((x) & ~(y)) == (y)

static UINT env_parse_flags( LPCWSTR *name, LPCWSTR *value, DWORD *flags )
{
    LPCWSTR cptr = *name;

    static const WCHAR prefix[] = {'[','~',']',0};
    static const int prefix_len = 3;

    *flags = 0;
    while (*cptr)
    {
        if (*cptr == '=')
            *flags |= ENV_ACT_SETALWAYS;
        else if (*cptr == '+')
            *flags |= ENV_ACT_SETABSENT;
        else if (*cptr == '-')
            *flags |= ENV_ACT_REMOVE;
        else if (*cptr == '!')
            *flags |= ENV_ACT_REMOVEMATCH;
        else if (*cptr == '*')
            *flags |= ENV_MOD_MACHINE;
        else
            break;

        cptr++;
        (*name)++;
    }

    if (!*cptr)
    {
        ERR("Missing environment variable\n");
        return ERROR_FUNCTION_FAILED;
    }

    if (*value)
    {
        LPCWSTR ptr = *value;
        if (!strncmpW(ptr, prefix, prefix_len))
        {
            if (ptr[prefix_len] == szSemiColon[0])
            {
                *flags |= ENV_MOD_APPEND;
                *value += lstrlenW(prefix);
            }
            else
            {
                *value = NULL;
            }
        }
        else if (lstrlenW(*value) >= prefix_len)
        {
            ptr += lstrlenW(ptr) - prefix_len;
            if (!strcmpW( ptr, prefix ))
            {
                if ((ptr-1) > *value && *(ptr-1) == szSemiColon[0])
                {
                    *flags |= ENV_MOD_PREFIX;
                    /* the "[~]" will be removed by deformat_string */;
                }
                else
                {
                    *value = NULL;
                }
            }
        }
    }

    if (check_flag_combo(*flags, ENV_ACT_SETALWAYS | ENV_ACT_SETABSENT) ||
        check_flag_combo(*flags, ENV_ACT_REMOVEMATCH | ENV_ACT_SETABSENT) ||
        check_flag_combo(*flags, ENV_ACT_REMOVEMATCH | ENV_ACT_SETALWAYS) ||
        check_flag_combo(*flags, ENV_ACT_SETABSENT | ENV_MOD_MASK))
    {
        ERR("Invalid flags: %08x\n", *flags);
        return ERROR_FUNCTION_FAILED;
    }

    if (!*flags)
        *flags = ENV_ACT_SETALWAYS | ENV_ACT_REMOVE;

    return ERROR_SUCCESS;
}

static UINT open_env_key( DWORD flags, HKEY *key )
{
    static const WCHAR user_env[] =
        {'E','n','v','i','r','o','n','m','e','n','t',0};
    static const WCHAR machine_env[] =
        {'S','y','s','t','e','m','\\',
         'C','u','r','r','e','n','t','C','o','n','t','r','o','l','S','e','t','\\',
         'C','o','n','t','r','o','l','\\',
         'S','e','s','s','i','o','n',' ','M','a','n','a','g','e','r','\\',
         'E','n','v','i','r','o','n','m','e','n','t',0};
    const WCHAR *env;
    HKEY root;
    LONG res;

    if (flags & ENV_MOD_MACHINE)
    {
        env = machine_env;
        root = HKEY_LOCAL_MACHINE;
    }
    else
    {
        env = user_env;
        root = HKEY_CURRENT_USER;
    }

    res = RegOpenKeyExW( root, env, 0, KEY_ALL_ACCESS, key );
    if (res != ERROR_SUCCESS)
    {
        WARN("Failed to open key %s (%d)\n", debugstr_w(env), res);
        return ERROR_FUNCTION_FAILED;
    }

    return ERROR_SUCCESS;
}

static UINT ITERATE_WriteEnvironmentString( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR name, value, component;
    LPWSTR data = NULL, newval = NULL, deformatted = NULL, ptr;
    DWORD flags, type, size;
    UINT res;
    HKEY env = NULL;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    int action = 0;

    component = MSI_RecordGetString(rec, 4);
    comp = get_loaded_component(package, component);
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_LOCAL)
    {
        TRACE("Component not scheduled for installation: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_LOCAL;

    name = MSI_RecordGetString(rec, 2);
    value = MSI_RecordGetString(rec, 3);

    TRACE("name %s value %s\n", debugstr_w(name), debugstr_w(value));

    res = env_parse_flags(&name, &value, &flags);
    if (res != ERROR_SUCCESS || !value)
       goto done;

    if (value && !deformat_string(package, value, &deformatted))
    {
        res = ERROR_OUTOFMEMORY;
        goto done;
    }

    value = deformatted;

    res = open_env_key( flags, &env );
    if (res != ERROR_SUCCESS)
        goto done;

    if (flags & ENV_MOD_MACHINE)
        action |= 0x20000000;

    size = 0;
    type = REG_SZ;
    res = RegQueryValueExW(env, name, NULL, &type, NULL, &size);
    if ((res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND) ||
        (res == ERROR_SUCCESS && type != REG_SZ && type != REG_EXPAND_SZ))
        goto done;

    if ((res == ERROR_FILE_NOT_FOUND || !(flags & ENV_MOD_MASK)))
    {
        action = 0x2;

        /* Nothing to do. */
        if (!value)
        {
            res = ERROR_SUCCESS;
            goto done;
        }

        /* If we are appending but the string was empty, strip ; */
        if ((flags & ENV_MOD_APPEND) && (value[0] == szSemiColon[0])) value++;

        size = (lstrlenW(value) + 1) * sizeof(WCHAR);
        newval = strdupW(value);
        if (!newval)
        {
            res = ERROR_OUTOFMEMORY;
            goto done;
        }
    }
    else
    {
        action = 0x1;

        /* Contrary to MSDN, +-variable to [~];path works */
        if (flags & ENV_ACT_SETABSENT && !(flags & ENV_MOD_MASK))
        {
            res = ERROR_SUCCESS;
            goto done;
        }

        data = msi_alloc(size);
        if (!data)
        {
            RegCloseKey(env);
            return ERROR_OUTOFMEMORY;
        }

        res = RegQueryValueExW(env, name, NULL, &type, (LPVOID)data, &size);
        if (res != ERROR_SUCCESS)
            goto done;

        if (flags & ENV_ACT_REMOVEMATCH && (!value || !strcmpW( data, value )))
        {
            action = 0x4;
            res = RegDeleteValueW(env, name);
            if (res != ERROR_SUCCESS)
                WARN("Failed to remove value %s (%d)\n", debugstr_w(name), res);
            goto done;
        }

        size = (lstrlenW(data) + 1) * sizeof(WCHAR);
        if (flags & ENV_MOD_MASK)
        {
            DWORD mod_size;
            int multiplier = 0;
            if (flags & ENV_MOD_APPEND) multiplier++;
            if (flags & ENV_MOD_PREFIX) multiplier++;
            mod_size = lstrlenW(value) * multiplier;
            size += mod_size * sizeof(WCHAR);
        }

        newval = msi_alloc(size);
        ptr = newval;
        if (!newval)
        {
            res = ERROR_OUTOFMEMORY;
            goto done;
        }

        if (flags & ENV_MOD_PREFIX)
        {
            lstrcpyW(newval, value);
            ptr = newval + lstrlenW(value);
            action |= 0x80000000;
        }

        lstrcpyW(ptr, data);

        if (flags & ENV_MOD_APPEND)
        {
            lstrcatW(newval, value);
            action |= 0x40000000;
        }
    }
    TRACE("setting %s to %s\n", debugstr_w(name), debugstr_w(newval));
    res = RegSetValueExW(env, name, 0, type, (LPVOID)newval, size);
    if (res)
    {
        WARN("Failed to set %s to %s (%d)\n",  debugstr_w(name), debugstr_w(newval), res);
    }

done:
    uirow = MSI_CreateRecord( 3 );
    MSI_RecordSetStringW( uirow, 1, name );
    MSI_RecordSetStringW( uirow, 2, newval );
    MSI_RecordSetInteger( uirow, 3, action );
    ui_actiondata( package, szWriteEnvironmentStrings, uirow );
    msiobj_release( &uirow->hdr );

    if (env) RegCloseKey(env);
    msi_free(deformatted);
    msi_free(data);
    msi_free(newval);
    return res;
}

static UINT ACTION_WriteEnvironmentStrings( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY * view;
    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','E','n','v','i','r','o','n','m','e','n','t','`',0};
    rc = MSI_DatabaseOpenViewW(package->db, ExecSeqQuery, &view);
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords(view, NULL, ITERATE_WriteEnvironmentString, package);
    msiobj_release(&view->hdr);

    return rc;
}

static UINT ITERATE_RemoveEnvironmentString( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    LPCWSTR name, value, component;
    LPWSTR deformatted = NULL;
    DWORD flags;
    HKEY env;
    MSICOMPONENT *comp;
    MSIRECORD *uirow;
    int action = 0;
    LONG res;
    UINT r;

    component = MSI_RecordGetString( rec, 4 );
    comp = get_loaded_component( package, component );
    if (!comp)
        return ERROR_SUCCESS;

    if (!comp->Enabled)
    {
        TRACE("component is disabled\n");
        return ERROR_SUCCESS;
    }

    if (comp->ActionRequest != INSTALLSTATE_ABSENT)
    {
        TRACE("Component not scheduled for removal: %s\n", debugstr_w(component));
        comp->Action = comp->Installed;
        return ERROR_SUCCESS;
    }
    comp->Action = INSTALLSTATE_ABSENT;

    name = MSI_RecordGetString( rec, 2 );
    value = MSI_RecordGetString( rec, 3 );

    TRACE("name %s value %s\n", debugstr_w(name), debugstr_w(value));

    r = env_parse_flags( &name, &value, &flags );
    if (r != ERROR_SUCCESS)
       return r;

    if (!(flags & ENV_ACT_REMOVE))
    {
        TRACE("Environment variable %s not marked for removal\n", debugstr_w(name));
        return ERROR_SUCCESS;
    }

    if (value && !deformat_string( package, value, &deformatted ))
        return ERROR_OUTOFMEMORY;

    value = deformatted;

    r = open_env_key( flags, &env );
    if (r != ERROR_SUCCESS)
    {
        r = ERROR_SUCCESS;
        goto done;
    }

    if (flags & ENV_MOD_MACHINE)
        action |= 0x20000000;

    TRACE("Removing %s\n", debugstr_w(name));

    res = RegDeleteValueW( env, name );
    if (res != ERROR_SUCCESS)
    {
        WARN("Failed to delete value %s (%d)\n", debugstr_w(name), res);
        r = ERROR_SUCCESS;
    }

done:
    uirow = MSI_CreateRecord( 3 );
    MSI_RecordSetStringW( uirow, 1, name );
    MSI_RecordSetStringW( uirow, 2, value );
    MSI_RecordSetInteger( uirow, 3, action );
    ui_actiondata( package, szRemoveEnvironmentStrings, uirow );
    msiobj_release( &uirow->hdr );

    if (env) RegCloseKey( env );
    msi_free( deformatted );
    return r;
}

static UINT ACTION_RemoveEnvironmentStrings( MSIPACKAGE *package )
{
    UINT rc;
    MSIQUERY *view;
    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','E','n','v','i','r','o','n','m','e','n','t','`',0};

    rc = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (rc != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    rc = MSI_IterateRecords( view, NULL, ITERATE_RemoveEnvironmentString, package );
    msiobj_release( &view->hdr );

    return rc;
}

static UINT ACTION_ValidateProductID( MSIPACKAGE *package )
{
    LPWSTR key, template, id;
    UINT r = ERROR_SUCCESS;

    id = msi_dup_property( package->db, szProductID );
    if (id)
    {
        msi_free( id );
        return ERROR_SUCCESS;
    }
    template = msi_dup_property( package->db, szPIDTemplate );
    key = msi_dup_property( package->db, szPIDKEY );

    if (key && template)
    {
        FIXME( "partial stub: template %s key %s\n", debugstr_w(template), debugstr_w(key) );
        r = msi_set_property( package->db, szProductID, key );
    }
    msi_free( template );
    msi_free( key );
    return r;
}

static UINT ACTION_ScheduleReboot( MSIPACKAGE *package )
{
    TRACE("\n");
    package->need_reboot = 1;
    return ERROR_SUCCESS;
}

static UINT ACTION_AllocateRegistrySpace( MSIPACKAGE *package )
{
    static const WCHAR szAvailableFreeReg[] =
        {'A','V','A','I','L','A','B','L','E','F','R','E','E','R','E','G',0};
    MSIRECORD *uirow;
    int space = msi_get_property_int( package->db, szAvailableFreeReg, 0 );

    TRACE("%p %d kilobytes\n", package, space);

    uirow = MSI_CreateRecord( 1 );
    MSI_RecordSetInteger( uirow, 1, space );
    ui_actiondata( package, szAllocateRegistrySpace, uirow );
    msiobj_release( &uirow->hdr );

    return ERROR_SUCCESS;
}

static UINT ACTION_DisableRollback( MSIPACKAGE *package )
{
    FIXME("%p\n", package);
    return ERROR_SUCCESS;
}

static UINT ACTION_InstallAdminPackage( MSIPACKAGE *package )
{
    FIXME("%p\n", package);
    return ERROR_SUCCESS;
}

static UINT ACTION_SetODBCFolders( MSIPACKAGE *package )
{
    UINT r, count;
    MSIQUERY *view;

    static const WCHAR driver_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','D','r','i','v','e','r',0 };

    static const WCHAR translator_query[] = {
        'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
        'O','D','B','C','T','r','a','n','s','l','a','t','o','r',0 };

    r = MSI_DatabaseOpenViewW( package->db, driver_query, &view );
    if (r == ERROR_SUCCESS)
    {
        count = 0;
        r = MSI_IterateRecords( view, &count, NULL, package );
        msiobj_release( &view->hdr );
        if (count) FIXME("ignored %u rows in ODBCDriver table\n", count);
    }

    r = MSI_DatabaseOpenViewW( package->db, translator_query, &view );
    if (r == ERROR_SUCCESS)
    {
        count = 0;
        r = MSI_IterateRecords( view, &count, NULL, package );
        msiobj_release( &view->hdr );
        if (count) FIXME("ignored %u rows in ODBCTranslator table\n", count);
    }

    return ERROR_SUCCESS;
}

static UINT ITERATE_RemoveExistingProducts( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    const WCHAR *property = MSI_RecordGetString( rec, 1 );
    WCHAR *value;

    if ((value = msi_dup_property( package->db, property )))
    {
        FIXME("remove %s\n", debugstr_w(value));
        msi_free( value );
    }
    return ERROR_SUCCESS;
}

static UINT ACTION_RemoveExistingProducts( MSIPACKAGE *package )
{
    UINT r;
    MSIQUERY *view;

    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','A','c','t','i','o','n','P','r','o','p','e','r','t','y',
         ' ','F','R','O','M',' ','U','p','g','r','a','d','e',0};

    r = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (r == ERROR_SUCCESS)
    {
        r = MSI_IterateRecords( view, NULL, ITERATE_RemoveExistingProducts, package );
        msiobj_release( &view->hdr );
    }
    return ERROR_SUCCESS;
}

static UINT ITERATE_MigrateFeatureStates( MSIRECORD *rec, LPVOID param )
{
    MSIPACKAGE *package = param;
    int attributes = MSI_RecordGetInteger( rec, 5 );

    if (attributes & msidbUpgradeAttributesMigrateFeatures)
    {
        const WCHAR *upgrade_code = MSI_RecordGetString( rec, 1 );
        const WCHAR *version_min = MSI_RecordGetString( rec, 2 );
        const WCHAR *version_max = MSI_RecordGetString( rec, 3 );
        const WCHAR *language = MSI_RecordGetString( rec, 4 );
        HKEY hkey;
        UINT r;

        if (package->Context == MSIINSTALLCONTEXT_MACHINE)
        {
            r = MSIREG_OpenClassesUpgradeCodesKey( upgrade_code, &hkey, FALSE );
            if (r != ERROR_SUCCESS)
                return ERROR_SUCCESS;
        }
        else
        {
            r = MSIREG_OpenUserUpgradeCodesKey( upgrade_code, &hkey, FALSE );
            if (r != ERROR_SUCCESS)
                return ERROR_SUCCESS;
        }
        RegCloseKey( hkey );

        FIXME("migrate feature states from %s version min %s version max %s language %s\n",
              debugstr_w(upgrade_code), debugstr_w(version_min),
              debugstr_w(version_max), debugstr_w(language));
    }
    return ERROR_SUCCESS;
}

static UINT ACTION_MigrateFeatureStates( MSIPACKAGE *package )
{
    UINT r;
    MSIQUERY *view;

    static const WCHAR query[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ','U','p','g','r','a','d','e',0};

    if (msi_get_property_int( package->db, szInstalled, 0 ))
    {
        TRACE("product is installed, skipping action\n");
        return ERROR_SUCCESS;
    }
    if (msi_get_property_int( package->db, szPreselected, 0 ))
    {
        TRACE("Preselected property is set, not migrating feature states\n");
        return ERROR_SUCCESS;
    }

    r = MSI_DatabaseOpenViewW( package->db, query, &view );
    if (r == ERROR_SUCCESS)
    {
        r = MSI_IterateRecords( view, NULL, ITERATE_MigrateFeatureStates, package );
        msiobj_release( &view->hdr );
    }
    return ERROR_SUCCESS;
}

static UINT msi_unimplemented_action_stub( MSIPACKAGE *package,
                                           LPCSTR action, LPCWSTR table )
{
    static const WCHAR query[] = {
        'S','E','L','E','C','T',' ','*',' ',
        'F','R','O','M',' ','`','%','s','`',0 };
    MSIQUERY *view = NULL;
    DWORD count = 0;
    UINT r;
    
    r = MSI_OpenQuery( package->db, &view, query, table );
    if (r == ERROR_SUCCESS)
    {
        r = MSI_IterateRecords(view, &count, NULL, package);
        msiobj_release(&view->hdr);
    }

    if (count)
        FIXME("%s -> %u ignored %s table values\n",
              action, count, debugstr_w(table));

    return ERROR_SUCCESS;
}

static UINT ACTION_PatchFiles( MSIPACKAGE *package )
{
    static const WCHAR table[] = { 'P','a','t','c','h',0 };
    return msi_unimplemented_action_stub( package, "PatchFiles", table );
}

static UINT ACTION_BindImage( MSIPACKAGE *package )
{
    static const WCHAR table[] = { 'B','i','n','d','I','m','a','g','e',0 };
    return msi_unimplemented_action_stub( package, "BindImage", table );
}

static UINT ACTION_IsolateComponents( MSIPACKAGE *package )
{
    static const WCHAR table[] = {
        'I','s','o','l','a','t','e','d','C','o','m','p','o','n','e','n','t',0 };
    return msi_unimplemented_action_stub( package, "IsolateComponents", table );
}

static UINT ACTION_RMCCPSearch( MSIPACKAGE *package )
{
    static const WCHAR table[] = { 'C','C','P','S','e','a','r','c','h',0 };
    return msi_unimplemented_action_stub( package, "RMCCPSearch", table );
}

static UINT ACTION_RegisterComPlus( MSIPACKAGE *package )
{
    static const WCHAR table[] = { 'C','o','m','p','l','u','s',0 };
    return msi_unimplemented_action_stub( package, "RegisterComPlus", table );
}

static UINT ACTION_UnregisterComPlus( MSIPACKAGE *package )
{
    static const WCHAR table[] = { 'C','o','m','p','l','u','s',0 };
    return msi_unimplemented_action_stub( package, "UnregisterComPlus", table );
}

static UINT ACTION_InstallSFPCatalogFile( MSIPACKAGE *package )
{
    static const WCHAR table[] = { 'S','F','P','C','a','t','a','l','o','g',0 };
    return msi_unimplemented_action_stub( package, "InstallSFPCatalogFile", table );
}

typedef UINT (*STANDARDACTIONHANDLER)(MSIPACKAGE*);

static const struct
{
    const WCHAR *action;
    UINT (*handler)(MSIPACKAGE *);
}
StandardActions[] =
{
    { szAllocateRegistrySpace, ACTION_AllocateRegistrySpace },
    { szAppSearch, ACTION_AppSearch },
    { szBindImage, ACTION_BindImage },
    { szCCPSearch, ACTION_CCPSearch },
    { szCostFinalize, ACTION_CostFinalize },
    { szCostInitialize, ACTION_CostInitialize },
    { szCreateFolders, ACTION_CreateFolders },
    { szCreateShortcuts, ACTION_CreateShortcuts },
    { szDeleteServices, ACTION_DeleteServices },
    { szDisableRollback, ACTION_DisableRollback },
    { szDuplicateFiles, ACTION_DuplicateFiles },
    { szExecuteAction, ACTION_ExecuteAction },
    { szFileCost, ACTION_FileCost },
    { szFindRelatedProducts, ACTION_FindRelatedProducts },
    { szForceReboot, ACTION_ForceReboot },
    { szInstallAdminPackage, ACTION_InstallAdminPackage },
    { szInstallExecute, ACTION_InstallExecute },
    { szInstallExecuteAgain, ACTION_InstallExecute },
    { szInstallFiles, ACTION_InstallFiles},
    { szInstallFinalize, ACTION_InstallFinalize },
    { szInstallInitialize, ACTION_InstallInitialize },
    { szInstallSFPCatalogFile, ACTION_InstallSFPCatalogFile },
    { szInstallValidate, ACTION_InstallValidate },
    { szIsolateComponents, ACTION_IsolateComponents },
    { szLaunchConditions, ACTION_LaunchConditions },
    { szMigrateFeatureStates, ACTION_MigrateFeatureStates },
    { szMoveFiles, ACTION_MoveFiles },
    { szMsiPublishAssemblies, ACTION_MsiPublishAssemblies },
    { szMsiUnpublishAssemblies, ACTION_MsiUnpublishAssemblies },
    { szInstallODBC, ACTION_InstallODBC },
    { szInstallServices, ACTION_InstallServices },
    { szPatchFiles, ACTION_PatchFiles },
    { szProcessComponents, ACTION_ProcessComponents },
    { szPublishComponents, ACTION_PublishComponents },
    { szPublishFeatures, ACTION_PublishFeatures },
    { szPublishProduct, ACTION_PublishProduct },
    { szRegisterClassInfo, ACTION_RegisterClassInfo },
    { szRegisterComPlus, ACTION_RegisterComPlus},
    { szRegisterExtensionInfo, ACTION_RegisterExtensionInfo },
    { szRegisterFonts, ACTION_RegisterFonts },
    { szRegisterMIMEInfo, ACTION_RegisterMIMEInfo },
    { szRegisterProduct, ACTION_RegisterProduct },
    { szRegisterProgIdInfo, ACTION_RegisterProgIdInfo },
    { szRegisterTypeLibraries, ACTION_RegisterTypeLibraries },
    { szRegisterUser, ACTION_RegisterUser },
    { szRemoveDuplicateFiles, ACTION_RemoveDuplicateFiles },
    { szRemoveEnvironmentStrings, ACTION_RemoveEnvironmentStrings },
    { szRemoveExistingProducts, ACTION_RemoveExistingProducts },
    { szRemoveFiles, ACTION_RemoveFiles },
    { szRemoveFolders, ACTION_RemoveFolders },
    { szRemoveIniValues, ACTION_RemoveIniValues },
    { szRemoveODBC, ACTION_RemoveODBC },
    { szRemoveRegistryValues, ACTION_RemoveRegistryValues },
    { szRemoveShortcuts, ACTION_RemoveShortcuts },
    { szResolveSource, ACTION_ResolveSource },
    { szRMCCPSearch, ACTION_RMCCPSearch },
    { szScheduleReboot, ACTION_ScheduleReboot },
    { szSelfRegModules, ACTION_SelfRegModules },
    { szSelfUnregModules, ACTION_SelfUnregModules },
    { szSetODBCFolders, ACTION_SetODBCFolders },
    { szStartServices, ACTION_StartServices },
    { szStopServices, ACTION_StopServices },
    { szUnpublishComponents, ACTION_UnpublishComponents },
    { szUnpublishFeatures, ACTION_UnpublishFeatures },
    { szUnregisterClassInfo, ACTION_UnregisterClassInfo },
    { szUnregisterComPlus, ACTION_UnregisterComPlus },
    { szUnregisterExtensionInfo, ACTION_UnregisterExtensionInfo },
    { szUnregisterFonts, ACTION_UnregisterFonts },
    { szUnregisterMIMEInfo, ACTION_UnregisterMIMEInfo },
    { szUnregisterProgIdInfo, ACTION_UnregisterProgIdInfo },
    { szUnregisterTypeLibraries, ACTION_UnregisterTypeLibraries },
    { szValidateProductID, ACTION_ValidateProductID },
    { szWriteEnvironmentStrings, ACTION_WriteEnvironmentStrings },
    { szWriteIniValues, ACTION_WriteIniValues },
    { szWriteRegistryValues, ACTION_WriteRegistryValues },
    { NULL, NULL },
};

static BOOL ACTION_HandleStandardAction( MSIPACKAGE *package, LPCWSTR action, UINT *rc )
{
    BOOL ret = FALSE;
    UINT i;

    i = 0;
    while (StandardActions[i].action != NULL)
    {
        if (!strcmpW( StandardActions[i].action, action ))
        {
            ui_actionstart( package, action );
            if (StandardActions[i].handler)
            {
                ui_actioninfo( package, action, TRUE, 0 );
                *rc = StandardActions[i].handler( package );
                ui_actioninfo( package, action, FALSE, *rc );
            }
            else
            {
                FIXME("unhandled standard action %s\n", debugstr_w(action));
                *rc = ERROR_SUCCESS;
            }
            ret = TRUE;
            break;
        }
        i++;
    }
    return ret;
}

UINT ACTION_PerformAction(MSIPACKAGE *package, const WCHAR *action, UINT script)
{
    UINT rc = ERROR_SUCCESS;
    BOOL handled;

    TRACE("Performing action (%s)\n", debugstr_w(action));

    handled = ACTION_HandleStandardAction(package, action, &rc);

    if (!handled)
        handled = ACTION_HandleCustomAction(package, action, &rc, script, TRUE);

    if (!handled)
    {
        WARN("unhandled msi action %s\n", debugstr_w(action));
        rc = ERROR_FUNCTION_NOT_CALLED;
    }

    return rc;
}

UINT ACTION_PerformUIAction(MSIPACKAGE *package, const WCHAR *action, UINT script)
{
    UINT rc = ERROR_SUCCESS;
    BOOL handled = FALSE;

    TRACE("Performing action (%s)\n", debugstr_w(action));

    handled = ACTION_HandleStandardAction(package, action, &rc);

    if (!handled)
        handled = ACTION_HandleCustomAction(package, action, &rc, script, FALSE);

    if( !handled && ACTION_DialogBox(package, action) == ERROR_SUCCESS )
        handled = TRUE;

    if (!handled)
    {
        WARN("unhandled msi action %s\n", debugstr_w(action));
        rc = ERROR_FUNCTION_NOT_CALLED;
    }

    return rc;
}

static UINT ACTION_PerformActionSequence(MSIPACKAGE *package, UINT seq)
{
    UINT rc = ERROR_SUCCESS;
    MSIRECORD *row;

    static const WCHAR ExecSeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','I','n','s','t','a','l','l','E','x','e','c','u','t','e',
         'S','e','q','u','e','n','c','e','`',' ', 'W','H','E','R','E',' ',
         '`','S','e','q','u','e','n','c','e','`',' ', '=',' ','%','i',0};
    static const WCHAR UISeqQuery[] =
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
     '`','I','n','s','t','a','l','l','U','I','S','e','q','u','e','n','c','e',
     '`', ' ', 'W','H','E','R','E',' ','`','S','e','q','u','e','n','c','e','`',
	 ' ', '=',' ','%','i',0};

    if (needs_ui_sequence(package))
        row = MSI_QueryGetRecord(package->db, UISeqQuery, seq);
    else
        row = MSI_QueryGetRecord(package->db, ExecSeqQuery, seq);

    if (row)
    {
        LPCWSTR action, cond;

        TRACE("Running the actions\n");

        /* check conditions */
        cond = MSI_RecordGetString(row, 2);

        /* this is a hack to skip errors in the condition code */
        if (MSI_EvaluateConditionW(package, cond) == MSICONDITION_FALSE)
        {
            msiobj_release(&row->hdr);
            return ERROR_SUCCESS;
        }

        action = MSI_RecordGetString(row, 1);
        if (!action)
        {
            ERR("failed to fetch action\n");
            msiobj_release(&row->hdr);
            return ERROR_FUNCTION_FAILED;
        }

        if (needs_ui_sequence(package))
            rc = ACTION_PerformUIAction(package, action, -1);
        else
            rc = ACTION_PerformAction(package, action, -1);

        msiobj_release(&row->hdr);
    }

    return rc;
}

/****************************************************
 * TOP level entry points
 *****************************************************/

UINT MSI_InstallPackage( MSIPACKAGE *package, LPCWSTR szPackagePath,
                         LPCWSTR szCommandLine )
{
    UINT rc;
    BOOL ui_exists;

    static const WCHAR szAction[] = {'A','C','T','I','O','N',0};
    static const WCHAR szInstall[] = {'I','N','S','T','A','L','L',0};

    msi_set_property( package->db, szAction, szInstall );

    package->script->InWhatSequence = SEQUENCE_INSTALL;

    if (szPackagePath)
    {
        LPWSTR p, dir;
        LPCWSTR file;

        dir = strdupW(szPackagePath);
        p = strrchrW(dir, '\\');
        if (p)
        {
            *(++p) = 0;
            file = szPackagePath + (p - dir);
        }
        else
        {
            msi_free(dir);
            dir = msi_alloc(MAX_PATH * sizeof(WCHAR));
            GetCurrentDirectoryW(MAX_PATH, dir);
            lstrcatW(dir, szBackSlash);
            file = szPackagePath;
        }

        msi_free( package->PackagePath );
        package->PackagePath = msi_alloc((lstrlenW(dir) + lstrlenW(file) + 1) * sizeof(WCHAR));
        if (!package->PackagePath)
        {
            msi_free(dir);
            return ERROR_OUTOFMEMORY;
        }

        lstrcpyW(package->PackagePath, dir);
        lstrcatW(package->PackagePath, file);
        msi_free(dir);

        msi_set_sourcedir_props(package, FALSE);
    }

    rc = msi_parse_command_line( package, szCommandLine, FALSE );
    if (rc != ERROR_SUCCESS)
        return rc;

    msi_apply_transforms( package );
    msi_apply_patches( package );

    if (!szCommandLine && msi_get_property_int( package->db, szInstalled, 0 ))
    {
        TRACE("setting reinstall property\n");
        msi_set_property( package->db, szReinstall, szAll );
    }

    /* properties may have been added by a transform */
    msi_clone_properties( package );

    msi_parse_command_line( package, szCommandLine, FALSE );
    msi_adjust_privilege_properties( package );
    msi_set_context( package );

    if (needs_ui_sequence( package))
    {
        package->script->InWhatSequence |= SEQUENCE_UI;
        rc = ACTION_ProcessUISequence(package);
        ui_exists = ui_sequence_exists(package);
        if (rc == ERROR_SUCCESS || !ui_exists)
        {
            package->script->InWhatSequence |= SEQUENCE_EXEC;
            rc = ACTION_ProcessExecSequence(package, ui_exists);
        }
    }
    else
        rc = ACTION_ProcessExecSequence(package, FALSE);

    package->script->CurrentlyScripting = FALSE;

    /* process the ending type action */
    if (rc == ERROR_SUCCESS)
        ACTION_PerformActionSequence(package, -1);
    else if (rc == ERROR_INSTALL_USEREXIT)
        ACTION_PerformActionSequence(package, -2);
    else if (rc == ERROR_INSTALL_SUSPEND)
        ACTION_PerformActionSequence(package, -4);
    else  /* failed */
        ACTION_PerformActionSequence(package, -3);

    /* finish up running custom actions */
    ACTION_FinishCustomActions(package);

    if (rc == ERROR_SUCCESS && package->need_reboot)
        return ERROR_SUCCESS_REBOOT_REQUIRED;

    return rc;
}
