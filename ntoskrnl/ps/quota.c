/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/ps/quota.c
 * PURPOSE:         Process Pool Quotas
 *
 * PROGRAMMERS:     Alex Ionescu (alex@relsoft.net)
 *                  Mike Nordell
 */

/* INCLUDES **************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

EPROCESS_QUOTA_BLOCK PspDefaultQuotaBlock;


/* Define this macro to enable quota code testing. Once quota code is */
/* stable and verified, remove this macro and checks for it. */
/*#define PS_QUOTA_ENABLE_QUOTA_CODE*/


/* PRIVATE FUNCTIONS *******************************************************/

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE

/*
 * Private helper to charge the specified process quota.
 * ReturnsSTATUS_QUOTA_EXCEEDED on quota limit check failure.
 * Updates QuotaPeak as needed for specified PoolIndex.
 * TODO: Research and possibly add (the undocumented) enum type PS_QUOTA_TYPE
 *       to replace UCHAR for 'PoolIndex'.
 * Notes: Conceptually translation unit local/private.
 */
NTSTATUS
NTAPI
PspChargeProcessQuotaSpecifiedPool(IN PEPROCESS Process,
                                   IN UCHAR     PoolIndex,
                                   IN SIZE_T    Amount)
{
    ASSERT(Process);
    ASSERT(Process != PsInitialSystemProcess);
    ASSERT(PoolIndex <= 2);
    ASSERT(Process->QuotaBlock);

    /* Note: Race warning. TODO: Needs to add/use lock for this */
    if (Process->QuotaUsage[PoolIndex] + Amount >
        Process->QuotaBlock->QuotaEntry[PoolIndex].Limit)
    {
        return STATUS_QUOTA_EXCEEDED; /* caller raises the exception */
    }

    InterlockedExchangeAdd((LONG*)&Process->QuotaUsage[PoolIndex], Amount);

    /* Note: Race warning. TODO: Needs to add/use lock for this */
    if (Process->QuotaPeak[PoolIndex] < Process->QuotaUsage[PoolIndex])
    {
        Process->QuotaPeak[PoolIndex] = Process->QuotaUsage[PoolIndex];
    }

    return STATUS_SUCCESS;
}


/*
 * Private helper to remove quota charge from the specified process quota.
 * TODO: Research and possibly add (the undocumented) enum type PS_QUOTA_TYPE
 *       to replace UCHAR for 'PoolIndex'.
 * Notes: Conceptually translation unit local/private.
 */
VOID
NTAPI
PspReturnProcessQuotaSpecifiedPool(IN PEPROCESS Process,
                                   IN UCHAR     PoolIndex,
                                   IN SIZE_T    Amount)
{
    ASSERT(Process);
    ASSERT(Process != PsInitialSystemProcess);
    ASSERT(PoolIndex <= 2);
    ASSERT(!(Amount & 0x80000000)); /* we need to be able to negate it */
    if (Process->QuotaUsage[PoolIndex] < Amount)
    {
        DPRINT1("WARNING: Process->QuotaUsage sanity check failed.\n");
    }
    else
    {
        InterlockedExchangeAdd((LONG*)&Process->QuotaUsage[PoolIndex],
                               -(LONG)Amount);
    }
}

#endif /* PS_QUOTA_ENABLE_QUOTA_CODE */


/* FUNCTIONS ***************************************************************/

VOID
NTAPI
INIT_FUNCTION
PsInitializeQuotaSystem(VOID)
{
    RtlZeroMemory(&PspDefaultQuotaBlock, sizeof(PspDefaultQuotaBlock));
    PspDefaultQuotaBlock.QuotaEntry[PagedPool].Limit = (SIZE_T)-1;
    PspDefaultQuotaBlock.QuotaEntry[NonPagedPool].Limit = (SIZE_T)-1;
    PspDefaultQuotaBlock.QuotaEntry[2].Limit = (SIZE_T)-1; /* Page file */
    PsGetCurrentProcess()->QuotaBlock = &PspDefaultQuotaBlock;
}

VOID
NTAPI
PspInheritQuota(PEPROCESS Process, PEPROCESS ParentProcess)
{
    if (ParentProcess != NULL)
    {
        PEPROCESS_QUOTA_BLOCK QuotaBlock = ParentProcess->QuotaBlock;

        ASSERT(QuotaBlock != NULL);

        (void)InterlockedIncrementUL(&QuotaBlock->ReferenceCount);

        Process->QuotaBlock = QuotaBlock;
    }
    else
        Process->QuotaBlock = &PspDefaultQuotaBlock;
}

VOID
NTAPI
PspDestroyQuotaBlock(PEPROCESS Process)
{
    PEPROCESS_QUOTA_BLOCK QuotaBlock = Process->QuotaBlock;

    if (QuotaBlock != &PspDefaultQuotaBlock &&
        InterlockedDecrementUL(&QuotaBlock->ReferenceCount) == 0)
    {
        ExFreePool(QuotaBlock);
    }
}

NTSTATUS
NTAPI
PsChargeProcessPageFileQuota(IN PEPROCESS Process,
                             IN SIZE_T Amount)
{
    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return STATUS_SUCCESS;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    return PspChargeProcessQuotaSpecifiedPool(Process, 2, Amount);
#else
    /* Otherwise, not implemented */
    UNIMPLEMENTED;
    return STATUS_SUCCESS;
#endif
}

/*
 * @implemented
 */
VOID
NTAPI
PsChargePoolQuota(IN PEPROCESS Process,
                  IN POOL_TYPE PoolType,
                  IN SIZE_T Amount)
{
    NTSTATUS Status;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    /* MS-documented IRQL requirement. Not yet enabled as it */
    /* needs verification that it does not break Odyssey, */
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
#endif

    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return;

    /* Charge the usage */
    Status = PsChargeProcessPoolQuota(Process, PoolType, Amount);
    if (!NT_SUCCESS(Status)) ExRaiseStatus(Status);
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
PsChargeProcessNonPagedPoolQuota(IN PEPROCESS Process,
                                 IN SIZE_T Amount)
{
    /* Call the general function */
    return PsChargeProcessPoolQuota(Process, NonPagedPool, Amount);
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
PsChargeProcessPagedPoolQuota(IN PEPROCESS Process,
                              IN SIZE_T Amount)
{
    /* Call the general function */
    return PsChargeProcessPoolQuota(Process, PagedPool, Amount);
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
PsChargeProcessPoolQuota(IN PEPROCESS Process,
                         IN POOL_TYPE PoolType,
                         IN SIZE_T Amount)
{
    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return STATUS_SUCCESS;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    return PspChargeProcessQuotaSpecifiedPool(Process,
                                              (PoolType & PAGED_POOL_MASK),
                                              Amount);
#else
    UNIMPLEMENTED;
    return STATUS_SUCCESS;
#endif
}

/*
 * @unimplemented
 */
VOID
NTAPI
PsReturnPoolQuota(IN PEPROCESS Process,
                  IN POOL_TYPE PoolType,
                  IN SIZE_T Amount)
{
#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    /* MS-documented IRQL requirement. Not yet enabled as it */
    /* needs verification that it does not break Odyssey, */
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
#endif

    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    PspReturnProcessQuotaSpecifiedPool(Process,
                                       (PoolType & PAGED_POOL_MASK),
                                       Amount);
#else
    UNIMPLEMENTED;
#endif
}

/*
 * @unimplemented
 */
VOID
NTAPI
PsReturnProcessNonPagedPoolQuota(IN PEPROCESS Process,
                                 IN SIZE_T Amount)
{
    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    PsReturnPoolQuota(Process, NonPagedPool, Amount);
#else
    UNIMPLEMENTED;
#endif
}

/*
 * @unimplemented
 */
VOID
NTAPI
PsReturnProcessPagedPoolQuota(IN PEPROCESS Process,
                              IN SIZE_T Amount)
{
    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    PsReturnPoolQuota(Process, PagedPool, Amount);
#else
    UNIMPLEMENTED;
#endif
}

NTSTATUS
NTAPI
PsReturnProcessPageFileQuota(IN PEPROCESS Process,
                             IN SIZE_T Amount)
{
    /* Don't do anything for the system process */
    if (Process == PsInitialSystemProcess) return STATUS_SUCCESS;

#ifdef PS_QUOTA_ENABLE_QUOTA_CODE
    PspReturnProcessQuotaSpecifiedPool(Process, 2, Amount);
#else
    /* Otherwise, not implemented */
    UNIMPLEMENTED;
#endif
    return STATUS_SUCCESS;
}

/* EOF */
