/* Copyright (C) 2003 Art Yerkes
 * A reimplementation of ifenum.c by Juan Lang
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
 * This file is implemented on the IOCTL_TCP_QUERY_INFORMATION_EX ioctl on
 * tcpip.sys
 */

#include "iphlpapi_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(iphlpapi);

#ifndef TCPS_ESTABLISHED
# define TCPS_ESTABLISHED TCP_ESTABLISHED
#endif
#ifndef TCPS_SYN_SENT
# define TCPS_SYN_SENT TCP_SYN_SENT
#endif
#ifndef TCPS_SYN_RECEIVED
# define TCPS_SYN_RECEIVED TCP_SYN_RECV
#endif
#ifndef TCPS_FIN_WAIT_1
# define TCPS_FIN_WAIT_1 TCP_FIN_WAIT1
#endif
#ifndef TCPS_FIN_WAIT_2
# define TCPS_FIN_WAIT_2 TCP_FIN_WAIT2
#endif
#ifndef TCPS_TIME_WAIT
# define TCPS_TIME_WAIT TCP_TIME_WAIT
#endif
#ifndef TCPS_CLOSED
# define TCPS_CLOSED TCP_CLOSE
#endif
#ifndef TCPS_CLOSE_WAIT
# define TCPS_CLOSE_WAIT TCP_CLOSE_WAIT
#endif
#ifndef TCPS_LAST_ACK
# define TCPS_LAST_ACK TCP_LAST_ACK
#endif
#ifndef TCPS_LISTEN
# define TCPS_LISTEN TCP_LISTEN
#endif
#ifndef TCPS_CLOSING
# define TCPS_CLOSING TCP_CLOSING
#endif

BOOL isIpEntity( HANDLE tcpFile, TDIEntityID *ent ) {
    return (ent->tei_entity == CL_NL_ENTITY ||
            ent->tei_entity == CO_NL_ENTITY);
}

NTSTATUS getNthIpEntity( HANDLE tcpFile, DWORD index, TDIEntityID *ent ) {
    DWORD numEntities = 0;
    DWORD numRoutes = 0;
    TDIEntityID *entitySet = 0;
    NTSTATUS status = tdiGetEntityIDSet( tcpFile, &entitySet, &numEntities );
    int i;

    if( !NT_SUCCESS(status) )
        return status;

    for( i = 0; i < numEntities; i++ ) {
        if( isIpEntity( tcpFile, &entitySet[i] ) ) {
            TRACE("Entity %d is an IP Entity\n", i);
            if( numRoutes == index ) break;
            else numRoutes++;
        }
    }

    if( numRoutes == index && i < numEntities ) {
        TRACE("Index %d is entity #%d - %04x:%08x\n", index, i,
               entitySet[i].tei_entity, entitySet[i].tei_instance );
        memcpy( ent, &entitySet[i], sizeof(*ent) );
        tdiFreeThingSet( entitySet );
        return STATUS_SUCCESS;
    } else {
        tdiFreeThingSet( entitySet );
        return STATUS_UNSUCCESSFUL;
    }
}

NTSTATUS tdiGetMibForIpEntity
( HANDLE tcpFile, TDIEntityID *ent, IPSNMPInfo *entry ) {
    TCP_REQUEST_QUERY_INFORMATION_EX req = TCP_REQUEST_QUERY_INFORMATION_INIT;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD returnSize;

    memset( entry, 0, sizeof( *entry ) );

    TRACE("TdiGetMibForIpEntity(tcpFile %x,entityId %x)\n",
           (DWORD)tcpFile, ent->tei_instance);

    req.ID.toi_class                = INFO_CLASS_PROTOCOL;
    req.ID.toi_type                 = INFO_TYPE_PROVIDER;
    req.ID.toi_id                   = IP_MIB_STATS_ID;
    req.ID.toi_entity               = *ent;

    status = DeviceIoControl( tcpFile,
                              IOCTL_TCP_QUERY_INFORMATION_EX,
                              &req,
                              sizeof(req),
                              entry,
                              sizeof(*entry),
                              &returnSize,
                              NULL );

    TRACE("TdiGetMibForIpEntity() => {\n"
           "  ipsi_forwarding ............ %d\n"
           "  ipsi_defaultttl ............ %d\n"
           "  ipsi_inreceives ............ %d\n"
           "  ipsi_indelivers ............ %d\n"
           "  ipsi_outrequests ........... %d\n"
           "  ipsi_routingdiscards ....... %d\n"
           "  ipsi_outdiscards ........... %d\n"
           "  ipsi_outnoroutes ........... %d\n"
           "  ipsi_numif ................. %d\n"
           "  ipsi_numaddr ............... %d\n"
           "  ipsi_numroutes ............. %d\n"
           "} status %08x\n",
           entry->ipsi_forwarding,
           entry->ipsi_defaultttl,
           entry->ipsi_inreceives,
           entry->ipsi_indelivers,
           entry->ipsi_outrequests,
           entry->ipsi_routingdiscards,
           entry->ipsi_outdiscards,
           entry->ipsi_outnoroutes,
           entry->ipsi_numif,
           entry->ipsi_numaddr,
           entry->ipsi_numroutes,
           status);

    return status;
}

NTSTATUS tdiGetRoutesForIpEntity
( HANDLE tcpFile, TDIEntityID *ent, IPRouteEntry **routes, PDWORD numRoutes ) {
    NTSTATUS status = STATUS_SUCCESS;

    TRACE("TdiGetRoutesForIpEntity(tcpFile %x,entityId %x)\n",
           (DWORD)tcpFile, ent->tei_instance);

    status = tdiGetSetOfThings( tcpFile,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_PROVIDER,
                                IP_MIB_ARPTABLE_ENTRY_ID,
                                CL_NL_ENTITY,
				ent->tei_instance,
                                0,
                                sizeof(IPRouteEntry),
                                (PVOID *)routes,
                                numRoutes);

    return status;
}

NTSTATUS tdiGetIpAddrsForIpEntity
( HANDLE tcpFile, TDIEntityID *ent, IPAddrEntry **addrs, PDWORD numAddrs ) {
    NTSTATUS status;

    TRACE("TdiGetIpAddrsForIpEntity(tcpFile %x,entityId %x)\n",
           (DWORD)tcpFile, ent->tei_instance);

    status = tdiGetSetOfThings( tcpFile,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_PROVIDER,
                                IP_MIB_ADDRTABLE_ENTRY_ID,
                                CL_NL_ENTITY,
				ent->tei_instance,
                                0,
                                sizeof(IPAddrEntry),
                                (PVOID *)addrs,
                                numAddrs );

    return status;
}

DWORD getInterfaceStatsByName(const char *name, PMIB_IFROW entry)
{
  if (!name)
    return ERROR_INVALID_PARAMETER;
  if (!entry)
    return ERROR_INVALID_PARAMETER;

  return NO_ERROR;
}

DWORD getInterfaceStatsByIndex(DWORD index, PMIB_IFROW entry)
{
    return ERROR_INVALID_PARAMETER;
}

DWORD getICMPStats(MIB_ICMP *stats)
{
  FILE *fp;

  if (!stats)
    return ERROR_INVALID_PARAMETER;

  memset(stats, 0, sizeof(MIB_ICMP));
  /* get most of these stats from /proc/net/snmp, no error if can't */
  fp = fopen("/proc/net/snmp", "r");
  if (fp) {
    const char hdr[] = "Icmp:";
    char buf[512] = { 0 }, *ptr;

    do {
      ptr = fgets(buf, sizeof(buf), fp);
    } while (ptr && strncasecmp(buf, hdr, sizeof(hdr) - 1));
    if (ptr) {
      /* last line was a header, get another */
      ptr = fgets(buf, sizeof(buf), fp);
      if (ptr && strncasecmp(buf, hdr, sizeof(hdr) - 1) == 0) {
        char *endPtr;

        ptr += sizeof(hdr);
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwMsgs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwErrors = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwDestUnreachs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwTimeExcds = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwParmProbs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwSrcQuenchs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwRedirects = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwEchoReps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwTimestamps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwTimestampReps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwAddrMasks = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpInStats.dwAddrMaskReps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwMsgs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwErrors = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwDestUnreachs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwTimeExcds = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwParmProbs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwSrcQuenchs = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwRedirects = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwEchoReps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwTimestamps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwTimestampReps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwAddrMasks = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
        if (ptr && *ptr) {
          stats->stats.icmpOutStats.dwAddrMaskReps = strtoul(ptr, &endPtr, 10);
          ptr = endPtr;
        }
      }
    }
    fclose(fp);
  }
  return NO_ERROR;
}

DWORD getIPStats(PMIB_IPSTATS stats, DWORD family)
{
  if (!stats)
    return ERROR_INVALID_PARAMETER;
  return NO_ERROR;
}

DWORD getTCPStats(MIB_TCPSTATS *stats, DWORD family)
{
  if (!stats)
    return ERROR_INVALID_PARAMETER;
  return NO_ERROR;
}

DWORD getUDPStats(MIB_UDPSTATS *stats, DWORD family)
{
  if (!stats)
    return ERROR_INVALID_PARAMETER;
  return NO_ERROR;
}

static DWORD getNumWithOneHeader(const char *filename)
{
    return 0;
}

DWORD getNumRoutes(void)
{
    DWORD numEntities, numRoutes = 0;
    TDIEntityID *entitySet;
    HANDLE tcpFile;
    int i;
    NTSTATUS status;

    TRACE("called.\n");

    status = openTcpFile( &tcpFile );

    if( !NT_SUCCESS(status) ) {
        TRACE("failure: %08x\n", (int)status );
        return 0;
    }

    status = tdiGetEntityIDSet( tcpFile, &entitySet, &numEntities );

    if( !NT_SUCCESS(status) ) {
        TRACE("failure: %08x\n", (int)status );
        closeTcpFile( tcpFile );
        return 0;
    }

    for( i = 0; i < numEntities; i++ ) {
        if( isIpEntity( tcpFile, &entitySet[i] ) ) {
            IPSNMPInfo isnmp;
            memset( &isnmp, 0, sizeof( isnmp ) );
            status = tdiGetMibForIpEntity( tcpFile, &entitySet[i], &isnmp );
            if( !NT_SUCCESS(status) ) {
                tdiFreeThingSet( entitySet );
                closeTcpFile( tcpFile );
                return status;
            }
            numRoutes += isnmp.ipsi_numroutes;
        }
    }

    TRACE("numRoutes: %d\n", (int)numRoutes);

    closeTcpFile( tcpFile );

    return numRoutes;
}

VOID HexDump( PCHAR Data, DWORD Len ) {
    int i;

    for( i = 0; i < Len; i++ ) {
        if( !(i & 0xf) ) {
            if( i ) fprintf(stderr,"\n");
            fprintf(stderr,"%08x:", i);
        }
        fprintf( stderr, " %02x", Data[i] & 0xff );
    }
    fprintf(stderr,"\n");
}

RouteTable *getRouteTable(void)
{
    RouteTable *out_route_table;
    DWORD numRoutes = getNumRoutes(), routesAdded = 0;
    TDIEntityID ent;
    HANDLE tcpFile;
    NTSTATUS status = openTcpFile( &tcpFile );
    int i;

    if( !NT_SUCCESS(status) )
        return 0;

    TRACE("GETTING ROUTE TABLE\n");

    out_route_table = HeapAlloc( GetProcessHeap(), 0,
                                 sizeof(RouteTable) +
                                 (sizeof(RouteEntry) * (numRoutes - 1)) );
    if (!out_route_table) {
        closeTcpFile(tcpFile);
        return NULL;
    }

    out_route_table->numRoutes = numRoutes;

    for( i = 0; routesAdded < out_route_table->numRoutes; i++ ) {
        int j;
        IPRouteEntry *route_set;

        getNthIpEntity( tcpFile, i, &ent );

        tdiGetRoutesForIpEntity( tcpFile, &ent, &route_set, &numRoutes );
        
        if( !route_set ) {
            closeTcpFile( tcpFile );
            HeapFree( GetProcessHeap(), 0, out_route_table );
            return 0;
        }

        TRACE( "%d routes in instance %d\n", numRoutes, i );
#if 0
        HexDump( route_set,
                 sizeof( IPRouteEntry ) *
                 snmpInfo.ipsi_numroutes );
#endif

        for( j = 0; j < numRoutes; j++ ) {
            int routeNum = j + routesAdded;
            out_route_table->routes[routeNum].dest =
                route_set[j].ire_dest;
            out_route_table->routes[routeNum].mask =
                route_set[j].ire_mask;
            out_route_table->routes[routeNum].gateway =
                route_set[j].ire_gw;
            out_route_table->routes[routeNum].ifIndex =
                route_set[j].ire_index;
            out_route_table->routes[routeNum].metric =
                route_set[j].ire_metric1;
        }

        if( route_set ) tdiFreeThingSet( route_set );

        routesAdded += numRoutes;
    }

    closeTcpFile( tcpFile );

    TRACE("Return: %08x, %08x\n", status, out_route_table);

    return out_route_table;
}

DWORD getNumArpEntries(void)
{
    DWORD numEntities;
    TDIEntityID *entitySet = NULL;
    HANDLE tcpFile;
    int i, totalNumber = 0;
    NTSTATUS status;
    PMIB_IPNETROW IpArpTable = NULL;
    DWORD returnSize;

    TRACE("called.\n");

    status = openTcpFile( &tcpFile );

    if( !NT_SUCCESS(status) ) {
        TRACE("failure: %08x\n", (int)status );
        return 0;
    }

    status = tdiGetEntityIDSet( tcpFile, &entitySet, &numEntities );

    for( i = 0; i < numEntities; i++ ) {
        if( isInterface( &entitySet[i] ) &&
	    hasArp( tcpFile, &entitySet[i] ) ) {

	    status = tdiGetSetOfThings( tcpFile,
					INFO_CLASS_PROTOCOL,
					INFO_TYPE_PROVIDER,
					IP_MIB_ARPTABLE_ENTRY_ID,
					AT_ENTITY,
					entitySet[i].tei_instance,
					0,
					sizeof(MIB_IPNETROW),
					(PVOID *)&IpArpTable,
					&returnSize );

	    if( status == STATUS_SUCCESS ) totalNumber += returnSize;
		if( IpArpTable ) {
			tdiFreeThingSet( IpArpTable );
			IpArpTable = NULL;
		}
	}
    }

    closeTcpFile( tcpFile );
    if( IpArpTable ) tdiFreeThingSet( IpArpTable );
    if( entitySet ) tdiFreeThingSet( entitySet );
    return totalNumber;
}

PMIB_IPNETTABLE getArpTable(void)
{
    DWORD numEntities, returnSize;
    TDIEntityID *entitySet;
    HANDLE tcpFile;
    int i, totalNumber, TmpIdx, CurrIdx = 0;
    NTSTATUS status;
    PMIB_IPNETTABLE IpArpTable = NULL;
    PMIB_IPNETROW AdapterArpTable = NULL;

    TRACE("called.\n");

    totalNumber = getNumArpEntries();

    status = openTcpFile( &tcpFile );

    if( !NT_SUCCESS(status) ) {
        TRACE("failure: %08x\n", (int)status );
        return 0;
    }

    IpArpTable = HeapAlloc
	( GetProcessHeap(), 0,
	  sizeof(DWORD) + (sizeof(MIB_IPNETROW) * totalNumber) );
    if (!IpArpTable) {
        closeTcpFile(tcpFile);
        return NULL;
    }

    status = tdiGetEntityIDSet( tcpFile, &entitySet, &numEntities );

    for( i = 0; i < numEntities; i++ ) {
        if( isInterface( &entitySet[i] ) &&
	    hasArp( tcpFile, &entitySet[i] ) ) {

	    status = tdiGetSetOfThings( tcpFile,
					INFO_CLASS_PROTOCOL,
					INFO_TYPE_PROVIDER,
					IP_MIB_ARPTABLE_ENTRY_ID,
					AT_ENTITY,
					entitySet[i].tei_instance,
					0,
					sizeof(MIB_IPNETROW),
					(PVOID *)&AdapterArpTable,
					&returnSize );

	    if( status == STATUS_SUCCESS ) {
		for( TmpIdx = 0; TmpIdx < returnSize; TmpIdx++, CurrIdx++ )
		    IpArpTable->table[CurrIdx] = AdapterArpTable[TmpIdx];
	    }

	    if( AdapterArpTable ) tdiFreeThingSet( AdapterArpTable );
	}
    }

    closeTcpFile( tcpFile );

    tdiFreeThingSet( entitySet );
    IpArpTable->dwNumEntries = CurrIdx;

    return IpArpTable;
}

DWORD getNumUdpEntries(void)
{
  return getNumWithOneHeader("/proc/net/udp");
}

PMIB_UDPTABLE getUdpTable(void)
{
  DWORD numEntries = getNumUdpEntries();
  PMIB_UDPTABLE ret;

  ret = (PMIB_UDPTABLE)calloc(1, sizeof(MIB_UDPTABLE) +
   (numEntries - 1) * sizeof(MIB_UDPROW));
  if (ret) {
    FILE *fp;

    /* get from /proc/net/udp, no error if can't */
    fp = fopen("/proc/net/udp", "r");
    if (fp) {
      char buf[512] = { 0 }, *ptr;

      /* skip header line */
      ptr = fgets(buf, sizeof(buf), fp);
      while (ptr && ret->dwNumEntries < numEntries) {
        ptr = fgets(buf, sizeof(buf), fp);
        if (ptr) {
          char *endPtr;

          if (ptr && *ptr) {
            strtoul(ptr, &endPtr, 16); /* skip */
            ptr = endPtr;
          }
          if (ptr && *ptr) {
            ptr++;
            ret->table[ret->dwNumEntries].dwLocalAddr = strtoul(ptr, &endPtr,
             16);
            ptr = endPtr;
          }
          if (ptr && *ptr) {
            ptr++;
            ret->table[ret->dwNumEntries].dwLocalPort = strtoul(ptr, &endPtr,
             16);
            ptr = endPtr;
          }
          ret->dwNumEntries++;
        }
      }
      fclose(fp);
    }
  }
  return ret;
}

DWORD getNumTcpEntries(void)
{
  return getNumWithOneHeader("/proc/net/tcp");
}

PMIB_TCPTABLE getTcpTable(void)
{
    return 0;
}
