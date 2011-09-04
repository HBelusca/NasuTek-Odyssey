/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey Kernel
 * FILE:        drivers/bus/pcmcia/pdo.c
 * PURPOSE:     PCMCIA Bus Driver
 * PROGRAMMERS: Cameron Gutman (cameron.gutman@odyssey.org)
 */

#include <pcmcia.h>

//#define NDEBUG
#include <debug.h>

NTSTATUS
NTAPI
PcmciaPdoPlugPlay(PPCMCIA_PDO_EXTENSION PdoExt,
                  PIRP Irp)
{
  UNIMPLEMENTED

  IoCompleteRequest(Irp, IO_NO_INCREMENT);

  return STATUS_NOT_SUPPORTED;
}

NTSTATUS
NTAPI
PcmciaPdoSetPowerState(PPCMCIA_PDO_EXTENSION PdoExt)
{
  UNIMPLEMENTED

  return STATUS_SUCCESS;
}

