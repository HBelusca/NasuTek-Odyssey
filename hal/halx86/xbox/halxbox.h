/* $Id: halxbox.h 31195 2007-12-13 15:35:27Z hpoussin $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Xbox HAL
 * FILE:            hal/halx86/xbox/halxbox.h
 * PURPOSE:         Xbox specific routines
 * PROGRAMMER:      Ge van Geldorp (gvg@odyssey.com)
 * UPDATE HISTORY:
 *                  Created 2004/12/02
 */

#ifndef HALXBOX_H_INCLUDED
#define HALXBOX_H_INCLUDED

#include <hal.h>
#include <ntdddisk.h>
#include <rosldr.h>

VOID HalpXboxInitPciBus(PBUS_HANDLER BusHandler);
VOID HalpXboxInitPartIo(VOID);

#endif /* HALXBOX_H_INCLUDED */

/* EOF */
