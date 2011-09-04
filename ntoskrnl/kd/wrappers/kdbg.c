/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/kd/wrappers/kdbg.c
 * PURPOSE:         KDBG Wrapper for Kd
 *
 * PROGRAMMERS:     Aleksey Bragin (aleksey@odyssey.org)
 */

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

VOID NTAPI
KdbInitialize(PKD_DISPATCH_TABLE DispatchTable, ULONG BootPhase);

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
KdpKdbgInit(PKD_DISPATCH_TABLE DispatchTable,
            ULONG BootPhase)
{
#ifdef KDBG
    /* Forward the call */
    KdbInitialize(DispatchTable, BootPhase);
#else
    /* When KDBG is disabled, it is not used/initialized at all */
#endif
}
