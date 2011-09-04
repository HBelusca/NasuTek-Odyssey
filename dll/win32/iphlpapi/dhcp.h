/*
 * PROJECT:     Odyssey Networking
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        lib/iphlpapi/dhcp_odyssey.c
 * PURPOSE:     DHCP helper functions for Odyssey
 * COPYRIGHT:   Copyright 2006 Ge van Geldorp <gvg@odyssey.org>
 */

#ifndef WINE_DHCP_H_
#define WINE_DHCP_H_

DWORD getDhcpInfoForAdapter(DWORD AdapterIndex,
                            PBOOL DhcpEnabled,
                            PDWORD DhcpServer,
                            time_t *LeaseObtained,
                            time_t *LeaseExpires);

#endif /* ndef WINE_DHCP_H_ */
