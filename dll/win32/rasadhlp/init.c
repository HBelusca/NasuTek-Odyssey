/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey Winsock 2 SPI
 * FILE:        lib/mswsock/lib/init.c
 * PURPOSE:     DLL Initialization
 */

/* INCLUDES ******************************************************************/
#include "precomp.h"

/* DATA **********************************************************************/

/* FUNCTIONS *****************************************************************/

BOOLEAN
WINAPI
DllMain(HINSTANCE Instance,
        DWORD Reason,
        LPVOID Reserved)
{
    /* Check if we're being attached */
    if (Reason == DLL_PROCESS_ATTACH)
    {
        /* Let's disable TLC calls as an optimization */
        DisableThreadLibraryCalls(Instance);
    }

    /* We're done */
    return TRUE;
}
