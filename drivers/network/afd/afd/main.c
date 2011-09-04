/* $Id: main.c 53145 2011-08-08 22:02:55Z cgutman $
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * FILE:             drivers/net/afd/afd/main.c
 * PURPOSE:          Ancillary functions driver
 * PROGRAMMER:       Art Yerkes (ayerkes@speakeasy.net)
 * UPDATE HISTORY:
 * 20040630 Created
 *
 * Suggestions: Uniform naming (AfdXxx)
 */

/* INCLUDES */

#include "afd.h"

#if DBG

/* See debug.h for debug/trace constants */
//DWORD DebugTraceLevel = DEBUG_ULTRA;
DWORD DebugTraceLevel = MIN_TRACE;

#endif /* DBG */

void OskitDumpBuffer( PCHAR Data, UINT Len ) {
    unsigned int i;

    for( i = 0; i < Len; i++ ) {
	if( i && !(i & 0xf) ) DbgPrint( "\n" );
	if( !(i & 0xf) ) DbgPrint( "%08x: ", (UINT)(Data + i) );
	DbgPrint( " %02x", Data[i] & 0xff );
    }
    DbgPrint("\n");
}

/* FUNCTIONS */

NTSTATUS NTAPI
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

NTSTATUS NTAPI
AfdGetDisconnectOptions(PDEVICE_OBJECT DeviceObject, PIRP Irp,
	          PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    UINT BufferSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);

    if (FCB->DisconnectOptionsSize == 0)
    {
        AFD_DbgPrint(MIN_TRACE,("Invalid parameter\n"));
        return UnlockAndMaybeComplete(FCB, STATUS_INVALID_PARAMETER, Irp, 0);
    }

    ASSERT(FCB->DisconnectOptions);

    if (FCB->FilledDisconnectOptions < BufferSize) BufferSize = FCB->FilledDisconnectOptions;

    RtlCopyMemory(Irp->UserBuffer,
                  FCB->DisconnectOptions,
                  BufferSize);

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, BufferSize);
}

NTSTATUS
NTAPI
AfdSetDisconnectOptions(PDEVICE_OBJECT DeviceObject, PIRP Irp,
                  PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PVOID DisconnectOptions = LockRequest(Irp, IrpSp);
    UINT DisconnectOptionsSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);
    
    if (!DisconnectOptions)
        return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    if (FCB->DisconnectOptions)
    {
        ExFreePool(FCB->DisconnectOptions);
        FCB->DisconnectOptions = NULL;
        FCB->DisconnectOptionsSize = 0;
        FCB->FilledDisconnectOptions = 0;
    }

    FCB->DisconnectOptions = ExAllocatePool(PagedPool, DisconnectOptionsSize);
    if (!FCB->DisconnectOptions) return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    RtlCopyMemory(FCB->DisconnectOptions,
                  DisconnectOptions,
                  DisconnectOptionsSize);

    FCB->DisconnectOptionsSize = DisconnectOptionsSize;

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, 0);
}

NTSTATUS
NTAPI
AfdSetDisconnectOptionsSize(PDEVICE_OBJECT DeviceObject, PIRP Irp,
                      PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PUINT DisconnectOptionsSize = LockRequest(Irp, IrpSp);
    UINT BufferSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);
    
    if (!DisconnectOptionsSize)
        return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    if (BufferSize < sizeof(UINT))
    {
        AFD_DbgPrint(MIN_TRACE,("Buffer too small\n"));
        return UnlockAndMaybeComplete(FCB, STATUS_BUFFER_TOO_SMALL, Irp, 0);
    }

    if (FCB->DisconnectOptions)
    {
        ExFreePool(FCB->DisconnectOptions);
        FCB->DisconnectOptionsSize = 0;
        FCB->FilledDisconnectOptions = 0;
    }

    FCB->DisconnectOptions = ExAllocatePool(PagedPool, *DisconnectOptionsSize);
    if (!FCB->DisconnectOptions) return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    FCB->DisconnectOptionsSize = *DisconnectOptionsSize;

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, 0);
}

NTSTATUS NTAPI
AfdGetDisconnectData(PDEVICE_OBJECT DeviceObject, PIRP Irp,
	          PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    UINT BufferSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);

    if (FCB->DisconnectDataSize == 0)
    {
        AFD_DbgPrint(MIN_TRACE,("Invalid parameter\n"));
        return UnlockAndMaybeComplete(FCB, STATUS_INVALID_PARAMETER, Irp, 0);
    }

    ASSERT(FCB->DisconnectData);

    if (FCB->FilledDisconnectData < BufferSize) BufferSize = FCB->FilledDisconnectData;

    RtlCopyMemory(Irp->UserBuffer,
                  FCB->DisconnectData,
                  BufferSize);

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, BufferSize);
}

NTSTATUS
NTAPI
AfdSetDisconnectData(PDEVICE_OBJECT DeviceObject, PIRP Irp,
                  PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PVOID DisconnectData = LockRequest(Irp, IrpSp);
    UINT DisconnectDataSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);
    
    if (!DisconnectData)
        return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    if (FCB->DisconnectData)
    {
        ExFreePool(FCB->DisconnectData);
        FCB->DisconnectData = NULL;
        FCB->DisconnectDataSize = 0;
        FCB->FilledDisconnectData = 0;
    }

    FCB->DisconnectData = ExAllocatePool(PagedPool, DisconnectDataSize);
    if (!FCB->DisconnectData) return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    RtlCopyMemory(FCB->DisconnectData,
                  DisconnectData,
                  DisconnectDataSize);

    FCB->DisconnectDataSize = DisconnectDataSize;

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, 0);
}

NTSTATUS
NTAPI
AfdSetDisconnectDataSize(PDEVICE_OBJECT DeviceObject, PIRP Irp,
                      PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PUINT DisconnectDataSize = LockRequest(Irp, IrpSp);
    UINT BufferSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);
    
    if (!DisconnectDataSize)
        return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    if (BufferSize < sizeof(UINT))
    {
        AFD_DbgPrint(MIN_TRACE,("Buffer too small\n"));
        return UnlockAndMaybeComplete(FCB, STATUS_BUFFER_TOO_SMALL, Irp, 0);
    }

    if (FCB->DisconnectData)
    {
        ExFreePool(FCB->DisconnectData);
        FCB->DisconnectDataSize = 0;
        FCB->FilledDisconnectData = 0;
    }

    FCB->DisconnectData = ExAllocatePool(PagedPool, *DisconnectDataSize);
    if (!FCB->DisconnectData) return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    FCB->DisconnectDataSize = *DisconnectDataSize;

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, 0);
}

static NTSTATUS NTAPI
AfdGetTdiHandles(PDEVICE_OBJECT DeviceObject, PIRP Irp,
                PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PULONG HandleFlags = LockRequest(Irp, IrpSp);
    PAFD_TDI_HANDLE_DATA HandleData = Irp->UserBuffer;

    if (!SocketAcquireStateLock(FCB)) return LostSocket(Irp);
    
    if (!HandleFlags)
        return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(ULONG) ||
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(*HandleData))
    {
        AFD_DbgPrint(MIN_TRACE,("Buffer too small\n"));
        return UnlockAndMaybeComplete(FCB, STATUS_BUFFER_TOO_SMALL, Irp, 0);
    }

    if ((*HandleFlags) & AFD_ADDRESS_HANDLE)
        HandleData->TdiAddressHandle = FCB->AddressFile.Handle;

    if ((*HandleFlags) & AFD_CONNECTION_HANDLE)
        HandleData->TdiConnectionHandle = FCB->Connection.Handle;

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, 0);
}

static NTSTATUS NTAPI
AfdCreateSocket(PDEVICE_OBJECT DeviceObject, PIRP Irp,
		PIO_STACK_LOCATION IrpSp) {
    PAFD_FCB FCB;
    PFILE_OBJECT FileObject;
    PAFD_DEVICE_EXTENSION DeviceExt;
    PFILE_FULL_EA_INFORMATION EaInfo;
    PAFD_CREATE_PACKET ConnectInfo = NULL;
    ULONG EaLength;
    PWCHAR EaInfoValue = NULL;
    UINT Disposition, i;
    NTSTATUS Status = STATUS_SUCCESS;

    AFD_DbgPrint(MID_TRACE,
		 ("AfdCreate(DeviceObject %p Irp %p)\n", DeviceObject, Irp));

    DeviceExt = DeviceObject->DeviceExtension;
    FileObject = IrpSp->FileObject;
    Disposition = (IrpSp->Parameters.Create.Options >> 24) & 0xff;

    Irp->IoStatus.Information = 0;

    EaInfo = Irp->AssociatedIrp.SystemBuffer;

    if( EaInfo ) {
	ConnectInfo = (PAFD_CREATE_PACKET)(EaInfo->EaName + EaInfo->EaNameLength + 1);
	EaInfoValue = (PWCHAR)(((PCHAR)ConnectInfo) + sizeof(AFD_CREATE_PACKET));

	EaLength = sizeof(FILE_FULL_EA_INFORMATION) +
	    EaInfo->EaNameLength +
	    EaInfo->EaValueLength;

	AFD_DbgPrint(MID_TRACE,("EaInfo: %x, EaInfoValue: %x\n",
				EaInfo, EaInfoValue));
    }

    AFD_DbgPrint(MID_TRACE,("About to allocate the new FCB\n"));

    FCB = ExAllocatePool(NonPagedPool, sizeof(AFD_FCB));
    if( FCB == NULL ) {
	Irp->IoStatus.Status = STATUS_NO_MEMORY;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_NO_MEMORY;
    }

    AFD_DbgPrint(MID_TRACE,("Initializing the new FCB @ %x (FileObject %x Flags %x)\n", FCB, FileObject, ConnectInfo ? ConnectInfo->EndpointFlags : 0));

    RtlZeroMemory( FCB, sizeof( *FCB ) );

    FCB->Flags = ConnectInfo ? ConnectInfo->EndpointFlags : 0;
    FCB->GroupID = ConnectInfo ? ConnectInfo->GroupID : 0;
    FCB->GroupType = 0; /* FIXME */
    FCB->State = SOCKET_STATE_CREATED;
    FCB->FileObject = FileObject;
    FCB->DeviceExt = DeviceExt;
    FCB->AddressFile.Handle = INVALID_HANDLE_VALUE;
    FCB->Connection.Handle = INVALID_HANDLE_VALUE;

    KeInitializeMutex( &FCB->Mutex, 0 );

    for( i = 0; i < MAX_FUNCTIONS; i++ ) {
	InitializeListHead( &FCB->PendingIrpList[i] );
    }

    InitializeListHead( &FCB->DatagramList );
    InitializeListHead( &FCB->PendingConnections );

    AFD_DbgPrint(MID_TRACE,("%x: Checking command channel\n", FCB));

    if( ConnectInfo ) {
	FCB->TdiDeviceName.Length = ConnectInfo->SizeOfTransportName;
	FCB->TdiDeviceName.MaximumLength = FCB->TdiDeviceName.Length;
	FCB->TdiDeviceName.Buffer =
	    ExAllocatePool( NonPagedPool, FCB->TdiDeviceName.Length );

	if( !FCB->TdiDeviceName.Buffer ) {
	    ExFreePool(FCB);
	    AFD_DbgPrint(MID_TRACE,("Could not copy target string\n"));
	    Irp->IoStatus.Status = STATUS_NO_MEMORY;
	    IoCompleteRequest( Irp, IO_NETWORK_INCREMENT );
	    return STATUS_NO_MEMORY;
	}

	RtlCopyMemory( FCB->TdiDeviceName.Buffer,
		       ConnectInfo->TransportName,
		       FCB->TdiDeviceName.Length );

	AFD_DbgPrint(MID_TRACE,("Success: %s %wZ\n",
				EaInfo->EaName, &FCB->TdiDeviceName));
    } else {
	AFD_DbgPrint(MID_TRACE,("Success: Control connection\n"));
    }

    FileObject->FsContext = FCB;

    /* It seems that UDP sockets are writable from inception */
    if( FCB->Flags & AFD_ENDPOINT_CONNECTIONLESS ) {
        AFD_DbgPrint(MID_TRACE,("Packet oriented socket\n"));
        
	/* A datagram socket is always sendable */
	FCB->PollState |= AFD_EVENT_SEND;
        FCB->PollStatus[FD_WRITE_BIT] = STATUS_SUCCESS;
        PollReeval( FCB->DeviceExt, FCB->FileObject );
    }

    if( !NT_SUCCESS(Status) ) {
	if( FCB->TdiDeviceName.Buffer ) ExFreePool( FCB->TdiDeviceName.Buffer );
	ExFreePool( FCB );
	FileObject->FsContext = NULL;
    }

    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp, IO_NETWORK_INCREMENT );

    return Status;
}

static NTSTATUS NTAPI
AfdCleanupSocket(PDEVICE_OBJECT DeviceObject, PIRP Irp,
                 PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PLIST_ENTRY CurrentEntry, NextEntry;
    UINT Function;
    PIRP CurrentIrp;

    if( !SocketAcquireStateLock( FCB ) ) return LostSocket(Irp);

    for (Function = 0; Function < MAX_FUNCTIONS; Function++)
    {
        CurrentEntry = FCB->PendingIrpList[Function].Flink;
        while (CurrentEntry != &FCB->PendingIrpList[Function])
        {
           NextEntry = CurrentEntry->Flink;
           CurrentIrp = CONTAINING_RECORD(CurrentEntry, IRP, Tail.Overlay.ListEntry);

           /* The cancel routine will remove the IRP from the list */
           IoCancelIrp(CurrentIrp);

           CurrentEntry = NextEntry;
        }
    }

    KillSelectsForFCB( FCB->DeviceExt, FileObject, FALSE );

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, 0);
}

static NTSTATUS NTAPI
AfdCloseSocket(PDEVICE_OBJECT DeviceObject, PIRP Irp,
	       PIO_STACK_LOCATION IrpSp)
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    UINT i;
    PAFD_IN_FLIGHT_REQUEST InFlightRequest[IN_FLIGHT_REQUESTS];
    PAFD_TDI_OBJECT_QELT Qelt;
    PLIST_ENTRY QeltEntry;
    

    AFD_DbgPrint(MID_TRACE,
		 ("AfdClose(DeviceObject %p Irp %p)\n", DeviceObject, Irp));

    if( !SocketAcquireStateLock( FCB ) ) return STATUS_FILE_CLOSED;

    FCB->State = SOCKET_STATE_CLOSED;

    InFlightRequest[0] = &FCB->ListenIrp;
    InFlightRequest[1] = &FCB->ReceiveIrp;
    InFlightRequest[2] = &FCB->SendIrp;
    InFlightRequest[3] = &FCB->ConnectIrp;
    InFlightRequest[4] = &FCB->DisconnectIrp;

    /* Cancel our pending requests */
    for( i = 0; i < IN_FLIGHT_REQUESTS; i++ ) {
	if( InFlightRequest[i]->InFlightRequest ) {
	    AFD_DbgPrint(MID_TRACE,("Cancelling in flight irp %d (%x)\n",
				    i, InFlightRequest[i]->InFlightRequest));
            IoCancelIrp(InFlightRequest[i]->InFlightRequest);
	}
    }

    KillSelectsForFCB( FCB->DeviceExt, FileObject, FALSE );

    ASSERT(IsListEmpty(&FCB->PendingIrpList[FUNCTION_CONNECT]));
    ASSERT(IsListEmpty(&FCB->PendingIrpList[FUNCTION_SEND]));
    ASSERT(IsListEmpty(&FCB->PendingIrpList[FUNCTION_RECV]));
    ASSERT(IsListEmpty(&FCB->PendingIrpList[FUNCTION_PREACCEPT]));
    ASSERT(IsListEmpty(&FCB->PendingIrpList[FUNCTION_DISCONNECT]));

    while (!IsListEmpty(&FCB->PendingConnections))
    {
        QeltEntry = RemoveHeadList(&FCB->PendingConnections);
        Qelt = CONTAINING_RECORD(QeltEntry, AFD_TDI_OBJECT_QELT, ListEntry);

        /* We have to close all pending connections or the listen won't get closed */
        TdiDisassociateAddressFile(Qelt->Object.Object);
        ObDereferenceObject(Qelt->Object.Object);
        ZwClose(Qelt->Object.Handle);

        ExFreePool(Qelt);
    }

    SocketStateUnlock( FCB );

    if( FCB->EventSelect )
        ObDereferenceObject( FCB->EventSelect );

    if( FCB->Context )
        ExFreePool( FCB->Context );

    if( FCB->Recv.Window )
	ExFreePool( FCB->Recv.Window );

    if( FCB->Send.Window )
	ExFreePool( FCB->Send.Window );

    if( FCB->AddressFrom )
	ExFreePool( FCB->AddressFrom );

    if( FCB->ConnectCallInfo )
        ExFreePool( FCB->ConnectCallInfo );
    
    if( FCB->ConnectReturnInfo )
        ExFreePool( FCB->ConnectReturnInfo );

    if( FCB->ConnectData )
        ExFreePool( FCB->ConnectData );

    if( FCB->DisconnectData )
        ExFreePool( FCB->DisconnectData );

    if( FCB->ConnectOptions )
        ExFreePool( FCB->ConnectOptions );

    if( FCB->DisconnectOptions )
        ExFreePool( FCB->DisconnectOptions );

    if( FCB->LocalAddress )
	ExFreePool( FCB->LocalAddress );

    if( FCB->RemoteAddress )
	ExFreePool( FCB->RemoteAddress );

    if( FCB->Connection.Object )
    {
        TdiDisassociateAddressFile(FCB->Connection.Object);
	ObDereferenceObject(FCB->Connection.Object);
    }

    if( FCB->AddressFile.Object )
	ObDereferenceObject(FCB->AddressFile.Object);

    if( FCB->AddressFile.Handle != INVALID_HANDLE_VALUE )
    {
        if (ZwClose(FCB->AddressFile.Handle) == STATUS_INVALID_HANDLE)
        {
            DbgPrint("INVALID ADDRESS FILE HANDLE VALUE: %x %x\n", FCB->AddressFile.Handle, FCB->AddressFile.Object);
        }
    }

    if( FCB->Connection.Handle != INVALID_HANDLE_VALUE )
    {
        if (ZwClose(FCB->Connection.Handle) == STATUS_INVALID_HANDLE)
        {
            DbgPrint("INVALID CONNECTION HANDLE VALUE: %x %x\n", FCB->Connection.Handle, FCB->Connection.Object);
        }
    }

    if( FCB->TdiDeviceName.Buffer )
	ExFreePool(FCB->TdiDeviceName.Buffer);

    ExFreePool(FCB);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);

    AFD_DbgPrint(MID_TRACE, ("Returning success.\n"));

    return STATUS_SUCCESS;
}

static
NTSTATUS
NTAPI
DisconnectComplete(PDEVICE_OBJECT DeviceObject,
                   PIRP Irp,
                   PVOID Context)
{
    PAFD_FCB FCB = Context;
    PIRP CurrentIrp;
    PLIST_ENTRY CurrentEntry;
    
    if( !SocketAcquireStateLock( FCB ) )
        return STATUS_FILE_CLOSED;
    
    ASSERT(FCB->DisconnectIrp.InFlightRequest == Irp);
    FCB->DisconnectIrp.InFlightRequest = NULL;
    
    ASSERT(FCB->DisconnectPending);
    ASSERT((IsListEmpty(&FCB->PendingIrpList[FUNCTION_SEND]) && !FCB->SendIrp.InFlightRequest) ||
           (FCB->DisconnectFlags & TDI_DISCONNECT_ABORT));
    
    if (NT_SUCCESS(Irp->IoStatus.Status) && (FCB->DisconnectFlags & TDI_DISCONNECT_RELEASE))
    {
        FCB->FilledDisconnectData = MIN(FCB->DisconnectDataSize, FCB->ConnectReturnInfo->UserDataLength);
        if (FCB->FilledDisconnectData)
        {
            RtlCopyMemory(FCB->DisconnectData,
                          FCB->ConnectReturnInfo->UserData,
                          FCB->FilledDisconnectData);
        }
        
        FCB->FilledDisconnectOptions = MIN(FCB->DisconnectOptionsSize, FCB->ConnectReturnInfo->OptionsLength);
        if (FCB->FilledDisconnectOptions)
        {
            RtlCopyMemory(FCB->DisconnectOptions,
                          FCB->ConnectReturnInfo->Options,
                          FCB->FilledDisconnectOptions);
        }
    }
    
    FCB->DisconnectPending = FALSE;
    
    while (!IsListEmpty(&FCB->PendingIrpList[FUNCTION_DISCONNECT]))
    {
        CurrentEntry = RemoveHeadList(&FCB->PendingIrpList[FUNCTION_DISCONNECT]);
        CurrentIrp = CONTAINING_RECORD(CurrentEntry, IRP, Tail.Overlay.ListEntry);
        CurrentIrp->IoStatus.Status = Irp->IoStatus.Status;
        CurrentIrp->IoStatus.Information = 0;
        UnlockRequest(CurrentIrp, IoGetCurrentIrpStackLocation(CurrentIrp));
        (void)IoSetCancelRoutine(CurrentIrp, NULL);
        IoCompleteRequest(CurrentIrp, IO_NETWORK_INCREMENT );
    }
    
    if (!(FCB->DisconnectFlags & TDI_DISCONNECT_RELEASE))
    {
        /* Signal complete connection closure immediately */
        FCB->PollState |= AFD_EVENT_ABORT;
        FCB->PollStatus[FD_CLOSE_BIT] = Irp->IoStatus.Status;
        PollReeval(FCB->DeviceExt, FCB->FileObject);
    }
    
    SocketStateUnlock(FCB);
    
    return Irp->IoStatus.Status;
}

static
NTSTATUS
DoDisconnect(PAFD_FCB FCB)
{
    NTSTATUS Status;
    
    ASSERT(FCB->DisconnectPending);
    ASSERT((IsListEmpty(&FCB->PendingIrpList[FUNCTION_SEND]) && !FCB->SendIrp.InFlightRequest) ||
           (FCB->DisconnectFlags & TDI_DISCONNECT_ABORT));
    
    if (FCB->DisconnectIrp.InFlightRequest)
    {
        return STATUS_PENDING;
    }

    FCB->ConnectCallInfo->UserData = FCB->DisconnectData;
    FCB->ConnectCallInfo->UserDataLength = FCB->DisconnectDataSize;
    FCB->ConnectCallInfo->Options = FCB->DisconnectOptions;
    FCB->ConnectCallInfo->OptionsLength = FCB->DisconnectOptionsSize;

    Status = TdiDisconnect(&FCB->DisconnectIrp.InFlightRequest,
                           FCB->Connection.Object,
                           &FCB->DisconnectTimeout,
                           FCB->DisconnectFlags,
                           &FCB->DisconnectIrp.Iosb,
                           DisconnectComplete,
                           FCB,
                           FCB->ConnectCallInfo,
                           FCB->ConnectReturnInfo);
    if (Status != STATUS_PENDING)
    {
        FCB->DisconnectPending = FALSE;
    }
    
    return Status;
}

VOID
RetryDisconnectCompletion(PAFD_FCB FCB)
{
    ASSERT(FCB->RemoteAddress);

    if (IsListEmpty(&FCB->PendingIrpList[FUNCTION_SEND]) && !FCB->SendIrp.InFlightRequest && FCB->DisconnectPending)
    {
        /* Sends are done; fire off a TDI_DISCONNECT request */
        DoDisconnect(FCB);
    }
}

static NTSTATUS NTAPI
AfdDisconnect(PDEVICE_OBJECT DeviceObject, PIRP Irp,
	      PIO_STACK_LOCATION IrpSp) {
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PAFD_DISCONNECT_INFO DisReq;
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT Flags;
    PLIST_ENTRY CurrentEntry;
    PIRP CurrentIrp;

    if( !SocketAcquireStateLock( FCB ) ) return LostSocket( Irp );

    if( !(DisReq = LockRequest( Irp, IrpSp )) )
	return UnlockAndMaybeComplete( FCB, STATUS_NO_MEMORY,
				       Irp, 0 );
    
    /* Send direction only */
    if ((DisReq->DisconnectType & AFD_DISCONNECT_SEND) &&
        !(DisReq->DisconnectType & AFD_DISCONNECT_RECV))
    {
        /* Perform a controlled disconnect */
        Flags = TDI_DISCONNECT_RELEASE;
    }
    /* Receive direction or both */
    else
    {
        /* Mark that we can't issue another receive request */
        FCB->TdiReceiveClosed = TRUE;

        /* Try to cancel a pending TDI receive IRP if there was one in progress */
        if (FCB->ReceiveIrp.InFlightRequest)
            IoCancelIrp(FCB->ReceiveIrp.InFlightRequest);

        /* Discard any pending data */
        FCB->Recv.Content = 0;
        FCB->Recv.BytesUsed = 0;

        /* Mark us as overread to complete future reads with an error */
        FCB->Overread = TRUE;

        /* Set a successful close status to indicate a shutdown on overread */
        FCB->PollStatus[FD_CLOSE_BIT] = STATUS_SUCCESS;

        /* Clear the receive event */
        FCB->PollState &= ~AFD_EVENT_RECEIVE;

        /* Receive direction only */
        if ((DisReq->DisconnectType & AFD_DISCONNECT_RECV) &&
            !(DisReq->DisconnectType & AFD_DISCONNECT_SEND))
        {
            /* No need to tell the transport driver for receive direction only */
            return UnlockAndMaybeComplete( FCB, STATUS_SUCCESS, Irp, 0 );
        }
        else
        {
            /* Perform an abortive disconnect */
            Flags = TDI_DISCONNECT_ABORT;
        }
    }

    if (!(FCB->Flags & AFD_ENDPOINT_CONNECTIONLESS))
    {
        if( !FCB->ConnectCallInfo )
        {
            AFD_DbgPrint(MIN_TRACE,("Invalid parameter\n"));
            return UnlockAndMaybeComplete( FCB, STATUS_INVALID_PARAMETER,
                                           Irp, 0 );
        }

        if (FCB->DisconnectPending)
        {
            if (FCB->DisconnectIrp.InFlightRequest)
            {
                IoCancelIrp(FCB->DisconnectIrp.InFlightRequest);
                ASSERT(!FCB->DisconnectIrp.InFlightRequest);
            }
            else
            {
                while (!IsListEmpty(&FCB->PendingIrpList[FUNCTION_DISCONNECT]))
                {
                    CurrentEntry = RemoveHeadList(&FCB->PendingIrpList[FUNCTION_DISCONNECT]);
                    CurrentIrp = CONTAINING_RECORD(CurrentEntry, IRP, Tail.Overlay.ListEntry);
                    CurrentIrp->IoStatus.Status = STATUS_CANCELLED;
                    CurrentIrp->IoStatus.Information = 0;
                    UnlockRequest(CurrentIrp, IoGetCurrentIrpStackLocation(CurrentIrp));
                    (void)IoSetCancelRoutine(CurrentIrp, NULL);
                    IoCompleteRequest(CurrentIrp, IO_NETWORK_INCREMENT );
                }
            }
        }
        
        FCB->DisconnectFlags = Flags;
        FCB->DisconnectTimeout = DisReq->Timeout;
        FCB->DisconnectPending = TRUE;
        FCB->SendClosed = TRUE;
        FCB->PollState &= ~AFD_EVENT_SEND;
        
        Status = QueueUserModeIrp(FCB, Irp, FUNCTION_DISCONNECT);
        if (Status == STATUS_PENDING)
        {
            if ((IsListEmpty(&FCB->PendingIrpList[FUNCTION_SEND]) && !FCB->SendIrp.InFlightRequest) ||
                (FCB->DisconnectFlags & TDI_DISCONNECT_ABORT))
            {
                /* Go ahead and execute the disconnect because we're ready for it */
                Status = DoDisconnect(FCB);
            }
            
            if (Status != STATUS_PENDING)
                RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
        }
        
        if (Status == STATUS_PENDING)
        {
            SocketStateUnlock(FCB);

            return Status;
        }
    }
    else
    {
        if (!(Flags & TDI_DISCONNECT_RELEASE))
        {
            if (!FCB->RemoteAddress)
            {
                AFD_DbgPrint(MIN_TRACE,("Invalid parameter\n"));
                return UnlockAndMaybeComplete(FCB, STATUS_INVALID_PARAMETER, Irp, 0);
            }
        
            ExFreePool(FCB->RemoteAddress);
        
            FCB->RemoteAddress = NULL;
        }
        
        FCB->PollState &= ~AFD_EVENT_SEND;
        FCB->SendClosed = TRUE;
    }

    return UnlockAndMaybeComplete( FCB, Status, Irp, 0 );
}

static NTSTATUS NTAPI
AfdDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS Status = STATUS_NOT_IMPLEMENTED;
#if DBG
    PFILE_OBJECT FileObject = IrpSp->FileObject;
#endif

    AFD_DbgPrint(MID_TRACE,("AfdDispatch: %d\n", IrpSp->MajorFunction));
    if( IrpSp->MajorFunction != IRP_MJ_CREATE) {
	AFD_DbgPrint(MID_TRACE,("FO %x, IrpSp->FO %x\n",
				FileObject, IrpSp->FileObject));
	ASSERT(FileObject == IrpSp->FileObject);
    }

    Irp->IoStatus.Information = 0;

    switch(IrpSp->MajorFunction)
    {
	/* opening and closing handles to the device */
    case IRP_MJ_CREATE:
	/* Mostly borrowed from the named pipe file system */
	return AfdCreateSocket(DeviceObject, Irp, IrpSp);

    case IRP_MJ_CLOSE:
	/* Ditto the borrowing */
	return AfdCloseSocket(DeviceObject, Irp, IrpSp);

    case IRP_MJ_CLEANUP:
        return AfdCleanupSocket(DeviceObject, Irp, IrpSp);

    /* write data */
    case IRP_MJ_WRITE:
	return AfdConnectedSocketWriteData( DeviceObject, Irp, IrpSp, TRUE );

    /* read data */
    case IRP_MJ_READ:
	return AfdConnectedSocketReadData( DeviceObject, Irp, IrpSp, TRUE );

    case IRP_MJ_DEVICE_CONTROL:
    {
	switch( IrpSp->Parameters.DeviceIoControl.IoControlCode ) {
	case IOCTL_AFD_BIND:
	    return AfdBindSocket( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_CONNECT:
	    return AfdStreamSocketConnect( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_START_LISTEN:
	    return AfdListenSocket( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_RECV:
	    return AfdConnectedSocketReadData( DeviceObject, Irp, IrpSp,
					       FALSE );

	case IOCTL_AFD_SELECT:
	    return AfdSelect( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_EVENT_SELECT:
	    return AfdEventSelect( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_ENUM_NETWORK_EVENTS:
	    return AfdEnumEvents( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_RECV_DATAGRAM:
	    return AfdPacketSocketReadData( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_SEND:
	    return AfdConnectedSocketWriteData( DeviceObject, Irp, IrpSp,
						FALSE );

	case IOCTL_AFD_SEND_DATAGRAM:
	    return AfdPacketSocketWriteData( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_GET_INFO:
	    return AfdGetInfo( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_SET_INFO:
	    return AfdSetInfo( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_GET_CONTEXT_SIZE:
	    return AfdGetContextSize( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_GET_CONTEXT:
	    return AfdGetContext( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_SET_CONTEXT:
	    return AfdSetContext( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_WAIT_FOR_LISTEN:
	    return AfdWaitForListen( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_ACCEPT:
	    return AfdAccept( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_DISCONNECT:
	    return AfdDisconnect( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_GET_SOCK_NAME:
	    return AfdGetSockName( DeviceObject, Irp, IrpSp );

        case IOCTL_AFD_GET_PEER_NAME:
            return AfdGetPeerName( DeviceObject, Irp, IrpSp );

	case IOCTL_AFD_GET_CONNECT_DATA:
	    return AfdGetConnectData(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_CONNECT_DATA:
	    return AfdSetConnectData(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_DISCONNECT_DATA:
	    return AfdSetDisconnectData(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_GET_DISCONNECT_DATA:
	    return AfdGetDisconnectData(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_CONNECT_DATA_SIZE:
	    return AfdSetConnectDataSize(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_DISCONNECT_DATA_SIZE:
	    return AfdSetDisconnectDataSize(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_CONNECT_OPTIONS:
	    return AfdSetConnectOptions(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_DISCONNECT_OPTIONS:
	    return AfdSetDisconnectOptions(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_GET_CONNECT_OPTIONS:
	    return AfdGetConnectOptions(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_GET_DISCONNECT_OPTIONS:
	    return AfdGetDisconnectOptions(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_CONNECT_OPTIONS_SIZE:
	    return AfdSetConnectOptionsSize(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_SET_DISCONNECT_OPTIONS_SIZE:
	    return AfdSetDisconnectOptionsSize(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_GET_TDI_HANDLES:
	    return AfdGetTdiHandles(DeviceObject, Irp, IrpSp);

	case IOCTL_AFD_DEFER_ACCEPT:
	    DbgPrint("IOCTL_AFD_DEFER_ACCEPT is UNIMPLEMENTED!\n");
	    break;

	case IOCTL_AFD_GET_PENDING_CONNECT_DATA:
	    DbgPrint("IOCTL_AFD_GET_PENDING_CONNECT_DATA is UNIMPLEMENTED!\n");
	    break;

	case IOCTL_AFD_VALIDATE_GROUP:
	    DbgPrint("IOCTL_AFD_VALIDATE_GROUP is UNIMPLEMENTED!\n");
	    break;

	default:
	    Status = STATUS_NOT_SUPPORTED;
	    DbgPrint("Unknown IOCTL (0x%x)\n",
		     IrpSp->Parameters.DeviceIoControl.IoControlCode);
	    break;
	}
	break;
    }

/* unsupported operations */
    default:
    {
	Status = STATUS_NOT_IMPLEMENTED;
	AFD_DbgPrint(MIN_TRACE,
		     ("Irp: Unknown Major code was %x\n",
		      IrpSp->MajorFunction));
	break;
    }
    }

    AFD_DbgPrint(MID_TRACE, ("Returning %x\n", Status));
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return (Status);
}

BOOLEAN CheckUnlockExtraBuffers(PAFD_FCB FCB, PIO_STACK_LOCATION IrpSp)
{
    if (FCB->Flags & AFD_ENDPOINT_CONNECTIONLESS)
    {
        if (IrpSp->MajorFunction == IRP_MJ_READ || IrpSp->MajorFunction == IRP_MJ_WRITE)
        {
            /* read()/write() call - no extra buffers */
            return FALSE;
        }
        else if (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL)
        {
            if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_RECV_DATAGRAM)
            {
                /* recvfrom() call - extra buffers */
                return TRUE;
            }
            else if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_RECV)
            {
                /* recv() call - no extra buffers */
                return FALSE;
            }
            else if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_SEND ||
                     IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_SEND_DATAGRAM)
            {
                /* send()/sendto() call - no extra buffers */
                return FALSE;
            }
            else
            {
                /* Unknown IOCTL */
                ASSERT(FALSE);
                return FALSE;
            }
        }
        else
        {
            /* Unknown IRP_MJ code */
            ASSERT(FALSE);
            return FALSE;
        }
    }
    else
    {
        /* Connection-oriented never has extra buffers */
        return FALSE;
    }
}

VOID
CleanupPendingIrp(PAFD_FCB FCB, PIRP Irp, PIO_STACK_LOCATION IrpSp, PAFD_ACTIVE_POLL Poll)
{
    PAFD_RECV_INFO RecvReq;
    PAFD_SEND_INFO SendReq;
    PAFD_POLL_INFO PollReq;
    
    if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_RECV ||
        IrpSp->MajorFunction == IRP_MJ_READ)
    {
        RecvReq = GetLockedData(Irp, IrpSp);
        UnlockBuffers(RecvReq->BufferArray, RecvReq->BufferCount, CheckUnlockExtraBuffers(FCB, IrpSp));
    }
    else if ((IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_SEND ||
              IrpSp->MajorFunction == IRP_MJ_WRITE))
    {
        SendReq = GetLockedData(Irp, IrpSp);
        UnlockBuffers(SendReq->BufferArray, SendReq->BufferCount, CheckUnlockExtraBuffers(FCB, IrpSp));
    }
    else if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_AFD_SELECT)
    {
        PollReq = Irp->AssociatedIrp.SystemBuffer;
        ZeroEvents(PollReq->Handles, PollReq->HandleCount);
        SignalSocket(Poll, NULL, PollReq, STATUS_CANCELLED);
    }       
}

VOID NTAPI
AfdCancelHandler(PDEVICE_OBJECT DeviceObject,
                 PIRP Irp)
{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    ULONG Function, IoctlCode;
    PIRP CurrentIrp;
    PLIST_ENTRY CurrentEntry;
    PAFD_DEVICE_EXTENSION DeviceExt = DeviceObject->DeviceExtension;
    KIRQL OldIrql;
    PAFD_ACTIVE_POLL Poll;

    IoReleaseCancelSpinLock(Irp->CancelIrql);
    
    if (!SocketAcquireStateLock(FCB))
        return;
    
    switch (IrpSp->MajorFunction)
    {
        case IRP_MJ_DEVICE_CONTROL:
            IoctlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
            break;
            
        case IRP_MJ_READ:
            IoctlCode = IOCTL_AFD_RECV;
            break;
            
        case IRP_MJ_WRITE:
            IoctlCode = IOCTL_AFD_SEND;
            break;
            
        default:
            ASSERT(FALSE);
            SocketStateUnlock(FCB);
            return;
    }

    switch (IoctlCode)
    {
        case IOCTL_AFD_RECV:
        case IOCTL_AFD_RECV_DATAGRAM:
        Function = FUNCTION_RECV;
        break;

        case IOCTL_AFD_SEND:
        case IOCTL_AFD_SEND_DATAGRAM:
        Function = FUNCTION_SEND;
        break;

        case IOCTL_AFD_CONNECT:
        Function = FUNCTION_CONNECT;
        break;

        case IOCTL_AFD_WAIT_FOR_LISTEN:
        Function = FUNCTION_PREACCEPT;
        break;

        case IOCTL_AFD_SELECT:
        KeAcquireSpinLock(&DeviceExt->Lock, &OldIrql);

        CurrentEntry = DeviceExt->Polls.Flink;
        while (CurrentEntry != &DeviceExt->Polls)
        {
            Poll = CONTAINING_RECORD(CurrentEntry, AFD_ACTIVE_POLL, ListEntry);

            if (Irp == Poll->Irp)
            {
                CleanupPendingIrp(FCB, Irp, IrpSp, Poll);
                KeReleaseSpinLock(&DeviceExt->Lock, OldIrql);
                SocketStateUnlock(FCB);
                return;
            }
            else
            {
                CurrentEntry = CurrentEntry->Flink;
            }
        }

        KeReleaseSpinLock(&DeviceExt->Lock, OldIrql);

        SocketStateUnlock(FCB);
            
        DbgPrint("WARNING!!! IRP cancellation race could lead to a process hang! (IOCTL_AFD_SELECT)\n");
        return;
            
        case IOCTL_AFD_DISCONNECT:
        Function = FUNCTION_DISCONNECT;
        break;
            
        default:
        ASSERT(FALSE);
        UnlockAndMaybeComplete(FCB, STATUS_CANCELLED, Irp, 0);
        return;
    }

    CurrentEntry = FCB->PendingIrpList[Function].Flink;
    while (CurrentEntry != &FCB->PendingIrpList[Function])
    {
        CurrentIrp = CONTAINING_RECORD(CurrentEntry, IRP, Tail.Overlay.ListEntry);

        if (CurrentIrp == Irp)
        {
            RemoveEntryList(CurrentEntry);
            CleanupPendingIrp(FCB, Irp, IrpSp, NULL);
            UnlockAndMaybeComplete(FCB, STATUS_CANCELLED, Irp, 0);
            return;
        }
        else
        {
            CurrentEntry = CurrentEntry->Flink;
        }
    }
    
    SocketStateUnlock(FCB);
    
    DbgPrint("WARNING!!! IRP cancellation race could lead to a process hang! (Function: %d)\n", Function);
}

static VOID NTAPI
AfdUnload(PDRIVER_OBJECT DriverObject)
{
}

NTSTATUS NTAPI
DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING wstrDeviceName = RTL_CONSTANT_STRING(L"\\Device\\Afd");
    PAFD_DEVICE_EXTENSION DeviceExt;
    NTSTATUS Status;

    /* register driver routines */
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = AfdDispatch;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = AfdDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = AfdDispatch;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = AfdDispatch;
    DriverObject->MajorFunction[IRP_MJ_READ] = AfdDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = AfdDispatch;
    DriverObject->DriverUnload = AfdUnload;

    Status = IoCreateDevice
	( DriverObject,
	  sizeof(AFD_DEVICE_EXTENSION),
	  &wstrDeviceName,
	  FILE_DEVICE_NAMED_PIPE,
	  0,
	  FALSE,
	  &DeviceObject );

    /* failure */
    if(!NT_SUCCESS(Status))
    {
	return (Status);
    }

    DeviceExt = DeviceObject->DeviceExtension;
    KeInitializeSpinLock( &DeviceExt->Lock );
    InitializeListHead( &DeviceExt->Polls );

    AFD_DbgPrint(MID_TRACE,("Device created: object %x ext %x\n",
			    DeviceObject, DeviceExt));

    return (Status);
}

/* EOF */
