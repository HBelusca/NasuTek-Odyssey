/*
 * Implementation of the Microsoft Installer (msi.dll)
 *
 * Copyright 2004 Aric Stewart for CodeWeavers
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

#define NONAMELESSUNION
#define NONAMELESSSTRUCT
#define COBJMACROS

#include <stdarg.h>
#include "windef.h"
#include "winbase.h"
#include "winreg.h"
#include "winnls.h"
#include "shlwapi.h"
#include "wingdi.h"
#include "wine/debug.h"
#include "msi.h"
#include "msiquery.h"
#include "objidl.h"
#include "wincrypt.h"
#include "winuser.h"
#include "wininet.h"
#include "winver.h"
#include "urlmon.h"
#include "shlobj.h"
#include "wine/unicode.h"
#include "objbase.h"
#include "msidefs.h"
#include "sddl.h"

#include "msipriv.h"
#include "msiserver.h"

WINE_DEFAULT_DEBUG_CHANNEL(msi);

static void remove_tracked_tempfiles( MSIPACKAGE *package )
{
    struct list *item, *cursor;

    LIST_FOR_EACH_SAFE( item, cursor, &package->tempfiles )
    {
        MSITEMPFILE *temp = LIST_ENTRY( item, MSITEMPFILE, entry );

        list_remove( &temp->entry );
        TRACE("deleting temp file %s\n", debugstr_w( temp->Path ));
        DeleteFileW( temp->Path );
        msi_free( temp->Path );
        msi_free( temp );
    }
}

static void free_feature( MSIFEATURE *feature )
{
    struct list *item, *cursor;

    LIST_FOR_EACH_SAFE( item, cursor, &feature->Children )
    {
        FeatureList *fl = LIST_ENTRY( item, FeatureList, entry );
        list_remove( &fl->entry );
        msi_free( fl );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &feature->Components )
    {
        ComponentList *cl = LIST_ENTRY( item, ComponentList, entry );
        list_remove( &cl->entry );
        msi_free( cl );
    }
    msi_free( feature->Feature );
    msi_free( feature->Feature_Parent );
    msi_free( feature->Directory );
    msi_free( feature->Description );
    msi_free( feature->Title );
    msi_free( feature );
}

static void free_extension( MSIEXTENSION *ext )
{
    struct list *item, *cursor;

    LIST_FOR_EACH_SAFE( item, cursor, &ext->verbs )
    {
        MSIVERB *verb = LIST_ENTRY( item, MSIVERB, entry );

        list_remove( &verb->entry );
        msi_free( verb->Verb );
        msi_free( verb->Command );
        msi_free( verb->Argument );
        msi_free( verb );
    }

    msi_free( ext->Extension );
    msi_free( ext->ProgIDText );
    msi_free( ext );
}

static void free_assembly( MSIASSEMBLY *assembly )
{
    msi_free( assembly->feature );
    msi_free( assembly->manifest );
    msi_free( assembly->application );
    msi_free( assembly->display_name );
    if (assembly->tempdir) RemoveDirectoryW( assembly->tempdir );
    msi_free( assembly->tempdir );
    msi_free( assembly );
}

static void free_package_structures( MSIPACKAGE *package )
{
    INT i;
    struct list *item, *cursor;

    TRACE("Freeing package action data\n");

    remove_tracked_tempfiles(package);

    LIST_FOR_EACH_SAFE( item, cursor, &package->features )
    {
        MSIFEATURE *feature = LIST_ENTRY( item, MSIFEATURE, entry );
        list_remove( &feature->entry );
        free_feature( feature );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->folders )
    {
        MSIFOLDER *folder = LIST_ENTRY( item, MSIFOLDER, entry );

        list_remove( &folder->entry );
        msi_free( folder->Parent );
        msi_free( folder->Directory );
        msi_free( folder->TargetDefault );
        msi_free( folder->SourceLongPath );
        msi_free( folder->SourceShortPath );
        msi_free( folder->ResolvedTarget );
        msi_free( folder->ResolvedSource );
        msi_free( folder->Property );
        msi_free( folder );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->components )
    {
        MSICOMPONENT *comp = LIST_ENTRY( item, MSICOMPONENT, entry );

        list_remove( &comp->entry );
        msi_free( comp->Component );
        msi_free( comp->ComponentId );
        msi_free( comp->Directory );
        msi_free( comp->Condition );
        msi_free( comp->KeyPath );
        msi_free( comp->FullKeypath );
        if (comp->assembly) free_assembly( comp->assembly );
        msi_free( comp );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->files )
    {
        MSIFILE *file = LIST_ENTRY( item, MSIFILE, entry );

        list_remove( &file->entry );
        msi_free( file->File );
        msi_free( file->FileName );
        msi_free( file->ShortName );
        msi_free( file->LongName );
        msi_free( file->Version );
        msi_free( file->Language );
        msi_free( file->TargetPath );
        msi_free( file );
    }

    /* clean up extension, progid, class and verb structures */
    LIST_FOR_EACH_SAFE( item, cursor, &package->classes )
    {
        MSICLASS *cls = LIST_ENTRY( item, MSICLASS, entry );

        list_remove( &cls->entry );
        msi_free( cls->clsid );
        msi_free( cls->Context );
        msi_free( cls->Description );
        msi_free( cls->FileTypeMask );
        msi_free( cls->IconPath );
        msi_free( cls->DefInprocHandler );
        msi_free( cls->DefInprocHandler32 );
        msi_free( cls->Argument );
        msi_free( cls->ProgIDText );
        msi_free( cls );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->extensions )
    {
        MSIEXTENSION *ext = LIST_ENTRY( item, MSIEXTENSION, entry );

        list_remove( &ext->entry );
        free_extension( ext );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->progids )
    {
        MSIPROGID *progid = LIST_ENTRY( item, MSIPROGID, entry );

        list_remove( &progid->entry );
        msi_free( progid->ProgID );
        msi_free( progid->Description );
        msi_free( progid->IconPath );
        msi_free( progid );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->mimes )
    {
        MSIMIME *mt = LIST_ENTRY( item, MSIMIME, entry );

        list_remove( &mt->entry );
        msi_free( mt->suffix );
        msi_free( mt->clsid );
        msi_free( mt->ContentType );
        msi_free( mt );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->appids )
    {
        MSIAPPID *appid = LIST_ENTRY( item, MSIAPPID, entry );

        list_remove( &appid->entry );
        msi_free( appid->AppID );
        msi_free( appid->RemoteServerName );
        msi_free( appid->LocalServer );
        msi_free( appid->ServiceParameters );
        msi_free( appid->DllSurrogate );
        msi_free( appid );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->sourcelist_info )
    {
        MSISOURCELISTINFO *info = LIST_ENTRY( item, MSISOURCELISTINFO, entry );

        list_remove( &info->entry );
        msi_free( info->value );
        msi_free( info );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->sourcelist_media )
    {
        MSIMEDIADISK *info = LIST_ENTRY( item, MSIMEDIADISK, entry );

        list_remove( &info->entry );
        msi_free( info->volume_label );
        msi_free( info->disk_prompt );
        msi_free( info );
    }

    if (package->script)
    {
        for (i = 0; i < TOTAL_SCRIPTS; i++)
            msi_free_action_script( package, i );

        for (i = 0; i < package->script->UniqueActionsCount; i++)
            msi_free( package->script->UniqueActions[i] );

        msi_free( package->script->UniqueActions );
        msi_free( package->script );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->patches )
    {
        MSIPATCHINFO *patch = LIST_ENTRY( item, MSIPATCHINFO, entry );

        list_remove( &patch->entry );
        msi_free( patch->patchcode );
        msi_free( patch->transforms );
        msi_free( patch->localfile );
        msi_free( patch );
    }

    LIST_FOR_EACH_SAFE( item, cursor, &package->binaries )
    {
        MSIBINARY *binary = LIST_ENTRY( item, MSIBINARY, entry );

        list_remove( &binary->entry );
        if (binary->module)
            FreeLibrary( binary->module );
        if (!DeleteFileW( binary->tmpfile ))
            ERR("failed to delete %s (%u)\n", debugstr_w(binary->tmpfile), GetLastError());
        msi_free( binary->source );
        msi_free( binary->tmpfile );
        msi_free( binary );
    }

    msi_free( package->BaseURL );
    msi_free( package->PackagePath );
    msi_free( package->ProductCode );
    msi_free( package->ActionFormat );
    msi_free( package->LastAction );
    msi_free( package->langids );

    /* cleanup control event subscriptions */
    ControlEvent_CleanupSubscriptions( package );
}

static void MSI_FreePackage( MSIOBJECTHDR *arg)
{
    UINT i;
    MSIPACKAGE *package = (MSIPACKAGE *)arg;

    if( package->dialog )
        msi_dialog_destroy( package->dialog );

    msiobj_release( &package->db->hdr );
    free_package_structures(package);
    CloseHandle( package->log_file );

    for (i = 0; i < CLR_VERSION_MAX; i++)
        if (package->cache_net[i]) IAssemblyCache_Release( package->cache_net[i] );
    if (package->cache_sxs) IAssemblyCache_Release( package->cache_sxs );
}

static UINT create_temp_property_table(MSIPACKAGE *package)
{
    MSIQUERY *view = NULL;
    UINT rc;

    static const WCHAR CreateSql[] = {
       'C','R','E','A','T','E',' ','T','A','B','L','E',' ',
       '`','_','P','r','o','p','e','r','t','y','`',' ','(',' ',
       '`','_','P','r','o','p','e','r','t','y','`',' ',
       'C','H','A','R','(','5','6',')',' ','N','O','T',' ','N','U','L','L',' ',
       'T','E','M','P','O','R','A','R','Y',',',' ',
       '`','V','a','l','u','e','`',' ','C','H','A','R','(','9','8',')',' ',
       'N','O','T',' ','N','U','L','L',' ','T','E','M','P','O','R','A','R','Y',
       ' ','P','R','I','M','A','R','Y',' ','K','E','Y',' ',
       '`','_','P','r','o','p','e','r','t','y','`',')',' ','H','O','L','D',0};

    rc = MSI_DatabaseOpenViewW(package->db, CreateSql, &view);
    if (rc != ERROR_SUCCESS)
        return rc;

    rc = MSI_ViewExecute(view, 0);
    MSI_ViewClose(view);
    msiobj_release(&view->hdr);
    return rc;
}

UINT msi_clone_properties(MSIPACKAGE *package)
{
    MSIQUERY *view_select = NULL;
    UINT rc;

    static const WCHAR query_select[] = {
       'S','E','L','E','C','T',' ','*',' ',
       'F','R','O','M',' ','`','P','r','o','p','e','r','t','y','`',0};
    static const WCHAR query_insert[] = {
       'I','N','S','E','R','T',' ','i','n','t','o',' ',
       '`','_','P','r','o','p','e','r','t','y','`',' ',
       '(','`','_','P','r','o','p','e','r','t','y','`',',',
       '`','V','a','l','u','e','`',')',' ',
       'V','A','L','U','E','S',' ','(','?',',','?',')',0};
    static const WCHAR query_update[] = {
        'U','P','D','A','T','E',' ','`','_','P','r','o','p','e','r','t','y','`',' ',
        'S','E','T',' ','`','V','a','l','u','e','`',' ','=',' ','?',' ',
        'W','H','E','R','E',' ','`','_','P','r','o','p','e','r','t','y','`',' ','=',' ','?',0};

    rc = MSI_DatabaseOpenViewW( package->db, query_select, &view_select );
    if (rc != ERROR_SUCCESS)
        return rc;

    rc = MSI_ViewExecute( view_select, 0 );
    if (rc != ERROR_SUCCESS)
    {
        MSI_ViewClose( view_select );
        msiobj_release( &view_select->hdr );
        return rc;
    }

    while (1)
    {
        MSIQUERY *view_insert, *view_update;
        MSIRECORD *rec_select;

        rc = MSI_ViewFetch( view_select, &rec_select );
        if (rc != ERROR_SUCCESS)
            break;

        rc = MSI_DatabaseOpenViewW( package->db, query_insert, &view_insert );
        if (rc != ERROR_SUCCESS)
        {
            msiobj_release( &rec_select->hdr );
            continue;
        }

        rc = MSI_ViewExecute( view_insert, rec_select );
        MSI_ViewClose( view_insert );
        msiobj_release( &view_insert->hdr );
        if (rc != ERROR_SUCCESS)
        {
            MSIRECORD *rec_update;

            TRACE("insert failed, trying update\n");

            rc = MSI_DatabaseOpenViewW( package->db, query_update, &view_update );
            if (rc != ERROR_SUCCESS)
            {
                WARN("open view failed %u\n", rc);
                msiobj_release( &rec_select->hdr );
                continue;
            }

            rec_update = MSI_CreateRecord( 2 );
            MSI_RecordCopyField( rec_select, 1, rec_update, 2 );
            MSI_RecordCopyField( rec_select, 2, rec_update, 1 );
            rc = MSI_ViewExecute( view_update, rec_update );
            if (rc != ERROR_SUCCESS)
                WARN("update failed %u\n", rc);

            MSI_ViewClose( view_update );
            msiobj_release( &view_update->hdr );
            msiobj_release( &rec_update->hdr );
        }

        msiobj_release( &rec_select->hdr );
    }

    MSI_ViewClose( view_select );
    msiobj_release( &view_select->hdr );
    return rc;
}

/*
 * set_installed_prop
 *
 * Sets the "Installed" property to indicate that
 *  the product is installed for the current user.
 */
static UINT set_installed_prop( MSIPACKAGE *package )
{
    HKEY hkey = 0;
    UINT r;

    r = MSIREG_OpenUninstallKey( package, &hkey, FALSE );
    if (r == ERROR_SUCCESS)
    {
        RegCloseKey( hkey );
        msi_set_property( package->db, szInstalled, szOne );
    }

    return r;
}

static UINT set_user_sid_prop( MSIPACKAGE *package )
{
    SID_NAME_USE use;
    LPWSTR user_name;
    LPWSTR sid_str = NULL, dom = NULL;
    DWORD size, dom_size;
    PSID psid = NULL;
    UINT r = ERROR_FUNCTION_FAILED;

    size = 0;
    GetUserNameW( NULL, &size );

    user_name = msi_alloc( (size + 1) * sizeof(WCHAR) );
    if (!user_name)
        return ERROR_OUTOFMEMORY;

    if (!GetUserNameW( user_name, &size ))
        goto done;

    size = 0;
    dom_size = 0;
    LookupAccountNameW( NULL, user_name, NULL, &size, NULL, &dom_size, &use );

    psid = msi_alloc( size );
    dom = msi_alloc( dom_size*sizeof (WCHAR) );
    if (!psid || !dom)
    {
        r = ERROR_OUTOFMEMORY;
        goto done;
    }

    if (!LookupAccountNameW( NULL, user_name, psid, &size, dom, &dom_size, &use ))
        goto done;

    if (!ConvertSidToStringSidW( psid, &sid_str ))
        goto done;

    r = msi_set_property( package->db, szUserSID, sid_str );

done:
    LocalFree( sid_str );
    msi_free( dom );
    msi_free( psid );
    msi_free( user_name );

    return r;
}

static LPWSTR get_fusion_filename(MSIPACKAGE *package)
{
    HKEY netsetup;
    LONG res;
    LPWSTR file = NULL;
    DWORD index = 0, size;
    WCHAR ver[MAX_PATH];
    WCHAR name[MAX_PATH];
    WCHAR windir[MAX_PATH];

    static const WCHAR fusion[] = {'f','u','s','i','o','n','.','d','l','l',0};
    static const WCHAR sub[] = {
        'S','o','f','t','w','a','r','e','\\',
        'M','i','c','r','o','s','o','f','t','\\',
        'N','E','T',' ','F','r','a','m','e','w','o','r','k',' ','S','e','t','u','p','\\',
        'N','D','P',0
    };
    static const WCHAR subdir[] = {
        'M','i','c','r','o','s','o','f','t','.','N','E','T','\\',
        'F','r','a','m','e','w','o','r','k','\\',0
    };

    res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, sub, 0, KEY_ENUMERATE_SUB_KEYS, &netsetup);
    if (res != ERROR_SUCCESS)
        return NULL;

    GetWindowsDirectoryW(windir, MAX_PATH);

    ver[0] = '\0';
    size = MAX_PATH;
    while (RegEnumKeyExW(netsetup, index, name, &size, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        index++;

        /* verify existence of fusion.dll .Net 3.0 does not install a new one */
        if (strcmpW( ver, name ) < 0)
        {
            LPWSTR check;
            size = lstrlenW(windir) + lstrlenW(subdir) + lstrlenW(name) +lstrlenW(fusion) + 3;
            check = msi_alloc(size * sizeof(WCHAR));

            if (!check)
            {
                msi_free(file);
                return NULL;
            }

            lstrcpyW(check, windir);
            lstrcatW(check, szBackSlash);
            lstrcatW(check, subdir);
            lstrcatW(check, name);
            lstrcatW(check, szBackSlash);
            lstrcatW(check, fusion);

            if(GetFileAttributesW(check) != INVALID_FILE_ATTRIBUTES)
            {
                msi_free(file);
                file = check;
                lstrcpyW(ver, name);
            }
            else
                msi_free(check);
        }
    }

    RegCloseKey(netsetup);
    return file;
}

typedef struct tagLANGANDCODEPAGE
{
  WORD wLanguage;
  WORD wCodePage;
} LANGANDCODEPAGE;

static void set_msi_assembly_prop(MSIPACKAGE *package)
{
    UINT val_len;
    DWORD size, handle;
    LPVOID version = NULL;
    WCHAR buf[MAX_PATH];
    LPWSTR fusion, verstr;
    LANGANDCODEPAGE *translate;

    static const WCHAR netasm[] = {
        'M','s','i','N','e','t','A','s','s','e','m','b','l','y','S','u','p','p','o','r','t',0
    };
    static const WCHAR translation[] = {
        '\\','V','a','r','F','i','l','e','I','n','f','o',
        '\\','T','r','a','n','s','l','a','t','i','o','n',0
    };
    static const WCHAR verfmt[] = {
        '\\','S','t','r','i','n','g','F','i','l','e','I','n','f','o',
        '\\','%','0','4','x','%','0','4','x',
        '\\','P','r','o','d','u','c','t','V','e','r','s','i','o','n',0
    };

    fusion = get_fusion_filename(package);
    if (!fusion)
        return;

    size = GetFileVersionInfoSizeW(fusion, &handle);
    if (!size) return;

    version = msi_alloc(size);
    if (!version) return;

    if (!GetFileVersionInfoW(fusion, handle, size, version))
        goto done;

    if (!VerQueryValueW(version, translation, (LPVOID *)&translate, &val_len))
        goto done;

    sprintfW(buf, verfmt, translate[0].wLanguage, translate[0].wCodePage);

    if (!VerQueryValueW(version, buf, (LPVOID *)&verstr, &val_len))
        goto done;

    if (!val_len || !verstr)
        goto done;

    msi_set_property(package->db, netasm, verstr);

done:
    msi_free(fusion);
    msi_free(version);
}

static VOID set_installer_properties(MSIPACKAGE *package)
{
    WCHAR pth[MAX_PATH];
    WCHAR *ptr;
    OSVERSIONINFOEXW OSVersion;
    MEMORYSTATUSEX msex;
    DWORD verval, len;
    WCHAR verstr[10], bufstr[20];
    HDC dc;
    HKEY hkey;
    LPWSTR username, companyname;
    SYSTEM_INFO sys_info;
    SYSTEMTIME systemtime;
    LANGID langid;

    static const WCHAR szCommonFilesFolder[] = {'C','o','m','m','o','n','F','i','l','e','s','F','o','l','d','e','r',0};
    static const WCHAR szProgramFilesFolder[] = {'P','r','o','g','r','a','m','F','i','l','e','s','F','o','l','d','e','r',0};
    static const WCHAR szCommonAppDataFolder[] = {'C','o','m','m','o','n','A','p','p','D','a','t','a','F','o','l','d','e','r',0};
    static const WCHAR szFavoritesFolder[] = {'F','a','v','o','r','i','t','e','s','F','o','l','d','e','r',0};
    static const WCHAR szFontsFolder[] = {'F','o','n','t','s','F','o','l','d','e','r',0};
    static const WCHAR szSendToFolder[] = {'S','e','n','d','T','o','F','o','l','d','e','r',0};
    static const WCHAR szStartMenuFolder[] = {'S','t','a','r','t','M','e','n','u','F','o','l','d','e','r',0};
    static const WCHAR szStartupFolder[] = {'S','t','a','r','t','u','p','F','o','l','d','e','r',0};
    static const WCHAR szTemplateFolder[] = {'T','e','m','p','l','a','t','e','F','o','l','d','e','r',0};
    static const WCHAR szDesktopFolder[] = {'D','e','s','k','t','o','p','F','o','l','d','e','r',0};
    static const WCHAR szProgramMenuFolder[] = {'P','r','o','g','r','a','m','M','e','n','u','F','o','l','d','e','r',0};
    static const WCHAR szAdminToolsFolder[] = {'A','d','m','i','n','T','o','o','l','s','F','o','l','d','e','r',0};
    static const WCHAR szAppDataFolder[] = {'A','p','p','D','a','t','a','F','o','l','d','e','r',0};
    static const WCHAR szSystemFolder[] = {'S','y','s','t','e','m','F','o','l','d','e','r',0};
    static const WCHAR szSystem16Folder[] = {'S','y','s','t','e','m','1','6','F','o','l','d','e','r',0};
    static const WCHAR szLocalAppDataFolder[] = {'L','o','c','a','l','A','p','p','D','a','t','a','F','o','l','d','e','r',0};
    static const WCHAR szMyPicturesFolder[] = {'M','y','P','i','c','t','u','r','e','s','F','o','l','d','e','r',0};
    static const WCHAR szPersonalFolder[] = {'P','e','r','s','o','n','a','l','F','o','l','d','e','r',0};
    static const WCHAR szWindowsFolder[] = {'W','i','n','d','o','w','s','F','o','l','d','e','r',0};
    static const WCHAR szWindowsVolume[] = {'W','i','n','d','o','w','s','V','o','l','u','m','e',0};
    static const WCHAR szTempFolder[]= {'T','e','m','p','F','o','l','d','e','r',0};
    static const WCHAR szPrivileged[] = {'P','r','i','v','i','l','e','g','e','d',0};
    static const WCHAR szVersion9x[] = {'V','e','r','s','i','o','n','9','X',0};
    static const WCHAR szVersionNT[] = {'V','e','r','s','i','o','n','N','T',0};
    static const WCHAR szMsiNTProductType[] = {'M','s','i','N','T','P','r','o','d','u','c','t','T','y','p','e',0};
    static const WCHAR szFormat[] = {'%','l','i',0};
    static const WCHAR szWindowsBuild[] = {'W','i','n','d','o','w','s','B','u','i','l','d',0};
    static const WCHAR szServicePackLevel[] = {'S','e','r','v','i','c','e','P','a','c','k','L','e','v','e','l',0};
    static const WCHAR szSix[] = {'6',0 };
    static const WCHAR szVersionMsi[] = { 'V','e','r','s','i','o','n','M','s','i',0 };
    static const WCHAR szVersionDatabase[] = { 'V','e','r','s','i','o','n','D','a','t','a','b','a','s','e',0 };
    static const WCHAR szPhysicalMemory[] = { 'P','h','y','s','i','c','a','l','M','e','m','o','r','y',0 };
    static const WCHAR szFormat2[] = {'%','l','i','.','%','l','i',0};
    static const WCHAR szScreenX[] = {'S','c','r','e','e','n','X',0};
    static const WCHAR szScreenY[] = {'S','c','r','e','e','n','Y',0};
    static const WCHAR szColorBits[] = {'C','o','l','o','r','B','i','t','s',0};
    static const WCHAR szIntFormat[] = {'%','d',0};
    static const WCHAR szMsiAMD64[] = { 'M','s','i','A','M','D','6','4',0 };
    static const WCHAR szMsix64[] = { 'M','s','i','x','6','4',0 };
    static const WCHAR szSystem64Folder[] = { 'S','y','s','t','e','m','6','4','F','o','l','d','e','r',0 };
    static const WCHAR szCommonFiles64Folder[] = { 'C','o','m','m','o','n','F','i','l','e','s','6','4','F','o','l','d','e','r',0 };
    static const WCHAR szProgramFiles64Folder[] = { 'P','r','o','g','r','a','m','F','i','l','e','s','6','4','F','o','l','d','e','r',0 };
    static const WCHAR szVersionNT64[] = { 'V','e','r','s','i','o','n','N','T','6','4',0 };
    static const WCHAR szUserInfo[] = {
        'S','O','F','T','W','A','R','E','\\',
        'M','i','c','r','o','s','o','f','t','\\',
        'M','S',' ','S','e','t','u','p',' ','(','A','C','M','E',')','\\',
        'U','s','e','r',' ','I','n','f','o',0
    };
    static const WCHAR szDefName[] = { 'D','e','f','N','a','m','e',0 };
    static const WCHAR szDefCompany[] = { 'D','e','f','C','o','m','p','a','n','y',0 };
    static const WCHAR szCurrentVersion[] = {
        'S','O','F','T','W','A','R','E','\\',
        'M','i','c','r','o','s','o','f','t','\\',
        'W','i','n','d','o','w','s',' ','N','T','\\',
        'C','u','r','r','e','n','t','V','e','r','s','i','o','n',0
    };
    static const WCHAR szRegisteredUser[] = {'R','e','g','i','s','t','e','r','e','d','O','w','n','e','r',0};
    static const WCHAR szRegisteredOrganization[] = {
        'R','e','g','i','s','t','e','r','e','d','O','r','g','a','n','i','z','a','t','i','o','n',0
    };
    static const WCHAR szUSERNAME[] = {'U','S','E','R','N','A','M','E',0};
    static const WCHAR szCOMPANYNAME[] = {'C','O','M','P','A','N','Y','N','A','M','E',0};
    static const WCHAR szDate[] = {'D','a','t','e',0};
    static const WCHAR szTime[] = {'T','i','m','e',0};
    static const WCHAR szUserLanguageID[] = {'U','s','e','r','L','a','n','g','u','a','g','e','I','D',0};
    static const WCHAR szSystemLangID[] = {'S','y','s','t','e','m','L','a','n','g','u','a','g','e','I','D',0};
    static const WCHAR szProductState[] = {'P','r','o','d','u','c','t','S','t','a','t','e',0};
    static const WCHAR szLogonUser[] = {'L','o','g','o','n','U','s','e','r',0};
    static const WCHAR szNetHoodFolder[] = {'N','e','t','H','o','o','d','F','o','l','d','e','r',0};
    static const WCHAR szPrintHoodFolder[] = {'P','r','i','n','t','H','o','o','d','F','o','l','d','e','r',0};
    static const WCHAR szRecentFolder[] = {'R','e','c','e','n','t','F','o','l','d','e','r',0};

    /*
     * Other things that probably should be set:
     *
     * ComputerName VirtualMemory
     * ShellAdvSupport DefaultUIFont PackagecodeChanging
     * CaptionHeight BorderTop BorderSide TextHeight
     * RedirectedDllSupport
     */

    SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szCommonAppDataFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_FAVORITES, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szFavoritesFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_FONTS, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szFontsFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_SENDTO, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szSendToFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_STARTMENU, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szStartMenuFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szStartupFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_TEMPLATES, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szTemplateFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szDesktopFolder, pth);

    /* FIXME: set to AllUsers profile path if ALLUSERS is set */
    SHGetFolderPathW(NULL, CSIDL_PROGRAMS, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szProgramMenuFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_ADMINTOOLS, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szAdminToolsFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szAppDataFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_SYSTEM, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szSystemFolder, pth);
    msi_set_property(package->db, szSystem16Folder, pth);

    SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szLocalAppDataFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_MYPICTURES, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szMyPicturesFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szPersonalFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_WINDOWS, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szWindowsFolder, pth);
    
    SHGetFolderPathW(NULL, CSIDL_PRINTHOOD, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szPrintHoodFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_NETHOOD, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szNetHoodFolder, pth);

    SHGetFolderPathW(NULL, CSIDL_RECENT, NULL, 0, pth);
    strcatW(pth, szBackSlash);
    msi_set_property(package->db, szRecentFolder, pth);

    /* Physical Memory is specified in MB. Using total amount. */
    msex.dwLength = sizeof(msex);
    GlobalMemoryStatusEx( &msex );
    sprintfW( bufstr, szIntFormat, (int)(msex.ullTotalPhys / 1024 / 1024) );
    msi_set_property(package->db, szPhysicalMemory, bufstr);

    SHGetFolderPathW(NULL, CSIDL_WINDOWS, NULL, 0, pth);
    ptr = strchrW(pth,'\\');
    if (ptr) *(ptr + 1) = 0;
    msi_set_property(package->db, szWindowsVolume, pth);
    
    GetTempPathW(MAX_PATH,pth);
    msi_set_property(package->db, szTempFolder, pth);

    /* in a wine environment the user is always admin and privileged */
    msi_set_property(package->db, szAdminUser, szOne);
    msi_set_property(package->db, szPrivileged, szOne);

    /* set the os things */
    OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((OSVERSIONINFOW *)&OSVersion);
    verval = OSVersion.dwMinorVersion + OSVersion.dwMajorVersion * 100;
    sprintfW(verstr, szFormat, verval);
    switch (OSVersion.dwPlatformId)
    {
        case VER_PLATFORM_WIN32_WINDOWS:    
            msi_set_property(package->db, szVersion9x, verstr);
            break;
        case VER_PLATFORM_WIN32_NT:
            msi_set_property(package->db, szVersionNT, verstr);
            sprintfW(verstr, szFormat,OSVersion.wProductType);
            msi_set_property(package->db, szMsiNTProductType, verstr);
            break;
    }
    sprintfW(verstr, szFormat, OSVersion.dwBuildNumber);
    msi_set_property(package->db, szWindowsBuild, verstr);
    /* just fudge this */
    msi_set_property(package->db, szServicePackLevel, szSix);

    sprintfW( bufstr, szFormat2, MSI_MAJORVERSION, MSI_MINORVERSION);
    msi_set_property( package->db, szVersionMsi, bufstr );
    sprintfW( bufstr, szFormat, MSI_MAJORVERSION * 100);
    msi_set_property( package->db, szVersionDatabase, bufstr );

    GetNativeSystemInfo( &sys_info );
    sprintfW( bufstr, szIntFormat, sys_info.wProcessorLevel );
    if (sys_info.u.s.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    {
        msi_set_property( package->db, szIntel, bufstr );

        GetSystemDirectoryW( pth, MAX_PATH );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szSystemFolder, pth );

        SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES, NULL, 0, pth );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szProgramFilesFolder, pth );

        SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, pth );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szCommonFilesFolder, pth );
    }
    else if (sys_info.u.s.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    {
        msi_set_property( package->db, szMsiAMD64, bufstr );
        msi_set_property( package->db, szMsix64, bufstr );
        msi_set_property( package->db, szVersionNT64, verstr );

        GetSystemDirectoryW( pth, MAX_PATH );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szSystem64Folder, pth );

        GetSystemWow64DirectoryW( pth, MAX_PATH );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szSystemFolder, pth );

        SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES, NULL, 0, pth );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szProgramFiles64Folder, pth );

        SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, pth );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szProgramFilesFolder, pth );

        SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, pth );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szCommonFiles64Folder, pth );

        SHGetFolderPathW( NULL, CSIDL_PROGRAM_FILES_COMMONX86, NULL, 0, pth );
        PathAddBackslashW( pth );
        msi_set_property( package->db, szCommonFilesFolder, pth );
    }

    /* Screen properties. */
    dc = GetDC(0);
    sprintfW( bufstr, szIntFormat, GetDeviceCaps( dc, HORZRES ) );
    msi_set_property( package->db, szScreenX, bufstr );
    sprintfW( bufstr, szIntFormat, GetDeviceCaps( dc, VERTRES ));
    msi_set_property( package->db, szScreenY, bufstr );
    sprintfW( bufstr, szIntFormat, GetDeviceCaps( dc, BITSPIXEL ));
    msi_set_property( package->db, szColorBits, bufstr );
    ReleaseDC(0, dc);

    /* USERNAME and COMPANYNAME */
    username = msi_dup_property( package->db, szUSERNAME );
    companyname = msi_dup_property( package->db, szCOMPANYNAME );

    if ((!username || !companyname) &&
        RegOpenKeyW( HKEY_CURRENT_USER, szUserInfo, &hkey ) == ERROR_SUCCESS)
    {
        if (!username &&
            (username = msi_reg_get_val_str( hkey, szDefName )))
            msi_set_property( package->db, szUSERNAME, username );
        if (!companyname &&
            (companyname = msi_reg_get_val_str( hkey, szDefCompany )))
            msi_set_property( package->db, szCOMPANYNAME, companyname );
        CloseHandle( hkey );
    }
    if ((!username || !companyname) &&
        RegOpenKeyW( HKEY_LOCAL_MACHINE, szCurrentVersion, &hkey ) == ERROR_SUCCESS)
    {
        if (!username &&
            (username = msi_reg_get_val_str( hkey, szRegisteredUser )))
            msi_set_property( package->db, szUSERNAME, username );
        if (!companyname &&
            (companyname = msi_reg_get_val_str( hkey, szRegisteredOrganization )))
            msi_set_property( package->db, szCOMPANYNAME, companyname );
        CloseHandle( hkey );
    }
    msi_free( username );
    msi_free( companyname );

    if ( set_user_sid_prop( package ) != ERROR_SUCCESS)
        ERR("Failed to set the UserSID property\n");

    /* Date and time properties */
    GetSystemTime( &systemtime );
    if (GetDateFormatW( LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systemtime,
                        NULL, bufstr, sizeof(bufstr)/sizeof(bufstr[0]) ))
        msi_set_property( package->db, szDate, bufstr );
    else
        ERR("Couldn't set Date property: GetDateFormat failed with error %d\n", GetLastError());

    if (GetTimeFormatW( LOCALE_USER_DEFAULT,
                        TIME_FORCE24HOURFORMAT | TIME_NOTIMEMARKER,
                        &systemtime, NULL, bufstr,
                        sizeof(bufstr)/sizeof(bufstr[0]) ))
        msi_set_property( package->db, szTime, bufstr );
    else
        ERR("Couldn't set Time property: GetTimeFormat failed with error %d\n", GetLastError());

    set_msi_assembly_prop( package );

    langid = GetUserDefaultLangID();
    sprintfW(bufstr, szIntFormat, langid);
    msi_set_property( package->db, szUserLanguageID, bufstr );

    langid = GetSystemDefaultLangID();
    sprintfW(bufstr, szIntFormat, langid);
    msi_set_property( package->db, szSystemLangID, bufstr );

    sprintfW(bufstr, szIntFormat, MsiQueryProductStateW(package->ProductCode));
    msi_set_property( package->db, szProductState, bufstr );

    len = 0;
    if (!GetUserNameW( NULL, &len ) && GetLastError() == ERROR_MORE_DATA)
    {
        WCHAR *username;
        if ((username = msi_alloc( len * sizeof(WCHAR) )))
        {
            if (GetUserNameW( username, &len ))
                msi_set_property( package->db, szLogonUser, username );
            msi_free( username );
        }
    }
}

static UINT msi_load_summary_properties( MSIPACKAGE *package )
{
    UINT rc;
    MSIHANDLE suminfo;
    MSIHANDLE hdb = alloc_msihandle( &package->db->hdr );
    INT count;
    DWORD len;
    LPWSTR package_code;
    static const WCHAR szPackageCode[] = {
        'P','a','c','k','a','g','e','C','o','d','e',0};

    if (!hdb) {
        ERR("Unable to allocate handle\n");
        return ERROR_OUTOFMEMORY;
    }

    rc = MsiGetSummaryInformationW( hdb, NULL, 0, &suminfo );
    MsiCloseHandle(hdb);
    if (rc != ERROR_SUCCESS)
    {
        ERR("Unable to open Summary Information\n");
        return rc;
    }

    rc = MsiSummaryInfoGetPropertyW( suminfo, PID_PAGECOUNT, NULL,
                                     &count, NULL, NULL, NULL );
    if (rc != ERROR_SUCCESS)
    {
        WARN("Unable to query page count: %d\n", rc);
        goto done;
    }

    /* load package code property */
    len = 0;
    rc = MsiSummaryInfoGetPropertyW( suminfo, PID_REVNUMBER, NULL,
                                     NULL, NULL, NULL, &len );
    if (rc != ERROR_MORE_DATA)
    {
        WARN("Unable to query revision number: %d\n", rc);
        rc = ERROR_FUNCTION_FAILED;
        goto done;
    }

    len++;
    package_code = msi_alloc( len * sizeof(WCHAR) );
    rc = MsiSummaryInfoGetPropertyW( suminfo, PID_REVNUMBER, NULL,
                                     NULL, NULL, package_code, &len );
    if (rc != ERROR_SUCCESS)
    {
        WARN("Unable to query rev number: %d\n", rc);
        goto done;
    }

    msi_set_property( package->db, szPackageCode, package_code );
    msi_free( package_code );

    /* load package attributes */
    count = 0;
    MsiSummaryInfoGetPropertyW( suminfo, PID_WORDCOUNT, NULL,
                                &count, NULL, NULL, NULL );
    package->WordCount = count;

done:
    MsiCloseHandle(suminfo);
    return rc;
}

static MSIPACKAGE *msi_alloc_package( void )
{
    MSIPACKAGE *package;

    package = alloc_msiobject( MSIHANDLETYPE_PACKAGE, sizeof (MSIPACKAGE),
                               MSI_FreePackage );
    if( package )
    {
        list_init( &package->components );
        list_init( &package->features );
        list_init( &package->files );
        list_init( &package->tempfiles );
        list_init( &package->folders );
        list_init( &package->subscriptions );
        list_init( &package->appids );
        list_init( &package->classes );
        list_init( &package->mimes );
        list_init( &package->extensions );
        list_init( &package->progids );
        list_init( &package->RunningActions );
        list_init( &package->sourcelist_info );
        list_init( &package->sourcelist_media );
        list_init( &package->patches );
        list_init( &package->binaries );
    }

    return package;
}

static UINT msi_load_admin_properties(MSIPACKAGE *package)
{
    BYTE *data;
    UINT r, sz;

    static const WCHAR stmname[] = {'A','d','m','i','n','P','r','o','p','e','r','t','i','e','s',0};

    r = read_stream_data(package->db->storage, stmname, FALSE, &data, &sz);
    if (r != ERROR_SUCCESS)
        return r;

    r = msi_parse_command_line(package, (WCHAR *)data, TRUE);

    msi_free(data);
    return r;
}

void msi_adjust_privilege_properties( MSIPACKAGE *package )
{
    /* FIXME: this should depend on the user's privileges */
    if (msi_get_property_int( package->db, szAllUsers, 0 ) == 2)
    {
        TRACE("resetting ALLUSERS property from 2 to 1\n");
        msi_set_property( package->db, szAllUsers, szOne );
    }
    msi_set_property( package->db, szAdminUser, szOne );
}

MSIPACKAGE *MSI_CreatePackage( MSIDATABASE *db, LPCWSTR base_url )
{
    static const WCHAR szLevel[] = { 'U','I','L','e','v','e','l',0 };
    static const WCHAR szpi[] = {'%','i',0};
    MSIPACKAGE *package;
    WCHAR uilevel[10];
    UINT r;

    TRACE("%p\n", db);

    package = msi_alloc_package();
    if (package)
    {
        msiobj_addref( &db->hdr );
        package->db = db;

        package->WordCount = 0;
        package->PackagePath = strdupW( db->path );
        package->BaseURL = strdupW( base_url );

        create_temp_property_table( package );
        msi_clone_properties( package );
        msi_adjust_privilege_properties( package );

        package->ProductCode = msi_dup_property( package->db, szProductCode );
        package->script = msi_alloc_zero( sizeof(MSISCRIPT) );

        set_installed_prop( package );
        set_installer_properties( package );

        sprintfW(uilevel,szpi,gUILevel);
        msi_set_property(package->db, szLevel, uilevel);

        r = msi_load_summary_properties( package );
        if (r != ERROR_SUCCESS)
        {
            msiobj_release( &package->hdr );
            return NULL;
        }

        if (package->WordCount & msidbSumInfoSourceTypeAdminImage)
            msi_load_admin_properties( package );

        package->log_file = INVALID_HANDLE_VALUE;
    }

    return package;
}

/*
 * copy_package_to_temp   [internal]
 *
 * copy the msi file to a temp file to prevent locking a CD
 * with a multi disc install 
 *
 * FIXME: I think this is wrong, and instead of copying the package,
 *        we should read all the tables to memory, then open the
 *        database to read binary streams on demand.
 */ 
static UINT copy_package_to_temp( LPCWSTR szPackage, LPWSTR filename )
{
    WCHAR path[MAX_PATH];

    GetTempPathW( MAX_PATH, path );
    GetTempFileNameW( path, szMsi, 0, filename );

    if( !CopyFileW( szPackage, filename, FALSE ) )
    {
        UINT error = GetLastError();
        if ( error == ERROR_FILE_NOT_FOUND )
            ERR("can't find %s\n", debugstr_w(szPackage));
        else
            ERR("failed to copy package %s to %s (%u)\n", debugstr_w(szPackage), debugstr_w(filename), error);
        DeleteFileW( filename );
        return error;
    }

    return ERROR_SUCCESS;
}

UINT msi_download_file( LPCWSTR szUrl, LPWSTR filename )
{
    LPINTERNET_CACHE_ENTRY_INFOW cache_entry;
    DWORD size = 0;
    HRESULT hr;

    /* call will always fail, becase size is 0,
     * but will return ERROR_FILE_NOT_FOUND first
     * if the file doesn't exist
     */
    GetUrlCacheEntryInfoW( szUrl, NULL, &size );
    if ( GetLastError() != ERROR_FILE_NOT_FOUND )
    {
        cache_entry = msi_alloc( size );
        if ( !GetUrlCacheEntryInfoW( szUrl, cache_entry, &size ) )
        {
            UINT error = GetLastError();
            msi_free( cache_entry );
            return error;
        }

        lstrcpyW( filename, cache_entry->lpszLocalFileName );
        msi_free( cache_entry );
        return ERROR_SUCCESS;
    }

    hr = URLDownloadToCacheFileW( NULL, szUrl, filename, MAX_PATH, 0, NULL );
    if ( FAILED(hr) )
    {
        WARN("failed to download %s to cache file\n", debugstr_w(szUrl));
        return ERROR_FUNCTION_FAILED;
    }

    return ERROR_SUCCESS;
}

UINT msi_get_local_package_name( LPWSTR path, LPCWSTR suffix )
{
    static const WCHAR szInstaller[] = {
        '\\','I','n','s','t','a','l','l','e','r','\\',0};
    static const WCHAR fmt[] = {'%','x',0};
    DWORD time, len, i, offset;
    HANDLE handle;

    time = GetTickCount();
    GetWindowsDirectoryW( path, MAX_PATH );
    strcatW( path, szInstaller );
    CreateDirectoryW( path, NULL );

    len = strlenW(path);
    for (i = 0; i < 0x10000; i++)
    {
        offset = snprintfW( path + len, MAX_PATH - len, fmt, (time + i) & 0xffff );
        memcpy( path + len + offset, suffix, (strlenW( suffix ) + 1) * sizeof(WCHAR) );
        handle = CreateFileW( path, GENERIC_WRITE, 0, NULL,
                              CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0 );
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
            break;
        }
        if (GetLastError() != ERROR_FILE_EXISTS &&
            GetLastError() != ERROR_SHARING_VIOLATION)
            return ERROR_FUNCTION_FAILED;
    }

    return ERROR_SUCCESS;
}

static UINT apply_registered_patch( MSIPACKAGE *package, LPCWSTR patch_code )
{
    UINT r;
    DWORD len;
    WCHAR patch_file[MAX_PATH];
    MSIDATABASE *patch_db;
    MSIPATCHINFO *patch_info;
    MSISUMMARYINFO *si;

    len = sizeof(patch_file) / sizeof(WCHAR);
    r = MsiGetPatchInfoExW( patch_code, package->ProductCode, NULL, package->Context,
                            INSTALLPROPERTY_LOCALPACKAGEW, patch_file, &len );
    if (r != ERROR_SUCCESS)
    {
        ERR("failed to get patch filename %u\n", r);
        return r;
    }

    r = MSI_OpenDatabaseW( patch_file, MSIDBOPEN_READONLY + MSIDBOPEN_PATCHFILE, &patch_db );
    if (r != ERROR_SUCCESS)
    {
        ERR("failed to open patch database %s\n", debugstr_w( patch_file ));
        return r;
    }

    si = MSI_GetSummaryInformationW( patch_db->storage, 0 );
    if (!si)
    {
        msiobj_release( &patch_db->hdr );
        return ERROR_FUNCTION_FAILED;
    }

    r = msi_parse_patch_summary( si, &patch_info );
    msiobj_release( &si->hdr );
    if (r != ERROR_SUCCESS)
    {
        ERR("failed to parse patch summary %u\n", r);
        msiobj_release( &patch_db->hdr );
        return r;
    }

    patch_info->localfile = strdupW( patch_file );
    if (!patch_info->localfile)
    {
        msiobj_release( &patch_db->hdr );
        return ERROR_OUTOFMEMORY;
    }

    r = msi_apply_patch_db( package, patch_db, patch_info );
    msiobj_release( &patch_db->hdr );
    if (r != ERROR_SUCCESS)
    {
        ERR("failed to apply patch %u\n", r);
        msi_free( patch_info->patchcode );
        msi_free( patch_info->transforms );
        msi_free( patch_info->localfile );
        msi_free( patch_info );
    }
    return r;
}

static UINT msi_parse_summary( MSISUMMARYINFO *si, MSIPACKAGE *package )
{
    WCHAR *template, *p, *q;
    DWORD i, count;

    package->version = msi_suminfo_get_int32( si, PID_PAGECOUNT );
    TRACE("version: %d\n", package->version);

    template = msi_suminfo_dup_string( si, PID_TEMPLATE );
    if (!template)
        return ERROR_SUCCESS; /* native accepts missing template property */

    TRACE("template: %s\n", debugstr_w(template));

    p = strchrW( template, ';' );
    if (!p)
    {
        WARN("invalid template string %s\n", debugstr_w(template));
        msi_free( template );
        return ERROR_PATCH_PACKAGE_INVALID;
    }
    *p = 0;
    if (!template[0] || !strcmpW( template, szIntel ))
        package->platform = PLATFORM_INTEL;
    else if (!strcmpW( template, szIntel64 ))
        package->platform = PLATFORM_INTEL64;
    else if (!strcmpW( template, szX64 ) || !strcmpW( template, szAMD64 ))
        package->platform = PLATFORM_X64;
    else
    {
        WARN("unknown platform %s\n", debugstr_w(template));
        msi_free( template );
        return ERROR_INSTALL_PLATFORM_UNSUPPORTED;
    }
    p++;
    if (!*p)
    {
        msi_free( template );
        return ERROR_SUCCESS;
    }
    count = 1;
    for (q = p; (q = strchrW( q, ',' )); q++) count++;

    package->langids = msi_alloc( count * sizeof(LANGID) );
    if (!package->langids)
    {
        msi_free( template );
        return ERROR_OUTOFMEMORY;
    }

    i = 0;
    while (*p)
    {
        q = strchrW( p, ',' );
        if (q) *q = 0;
        package->langids[i] = atoiW( p );
        if (!q) break;
        p = q + 1;
        i++;
    }
    package->num_langids = i + 1;

    msi_free( template );
    return ERROR_SUCCESS;
}

static UINT validate_package( MSIPACKAGE *package )
{
    BOOL is_wow64;
    UINT i;

    IsWow64Process( GetCurrentProcess(), &is_wow64 );
    if (package->platform == PLATFORM_X64)
    {
        if (!is_64bit && !is_wow64)
            return ERROR_INSTALL_PLATFORM_UNSUPPORTED;
        if (package->version < 200)
            return ERROR_INSTALL_PACKAGE_INVALID;
    }
    if (!package->num_langids)
    {
        return ERROR_SUCCESS;
    }
    for (i = 0; i < package->num_langids; i++)
    {
        LANGID langid = package->langids[i];

        if (PRIMARYLANGID( langid ) == LANG_NEUTRAL)
        {
            langid = MAKELANGID( PRIMARYLANGID( GetSystemDefaultLangID() ), SUBLANGID( langid ) );
        }
        if (SUBLANGID( langid ) == SUBLANG_NEUTRAL)
        {
            langid = MAKELANGID( PRIMARYLANGID( langid ), SUBLANGID( GetSystemDefaultLangID() ) );
        }
        if (IsValidLocale( langid, LCID_INSTALLED ))
            return ERROR_SUCCESS;
    }
    return ERROR_INSTALL_LANGUAGE_UNSUPPORTED;
}

UINT MSI_OpenPackageW(LPCWSTR szPackage, MSIPACKAGE **pPackage)
{
    static const WCHAR Database[] = {'D','A','T','A','B','A','S','E',0};
    static const WCHAR dotmsi[] = {'.','m','s','i',0};
    MSIDATABASE *db = NULL;
    MSIPACKAGE *package;
    MSIHANDLE handle;
    LPWSTR ptr, base_url = NULL;
    UINT r;
    WCHAR temppath[MAX_PATH], localfile[MAX_PATH], cachefile[MAX_PATH];
    LPCWSTR file = szPackage;
    DWORD index = 0;
    MSISUMMARYINFO *si;

    TRACE("%s %p\n", debugstr_w(szPackage), pPackage);

    if( szPackage[0] == '#' )
    {
        handle = atoiW(&szPackage[1]);
        db = msihandle2msiinfo( handle, MSIHANDLETYPE_DATABASE );
        if( !db )
        {
            IWineMsiRemoteDatabase *remote_database;

            remote_database = (IWineMsiRemoteDatabase *)msi_get_remote( handle );
            if ( !remote_database )
                return ERROR_INVALID_HANDLE;

            IWineMsiRemoteDatabase_Release( remote_database );
            WARN("MsiOpenPackage not allowed during a custom action!\n");

            return ERROR_FUNCTION_FAILED;
        }
    }
    else
    {
        if ( UrlIsW( szPackage, URLIS_URL ) )
        {
            r = msi_download_file( szPackage, cachefile );
            if ( r != ERROR_SUCCESS )
                return r;

            r = copy_package_to_temp( cachefile, temppath );
            if ( r != ERROR_SUCCESS )
                return r;

            file = temppath;

            base_url = strdupW( szPackage );
            if ( !base_url )
                return ERROR_OUTOFMEMORY;

            ptr = strrchrW( base_url, '/' );
            if (ptr) *(ptr + 1) = '\0';
        }
        else
        {
            r = copy_package_to_temp( szPackage, temppath );
            if ( r != ERROR_SUCCESS )
                return r;

            file = temppath;
        }

        r = msi_get_local_package_name( localfile, dotmsi );
        if (r != ERROR_SUCCESS)
            return r;

        TRACE("Copying to local package %s\n", debugstr_w(localfile));

        if (!CopyFileW( file, localfile, FALSE ))
        {
            ERR("Unable to copy package (%s -> %s) (error %u)\n",
                debugstr_w(file), debugstr_w(localfile), GetLastError());
            return GetLastError();
        }

        TRACE("Opening relocated package %s\n", debugstr_w( file ));

        /* transforms that add binary streams require that we open the database
         * read/write, which is safe because we always create a copy that is thrown
         * away when we're done.
         */
        r = MSI_OpenDatabaseW( file, MSIDBOPEN_TRANSACT, &db );
        if( r != ERROR_SUCCESS )
        {
            if (file != szPackage)
                DeleteFileW( file );

            if (GetFileAttributesW(szPackage) == INVALID_FILE_ATTRIBUTES)
                return ERROR_FILE_NOT_FOUND;

            return r;
        }

        db->localfile = strdupW( localfile );
    }

    package = MSI_CreatePackage( db, base_url );
    msi_free( base_url );
    msiobj_release( &db->hdr );
    if( !package )
    {
        if (file != szPackage)
            DeleteFileW( file );

        return ERROR_INSTALL_PACKAGE_INVALID;
    }

    if( file != szPackage )
        track_tempfile( package, file );

    si = MSI_GetSummaryInformationW( db->storage, 0 );
    if (!si)
    {
        WARN("failed to load summary info %u\n", r);
        msiobj_release( &package->hdr );
        return ERROR_INSTALL_PACKAGE_INVALID;
    }

    r = msi_parse_summary( si, package );
    msiobj_release( &si->hdr );
    if (r != ERROR_SUCCESS)
    {
        WARN("failed to parse summary info %u\n", r);
        msiobj_release( &package->hdr );
        return r;
    }

    r = validate_package( package );
    if (r != ERROR_SUCCESS)
    {
        msiobj_release( &package->hdr );
        return r;
    }
    msi_set_property( package->db, Database, db->path );

    if( UrlIsW( szPackage, URLIS_URL ) )
        msi_set_property( package->db, szOriginalDatabase, szPackage );
    else if( szPackage[0] == '#' )
        msi_set_property( package->db, szOriginalDatabase, db->path );
    else
    {
        WCHAR fullpath[MAX_PATH];

        GetFullPathNameW( szPackage, MAX_PATH, fullpath, NULL );
        msi_set_property( package->db, szOriginalDatabase, fullpath );
    }

    msi_set_context( package );

    while (1)
    {
        WCHAR patch_code[GUID_SIZE];
        r = MsiEnumPatchesExW( package->ProductCode, NULL, package->Context,
                               MSIPATCHSTATE_APPLIED, index, patch_code, NULL, NULL, NULL, NULL );
        if (r != ERROR_SUCCESS)
            break;

        TRACE("found registered patch %s\n", debugstr_w(patch_code));

        r = apply_registered_patch( package, patch_code );
        if (r != ERROR_SUCCESS)
        {
            ERR("registered patch failed to apply %u\n", r);
            msiobj_release( &package->hdr );
            return r;
        }

        index++;
    }

    if (index)
    {
        msi_clone_properties( package );
        msi_adjust_privilege_properties( package );
    }

    if (gszLogFile)
        package->log_file = CreateFileW( gszLogFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
                                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

    *pPackage = package;
    return ERROR_SUCCESS;
}

UINT WINAPI MsiOpenPackageExW(LPCWSTR szPackage, DWORD dwOptions, MSIHANDLE *phPackage)
{
    MSIPACKAGE *package = NULL;
    UINT ret;

    TRACE("%s %08x %p\n", debugstr_w(szPackage), dwOptions, phPackage );

    if( !szPackage || !phPackage )
        return ERROR_INVALID_PARAMETER;

    if ( !*szPackage )
    {
        FIXME("Should create an empty database and package\n");
        return ERROR_FUNCTION_FAILED;
    }

    if( dwOptions )
        FIXME("dwOptions %08x not supported\n", dwOptions);

    ret = MSI_OpenPackageW( szPackage, &package );
    if( ret == ERROR_SUCCESS )
    {
        *phPackage = alloc_msihandle( &package->hdr );
        if (! *phPackage)
            ret = ERROR_NOT_ENOUGH_MEMORY;
        msiobj_release( &package->hdr );
    }

    return ret;
}

UINT WINAPI MsiOpenPackageW(LPCWSTR szPackage, MSIHANDLE *phPackage)
{
    return MsiOpenPackageExW( szPackage, 0, phPackage );
}

UINT WINAPI MsiOpenPackageExA(LPCSTR szPackage, DWORD dwOptions, MSIHANDLE *phPackage)
{
    LPWSTR szwPack = NULL;
    UINT ret;

    if( szPackage )
    {
        szwPack = strdupAtoW( szPackage );
        if( !szwPack )
            return ERROR_OUTOFMEMORY;
    }

    ret = MsiOpenPackageExW( szwPack, dwOptions, phPackage );

    msi_free( szwPack );

    return ret;
}

UINT WINAPI MsiOpenPackageA(LPCSTR szPackage, MSIHANDLE *phPackage)
{
    return MsiOpenPackageExA( szPackage, 0, phPackage );
}

MSIHANDLE WINAPI MsiGetActiveDatabase(MSIHANDLE hInstall)
{
    MSIPACKAGE *package;
    MSIHANDLE handle = 0;
    IUnknown *remote_unk;
    IWineMsiRemotePackage *remote_package;

    TRACE("(%d)\n",hInstall);

    package = msihandle2msiinfo( hInstall, MSIHANDLETYPE_PACKAGE);
    if( package)
    {
        handle = alloc_msihandle( &package->db->hdr );
        msiobj_release( &package->hdr );
    }
    else if ((remote_unk = msi_get_remote(hInstall)))
    {
        if (IUnknown_QueryInterface(remote_unk, &IID_IWineMsiRemotePackage,
                                        (LPVOID *)&remote_package) == S_OK)
        {
            IWineMsiRemotePackage_GetActiveDatabase(remote_package, &handle);
            IWineMsiRemotePackage_Release(remote_package);
        }
        else
        {
            WARN("remote handle %d is not a package\n", hInstall);
        }
        IUnknown_Release(remote_unk);
    }

    return handle;
}

INT MSI_ProcessMessage( MSIPACKAGE *package, INSTALLMESSAGE eMessageType,
                               MSIRECORD *record)
{
    static const WCHAR szActionData[] =
        {'A','c','t','i','o','n','D','a','t','a',0};
    static const WCHAR szSetProgress[] =
        {'S','e','t','P','r','o','g','r','e','s','s',0};
    static const WCHAR szActionText[] =
        {'A','c','t','i','o','n','T','e','x','t',0};
    LPWSTR message;
    DWORD sz, total_size = 0, log_type = 0;
    INT i, rc = 0;
    char *msg;
    int len;

    TRACE("%x\n", eMessageType);

    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_ERROR)
        log_type |= INSTALLLOGMODE_ERROR;
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_WARNING)
        log_type |= INSTALLLOGMODE_WARNING;
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_USER)
        log_type |= INSTALLLOGMODE_USER;
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_INFO)
        log_type |= INSTALLLOGMODE_INFO;
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_COMMONDATA)
        log_type |= INSTALLLOGMODE_COMMONDATA;
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_ACTIONSTART)
        log_type |= INSTALLLOGMODE_ACTIONSTART;
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_ACTIONDATA)
        log_type |= INSTALLLOGMODE_ACTIONDATA;
    /* just a guess */
    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_PROGRESS)
        log_type |= 0x800;

    if ((eMessageType & 0xff000000) == INSTALLMESSAGE_ACTIONSTART)
    {
        static const WCHAR template_s[]=
            {'A','c','t','i','o','n',' ','%','s',':',' ','%','s','.',' ',0};
        static const WCHAR format[] = 
            {'H','H','\'',':','\'','m','m','\'',':','\'','s','s',0};
        WCHAR timet[0x100];
        LPCWSTR action_text, action;
        LPWSTR deformatted = NULL;

        GetTimeFormatW(LOCALE_USER_DEFAULT, 0, NULL, format, timet, 0x100);

        action = MSI_RecordGetString(record, 1);
        action_text = MSI_RecordGetString(record, 2);

        if (!action || !action_text)
            return IDOK;

        deformat_string(package, action_text, &deformatted);

        len = strlenW(timet) + strlenW(action) + strlenW(template_s);
        if (deformatted)
            len += strlenW(deformatted);
        message = msi_alloc(len*sizeof(WCHAR));
        sprintfW(message, template_s, timet, action);
        if (deformatted)
            strcatW(message, deformatted);
        msi_free(deformatted);
    }
    else
    {
        INT msg_field=1;
        message = msi_alloc(1*sizeof (WCHAR));
        message[0]=0;
        msg_field = MSI_RecordGetFieldCount(record);
        for (i = 1; i <= msg_field; i++)
        {
            LPWSTR tmp;
            WCHAR number[3];
            static const WCHAR format[] = { '%','i',':',' ',0};
            sz = 0;
            MSI_RecordGetStringW(record,i,NULL,&sz);
            sz+=4;
            total_size+=sz*sizeof(WCHAR);
            tmp = msi_alloc(sz*sizeof(WCHAR));
            message = msi_realloc(message,total_size*sizeof (WCHAR));

            MSI_RecordGetStringW(record,i,tmp,&sz);

            if (msg_field > 1)
            {
                sprintfW(number,format,i);
                strcatW(message,number);
            }
            strcatW(message,tmp);
            if (msg_field > 1)
                strcatW(message, szSpace);

            msi_free(tmp);
        }
    }

    TRACE("%p %p %p %x %x %s\n", gUIHandlerA, gUIHandlerW, gUIHandlerRecord,
          gUIFilter, log_type, debugstr_w(message));

    /* convert it to ASCII */
    len = WideCharToMultiByte( CP_ACP, 0, message, -1, NULL, 0, NULL, NULL );
    msg = msi_alloc( len );
    WideCharToMultiByte( CP_ACP, 0, message, -1, msg, len, NULL, NULL );

    if (gUIHandlerW && (gUIFilter & log_type))
    {
        rc = gUIHandlerW( gUIContext, eMessageType, message );
    }
    else if (gUIHandlerA && (gUIFilter & log_type))
    {
        rc = gUIHandlerA( gUIContext, eMessageType, msg );
    }
    else if (gUIHandlerRecord && (gUIFilter & log_type))
    {
        MSIHANDLE rec = MsiCreateRecord( 1 );
        MsiRecordSetStringW( rec, 0, message );
        rc = gUIHandlerRecord( gUIContext, eMessageType, rec );
        MsiCloseHandle( rec );
    }

    if (!rc && package->log_file != INVALID_HANDLE_VALUE &&
        (eMessageType & 0xff000000) != INSTALLMESSAGE_PROGRESS)
    {
        DWORD written;
        WriteFile( package->log_file, msg, len - 1, &written, NULL );
        WriteFile( package->log_file, "\n", 1, &written, NULL );
    }
    msi_free( msg );
    msi_free( message );

    switch (eMessageType & 0xff000000)
    {
    case INSTALLMESSAGE_ACTIONDATA:
        /* FIXME: format record here instead of in ui_actiondata to get the
         * correct action data for external scripts */
        ControlEvent_FireSubscribedEvent(package, szActionData, record);
        break;
    case INSTALLMESSAGE_ACTIONSTART:
    {
        MSIRECORD *uirow;
        LPWSTR deformated;
        LPCWSTR action_text = MSI_RecordGetString(record, 2);

        deformat_string(package, action_text, &deformated);
        uirow = MSI_CreateRecord(1);
        MSI_RecordSetStringW(uirow, 1, deformated);
        TRACE("INSTALLMESSAGE_ACTIONSTART: %s\n", debugstr_w(deformated));
        msi_free(deformated);

        ControlEvent_FireSubscribedEvent(package, szActionText, uirow);

        msiobj_release(&uirow->hdr);
        break;
    }
    case INSTALLMESSAGE_PROGRESS:
        ControlEvent_FireSubscribedEvent(package, szSetProgress, record);
        break;
    }

    return ERROR_SUCCESS;
}

INT WINAPI MsiProcessMessage( MSIHANDLE hInstall, INSTALLMESSAGE eMessageType,
                              MSIHANDLE hRecord)
{
    UINT ret = ERROR_INVALID_HANDLE;
    MSIPACKAGE *package = NULL;
    MSIRECORD *record = NULL;

    package = msihandle2msiinfo( hInstall, MSIHANDLETYPE_PACKAGE );
    if( !package )
    {
        HRESULT hr;
        IWineMsiRemotePackage *remote_package;

        remote_package = (IWineMsiRemotePackage *)msi_get_remote( hInstall );
        if (!remote_package)
            return ERROR_INVALID_HANDLE;

        hr = IWineMsiRemotePackage_ProcessMessage( remote_package, eMessageType, hRecord );

        IWineMsiRemotePackage_Release( remote_package );

        if (FAILED(hr))
        {
            if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
                return HRESULT_CODE(hr);

            return ERROR_FUNCTION_FAILED;
        }

        return ERROR_SUCCESS;
    }

    record = msihandle2msiinfo( hRecord, MSIHANDLETYPE_RECORD );
    if( !record )
        goto out;

    ret = MSI_ProcessMessage( package, eMessageType, record );

out:
    msiobj_release( &package->hdr );
    if( record )
        msiobj_release( &record->hdr );

    return ret;
}

/* property code */

UINT WINAPI MsiSetPropertyA( MSIHANDLE hInstall, LPCSTR szName, LPCSTR szValue )
{
    LPWSTR szwName = NULL, szwValue = NULL;
    UINT r = ERROR_OUTOFMEMORY;

    szwName = strdupAtoW( szName );
    if( szName && !szwName )
        goto end;

    szwValue = strdupAtoW( szValue );
    if( szValue && !szwValue )
        goto end;

    r = MsiSetPropertyW( hInstall, szwName, szwValue);

end:
    msi_free( szwName );
    msi_free( szwValue );

    return r;
}

void msi_reset_folders( MSIPACKAGE *package, BOOL source )
{
    MSIFOLDER *folder;

    LIST_FOR_EACH_ENTRY( folder, &package->folders, MSIFOLDER, entry )
    {
        if ( source )
        {
            msi_free( folder->ResolvedSource );
            folder->ResolvedSource = NULL;
        }
        else
        {
            msi_free( folder->ResolvedTarget );
            folder->ResolvedTarget = NULL;
        }
    }
}

UINT msi_set_property( MSIDATABASE *db, LPCWSTR szName, LPCWSTR szValue )
{
    MSIQUERY *view;
    MSIRECORD *row = NULL;
    UINT rc;
    DWORD sz = 0;
    WCHAR Query[1024];

    static const WCHAR Insert[] = {
        'I','N','S','E','R','T',' ','i','n','t','o',' ',
        '`','_','P','r','o','p','e','r','t','y','`',' ','(',
        '`','_','P','r','o','p','e','r','t','y','`',',',
        '`','V','a','l','u','e','`',')',' ','V','A','L','U','E','S'
        ,' ','(','?',',','?',')',0};
    static const WCHAR Update[] = {
        'U','P','D','A','T','E',' ','`','_','P','r','o','p','e','r','t','y','`',
        ' ','s','e','t',' ','`','V','a','l','u','e','`',' ','=',' ','?',' ',
        'w','h','e','r','e',' ','`','_','P','r','o','p','e','r','t','y','`',
        ' ','=',' ','\'','%','s','\'',0};
    static const WCHAR Delete[] = {
        'D','E','L','E','T','E',' ','F','R','O','M',' ',
        '`','_','P','r','o','p','e','r','t','y','`',' ','W','H','E','R','E',' ',
        '`','_','P','r','o','p','e','r','t','y','`',' ','=',' ','\'','%','s','\'',0};

    TRACE("%p %s %s\n", db, debugstr_w(szName), debugstr_w(szValue));

    if (!szName)
        return ERROR_INVALID_PARAMETER;

    /* this one is weird... */
    if (!szName[0])
        return szValue ? ERROR_FUNCTION_FAILED : ERROR_SUCCESS;

    rc = msi_get_property(db, szName, 0, &sz);
    if (!szValue || !*szValue)
    {
        sprintfW(Query, Delete, szName);
    }
    else if (rc == ERROR_MORE_DATA || rc == ERROR_SUCCESS)
    {
        sprintfW(Query, Update, szName);

        row = MSI_CreateRecord(1);
        MSI_RecordSetStringW(row, 1, szValue);
    }
    else
    {
        strcpyW(Query, Insert);

        row = MSI_CreateRecord(2);
        MSI_RecordSetStringW(row, 1, szName);
        MSI_RecordSetStringW(row, 2, szValue);
    }

    rc = MSI_DatabaseOpenViewW(db, Query, &view);
    if (rc == ERROR_SUCCESS)
    {
        rc = MSI_ViewExecute(view, row);
        MSI_ViewClose(view);
        msiobj_release(&view->hdr);
    }

    if (row)
      msiobj_release(&row->hdr);

    return rc;
}

UINT WINAPI MsiSetPropertyW( MSIHANDLE hInstall, LPCWSTR szName, LPCWSTR szValue)
{
    MSIPACKAGE *package;
    UINT ret;

    package = msihandle2msiinfo( hInstall, MSIHANDLETYPE_PACKAGE);
    if( !package )
    {
        HRESULT hr;
        BSTR name = NULL, value = NULL;
        IWineMsiRemotePackage *remote_package;

        remote_package = (IWineMsiRemotePackage *)msi_get_remote( hInstall );
        if (!remote_package)
            return ERROR_INVALID_HANDLE;

        name = SysAllocString( szName );
        value = SysAllocString( szValue );
        if ((!name && szName) || (!value && szValue))
        {
            SysFreeString( name );
            SysFreeString( value );
            IWineMsiRemotePackage_Release( remote_package );
            return ERROR_OUTOFMEMORY;
        }

        hr = IWineMsiRemotePackage_SetProperty( remote_package, name, value );

        SysFreeString( name );
        SysFreeString( value );
        IWineMsiRemotePackage_Release( remote_package );

        if (FAILED(hr))
        {
            if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
                return HRESULT_CODE(hr);

            return ERROR_FUNCTION_FAILED;
        }

        return ERROR_SUCCESS;
    }

    ret = msi_set_property( package->db, szName, szValue );
    if (ret == ERROR_SUCCESS && !strcmpW( szName, cszSourceDir ))
        msi_reset_folders( package, TRUE );

    msiobj_release( &package->hdr );
    return ret;
}

static MSIRECORD *msi_get_property_row( MSIDATABASE *db, LPCWSTR name )
{
    MSIQUERY *view;
    MSIRECORD *rec, *row = NULL;
    UINT r;

    static const WCHAR query[]= {
        'S','E','L','E','C','T',' ','`','V','a','l','u','e','`',' ',
        'F','R','O','M',' ' ,'`','_','P','r','o','p','e','r','t','y','`',
        ' ','W','H','E','R','E',' ' ,'`','_','P','r','o','p','e','r','t','y','`',
        '=','?',0};

    if (!name || !*name)
        return NULL;

    rec = MSI_CreateRecord(1);
    if (!rec)
        return NULL;

    MSI_RecordSetStringW(rec, 1, name);

    r = MSI_DatabaseOpenViewW(db, query, &view);
    if (r == ERROR_SUCCESS)
    {
        MSI_ViewExecute(view, rec);
        MSI_ViewFetch(view, &row);
        MSI_ViewClose(view);
        msiobj_release(&view->hdr);
    }

    msiobj_release(&rec->hdr);
    return row;
}

/* internal function, not compatible with MsiGetPropertyW */
UINT msi_get_property( MSIDATABASE *db, LPCWSTR szName,
                       LPWSTR szValueBuf, LPDWORD pchValueBuf )
{
    MSIRECORD *row;
    UINT rc = ERROR_FUNCTION_FAILED;

    row = msi_get_property_row( db, szName );

    if (*pchValueBuf > 0)
        szValueBuf[0] = 0;

    if (row)
    {
        rc = MSI_RecordGetStringW(row, 1, szValueBuf, pchValueBuf);
        msiobj_release(&row->hdr);
    }

    if (rc == ERROR_SUCCESS)
        TRACE("returning %s for property %s\n", debugstr_w(szValueBuf),
            debugstr_w(szName));
    else if (rc == ERROR_MORE_DATA)
        TRACE("need %d sized buffer for %s\n", *pchValueBuf,
            debugstr_w(szName));
    else
    {
        *pchValueBuf = 0;
        TRACE("property %s not found\n", debugstr_w(szName));
    }

    return rc;
}

LPWSTR msi_dup_property(MSIDATABASE *db, LPCWSTR prop)
{
    DWORD sz = 0;
    LPWSTR str;
    UINT r;

    r = msi_get_property(db, prop, NULL, &sz);
    if (r != ERROR_SUCCESS && r != ERROR_MORE_DATA)
        return NULL;

    sz++;
    str = msi_alloc(sz * sizeof(WCHAR));
    r = msi_get_property(db, prop, str, &sz);
    if (r != ERROR_SUCCESS)
    {
        msi_free(str);
        str = NULL;
    }

    return str;
}

int msi_get_property_int( MSIDATABASE *db, LPCWSTR prop, int def )
{
    LPWSTR str = msi_dup_property( db, prop );
    int val = str ? atoiW(str) : def;
    msi_free(str);
    return val;
}

static UINT MSI_GetProperty( MSIHANDLE handle, LPCWSTR name,
                             awstring *szValueBuf, LPDWORD pchValueBuf )
{
    MSIPACKAGE *package;
    MSIRECORD *row = NULL;
    UINT r = ERROR_FUNCTION_FAILED;
    LPCWSTR val = NULL;

    TRACE("%u %s %p %p\n", handle, debugstr_w(name),
          szValueBuf->str.w, pchValueBuf );

    if (!name)
        return ERROR_INVALID_PARAMETER;

    package = msihandle2msiinfo( handle, MSIHANDLETYPE_PACKAGE );
    if (!package)
    {
        HRESULT hr;
        IWineMsiRemotePackage *remote_package;
        LPWSTR value = NULL;
        BSTR bname;
        DWORD len;

        remote_package = (IWineMsiRemotePackage *)msi_get_remote( handle );
        if (!remote_package)
            return ERROR_INVALID_HANDLE;

        bname = SysAllocString( name );
        if (!bname)
        {
            IWineMsiRemotePackage_Release( remote_package );
            return ERROR_OUTOFMEMORY;
        }

        len = 0;
        hr = IWineMsiRemotePackage_GetProperty( remote_package, bname, NULL, &len );
        if (FAILED(hr))
            goto done;

        len++;
        value = msi_alloc(len * sizeof(WCHAR));
        if (!value)
        {
            r = ERROR_OUTOFMEMORY;
            goto done;
        }

        hr = IWineMsiRemotePackage_GetProperty( remote_package, bname, (BSTR *)value, &len );
        if (FAILED(hr))
            goto done;

        r = msi_strcpy_to_awstring( value, szValueBuf, pchValueBuf );

        /* Bug required by Adobe installers */
        if (!szValueBuf->unicode && !szValueBuf->str.a)
            *pchValueBuf *= sizeof(WCHAR);

done:
        IWineMsiRemotePackage_Release(remote_package);
        SysFreeString(bname);
        msi_free(value);

        if (FAILED(hr))
        {
            if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
                return HRESULT_CODE(hr);

            return ERROR_FUNCTION_FAILED;
        }

        return r;
    }

    row = msi_get_property_row( package->db, name );
    if (row)
        val = MSI_RecordGetString( row, 1 );

    if (!val)
        val = szEmpty;

    r = msi_strcpy_to_awstring( val, szValueBuf, pchValueBuf );

    if (row)
        msiobj_release( &row->hdr );
    msiobj_release( &package->hdr );

    return r;
}

UINT WINAPI MsiGetPropertyA( MSIHANDLE hInstall, LPCSTR szName,
                             LPSTR szValueBuf, LPDWORD pchValueBuf )
{
    awstring val;
    LPWSTR name;
    UINT r;

    val.unicode = FALSE;
    val.str.a = szValueBuf;

    name = strdupAtoW( szName );
    if (szName && !name)
        return ERROR_OUTOFMEMORY;

    r = MSI_GetProperty( hInstall, name, &val, pchValueBuf );
    msi_free( name );
    return r;
}

UINT WINAPI MsiGetPropertyW( MSIHANDLE hInstall, LPCWSTR szName,
                             LPWSTR szValueBuf, LPDWORD pchValueBuf )
{
    awstring val;

    val.unicode = TRUE;
    val.str.w = szValueBuf;

    return MSI_GetProperty( hInstall, szName, &val, pchValueBuf );
}

typedef struct _msi_remote_package_impl {
    IWineMsiRemotePackage IWineMsiRemotePackage_iface;
    MSIHANDLE package;
    LONG refs;
} msi_remote_package_impl;

static inline msi_remote_package_impl *impl_from_IWineMsiRemotePackage( IWineMsiRemotePackage *iface )
{
    return CONTAINING_RECORD(iface, msi_remote_package_impl, IWineMsiRemotePackage_iface);
}

static HRESULT WINAPI mrp_QueryInterface( IWineMsiRemotePackage *iface,
                REFIID riid,LPVOID *ppobj)
{
    if( IsEqualCLSID( riid, &IID_IUnknown ) ||
        IsEqualCLSID( riid, &IID_IWineMsiRemotePackage ) )
    {
        IUnknown_AddRef( iface );
        *ppobj = iface;
        return S_OK;
    }

    return E_NOINTERFACE;
}

static ULONG WINAPI mrp_AddRef( IWineMsiRemotePackage *iface )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );

    return InterlockedIncrement( &This->refs );
}

static ULONG WINAPI mrp_Release( IWineMsiRemotePackage *iface )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    ULONG r;

    r = InterlockedDecrement( &This->refs );
    if (r == 0)
    {
        MsiCloseHandle( This->package );
        msi_free( This );
    }
    return r;
}

static HRESULT WINAPI mrp_SetMsiHandle( IWineMsiRemotePackage *iface, MSIHANDLE handle )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    This->package = handle;
    return S_OK;
}

static HRESULT WINAPI mrp_GetActiveDatabase( IWineMsiRemotePackage *iface, MSIHANDLE *handle )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    IWineMsiRemoteDatabase *rdb = NULL;
    HRESULT hr;
    MSIHANDLE hdb;

    hr = create_msi_remote_database( NULL, (LPVOID *)&rdb );
    if (FAILED(hr) || !rdb)
    {
        ERR("Failed to create remote database\n");
        return hr;
    }

    hdb = MsiGetActiveDatabase(This->package);

    hr = IWineMsiRemoteDatabase_SetMsiHandle( rdb, hdb );
    if (FAILED(hr))
    {
        ERR("Failed to set the database handle\n");
        return hr;
    }

    *handle = alloc_msi_remote_handle( (IUnknown *)rdb );
    return S_OK;
}

static HRESULT WINAPI mrp_GetProperty( IWineMsiRemotePackage *iface, BSTR property, BSTR *value, DWORD *size )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r;

    r = MsiGetPropertyW(This->package, (LPWSTR)property, (LPWSTR)value, size);
    if (r != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(r);

    return S_OK;
}

static HRESULT WINAPI mrp_SetProperty( IWineMsiRemotePackage *iface, BSTR property, BSTR value )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSetPropertyW(This->package, property, value);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_ProcessMessage( IWineMsiRemotePackage *iface, INSTALLMESSAGE message, MSIHANDLE record )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiProcessMessage(This->package, message, record);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_DoAction( IWineMsiRemotePackage *iface, BSTR action )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiDoActionW(This->package, action);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_Sequence( IWineMsiRemotePackage *iface, BSTR table, int sequence )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSequenceW(This->package, table, sequence);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetTargetPath( IWineMsiRemotePackage *iface, BSTR folder, BSTR *value, DWORD *size )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiGetTargetPathW(This->package, (LPWSTR)folder, (LPWSTR)value, size);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_SetTargetPath( IWineMsiRemotePackage *iface, BSTR folder, BSTR value)
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSetTargetPathW(This->package, folder, value);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetSourcePath( IWineMsiRemotePackage *iface, BSTR folder, BSTR *value, DWORD *size )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiGetSourcePathW(This->package, (LPWSTR)folder, (LPWSTR)value, size);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetMode( IWineMsiRemotePackage *iface, MSIRUNMODE mode, BOOL *ret )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    *ret = MsiGetMode(This->package, mode);
    return S_OK;
}

static HRESULT WINAPI mrp_SetMode( IWineMsiRemotePackage *iface, MSIRUNMODE mode, BOOL state )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSetMode(This->package, mode, state);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetFeatureState( IWineMsiRemotePackage *iface, BSTR feature,
                                    INSTALLSTATE *installed, INSTALLSTATE *action )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiGetFeatureStateW(This->package, feature, installed, action);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_SetFeatureState( IWineMsiRemotePackage *iface, BSTR feature, INSTALLSTATE state )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSetFeatureStateW(This->package, feature, state);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetComponentState( IWineMsiRemotePackage *iface, BSTR component,
                                      INSTALLSTATE *installed, INSTALLSTATE *action )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiGetComponentStateW(This->package, component, installed, action);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_SetComponentState( IWineMsiRemotePackage *iface, BSTR component, INSTALLSTATE state )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSetComponentStateW(This->package, component, state);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetLanguage( IWineMsiRemotePackage *iface, LANGID *language )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    *language = MsiGetLanguage(This->package);
    return S_OK;
}

static HRESULT WINAPI mrp_SetInstallLevel( IWineMsiRemotePackage *iface, int level )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiSetInstallLevel(This->package, level);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_FormatRecord( IWineMsiRemotePackage *iface, MSIHANDLE record,
                                        BSTR *value)
{
    DWORD size = 0;
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiFormatRecordW(This->package, record, NULL, &size);
    if (r == ERROR_SUCCESS)
    {
        *value = SysAllocStringLen(NULL, size);
        if (!*value)
            return E_OUTOFMEMORY;
        size++;
        r = MsiFormatRecordW(This->package, record, *value, &size);
    }
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_EvaluateCondition( IWineMsiRemotePackage *iface, BSTR condition )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiEvaluateConditionW(This->package, condition);
    return HRESULT_FROM_WIN32(r);
}

static HRESULT WINAPI mrp_GetFeatureCost( IWineMsiRemotePackage *iface, BSTR feature,
                                          INT cost_tree, INSTALLSTATE state, INT *cost )
{
    msi_remote_package_impl* This = impl_from_IWineMsiRemotePackage( iface );
    UINT r = MsiGetFeatureCostW(This->package, feature, cost_tree, state, cost);
    return HRESULT_FROM_WIN32(r);
}

static const IWineMsiRemotePackageVtbl msi_remote_package_vtbl =
{
    mrp_QueryInterface,
    mrp_AddRef,
    mrp_Release,
    mrp_SetMsiHandle,
    mrp_GetActiveDatabase,
    mrp_GetProperty,
    mrp_SetProperty,
    mrp_ProcessMessage,
    mrp_DoAction,
    mrp_Sequence,
    mrp_GetTargetPath,
    mrp_SetTargetPath,
    mrp_GetSourcePath,
    mrp_GetMode,
    mrp_SetMode,
    mrp_GetFeatureState,
    mrp_SetFeatureState,
    mrp_GetComponentState,
    mrp_SetComponentState,
    mrp_GetLanguage,
    mrp_SetInstallLevel,
    mrp_FormatRecord,
    mrp_EvaluateCondition,
    mrp_GetFeatureCost,
};

HRESULT create_msi_remote_package( IUnknown *pOuter, LPVOID *ppObj )
{
    msi_remote_package_impl* This;

    This = msi_alloc( sizeof *This );
    if (!This)
        return E_OUTOFMEMORY;

    This->IWineMsiRemotePackage_iface.lpVtbl = &msi_remote_package_vtbl;
    This->package = 0;
    This->refs = 1;

    *ppObj = This;

    return S_OK;
}

UINT msi_package_add_info(MSIPACKAGE *package, DWORD context, DWORD options,
                          LPCWSTR property, LPWSTR value)
{
    MSISOURCELISTINFO *info;

    LIST_FOR_EACH_ENTRY( info, &package->sourcelist_info, MSISOURCELISTINFO, entry )
    {
        if (!strcmpW( info->value, value )) return ERROR_SUCCESS;
    }

    info = msi_alloc(sizeof(MSISOURCELISTINFO));
    if (!info)
        return ERROR_OUTOFMEMORY;

    info->context = context;
    info->options = options;
    info->property = property;
    info->value = strdupW(value);
    list_add_head(&package->sourcelist_info, &info->entry);

    return ERROR_SUCCESS;
}

UINT msi_package_add_media_disk(MSIPACKAGE *package, DWORD context, DWORD options,
                                DWORD disk_id, LPWSTR volume_label, LPWSTR disk_prompt)
{
    MSIMEDIADISK *disk;

    LIST_FOR_EACH_ENTRY( disk, &package->sourcelist_media, MSIMEDIADISK, entry )
    {
        if (disk->disk_id == disk_id) return ERROR_SUCCESS;
    }

    disk = msi_alloc(sizeof(MSIMEDIADISK));
    if (!disk)
        return ERROR_OUTOFMEMORY;

    disk->context = context;
    disk->options = options;
    disk->disk_id = disk_id;
    disk->volume_label = strdupW(volume_label);
    disk->disk_prompt = strdupW(disk_prompt);
    list_add_head(&package->sourcelist_media, &disk->entry);

    return ERROR_SUCCESS;
}
