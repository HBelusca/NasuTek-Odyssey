/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         PCI IDE bus driver extension
 * FILE:            drivers/storage/pciidex/miniport.c
 * PURPOSE:         Miniport functions
 * PROGRAMMERS:     Herv� Poussineau (hpoussin@odyssey.org)
 */

#define INITGUID
#include "pciidex.h"

#define NDEBUG
#include <debug.h>

static DRIVER_DISPATCH PciIdeXForwardOrIgnore;
static NTSTATUS NTAPI
PciIdeXForwardOrIgnore(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	if (((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->IsFDO)
		return ForwardIrpAndForget(DeviceObject, Irp);
	else
	{
		ULONG MajorFunction = IoGetCurrentIrpStackLocation(Irp)->MajorFunction;
		NTSTATUS Status;

		if (MajorFunction == IRP_MJ_CREATE ||
		    MajorFunction == IRP_MJ_CLEANUP ||
		    MajorFunction == IRP_MJ_CLOSE)
		{
			Status = STATUS_SUCCESS;
		}
		else
		{
			DPRINT1("PDO stub for major function 0x%lx\n", MajorFunction);
			Status = STATUS_NOT_SUPPORTED;
		}
		Irp->IoStatus.Information = 0;
		Irp->IoStatus.Status = Status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return Status;
	}
}

static DRIVER_DISPATCH PciIdeXPnpDispatch;
static NTSTATUS NTAPI
PciIdeXPnpDispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	if (((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->IsFDO)
		return PciIdeXFdoPnpDispatch(DeviceObject, Irp);
	else
		return PciIdeXPdoPnpDispatch(DeviceObject, Irp);
}

NTSTATUS NTAPI
PciIdeXInitialize(
	IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath,
	IN PCONTROLLER_PROPERTIES HwGetControllerProperties,
	IN ULONG ExtensionSize)
{
	ULONG i;
	PPCIIDEX_DRIVER_EXTENSION DriverExtension;
	NTSTATUS Status;

	DPRINT("PciIdeXInitialize(%p '%wZ' %p 0x%lx)\n",
		DriverObject, RegistryPath, HwGetControllerProperties, ExtensionSize);

	Status = IoAllocateDriverObjectExtension(
		DriverObject,
		DriverObject,
		sizeof(PCIIDEX_DRIVER_EXTENSION),
		(PVOID*)&DriverExtension);
	if (!NT_SUCCESS(Status))
		return Status;
	RtlZeroMemory(DriverExtension, sizeof(PCIIDEX_DRIVER_EXTENSION));
	DriverExtension->MiniControllerExtensionSize = ExtensionSize;
	DriverExtension->HwGetControllerProperties = HwGetControllerProperties;

	DriverObject->DriverExtension->AddDevice = PciIdeXAddDevice;

	for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = PciIdeXForwardOrIgnore;
	DriverObject->MajorFunction[IRP_MJ_PNP] = PciIdeXPnpDispatch;

	return STATUS_SUCCESS;
}

/* May be called at IRQL <= DISPATCH_LEVEL */
NTSTATUS NTAPI
PciIdeXGetBusData(
	IN PVOID DeviceExtension,
	IN PVOID Buffer,
	IN ULONG ConfigDataOffset,
	IN ULONG BufferLength)
{
	PFDO_DEVICE_EXTENSION FdoDeviceExtension;
	ULONG BytesRead;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	DPRINT("PciIdeXGetBusData(%p %p 0x%lx 0x%lx)\n",
		DeviceExtension, Buffer, ConfigDataOffset, BufferLength);

	FdoDeviceExtension = CONTAINING_RECORD(DeviceExtension, FDO_DEVICE_EXTENSION, MiniControllerExtension);
	if (FdoDeviceExtension->BusInterface)
	{
		BytesRead = (*FdoDeviceExtension->BusInterface->GetBusData)(
			FdoDeviceExtension->BusInterface->Context,
			PCI_WHICHSPACE_CONFIG,
			Buffer,
			ConfigDataOffset,
			BufferLength);
		if (BytesRead == BufferLength)
			Status = STATUS_SUCCESS;
	}

	return Status;
}

/* May be called at IRQL <= DISPATCH_LEVEL */
NTSTATUS NTAPI
PciIdeXSetBusData(
	IN PVOID DeviceExtension,
	IN PVOID Buffer,
	IN PVOID DataMask,
	IN ULONG ConfigDataOffset,
	IN ULONG BufferLength)
{
	PFDO_DEVICE_EXTENSION FdoDeviceExtension;
	PUCHAR CurrentBuffer = NULL;
	ULONG i, BytesWritten;
	NTSTATUS Status;

	DPRINT("PciIdeXSetBusData(%p %p %p 0x%lx 0x%lx)\n",
		DeviceExtension, Buffer, DataMask, ConfigDataOffset, BufferLength);

	CurrentBuffer = ExAllocatePool(NonPagedPool, BufferLength);
	if (!CurrentBuffer)
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
		return Status;
	}

	Status = PciIdeXGetBusData(DeviceExtension, Buffer, ConfigDataOffset, BufferLength);
	if (!NT_SUCCESS(Status))
		goto cleanup;

	for (i = 0; i < BufferLength; i++)
		CurrentBuffer[i] = (CurrentBuffer[i] & ~((PUCHAR)DataMask)[i]) | (((PUCHAR)DataMask)[i] & ((PUCHAR)Buffer)[i]);

	FdoDeviceExtension = CONTAINING_RECORD(DeviceExtension, FDO_DEVICE_EXTENSION, MiniControllerExtension);
	if (!FdoDeviceExtension->BusInterface)
	{
		Status = STATUS_UNSUCCESSFUL;
		goto cleanup;
	}

	BytesWritten = (*FdoDeviceExtension->BusInterface->SetBusData)(
		FdoDeviceExtension->BusInterface->Context,
		PCI_WHICHSPACE_CONFIG,
		CurrentBuffer,
		ConfigDataOffset,
		BufferLength);
	if (BytesWritten == BufferLength)
		Status = STATUS_SUCCESS;
	else
		Status = STATUS_UNSUCCESSFUL;

cleanup:
	ExFreePool(CurrentBuffer);
	return Status;
}
