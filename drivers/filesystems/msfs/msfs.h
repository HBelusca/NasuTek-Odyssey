/*
 * COPYRIGHT:  See COPYING in the top level directory
 * PROJECT:    Odyssey kernel
 * FILE:       drivers/filesystems/msfs/msfs.h
 * PURPOSE:    Mailslot filesystem
 * PROGRAMMER: Eric Kohl
 */

#ifndef __DRIVERS_FS_MS_MSFS_H
#define __DRIVERS_FS_MS_MSFS_H

#include <ntifs.h>
#include <iotypes.h>

#define DEFAULTAPI NTAPI

typedef struct _MSFS_DEVICE_EXTENSION
{
    LIST_ENTRY FcbListHead;
    KMUTEX FcbListLock;
} MSFS_DEVICE_EXTENSION, *PMSFS_DEVICE_EXTENSION;


typedef struct _MSFS_FCB
{
    FSRTL_COMMON_FCB_HEADER RFCB;
    UNICODE_STRING Name;
    LIST_ENTRY FcbListEntry;
    KSPIN_LOCK CcbListLock;
    LIST_ENTRY CcbListHead;
    struct _MSFS_CCB *ServerCcb;
    ULONG ReferenceCount;
    LARGE_INTEGER TimeOut;
    KEVENT MessageEvent;
    ULONG MaxMessageSize;
    ULONG MessageCount;
    KSPIN_LOCK MessageListLock;
    LIST_ENTRY MessageListHead;
} MSFS_FCB, *PMSFS_FCB;


typedef struct _MSFS_CCB
{
    LIST_ENTRY CcbListEntry;
    PMSFS_FCB Fcb;
} MSFS_CCB, *PMSFS_CCB;


typedef struct _MSFS_MESSAGE
{
    LIST_ENTRY MessageListEntry;
    ULONG Size;
    UCHAR Buffer[1];
} MSFS_MESSAGE, *PMSFS_MESSAGE;


#define KeLockMutex(x) KeWaitForSingleObject(x, \
                                             UserRequest, \
                                             KernelMode, \
                                             FALSE, \
                                             NULL);

#define KeUnlockMutex(x) KeReleaseMutex(x, FALSE);

DRIVER_DISPATCH MsfsCreate;
NTSTATUS DEFAULTAPI MsfsCreate(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsCreateMailslot;
NTSTATUS DEFAULTAPI MsfsCreateMailslot(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsClose;
NTSTATUS DEFAULTAPI MsfsClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsQueryInformation;
NTSTATUS DEFAULTAPI MsfsQueryInformation(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsSetInformation;
NTSTATUS DEFAULTAPI MsfsSetInformation(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsRead;
NTSTATUS DEFAULTAPI MsfsRead(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsWrite;
NTSTATUS DEFAULTAPI MsfsWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp);

DRIVER_DISPATCH MsfsFileSystemControl;
NTSTATUS DEFAULTAPI MsfsFileSystemControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS NTAPI
DriverEntry(PDRIVER_OBJECT DriverObject,
            PUNICODE_STRING RegistryPath);

#endif /* __DRIVERS_FS_MS_MSFS_H */
