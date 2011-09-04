#pragma once

typedef enum _LOG_EVENT_TYPE
{
    EVENT_ALLOCATE,
    EVENT_CREATE_HANDLE,
    EVENT_REFERENCE,
    EVENT_DEREFERENCE,
    EVENT_LOCK,
    EVENT_UNLOCK,
    EVENT_DELETE,
    EVENT_FREE,
    EVENT_SET_OWNER,
} LOG_EVENT_TYPE;

typedef struct _LOGENTRY
{
    SLIST_ENTRY sleLink;
    LOG_EVENT_TYPE nEventType;
    DWORD dwProcessId;
    DWORD dwThreadId;
    ULONG ulUnique;
    LPARAM lParam;
    PVOID apvBackTrace[20];
    union
    {
        ULONG_PTR data1;
    } data;
} LOGENTRY, *PLOGENTRY;

#if DBG_ENABLE_EVENT_LOGGING
VOID NTAPI DbgDumpEventList(PSLIST_HEADER pslh);
VOID NTAPI DbgLogEvent(PSLIST_HEADER pslh, LOG_EVENT_TYPE nEventType, LPARAM lParam);
VOID NTAPI DbgCleanupEventList(PSLIST_HEADER pslh);
#define DBG_LOGEVENT(pslh, type, val) DbgLogEvent(pslh, type, (ULONG_PTR)val)
#define DBG_INITLOG(pslh) InitializeSListHead(pslh)
#define DBG_DUMP_EVENT_LIST(pslh) DbgDumpEventList(pslh)
#define DBG_CLEANUP_EVENT_LIST(pslh) DbgCleanupEventList(pslh)
#else
#define DBG_LOGEVENT(pslh, type, val)
#define DBG_INITLOG(pslh)
#define DBG_DUMP_EVENT_LIST(pslh)
#define DBG_CLEANUP_EVENT_LIST(pslh)
#endif


VOID NTAPI DbgDumpGdiHandleTable(VOID);
ULONG NTAPI DbgCaptureStackBackTace(PVOID* pFrames, ULONG nFramesToCapture);
BOOL NTAPI DbgGdiHTIntegrityCheck(VOID);
VOID NTAPI DbgDumpLockedGdiHandles(VOID);

#define KeRosDumpStackFrames(Frames, Count) KdSystemDebugControl('DsoR', (PVOID)Frames, Count, NULL, 0, NULL, KernelMode)
NTSYSAPI ULONG APIENTRY RtlWalkFrameChain(OUT PVOID *Callers, IN ULONG Count, IN ULONG Flags);

#if DBG
void
NTAPI
DbgPreServiceHook(ULONG ulSyscallId, PULONG_PTR pulArguments);

ULONG_PTR
NTAPI
DbgPostServiceHook(ULONG ulSyscallId, ULONG_PTR ulResult);

#define ID_Win32PreServiceHook 'WSH0'
#define ID_Win32PostServiceHook 'WSH1'

FORCEINLINE void
GdiDbgAssertNoLocks(char * pszFile, ULONG nLine)
{
    PTHREADINFO pti = (PTHREADINFO)PsGetCurrentThreadWin32Thread();
    if (pti && pti->cExclusiveLocks != 0)
    {
        DbgPrint("(%s:%ld) There are %ld exclusive locks!\n",
                 pszFile, nLine, pti->cExclusiveLocks);
        ASSERT(FALSE);
    }
}

#define ASSERT_NOGDILOCKS() GdiDbgAssertNoLocks(__FILE__,__LINE__)
#else
#define ASSERT_NOGDILOCKS()
#endif

