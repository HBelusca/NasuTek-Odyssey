/* $Id: halinit_xbox.c 31207 2007-12-14 08:53:56Z hpoussin $
 *
 * COPYRIGHT:     See COPYING in the top level directory
 * PROJECT:       Odyssey kernel
 * FILE:          ntoskrnl/hal/x86/halinit.c
 * PURPOSE:       Initalize the x86 hal
 * PROGRAMMER:    David Welch (welch@cwcom.net)
 * UPDATE HISTORY:
 *              11/06/98: Created
 */

/* INCLUDES *****************************************************************/

#include "halxbox.h"

#define NDEBUG
#include <debug.h>

/* FUNCTIONS ***************************************************************/

VOID
HalpInitPhase0(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
    HalpXboxInitPartIo();
}

VOID
HalpInitPhase1(VOID)
{
}

/* EOF */
