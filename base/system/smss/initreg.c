/*
 * PROJECT:         Odyssey Session Manager
 * LICENSE:         GPL v2 or later - See COPYING in the top level directory
 * FILE:            base/system/smss/initreg.c
 * PURPOSE:         Hive loading.
 * PROGRAMMERS:     Odyssey Development Team
 */

/* INCLUDES ******************************************************************/
#include "smss.h"

#define NDEBUG
#include <debug.h>

NTSTATUS
SmInitializeRegistry(VOID)
{
  DPRINT("SM: %s: initializing registry\n", __FUNCTION__);

  /* Load remaining registry hives */
  return NtInitializeRegistry(CM_BOOT_FLAG_SMSS);
}

/* EOF */
