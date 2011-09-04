/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey Keyboard class driver
 * FILE:            drivers/input/kbdclass/misc.c
 * PURPOSE:         Misceallenous operations
 *
 * PROGRAMMERS:     Herv� Poussineau (hpoussin@odyssey.org)
 */

#include "kbdclass.h"

static IO_COMPLETION_ROUTINE ForwardIrpAndWaitCompletion;

static NTSTATUS NTAPI
ForwardIrpAndWaitCompletion(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context)
{
	if (Irp->PendingReturned)
		KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
ForwardIrpAndWait(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	PDEVICE_OBJECT LowerDevice;
	KEVENT Event;
	NTSTATUS Status;

	ASSERT(!((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->IsClassDO);
	LowerDevice = ((PPORT_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerDevice;

	KeInitializeEvent(&Event, NotificationEvent, FALSE);
	IoCopyCurrentIrpStackLocationToNext(Irp);

	TRACE_(CLASS_NAME, "Calling lower device %p\n", LowerDevice);
	IoSetCompletionRoutine(Irp, ForwardIrpAndWaitCompletion, &Event, TRUE, TRUE, TRUE);

	Status = IoCallDriver(LowerDevice, Irp);
	if (Status == STATUS_PENDING)
	{
		Status = KeWaitForSingleObject(&Event, Suspended, KernelMode, FALSE, NULL);
		if (NT_SUCCESS(Status))
			Status = Irp->IoStatus.Status;
	}

	return Status;
}

NTSTATUS NTAPI
ForwardIrpAndForget(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp)
{
	PDEVICE_OBJECT LowerDevice;

	ASSERT(!((PCOMMON_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->IsClassDO);
	LowerDevice = ((PPORT_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->LowerDevice;

	IoSkipCurrentIrpStackLocation(Irp);
	return IoCallDriver(LowerDevice, Irp);
}

NTSTATUS
DuplicateUnicodeString(
	IN ULONG Flags,
	IN PCUNICODE_STRING SourceString,
	OUT PUNICODE_STRING DestinationString)
{
	if (SourceString == NULL || DestinationString == NULL
	 || SourceString->Length > SourceString->MaximumLength
	 || (SourceString->Length == 0 && SourceString->MaximumLength > 0 && SourceString->Buffer == NULL)
	 || Flags == RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING || Flags >= 4)
	{
		return STATUS_INVALID_PARAMETER;
	}


	if ((SourceString->Length == 0)
	 && (Flags != (RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
	               RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING)))
	{
		DestinationString->Length = 0;
		DestinationString->MaximumLength = 0;
		DestinationString->Buffer = NULL;
	}
	else
	{
		USHORT DestMaxLength = SourceString->Length;

		if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)
			DestMaxLength += sizeof(UNICODE_NULL);

		DestinationString->Buffer = ExAllocatePoolWithTag(PagedPool, DestMaxLength, CLASS_TAG);
		if (DestinationString->Buffer == NULL)
			return STATUS_NO_MEMORY;

		RtlCopyMemory(DestinationString->Buffer, SourceString->Buffer, SourceString->Length);
		DestinationString->Length = SourceString->Length;
		DestinationString->MaximumLength = DestMaxLength;

		if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)
			DestinationString->Buffer[DestinationString->Length / sizeof(WCHAR)] = 0;
	}

	return STATUS_SUCCESS;
}
