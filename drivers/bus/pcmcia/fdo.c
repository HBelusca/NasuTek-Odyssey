/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey Kernel
 * FILE:        drivers/bus/pcmcia/fdo.c
 * PURPOSE:     PCMCIA Bus Driver
 * PROGRAMMERS: Cameron Gutman (cameron.gutman@odyssey.org)
 */

#include <pcmcia.h>

//#define NDEBUG
#include <debug.h>

NTSTATUS
NTAPI
PcmciaFdoPlugPlay(PPCMCIA_FDO_EXTENSION FdoExt,
                  PIRP Irp)
{
  UNIMPLEMENTED

  IoCompleteRequest(Irp, IO_NO_INCREMENT);

  return STATUS_NOT_SUPPORTED;
}

