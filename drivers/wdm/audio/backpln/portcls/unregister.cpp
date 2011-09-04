/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey Kernel Streaming
 * FILE:            drivers/wdm/audio/backpln/portcls/unregister.cpp
 * PURPOSE:         Unregisters a subdevice
 * PROGRAMMER:      Johannes Anderwald
 */

#include "private.hpp"

class CUnregisterSubdevice : public IUnregisterSubdevice
{
public:
    STDMETHODIMP QueryInterface( REFIID InterfaceId, PVOID* Interface);

    STDMETHODIMP_(ULONG) AddRef()
    {
        InterlockedIncrement(&m_Ref);
        return m_Ref;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        InterlockedDecrement(&m_Ref);

        if (!m_Ref)
        {
            delete this;
            return 0;
        }
        return m_Ref;
    }


    IMP_IUnregisterSubdevice;

    CUnregisterSubdevice(IUnknown * OuterUnknown) : m_Ref(0) {}
    virtual ~CUnregisterSubdevice(){}

protected:
    LONG m_Ref;

};

NTSTATUS
NTAPI
CUnregisterSubdevice::QueryInterface(
    IN  REFIID refiid,
    OUT PVOID* Output)
{
    if (IsEqualGUIDAligned(refiid, IID_IUnregisterSubdevice) || 
        IsEqualGUIDAligned(refiid, IID_IUnknown))
    {
        *Output = PVOID(PUNREGISTERSUBDEVICE(this));
        PUNKNOWN(*Output)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
NTAPI
CUnregisterSubdevice::UnregisterSubdevice(
    IN PDEVICE_OBJECT  DeviceObject,
    IN PUNKNOWN  Unknown)
{
    PPCLASS_DEVICE_EXTENSION DeviceExtension;
    PLIST_ENTRY Entry;
    PSYMBOLICLINK_ENTRY SymLinkEntry;
    PSUBDEVICE_DESCRIPTOR SubDeviceDescriptor;
    ISubdevice *SubDevice;
    ULONG Index;
    NTSTATUS Status;

    PC_ASSERT_IRQL_EQUAL(PASSIVE_LEVEL);

    DeviceExtension = (PPCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PC_ASSERT(DeviceExtension);

    // look up our undocumented interface
    Status = Unknown->QueryInterface(IID_ISubdevice, (LPVOID*)&SubDevice);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("No ISubdevice interface\n");
        // the provided port driver doesnt support ISubdevice
        return STATUS_INVALID_PARAMETER;
    }

    Status = SubDevice->GetDescriptor(&SubDeviceDescriptor);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("Failed to retrieve subdevice descriptor %x\n", Status);
        // the provided port driver doesnt support ISubdevice
        return STATUS_INVALID_PARAMETER;
    }

    // loop our create items and disable the create handler
    for(Index = 0; Index < DeviceExtension->MaxSubDevices; Index++)
    {
        if (!RtlCompareUnicodeString(&SubDeviceDescriptor->RefString, &DeviceExtension->CreateItems[Index].ObjectClass, TRUE))
        {
            DeviceExtension->CreateItems[Index].Create = NULL;
            RtlInitUnicodeString(&DeviceExtension->CreateItems[Index].ObjectClass, NULL);
            break;
        }
    }

    // now unregister device interfaces
    while(!IsListEmpty(&SubDeviceDescriptor->SymbolicLinkList))
    {
        // remove entry
        Entry = RemoveHeadList(&SubDeviceDescriptor->SymbolicLinkList);
        // get symlink entry
        SymLinkEntry = (PSYMBOLICLINK_ENTRY)CONTAINING_RECORD(Entry, SYMBOLICLINK_ENTRY, Entry);

        // unregister device interface
        IoSetDeviceInterfaceState(&SymLinkEntry->SymbolicLink, FALSE);
        // free symbolic link
        RtlFreeUnicodeString(&SymLinkEntry->SymbolicLink);
        // free sym entry
        FreeItem(SymLinkEntry, TAG_PORTCLASS);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
NewIUnregisterSubdevice(
    OUT PUNREGISTERSUBDEVICE *OutDevice)
{
    NTSTATUS Status;
    CUnregisterSubdevice * This = new(NonPagedPool, TAG_PORTCLASS) CUnregisterSubdevice(NULL);
    if (!This)
        return STATUS_INSUFFICIENT_RESOURCES;

    Status = This->QueryInterface(IID_IUnregisterSubdevice, (PVOID*)OutDevice);
    if (!NT_SUCCESS(Status))
    {
        delete This;
        return Status;
    }

    *OutDevice = (PUNREGISTERSUBDEVICE)This;
    return Status;
}
