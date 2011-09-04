/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            lib/rossym/initkm.c
 * PURPOSE:         Initialize library for use in kernel mode
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@odyssey.com)
 */

#include <ntdef.h>
#undef DECLSPEC_IMPORT
#define DECLSPEC_IMPORT
#include <ntddk.h>
#include <odyssey/rossym.h>
#include "rossympriv.h"

#define NDEBUG
#include <debug.h>

#define TAG_ROSSYM 'MYSR'

static PVOID
RosSymAllocMemKM(ULONG_PTR Size)
{
  return ExAllocatePoolWithTag(NonPagedPool, Size, TAG_ROSSYM);
}

static VOID
RosSymFreeMemKM(PVOID Area)
{
  ExFreePool(Area);
}

VOID
RosSymInitKernelMode(VOID)
{
  static ROSSYM_CALLBACKS KmCallbacks =
    {
      RosSymAllocMemKM,
      RosSymFreeMemKM,
      RosSymIoReadFile,
      RosSymIoSeekFile
    };

  RosSymInit(&KmCallbacks);
}

/* EOF */
