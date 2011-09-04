/*
 * PROJECT:         Odyssey HAL
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            hal/halx86/generic/profil.c
 * PURPOSE:         System Profiling
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <hal.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

/*
 * @unimplemented
 */
VOID
NTAPI
HalStopProfileInterrupt(IN KPROFILE_SOURCE ProfileSource)
{
    UNIMPLEMENTED;
    return;
}

/*
 * @unimplemented
 */
VOID
NTAPI
HalStartProfileInterrupt(IN KPROFILE_SOURCE ProfileSource)
{
    UNIMPLEMENTED;
    return;
}

/*
 * @unimplemented
 */
ULONG_PTR
NTAPI
HalSetProfileInterval(IN ULONG_PTR Interval)
{
    UNIMPLEMENTED;
    return Interval;
}

ULONG HalpDecrementerRoll = 0;

LARGE_INTEGER
KeQueryPerformanceCounter(PLARGE_INTEGER PerformanceFrequency)
{
    LARGE_INTEGER Result;
    /* for now */
    if(PerformanceFrequency) PerformanceFrequency->QuadPart = 100000000;
    Result.HighPart = HalpDecrementerRoll;
    Result.LowPart = __rdtsc();
    return Result;
}
