/* $Id: dllmain.c 53225 2011-08-14 11:31:23Z akhaldi $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/advapi32/misc/dllmain.c
 * PURPOSE:         Library main function
 * PROGRAMMER:      ???
 * UPDATE HISTORY:
 *                  Created ???
 */

#include <advapi32.h>
WINE_DEFAULT_DEBUG_CHANNEL(advapi);

extern BOOL RegInitialize(VOID);
extern BOOL RegCleanup(VOID);
extern VOID UnloadNtMarta(VOID);

BOOL WINAPI
DllMain(HINSTANCE hinstDll,
    DWORD dwReason,
    LPVOID reserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hinstDll);
            RegInitialize();
            break;
        case DLL_PROCESS_DETACH:
            RegCleanup();
            UnloadNtMarta();
            break;
    }

   return TRUE;
}

/* EOF */
