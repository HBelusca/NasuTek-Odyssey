/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           Odyssey Run-Time Library
 * PURPOSE:           AMD64 stubs
 * FILE:              lib/rtl/amd64/stubs.c
 * PROGRAMMERS:        Stefan Ginsberg (stefan.ginsberg@odyssey.org)
 */

/* INCLUDES *****************************************************************/

#include <rtl.h>
#define NDEBUG
#include <debug.h>

/* PUBLIC FUNCTIONS **********************************************************/

/*
 * @unimplemented
 */
VOID
NTAPI
RtlInitializeContext(IN HANDLE ProcessHandle,
                     OUT PCONTEXT ThreadContext,
                     IN PVOID ThreadStartParam  OPTIONAL,
                     IN PTHREAD_START_ROUTINE ThreadStartAddress,
                     IN PINITIAL_TEB InitialTeb)
{
    UNIMPLEMENTED;
    return;
}

/*
 * @unimplemented
 */
PVOID
NTAPI
RtlpGetExceptionAddress(VOID)
{
    UNIMPLEMENTED;
    return NULL;
}

/*
 * @unimplemented
 */
BOOLEAN
NTAPI
RtlDispatchException(IN PEXCEPTION_RECORD ExceptionRecord,
                     IN PCONTEXT Context)
{
    UNIMPLEMENTED;
    return FALSE;
}

NTSYSAPI
VOID
RtlRestoreContext(
    PCONTEXT ContextRecord,
    PEXCEPTION_RECORD ExceptionRecord)
{
    UNIMPLEMENTED;
}


