/* $Id: $
 *
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * FILE:             subsys/system/dhcp/api.c
 * PURPOSE:          DHCP client api handlers
 * PROGRAMMER:       arty
 */

#include "rosdhcp.h"

#define NDEBUG
#include <odyssey/debug.h>

static CRITICAL_SECTION ApiCriticalSection;

VOID ApiInit() {
    InitializeCriticalSection( &ApiCriticalSection );
}

VOID ApiLock() {
    EnterCriticalSection( &ApiCriticalSection );
}

VOID ApiUnlock() {
    LeaveCriticalSection( &ApiCriticalSection );
}

VOID ApiFree() {
    DeleteCriticalSection( &ApiCriticalSection );
}

/* This represents the service portion of the DHCP client API */

DWORD DSLeaseIpAddress( PipeSendFunc Send, COMM_DHCP_REQ *Req ) {
    COMM_DHCP_REPLY Reply;
    PDHCP_ADAPTER Adapter;

    ApiLock();

    Adapter = AdapterFindIndex( Req->AdapterIndex );

    Reply.Reply = Adapter ? 1 : 0;

    if( Adapter ) {
        add_protocol( Adapter->DhclientInfo.name,
                      Adapter->DhclientInfo.rfdesc, got_one,
                      &Adapter->DhclientInfo );
	Adapter->DhclientInfo.client->state = S_INIT;
	state_reboot(&Adapter->DhclientInfo);
    }

    ApiUnlock();

    return Send( &Reply );
}

DWORD DSQueryHWInfo( PipeSendFunc Send, COMM_DHCP_REQ *Req ) {
    COMM_DHCP_REPLY Reply;
    PDHCP_ADAPTER Adapter;

    ApiLock();

    Adapter = AdapterFindIndex( Req->AdapterIndex );

    Reply.Reply = Adapter ? 1 : 0;

    if (Adapter) {
        Reply.QueryHWInfo.AdapterIndex = Req->AdapterIndex;
        Reply.QueryHWInfo.MediaType = Adapter->IfMib.dwType;
        Reply.QueryHWInfo.Mtu = Adapter->IfMib.dwMtu;
        Reply.QueryHWInfo.Speed = Adapter->IfMib.dwSpeed;
    }

    ApiUnlock();

    return Send( &Reply );
}

DWORD DSReleaseIpAddressLease( PipeSendFunc Send, COMM_DHCP_REQ *Req ) {
    COMM_DHCP_REPLY Reply;
    PDHCP_ADAPTER Adapter;
    struct protocol* proto;

    ApiLock();

    Adapter = AdapterFindIndex( Req->AdapterIndex );

    Reply.Reply = Adapter ? 1 : 0;

    if( Adapter ) {
        if (Adapter->NteContext)
            DeleteIPAddress( Adapter->NteContext );

        proto = find_protocol_by_adapter( &Adapter->DhclientInfo );
        if (proto)
           remove_protocol(proto);
    }

    ApiUnlock();

    return Send( &Reply );
}

DWORD DSRenewIpAddressLease( PipeSendFunc Send, COMM_DHCP_REQ *Req ) {
    COMM_DHCP_REPLY Reply;
    PDHCP_ADAPTER Adapter;

    ApiLock();

    Adapter = AdapterFindIndex( Req->AdapterIndex );

    if( !Adapter || Adapter->DhclientState.state == S_STATIC ) {
        Reply.Reply = 0;
        ApiUnlock();
        return Send( &Reply );
    }

    Reply.Reply = 1;

    add_protocol( Adapter->DhclientInfo.name,
                  Adapter->DhclientInfo.rfdesc, got_one,
                  &Adapter->DhclientInfo );
    Adapter->DhclientInfo.client->state = S_INIT;
    state_reboot(&Adapter->DhclientInfo);

    ApiUnlock();

    return Send( &Reply );
}

DWORD DSStaticRefreshParams( PipeSendFunc Send, COMM_DHCP_REQ *Req ) {
    NTSTATUS Status;
    COMM_DHCP_REPLY Reply;
    PDHCP_ADAPTER Adapter;
    struct protocol* proto;

    ApiLock();

    Adapter = AdapterFindIndex( Req->AdapterIndex );

    Reply.Reply = Adapter ? 1 : 0;

    if( Adapter ) {
        if (Adapter->NteContext)
            DeleteIPAddress( Adapter->NteContext );
        Adapter->DhclientState.state = S_STATIC;
        proto = find_protocol_by_adapter( &Adapter->DhclientInfo );
        if (proto)
            remove_protocol(proto);
        Status = AddIPAddress( Req->Body.StaticRefreshParams.IPAddress,
                               Req->Body.StaticRefreshParams.Netmask,
                               Req->AdapterIndex,
                               &Adapter->NteContext,
                               &Adapter->NteInstance );
        Reply.Reply = NT_SUCCESS(Status);
    }

    ApiUnlock();

    return Send( &Reply );
}

DWORD DSGetAdapterInfo( PipeSendFunc Send, COMM_DHCP_REQ *Req ) {
    COMM_DHCP_REPLY Reply;
    PDHCP_ADAPTER Adapter;

    ApiLock();

    Adapter = AdapterFindIndex( Req->AdapterIndex );

    Reply.Reply = Adapter ? 1 : 0;

    if( Adapter ) {
        Reply.GetAdapterInfo.DhcpEnabled = (S_STATIC != Adapter->DhclientState.state);
        if (S_BOUND == Adapter->DhclientState.state) {
            if (sizeof(Reply.GetAdapterInfo.DhcpServer) ==
                Adapter->DhclientState.active->serveraddress.len) {
                memcpy(&Reply.GetAdapterInfo.DhcpServer,
                       Adapter->DhclientState.active->serveraddress.iabuf,
                       Adapter->DhclientState.active->serveraddress.len);
            } else {
                DPRINT1("Unexpected server address len %d\n",
                        Adapter->DhclientState.active->serveraddress.len);
                Reply.GetAdapterInfo.DhcpServer = htonl(INADDR_NONE);
            }
            Reply.GetAdapterInfo.LeaseObtained = Adapter->DhclientState.active->obtained;
            Reply.GetAdapterInfo.LeaseExpires = Adapter->DhclientState.active->expiry;
        } else {
            Reply.GetAdapterInfo.DhcpServer = htonl(INADDR_NONE);
            Reply.GetAdapterInfo.LeaseObtained = 0;
            Reply.GetAdapterInfo.LeaseExpires = 0;
        }
    }

    ApiUnlock();

    return Send( &Reply );
}
