/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            lib/rossym/delete.c
 * PURPOSE:         Free rossym info
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@odyssey.com)
 */

#define NTOSAPI
#include <ntddk.h>
#include <odyssey/rossym.h>
#include "rossympriv.h"

#define NDEBUG
#include <debug.h>

VOID
RosSymDelete(PROSSYM_INFO RosSymInfo)
{
  RosSymFreeMem(RosSymInfo);
}

/* EOF */
