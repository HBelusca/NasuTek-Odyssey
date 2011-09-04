/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         PCI IDE bus driver extension
 * FILE:            drivers/storage/pciidex/pciidex.c
 * PURPOSE:         Main file
 * PROGRAMMERS:     Herv� Poussineau (hpoussin@odyssey.org)
 */

#include "pciidex.h"

#define NDEBUG
#include <debug.h>

NTSTATUS NTAPI
DriverEntry(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	return STATUS_SUCCESS;
}
