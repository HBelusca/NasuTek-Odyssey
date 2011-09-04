/* $Id: dllmain.c 37763 2008-11-30 11:42:05Z sginsberg $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/secur32/lsa.c
 * PURPOSE:         Client-side LSA functions
 * UPDATE HISTORY:
 *                  Created 05/08/00
 */

/* INCLUDES ******************************************************************/
#include <precomp.h>

/* GLOBALS *******************************************************************/

HANDLE Secur32Heap;

/* FUNCTIONS *****************************************************************/

BOOL WINAPI DllMain(HINSTANCE hInstance, ULONG Reason, PVOID Reserved)
{
   switch (Reason)
     {
      case DLL_PROCESS_ATTACH:
	Secur32Heap = RtlCreateHeap(0, NULL, 0, 4096, NULL, NULL);
	if (Secur32Heap == 0)
	  {
	     return(FALSE);
	  }
	break;

      case DLL_PROCESS_DETACH:
	if (!RtlDestroyHeap(Secur32Heap))
	  {
	     return(FALSE);
	  }
	break;
     }
   return(TRUE);
}
