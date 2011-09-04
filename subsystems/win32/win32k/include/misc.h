#pragma once

typedef struct INTENG_ENTER_LEAVE_TAG
  {
  /* Contents is private to EngEnter/EngLeave */
  SURFOBJ *DestObj;
  SURFOBJ *OutputObj;
  HBITMAP OutputBitmap;
  CLIPOBJ *TrivialClipObj;
  RECTL DestRect;
  BOOL ReadOnly;
  } INTENG_ENTER_LEAVE, *PINTENG_ENTER_LEAVE;

extern BOOL APIENTRY IntEngEnter(PINTENG_ENTER_LEAVE EnterLeave,
                                SURFOBJ *DestObj,
                                RECTL *DestRect,
                                BOOL ReadOnly,
                                POINTL *Translate,
                                SURFOBJ **OutputObj);

extern BOOL APIENTRY IntEngLeave(PINTENG_ENTER_LEAVE EnterLeave);

extern HGDIOBJ StockObjects[];
extern SHORT gusLanguageID;

SHORT FASTCALL IntGdiGetLanguageID(VOID);
VOID FASTCALL IntUserManualGuiCheck(LONG Check);
PVOID APIENTRY HackSecureVirtualMemory(IN PVOID,IN SIZE_T,IN ULONG,OUT PVOID *);
VOID APIENTRY HackUnsecureVirtualMemory(IN PVOID);

NTSTATUS
NTAPI
RegOpenKey(
    LPCWSTR pwszKeyName,
    PHKEY phkey);

NTSTATUS
NTAPI
RegQueryValue(
    IN HKEY hkey,
    IN PCWSTR pwszValueName,
    IN ULONG ulType,
    OUT PVOID pvData,
    IN OUT PULONG pcbValue);

VOID
NTAPI
RegWriteSZ(HKEY hkey, PWSTR pwszValue, PWSTR pwszData);

VOID
NTAPI
RegWriteDWORD(HKEY hkey, PWSTR pwszValue, DWORD dwData);

BOOL
NTAPI
RegReadDWORD(HKEY hkey, PWSTR pwszValue, PDWORD pdwData);

BOOL
NTAPI
RegReadUserSetting(
    IN PCWSTR pwszKeyName,
    IN PCWSTR pwszValueName,
    IN ULONG ulType,
    OUT PVOID pvData,
    IN ULONG cbDataSize);

BOOL
NTAPI
RegWriteUserSetting(
    IN PCWSTR pwszKeyName,
    IN PCWSTR pwszValueName,
    IN ULONG ulType,
    OUT PVOID pvData,
    IN ULONG cbDataSize);

VOID FASTCALL
SetLastNtError(
  NTSTATUS Status);

typedef struct _GDI_POOL *PGDI_POOL;

PGDI_POOL
NTAPI
GdiPoolCreate(
    ULONG cjAllocSize,
    ULONG ulTag);

VOID
NTAPI
GdiPoolDestroy(PGDI_POOL pPool);

PVOID
NTAPI
GdiPoolAllocate(
    PGDI_POOL pPool);

VOID
NTAPI
GdiPoolFree(
    PGDI_POOL pPool,
    PVOID pvAlloc);

FORCEINLINE
VOID
ExAcquirePushLockExclusive(PEX_PUSH_LOCK PushLock)
{
    /* Try acquiring the lock */
    if (InterlockedBitTestAndSet((PLONG)PushLock, EX_PUSH_LOCK_LOCK_V))
    {
        /* Someone changed it, use the slow path */
        ExfAcquirePushLockExclusive(PushLock);
    }
}

FORCEINLINE
VOID
ExReleasePushLockExclusive(PEX_PUSH_LOCK PushLock)
{
    EX_PUSH_LOCK OldValue;

    /* Unlock the pushlock */
    OldValue.Value = InterlockedExchangeAddSizeT((PSIZE_T)PushLock,
                                                 -(SSIZE_T)EX_PUSH_LOCK_LOCK);
    /* Check if anyone is waiting on it and it's not already waking */
    if ((OldValue.Waiting) && !(OldValue.Waking))
    {
        /* Wake it up */
        ExfTryToWakePushLock(PushLock);
    }
}

FORCEINLINE
VOID
_ExInitializePushLock(PEX_PUSH_LOCK Lock)
{
    *(PULONG_PTR)Lock = 0;
}
#define ExInitializePushLock _ExInitializePushLock

NTSTATUS FASTCALL
IntSafeCopyUnicodeString(PUNICODE_STRING Dest,
                         PUNICODE_STRING Source);

NTSTATUS FASTCALL
IntSafeCopyUnicodeStringTerminateNULL(PUNICODE_STRING Dest,
                                      PUNICODE_STRING Source);


#define ROUND_DOWN(n, align) \
    (((ULONG)n) & ~((align) - 1l))

#define ROUND_UP(n, align) \
    ROUND_DOWN(((ULONG)n) + (align) - 1, (align))

#define LIST_FOR_EACH(elem, list, type, field) \
    for ((elem) = CONTAINING_RECORD((list)->Flink, type, field); \
         &(elem)->field != (list) && ((&((elem)->field)) != NULL); \
         (elem) = CONTAINING_RECORD((elem)->field.Flink, type, field))

#define LIST_FOR_EACH_SAFE(cursor, cursor2, list, type, field) \
    for ((cursor) = CONTAINING_RECORD((list)->Flink, type, field), \
         (cursor2) = CONTAINING_RECORD((cursor)->field.Flink, type, field); \
         &(cursor)->field != (list) && ((&((cursor)->field)) != NULL); \
         (cursor) = (cursor2), \
         (cursor2) = CONTAINING_RECORD((cursor)->field.Flink, type, field))
