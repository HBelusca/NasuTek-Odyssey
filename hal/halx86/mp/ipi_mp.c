/* $Id: ipi_mp.c 45020 2010-01-09 22:43:16Z ekohl $
 *
 * COPYRIGHT:             See COPYING in the top level directory
 * PROJECT:               Odyssey kernel
 * FILE:                  hal/halx86/mp/ipi_mp.c
 * PURPOSE:               IPI functions for MP
 * PROGRAMMER:            Eric Kohl
 */

/* INCLUDES *****************************************************************/

#include <hal.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

VOID NTAPI
HalRequestIpi(KAFFINITY TargetProcessors)
{
  /* FIXME: SMP HAL is...very broken */
  DPRINT("HalRequestIpi(TargetProcessors %d)\n", TargetProcessors);
  APICSendIPI(1 << TargetProcessors,
	      IPI_VECTOR|APIC_ICR0_LEVEL_DEASSERT|APIC_ICR0_DESTM);
}

/* EOF */
