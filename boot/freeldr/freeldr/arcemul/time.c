/*
 * PROJECT:         Odyssey Boot Loader (FreeLDR)
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            boot/freeldr/freeldr/arcemul/time.c
 * PURPOSE:         Routines for Time measurement
 * PROGRAMMERS:     Hervé Poussineau  <hpoussin@odyssey.org>
 */

/* INCLUDES *******************************************************************/

#include <freeldr.h>

/* FUNCTIONS ******************************************************************/

TIMEINFO*
ArcGetTime(VOID)
{
    return MachVtbl.GetTime();
}

ULONG
ArcGetRelativeTime(VOID)
{
    TIMEINFO* TimeInfo;
    ULONG ret;

    TimeInfo = ArcGetTime();
    ret = ((TimeInfo->Hour * 24) + TimeInfo->Minute) * 60 + TimeInfo->Second;
    return ret;
}
