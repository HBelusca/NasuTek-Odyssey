/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            lib/rossym/data.c
 * PURPOSE:         Definition of external variables
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@odyssey.com)
 */

#include <windows.h>
#include <odyssey/rossym.h>
#include "rossympriv.h"

ROSSYM_CALLBACKS RosSymCallbacks;

VOID
RosSymInit(PROSSYM_CALLBACKS Callbacks)
{
  RosSymCallbacks = *Callbacks;
}

/* EOF */
