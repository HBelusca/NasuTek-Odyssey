/*
 *  GDI object common header definition
 *
 */

#pragma once

/* The first 10 entries are never used in windows, they are empty */
#define RESERVE_ENTRIES_COUNT 10

typedef struct _GDI_HANDLE_TABLE
{
/* The table must be located at the beginning of this structure so it can be
 * properly mapped!
 */
//////////////////////////////////////////////////////////////////////////////
  GDI_TABLE_ENTRY Entries[GDI_HANDLE_COUNT];
  DEVCAPS         DevCaps;                 // Device Capabilities.
  FLONG           flDeviceUniq;            // Device settings uniqueness.
  PVOID           pvLangPack;              // Language Pack.
  CFONT           cfPublic[GDI_CFONT_MAX]; // Public Fonts.
  DWORD           dwCFCount;


} GDI_HANDLE_TABLE, *PGDI_HANDLE_TABLE;

extern PGDI_HANDLE_TABLE GdiHandleTable;

typedef PVOID PGDIOBJ;

typedef BOOL (NTAPI *GDICLEANUPPROC)(PVOID ObjectBody);

/* Every GDI Object must have this standard type of header.
 * It's for thread locking. */
typedef struct _BASEOBJECT
{
    HGDIOBJ hHmgr;
    union {
        ULONG ulShareCount; /* For objects without a handle */
        DWORD dwThreadId;   /* Exclusive lock owner */
    };
    USHORT cExclusiveLock;
    USHORT BaseFlags;
    EX_PUSH_LOCK pushlock;
#if DBG_ENABLE_EVENT_LOGGING
    SLIST_HEADER slhLog;
#endif
} BASEOBJECT, *POBJ;

enum BASEFLAGS
{
    BASEFLAG_LOOKASIDE = 0x80,

    /* Odyssey specific: */
    BASEFLAG_READY_TO_DIE = 0x1000
};

typedef struct _CLIENTOBJ
{
  BASEOBJECT BaseObject;
} CLIENTOBJ, *PCLIENTOBJ;

#define GDIOBJFLAG_DEFAULT	(0x0)
#define GDIOBJFLAG_IGNOREPID 	(0x1)
#define GDIOBJFLAG_IGNORELOCK 	(0x2)

INIT_FUNCTION
NTSTATUS
NTAPI
InitGdiHandleTable(VOID);

BOOL
NTAPI
GreIsHandleValid(
    HGDIOBJ hobj);

BOOL
NTAPI
GreDeleteObject(
    HGDIOBJ hObject);

ULONG
NTAPI
GreGetObjectOwner(
    HGDIOBJ hobj);

BOOL
NTAPI
GreSetObjectOwner(
    HGDIOBJ hobj,
    ULONG ulOwner);

INT
NTAPI
GreGetObject(
    IN HGDIOBJ hobj,
    IN INT cbCount,
    IN PVOID pvBuffer);

POBJ
NTAPI
GDIOBJ_AllocateObject(
    UCHAR objt,
    ULONG cjSize,
    FLONG fl);

VOID
NTAPI
GDIOBJ_vDeleteObject(
    POBJ pobj);

POBJ
NTAPI
GDIOBJ_ReferenceObjectByHandle(
    HGDIOBJ hobj,
    UCHAR objt);

VOID
NTAPI
GDIOBJ_vReferenceObjectByPointer(
    POBJ pobj);

VOID
NTAPI
GDIOBJ_vDereferenceObject(
    POBJ pobj);

PGDIOBJ
NTAPI
GDIOBJ_LockObject(
    HGDIOBJ hobj,
    UCHAR objt);

VOID
NTAPI
GDIOBJ_vUnlockObject(
    POBJ pobj);

VOID
NTAPI
GDIOBJ_vSetObjectOwner(
    POBJ pobj,
    ULONG ulOwner);

BOOL
NTAPI
GDIOBJ_bLockMultipleObjects(
    ULONG ulCount,
    HGDIOBJ* ahObj,
    PGDIOBJ* apObj,
    UCHAR objt);

HGDIOBJ
NTAPI
GDIOBJ_hInsertObject(
    POBJ pobj,
    ULONG ulOwner);

VOID
NTAPI
GDIOBJ_vFreeObject(
    POBJ pobj);

VOID
NTAPI
GDIOBJ_vSetObjectAttr(
    POBJ pobj,
    PVOID pvObjAttr);

PVOID
NTAPI
GDIOBJ_pvGetObjectAttr(
    POBJ pobj);

BOOL    NTAPI GDIOBJ_ConvertToStockObj(HGDIOBJ *hObj);
POBJ    NTAPI GDIOBJ_AllocObjWithHandle(ULONG ObjectType, ULONG cjSize);
PGDIOBJ NTAPI GDIOBJ_ShareLockObj(HGDIOBJ hObj, DWORD ObjectType);
PVOID   NTAPI GDI_MapHandleTable(PEPROCESS Process);

