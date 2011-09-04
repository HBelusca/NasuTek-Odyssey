/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TCP/IP protocol driver
 * FILE:        include/address.h
 * PURPOSE:     Address manipulation prototypes
 */

#pragma once

/*
 * Initialize an IPv4 style address
 * VOID AddrInitIPv4(
 *     PIP_ADDRESS IPAddress,
 *     IPv4_RAW_ADDRESS RawAddress)
 */
#define AddrInitIPv4(IPAddress, RawAddress)           \
{                                                     \
    (IPAddress)->Type                = IP_ADDRESS_V4; \
    (IPAddress)->Address.IPv4Address = (RawAddress);  \
}

#if DBG

PCHAR A2S(
    PIP_ADDRESS Address);

#endif /* DBG */

VOID IPAddressFree(
    PVOID Object);

BOOLEAN AddrIsUnspecified(
    PIP_ADDRESS Address);

NTSTATUS AddrGetAddress(
    PTRANSPORT_ADDRESS AddrList,
    PIP_ADDRESS Address,
    PUSHORT Port);

NTSTATUS AddrBuildAddress(
    PTRANSPORT_ADDRESS TdiAddress,
    PIP_ADDRESS Address,
    PUSHORT Port);

BOOLEAN AddrIsEqual(
    PIP_ADDRESS Address1,
    PIP_ADDRESS Address2);

INT AddrCompare(
    PIP_ADDRESS Address1,
    PIP_ADDRESS Address2);

BOOLEAN AddrIsEqualIPv4(
    PIP_ADDRESS Address1,
    IPv4_RAW_ADDRESS Address2);

PIP_INTERFACE AddrLocateInterface(
    PIP_ADDRESS MatchAddress);

PADDRESS_FILE AddrSearchFirst(
    PIP_ADDRESS Address,
    USHORT Port,
    USHORT Protocol,
    PAF_SEARCH SearchContext);

PADDRESS_FILE AddrSearchNext(
    PAF_SEARCH SearchContext);

unsigned long NTAPI inet_addr(const char*);

ULONG IPv4NToHl( ULONG Address );

UINT AddrCountPrefixBits( PIP_ADDRESS Netmask );

VOID AddrWidenAddress( PIP_ADDRESS Network, PIP_ADDRESS Source,
		       PIP_ADDRESS Netmask );

/* EOF */
