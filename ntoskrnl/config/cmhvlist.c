/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/config/cmwraprs.c
 * PURPOSE:         Configuration Manager - Wrappers for Hive Operations
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include "ntoskrnl.h"
#define NDEBUG
#include "debug.h"

/* FUNCTIONS *****************************************************************/

NTSTATUS
NTAPI
CmpAddToHiveFileList(IN PCMHIVE Hive)
{
    return STATUS_SUCCESS;
}

/* EOF */