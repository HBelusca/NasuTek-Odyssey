/*
 * PROJECT:         Odyssey HAL
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            hal/halx86/up/spinlock.c
 * PURPOSE:         Spinlock and Queued Spinlock Support
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

/* This file is compiled twice. Once for UP and once for MP */

#include <hal.h>
#define NDEBUG
#include <debug.h>

#include <internal/spinlock.h>

#undef KeAcquireSpinLock
#undef KeReleaseSpinLock

/* GLOBALS *******************************************************************/

ULONG HalpSystemHardwareFlags;
KSPIN_LOCK HalpSystemHardwareLock;

/* FUNCTIONS *****************************************************************/

#ifdef _M_IX86

/*
 * @implemented
 */
KIRQL
FASTCALL
KeAcquireSpinLockRaiseToSynch(PKSPIN_LOCK SpinLock)
{
    KIRQL OldIrql;

    /* Raise to sync */
    KeRaiseIrql(SYNCH_LEVEL, &OldIrql);

    /* Acquire the lock and return */
    KxAcquireSpinLock(SpinLock);
    return OldIrql;
}

/*
 * @implemented
 */
KIRQL
FASTCALL
KfAcquireSpinLock(PKSPIN_LOCK SpinLock)
{
    KIRQL OldIrql;

    /* Raise to dispatch and acquire the lock */
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
    KxAcquireSpinLock(SpinLock);
    return OldIrql;
}

/*
 * @implemented
 */
VOID
FASTCALL
KfReleaseSpinLock(PKSPIN_LOCK SpinLock,
                  KIRQL OldIrql)
{
    /* Release the lock and lower IRQL back */
    KxReleaseSpinLock(SpinLock);
    KeLowerIrql(OldIrql);
}

/*
 * @implemented
 */
KIRQL
FASTCALL
KeAcquireQueuedSpinLock(IN KSPIN_LOCK_QUEUE_NUMBER LockNumber)
{
    KIRQL OldIrql;

    /* Raise to dispatch */
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    /* Acquire the lock */
    KxAcquireSpinLock(KeGetCurrentPrcb()->LockQueue[LockNumber].Lock); // HACK
    return OldIrql;
}

/*
 * @implemented
 */
KIRQL
FASTCALL
KeAcquireQueuedSpinLockRaiseToSynch(IN KSPIN_LOCK_QUEUE_NUMBER LockNumber)
{
    KIRQL OldIrql;

    /* Raise to synch */
    KeRaiseIrql(SYNCH_LEVEL, &OldIrql);

    /* Acquire the lock */
    KxAcquireSpinLock(KeGetCurrentPrcb()->LockQueue[LockNumber].Lock); // HACK
    return OldIrql;
}

/*
 * @implemented
 */
VOID
FASTCALL
KeAcquireInStackQueuedSpinLock(IN PKSPIN_LOCK SpinLock,
                               IN PKLOCK_QUEUE_HANDLE LockHandle)
{
    /* Set up the lock */
    LockHandle->LockQueue.Next = NULL;
    LockHandle->LockQueue.Lock = SpinLock;

    /* Raise to dispatch */
    KeRaiseIrql(DISPATCH_LEVEL, &LockHandle->OldIrql);

    /* Acquire the lock */
    KxAcquireSpinLock(LockHandle->LockQueue.Lock); // HACK
}

/*
 * @implemented
 */
VOID
FASTCALL
KeAcquireInStackQueuedSpinLockRaiseToSynch(IN PKSPIN_LOCK SpinLock,
                                           IN PKLOCK_QUEUE_HANDLE LockHandle)
{
    /* Set up the lock */
    LockHandle->LockQueue.Next = NULL;
    LockHandle->LockQueue.Lock = SpinLock;

    /* Raise to synch */
    KeRaiseIrql(SYNCH_LEVEL, &LockHandle->OldIrql);

    /* Acquire the lock */
    KxAcquireSpinLock(LockHandle->LockQueue.Lock); // HACK
}

/*
 * @implemented
 */
VOID
FASTCALL
KeReleaseQueuedSpinLock(IN KSPIN_LOCK_QUEUE_NUMBER LockNumber,
                        IN KIRQL OldIrql)
{
    /* Release the lock */
    KxReleaseSpinLock(KeGetCurrentPrcb()->LockQueue[LockNumber].Lock); // HACK

    /* Lower IRQL back */
    KeLowerIrql(OldIrql);
}

/*
 * @implemented
 */
VOID
FASTCALL
KeReleaseInStackQueuedSpinLock(IN PKLOCK_QUEUE_HANDLE LockHandle)
{
    /* Simply lower IRQL back */
    KxReleaseSpinLock(LockHandle->LockQueue.Lock); // HACK
    KeLowerIrql(LockHandle->OldIrql);
}

/*
 * @implemented
 */
BOOLEAN
FASTCALL
KeTryToAcquireQueuedSpinLockRaiseToSynch(IN KSPIN_LOCK_QUEUE_NUMBER LockNumber,
                                         IN PKIRQL OldIrql)
{
#ifdef CONFIG_SMP
    ASSERT(FALSE); // FIXME: Unused
    while (TRUE);
#endif

    /* Simply raise to synch */
    KeRaiseIrql(SYNCH_LEVEL, OldIrql);

    /* Add an explicit memory barrier to prevent the compiler from reordering
       memory accesses across the borders of spinlocks */
    KeMemoryBarrierWithoutFence();

    /* Always return true on UP Machines */
    return TRUE;
}

/*
 * @implemented
 */
LOGICAL
FASTCALL
KeTryToAcquireQueuedSpinLock(IN KSPIN_LOCK_QUEUE_NUMBER LockNumber,
                             OUT PKIRQL OldIrql)
{
#ifdef CONFIG_SMP
    ASSERT(FALSE); // FIXME: Unused
    while (TRUE);
#endif

    /* Simply raise to dispatch */
    KeRaiseIrql(DISPATCH_LEVEL, OldIrql);

    /* Add an explicit memory barrier to prevent the compiler from reordering
       memory accesses across the borders of spinlocks */
    KeMemoryBarrierWithoutFence();

    /* Always return true on UP Machines */
    return TRUE;
}

#endif

VOID
NTAPI
HalpAcquireSystemHardwareSpinLock(VOID)
{
    ULONG Flags;

    /* Get flags and disable interrupts */
    Flags = __readeflags();
    _disable();

    /* Acquire the lock */
    KxAcquireSpinLock(&HalpSystemHardwareLock);

    /* We have the lock, save the flags now */
    HalpSystemHardwareFlags = Flags;
}

VOID
NTAPI
HalpReleaseCmosSpinLock(VOID)
{
    ULONG Flags;

    /* Get the flags */
    Flags = HalpSystemHardwareFlags;

    /* Release the lock */
    KxReleaseSpinLock(&HalpSystemHardwareLock);

    /* Restore the flags */
    __writeeflags(Flags);
}

