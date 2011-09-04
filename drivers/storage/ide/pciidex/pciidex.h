#include <ntifs.h>
#include <ntddk.h>
#include <ide.h>
#include <wdmguid.h>
#include <stdio.h>

typedef struct _PCIIDEX_DRIVER_EXTENSION
{
	PCONTROLLER_PROPERTIES HwGetControllerProperties;
	ULONG MiniControllerExtensionSize;
	PCIIDE_UDMA_MODES_SUPPORTED HwUdmaModesSupported;
} PCIIDEX_DRIVER_EXTENSION, *PPCIIDEX_DRIVER_EXTENSION;

typedef struct _COMMON_DEVICE_EXTENSION
{
	BOOLEAN IsFDO;
} COMMON_DEVICE_EXTENSION, *PCOMMON_DEVICE_EXTENSION;

typedef struct _FDO_DEVICE_EXTENSION
{
	COMMON_DEVICE_EXTENSION Common;

	PBUS_INTERFACE_STANDARD BusInterface;
	IDE_CONTROLLER_PROPERTIES Properties;
	PHYSICAL_ADDRESS BusMasterPortBase;
	PDEVICE_OBJECT LowerDevice;
	PDEVICE_OBJECT Pdo[MAX_IDE_CHANNEL];
	USHORT VendorId;
	USHORT DeviceId;
	PUCHAR MiniControllerExtension[0];
} FDO_DEVICE_EXTENSION, *PFDO_DEVICE_EXTENSION;

typedef struct _PDO_DEVICE_EXTENSION
{
	COMMON_DEVICE_EXTENSION Common;

	ULONG Channel;
	PDEVICE_OBJECT ControllerFdo;
} PDO_DEVICE_EXTENSION, *PPDO_DEVICE_EXTENSION;

/* fdo.c */

DRIVER_ADD_DEVICE PciIdeXAddDevice;
NTSTATUS NTAPI
PciIdeXAddDevice(
	IN PDRIVER_OBJECT DriverObject,
	IN PDEVICE_OBJECT Pdo);

NTSTATUS NTAPI
PciIdeXFdoPnpDispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp);

/* misc.c */

IO_COMPLETION_ROUTINE PciIdeXGenericCompletion;
NTSTATUS NTAPI
PciIdeXGenericCompletion(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp,
	IN PVOID Context);

NTSTATUS
ForwardIrpAndWait(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp);

NTSTATUS NTAPI
ForwardIrpAndForget(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp);

NTSTATUS
DuplicateUnicodeString(
	IN ULONG Flags,
	IN PCUNICODE_STRING SourceString,
	OUT PUNICODE_STRING DestinationString);

/* pdo.c */

NTSTATUS NTAPI
PciIdeXPdoPnpDispatch(
	IN PDEVICE_OBJECT DeviceObject,
	IN PIRP Irp);
