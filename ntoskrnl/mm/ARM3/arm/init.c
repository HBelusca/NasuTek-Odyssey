/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/mm/ARM3/init.c
 * PURPOSE:         ARM Memory Manager Initialization
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

#define MODULE_INVOLVED_IN_ARM3
#include "../../ARM3/miarm.h"

/* GLOBALS ********************************************************************/

ULONG MmMaximumNonPagedPoolPercent;
ULONG MmSizeOfNonPagedPoolInBytes;
ULONG MmMaximumNonPagedPoolInBytes;
PVOID MmNonPagedSystemStart;
PVOID MmNonPagedPoolStart;
PVOID MmNonPagedPoolExpansionStart;
PVOID MmPagedPoolEnd;
PVOID MiSessionSpaceEnd;
PVOID MiSessionImageEnd;
PVOID MiSessionImageStart;
PVOID MiSessionViewStart;
PVOID MiSessionPoolEnd;
PVOID MiSessionPoolStart;
PVOID MmSessionBase;
ULONG MmSessionSize;
ULONG MmSessionViewSize;
ULONG MmSessionPoolSize;
ULONG MmSessionImageSize;
PVOID MiSystemViewStart;
ULONG MmSystemViewSize;
PFN_NUMBER MmSystemPageDirectory[PD_COUNT];
PMMPDE MmSystemPagePtes;
ULONG MmNumberOfSystemPtes;
ULONG MxPfnAllocation;
RTL_BITMAP MiPfnBitMap;
PPHYSICAL_MEMORY_DESCRIPTOR MmPhysicalMemoryBlock;
PMEMORY_ALLOCATION_DESCRIPTOR MxFreeDescriptor;
MEMORY_ALLOCATION_DESCRIPTOR MxOldFreeDescriptor;
ULONG MmNumberOfPhysicalPages, MmHighestPhysicalPage;
ULONG MmBootImageSize;
ULONG MmUserProbeAddress;
PVOID MmHighestUserAddress;
PVOID MmSystemRangeStart;
PVOID MmSystemCacheStart;
PVOID MmSystemCacheEnd;
MMSUPPORT MmSystemCacheWs;
PVOID MmHyperSpaceEnd;

/* PRIVATE FUNCTIONS **********************************************************/

NTSTATUS
NTAPI
INIT_FUNCTION
MiInitMachineDependent(IN PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    //
    // Always return success for now
    //
    DPRINT1("NEVER TELL ME THE ODDS!\n");
    while (TRUE);
    return STATUS_SUCCESS;
}

/* EOF */
