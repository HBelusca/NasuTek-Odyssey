/*
 * Implementation of the Microsoft Installer (msi.dll)
 *
 * Copyright 2005 Aric Stewart for CodeWeavers
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
 * Here are helper functions formally in action.c that are used by a variety of
 * actions and functions.
 */

#include <stdarg.h>

#include "windef.h"
#include "wine/debug.h"
#include "msipriv.h"
#include "winuser.h"
#include "wine/unicode.h"
#include "msidefs.h"

WINE_DEFAULT_DEBUG_CHANNEL(msi);

static const WCHAR cszTargetDir[] = {'T','A','R','G','E','T','D','I','R',0};
static const WCHAR cszDatabase[]={'D','A','T','A','B','A','S','E',0};

LPWSTR build_icon_path(MSIPACKAGE *package, LPCWSTR icon_name )
{
    LPWSTR SystemFolder, dest, FilePath;

    static const WCHAR szMicrosoft[] =
        {'M','i','c','r','o','s','o','f','t','\\',0};
    static const WCHAR szInstaller[] = 
        {'I','n','s','t','a','l','l','e','r','\\',0};
    static const WCHAR szADFolder[] =
        {'A','p','p','D','a','t','a','F','o','l','d','e','r',0};
    static const WCHAR szWFolder[] =
        {'W','i','n','d','o','w','s','F','o','l','d','e','r',0};

    if(package->Context == MSIINSTALLCONTEXT_MACHINE)
        SystemFolder = msi_dup_property( package->db, szWFolder );
    else
    {
        LPWSTR ADTgt = msi_dup_property( package->db, szADFolder );
        SystemFolder = build_directory_name(2, ADTgt, szMicrosoft);
        msi_free(ADTgt);
    }

    dest = build_directory_name(3, SystemFolder, szInstaller, package->ProductCode);

    create_full_pathW(dest);

    FilePath = build_directory_name(2, dest, icon_name);

    msi_free(SystemFolder);
    msi_free(dest);
    return FilePath;
}

LPWSTR msi_dup_record_field( MSIRECORD *rec, INT field )
{
    DWORD sz = 0;
    LPWSTR str;
    UINT r;

    if (MSI_RecordIsNull( rec, field ))
        return NULL;

    r = MSI_RecordGetStringW( rec, field, NULL, &sz );
    if (r != ERROR_SUCCESS)
        return NULL;

    sz ++;
    str = msi_alloc( sz * sizeof (WCHAR) );
    if (!str)
        return str;
    str[0] = 0;
    r = MSI_RecordGetStringW( rec, field, str, &sz );
    if (r != ERROR_SUCCESS)
    {
        ERR("failed to get string!\n");
        msi_free( str );
        return NULL;
    }
    return str;
}

MSICOMPONENT* get_loaded_component( MSIPACKAGE* package, LPCWSTR Component )
{
    MSICOMPONENT *comp;

    LIST_FOR_EACH_ENTRY( comp, &package->components, MSICOMPONENT, entry )
    {
        if (!strcmpW( Component, comp->Component ))
            return comp;
    }
    return NULL;
}

MSIFEATURE* get_loaded_feature(MSIPACKAGE* package, LPCWSTR Feature )
{
    MSIFEATURE *feature;

    LIST_FOR_EACH_ENTRY( feature, &package->features, MSIFEATURE, entry )
    {
        if (!strcmpW( Feature, feature->Feature ))
            return feature;
    }
    return NULL;
}

MSIFILE* get_loaded_file( MSIPACKAGE* package, LPCWSTR key )
{
    MSIFILE *file;

    LIST_FOR_EACH_ENTRY( file, &package->files, MSIFILE, entry )
    {
        if (!strcmpW( key, file->File ))
            return file;
    }
    return NULL;
}

int track_tempfile( MSIPACKAGE *package, LPCWSTR path )
{
    MSITEMPFILE *temp;

    TRACE("%s\n", debugstr_w(path));

    LIST_FOR_EACH_ENTRY( temp, &package->tempfiles, MSITEMPFILE, entry )
        if (!strcmpW( path, temp->Path ))
            return 0;

    temp = msi_alloc_zero( sizeof (MSITEMPFILE) );
    if (!temp)
        return -1;

    list_add_head( &package->tempfiles, &temp->entry );
    temp->Path = strdupW( path );

    return 0;
}

MSIFOLDER *get_loaded_folder( MSIPACKAGE *package, LPCWSTR dir )
{
    MSIFOLDER *folder;

    LIST_FOR_EACH_ENTRY( folder, &package->folders, MSIFOLDER, entry )
    {
        if (!strcmpW( dir, folder->Directory ))
            return folder;
    }
    return NULL;
}

static LPWSTR get_source_root( MSIPACKAGE *package )
{
    LPWSTR path, p;

    path = msi_dup_property( package->db, cszSourceDir );
    if (path)
        return path;

    path = msi_dup_property( package->db, cszDatabase );
    if (path)
    {
        p = strrchrW(path,'\\');
        if (p)
            *(p+1) = 0;
    }
    return path;
}

/*
 * clean_spaces_from_path()
 *
 * removes spaces from the beginning and end of path segments
 * removes multiple \\ characters
 */
static void clean_spaces_from_path( LPWSTR p )
{
    LPWSTR q = p;
    int n, len = 0;

    while (1)
    {
        /* copy until the end of the string or a space */
        while (*p != ' ' && (*q = *p))
        {
            p++, len++;
            /* reduce many backslashes to one */
            if (*p != '\\' || *q != '\\')
                q++;
        }

        /* quit at the end of the string */
        if (!*p)
            break;

        /* count the number of spaces */
        n = 0;
        while (p[n] == ' ')
            n++;

        /* if it's leading or trailing space, skip it */
        if ( len == 0 || p[-1] == '\\' || p[n] == '\\' )
            p += n;
        else  /* copy n spaces */
            while (n && (*q++ = *p++)) n--;
    }
}

LPWSTR resolve_file_source(MSIPACKAGE *package, MSIFILE *file)
{
    LPWSTR p, path;

    TRACE("Working to resolve source of file %s\n", debugstr_w(file->File));

    if (file->IsCompressed)
        return NULL;

    p = resolve_source_folder( package, file->Component->Directory, NULL );
    path = build_directory_name(2, p, file->ShortName);

    if (file->LongName &&
        GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES)
    {
        msi_free(path);
        path = build_directory_name(2, p, file->LongName);
    }

    msi_free(p);

    TRACE("file %s source resolves to %s\n", debugstr_w(file->File), debugstr_w(path));
    return path;
}

LPWSTR resolve_source_folder( MSIPACKAGE *package, LPCWSTR name, MSIFOLDER **folder )
{
    MSIFOLDER *f;
    LPWSTR p, path = NULL, parent;

    TRACE("working to resolve %s\n", debugstr_w(name));

    if (!strcmpW( name, cszSourceDir ))
        name = cszTargetDir;

    f = get_loaded_folder( package, name );
    if (!f)
        return NULL;

    /* special resolving for Target and Source root dir */
    if (!strcmpW( name, cszTargetDir ))
    {
        if (!f->ResolvedSource)
            f->ResolvedSource = get_source_root( package );
    }

    if (folder)
        *folder = f;

    if (f->ResolvedSource)
    {
        path = strdupW( f->ResolvedSource );
        TRACE("   already resolved to %s\n", debugstr_w(path));
        return path;
    }

    if (!f->Parent)
        return path;

    parent = f->Parent;
    TRACE(" ! parent is %s\n", debugstr_w(parent));

    p = resolve_source_folder( package, parent, NULL );

    if (package->WordCount & msidbSumInfoSourceTypeCompressed)
        path = get_source_root( package );
    else if (package->WordCount & msidbSumInfoSourceTypeSFN)
        path = build_directory_name( 3, p, f->SourceShortPath, NULL );
    else
        path = build_directory_name( 3, p, f->SourceLongPath, NULL );

    TRACE("-> %s\n", debugstr_w(path));
    f->ResolvedSource = strdupW( path );
    msi_free( p );

    return path;
}

LPWSTR resolve_target_folder( MSIPACKAGE *package, LPCWSTR name, BOOL set_prop, BOOL load_prop,
                              MSIFOLDER **folder )
{
    MSIFOLDER *f;
    LPWSTR p, path = NULL, parent;

    TRACE("working to resolve %s\n", debugstr_w(name));

    f = get_loaded_folder( package, name );
    if (!f)
        return NULL;

    /* special resolving for Target and Source root dir */
    if (!strcmpW( name, cszTargetDir ))
    {
        if (!f->ResolvedTarget && !f->Property)
        {
            LPWSTR check_path;
            check_path = msi_dup_property( package->db, cszTargetDir );
            if (!check_path)
            {
                check_path = msi_dup_property( package->db, cszRootDrive );
                if (set_prop)
                    msi_set_property( package->db, cszTargetDir, check_path );
            }

            /* correct misbuilt target dir */
            path = build_directory_name(2, check_path, NULL);
            clean_spaces_from_path( path );
            if (strcmpiW( path, check_path ))
                msi_set_property( package->db, cszTargetDir, path );
            msi_free(check_path);

            f->ResolvedTarget = path;
        }
    }

    if (folder)
        *folder = f;

    if (f->ResolvedTarget)
    {
        path = strdupW( f->ResolvedTarget );
        TRACE("   already resolved to %s\n", debugstr_w(path));
        return path;
    }

    if (f->Property)
    {
        path = build_directory_name( 2, f->Property, NULL );
        TRACE("   internally set to %s\n", debugstr_w(path));
        if (set_prop) msi_set_property( package->db, name, path );
        return path;
    }

    if (load_prop && (path = msi_dup_property( package->db, name )))
    {
        f->ResolvedTarget = strdupW( path );
        TRACE("   property set to %s\n", debugstr_w(path));
        return path;
    }

    if (!f->Parent)
        return path;

    parent = f->Parent;

    TRACE(" ! parent is %s\n", debugstr_w(parent));

    p = resolve_target_folder( package, parent, set_prop, load_prop, NULL );

    TRACE("   TargetDefault = %s\n", debugstr_w(f->TargetDefault));
    path = build_directory_name( 3, p, f->TargetDefault, NULL );
    clean_spaces_from_path( path );
    f->ResolvedTarget = strdupW( path );

    TRACE("-> %s\n", debugstr_w(path));
    if (set_prop) msi_set_property( package->db, name, path );
    msi_free( p );

    return path;
}

/* wrapper to resist a need for a full rewrite right now */
DWORD deformat_string(MSIPACKAGE *package, LPCWSTR ptr, WCHAR** data )
{
    if (ptr)
    {
        MSIRECORD *rec = MSI_CreateRecord(1);
        DWORD size = 0;

        MSI_RecordSetStringW(rec,0,ptr);
        MSI_FormatRecordW(package,rec,NULL,&size);

        size++;
        *data = msi_alloc(size*sizeof(WCHAR));
        if (size > 1)
            MSI_FormatRecordW(package,rec,*data,&size);
        else
            *data[0] = 0;

        msiobj_release( &rec->hdr );
        return sizeof(WCHAR)*size;
    }

    *data = NULL;
    return 0;
}

UINT schedule_action(MSIPACKAGE *package, UINT script, LPCWSTR action)
{
    UINT count;
    LPWSTR *newbuf = NULL;
    if (script >= TOTAL_SCRIPTS)
    {
        FIXME("Unknown script requested %i\n",script);
        return ERROR_FUNCTION_FAILED;
    }
    TRACE("Scheduling Action %s in script %i\n",debugstr_w(action), script);
    
    count = package->script->ActionCount[script];
    package->script->ActionCount[script]++;
    if (count != 0)
        newbuf = msi_realloc( package->script->Actions[script],
                        package->script->ActionCount[script]* sizeof(LPWSTR));
    else
        newbuf = msi_alloc( sizeof(LPWSTR));

    newbuf[count] = strdupW(action);
    package->script->Actions[script] = newbuf;

   return ERROR_SUCCESS;
}

void msi_free_action_script(MSIPACKAGE *package, UINT script)
{
    UINT i;
    for (i = 0; i < package->script->ActionCount[script]; i++)
        msi_free(package->script->Actions[script][i]);

    msi_free(package->script->Actions[script]);
    package->script->Actions[script] = NULL;
    package->script->ActionCount[script] = 0;
}

/*
 *  build_directory_name()
 *
 *  This function is to save messing round with directory names
 *  It handles adding backslashes between path segments, 
 *   and can add \ at the end of the directory name if told to.
 *
 *  It takes a variable number of arguments.
 *  It always allocates a new string for the result, so make sure
 *   to free the return value when finished with it.
 *
 *  The first arg is the number of path segments that follow.
 *  The arguments following count are a list of path segments.
 *  A path segment may be NULL.
 *
 *  Path segments will be added with a \ separating them.
 *  A \ will not be added after the last segment, however if the
 *    last segment is NULL, then the last character will be a \
 * 
 */
LPWSTR build_directory_name(DWORD count, ...)
{
    DWORD sz = 1, i;
    LPWSTR dir;
    va_list va;

    va_start(va,count);
    for(i=0; i<count; i++)
    {
        LPCWSTR str = va_arg(va,LPCWSTR);
        if (str)
            sz += strlenW(str) + 1;
    }
    va_end(va);

    dir = msi_alloc(sz*sizeof(WCHAR));
    dir[0]=0;

    va_start(va,count);
    for(i=0; i<count; i++)
    {
        LPCWSTR str = va_arg(va,LPCWSTR);
        if (!str)
            continue;
        strcatW(dir, str);
        if( ((i+1)!=count) && dir[strlenW(dir)-1]!='\\')
            strcatW(dir, szBackSlash);
    }
    va_end(va);

    return dir;
}

/***********************************************************************
 *            create_full_pathW
 *
 * Recursively create all directories in the path.
 *
 * shamelessly stolen from setupapi/queue.c
 */
BOOL create_full_pathW(const WCHAR *path)
{
    BOOL ret = TRUE;
    int len;
    WCHAR *new_path;

    new_path = msi_alloc( (strlenW(path) + 1) * sizeof(WCHAR));

    strcpyW(new_path, path);

    while((len = strlenW(new_path)) && new_path[len - 1] == '\\')
    new_path[len - 1] = 0;

    while(!CreateDirectoryW(new_path, NULL))
    {
        WCHAR *slash;
        DWORD last_error = GetLastError();
        if(last_error == ERROR_ALREADY_EXISTS)
            break;

        if(last_error != ERROR_PATH_NOT_FOUND)
        {
            ret = FALSE;
            break;
        }

        if(!(slash = strrchrW(new_path, '\\')))
        {
            ret = FALSE;
            break;
        }

        len = slash - new_path;
        new_path[len] = 0;
        if(!create_full_pathW(new_path))
        {
            ret = FALSE;
            break;
        }
        new_path[len] = '\\';
    }

    msi_free(new_path);
    return ret;
}

void ui_progress(MSIPACKAGE *package, int a, int b, int c, int d )
{
    MSIRECORD * row;

    row = MSI_CreateRecord(4);
    MSI_RecordSetInteger(row,1,a);
    MSI_RecordSetInteger(row,2,b);
    MSI_RecordSetInteger(row,3,c);
    MSI_RecordSetInteger(row,4,d);
    MSI_ProcessMessage(package, INSTALLMESSAGE_PROGRESS, row);
    msiobj_release(&row->hdr);

    msi_dialog_check_messages(NULL);
}

void ui_actiondata(MSIPACKAGE *package, LPCWSTR action, MSIRECORD * record)
{
    static const WCHAR Query_t[] = 
        {'S','E','L','E','C','T',' ','*',' ','F','R','O','M',' ',
         '`','A','c','t','i','o', 'n','T','e','x','t','`',' ',
         'W','H','E','R','E',' ', '`','A','c','t','i','o','n','`',' ','=', 
         ' ','\'','%','s','\'',0};
    WCHAR message[1024];
    MSIRECORD * row = 0;
    DWORD size;

    if (!package->LastAction || strcmpW(package->LastAction, action))
    {
        row = MSI_QueryGetRecord(package->db, Query_t, action);
        if (!row)
            return;

        if (MSI_RecordIsNull(row,3))
        {
            msiobj_release(&row->hdr);
            return;
        }

        /* update the cached actionformat */
        msi_free(package->ActionFormat);
        package->ActionFormat = msi_dup_record_field(row,3);

        msi_free(package->LastAction);
        package->LastAction = strdupW(action);

        msiobj_release(&row->hdr);
    }

    MSI_RecordSetStringW(record,0,package->ActionFormat);
    size = 1024;
    MSI_FormatRecordW(package,record,message,&size);

    row = MSI_CreateRecord(1);
    MSI_RecordSetStringW(row,1,message);
 
    MSI_ProcessMessage(package, INSTALLMESSAGE_ACTIONDATA, row);

    msiobj_release(&row->hdr);
}

void reduce_to_longfilename(WCHAR* filename)
{
    LPWSTR p = strchrW(filename,'|');
    if (p)
        memmove(filename, p+1, (strlenW(p+1)+1)*sizeof(WCHAR));
}

LPWSTR create_component_advertise_string(MSIPACKAGE* package, 
                MSICOMPONENT* component, LPCWSTR feature)
{
    static const WCHAR fmt[] = {'%','s','%','s','%','c','%','s',0};
    WCHAR productid_85[21], component_85[21];
    LPWSTR output = NULL;
    DWORD sz = 0;
    GUID clsid;

    /* > is used if there is a component GUID and < if not.  */

    productid_85[0] = 0;
    component_85[0] = 0;

    CLSIDFromString(package->ProductCode, &clsid);
    encode_base85_guid(&clsid, productid_85);

    if (component)
    {
        CLSIDFromString(component->ComponentId, &clsid);
        encode_base85_guid(&clsid, component_85);
    }

    TRACE("prod=%s feat=%s comp=%s\n", debugstr_w(productid_85),
          debugstr_w(feature), debugstr_w(component_85));
 
    sz = 20 + lstrlenW(feature) + 20 + 3;

    output = msi_alloc_zero(sz*sizeof(WCHAR));

    sprintfW(output, fmt, productid_85, feature,
             component?'>':'<', component_85);
    
    return output;
}

/* update component state based on a feature change */
void ACTION_UpdateComponentStates( MSIPACKAGE *package, MSIFEATURE *feature )
{
    INSTALLSTATE newstate;
    ComponentList *cl;

    newstate = feature->ActionRequest;

    if (newstate == INSTALLSTATE_ABSENT)
        newstate = INSTALLSTATE_UNKNOWN;

    LIST_FOR_EACH_ENTRY( cl, &feature->Components, ComponentList, entry )
    {
        MSICOMPONENT* component = cl->component;
    
        if (!component->Enabled) continue;

        TRACE("MODIFYING(%i): Component %s (Installed %i, Action %i, Request %i)\n",
            newstate, debugstr_w(component->Component), component->Installed, 
            component->Action, component->ActionRequest);
        
        if (newstate == INSTALLSTATE_LOCAL)
        {
            component->Action = INSTALLSTATE_LOCAL;
            component->ActionRequest = INSTALLSTATE_LOCAL;
        }
        else 
        {
            ComponentList *clist;
            MSIFEATURE *f;

            component->hasLocalFeature = FALSE;

            component->Action = newstate;
            component->ActionRequest = newstate;

            /* if any other feature wants it local we need to set it local */
            LIST_FOR_EACH_ENTRY( f, &package->features, MSIFEATURE, entry )
            {
                if ( f->ActionRequest != INSTALLSTATE_LOCAL &&
                     f->ActionRequest != INSTALLSTATE_SOURCE )
                {
                    continue;
                }

                LIST_FOR_EACH_ENTRY( clist, &f->Components, ComponentList, entry )
                {
                    if ( clist->component == component &&
                         (f->ActionRequest == INSTALLSTATE_LOCAL ||
                          f->ActionRequest == INSTALLSTATE_SOURCE) )
                    {
                        TRACE("Saved by %s\n", debugstr_w(f->Feature));
                        component->hasLocalFeature = TRUE;

                        if (component->Attributes & msidbComponentAttributesOptional)
                        {
                            if (f->Attributes & msidbFeatureAttributesFavorSource)
                            {
                                component->Action = INSTALLSTATE_SOURCE;
                                component->ActionRequest = INSTALLSTATE_SOURCE;
                            }
                            else
                            {
                                component->Action = INSTALLSTATE_LOCAL;
                                component->ActionRequest = INSTALLSTATE_LOCAL;
                            }
                        }
                        else if (component->Attributes & msidbComponentAttributesSourceOnly)
                        {
                            component->Action = INSTALLSTATE_SOURCE;
                            component->ActionRequest = INSTALLSTATE_SOURCE;
                        }
                        else
                        {
                            component->Action = INSTALLSTATE_LOCAL;
                            component->ActionRequest = INSTALLSTATE_LOCAL;
                        }
                    }
                }
            }
        }
        TRACE("Result (%i): Component %s (Installed %i, Action %i, Request %i)\n",
            newstate, debugstr_w(component->Component), component->Installed, 
            component->Action, component->ActionRequest);
    } 
}

UINT register_unique_action(MSIPACKAGE *package, LPCWSTR action)
{
    UINT count;
    LPWSTR *newbuf = NULL;

    if (!package->script)
        return FALSE;

    TRACE("Registering %s as unique action\n", debugstr_w(action));
    
    count = package->script->UniqueActionsCount;
    package->script->UniqueActionsCount++;
    if (count != 0)
        newbuf = msi_realloc( package->script->UniqueActions,
                        package->script->UniqueActionsCount* sizeof(LPWSTR));
    else
        newbuf = msi_alloc( sizeof(LPWSTR));

    newbuf[count] = strdupW(action);
    package->script->UniqueActions = newbuf;

    return ERROR_SUCCESS;
}

BOOL check_unique_action(const MSIPACKAGE *package, LPCWSTR action)
{
    UINT i;

    if (!package->script)
        return FALSE;

    for (i = 0; i < package->script->UniqueActionsCount; i++)
        if (!strcmpW(package->script->UniqueActions[i], action))
            return TRUE;

    return FALSE;
}

WCHAR* generate_error_string(MSIPACKAGE *package, UINT error, DWORD count, ... )
{
    static const WCHAR query[] = {'S','E','L','E','C','T',' ','`','M','e','s','s','a','g','e','`',' ','F','R','O','M',' ','`','E','r','r','o','r','`',' ','W','H','E','R','E',' ','`','E','r','r','o','r','`',' ','=',' ','%','i',0};

    MSIRECORD *rec;
    MSIRECORD *row;
    DWORD size = 0;
    DWORD i;
    va_list va;
    LPCWSTR str;
    LPWSTR data;

    row = MSI_QueryGetRecord(package->db, query, error);
    if (!row)
        return 0;

    rec = MSI_CreateRecord(count+2);

    str = MSI_RecordGetString(row,1);
    MSI_RecordSetStringW(rec,0,str);
    msiobj_release( &row->hdr );
    MSI_RecordSetInteger(rec,1,error);

    va_start(va,count);
    for (i = 0; i < count; i++)
    {
        str = va_arg(va,LPCWSTR);
        MSI_RecordSetStringW(rec,(i+2),str);
    }
    va_end(va);

    MSI_FormatRecordW(package,rec,NULL,&size);

    size++;
    data = msi_alloc(size*sizeof(WCHAR));
    if (size > 1)
        MSI_FormatRecordW(package,rec,data,&size);
    else
        data[0] = 0;
    msiobj_release( &rec->hdr );
    return data;
}
