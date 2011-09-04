/* $Id: class2.h 49129 2010-10-12 20:17:55Z pschweitzer $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            services/storage/include/class2.h
 * PURPOSE:         SCSI class driver definitions
 * PROGRAMMER:      Eric Kohl
 */

#pragma once

#include "ntddscsi.h"
#include "srb.h"

#define MAXIMUM_RETRIES    15
#define RETRY_WAIT         2000000 /* 200 ms in units of 100 ns */

//
// Indicates that the device has write caching enabled.
//

#define DEV_WRITE_CACHE     0x00000001


//
// Build SCSI 1 or SCSI 2 CDBs
//

#define DEV_USE_SCSI1       0x00000002

//
// Indicates whether is is safe to send StartUnit commands
// to this device. It will only be off for some removeable devices.
//

#define DEV_SAFE_START_UNIT 0x00000004

//
// Indicates whether it is unsafe to send SCSIOP_MECHANISM_STATUS commands to
// this device.  Some devices don't like these 12 byte commands
//

#define DEV_NO_12BYTE_CDB 0x00000008


struct _CLASS_INIT_DATA;

typedef VOID
(NTAPI *PCLASS_ERROR)(IN PDEVICE_OBJECT DeviceObject,
		IN PSCSI_REQUEST_BLOCK Srb,
		IN OUT NTSTATUS *Status,
		IN OUT BOOLEAN *Retry);

typedef BOOLEAN
(NTAPI *PCLASS_DEVICE_CALLBACK)(IN PINQUIRYDATA);

typedef NTSTATUS
(NTAPI *PCLASS_READ_WRITE)(IN PDEVICE_OBJECT DeviceObject,
		     IN PIRP Irp);

typedef BOOLEAN
(NTAPI *PCLASS_FIND_DEVICES)(IN PDRIVER_OBJECT DriverObject,
		       IN PUNICODE_STRING RegistryPath,
		       IN struct _CLASS_INIT_DATA *InitializationData,
		       IN PDEVICE_OBJECT PortDeviceObject,
		       IN ULONG PortNumber);

typedef NTSTATUS
(NTAPI *PCLASS_DEVICE_CONTROL)(IN PDEVICE_OBJECT DeviceObject,
			 IN PIRP Irp);

typedef NTSTATUS
(NTAPI *PCLASS_SHUTDOWN_FLUSH)(IN PDEVICE_OBJECT DeviceObject,
			 IN PIRP Irp);

typedef NTSTATUS
(NTAPI *PCLASS_CREATE_CLOSE)(IN PDEVICE_OBJECT DeviceObject,
		       IN PIRP Irp);


typedef struct _CLASS_INIT_DATA
{
  ULONG InitializationDataSize;
  ULONG DeviceExtensionSize;
  DEVICE_TYPE DeviceType;
  ULONG DeviceCharacteristics;
  PCLASS_ERROR ClassError;
  PCLASS_READ_WRITE ClassReadWriteVerification;
  PCLASS_DEVICE_CALLBACK ClassFindDeviceCallBack;
  PCLASS_FIND_DEVICES ClassFindDevices;
  PCLASS_DEVICE_CONTROL ClassDeviceControl;
  PCLASS_SHUTDOWN_FLUSH ClassShutdownFlush;
  PCLASS_CREATE_CLOSE ClassCreateClose;
  PDRIVER_STARTIO ClassStartIo;
} CLASS_INIT_DATA, *PCLASS_INIT_DATA;


typedef struct _DEVICE_EXTENSION
{
  PDEVICE_OBJECT DeviceObject;
  PDEVICE_OBJECT PortDeviceObject;
  LARGE_INTEGER PartitionLength;
  LARGE_INTEGER StartingOffset;
  ULONG DMByteSkew;
  ULONG DMSkew;
  BOOLEAN DMActive;
  PCLASS_ERROR ClassError;
  PCLASS_READ_WRITE ClassReadWriteVerification;
  PCLASS_FIND_DEVICES ClassFindDevices;
  PCLASS_DEVICE_CONTROL ClassDeviceControl;
  PCLASS_SHUTDOWN_FLUSH ClassShutdownFlush;
  PCLASS_CREATE_CLOSE ClassCreateClose;
  PDRIVER_STARTIO ClassStartIo;
  PIO_SCSI_CAPABILITIES PortCapabilities;
  PDISK_GEOMETRY_EX DiskGeometry;
  PDEVICE_OBJECT PhysicalDevice;
  PSENSE_DATA SenseData;
  ULONG TimeOutValue;
  ULONG DeviceNumber;
  ULONG SrbFlags;
  ULONG ErrorCount;
  KSPIN_LOCK SplitRequestSpinLock;
  NPAGED_LOOKASIDE_LIST SrbLookasideListHead;
  LONG LockCount;
  UCHAR PortNumber;
  UCHAR PathId;
  UCHAR TargetId;
  UCHAR Lun;
  UCHAR SectorShift;
  UCHAR ReservedByte;
  USHORT DeviceFlags;
  PKEVENT MediaChangeEvent;
  HANDLE MediaChangeEventHandle;
  BOOLEAN MediaChangeNoMedia;
  ULONG MediaChangeCount;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


typedef struct _COMPLETION_CONTEXT
{
  PDEVICE_OBJECT DeviceObject;
  SCSI_REQUEST_BLOCK Srb;
} COMPLETION_CONTEXT, *PCOMPLETION_CONTEXT;


/* FUNCTIONS ****************************************************************/

IO_COMPLETION_ROUTINE ScsiClassAsynchronousCompletion;
NTSTATUS NTAPI
ScsiClassAsynchronousCompletion(IN PDEVICE_OBJECT DeviceObject,
				IN PIRP Irp,
				IN PVOID Context);

VOID NTAPI
ScsiClassBuildRequest(IN PDEVICE_OBJECT DeviceObject,
		      IN PIRP Irp);

NTSTATUS NTAPI
ScsiClassClaimDevice(IN PDEVICE_OBJECT PortDeviceObject,
		     IN PSCSI_INQUIRY_DATA LunInfo,
		     IN BOOLEAN Release,
		     OUT PDEVICE_OBJECT *NewPortDeviceObject OPTIONAL);

NTSTATUS NTAPI
ScsiClassCreateDeviceObject(IN PDRIVER_OBJECT DriverObject,
			    IN PCCHAR ObjectNameBuffer,
			    IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
			    IN OUT PDEVICE_OBJECT *DeviceObject,
			    IN PCLASS_INIT_DATA InitializationData);

NTSTATUS NTAPI
ScsiClassDeviceControl(IN PDEVICE_OBJECT DeviceObject,
		       IN PIRP Irp);

PVOID NTAPI
ScsiClassFindModePage(IN PCHAR ModeSenseBuffer,
		      IN ULONG Length,
		      IN UCHAR PageMode,
		      IN BOOLEAN Use6Byte);

ULONG NTAPI
ScsiClassFindUnclaimedDevices(IN PCLASS_INIT_DATA InitializationData,
			      OUT PSCSI_ADAPTER_BUS_INFO AdapterInformation);

NTSTATUS NTAPI
ScsiClassGetCapabilities(IN PDEVICE_OBJECT PortDeviceObject,
			 OUT PIO_SCSI_CAPABILITIES *PortCapabilities);

NTSTATUS NTAPI
ScsiClassGetInquiryData(IN PDEVICE_OBJECT PortDeviceObject,
			OUT PSCSI_ADAPTER_BUS_INFO *ConfigInfo);

ULONG NTAPI
ScsiClassInitialize(IN PVOID Argument1,
		    IN PVOID Argument2,
		    IN PCLASS_INIT_DATA InitializationData);

VOID NTAPI
ScsiClassInitializeSrbLookasideList(IN PDEVICE_EXTENSION DeviceExtension,
				    IN ULONG NumberElements);

NTSTATUS NTAPI
ScsiClassInternalIoControl(IN PDEVICE_OBJECT DeviceObject,
			   IN PIRP Irp);

BOOLEAN NTAPI
ScsiClassInterpretSenseInfo(IN PDEVICE_OBJECT DeviceObject,
			    IN PSCSI_REQUEST_BLOCK Srb,
			    IN UCHAR MajorFunctionCode,
			    IN ULONG IoDeviceCode,
			    IN ULONG RetryCount,
			    OUT NTSTATUS *Status);

NTSTATUS NTAPI
ScsiClassIoComplete(IN PDEVICE_OBJECT DeviceObject,
		    IN PIRP Irp,
		    IN PVOID Context);

NTSTATUS NTAPI
ScsiClassIoCompleteAssociated(IN PDEVICE_OBJECT DeviceObject,
			      IN PIRP Irp,
			      IN PVOID Context);

ULONG NTAPI
ScsiClassModeSense(IN PDEVICE_OBJECT DeviceObject,
		   IN PCHAR ModeSenseBuffer,
		   IN ULONG Length,
		   IN UCHAR PageMode);

ULONG NTAPI
ScsiClassQueryTimeOutRegistryValue(IN PUNICODE_STRING RegistryPath);

NTSTATUS NTAPI
ScsiClassReadDriveCapacity(IN PDEVICE_OBJECT DeviceObject);

VOID NTAPI
ScsiClassReleaseQueue(IN PDEVICE_OBJECT DeviceObject);

NTSTATUS NTAPI
ScsiClassSendSrbAsynchronous(PDEVICE_OBJECT DeviceObject,
			     PSCSI_REQUEST_BLOCK Srb,
			     PIRP Irp,
			     PVOID BufferAddress,
			     ULONG BufferLength,
			     BOOLEAN WriteToDevice);

NTSTATUS NTAPI
ScsiClassSendSrbSynchronous(PDEVICE_OBJECT DeviceObject,
			    PSCSI_REQUEST_BLOCK Srb,
			    PVOID BufferAddress,
			    ULONG BufferLength,
			    BOOLEAN WriteToDevice);

VOID NTAPI
ScsiClassSplitRequest(IN PDEVICE_OBJECT DeviceObject,
		      IN PIRP Irp,
		      IN ULONG MaximumBytes);

NTSTATUS
NTAPI
ScsiClassCheckVerifyComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

/* EOF */
