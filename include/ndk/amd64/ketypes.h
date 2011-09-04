/*++ NDK Version: 0098

Copyright (c) Alex Ionescu.  All rights reserved.
Copyright (c) Timo Kreuzer.  All rights reserved.

Header Name:

    ketypes.h (AMD64)

Abstract:

    amd64 Type definitions for the Kernel services.

Author:

    Alex Ionescu (alexi@tinykrnl.org) - Updated - 27-Feb-2006
    Timo Kreuzer (timo.kreuzer@odyssey.org) - Updated - 14-Aug-2008

--*/

#ifndef _AMD64_KETYPES_H
#define _AMD64_KETYPES_H

//
// Dependencies
//

//
// KPCR Access for non-IA64 builds
//
//#define K0IPCR                  ((ULONG_PTR)(KIP0PCRADDRESS))
//#define PCR                     ((volatile KPCR * const)K0IPCR)
#define PCR ((volatile KPCR * const)__readgsqword(FIELD_OFFSET(KPCR, Self)))
//#if defined(CONFIG_SMP) || defined(NT_BUILD)
//#undef  KeGetPcr
//#define KeGetPcr()              ((volatile KPCR * const)__readfsdword(0x1C))
//#endif

//
// Machine Types
//
#define MACHINE_TYPE_ISA        0x0000
#define MACHINE_TYPE_EISA       0x0001
#define MACHINE_TYPE_MCA        0x0002

//
// X86 80386 Segment Types
//
#define I386_TASK_GATE          0x5
#define I386_TSS                0x9
#define I386_ACTIVE_TSS         0xB
#define I386_CALL_GATE          0xC
#define I386_INTERRUPT_GATE     0xE
#define I386_TRAP_GATE          0xF

//
// Selector Names
//
#define RPL_MASK                0x0003
#define MODE_MASK               0x0001
#define KGDT64_NULL             0x0000
#define KGDT64_R0_CODE          0x0010
#define KGDT64_R0_DATA          0x0018
#define KGDT64_R3_CMCODE        0x0020
#define KGDT64_R3_DATA          0x0028
#define KGDT64_R3_CODE          0x0030
#define KGDT64_SYS_TSS          0x0040
#define KGDT64_R3_CMTEB         0x0050


//
// CR4
//
#define CR4_VME                 0x1
#define CR4_PVI                 0x2
#define CR4_TSD                 0x4
#define CR4_DE                  0x8
#define CR4_PSE                 0x10
#define CR4_PAE                 0x20
#define CR4_MCE                 0x40
#define CR4_PGE                 0x80
#define CR4_FXSR                0x200
#define CR4_XMMEXCPT            0x400
#define CR4_CHANNELS            0x800

//
// EFlags
//
#define EFLAGS_CF               0x01L
#define EFLAGS_ZF               0x40L
#define EFLAGS_TF               0x100L
#define EFLAGS_INTERRUPT_MASK   0x200L
#define EFLAGS_DF               0x400L
#define EFLAGS_NESTED_TASK      0x4000L
#define EFLAGS_V86_MASK         0x20000
#define EFLAGS_ALIGN_CHECK      0x40000
#define EFLAGS_VIF              0x80000
#define EFLAGS_VIP              0x100000
#define EFLAGS_USER_SANITIZE    0x3F4DD7
#define EFLAG_SIGN              0x8000
#define EFLAG_ZERO              0x4000
#define EFLAGS_TF_MASK          0x0100
#define EFLAGS_TF_SHIFT         0x0008
#define EFLAGS_ID_MASK          0x200000
#define EFLAGS_IF_MASK          0x0200
#define EFLAGS_IF_SHIFT         0x0009

//
// Machine Specific Registers
//
#define MSR_EFER         0xC0000080
#define MSR_STAR         0xC0000081
#define MSR_LSTAR        0xC0000082
#define MSR_CSTAR        0xC0000083
#define MSR_SYSCALL_MASK 0xC0000084
#define MSR_FS_BASE      0xC0000100
#define MSR_GS_BASE      0xC0000101
#define MSR_GS_SWAP      0xC0000102
#define MSR_MCG_STATUS   0x017A
#define MSR_AMD_ACCESS   0x9C5A203A

//
// Flags in MSR_EFER
//
#define MSR_LMA                 0x0400
#define MSR_LME                 0x0100
#define MSR_SCE                 0x0001
#define MSR_NXE                 0x0800
#define MSR_PAT                 0x0277
#define MSR_DEGUG_CTL           0x01D9
#define MSR_LAST_BRANCH_FROM    0x01DB
#define MSR_LAST_BRANCH_TO      0x01DC
#define MSR_LAST_EXCEPTION_FROM 0x01DD
#define MSR_LAST_EXCEPTION_TO   0x01DE

//
// IPI Types
//
#define IPI_APC                 1
#define IPI_DPC                 2
#define IPI_FREEZE              4
#define IPI_PACKET_READY        8
#define IPI_SYNCH_REQUEST       16

//
// PRCB Flags
//
#define PRCB_MAJOR_VERSION      1
#define PRCB_BUILD_DEBUG        1
#define PRCB_BUILD_UNIPROCESSOR 2

//
// Service Table
//
#define NUMBER_SERVICE_TABLES 2
#define SERVICE_NUMBER_MASK 0xFFF
#define SERVICE_TABLE_SHIFT 7
#define SERVICE_TABLE_MASK 0x20
#define SERVICE_TABLE_TEST 0x20

//
// HAL Variables
//
#define INITIAL_STALL_COUNT     100

//
// IOPM Definitions
//
#define IO_ACCESS_MAP_NONE      0
#define IOPM_OFFSET             FIELD_OFFSET(KTSS, IoMaps[0].IoMap)
#define KiComputeIopmOffset(MapNumber)              \
    (MapNumber == IO_ACCESS_MAP_NONE) ?             \
        (USHORT)(sizeof(KTSS)) :                    \
        (USHORT)(FIELD_OFFSET(KTSS, IoMaps[MapNumber-1].IoMap))

//
// Static Kernel-Mode Address start (use MM_KSEG0_BASE for actual)
//
#define KSEG0_BASE 0xfffff80000000000ULL

//
// Synchronization-level IRQL
//
#ifndef CONFIG_SMP
#define SYNCH_LEVEL             DISPATCH_LEVEL
#else
#define SYNCH_LEVEL             (IPI_LEVEL - 2)
#endif

#define NMI_STACK_SIZE 0x2000

//
// Trap Frame Definition
//
typedef struct _KTRAP_FRAME
{
    UINT64 P1Home;
    UINT64 P2Home;
    UINT64 P3Home;
    UINT64 P4Home;
    UINT64 P5;
    CHAR PreviousMode;
    UCHAR PreviousIrql;
    UCHAR FaultIndicator;
    UCHAR ExceptionActive;
    ULONG MxCsr;
    UINT64 Rax;
    UINT64 Rcx;
    UINT64 Rdx;
    UINT64 R8;
    UINT64 R9;
    UINT64 R10;
    UINT64 R11;
    union
    {
        UINT64 GsBase;
        UINT64 GsSwap;
    };
    M128A Xmm0;
    M128A Xmm1;
    M128A Xmm2;
    M128A Xmm3;
    M128A Xmm4;
    M128A Xmm5;
    union
    {
        UINT64 FaultAddress;
        UINT64 ContextRecord;
        UINT64 TimeStampCKCL;
    };
    UINT64 Dr0;
    UINT64 Dr1;
    UINT64 Dr2;
    UINT64 Dr3;
    UINT64 Dr6;
    UINT64 Dr7;
    union
    {
        struct
        {
            UINT64 DebugControl;
            UINT64 LastBranchToRip;
            UINT64 LastBranchFromRip;
            UINT64 LastExceptionToRip;
            UINT64 LastExceptionFromRip;
        };
        struct
        {
            UINT64 LastBranchControl;
            ULONG LastBranchMSR;
        };
    };
    USHORT SegDs;
    USHORT SegEs;
    USHORT SegFs;
    USHORT SegGs;
    UINT64 TrapFrame;
    UINT64 Rbx;
    UINT64 Rdi;
    UINT64 Rsi;
    UINT64 Rbp;
    union
    {
        UINT64 ErrorCode;
        UINT64 ExceptionFrame;
        UINT64 TimeStampKlog;
    };
    UINT64 Rip;
    USHORT SegCs;
    UCHAR Fill0;
    UCHAR Logging;
    USHORT Fill1[2];
    ULONG EFlags;
    ULONG Fill2;
    UINT64 Rsp;
    USHORT SegSs;
    USHORT Fill3;
    LONG CodePatchCycle;
} KTRAP_FRAME, *PKTRAP_FRAME;

//
// Defines the Callback Stack Layout for User Mode Callbacks
//
typedef struct _KCALLOUT_FRAME
{
    ULONG64 InitialStack;
    ULONG64 TrapFrame;
    ULONG64 CallbackStack;
    ULONG64 Rdi;
    ULONG64 Rsi;
    ULONG64 Rbx;
    ULONG64 Rbp;
    ULONG64 ReturnAddress;
    ULONG64 Result;
    ULONG64 ResultLength;
} KCALLOUT_FRAME, *PKCALLOUT_FRAME;

//
// Dummy LDT_ENTRY
//
#ifndef _LDT_ENTRY_DEFINED
#define _LDT_ENTRY_DEFINED
typedef ULONG LDT_ENTRY;
#endif

//
// GDT Entry Definition
//
typedef union _KGDTENTRY64
{
    struct
    {
        USHORT LimitLow;
        USHORT BaseLow;
        union
        {
            struct
            {
                UCHAR BaseMiddle;
                UCHAR Flags1;
                UCHAR Flags2;
                UCHAR BaseHigh;
            } Bytes;
            struct
            {
                ULONG BaseMiddle:8;
                ULONG Type:5;
                ULONG Dpl:2;
                ULONG Present:1;
                ULONG LimitHigh:4;
                ULONG System:1;
                ULONG LongMode:1;
                ULONG DefaultBig:1;
                ULONG Granularity:1;
                ULONG BaseHigh:8;
            } Bits;
        };
        ULONG BaseUpper;
        ULONG MustBeZero;
    };
    UINT64 Alignment;
} KGDTENTRY64, *PKGDTENTRY64;
#define KGDTENTRY KGDTENTRY64
#define PKGDTENTRY PKGDTENTRY64

//
// IDT Entry Access Definition
//
typedef struct _KIDT_ACCESS
{
    union
    {
        struct
        {
            UCHAR Reserved;
            UCHAR SegmentType:4;
            UCHAR SystemSegmentFlag:1;
            UCHAR Dpl:2;
            UCHAR Present:1;
        };
        USHORT Value;
    };
} KIDT_ACCESS, *PKIDT_ACCESS;

//
// IDT Entry Definition
//
typedef union _KIDTENTRY64
{
    struct
    {
        USHORT OffsetLow;
        USHORT Selector;
        USHORT IstIndex:3;
        USHORT Reserved0:5;
        USHORT Type:5;
        USHORT Dpl:2;
        USHORT Present:1;
        USHORT OffsetMiddle;
        ULONG OffsetHigh;
        ULONG Reserved1;
    };
    UINT64 Alignment;
} KIDTENTRY64, *PKIDTENTRY64;
#define KIDTENTRY KIDTENTRY64
#define PKIDTENTRY PKIDTENTRY64

typedef struct _KDESCRIPTOR
{
    USHORT Pad[3];
    USHORT Limit;
    PVOID Base;
} KDESCRIPTOR, *PKDESCRIPTOR;

#ifndef NTOS_MODE_USER

//
// Special Registers Structure (outside of CONTEXT)
//
typedef struct _KSPECIAL_REGISTERS
{
    UINT64 Cr0;
    UINT64 Cr2;
    UINT64 Cr3;
    UINT64 Cr4;
    UINT64 KernelDr0;
    UINT64 KernelDr1;
    UINT64 KernelDr2;
    UINT64 KernelDr3;
    UINT64 KernelDr6;
    UINT64 KernelDr7;
    struct _KDESCRIPTOR Gdtr;
    struct _KDESCRIPTOR Idtr;
    USHORT Tr;
    USHORT Ldtr;
    ULONG MxCsr;
    UINT64 DebugControl;
    UINT64 LastBranchToRip;
    UINT64 LastBranchFromRip;
    UINT64 LastExceptionToRip;
    UINT64 LastExceptionFromRip;
    UINT64 Cr8;
    UINT64 MsrGsBase;
    UINT64 MsrGsSwap;
    UINT64 MsrStar;
    UINT64 MsrLStar;
    UINT64 MsrCStar;
    UINT64 MsrSyscallMask;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Processor State Data
//
typedef struct _KPROCESSOR_STATE
{
    KSPECIAL_REGISTERS SpecialRegisters;
    CONTEXT ContextFrame;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

#if (NTDDI_VERSION >= NTDDI_LONGHORN)
typedef struct _GENERAL_LOOKASIDE_POOL
{
    union
    {
        SLIST_HEADER ListHead;
        SINGLE_LIST_ENTRY SingleListHead;
    };
    USHORT Depth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    union
    {
        ULONG AllocateMisses;
        ULONG AllocateHits;
    };
    union
    {
        ULONG TotalFrees;
        ULONG FreeMisses;
    };
    ULONG FreeHits;
    POOL_TYPE Type;
    ULONG Tag;
    ULONG Size;
    union
    {
        PVOID AllocateEx;
        PVOID Allocate;
    };
    union
    {
        PVOID FreeEx;
        PVOID Free;
    };
    LIST_ENTRY ListEntry;
    ULONG LastTotalAllocates;
    union
    {
        ULONG LastAllocateMisses;
        ULONG LastAllocateHits;
    };
    ULONG Future[2];
} GENERAL_LOOKASIDE_POOL, *PGENERAL_LOOKASIDE_POOL;
#else
#define GENERAL_LOOKASIDE_POOL PP_LOOKASIDE_LIST
#endif

typedef struct _KREQUEST_PACKET
{
    PVOID CurrentPacket[3];
    PVOID WorkerRoutine;
} KREQUEST_PACKET, *PKREQUEST_PACKET;

typedef struct _REQUEST_MAILBOX
{
    INT64 RequestSummary;
    KREQUEST_PACKET RequestPacket;
    PVOID Virtual[7];
} REQUEST_MAILBOX, *PREQUEST_MAILBOX;

//
// Processor Region Control Block
//
#pragma pack(push,4)
typedef struct _KPRCB
{
    ULONG MxCsr;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    USHORT Number;
#else
    UCHAR Number;
    UCHAR NestingLevel;
#endif
    UCHAR InterruptRequest;
    UCHAR IdleHalt;
    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UCHAR NestingLevel;
    UCHAR Group;
    UCHAR PrcbPad00[6];
#else
    UINT64 UserRsp;
#endif
    UINT64 RspBase;
    UINT64 PrcbLock;
    UINT64 SetMember;
    KPROCESSOR_STATE ProcessorState;
    CHAR CpuType;
    CHAR CpuID;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    union
    {
        USHORT CpuStep;
        struct
        {
            UCHAR CpuStepping;
            UCHAR CpuModel;
        };
    };
#else
    USHORT CpuStep;
#endif
    ULONG MHz;
    UINT64 HalReserved[8];
    USHORT MinorVersion;
    USHORT MajorVersion;
    UCHAR BuildType;
    UCHAR CpuVendor;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UCHAR CoresPerPhysicalProcessor;
    UCHAR LogicalProcessorsPerCore;
#else
    UCHAR InitialApicId;
    UCHAR LogicalProcessorsPerPhysicalProcessor;
#endif
    ULONG ApicMask;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG CFlushSize;
#else
    UCHAR CFlushSize;
    UCHAR PrcbPad0x[3];
#endif
    PVOID AcpiReserved;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG InitialApicId;
    ULONG Stride;
    UINT64 PrcbPad01[3];
#else
    UINT64 PrcbPad00[4];
#endif
    KSPIN_LOCK_QUEUE LockQueue[LockQueueMaximumLock]; // 2003: 33, vista:49
    PP_LOOKASIDE_LIST PPLookasideList[16];
    GENERAL_LOOKASIDE_POOL PPNPagedLookasideList[32];
    GENERAL_LOOKASIDE_POOL PPPagedLookasideList[32];
    UINT64 PacketBarrier;
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    LONG MmPageFaultCount;
    LONG MmCopyOnWriteCount;
    LONG MmTransitionCount;
#if (NTDDI_VERSION < NTDDI_LONGHORN)
    LONG MmCacheTransitionCount;
#endif
    LONG MmDemandZeroCount;
    LONG MmPageReadCount;
    LONG MmPageReadIoCount;
#if (NTDDI_VERSION < NTDDI_LONGHORN)
    LONG MmCacheReadCount;
    LONG MmCacheIoCount;
#endif
    LONG MmDirtyPagesWriteCount;
    LONG MmDirtyWriteIoCount;
    LONG MmMappedPagesWriteCount;
    LONG MmMappedWriteIoCount;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG KeSystemCalls;
    ULONG KeContextSwitches;
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadNotPossible;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
    LONG LookasideIrpFloat;
#else
    LONG LookasideIrpFloat;
    ULONG KeSystemCalls;
#endif
    LONG IoReadOperationCount;
    LONG IoWriteOperationCount;
    LONG IoOtherOperationCount;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
#if (NTDDI_VERSION < NTDDI_LONGHORN)
    ULONG KeContextSwitches;
    UCHAR PrcbPad2[12];
#endif
    UINT64 TargetSet;
    ULONG IpiFrozen;
    UCHAR PrcbPad3[116];
    REQUEST_MAILBOX RequestMailbox[64];
    UINT64 SenderSummary;
    UCHAR PrcbPad4[120];
    KDPC_DATA DpcData[2];
    PVOID DpcStack;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID SparePtr0;
#else
    PVOID SavedRsp;
#endif
    LONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    UCHAR DpcInterruptRequested;
    UCHAR DpcThreadRequested;
    UCHAR DpcRoutineActive;
    UCHAR DpcThreadActive;
    UINT64 TimerHand;
    UINT64 TimerRequest;
    LONG TickOffset;
    LONG MasterOffset;
    ULONG DpcLastCount;
    UCHAR ThreadDpcEnable;
    UCHAR QuantumEnd;
    UCHAR PrcbPad50;
    UCHAR IdleSchedule;
    LONG DpcSetEventRequest;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    ULONG KeExceptionDispatchCount;
#else
    LONG PrcbPad40;
    PVOID DpcThread;
#endif
    KEVENT DpcEvent;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    PVOID PrcbPad51;
#endif
    KDPC CallDpc;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    LONG ClockKeepAlive;
    UCHAR ClockCheckSlot;
    UCHAR ClockPollCycle;
    UCHAR PrcbPad6[2];
    LONG DpcWatchdogPeriod;
    LONG DpcWatchdogCount;
    UINT64 PrcbPad70[2];
#else
    UINT64 PrcbPad7[4];
#endif
    LIST_ENTRY WaitListHead;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UINT64 WaitLock;
#endif
    ULONG ReadySummary;
    ULONG QueueIndex;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UINT64 PrcbPad71[12];
#endif
    LIST_ENTRY DispatcherReadyListHead[32];
    ULONG InterruptCount;
    ULONG KernelTime;
    ULONG UserTime;
    ULONG DpcTime;
    ULONG InterruptTime;
    ULONG AdjustDpcThreshold;
    UCHAR SkipTick;
    UCHAR DebuggerSavedIRQL;
    UCHAR PollSlot;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UCHAR PrcbPad80[5];
    ULONG DpcTimeCount;
    ULONG DpcTimeLimit;
    ULONG PeriodicCount;
    ULONG PeriodicBias;
    UINT64 PrcbPad81[2];
#else
    UCHAR PrcbPad8[13];
#endif
    struct _KNODE *ParentNode;
    UINT64 MultiThreadProcessorSet;
    struct _KPRCB *MultiThreadSetMaster;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UINT64 StartCycles;
    LONG MmSpinLockOrdering;
    ULONG PageColor;
    ULONG NodeColor;
    ULONG NodeShiftedColor;
    ULONG SecondaryColorMask;
#endif
    LONG Sleeping;
#if (NTDDI_VERSION >= NTDDI_LONGHORN)
    UINT64 CycleTime;
    ULONG CcFastMdlReadNoWait;
    ULONG CcFastMdlReadWait;
    ULONG CcFastMdlReadNotPossible;
    ULONG CcMapDataNoWait;
    ULONG CcMapDataWait;
    ULONG CcPinMappedDataCount;
    ULONG CcPinReadNoWait;
    ULONG CcPinReadWait;
    ULONG CcMdlReadNoWait;
    ULONG CcMdlReadWait;
    ULONG CcLazyWriteHotSpots;
    ULONG CcLazyWriteIos;
    ULONG CcLazyWritePages;
    ULONG CcDataFlushes;
    ULONG CcDataPages;
    ULONG CcLostDelayedWrites;
    ULONG CcFastReadResourceMiss;
    ULONG CcCopyReadWaitMiss;
    ULONG CcFastMdlReadResourceMiss;
    ULONG CcMapDataNoWaitMiss;
    ULONG CcMapDataWaitMiss;
    ULONG CcPinReadNoWaitMiss;
    ULONG CcPinReadWaitMiss;
    ULONG CcMdlReadNoWaitMiss;
    ULONG CcMdlReadWaitMiss;
    ULONG CcReadAheadIos;
    LONG MmCacheTransitionCount;
    LONG MmCacheReadCount;
    LONG MmCacheIoCount;
    ULONG PrcbPad91[3];
    PROCESSOR_POWER_STATE PowerState;
    ULONG KeAlignmentFixupCount;
    UCHAR VendorString[13];
    UCHAR PrcbPad10[3];
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;
    KDPC DpcWatchdogDpc;
    KTIMER DpcWatchdogTimer;
    CACHE_DESCRIPTOR Cache[5];
    ULONG CacheCount;
    ULONG CachedCommit;
    ULONG CachedResidentAvailable;
    PVOID HyperPte;
    PVOID WheaInfo;
    PVOID EtwSupport;
    SLIST_HEADER InterruptObjectPool;
    SLIST_HEADER HypercallPageList;
    PVOID HypercallPageVirtual;
    PVOID VirtualApicAssist;
    UINT64* StatisticsPage;
    PVOID RateControl;
    UINT64 CacheProcessorMask[5];
    UINT64 PackageProcessorSet;
    UINT64 CoreProcessorSet;
#else
    ULONG PrcbPad90[1];
    ULONG DebugDpcTime;
    ULONG PageColor;
    ULONG NodeColor;
    ULONG NodeShiftedColor;
    ULONG SecondaryColorMask;
    UCHAR PrcbPad9[12];
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadNotPossible;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
    ULONG KeAlignmentFixupCount;
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;
    UCHAR VendorString[13];
    UCHAR PrcbPad10[2];
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;
    PROCESSOR_POWER_STATE PowerState;
    CACHE_DESCRIPTOR Cache[5];
    ULONG CacheCount;
#endif
}
 KPRCB, *PKPRCB;

//
// Processor Control Region
//
typedef struct _KIPCR
{
    union
    {
        NT_TIB NtTib;
        struct
        {
            union _KGDTENTRY64 *GdtBase;
            struct _KTSS64 *TssBase;
            ULONG64 UserRsp;
            struct _KPCR *Self;
            struct _KPRCB *CurrentPrcb;
            PKSPIN_LOCK_QUEUE LockArray;
            PVOID Used_Self;
        };
    };
    union _KIDTENTRY64 *IdtBase;
    ULONG64 Unused[2];
    KIRQL Irql;
    UCHAR SecondLevelCacheAssociativity;
    UCHAR ObsoleteNumber;
    UCHAR Fill0;
    ULONG Unused0[3];
    USHORT MajorVersion;
    USHORT MinorVersion;
    ULONG StallScaleFactor;
    PVOID Unused1[3];
    ULONG KernelReserved[15];
    ULONG SecondLevelCacheSize;
    ULONG HalReserved[16];
    ULONG Unused2;
    ULONG Fill1;
    PVOID KdVersionBlock; // 0x108
    PVOID Unused3;
    ULONG PcrAlign1[24];
    ULONG Fill2[2]; // 0x178
    KPRCB Prcb; // 0x180

    // hack:
    ULONG ContextSwitches;

} KIPCR, *PKIPCR;
#pragma pack(pop)

//
// TSS Definition
//
typedef struct _KiIoAccessMap
{
    UCHAR DirectionMap[32];
    UCHAR IoMap[8196];
} KIIO_ACCESS_MAP;


#pragma pack(push,4)
typedef struct _KTSS64
{
 /* 000 */  ULONG Reserved0;
 /* 004 */  UINT64 Rsp0;
 /* 00c */  UINT64 Rsp1;
 /* 014 */  UINT64 Rsp2;
 /* 01c */  UINT64 Ist[8];
 /* 05c */  UINT64 Reserved1;
 /* 064 */  USHORT Reserved2;
 /* 066 */  USHORT IoMapBase;
} KTSS64, *PKTSS64;
#pragma pack(pop)
#define KTSS KTSS64
#define PKTSS PKTSS64

//
// KEXCEPTION_FRAME
//
typedef struct _KEXCEPTION_FRAME
{
    UINT64 P1Home;
    UINT64 P2Home;
    UINT64 P3Home;
    UINT64 P4Home;
    UINT64 P5;
    UINT64 InitialStack;
    M128A Xmm6;
    M128A Xmm7;
    M128A Xmm8;
    M128A Xmm9;
    M128A Xmm10;
    M128A Xmm11;
    M128A Xmm12;
    M128A Xmm13;
    M128A Xmm14;
    M128A Xmm15;
    UINT64 TrapFrame;
    UINT64 CallbackStack;
    UINT64 OutputBuffer;
    UINT64 OutputLength;
    UINT64 MxCsr;
    UINT64 Rbp;
    UINT64 Rbx;
    UINT64 Rdi;
    UINT64 Rsi;
    UINT64 R12;
    UINT64 R13;
    UINT64 R14;
    UINT64 R15;
    UINT64 Return;
} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

typedef struct _DISPATCHER_CONTEXT
{
    ULONG64 ControlPc;
    PVOID ImageBase;
    PVOID FunctionEntry;
    PVOID EstablisherFrame;
    ULONG64 TargetIp;
    PVOID ContextRecord;
    PVOID LanguageHandler;
    PVOID HandlerData;
    PVOID HistoryTable;
    ULONG ScopeIndex;
} DISPATCHER_CONTEXT, *PDISPATCHER_CONTEXT;


typedef struct _KSTART_FRAME
{
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 Return;
} KSTART_FRAME, *PKSTART_FRAME;

typedef struct _KSWITCH_FRAME
{
    ULONG64 P1Home;
    ULONG64 P2Home;
    ULONG64 P3Home;
    ULONG64 P4Home;
    ULONG64 P5Home;
    ULONG64 ApcBypass;
    ULONG64 Rbp;
    ULONG64 Return;
} KSWITCH_FRAME, *PKSWITCH_FRAME;

#if 0
typedef struct _KTIMER_TABLE_ENTRY
{
    ULONG_PTR Lock;
    LIST_ENTRY Entry;
    ULARGE_INTEGER Time;
} KTIMER_TABLE_ENTRY, *PKTIMER_TABLE_ENTRY;

typedef struct _KTIMER_TABLE
{
    KTIMER* TimerExpiry[64];
    KTIMER_TABLE_ENTRY TimerEntries[256];
} KTIMER_TABLE, *PKTIMER_TABLE;
#endif

//
// Inline function to get current KPRCB
//
FORCEINLINE
struct _KPRCB *
KeGetCurrentPrcb(VOID)
{
    return (struct _KPRCB *)__readgsqword(FIELD_OFFSET(KIPCR, CurrentPrcb));
}

#endif
#endif
