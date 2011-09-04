/*
 * PROJECT:         Odyssey HAL
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            hal/halx86/generic/bus/isabus.c
 * PURPOSE:
 * PROGRAMMERS:     Stefan Ginsberg (stefan.ginsberg@odyssey.org)
 */

/* INCLUDES *******************************************************************/

#include <hal.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

/* PRIVATE FUNCTIONS **********************************************************/

BOOLEAN
NTAPI
HalpTranslateIsaBusAddress(IN PBUS_HANDLER BusHandler,
                           IN PBUS_HANDLER RootHandler, 
                           IN PHYSICAL_ADDRESS BusAddress,
                           IN OUT PULONG AddressSpace,
                           OUT PPHYSICAL_ADDRESS TranslatedAddress)
{
    BOOLEAN Status;
    
    /* Use system translation */
    Status = HalpTranslateSystemBusAddress(BusHandler,
                                           RootHandler,
                                           BusAddress,
                                           AddressSpace,
                                           TranslatedAddress);
    
    /* If it didn't work and it was memory address space... */   
    if (!(Status) && (*AddressSpace == 0))
    {
        /* Try EISA translation instead */
        Status = HalTranslateBusAddress(Eisa,
                                        BusHandler->BusNumber,
                                        BusAddress,
                                        AddressSpace,
                                        TranslatedAddress);
    }
    
    /* Return the result */
    return Status;
}

/* EOF */
