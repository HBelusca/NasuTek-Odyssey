/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TCP/IP protocol driver
 * FILE:        transport/tcp/accept.c
 * PURPOSE:     Transmission Control Protocol Listen/Accept code
 * PROGRAMMERS: Art Yerkes (arty@users.sf.net)
 * REVISIONS:
 *   arty 12/21/2004 Created
 */

#include "precomp.h"

#include "rosip.h"

extern NPAGED_LOOKASIDE_LIST TdiBucketLookasideList;

NTSTATUS TCPCheckPeerForAccept(PVOID Context,
                               PTDI_REQUEST_KERNEL Request)
{
    struct tcp_pcb *newpcb = (struct tcp_pcb*)Context;
    NTSTATUS Status;
    PTDI_CONNECTION_INFORMATION WhoIsConnecting;
    PTA_IP_ADDRESS RemoteAddress;
    struct ip_addr ipaddr;
    
    if (Request->RequestFlags & TDI_QUERY_ACCEPT)
        DbgPrint("TDI_QUERY_ACCEPT NOT SUPPORTED!!!\n");

    WhoIsConnecting = (PTDI_CONNECTION_INFORMATION)Request->ReturnConnectionInformation;
    RemoteAddress = (PTA_IP_ADDRESS)WhoIsConnecting->RemoteAddress;
    
    RemoteAddress->TAAddressCount = 1;
    RemoteAddress->Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;
    RemoteAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
    
    Status = TCPTranslateError(LibTCPGetPeerName(newpcb,
                                                 &ipaddr,
                                                 &RemoteAddress->Address[0].Address[0].sin_port));
    
    RemoteAddress->Address[0].Address[0].in_addr = ipaddr.addr;
    
    return Status;
}

/* This listen is on a socket we keep as internal.  That socket has the same
 * lifetime as the address file */
NTSTATUS TCPListen(PCONNECTION_ENDPOINT Connection, UINT Backlog)
{
    NTSTATUS Status = STATUS_SUCCESS;
    struct ip_addr AddressToBind;
    KIRQL OldIrql;
    TA_IP_ADDRESS LocalAddress;

    ASSERT(Connection);

    LockObject(Connection, &OldIrql);

    ASSERT_KM_POINTER(Connection->AddressFile);

    TI_DbgPrint(DEBUG_TCP,("[IP, TCPListen] Called\n"));

    TI_DbgPrint(DEBUG_TCP, ("Connection->SocketContext %x\n",
        Connection->SocketContext));
    
    AddressToBind.addr = Connection->AddressFile->Address.Address.IPv4Address;

    Status = TCPTranslateError(LibTCPBind(Connection,
                                          &AddressToBind,
                                          Connection->AddressFile->Port));

    if (NT_SUCCESS(Status))
    {
        /* Check if we had an unspecified port */
        if (!Connection->AddressFile->Port)
        {
            /* We did, so we need to copy back the port */
            Status = TCPGetSockAddress(Connection, (PTRANSPORT_ADDRESS)&LocalAddress, FALSE);
            if (NT_SUCCESS(Status))
            {
                /* Allocate the port in the port bitmap */
                Connection->AddressFile->Port = TCPAllocatePort(LocalAddress.Address[0].Address[0].sin_port);
                
                /* This should never fail */
                ASSERT(Connection->AddressFile->Port != 0xFFFF);
            }
        }
    }

    if (NT_SUCCESS(Status))
    {
        Connection->SocketContext = LibTCPListen(Connection, Backlog);
        if (!Connection->SocketContext)
            Status = STATUS_UNSUCCESSFUL;
    }

    UnlockObject(Connection, OldIrql);

    TI_DbgPrint(DEBUG_TCP,("[IP, TCPListen] Leaving. Status = %x\n", Status));

    return Status;
}

BOOLEAN TCPAbortListenForSocket
(   PCONNECTION_ENDPOINT Listener,
    PCONNECTION_ENDPOINT Connection)
{
    PLIST_ENTRY ListEntry;
    PTDI_BUCKET Bucket;
    KIRQL OldIrql;
    BOOLEAN Found = FALSE;

    LockObject(Listener, &OldIrql);

    ListEntry = Listener->ListenRequest.Flink;
    while (ListEntry != &Listener->ListenRequest)
    {
        Bucket = CONTAINING_RECORD(ListEntry, TDI_BUCKET, Entry);

        if (Bucket->AssociatedEndpoint == Connection)
        {
            DereferenceObject(Bucket->AssociatedEndpoint);
            RemoveEntryList( &Bucket->Entry );
            ExFreeToNPagedLookasideList(&TdiBucketLookasideList, Bucket);
            Found = TRUE;
            break;
        }

        ListEntry = ListEntry->Flink;
    }

    UnlockObject(Listener, OldIrql);

    return Found;
}

NTSTATUS TCPAccept ( PTDI_REQUEST Request,
                     PCONNECTION_ENDPOINT Listener,
                     PCONNECTION_ENDPOINT Connection,
                     PTCP_COMPLETION_ROUTINE Complete,
                     PVOID Context )
{
    NTSTATUS Status;
    PTDI_BUCKET Bucket;
    KIRQL OldIrql;

    LockObject(Listener, &OldIrql);

    Bucket = ExAllocateFromNPagedLookasideList(&TdiBucketLookasideList);
    
    if (Bucket)
    {
        Bucket->AssociatedEndpoint = Connection;
        ReferenceObject(Bucket->AssociatedEndpoint);

        Bucket->Request.RequestNotifyObject = Complete;
        Bucket->Request.RequestContext = Context;
        InsertTailList( &Listener->ListenRequest, &Bucket->Entry );
        Status = STATUS_PENDING;
    }
    else
        Status = STATUS_NO_MEMORY;

    UnlockObject(Listener, OldIrql);

    return Status;
}
