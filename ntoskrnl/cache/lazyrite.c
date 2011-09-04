/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey Kernel
 * FILE:            ntoskrnl/cache/lazyrite.c
 * PURPOSE:         Logging and configuration routines
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#include "newcc.h"
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

KEVENT CcpLazyWriteEvent;

/* FUNCTIONS ******************************************************************/

VOID NTAPI
CcpLazyWriteThread(PVOID Unused)
{
	/* Not implemented */
}

NTSTATUS
NTAPI
CcWaitForCurrentLazyWriterActivity(VOID)
{
    //KeWaitForSingleObject(&CcpLazyWriteEvent, Executive, KernelMode, FALSE, NULL);
    return STATUS_SUCCESS;
}

/* EOF */
