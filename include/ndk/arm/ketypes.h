/*++ NDK Version: 0098

Copyright (c) Alex Ionescu.  All rights reserved.

Header Name:

    ketypes.h (ARM)

Abstract:

    ARM Type definitions for the Kernel services.

Author:

    Alex Ionescu (alexi@tinykrnl.org) - Updated - 27-Feb-2006

--*/

#ifndef _ARM_KETYPES_H
#define _ARM_KETYPES_H

//
// Dependencies
//

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
// No LDTs on ARM
//
#define LDT_ENTRY              ULONG

//
// HAL Variables
//
#define INITIAL_STALL_COUNT     100

//
// Static Kernel-Mode Address start (use MM_KSEG0_BASE for actual)
//
#define KSEG0_BASE              0x80000000

//
// Trap Frame Definition
//
typedef struct _KTRAP_FRAME
{
    ULONG DbgArgMark;
    ULONG R0;
    ULONG R1;
    ULONG R2;
    ULONG R3;
    ULONG R4;
    ULONG R5;
    ULONG R6;
    ULONG R7;
    ULONG R8;
    ULONG R9;
    ULONG R10;
    ULONG R11;
    ULONG R12;
    ULONG UserSp;
    ULONG UserLr;
    ULONG SvcSp;
    ULONG SvcLr;
    ULONG Pc;
    ULONG Spsr;
    ULONG OldIrql;
    ULONG PreviousMode;
    ULONG PreviousTrapFrame;
} KTRAP_FRAME, *PKTRAP_FRAME;

//
// Defines the Callback Stack Layout for User Mode Callbacks
//
// Just a stub with some required members for now
//
typedef struct _KCALLOUT_FRAME
{
    ULONG CallbackStack;
    ULONG DummyFramePointer;
} KCALLOUT_FRAME, *PKCALLOUT_FRAME;

#ifndef NTOS_MODE_USER

//
// Exception Frame Definition
//
typedef struct _KEXCEPTION_FRAME
{
  //  ULONG R0;
//    ULONG R1;
//    ULONG R2;
//    ULONG R3;    
    ULONG R4;
    ULONG R5;
    ULONG R6;
    ULONG R7;
    ULONG R8;
    ULONG R9;
    ULONG R10;
    ULONG R11;
//    ULONG R12;
    ULONG Lr;
    ULONG Psr;
} KEXCEPTION_FRAME, *PKEXCEPTION_FRAME;

//
// ARM Internal Registers
//
typedef union _ARM_TTB_REGISTER
{
    struct
    {
        ULONG Reserved:14;
        ULONG BaseAddress:18;
    };
    ULONG AsUlong;
} ARM_TTB_REGISTER;

typedef union _ARM_STATUS_REGISTER
{
    
    struct
    {
        ULONG Mode:5;
        ULONG State:1;
        ULONG FiqDisable:1;
        ULONG IrqDisable:1;
        ULONG ImpreciseAbort:1;
        ULONG Endianness:1;
        ULONG Sbz:6;
        ULONG GreaterEqual:4;
        ULONG Sbz1:4;
        ULONG Java:1;
        ULONG Sbz2:2;
        ULONG StickyOverflow:1;
        ULONG Overflow:1;
        ULONG CarryBorrowExtend:1;
        ULONG Zero:1;
        ULONG NegativeLessThan:1;
    };
    ULONG AsUlong;
} ARM_STATUS_REGISTER;

typedef union _ARM_DOMAIN_REGISTER
{
    struct
    {
        ULONG Domain0:2;
        ULONG Domain1:2;
        ULONG Domain2:2;
        ULONG Domain3:2;
        ULONG Domain4:2;
        ULONG Domain5:2;
        ULONG Domain6:2;
        ULONG Domain7:2;
        ULONG Domain8:2;
        ULONG Domain9:2;
        ULONG Domain10:2;
        ULONG Domain11:2;
        ULONG Domain12:2;
        ULONG Domain13:2;
        ULONG Domain14:2;
        ULONG Domain15:2;
    };
    ULONG AsUlong;
} ARM_DOMAIN_REGISTER;

typedef union _ARM_CONTROL_REGISTER
{
    struct
    {
        ULONG MmuEnabled:1;
        ULONG AlignmentFaultsEnabled:1;
        ULONG DCacheEnabled:1;
        ULONG Sbo:4;
        ULONG BigEndianEnabled:1;
        ULONG System:1;
        ULONG Rom:1;
        ULONG Sbz:2;
        ULONG ICacheEnabled:1;
        ULONG HighVectors:1;
        ULONG RoundRobinReplacementEnabled:1;
        ULONG Armv4Compat:1;
        ULONG Ignored:6;
        ULONG UnalignedAccess:1;
        ULONG ExtendedPageTables:1;
        ULONG Sbz1:1;
        ULONG ExceptionBit:1;
        ULONG Sbz2:1;
        ULONG Nmif:1;
        ULONG TexRemap:1;
        ULONG ForceAp:1;
        ULONG Reserved:2;
    };
    ULONG AsUlong;
} ARM_CONTROL_REGISTER, *PARM_CONTROL_REGISTER;

C_ASSERT(sizeof(ARM_CONTROL_REGISTER) == sizeof(ULONG));

typedef union _ARM_ID_CODE_REGISTER
{
    struct
    {
        ULONG Revision:4;
        ULONG PartNumber:12;
        ULONG Architecture:4;
        ULONG Variant:4;
        ULONG Identifier:8;
    };
    ULONG AsUlong;
} ARM_ID_CODE_REGISTER, *PARM_ID_CODE_REGISTER;

typedef union _ARM_CACHE_REGISTER
{
    struct
    {
        ULONG ILength:2;
        ULONG IMultipler:1;
        ULONG IAssociativty:3;
        ULONG ISize:4;
        ULONG IReserved:2;
        ULONG DLength:2;
        ULONG DMultipler:1;
        ULONG DAssociativty:3;
        ULONG DSize:4;
        ULONG DReserved:2;  
        ULONG Separate:1;
        ULONG CType:4;
        ULONG Reserved:3;
    };
    ULONG AsUlong;
} ARM_CACHE_REGISTER, *PARM_CACHE_REGISTER;

typedef union _ARM_LOCKDOWN_REGISTER
{
    struct
    {
        ULONG Preserve:1;
        ULONG Ignored:25;
        ULONG Victim:3;
        ULONG Reserved:3;
    };
    ULONG AsUlong;
} ARM_LOCKDOWN_REGISTER, *PARM_LOCKDOWN_REGISTER;

//
// ARM Domains
//
typedef enum _ARM_DOMAINS
{
    Domain0,
    Domain1,
    Domain2,
    Domain3,
    Domain4,
    Domain5,
    Domain6,
    Domain7,
    Domain8,
    Domain9,
    Domain10,
    Domain11,
    Domain12,
    Domain13,
    Domain14,
    Domain15
} ARM_DOMAINS;

//
// Special Registers Structure (outside of CONTEXT)
//
typedef struct _KSPECIAL_REGISTERS
{
    ARM_CONTROL_REGISTER ControlRegister;
    ARM_LOCKDOWN_REGISTER LockdownRegister;
    ARM_CACHE_REGISTER CacheRegister;
    ARM_STATUS_REGISTER StatusRegister;
} KSPECIAL_REGISTERS, *PKSPECIAL_REGISTERS;

//
// Processor State
//
typedef struct _KPROCESSOR_STATE
{
    struct _CONTEXT ContextFrame;
    struct _KSPECIAL_REGISTERS SpecialRegisters;
} KPROCESSOR_STATE, *PKPROCESSOR_STATE;

//
// Processor Region Control Block
//
typedef struct _KPRCB
{
    USHORT MinorVersion;
    USHORT MajorVersion;
    struct _KTHREAD *CurrentThread;
    struct _KTHREAD *NextThread;
    struct _KTHREAD *IdleThread;
    UCHAR Number;
    UCHAR Reserved;
    USHORT BuildType;
    KAFFINITY SetMember;
    UCHAR CpuType;
    UCHAR CpuID;
    USHORT CpuStep;
    KPROCESSOR_STATE ProcessorState;
    ULONG KernelReserved[16];
    ULONG HalReserved[16];
    UCHAR PrcbPad0[92];
    KSPIN_LOCK_QUEUE LockQueue[LockQueueMaximumLock];
    struct _KTHREAD *NpxThread;
    ULONG InterruptCount;
    ULONG KernelTime;
    ULONG UserTime;
    ULONG DpcTime;
    ULONG DebugDpcTime;
    ULONG InterruptTime;
    ULONG AdjustDpcThreshold;
    ULONG PageColor;
    UCHAR SkipTick;
    UCHAR DebuggerSavedIRQL;
    UCHAR NodeColor;
    UCHAR Spare1;
    ULONG NodeShiftedColor;
    struct _KNODE *ParentNode;
    ULONG MultiThreadProcessorSet;
    struct _KPRCB *MultiThreadSetMaster;
    ULONG SecondaryColorMask;
    LONG Sleeping;
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadNotPossible;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
    ULONG KeAlignmentFixupCount;
    ULONG SpareCounter0;
    ULONG KeDcacheFlushCount;
    ULONG KeExceptionDispatchCount;
    ULONG KeFirstLevelTbFills;
    ULONG KeFloatingEmulationCount;
    ULONG KeIcacheFlushCount;
    ULONG KeSecondLevelTbFills;
    ULONG KeSystemCalls;
    volatile ULONG IoReadOperationCount;
    volatile ULONG IoWriteOperationCount;
    volatile ULONG IoOtherOperationCount;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG SpareCounter1[8];
    PP_LOOKASIDE_LIST PPLookasideList[16];
    PP_LOOKASIDE_LIST PPNPagedLookasideList[32];
    PP_LOOKASIDE_LIST PPPagedLookasideList[32];
    volatile ULONG PacketBarrier;
    volatile ULONG ReverseStall;
    PVOID IpiFrame;
    UCHAR PrcbPad2[52];
    volatile PVOID CurrentPacket[3];
    volatile ULONG TargetSet;
    volatile PKIPI_WORKER WorkerRoutine;
    volatile ULONG IpiFrozen;
    UCHAR PrcbPad3[40];
    volatile ULONG RequestSummary;
    volatile struct _KPRCB *SignalDone;
    UCHAR PrcbPad4[56];
    struct _KDPC_DATA DpcData[2];
    PVOID DpcStack;
    ULONG MaximumDpcQueueDepth;
    ULONG DpcRequestRate;
    ULONG MinimumDpcRate;
    volatile UCHAR DpcInterruptRequested;
    volatile UCHAR DpcThreadRequested;
    volatile UCHAR DpcRoutineActive;
    volatile UCHAR DpcThreadActive;
    ULONG PrcbLock;
    ULONG DpcLastCount;
    volatile ULONG TimerHand;
    volatile ULONG TimerRequest;
    PVOID DpcThread;
    KEVENT DpcEvent;
    UCHAR ThreadDpcEnable;
    volatile BOOLEAN QuantumEnd;
    UCHAR PrcbPad50;
    volatile UCHAR IdleSchedule;
    LONG DpcSetEventRequest;
    UCHAR PrcbPad5[18];
    LONG TickOffset;
    KDPC CallDpc;
    ULONG PrcbPad7[8];
    LIST_ENTRY WaitListHead;
    ULONG ReadySummary;
    ULONG QueueIndex;
    LIST_ENTRY DispatcherReadyListHead[32];
    SINGLE_LIST_ENTRY DeferredReadyListHead;
    ULONG PrcbPad72[11];
    PVOID ChainedInterruptList;
    LONG LookasideIrpFloat;
    volatile LONG MmPageFaultCount;
    volatile LONG MmCopyOnWriteCount;
    volatile LONG MmTransitionCount;
    volatile LONG MmCacheTransitionCount;
    volatile LONG MmDemandZeroCount;
    volatile LONG MmPageReadCount;
    volatile LONG MmPageReadIoCount;
    volatile LONG MmCacheReadCount;
    volatile LONG MmCacheIoCount;
    volatile LONG MmDirtyPagesWriteCount;
    volatile LONG MmDirtyWriteIoCount;
    volatile LONG MmMappedPagesWriteCount;
    volatile LONG MmMappedWriteIoCount;
    ULONG SpareFields0[1];
    CHAR VendorString[13];
    UCHAR InitialApicId;
    UCHAR LogicalProcessorsPerPhysicalProcessor;
    ULONG MHz;
    ULONG FeatureBits;
    LARGE_INTEGER UpdateSignature;
    volatile LARGE_INTEGER IsrTime;
    LARGE_INTEGER SpareField1;
    //FX_SAVE_AREA NpxSaveArea;
    PROCESSOR_POWER_STATE PowerState;
} KPRCB, *PKPRCB;

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
            struct _EXCEPTION_REGISTRATION_RECORD *Used_ExceptionList; // Unused
            PVOID Used_StackBase; // Unused
            PVOID PerfGlobalGroupMask;
            PVOID TssCopy; // Unused
            ULONG ContextSwitches;
            KAFFINITY SetMemberCopy; // Unused
            PVOID Used_Self;
        };
    };
    struct _KPCR *Self;
    struct _KPRCB *Prcb;
    KIRQL Irql;
    ULONG IRR; // Unused
    ULONG IrrActive; // Unused
    ULONG IDR; // Unused
    PVOID KdVersionBlock;
    PVOID IDT; // Unused
    PVOID GDT; // Unused
    PVOID TSS; // Unused
    USHORT MajorVersion;
    USHORT MinorVersion;
    KAFFINITY SetMember;
    ULONG StallScaleFactor;
    UCHAR SpareUnused;
    UCHAR Number;
    UCHAR Spare0;
    UCHAR SecondLevelCacheAssociativity;
    ULONG VdmAlert;
    ULONG KernelReserved[14];
    ULONG SecondLevelCacheSize;
    ULONG HalReserved[16];  
    // arm part
    UCHAR IrqlMask[32];
    ULONG IrqlTable[32];
    PKINTERRUPT_ROUTINE InterruptRoutine[32];
    ULONG ReservedVectors;
    ULONG FirstLevelDcacheSize;
    ULONG FirstLevelDcacheFillSize;
    ULONG FirstLevelIcacheSize;
    ULONG FirstLevelIcacheFillSize;
    ULONG SecondLevelDcacheSize;
    ULONG SecondLevelDcacheFillSize;
    ULONG SecondLevelIcacheSize;
    ULONG SecondLevelIcacheFillSize;
    ULONG DcacheFillSize;
    ULONG DcacheAlignment;
    ULONG IcacheAlignment;
    ULONG IcacheFillSize;
    ULONG ProcessorId;
    PVOID InterruptStack;
    PVOID PanicStack;
    PVOID InitialStack;
    KPRCB PrcbData;
} KIPCR, *PKIPCR;

//
// Macro to get current KPRCB
//
FORCEINLINE
struct _KPRCB *
KeGetCurrentPrcb(VOID)
{
    return PCR->Prcb;
}

//
// Just read it from the PCR
//
#define KeGetCurrentProcessorNumber()  (int)PCR->Number
#define KeGetCurrentIrql()             PCR->Irql
#define _KeGetCurrentThread()          KeGetCurrentPrcb()->CurrentThread
#define _KeGetPreviousMode()           KeGetCurrentPrcb()->CurrentThread->PreviousMode
#define _KeIsExecutingDpc()            (KeGetCurrentPrcb()->DpcRoutineActive != 0)
#define KeGetCurrentThread()           _KeGetCurrentThread()
#define KeGetPreviousMode()            _KeGetPreviousMode()
#define KeGetDcacheFillSize()          PCR->DcacheFillSize

#endif
#endif
