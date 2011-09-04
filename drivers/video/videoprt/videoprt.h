/*
 * VideoPort driver
 *
 * Copyright (C) 2002, 2003, 2004 ReactOS Team; (C) 2011 NasuTek Enterprises
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef VIDEOPRT_H
#define VIDEOPRT_H

#include <stdio.h>

#include <ntifs.h>
#include <ndk/exfuncs.h>
#include <ndk/halfuncs.h>
#include <ndk/inbvfuncs.h>
#include <ndk/kefuncs.h>
#include <ndk/rtlfuncs.h>

#define __BROKEN__
#include <miniport.h>
#include <video.h>
#include <ntagp.h>
#include <dderror.h>
#include <windef.h>
#include <initguid.h>
#include <wdmguid.h>

#include <debug.h>

#define TAG_VIDEO_PORT  'PDIV'
#define TAG_VIDEO_PORT_BUFFER  '\0mpV'

typedef struct _VIDEO_PORT_ADDRESS_MAPPING
{
   LIST_ENTRY List;
   PVOID MappedAddress;
   ULONG NumberOfUchars;
   PHYSICAL_ADDRESS IoAddress;
   ULONG SystemIoBusNumber;
   UINT MappingCount;
} VIDEO_PORT_ADDRESS_MAPPING, *PVIDEO_PORT_ADDRESS_MAPPING;

struct _VIDEO_PORT_AGP_VIRTUAL_MAPPING;

typedef struct _VIDEO_PORT_AGP_MAPPING
{
   ULONG NumberOfPages;
   PVOID MapHandle;
   PHYSICAL_ADDRESS PhysicalAddress;
} VIDEO_PORT_AGP_MAPPING, *PVIDEO_PORT_AGP_MAPPING;

typedef struct _VIDEO_PORT_AGP_VIRTUAL_MAPPING
{
   PVIDEO_PORT_AGP_MAPPING AgpMapping;
   HANDLE ProcessHandle;
   PVOID MappedAddress;
} VIDEO_PORT_AGP_VIRTUAL_MAPPING, *PVIDEO_PORT_AGP_VIRTUAL_MAPPING;

typedef struct _VIDEO_PORT_DRIVER_EXTENSION
{
   VIDEO_HW_INITIALIZATION_DATA InitializationData;
   PVOID HwContext;
   UNICODE_STRING RegistryPath;
} VIDEO_PORT_DRIVER_EXTENSION, *PVIDEO_PORT_DRIVER_EXTENSION;

typedef struct _VIDEO_PORT_DEVICE_EXTENSTION
{
   ULONG DeviceNumber;
   PDRIVER_OBJECT DriverObject;
   PDEVICE_OBJECT PhysicalDeviceObject;
   PDEVICE_OBJECT FunctionalDeviceObject;
   PDEVICE_OBJECT NextDeviceObject;
   UNICODE_STRING RegistryPath;
   PKINTERRUPT InterruptObject;
   KSPIN_LOCK InterruptSpinLock;
   PCM_RESOURCE_LIST AllocatedResources;
   ULONG InterruptVector;
   ULONG InterruptLevel;
   BOOLEAN InterruptShared;
   INTERFACE_TYPE AdapterInterfaceType;
   ULONG SystemIoBusNumber;
   ULONG SystemIoSlotNumber;
   LIST_ENTRY AddressMappingListHead;
   KDPC DpcObject;
   VIDEO_PORT_DRIVER_EXTENSION *DriverExtension;
   ULONG DeviceOpened;
   AGP_BUS_INTERFACE_STANDARD AgpInterface;
   KMUTEX DeviceLock;
   LIST_ENTRY DmaAdapterList;
   CHAR MiniPortDeviceExtension[1];
} VIDEO_PORT_DEVICE_EXTENSION, *PVIDEO_PORT_DEVICE_EXTENSION;

#define VIDEO_PORT_GET_DEVICE_EXTENSION(MiniportExtension) \
   CONTAINING_RECORD( \
      HwDeviceExtension, \
      VIDEO_PORT_DEVICE_EXTENSION, \
      MiniPortDeviceExtension)

typedef struct _VIDEO_PORT_EVENT
{
    ENG_EVENT;
} VIDEO_PORT_EVENT, *PVIDEO_PORT_EVENT;

/* agp.c */

NTSTATUS
IopInitiatePnpIrp(
  PDEVICE_OBJECT DeviceObject,
  PIO_STATUS_BLOCK IoStatusBlock,
  UCHAR MinorFunction,
  PIO_STACK_LOCATION Stack OPTIONAL);

NTSTATUS NTAPI
IntAgpGetInterface(
   IN PVOID HwDeviceExtension,
   IN OUT PINTERFACE Interface);

/* dispatch.c */

NTSTATUS NTAPI
IntVideoPortAddDevice(
   IN PDRIVER_OBJECT DriverObject,
   IN PDEVICE_OBJECT PhysicalDeviceObject);

NTSTATUS NTAPI
IntVideoPortDispatchOpen(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchClose(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchCleanup(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchDeviceControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchPnp(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchPower(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchSystemControl(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

NTSTATUS NTAPI
IntVideoPortDispatchWrite(
   IN PDEVICE_OBJECT DeviceObject,
   IN PIRP Irp);

VOID NTAPI
IntVideoPortUnload(PDRIVER_OBJECT DriverObject);

/* timer.c */

BOOLEAN NTAPI
IntVideoPortSetupTimer(
   IN PDEVICE_OBJECT DeviceObject,
   IN PVIDEO_PORT_DRIVER_EXTENSION DriverExtension);

/* interrupt.c */

BOOLEAN NTAPI
IntVideoPortSetupInterrupt(
   IN PDEVICE_OBJECT DeviceObject,
   IN PVIDEO_PORT_DRIVER_EXTENSION DriverExtension,
   IN PVIDEO_PORT_CONFIG_INFO ConfigInfo);

/* resource.c */

NTSTATUS NTAPI
IntVideoPortMapPhysicalMemory(
   IN HANDLE Process,
   IN PHYSICAL_ADDRESS PhysicalAddress,
   IN ULONG SizeInBytes,
   IN ULONG Protect,
   IN OUT PVOID *VirtualAddress  OPTIONAL);

/* videoprt.c */

extern ULONG CsrssInitialized;
extern PKPROCESS Csrss;
extern ULONG VideoPortDeviceNumber;

VOID FASTCALL
IntAttachToCSRSS(PKPROCESS *CallingProcess, PKAPC_STATE ApcState);

VOID FASTCALL
IntDetachFromCSRSS(PKPROCESS *CallingProcess, PKAPC_STATE ApcState);

NTSTATUS NTAPI
IntVideoPortCreateAdapterDeviceObject(
   IN PDRIVER_OBJECT DriverObject,
   IN PVIDEO_PORT_DRIVER_EXTENSION DriverExtension,
   IN PDEVICE_OBJECT PhysicalDeviceObject  OPTIONAL,
   OUT PDEVICE_OBJECT *DeviceObject  OPTIONAL);

NTSTATUS NTAPI
IntVideoPortFindAdapter(
   IN PDRIVER_OBJECT DriverObject,
   IN PVIDEO_PORT_DRIVER_EXTENSION DriverExtension,
   IN PDEVICE_OBJECT DeviceObject);

PVOID NTAPI
IntVideoPortGetProcAddress(
   IN PVOID HwDeviceExtension,
   IN PUCHAR FunctionName);

/* int10.c */

VP_STATUS NTAPI
IntInt10AllocateBuffer(
   IN PVOID Context,
   OUT PUSHORT Seg,
   OUT PUSHORT Off,
   IN OUT PULONG Length);

VP_STATUS NTAPI
IntInt10FreeBuffer(
   IN PVOID Context,
   IN USHORT Seg,
   IN USHORT Off);

VP_STATUS NTAPI
IntInt10ReadMemory(
   IN PVOID Context,
   IN USHORT Seg,
   IN USHORT Off,
   OUT PVOID Buffer,
   IN ULONG Length);

VP_STATUS NTAPI
IntInt10WriteMemory(
   IN PVOID Context,
   IN USHORT Seg,
   IN USHORT Off,
   IN PVOID Buffer,
   IN ULONG Length);

VP_STATUS NTAPI
IntInt10CallBios(
   IN PVOID Context,
   IN OUT PINT10_BIOS_ARGUMENTS BiosArguments);

#endif /* VIDEOPRT_H */
