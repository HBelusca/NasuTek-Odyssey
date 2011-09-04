/*
* PROJECT:         Odyssey Kernel
* LICENSE:         GPL - See COPYING in the top level directory
* FILE:            ntoskrnl/ke/i386/patpge.c
* PURPOSE:         Support for PAT and PGE (Large Pages)
* PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
*/

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

ULONG_PTR
NTAPI
INIT_FUNCTION
Ki386EnableGlobalPage(IN volatile ULONG_PTR Context)
{
    volatile PLONG Count = (PLONG)Context;
    ULONG Cr4, Cr3;

    /* Disable interrupts */
    _disable();

    /* Decrease CPU Count and loop until it's reached 0 */
    do {InterlockedDecrement(Count);} while (!*Count);

    /* Now check if this is the Boot CPU */
    if (!KeGetPcr()->Number)
    {
        /* It is.FIXME: Patch KeFlushCurrentTb */
    }

    /* Now get CR4 and make sure PGE is masked out */
    Cr4 = __readcr4();
    __writecr4(Cr4 & ~CR4_PGE);

    /* Flush the TLB */
    Cr3 = __readcr3();
    __writecr3(Cr3);

    /* Now enable PGE */
    DPRINT1("Global page support detected but not yet taken advantage of\n");
    //__writecr4(Cr4 | CR4_PGE);

    /* Restore interrupts */
    _enable();
    return 0;
}

VOID
NTAPI
INIT_FUNCTION
KiInitializePAT(VOID)
{
    /* FIXME: Support this */
    DPRINT1("PAT support detected but not yet taken advantage of\n");
}
