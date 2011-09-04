/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/config/cmmapvw.c
 * PURPOSE:         Configuration Manager - Map-Viewed Hive Support
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include "ntoskrnl.h"
#define NDEBUG
#include "debug.h"

/* GLOBALS *******************************************************************/

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
CmpInitHiveViewList(IN PCMHIVE Hive)
{
    /* Initialize the list heads */
    InitializeListHead(&Hive->LRUViewListHead);
    InitializeListHead(&Hive->PinViewListHead);

    /* Reset data */
    Hive->MappedViews = 0;
    Hive->PinnedViews = 0;
    Hive->UseCount = 0;
}
