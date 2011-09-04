/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/include/ps.h
 * PURPOSE:         Internal header for the Process Manager
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

//
// Define this if you want debugging support
//
#define _PS_DEBUG_                                      0x00

//
// These define the Debug Masks Supported
//
#define PS_THREAD_DEBUG                                 0x01
#define PS_PROCESS_DEBUG                                0x02
#define PS_SECURITY_DEBUG                               0x04
#define PS_JOB_DEBUG                                    0x08
#define PS_NOTIFICATIONS_DEBUG                          0x10
#define PS_WIN32K_DEBUG                                 0x20
#define PS_STATE_DEBUG                                  0x40
#define PS_QUOTA_DEBUG                                  0x80
#define PS_KILL_DEBUG                                   0x100
#define PS_REF_DEBUG                                    0x200

//
// Debug/Tracing support
//
#if _PS_DEBUG_
#ifdef NEW_DEBUG_SYSTEM_IMPLEMENTED // enable when Debug Filters are implemented
#define PSTRACE(x, ...)                                     \
    {                                                       \
        DbgPrintEx("%s [%.16s] - ",                         \
                   __FUNCTION__,                            \
                   PsGetCurrentProcess()->ImageFileName);   \
        DbgPrintEx(__VA_ARGS__);                            \
    }
#else
#define PSTRACE(x, ...)                                     \
    if (x & PspTraceLevel)                                  \
    {                                                       \
        DbgPrint("%s [%.16s] - ",                           \
                 __FUNCTION__,                              \
                 PsGetCurrentProcess()->ImageFileName);     \
        DbgPrint(__VA_ARGS__);                              \
    }
#endif
#define PSREFTRACE(x)                                       \
    PSTRACE(PS_REF_DEBUG,                                   \
            "Pointer Count [%p] @%d: %lx\n",                \
            x,                                              \
            __LINE__,                                       \
            OBJECT_TO_OBJECT_HEADER(x)->PointerCount)
#else
#define PSTRACE(x, fmt, ...) DPRINT(fmt, ##__VA_ARGS__)
#define PSREFTRACE(x)
#endif

//
// Maximum Count of Notification Routines
//
#define PSP_MAX_CREATE_THREAD_NOTIFY            8
#define PSP_MAX_LOAD_IMAGE_NOTIFY               8
#define PSP_MAX_CREATE_PROCESS_NOTIFY           8

//
// Maximum Job Scheduling Classes
//
#define PSP_JOB_SCHEDULING_CLASSES              10

//
// Thread "Set/Get Context" Context Structure
//
typedef struct _GET_SET_CTX_CONTEXT
{
    KAPC Apc;
    KEVENT Event;
    KPROCESSOR_MODE Mode;
    CONTEXT Context;
} GET_SET_CTX_CONTEXT, *PGET_SET_CTX_CONTEXT;

//
// Initialization Functions
//
VOID
NTAPI
PspShutdownProcessManager(
    VOID
);

BOOLEAN
NTAPI
PsInitSystem(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
);

//
// Utility Routines
//
PETHREAD
NTAPI
PsGetNextProcessThread(
    IN PEPROCESS Process,
    IN PETHREAD Thread OPTIONAL
);

PEPROCESS
NTAPI
PsGetNextProcess(
    IN PEPROCESS OldProcess OPTIONAL
);

NTSTATUS
NTAPI
PspMapSystemDll(
    IN PEPROCESS Process,
    OUT PVOID *DllBase,
    IN BOOLEAN UseLargePages
);

NTSTATUS
NTAPI
PsLocateSystemDll(
    VOID
);

NTSTATUS
NTAPI
PspGetSystemDllEntryPoints(
    VOID
);

VOID
NTAPI
PsChangeQuantumTable(
    IN BOOLEAN Immediate,
    IN ULONG PrioritySeparation
);

NTSTATUS
NTAPI
PsReferenceProcessFilePointer(
    IN PEPROCESS Process,
    OUT PFILE_OBJECT *FileObject
);

//
// Process Routines
//
NTSTATUS
NTAPI
PspCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess OPTIONAL,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN BOOLEAN InJob
);

//
// Security Routines
//
PACCESS_TOKEN
NTAPI
PsReferenceEffectiveToken(
    IN PETHREAD Thread,
    OUT PTOKEN_TYPE TokenType,
    OUT PUCHAR b,
    OUT PSECURITY_IMPERSONATION_LEVEL Level
);

NTSTATUS
NTAPI
PsOpenTokenOfProcess(
    IN HANDLE ProcessHandle,
    OUT PACCESS_TOKEN* Token
);

NTSTATUS
NTAPI
PspSetPrimaryToken(
    IN PEPROCESS Process,
    IN HANDLE TokenHandle OPTIONAL,
    IN PACCESS_TOKEN Token OPTIONAL
);

NTSTATUS
NTAPI
PspInitializeProcessSecurity(
    IN PEPROCESS Process,
    IN PEPROCESS Parent OPTIONAL
);

VOID
NTAPI
PspDeleteProcessSecurity(
    IN PEPROCESS Process
);

VOID
NTAPI
PspDeleteThreadSecurity(
    IN PETHREAD Thread
);

//
// Reaping and Deletion
//
VOID
NTAPI
PsExitSpecialApc(
    PKAPC Apc,
    PKNORMAL_ROUTINE *NormalRoutine,
    PVOID *NormalContext,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
);

VOID
NTAPI
PspReapRoutine(
    IN PVOID Context
);

VOID
NTAPI
PspExitThread(
    IN NTSTATUS ExitStatus
);

NTSTATUS
NTAPI
PspTerminateThreadByPointer(
    IN PETHREAD Thread,
    IN NTSTATUS ExitStatus,
    IN BOOLEAN bSelf
);

VOID
NTAPI
PspExitProcess(
    IN BOOLEAN LastThread,
    IN PEPROCESS Process
);

NTSTATUS
NTAPI
PsTerminateProcess(
    IN PEPROCESS Process,
    IN NTSTATUS ExitStatus
);

VOID
NTAPI
PspDeleteProcess(
    IN PVOID ObjectBody
);

VOID
NTAPI
PspDeleteThread(
    IN PVOID ObjectBody
);

//
// Thread/Process Startup
//
VOID
NTAPI
PspSystemThreadStartup(
    PKSTART_ROUTINE StartRoutine,
    PVOID StartContext
);

VOID
NTAPI
PsIdleThreadMain(
    IN PVOID Context
);

//
// Quota Support
//
VOID
NTAPI
PspInheritQuota(
    IN PEPROCESS Process,
    IN PEPROCESS ParentProcess
);

VOID
NTAPI
PspDestroyQuotaBlock(
    IN PEPROCESS Process
);

#if defined(_X86_)
//
// VDM and LDT Support
//
NTSTATUS
NTAPI
PspDeleteLdt(
    IN PEPROCESS Process
);

NTSTATUS
NTAPI
PspDeleteVdmObjects(
    IN PEPROCESS Process
);

NTSTATUS
NTAPI
PspQueryDescriptorThread(
    IN PETHREAD Thread,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
);
#endif

//
// Job Routines
//
VOID
NTAPI
PspExitProcessFromJob(
    IN PEJOB Job,
    IN PEPROCESS Process
);

VOID
NTAPI
PspRemoveProcessFromJob(
    IN PEPROCESS Process,
    IN PEJOB Job
);

VOID
NTAPI
PspInitializeJobStructures(
    VOID
);

VOID
NTAPI
PspDeleteJob(
    IN PVOID ObjectBody
);

//
// State routines
//
NTSTATUS
NTAPI
PsResumeThread(
    IN PETHREAD Thread,
    OUT PULONG PreviousCount OPTIONAL
);

NTSTATUS
NTAPI
PsSuspendThread(
    IN PETHREAD Thread,
    OUT PULONG PreviousCount OPTIONAL
);

VOID
NTAPI
PspGetOrSetContextKernelRoutine(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE* NormalRoutine,
    IN OUT PVOID* NormalContext,
    IN OUT PVOID* SystemArgument1,
    IN OUT PVOID* SystemArgument2
);

//
// Process Quotas
//
NTSTATUS
NTAPI
PsReturnProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
);

NTSTATUS
NTAPI
PsChargeProcessPageFileQuota(
    IN PEPROCESS Process,
    IN SIZE_T Amount
);

BOOLEAN
NTAPI
PspIsProcessExiting(IN PEPROCESS Process);

//
// Global data inside the Process Manager
//
extern ULONG PspTraceLevel;
extern LCID PsDefaultThreadLocaleId;
extern LCID PsDefaultSystemLocaleId;
extern LIST_ENTRY PspReaperListHead;
extern WORK_QUEUE_ITEM PspReaperWorkItem;
extern BOOLEAN PspReaping;
extern PEPROCESS PsIdleProcess;
extern LIST_ENTRY PsActiveProcessHead;
extern KGUARDED_MUTEX PspActiveProcessMutex;
extern LARGE_INTEGER ShortPsLockDelay;
extern EPROCESS_QUOTA_BLOCK PspDefaultQuotaBlock;
extern PHANDLE_TABLE PspCidTable;
extern EX_CALLBACK PspThreadNotifyRoutine[PSP_MAX_CREATE_THREAD_NOTIFY];
extern EX_CALLBACK PspProcessNotifyRoutine[PSP_MAX_CREATE_PROCESS_NOTIFY];
extern EX_CALLBACK PspLoadImageNotifyRoutine[PSP_MAX_LOAD_IMAGE_NOTIFY];
extern PLEGO_NOTIFY_ROUTINE PspLegoNotifyRoutine;
extern ULONG PspThreadNotifyRoutineCount, PspProcessNotifyRoutineCount;
extern BOOLEAN PsImageNotifyEnabled;
extern PKWIN32_PROCESS_CALLOUT PspW32ProcessCallout;
extern PKWIN32_THREAD_CALLOUT PspW32ThreadCallout;
extern PVOID PspSystemDllEntryPoint;
extern PVOID PspSystemDllBase;
extern BOOLEAN PspUseJobSchedulingClasses;
extern CHAR PspJobSchedulingClasses[PSP_JOB_SCHEDULING_CLASSES];
extern ULONG PsRawPrioritySeparation;
extern POBJECT_TYPE _PsThreadType, _PsProcessType;
extern PTOKEN PspBootAccessToken;
extern GENERIC_MAPPING PspJobMapping;
extern POBJECT_TYPE PsJobType;
extern LARGE_INTEGER ShortPsLockDelay;
extern UNICODE_STRING PsNtDllPathName;
extern LIST_ENTRY PsLoadedModuleList;
extern KSPIN_LOCK PsLoadedModuleSpinLock;
extern ERESOURCE PsLoadedModuleResource;
extern ULONG_PTR PsNtosImageBase;

//
// Inlined Functions
//
#include "ps_x.h"
