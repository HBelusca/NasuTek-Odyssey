/*
 * PROJECT:     Odyssey Serial mouse driver
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        drivers/input/sermouse/createclose.c
 * PURPOSE:     IRP_MJ_CREATE and IRP_MJ_CLOSE operations
 * PROGRAMMERS: Copyright 2005-2006 Herv� Poussineau (hpoussin@odyssey.org)
 */

#include "sermouse.h"

NTSTATUS NTAPI
SermouseCreate(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	TRACE_(SERMOUSE, "IRP_MJ_CREATE\n");
	ASSERT(((PSERMOUSE_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->PnpState == dsStarted);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI
SermouseClose(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	TRACE_(SERMOUSE, "IRP_MJ_CLOSE\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI
SermouseCleanup(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	TRACE_(SERMOUSE, "IRP_MJ_CLEANUP\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}
