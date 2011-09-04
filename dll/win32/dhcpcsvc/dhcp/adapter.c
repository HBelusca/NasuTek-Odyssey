#include "rosdhcp.h"

static SOCKET DhcpSocket = INVALID_SOCKET;
static LIST_ENTRY AdapterList;
static WSADATA wsd;

PCHAR *GetSubkeyNames( PCHAR MainKeyName, PCHAR Append ) {
    int i = 0;
    DWORD Error;
    HKEY MainKey;
    PCHAR *Out, OutKeyName;
    DWORD CharTotal = 0, AppendLen = 1 + strlen(Append);
    DWORD MaxSubKeyLen = 0, MaxSubKeys = 0;

    Error = RegOpenKey( HKEY_LOCAL_MACHINE, MainKeyName, &MainKey );

    if( Error ) return NULL;

    Error = RegQueryInfoKey
        ( MainKey,
          NULL, NULL, NULL,
          &MaxSubKeys, &MaxSubKeyLen,
          NULL, NULL, NULL, NULL, NULL, NULL );

    DH_DbgPrint(MID_TRACE,("MaxSubKeys: %d, MaxSubKeyLen %d\n",
                           MaxSubKeys, MaxSubKeyLen));

    CharTotal = (sizeof(PCHAR) + MaxSubKeyLen + AppendLen) * (MaxSubKeys + 1);

    DH_DbgPrint(MID_TRACE,("AppendLen: %d, CharTotal: %d\n",
                           AppendLen, CharTotal));

    Out = (CHAR**) malloc( CharTotal );
    OutKeyName = ((PCHAR)&Out[MaxSubKeys+1]);

    if( !Out ) { RegCloseKey( MainKey ); return NULL; }

    i = 0;
    do {
        Out[i] = OutKeyName;
        Error = RegEnumKey( MainKey, i, OutKeyName, MaxSubKeyLen );
        if( !Error ) {
            strcat( OutKeyName, Append );
            DH_DbgPrint(MID_TRACE,("[%d]: %s\n", i, OutKeyName));
            OutKeyName += strlen(OutKeyName) + 1;
            i++;
        } else Out[i] = 0;
    } while( Error == ERROR_SUCCESS );

    RegCloseKey( MainKey );

    return Out;
}

PCHAR RegReadString( HKEY Root, PCHAR Subkey, PCHAR Value ) {
    PCHAR SubOut = NULL;
    DWORD SubOutLen = 0, Error = 0;
    HKEY  ValueKey = NULL;

    DH_DbgPrint(MID_TRACE,("Looking in %x:%s:%s\n", Root, Subkey, Value ));

    if( Subkey && strlen(Subkey) ) {
        if( RegOpenKey( Root, Subkey, &ValueKey ) != ERROR_SUCCESS )
            goto regerror;
    } else ValueKey = Root;

    DH_DbgPrint(MID_TRACE,("Got Key %x\n", ValueKey));

    if( (Error = RegQueryValueEx( ValueKey, Value, NULL, NULL,
                                  (LPBYTE)SubOut, &SubOutLen )) != ERROR_SUCCESS )
        goto regerror;

    DH_DbgPrint(MID_TRACE,("Value %s has size %d\n", Value, SubOutLen));

    if( !(SubOut = (CHAR*) malloc(SubOutLen)) )
        goto regerror;

    if( (Error = RegQueryValueEx( ValueKey, Value, NULL, NULL,
                                  (LPBYTE)SubOut, &SubOutLen )) != ERROR_SUCCESS )
        goto regerror;

    DH_DbgPrint(MID_TRACE,("Value %s is %s\n", Value, SubOut));

    goto cleanup;

regerror:
    if( SubOut ) { free( SubOut ); SubOut = NULL; }
cleanup:
    if( ValueKey && ValueKey != Root ) {
        DH_DbgPrint(MID_TRACE,("Closing key %x\n", ValueKey));
        RegCloseKey( ValueKey );
    }

    DH_DbgPrint(MID_TRACE,("Returning %x with error %d\n", SubOut, Error));

    return SubOut;
}

HKEY FindAdapterKey( PDHCP_ADAPTER Adapter ) {
    int i = 0;
    PCHAR EnumKeyName =
        "SYSTEM\\CurrentControlSet\\Control\\Class\\"
        "{4D36E972-E325-11CE-BFC1-08002BE10318}";
    PCHAR TargetKeyNameStart =
        "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\";
    PCHAR TargetKeyName = NULL;
    PCHAR *EnumKeysLinkage = GetSubkeyNames( EnumKeyName, "\\Linkage" );
    PCHAR *EnumKeysTop     = GetSubkeyNames( EnumKeyName, "" );
    PCHAR RootDevice = NULL;
    HKEY EnumKey, OutKey = NULL;
    DWORD Error = ERROR_SUCCESS;

    if( !EnumKeysLinkage || !EnumKeysTop ) goto cleanup;

    Error = RegOpenKey( HKEY_LOCAL_MACHINE, EnumKeyName, &EnumKey );

    if( Error ) goto cleanup;

    for( i = 0; EnumKeysLinkage[i]; i++ ) {
        RootDevice = RegReadString
            ( EnumKey, EnumKeysLinkage[i], "RootDevice" );

        if( RootDevice &&
            !strcmp( RootDevice, Adapter->DhclientInfo.name ) ) {
            TargetKeyName =
                (CHAR*) malloc( strlen( TargetKeyNameStart ) +
                        strlen( RootDevice ) + 1);
            if( !TargetKeyName ) goto cleanup;
            sprintf( TargetKeyName, "%s%s",
                     TargetKeyNameStart, RootDevice );
            Error = RegCreateKeyExA( HKEY_LOCAL_MACHINE, TargetKeyName, 0, NULL, 0, KEY_READ, NULL, &OutKey, NULL );
            break;
        } else {
            free( RootDevice ); RootDevice = 0;
        }
    }

cleanup:
    if( RootDevice ) free( RootDevice );
    if( EnumKeysLinkage ) free( EnumKeysLinkage );
    if( EnumKeysTop ) free( EnumKeysTop );
    if( TargetKeyName ) free( TargetKeyName );

    return OutKey;
}

BOOL PrepareAdapterForService( PDHCP_ADAPTER Adapter ) {
    HKEY AdapterKey;
    DWORD Error = ERROR_SUCCESS, DhcpEnabled, Length = sizeof(DWORD);

    Adapter->DhclientState.config = &Adapter->DhclientConfig;
    strncpy(Adapter->DhclientInfo.name, (char*)Adapter->IfMib.bDescr,
            sizeof(Adapter->DhclientInfo.name));

    AdapterKey = FindAdapterKey( Adapter );
    if( AdapterKey )
    {
        Error = RegQueryValueEx(AdapterKey, "EnableDHCP", NULL, NULL, (LPBYTE)&DhcpEnabled, &Length);

        if (Error != ERROR_SUCCESS || Length != sizeof(DWORD))
            DhcpEnabled = 1;

        CloseHandle(AdapterKey);
    }
    else
    {
        /* DHCP enabled by default */
        DhcpEnabled = 1;
    }           

    if( !DhcpEnabled ) {
        /* Non-automatic case */
        DbgPrint("DHCPCSVC: Adapter Name: [%s] (static)\n", Adapter->DhclientInfo.name);

        Adapter->DhclientState.state = S_STATIC;
    } else {
        /* Automatic case */
        DbgPrint("DHCPCSVC: Adapter Name: [%s] (dynamic)\n", Adapter->DhclientInfo.name);

	Adapter->DhclientInfo.client->state = S_INIT;
    }

    return TRUE;
}

void AdapterInit() {
    WSAStartup(0x0101,&wsd);

    InitializeListHead( &AdapterList );
}

int
InterfaceConnected(MIB_IFROW IfEntry)
{
    if (IfEntry.dwOperStatus == IF_OPER_STATUS_CONNECTED ||
        IfEntry.dwOperStatus == IF_OPER_STATUS_OPERATIONAL)
        return 1;

    DH_DbgPrint(MID_TRACE,("Interface %d is down\n", IfEntry.dwIndex));
    return 0;
}

/*
 * XXX Figure out the way to bind a specific adapter to a socket.
 */
DWORD WINAPI AdapterDiscoveryThread(LPVOID Context) {
    PMIB_IFTABLE Table = (PMIB_IFTABLE) malloc(sizeof(MIB_IFTABLE));
    DWORD Error, Size = sizeof(MIB_IFTABLE);
    PDHCP_ADAPTER Adapter = NULL;
    HANDLE AdapterStateChangedEvent = (HANDLE)Context;
    struct interface_info *ifi = NULL;
    int i, AdapterCount = 0, Broadcast;

    /* FIXME: Kill this thread when the service is stopped */

    do {
       DH_DbgPrint(MID_TRACE,("Getting Adapter List...\n"));

       while( (Error = GetIfTable(Table, &Size, 0 )) ==
               ERROR_INSUFFICIENT_BUFFER ) {
           DH_DbgPrint(MID_TRACE,("Error %d, New Buffer Size: %d\n", Error, Size));
           free( Table );
           Table = (PMIB_IFTABLE) malloc( Size );
       }

       if( Error != NO_ERROR )
       {
           /* HACK: We are waiting until TCP/IP starts */
           Sleep(2000);
           continue;
       }

       DH_DbgPrint(MID_TRACE,("Got Adapter List (%d entries)\n", Table->dwNumEntries));

       for( i = Table->dwNumEntries - 1; i >= 0; i-- ) {
            DH_DbgPrint(MID_TRACE,("Getting adapter %d attributes\n",
                                   Table->table[i].dwIndex));

            ApiLock();

            if ((Adapter = AdapterFindByHardwareAddress(Table->table[i].bPhysAddr, Table->table[i].dwPhysAddrLen)))
            {
                /* This is an existing adapter */
                if (InterfaceConnected(Table->table[i])) {
                    /* We're still active so we stay in the list */
                    ifi = &Adapter->DhclientInfo;
                } else {
                    /* We've lost our link so out we go */
                    RemoveEntryList(&Adapter->ListEntry);
                    free(Adapter);
                }

                ApiUnlock();

                continue;
            }

            ApiUnlock();

            Adapter = (DHCP_ADAPTER*) calloc( sizeof( DHCP_ADAPTER ) + Table->table[i].dwMtu, 1 );

            if( Adapter && Table->table[i].dwType == MIB_IF_TYPE_ETHERNET && InterfaceConnected(Table->table[i])) {
                memcpy( &Adapter->IfMib, &Table->table[i],
                        sizeof(Adapter->IfMib) );
                Adapter->DhclientInfo.client = &Adapter->DhclientState;
                Adapter->DhclientInfo.rbuf = Adapter->recv_buf;
                Adapter->DhclientInfo.rbuf_max = Table->table[i].dwMtu;
                Adapter->DhclientInfo.rbuf_len =
                    Adapter->DhclientInfo.rbuf_offset = 0;
                memcpy(Adapter->DhclientInfo.hw_address.haddr,
                       Adapter->IfMib.bPhysAddr,
                       Adapter->IfMib.dwPhysAddrLen);
                Adapter->DhclientInfo.hw_address.hlen = Adapter->IfMib.dwPhysAddrLen;

                /* I'm not sure where else to set this, but
                   some DHCP servers won't take a zero.
                   We checked the hardware type earlier in
                   the if statement. */
                Adapter->DhclientInfo.hw_address.htype = HTYPE_ETHER;

                if( DhcpSocket == INVALID_SOCKET ) {
                    DhcpSocket =
                        Adapter->DhclientInfo.rfdesc =
                        Adapter->DhclientInfo.wfdesc =
                        socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

                    if (DhcpSocket != INVALID_SOCKET) {
						
						/* Allow broadcast on this socket */
						Broadcast = 1;
						setsockopt(DhcpSocket,
								   SOL_SOCKET,
								   SO_BROADCAST,
								   (const char *)&Broadcast,
								   sizeof(Broadcast));
						
                        Adapter->ListenAddr.sin_family = AF_INET;
                        Adapter->ListenAddr.sin_port = htons(LOCAL_PORT);
                        Adapter->BindStatus =
                             (bind( Adapter->DhclientInfo.rfdesc,
                                    (struct sockaddr *)&Adapter->ListenAddr,
                                    sizeof(Adapter->ListenAddr) ) == 0) ?
                             0 : WSAGetLastError();
                    } else {
                        error("socket() failed: %d\n", WSAGetLastError());
                    }
                } else {
                    Adapter->DhclientInfo.rfdesc =
                        Adapter->DhclientInfo.wfdesc = DhcpSocket;
                }

                Adapter->DhclientConfig.timeout = DHCP_PANIC_TIMEOUT;
                Adapter->DhclientConfig.initial_interval = DHCP_DISCOVER_INTERVAL;
                Adapter->DhclientConfig.retry_interval = DHCP_DISCOVER_INTERVAL;
                Adapter->DhclientConfig.select_interval = 1;
                Adapter->DhclientConfig.reboot_timeout = DHCP_REBOOT_TIMEOUT;
                Adapter->DhclientConfig.backoff_cutoff = DHCP_BACKOFF_MAX;
                Adapter->DhclientState.interval =
                    Adapter->DhclientConfig.retry_interval;

                if( PrepareAdapterForService( Adapter ) ) {
                    Adapter->DhclientInfo.next = ifi;
                    ifi = &Adapter->DhclientInfo;

                    read_client_conf(&Adapter->DhclientInfo);

                    if (Adapter->DhclientInfo.client->state == S_INIT)
                    {
                        add_protocol(Adapter->DhclientInfo.name,
                                     Adapter->DhclientInfo.rfdesc,
                                     got_one, &Adapter->DhclientInfo);

	                state_init(&Adapter->DhclientInfo);
                    }

                    ApiLock();
                    InsertTailList( &AdapterList, &Adapter->ListEntry );
                    AdapterCount++;
                    SetEvent(AdapterStateChangedEvent);
                    ApiUnlock();
                } else { free( Adapter ); Adapter = 0; }
            } else { free( Adapter ); Adapter = 0; }

            if( !Adapter )
                DH_DbgPrint(MID_TRACE,("Adapter %d was rejected\n",
                                       Table->table[i].dwIndex));
        }
        Error = NotifyAddrChange(NULL, NULL);
#if 0
        if (Error != NO_ERROR)
            break;
#else
        if (AdapterCount)
            break;
        else
            Sleep(3000);
#endif
    } while (TRUE);

    DbgPrint("DHCPCSVC: Adapter discovery thread is terminating! (Error: %d)\n", Error);

    if( Table ) free( Table );
    return Error;
}

HANDLE StartAdapterDiscovery(VOID) {
    HANDLE /* ThreadHandle, */ EventHandle;

    EventHandle = CreateEvent(NULL,
                              FALSE,
                              FALSE,
                              NULL);

#if 0
    ThreadHandle = CreateThread(NULL,
                                0,
                                AdapterDiscoveryThread,
                                (LPVOID)EventHandle,
                                0,
                                NULL);

    if (ThreadHandle == NULL)
        return NULL;

    CloseHandle(ThreadHandle);
#else
    AdapterDiscoveryThread((LPVOID)EventHandle);
#endif

    return EventHandle;
}

void AdapterStop() {
    PLIST_ENTRY ListEntry;
    PDHCP_ADAPTER Adapter;
    ApiLock();
    while( !IsListEmpty( &AdapterList ) ) {
        ListEntry = (PLIST_ENTRY)RemoveHeadList( &AdapterList );
        Adapter = CONTAINING_RECORD( ListEntry, DHCP_ADAPTER, ListEntry );
        free( Adapter );
    }
    ApiUnlock();
    WSACleanup();
}

PDHCP_ADAPTER AdapterFindIndex( unsigned int indx ) {
    PDHCP_ADAPTER Adapter;
    PLIST_ENTRY ListEntry;

    for( ListEntry = AdapterList.Flink;
         ListEntry != &AdapterList;
         ListEntry = ListEntry->Flink ) {
        Adapter = CONTAINING_RECORD( ListEntry, DHCP_ADAPTER, ListEntry );
        if( Adapter->IfMib.dwIndex == indx ) return Adapter;
    }

    return NULL;
}

PDHCP_ADAPTER AdapterFindName( const WCHAR *name ) {
    PDHCP_ADAPTER Adapter;
    PLIST_ENTRY ListEntry;

    for( ListEntry = AdapterList.Flink;
         ListEntry != &AdapterList;
         ListEntry = ListEntry->Flink ) {
        Adapter = CONTAINING_RECORD( ListEntry, DHCP_ADAPTER, ListEntry );
        if( !wcsicmp( Adapter->IfMib.wszName, name ) ) return Adapter;
    }

    return NULL;
}

PDHCP_ADAPTER AdapterFindInfo( struct interface_info *ip ) {
    PDHCP_ADAPTER Adapter;
    PLIST_ENTRY ListEntry;

    for( ListEntry = AdapterList.Flink;
         ListEntry != &AdapterList;
         ListEntry = ListEntry->Flink ) {
        Adapter = CONTAINING_RECORD( ListEntry, DHCP_ADAPTER, ListEntry );
        if( ip == &Adapter->DhclientInfo ) return Adapter;
    }

    return NULL;
}

PDHCP_ADAPTER AdapterFindByHardwareAddress( u_int8_t haddr[16], u_int8_t hlen ) {
    PDHCP_ADAPTER Adapter;
    PLIST_ENTRY ListEntry;

    for(ListEntry = AdapterList.Flink;
        ListEntry != &AdapterList;
        ListEntry = ListEntry->Flink) {
       Adapter = CONTAINING_RECORD( ListEntry, DHCP_ADAPTER, ListEntry );
       if (Adapter->DhclientInfo.hw_address.hlen == hlen &&
           !memcmp(Adapter->DhclientInfo.hw_address.haddr,
                  haddr,
                  hlen)) return Adapter;
    }

    return NULL;
}

PDHCP_ADAPTER AdapterGetFirst() {
    if( IsListEmpty( &AdapterList ) ) return NULL; else {
        return CONTAINING_RECORD
            ( AdapterList.Flink, DHCP_ADAPTER, ListEntry );
    }
}

PDHCP_ADAPTER AdapterGetNext( PDHCP_ADAPTER This )
{
    if( This->ListEntry.Flink == &AdapterList ) return NULL;
    return CONTAINING_RECORD
        ( This->ListEntry.Flink, DHCP_ADAPTER, ListEntry );
}

void if_register_send(struct interface_info *ip) {

}

void if_register_receive(struct interface_info *ip) {
}
