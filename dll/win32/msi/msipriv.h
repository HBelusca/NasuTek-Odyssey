/*
 * Implementation of the Microsoft Installer (msi.dll)
 *
 * Copyright 2002-2005 Mike McCormack for CodeWeavers
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

#ifndef __WINE_MSI_PRIVATE__
#define __WINE_MSI_PRIVATE__

#include <stdarg.h>

#include "windef.h"
#include "winbase.h"
#include "fdi.h"
#include "msi.h"
#include "msiquery.h"
#include "msidefs.h"
#include "objbase.h"
#include "objidl.h"
#include "fusion.h"
#include "winnls.h"
#include "winver.h"
#include "wine/list.h"
#include "wine/debug.h"

static const BOOL is_64bit = sizeof(void *) > sizeof(int);

#define MSI_DATASIZEMASK 0x00ff
#define MSITYPE_VALID    0x0100
#define MSITYPE_LOCALIZABLE 0x200
#define MSITYPE_STRING   0x0800
#define MSITYPE_NULLABLE 0x1000
#define MSITYPE_KEY      0x2000
#define MSITYPE_TEMPORARY 0x4000

#define MAX_STREAM_NAME_LEN     62
#define LONG_STR_BYTES  3

/* Install UI level mask for AND operation to exclude flags */
#define INSTALLUILEVEL_MASK             0x0007

#define MSITYPE_IS_BINARY(type) (((type) & ~MSITYPE_NULLABLE) == (MSITYPE_STRING|MSITYPE_VALID))

struct tagMSITABLE;
typedef struct tagMSITABLE MSITABLE;

struct string_table;
typedef struct string_table string_table;

struct tagMSIOBJECTHDR;
typedef struct tagMSIOBJECTHDR MSIOBJECTHDR;

typedef VOID (*msihandledestructor)( MSIOBJECTHDR * );

struct tagMSIOBJECTHDR
{
    UINT magic;
    UINT type;
    LONG refcount;
    msihandledestructor destructor;
};

typedef struct tagMSIDATABASE
{
    MSIOBJECTHDR hdr;
    IStorage *storage;
    string_table *strings;
    UINT bytes_per_strref;
    LPWSTR path;
    LPWSTR deletefile;
    LPWSTR localfile;
    LPCWSTR mode;
    struct list tables;
    struct list transforms;
    struct list streams;
} MSIDATABASE;

typedef struct tagMSIVIEW MSIVIEW;

typedef struct tagMSIQUERY
{
    MSIOBJECTHDR hdr;
    MSIVIEW *view;
    UINT row;
    MSIDATABASE *db;
    struct list mem;
} MSIQUERY;

/* maybe we can use a Variant instead of doing it ourselves? */
typedef struct tagMSIFIELD
{
    UINT type;
    union
    {
        INT iVal;
        INT_PTR pVal;
        LPWSTR szwVal;
        IStream *stream;
    } u;
} MSIFIELD;

typedef struct tagMSIRECORD
{
    MSIOBJECTHDR hdr;
    UINT count;       /* as passed to MsiCreateRecord */
    MSIFIELD fields[1]; /* nb. array size is count+1 */
} MSIRECORD;

typedef struct tagMSISOURCELISTINFO
{
    struct list entry;
    DWORD context;
    DWORD options;
    LPCWSTR property;
    LPWSTR value;
} MSISOURCELISTINFO;

typedef struct tagMSIMEDIADISK
{
    struct list entry;
    DWORD context;
    DWORD options;
    DWORD disk_id;
    LPWSTR volume_label;
    LPWSTR disk_prompt;
} MSIMEDIADISK;

typedef struct tagMSIMEDIAINFO
{
    UINT disk_id;
    UINT type;
    UINT last_sequence;
    LPWSTR disk_prompt;
    LPWSTR cabinet;
    LPWSTR first_volume;
    LPWSTR volume_label;
    BOOL is_continuous;
    BOOL is_extracted;
    WCHAR sourcedir[MAX_PATH];
} MSIMEDIAINFO;

typedef struct tagMSIPATCHINFO
{
    struct list entry;
    LPWSTR patchcode;
    LPWSTR transforms;
    LPWSTR localfile;
    MSIPATCHSTATE state;
} MSIPATCHINFO;

typedef struct tagMSIBINARY
{
    struct list entry;
    WCHAR *source;
    WCHAR *tmpfile;
    HMODULE module;
} MSIBINARY;

typedef struct _column_info
{
    LPCWSTR table;
    LPCWSTR column;
    INT   type;
    BOOL   temporary;
    struct expr *val;
    struct _column_info *next;
} column_info;

typedef const struct tagMSICOLUMNHASHENTRY *MSIITERHANDLE;

typedef struct tagMSIVIEWOPS
{
    /*
     * fetch_int - reads one integer from {row,col} in the table
     *
     *  This function should be called after the execute method.
     *  Data returned by the function should not change until
     *   close or delete is called.
     *  To get a string value, query the database's string table with
     *   the integer value returned from this function.
     */
    UINT (*fetch_int)( struct tagMSIVIEW *view, UINT row, UINT col, UINT *val );

    /*
     * fetch_stream - gets a stream from {row,col} in the table
     *
     *  This function is similar to fetch_int, except fetches a
     *    stream instead of an integer.
     */
    UINT (*fetch_stream)( struct tagMSIVIEW *view, UINT row, UINT col, IStream **stm );

    /*
     * get_row - gets values from a row
     *
     */
    UINT (*get_row)( struct tagMSIVIEW *view, UINT row, MSIRECORD **rec );

    /*
     * set_row - sets values in a row as specified by mask
     *
     *  Similar semantics to fetch_int
     */
    UINT (*set_row)( struct tagMSIVIEW *view, UINT row, MSIRECORD *rec, UINT mask );

    /*
     * Inserts a new row into the database from the records contents
     */
    UINT (*insert_row)( struct tagMSIVIEW *view, MSIRECORD *record, UINT row, BOOL temporary );

    /*
     * Deletes a row from the database
     */
    UINT (*delete_row)( struct tagMSIVIEW *view, UINT row );

    /*
     * execute - loads the underlying data into memory so it can be read
     */
    UINT (*execute)( struct tagMSIVIEW *view, MSIRECORD *record );

    /*
     * close - clears the data read by execute from memory
     */
    UINT (*close)( struct tagMSIVIEW *view );

    /*
     * get_dimensions - returns the number of rows or columns in a table.
     *
     *  The number of rows can only be queried after the execute method
     *   is called. The number of columns can be queried at any time.
     */
    UINT (*get_dimensions)( struct tagMSIVIEW *view, UINT *rows, UINT *cols );

    /*
     * get_column_info - returns the name and type of a specific column
     *
     *  The name and tablename is HeapAlloc'ed by this function and should be
     *  freed by the caller.
     *  The column information can be queried at any time.
     */
    UINT (*get_column_info)( struct tagMSIVIEW *view, UINT n, LPWSTR *name, UINT *type, BOOL *temporary, LPWSTR *tableName);

    /*
     * modify - not yet implemented properly
     */
    UINT (*modify)( struct tagMSIVIEW *view, MSIMODIFY eModifyMode, MSIRECORD *record, UINT row );

    /*
     * delete - destroys the structure completely
     */
    UINT (*delete)( struct tagMSIVIEW * );

    /*
     * find_matching_rows - iterates through rows that match a value
     *
     * If the column type is a string then a string ID should be passed in.
     *  If the value to be looked up is an integer then no transformation of
     *  the input value is required, except if the column is a string, in which
     *  case a string ID should be passed in.
     * The handle is an input/output parameter that keeps track of the current
     *  position in the iteration. It must be initialised to zero before the
     *  first call and continued to be passed in to subsequent calls.
     */
    UINT (*find_matching_rows)( struct tagMSIVIEW *view, UINT col, UINT val, UINT *row, MSIITERHANDLE *handle );

    /*
     * add_ref - increases the reference count of the table
     */
    UINT (*add_ref)( struct tagMSIVIEW *view );

    /*
     * release - decreases the reference count of the table
     */
    UINT (*release)( struct tagMSIVIEW *view );

    /*
     * add_column - adds a column to the table
     */
    UINT (*add_column)( struct tagMSIVIEW *view, LPCWSTR table, UINT number, LPCWSTR column, UINT type, BOOL hold );

    /*
     * remove_column - removes the column represented by table name and column number from the table
     */
    UINT (*remove_column)( struct tagMSIVIEW *view, LPCWSTR table, UINT number );

    /*
     * sort - orders the table by columns
     */
    UINT (*sort)( struct tagMSIVIEW *view, column_info *columns );

    /*
     * drop - drops the table from the database
     */
    UINT (*drop)( struct tagMSIVIEW *view );
} MSIVIEWOPS;

struct tagMSIVIEW
{
    MSIOBJECTHDR hdr;
    const MSIVIEWOPS *ops;
};

struct msi_dialog_tag;
typedef struct msi_dialog_tag msi_dialog;

enum platform
{
    PLATFORM_INTEL,
    PLATFORM_INTEL64,
    PLATFORM_X64
};

enum clr_version
{
    CLR_VERSION_V11,
    CLR_VERSION_V20,
    CLR_VERSION_MAX
};

typedef struct tagMSIPACKAGE
{
    MSIOBJECTHDR hdr;
    MSIDATABASE *db;
    INT version;
    enum platform platform;
    UINT num_langids;
    LANGID *langids;
    struct list patches;
    struct list components;
    struct list features;
    struct list files;
    struct list tempfiles;
    struct list folders;
    struct list binaries;
    LPWSTR ActionFormat;
    LPWSTR LastAction;
    HANDLE log_file;
    IAssemblyCache *cache_net[CLR_VERSION_MAX];
    IAssemblyCache *cache_sxs;

    struct list classes;
    struct list extensions;
    struct list progids;
    struct list mimes;
    struct list appids;

    struct tagMSISCRIPT *script;

    struct list RunningActions;

    LPWSTR BaseURL;
    LPWSTR PackagePath;
    LPWSTR ProductCode;

    UINT CurrentInstallState;
    msi_dialog *dialog;
    LPWSTR next_dialog;
    float center_x;
    float center_y;

    UINT WordCount;
    UINT Context;

    struct list subscriptions;

    struct list sourcelist_info;
    struct list sourcelist_media;

    unsigned char scheduled_action_running : 1;
    unsigned char commit_action_running : 1;
    unsigned char rollback_action_running : 1;
    unsigned char need_reboot : 1;
} MSIPACKAGE;

typedef struct tagMSIPREVIEW
{
    MSIOBJECTHDR hdr;
    MSIPACKAGE *package;
    msi_dialog *dialog;
} MSIPREVIEW;

#define MSI_MAX_PROPS 20

typedef struct tagMSISUMMARYINFO
{
    MSIOBJECTHDR hdr;
    IStorage *storage;
    DWORD update_count;
    PROPVARIANT property[MSI_MAX_PROPS];
} MSISUMMARYINFO;

typedef struct tagMSIFEATURE
{
    struct list entry;
    LPWSTR Feature;
    LPWSTR Feature_Parent;
    LPWSTR Title;
    LPWSTR Description;
    INT Display;
    INT Level;
    LPWSTR Directory;
    INT Attributes;
    INSTALLSTATE Installed;
    INSTALLSTATE ActionRequest;
    INSTALLSTATE Action;
    struct list Children;
    struct list Components;
} MSIFEATURE;

typedef struct tagMSIASSEMBLY
{
    LPWSTR feature;
    LPWSTR manifest;
    LPWSTR application;
    DWORD attributes;
    LPWSTR display_name;
    LPWSTR tempdir;
    BOOL installed;
    BOOL clr_version[CLR_VERSION_MAX];
} MSIASSEMBLY;

typedef struct tagMSICOMPONENT
{
    struct list entry;
    LPWSTR Component;
    LPWSTR ComponentId;
    LPWSTR Directory;
    INT Attributes;
    LPWSTR Condition;
    LPWSTR KeyPath;
    INSTALLSTATE Installed;
    INSTALLSTATE ActionRequest;
    INSTALLSTATE Action;
    BOOL ForceLocalState;
    BOOL Enabled;
    INT  Cost;
    INT  RefCount;
    LPWSTR FullKeypath;
    LPWSTR AdvertiseString;
    MSIASSEMBLY *assembly;

    unsigned int anyAbsent:1;
    unsigned int hasAdvertiseFeature:1;
    unsigned int hasLocalFeature:1;
    unsigned int hasSourceFeature:1;
} MSICOMPONENT;

typedef struct tagComponentList
{
    struct list entry;
    MSICOMPONENT *component;
} ComponentList;

typedef struct tagFeatureList
{
    struct list entry;
    MSIFEATURE *feature;
} FeatureList;

typedef struct tagMSIFOLDER
{
    struct list entry;
    LPWSTR Directory;
    LPWSTR Parent;
    LPWSTR TargetDefault;
    LPWSTR SourceLongPath;
    LPWSTR SourceShortPath;

    LPWSTR ResolvedTarget;
    LPWSTR ResolvedSource;
    LPWSTR Property;   /* initially set property */
    INT   State;
        /* 0 = uninitialized */
        /* 1 = existing */
        /* 2 = created remove if empty */
        /* 3 = created persist if empty */
    INT   Cost;
    INT   Space;
} MSIFOLDER;

typedef enum _msi_file_state {
    msifs_invalid,
    msifs_missing,
    msifs_overwrite,
    msifs_present,
    msifs_installed,
    msifs_skipped,
    msifs_hashmatch
} msi_file_state;

typedef struct tagMSIFILE
{
    struct list entry;
    LPWSTR File;
    MSICOMPONENT *Component;
    LPWSTR FileName;
    LPWSTR ShortName;
    LPWSTR LongName;
    INT FileSize;
    LPWSTR Version;
    LPWSTR Language;
    INT Attributes;
    INT Sequence;
    msi_file_state state;
    LPWSTR  TargetPath;
    BOOL IsCompressed;
    MSIFILEHASHINFO hash;
    UINT disk_id;
} MSIFILE;

typedef struct tagMSITEMPFILE
{
    struct list entry;
    LPWSTR Path;
} MSITEMPFILE;

typedef struct tagMSIAPPID
{
    struct list entry;
    LPWSTR AppID; /* Primary key */
    LPWSTR RemoteServerName;
    LPWSTR LocalServer;
    LPWSTR ServiceParameters;
    LPWSTR DllSurrogate;
    BOOL ActivateAtStorage;
    BOOL RunAsInteractiveUser;
} MSIAPPID;

typedef struct tagMSIPROGID MSIPROGID;

typedef struct tagMSICLASS
{
    struct list entry;
    LPWSTR clsid;     /* Primary Key */
    LPWSTR Context;   /* Primary Key */
    MSICOMPONENT *Component;
    MSIPROGID *ProgID;
    LPWSTR ProgIDText;
    LPWSTR Description;
    MSIAPPID *AppID;
    LPWSTR FileTypeMask;
    LPWSTR IconPath;
    LPWSTR DefInprocHandler;
    LPWSTR DefInprocHandler32;
    LPWSTR Argument;
    MSIFEATURE *Feature;
    INT Attributes;
    /* not in the table, set during installation */
    BOOL Installed;
} MSICLASS;

typedef struct tagMSIMIME MSIMIME;

typedef struct tagMSIEXTENSION
{
    struct list entry;
    LPWSTR Extension;  /* Primary Key */
    MSICOMPONENT *Component;
    MSIPROGID *ProgID;
    LPWSTR ProgIDText;
    MSIMIME *Mime;
    MSIFEATURE *Feature;
    /* not in the table, set during installation */
    BOOL Installed;
    struct list verbs;
} MSIEXTENSION;

struct tagMSIPROGID
{
    struct list entry;
    LPWSTR ProgID;  /* Primary Key */
    MSIPROGID *Parent;
    MSICLASS *Class;
    LPWSTR Description;
    LPWSTR IconPath;
    /* not in the table, set during installation */
    BOOL InstallMe;
    MSIPROGID *CurVer;
    MSIPROGID *VersionInd;
};

typedef struct tagMSIVERB
{
    struct list entry;
    LPWSTR Verb;
    INT Sequence;
    LPWSTR Command;
    LPWSTR Argument;
} MSIVERB;

struct tagMSIMIME
{
    struct list entry;
    LPWSTR ContentType;  /* Primary Key */
    MSIEXTENSION *Extension;
    LPWSTR suffix;
    LPWSTR clsid;
    MSICLASS *Class;
    /* not in the table, set during installation */
    BOOL InstallMe;
};

enum SCRIPTS {
    INSTALL_SCRIPT = 0,
    COMMIT_SCRIPT = 1,
    ROLLBACK_SCRIPT = 2,
    TOTAL_SCRIPTS = 3
};

#define SEQUENCE_UI       0x1
#define SEQUENCE_EXEC     0x2
#define SEQUENCE_INSTALL  0x10

typedef struct tagMSISCRIPT
{
    LPWSTR  *Actions[TOTAL_SCRIPTS];
    UINT    ActionCount[TOTAL_SCRIPTS];
    BOOL    ExecuteSequenceRun;
    BOOL    CurrentlyScripting;
    UINT    InWhatSequence;
    LPWSTR  *UniqueActions;
    UINT    UniqueActionsCount;
} MSISCRIPT;

#define MSIHANDLETYPE_ANY 0
#define MSIHANDLETYPE_DATABASE 1
#define MSIHANDLETYPE_SUMMARYINFO 2
#define MSIHANDLETYPE_VIEW 3
#define MSIHANDLETYPE_RECORD 4
#define MSIHANDLETYPE_PACKAGE 5
#define MSIHANDLETYPE_PREVIEW 6

#define MSI_MAJORVERSION 4
#define MSI_MINORVERSION 5
#define MSI_BUILDNUMBER 6001

#define GUID_SIZE 39
#define SQUISH_GUID_SIZE 33

#define MSIHANDLE_MAGIC 0x4d434923

DEFINE_GUID(CLSID_IMsiServer,   0x000C101C,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(CLSID_IMsiServerX1, 0x000C103E,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(CLSID_IMsiServerX2, 0x000C1090,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
DEFINE_GUID(CLSID_IMsiServerX3, 0x000C1094,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);

DEFINE_GUID(CLSID_IMsiServerMessage, 0x000C101D,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
/* handle unicode/ascii output in the Msi* API functions */
typedef struct {
    BOOL unicode;
    union {
       LPSTR a;
       LPWSTR w;
    } str;
} awstring;

typedef struct {
    BOOL unicode;
    union {
       LPCSTR a;
       LPCWSTR w;
    } str;
} awcstring;

UINT msi_strcpy_to_awstring( LPCWSTR str, awstring *awbuf, DWORD *sz );

/* msi server interface */
extern ITypeLib *get_msi_typelib( LPWSTR *path );
extern HRESULT create_msi_custom_remote( IUnknown *pOuter, LPVOID *ppObj );
extern HRESULT create_msi_remote_package( IUnknown *pOuter, LPVOID *ppObj );
extern HRESULT create_msi_remote_database( IUnknown *pOuter, LPVOID *ppObj );
extern IUnknown *msi_get_remote(MSIHANDLE handle);

/* handle functions */
extern void *msihandle2msiinfo(MSIHANDLE handle, UINT type);
extern MSIHANDLE alloc_msihandle( MSIOBJECTHDR * );
extern MSIHANDLE alloc_msi_remote_handle( IUnknown *unk );
extern void *alloc_msiobject(UINT type, UINT size, msihandledestructor destroy );
extern void msiobj_addref(MSIOBJECTHDR *);
extern int msiobj_release(MSIOBJECTHDR *);
extern void msiobj_lock(MSIOBJECTHDR *);
extern void msiobj_unlock(MSIOBJECTHDR *);
extern void msi_free_handle_table(void);

extern void free_cached_tables( MSIDATABASE *db );
extern UINT MSI_CommitTables( MSIDATABASE *db );


/* string table functions */
enum StringPersistence
{
    StringPersistent = 0,
    StringNonPersistent = 1
};

extern BOOL msi_addstringW( string_table *st, const WCHAR *data, int len, USHORT refcount, enum StringPersistence persistence );
extern UINT msi_string2idW( const string_table *st, LPCWSTR buffer, UINT *id );
extern VOID msi_destroy_stringtable( string_table *st );
extern const WCHAR *msi_string_lookup_id( const string_table *st, UINT id );
extern HRESULT msi_init_string_table( IStorage *stg );
extern string_table *msi_load_string_table( IStorage *stg, UINT *bytes_per_strref );
extern UINT msi_save_string_table( const string_table *st, IStorage *storage, UINT *bytes_per_strref );
extern UINT msi_get_string_table_codepage( const string_table *st );
extern UINT msi_set_string_table_codepage( string_table *st, UINT codepage );

extern BOOL TABLE_Exists( MSIDATABASE *db, LPCWSTR name );
extern MSICONDITION MSI_DatabaseIsTablePersistent( MSIDATABASE *db, LPCWSTR table );

extern UINT read_stream_data( IStorage *stg, LPCWSTR stname, BOOL table,
                              BYTE **pdata, UINT *psz );
extern UINT write_stream_data( IStorage *stg, LPCWSTR stname,
                               LPCVOID data, UINT sz, BOOL bTable );

/* transform functions */
extern UINT msi_table_apply_transform( MSIDATABASE *db, IStorage *stg );
extern UINT MSI_DatabaseApplyTransformW( MSIDATABASE *db,
                 LPCWSTR szTransformFile, int iErrorCond );
extern void append_storage_to_db( MSIDATABASE *db, IStorage *stg );

/* patch functions */
extern UINT msi_check_patch_applicable( MSIPACKAGE *package, MSISUMMARYINFO *si );
extern UINT msi_parse_patch_summary( MSISUMMARYINFO *si, MSIPATCHINFO **patch );
extern UINT msi_apply_patch_db( MSIPACKAGE *package, MSIDATABASE *patch_db, MSIPATCHINFO *patch );

/* action internals */
extern UINT MSI_InstallPackage( MSIPACKAGE *, LPCWSTR, LPCWSTR );
extern UINT ACTION_DialogBox( MSIPACKAGE*, LPCWSTR);
extern UINT ACTION_ForceReboot(MSIPACKAGE *package);
extern UINT MSI_Sequence( MSIPACKAGE *package, LPCWSTR szTable, INT iSequenceMode );
extern UINT MSI_SetFeatureStates( MSIPACKAGE *package );
extern UINT msi_parse_command_line( MSIPACKAGE *package, LPCWSTR szCommandLine,
                                    BOOL preserve_case );

/* record internals */
extern void MSI_CloseRecord( MSIOBJECTHDR * );
extern UINT MSI_RecordSetIStream( MSIRECORD *, UINT, IStream *);
extern UINT MSI_RecordGetIStream( MSIRECORD *, UINT, IStream **);
extern const WCHAR *MSI_RecordGetString( const MSIRECORD *, UINT );
extern MSIRECORD *MSI_CreateRecord( UINT );
extern UINT MSI_RecordSetInteger( MSIRECORD *, UINT, int );
extern UINT MSI_RecordSetIntPtr( MSIRECORD *, UINT, INT_PTR );
extern UINT MSI_RecordSetStringW( MSIRECORD *, UINT, LPCWSTR );
extern BOOL MSI_RecordIsNull( MSIRECORD *, UINT );
extern UINT MSI_RecordGetStringW( MSIRECORD * , UINT, LPWSTR, LPDWORD);
extern UINT MSI_RecordGetStringA( MSIRECORD *, UINT, LPSTR, LPDWORD);
extern int MSI_RecordGetInteger( MSIRECORD *, UINT );
extern INT_PTR MSI_RecordGetIntPtr( MSIRECORD *, UINT );
extern UINT MSI_RecordReadStream( MSIRECORD *, UINT, char *, LPDWORD);
extern UINT MSI_RecordSetStream(MSIRECORD *, UINT, IStream *);
extern UINT MSI_RecordGetFieldCount( const MSIRECORD *rec );
extern UINT MSI_RecordStreamToFile( MSIRECORD *, UINT, LPCWSTR );
extern UINT MSI_RecordSetStreamFromFileW( MSIRECORD *, UINT, LPCWSTR );
extern UINT MSI_RecordCopyField( MSIRECORD *, UINT, MSIRECORD *, UINT );
extern MSIRECORD *MSI_CloneRecord( MSIRECORD * );
extern BOOL MSI_RecordsAreEqual( MSIRECORD *, MSIRECORD * );

/* stream internals */
extern void enum_stream_names( IStorage *stg );
extern LPWSTR encode_streamname(BOOL bTable, LPCWSTR in);
extern BOOL decode_streamname(LPCWSTR in, LPWSTR out);

/* database internals */
extern UINT db_get_raw_stream( MSIDATABASE *, LPCWSTR, IStream ** );
void db_destroy_stream( MSIDATABASE *, LPCWSTR );
extern UINT MSI_OpenDatabaseW( LPCWSTR, LPCWSTR, MSIDATABASE ** );
extern UINT MSI_DatabaseOpenViewW(MSIDATABASE *, LPCWSTR, MSIQUERY ** );
extern UINT MSI_OpenQuery( MSIDATABASE *, MSIQUERY **, LPCWSTR, ... );
typedef UINT (*record_func)( MSIRECORD *, LPVOID );
extern UINT MSI_IterateRecords( MSIQUERY *, LPDWORD, record_func, LPVOID );
extern MSIRECORD *MSI_QueryGetRecord( MSIDATABASE *db, LPCWSTR query, ... );
extern UINT MSI_DatabaseGetPrimaryKeys( MSIDATABASE *, LPCWSTR, MSIRECORD ** );

/* view internals */
extern UINT MSI_ViewExecute( MSIQUERY*, MSIRECORD * );
extern UINT MSI_ViewFetch( MSIQUERY*, MSIRECORD ** );
extern UINT MSI_ViewClose( MSIQUERY* );
extern UINT MSI_ViewGetColumnInfo(MSIQUERY *, MSICOLINFO, MSIRECORD **);
extern UINT MSI_ViewModify( MSIQUERY *, MSIMODIFY, MSIRECORD * );
extern UINT VIEW_find_column( MSIVIEW *, LPCWSTR, LPCWSTR, UINT * );
extern UINT msi_view_get_row(MSIDATABASE *, MSIVIEW *, UINT, MSIRECORD **);

/* install internals */
extern UINT MSI_SetInstallLevel( MSIPACKAGE *package, int iInstallLevel );

/* package internals */
extern MSIPACKAGE *MSI_CreatePackage( MSIDATABASE *, LPCWSTR );
extern UINT MSI_OpenPackageW( LPCWSTR szPackage, MSIPACKAGE **pPackage );
extern UINT MSI_SetTargetPathW( MSIPACKAGE *, LPCWSTR, LPCWSTR );
extern INT MSI_ProcessMessage( MSIPACKAGE *, INSTALLMESSAGE, MSIRECORD * );
extern MSICONDITION MSI_EvaluateConditionW( MSIPACKAGE *, LPCWSTR );
extern UINT MSI_GetComponentStateW( MSIPACKAGE *, LPCWSTR, INSTALLSTATE *, INSTALLSTATE * );
extern UINT MSI_GetFeatureStateW( MSIPACKAGE *, LPCWSTR, INSTALLSTATE *, INSTALLSTATE * );
extern UINT WINAPI MSI_SetFeatureStateW(MSIPACKAGE*, LPCWSTR, INSTALLSTATE );
extern UINT msi_download_file( LPCWSTR szUrl, LPWSTR filename );
extern UINT msi_package_add_info(MSIPACKAGE *, DWORD, DWORD, LPCWSTR, LPWSTR);
extern UINT msi_package_add_media_disk(MSIPACKAGE *, DWORD, DWORD, DWORD, LPWSTR, LPWSTR);
extern UINT msi_clone_properties(MSIPACKAGE *);
extern UINT msi_set_context(MSIPACKAGE *);
extern void msi_adjust_privilege_properties(MSIPACKAGE *);
extern UINT MSI_GetFeatureCost(MSIPACKAGE *, MSIFEATURE *, MSICOSTTREE, INSTALLSTATE, LPINT);

/* for deformating */
extern UINT MSI_FormatRecordW( MSIPACKAGE *, MSIRECORD *, LPWSTR, LPDWORD );

/* registry data encoding/decoding functions */
extern BOOL unsquash_guid(LPCWSTR in, LPWSTR out);
extern BOOL squash_guid(LPCWSTR in, LPWSTR out);
extern BOOL encode_base85_guid(GUID *,LPWSTR);
extern BOOL decode_base85_guid(LPCWSTR,GUID*);
extern UINT MSIREG_OpenUninstallKey(MSIPACKAGE *package, HKEY *key, BOOL create);
extern UINT MSIREG_DeleteUninstallKey(MSIPACKAGE *package);
extern UINT MSIREG_OpenProductKey(LPCWSTR szProduct, LPCWSTR szUserSid,
                                  MSIINSTALLCONTEXT context, HKEY* key, BOOL create);
extern UINT MSIREG_OpenFeaturesKey(LPCWSTR szProduct, MSIINSTALLCONTEXT context,
                                   HKEY *key, BOOL create);
extern UINT MSIREG_OpenUserPatchesKey(LPCWSTR szPatch, HKEY* key, BOOL create);
UINT MSIREG_OpenUserDataFeaturesKey(LPCWSTR szProduct, MSIINSTALLCONTEXT context,
                                    HKEY *key, BOOL create);
extern UINT MSIREG_OpenUserComponentsKey(LPCWSTR szComponent, HKEY* key, BOOL create);
extern UINT MSIREG_OpenUserDataComponentKey(LPCWSTR szComponent, LPCWSTR szUserSid,
                                            HKEY *key, BOOL create);
extern UINT MSIREG_OpenPatchesKey(LPCWSTR szPatch, HKEY* key, BOOL create);
extern UINT MSIREG_OpenUserDataProductKey(LPCWSTR szProduct, MSIINSTALLCONTEXT dwContext,
                                          LPCWSTR szUserSid, HKEY *key, BOOL create);
extern UINT MSIREG_OpenUserDataPatchKey(LPCWSTR szPatch, MSIINSTALLCONTEXT dwContext,
                                        HKEY *key, BOOL create);
extern UINT MSIREG_OpenUserDataProductPatchesKey(LPCWSTR product, MSIINSTALLCONTEXT context,
                                                 HKEY *key, BOOL create);
extern UINT MSIREG_OpenInstallProps(LPCWSTR szProduct, MSIINSTALLCONTEXT dwContext,
                                    LPCWSTR szUserSid, HKEY *key, BOOL create);
extern UINT MSIREG_OpenUpgradeCodesKey(LPCWSTR szProduct, HKEY* key, BOOL create);
extern UINT MSIREG_OpenUserUpgradeCodesKey(LPCWSTR szProduct, HKEY* key, BOOL create);
extern UINT MSIREG_DeleteProductKey(LPCWSTR szProduct);
extern UINT MSIREG_DeleteUserProductKey(LPCWSTR szProduct);
extern UINT MSIREG_DeleteUserDataPatchKey(LPCWSTR patch, MSIINSTALLCONTEXT context);
extern UINT MSIREG_DeleteUserDataProductKey(LPCWSTR szProduct);
extern UINT MSIREG_DeleteUserFeaturesKey(LPCWSTR szProduct);
extern UINT MSIREG_DeleteUserDataComponentKey(LPCWSTR szComponent, LPCWSTR szUserSid);
extern UINT MSIREG_DeleteUserUpgradeCodesKey(LPCWSTR szUpgradeCode);
extern UINT MSIREG_DeleteClassesUpgradeCodesKey(LPCWSTR szUpgradeCode);
extern UINT MSIREG_OpenClassesUpgradeCodesKey(LPCWSTR szUpgradeCode, HKEY* key, BOOL create);
extern UINT MSIREG_DeleteLocalClassesProductKey(LPCWSTR szProductCode);
extern UINT MSIREG_DeleteLocalClassesFeaturesKey(LPCWSTR szProductCode);

extern LPWSTR msi_reg_get_val_str( HKEY hkey, LPCWSTR name );
extern BOOL msi_reg_get_val_dword( HKEY hkey, LPCWSTR name, DWORD *val);

extern DWORD msi_version_str_to_dword(LPCWSTR p);
extern void msi_parse_version_string(LPCWSTR, PDWORD, PDWORD);
extern VS_FIXEDFILEINFO *msi_get_disk_file_version(LPCWSTR);
extern int msi_compare_file_versions(VS_FIXEDFILEINFO *, const WCHAR *);
extern int msi_compare_font_versions(const WCHAR *, const WCHAR *);
extern DWORD msi_get_disk_file_size(LPCWSTR);
extern BOOL msi_file_hash_matches(MSIFILE *);

extern LONG msi_reg_set_val_str( HKEY hkey, LPCWSTR name, LPCWSTR value );
extern LONG msi_reg_set_val_multi_str( HKEY hkey, LPCWSTR name, LPCWSTR value );
extern LONG msi_reg_set_val_dword( HKEY hkey, LPCWSTR name, DWORD val );
extern LONG msi_reg_set_subkey_val( HKEY hkey, LPCWSTR path, LPCWSTR name, LPCWSTR val );

/* msi dialog interface */
typedef UINT (*msi_dialog_event_handler)( MSIPACKAGE*, LPCWSTR, LPCWSTR, msi_dialog* );
extern msi_dialog *msi_dialog_create( MSIPACKAGE*, LPCWSTR, msi_dialog*, msi_dialog_event_handler );
extern UINT msi_dialog_run_message_loop( msi_dialog* );
extern void msi_dialog_end_dialog( msi_dialog* );
extern void msi_dialog_check_messages( HANDLE );
extern void msi_dialog_do_preview( msi_dialog* );
extern void msi_dialog_destroy( msi_dialog* );
extern void msi_dialog_unregister_class( void );
extern void msi_dialog_handle_event( msi_dialog*, LPCWSTR, LPCWSTR, MSIRECORD * );
extern UINT msi_dialog_reset( msi_dialog *dialog );
extern UINT msi_dialog_directorylist_up( msi_dialog *dialog );
extern msi_dialog *msi_dialog_get_parent( msi_dialog *dialog );
extern LPWSTR msi_dialog_get_name( msi_dialog *dialog );
extern UINT msi_spawn_error_dialog( MSIPACKAGE*, LPWSTR, LPWSTR );

/* summary information */
extern MSISUMMARYINFO *MSI_GetSummaryInformationW( IStorage *stg, UINT uiUpdateCount );
extern LPWSTR msi_suminfo_dup_string( MSISUMMARYINFO *si, UINT uiProperty );
extern INT msi_suminfo_get_int32( MSISUMMARYINFO *si, UINT uiProperty );
extern LPWSTR msi_get_suminfo_product( IStorage *stg );
extern UINT msi_add_suminfo( MSIDATABASE *db, LPWSTR **records, int num_records, int num_columns );

/* undocumented functions */
UINT WINAPI MsiCreateAndVerifyInstallerDirectory( DWORD );
UINT WINAPI MsiDecomposeDescriptorW( LPCWSTR, LPWSTR, LPWSTR, LPWSTR, LPDWORD );
UINT WINAPI MsiDecomposeDescriptorA( LPCSTR, LPSTR, LPSTR, LPSTR, LPDWORD );
LANGID WINAPI MsiLoadStringW( MSIHANDLE, UINT, LPWSTR, int, LANGID );
LANGID WINAPI MsiLoadStringA( MSIHANDLE, UINT, LPSTR, int, LANGID );

/* UI globals */
extern INSTALLUILEVEL gUILevel;
extern HWND gUIhwnd;
extern INSTALLUI_HANDLERA gUIHandlerA;
extern INSTALLUI_HANDLERW gUIHandlerW;
extern INSTALLUI_HANDLER_RECORD gUIHandlerRecord;
extern DWORD gUIFilter;
extern LPVOID gUIContext;
extern WCHAR *gszLogFile;
extern HINSTANCE msi_hInstance;

/* action related functions */
extern UINT ACTION_PerformAction(MSIPACKAGE *package, const WCHAR *action, UINT script);
extern UINT ACTION_PerformUIAction(MSIPACKAGE *package, const WCHAR *action, UINT script);
extern void ACTION_FinishCustomActions( const MSIPACKAGE* package);
extern UINT ACTION_CustomAction(MSIPACKAGE *package,const WCHAR *action, UINT script, BOOL execute);

/* actions in other modules */
extern UINT ACTION_AppSearch(MSIPACKAGE *package);
extern UINT ACTION_CCPSearch(MSIPACKAGE *package);
extern UINT ACTION_FindRelatedProducts(MSIPACKAGE *package);
extern UINT ACTION_InstallFiles(MSIPACKAGE *package);
extern UINT ACTION_RemoveFiles(MSIPACKAGE *package);
extern UINT ACTION_MoveFiles(MSIPACKAGE *package);
extern UINT ACTION_DuplicateFiles(MSIPACKAGE *package);
extern UINT ACTION_RemoveDuplicateFiles(MSIPACKAGE *package);
extern UINT ACTION_RegisterClassInfo(MSIPACKAGE *package);
extern UINT ACTION_RegisterProgIdInfo(MSIPACKAGE *package);
extern UINT ACTION_RegisterExtensionInfo(MSIPACKAGE *package);
extern UINT ACTION_RegisterMIMEInfo(MSIPACKAGE *package);
extern UINT ACTION_RegisterFonts(MSIPACKAGE *package);
extern UINT ACTION_UnregisterClassInfo(MSIPACKAGE *package);
extern UINT ACTION_UnregisterExtensionInfo(MSIPACKAGE *package);
extern UINT ACTION_UnregisterFonts(MSIPACKAGE *package);
extern UINT ACTION_UnregisterMIMEInfo(MSIPACKAGE *package);
extern UINT ACTION_UnregisterProgIdInfo(MSIPACKAGE *package);
extern UINT ACTION_MsiPublishAssemblies(MSIPACKAGE *package);
extern UINT ACTION_MsiUnpublishAssemblies(MSIPACKAGE *package);

/* Helpers */
extern DWORD deformat_string(MSIPACKAGE *package, LPCWSTR ptr, WCHAR** data );
extern LPWSTR msi_dup_record_field(MSIRECORD *row, INT index);
extern LPWSTR msi_dup_property( MSIDATABASE *db, LPCWSTR prop );
extern UINT msi_set_property( MSIDATABASE *, LPCWSTR, LPCWSTR );
extern UINT msi_get_property( MSIDATABASE *, LPCWSTR, LPWSTR, LPDWORD );
extern int msi_get_property_int( MSIDATABASE *package, LPCWSTR prop, int def );
extern LPWSTR resolve_source_folder(MSIPACKAGE *package, LPCWSTR name, MSIFOLDER **folder);
extern LPWSTR resolve_target_folder(MSIPACKAGE *package, LPCWSTR name, BOOL set_prop, BOOL load_prop, MSIFOLDER **folder);
extern LPWSTR resolve_file_source(MSIPACKAGE *package, MSIFILE *file);
extern void msi_reset_folders( MSIPACKAGE *package, BOOL source );
extern MSICOMPONENT *get_loaded_component( MSIPACKAGE* package, LPCWSTR Component );
extern MSIFEATURE *get_loaded_feature( MSIPACKAGE* package, LPCWSTR Feature );
extern MSIFILE *get_loaded_file( MSIPACKAGE* package, LPCWSTR file );
extern MSIFOLDER *get_loaded_folder( MSIPACKAGE *package, LPCWSTR dir );
extern int track_tempfile(MSIPACKAGE *package, LPCWSTR path);
extern UINT schedule_action(MSIPACKAGE *package, UINT script, LPCWSTR action);
extern void msi_free_action_script(MSIPACKAGE *package, UINT script);
extern LPWSTR build_icon_path(MSIPACKAGE *, LPCWSTR);
extern LPWSTR build_directory_name(DWORD , ...);
extern BOOL create_full_pathW(const WCHAR *path);
extern void reduce_to_longfilename(WCHAR*);
extern LPWSTR create_component_advertise_string(MSIPACKAGE*, MSICOMPONENT*, LPCWSTR);
extern void ACTION_UpdateComponentStates(MSIPACKAGE *package, MSIFEATURE *feature);
extern UINT register_unique_action(MSIPACKAGE *, LPCWSTR);
extern BOOL check_unique_action(const MSIPACKAGE *, LPCWSTR);
extern WCHAR* generate_error_string(MSIPACKAGE *, UINT, DWORD, ... );
extern UINT msi_set_last_used_source(LPCWSTR product, LPCWSTR usersid,
                        MSIINSTALLCONTEXT context, DWORD options, LPCWSTR value);
extern UINT msi_get_local_package_name(LPWSTR path, LPCWSTR suffix);
extern UINT msi_set_sourcedir_props(MSIPACKAGE *package, BOOL replace);
extern MSIASSEMBLY *load_assembly(MSIPACKAGE *, MSICOMPONENT *);
extern UINT install_assembly(MSIPACKAGE *, MSICOMPONENT *);
extern WCHAR *font_version_from_file(const WCHAR *);
extern WCHAR **msi_split_string(const WCHAR *, WCHAR);

/* media */

typedef BOOL (*PMSICABEXTRACTCB)(MSIPACKAGE *, LPCWSTR, DWORD, LPWSTR *, DWORD *, PVOID);

#define MSICABEXTRACT_BEGINEXTRACT  0x01
#define MSICABEXTRACT_FILEEXTRACTED 0x02

typedef struct
{
    MSIPACKAGE* package;
    MSIMEDIAINFO *mi;
    PMSICABEXTRACTCB cb;
    LPWSTR curfile;
    PVOID user;
} MSICABDATA;

extern UINT ready_media(MSIPACKAGE *package, MSIFILE *file, MSIMEDIAINFO *mi);
extern UINT msi_load_media_info(MSIPACKAGE *package, MSIFILE *file, MSIMEDIAINFO *mi);
extern void msi_free_media_info(MSIMEDIAINFO *mi);
extern BOOL msi_cabextract(MSIPACKAGE* package, MSIMEDIAINFO *mi, LPVOID data);

/* control event stuff */
extern VOID ControlEvent_FireSubscribedEvent(MSIPACKAGE *package, LPCWSTR event,
                                      MSIRECORD *data);
extern VOID ControlEvent_CleanupDialogSubscriptions(MSIPACKAGE *package, LPWSTR dialog);
extern VOID ControlEvent_CleanupSubscriptions(MSIPACKAGE *package);
extern VOID ControlEvent_SubscribeToEvent(MSIPACKAGE *package, msi_dialog *dialog,
                                      LPCWSTR event, LPCWSTR control, LPCWSTR attribute);

/* OLE automation */
extern HRESULT create_msiserver(IUnknown *pOuter, LPVOID *ppObj);
extern HRESULT create_session(MSIHANDLE msiHandle, IDispatch *pInstaller, IDispatch **pDispatch);
extern HRESULT load_type_info(IDispatch *iface, ITypeInfo **pptinfo, REFIID clsid, LCID lcid);

/* Scripting */
extern DWORD call_script(MSIHANDLE hPackage, INT type, LPCWSTR script, LPCWSTR function, LPCWSTR action);

/* User Interface messages from the actions */
extern void ui_progress(MSIPACKAGE *, int, int, int, int);
extern void ui_actiondata(MSIPACKAGE *, LPCWSTR, MSIRECORD *);

/* common strings */
static const WCHAR cszSourceDir[] = {'S','o','u','r','c','e','D','i','r',0};
static const WCHAR cszSOURCEDIR[] = {'S','O','U','R','C','E','D','I','R',0};
static const WCHAR cszRootDrive[] = {'R','O','O','T','D','R','I','V','E',0};
static const WCHAR szLocalSid[] = {'S','-','1','-','5','-','1','8',0};
static const WCHAR szEmpty[] = {0};
static const WCHAR szAll[] = {'A','L','L',0};
static const WCHAR szOne[] = {'1',0};
static const WCHAR szZero[] = {'0',0};
static const WCHAR szSpace[] = {' ',0};
static const WCHAR szBackSlash[] = {'\\',0};
static const WCHAR szForwardSlash[] = {'/',0};
static const WCHAR szDot[] = {'.',0};
static const WCHAR szDotDot[] = {'.','.',0};
static const WCHAR szSemiColon[] = {';',0};
static const WCHAR szPreselected[] = {'P','r','e','s','e','l','e','c','t','e','d',0};
static const WCHAR szPatches[] = {'P','a','t','c','h','e','s',0};
static const WCHAR szState[] = {'S','t','a','t','e',0};
static const WCHAR szMsi[] = {'m','s','i',0};
static const WCHAR szPatch[] = {'P','A','T','C','H',0};
static const WCHAR szSourceList[] = {'S','o','u','r','c','e','L','i','s','t',0};
static const WCHAR szInstalled[] = {'I','n','s','t','a','l','l','e','d',0};
static const WCHAR szReinstall[] = {'R','E','I','N','S','T','A','L','L',0};
static const WCHAR szReinstallMode[] = {'R','E','I','N','S','T','A','L','L','M','O','D','E',0};
static const WCHAR szRemove[] = {'R','E','M','O','V','E',0};
static const WCHAR szUserSID[] = {'U','s','e','r','S','I','D',0};
static const WCHAR szProductCode[] = {'P','r','o','d','u','c','t','C','o','d','e',0};
static const WCHAR szRegisterClassInfo[] = {'R','e','g','i','s','t','e','r','C','l','a','s','s','I','n','f','o',0};
static const WCHAR szRegisterProgIdInfo[] = {'R','e','g','i','s','t','e','r','P','r','o','g','I','d','I','n','f','o',0};
static const WCHAR szRegisterExtensionInfo[] = {'R','e','g','i','s','t','e','r','E','x','t','e','n','s','i','o','n','I','n','f','o',0};
static const WCHAR szRegisterMIMEInfo[] = {'R','e','g','i','s','t','e','r','M','I','M','E','I','n','f','o',0};
static const WCHAR szDuplicateFiles[] = {'D','u','p','l','i','c','a','t','e','F','i','l','e','s',0};
static const WCHAR szRemoveDuplicateFiles[] = {'R','e','m','o','v','e','D','u','p','l','i','c','a','t','e','F','i','l','e','s',0};
static const WCHAR szInstallFiles[] = {'I','n','s','t','a','l','l','F','i','l','e','s',0};
static const WCHAR szRemoveFiles[] = {'R','e','m','o','v','e','F','i','l','e','s',0};
static const WCHAR szFindRelatedProducts[] = {'F','i','n','d','R','e','l','a','t','e','d','P','r','o','d','u','c','t','s',0};
static const WCHAR szAllUsers[] = {'A','L','L','U','S','E','R','S',0};
static const WCHAR szCustomActionData[] = {'C','u','s','t','o','m','A','c','t','i','o','n','D','a','t','a',0};
static const WCHAR szUILevel[] = {'U','I','L','e','v','e','l',0};
static const WCHAR szProductID[] = {'P','r','o','d','u','c','t','I','D',0};
static const WCHAR szPIDTemplate[] = {'P','I','D','T','e','m','p','l','a','t','e',0};
static const WCHAR szPIDKEY[] = {'P','I','D','K','E','Y',0};
static const WCHAR szTYPELIB[] = {'T','Y','P','E','L','I','B',0};
static const WCHAR szSumInfo[] = {5 ,'S','u','m','m','a','r','y','I','n','f','o','r','m','a','t','i','o','n',0};
static const WCHAR szHCR[] = {'H','K','E','Y','_','C','L','A','S','S','E','S','_','R','O','O','T','\\',0};
static const WCHAR szHCU[] = {'H','K','E','Y','_','C','U','R','R','E','N','T','_','U','S','E','R','\\',0};
static const WCHAR szHLM[] = {'H','K','E','Y','_','L','O','C','A','L','_','M','A','C','H','I','N','E','\\',0};
static const WCHAR szHU[] = {'H','K','E','Y','_','U','S','E','R','S','\\',0};
static const WCHAR szWindowsFolder[] = {'W','i','n','d','o','w','s','F','o','l','d','e','r',0};
static const WCHAR szAppSearch[] = {'A','p','p','S','e','a','r','c','h',0};
static const WCHAR szMoveFiles[] = {'M','o','v','e','F','i','l','e','s',0};
static const WCHAR szCCPSearch[] = {'C','C','P','S','e','a','r','c','h',0};
static const WCHAR szUnregisterClassInfo[] = {'U','n','r','e','g','i','s','t','e','r','C','l','a','s','s','I','n','f','o',0};
static const WCHAR szUnregisterExtensionInfo[] = {'U','n','r','e','g','i','s','t','e','r','E','x','t','e','n','s','i','o','n','I','n','f','o',0};
static const WCHAR szUnregisterMIMEInfo[] = {'U','n','r','e','g','i','s','t','e','r','M','I','M','E','I','n','f','o',0};
static const WCHAR szUnregisterProgIdInfo[] = {'U','n','r','e','g','i','s','t','e','r','P','r','o','g','I','d','I','n','f','o',0};
static const WCHAR szRegisterFonts[] = {'R','e','g','i','s','t','e','r','F','o','n','t','s',0};
static const WCHAR szUnregisterFonts[] = {'U','n','r','e','g','i','s','t','e','r','F','o','n','t','s',0};
static const WCHAR szCLSID[] = {'C','L','S','I','D',0};
static const WCHAR szProgID[] = {'P','r','o','g','I','D',0};
static const WCHAR szVIProgID[] = {'V','e','r','s','i','o','n','I','n','d','e','p','e','n','d','e','n','t','P','r','o','g','I','D',0};
static const WCHAR szAppID[] = {'A','p','p','I','D',0};
static const WCHAR szDefaultIcon[] = {'D','e','f','a','u','l','t','I','c','o','n',0};
static const WCHAR szInprocHandler[] = {'I','n','p','r','o','c','H','a','n','d','l','e','r',0};
static const WCHAR szInprocHandler32[] = {'I','n','p','r','o','c','H','a','n','d','l','e','r','3','2',0};
static const WCHAR szMIMEDatabase[] = {'M','I','M','E','\\','D','a','t','a','b','a','s','e','\\','C','o','n','t','e','n','t',' ','T','y','p','e','\\',0};
static const WCHAR szLocalPackage[] = {'L','o','c','a','l','P','a','c','k','a','g','e',0};
static const WCHAR szOriginalDatabase[] = {'O','r','i','g','i','n','a','l','D','a','t','a','b','a','s','e',0};
static const WCHAR szUpgradeCode[] = {'U','p','g','r','a','d','e','C','o','d','e',0};
static const WCHAR szAdminUser[] = {'A','d','m','i','n','U','s','e','r',0};
static const WCHAR szIntel[] = {'I','n','t','e','l',0};
static const WCHAR szIntel64[] = {'I','n','t','e','l','6','4',0};
static const WCHAR szX64[] = {'x','6','4',0};
static const WCHAR szAMD64[] = {'A','M','D','6','4',0};
static const WCHAR szWow6432NodeCLSID[] = {'W','o','w','6','4','3','2','N','o','d','e','\\','C','L','S','I','D',0};
static const WCHAR szWow6432Node[] = {'W','o','w','6','4','3','2','N','o','d','e',0};
static const WCHAR szStreams[] = {'_','S','t','r','e','a','m','s',0};
static const WCHAR szStorages[] = {'_','S','t','o','r','a','g','e','s',0};
static const WCHAR szMsiPublishAssemblies[] = {'M','s','i','P','u','b','l','i','s','h','A','s','s','e','m','b','l','i','e','s',0};

/* memory allocation macro functions */
static void *msi_alloc( size_t len ) __WINE_ALLOC_SIZE(1);
static inline void *msi_alloc( size_t len )
{
    return HeapAlloc( GetProcessHeap(), 0, len );
}

static void *msi_alloc_zero( size_t len ) __WINE_ALLOC_SIZE(1);
static inline void *msi_alloc_zero( size_t len )
{
    return HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, len );
}

static void *msi_realloc( void *mem, size_t len ) __WINE_ALLOC_SIZE(2);
static inline void *msi_realloc( void *mem, size_t len )
{
    return HeapReAlloc( GetProcessHeap(), 0, mem, len );
}

static void *msi_realloc_zero( void *mem, size_t len ) __WINE_ALLOC_SIZE(2);
static inline void *msi_realloc_zero( void *mem, size_t len )
{
    return HeapReAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, mem, len );
}

static inline BOOL msi_free( void *mem )
{
    return HeapFree( GetProcessHeap(), 0, mem );
}

static inline char *strdupWtoA( LPCWSTR str )
{
    LPSTR ret = NULL;
    DWORD len;

    if (!str) return ret;
    len = WideCharToMultiByte( CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
    ret = msi_alloc( len );
    if (ret)
        WideCharToMultiByte( CP_ACP, 0, str, -1, ret, len, NULL, NULL );
    return ret;
}

static inline LPWSTR strdupAtoW( LPCSTR str )
{
    LPWSTR ret = NULL;
    DWORD len;

    if (!str) return ret;
    len = MultiByteToWideChar( CP_ACP, 0, str, -1, NULL, 0 );
    ret = msi_alloc( len * sizeof(WCHAR) );
    if (ret)
        MultiByteToWideChar( CP_ACP, 0, str, -1, ret, len );
    return ret;
}

static inline LPWSTR strdupW( LPCWSTR src )
{
    LPWSTR dest;
    if (!src) return NULL;
    dest = msi_alloc( (lstrlenW(src)+1)*sizeof(WCHAR) );
    if (dest)
        lstrcpyW(dest, src);
    return dest;
}

#endif /* __WINE_MSI_PRIVATE__ */
