/* $Id: halinit_up.c 53529 2011-09-02 14:45:19Z tkreuzer $
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

#include <hal.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS ***************************************************************/

VOID
HalpInitPhase0(IN PLOADER_PARAMETER_BLOCK LoaderBlock)
{

}

VOID
HalpInitPhase1(VOID)
{
#ifndef _MINIHAL_
        /* Enable IRQ 0 */
        HalpEnableInterruptHandler(IDT_DEVICE,
                                   0,
                                   PRIMARY_VECTOR_BASE,
                                   CLOCK2_LEVEL,
                                   HalpClockInterrupt,
                                   Latched);

        /* Enable IRQ 8 */
        HalpEnableInterruptHandler(IDT_DEVICE,
                                   0,
                                   PRIMARY_VECTOR_BASE + 8,
                                   PROFILE_LEVEL,
                                   HalpProfileInterrupt,
                                   Latched);

        /* Initialize DMA. NT does this in Phase 0 */
        HalpInitDma();
#endif
}

/* EOF */
