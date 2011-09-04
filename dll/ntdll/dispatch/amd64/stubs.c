/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey NT Library
 * FILE:            dll/ntdll/dispatch/amd64/stubs.c
 * PURPOSE:         AMD64 stubs
 * PROGRAMMERS:      Stefan Ginsberg (stefan.ginsberg@odyssey.org)
 */

/* INCLUDES *****************************************************************/

#include <ntdll.h>
#define NDEBUG
#include <debug.h>

/* PUBLIC FUNCTIONS **********************************************************/

/*
 * @unimplemented
 */
VOID
NTAPI
LdrInitializeThunk(ULONG Unknown1, // FIXME: Parameters!
                   ULONG Unknown2,
                   ULONG Unknown3,
                   ULONG Unknown4)
{
    UNIMPLEMENTED;
    return;
}

/*
 * @unimplemented
 */
VOID
NTAPI
KiUserApcDispatcher(IN PVOID NormalRoutine,
                    IN PVOID NormalContext,
                    IN PVOID SystemArgument1,
                    IN PVOID SystemArgument2)
{
    UNIMPLEMENTED;
    return;
}

VOID
NTAPI
KiRaiseUserExceptionDispatcher()
{
    UNIMPLEMENTED;
    return;
}

VOID
NTAPI
KiUserCallbackDispatcher()
{
    UNIMPLEMENTED;
    return;
}

VOID
NTAPI
KiUserExceptionDispatcher()
{
    UNIMPLEMENTED;
    return;
}
