/*
 * PROJECT:     Odyssey Networking
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        lib/iphlpapi/dhcp_odyssey.c
 * PURPOSE:     DHCP helper functions for Odyssey
 * COPYRIGHT:   Copyright 2006 Ge van Geldorp <gvg@odyssey.org>
 */

#include "iphlpapi_private.h"

DWORD APIENTRY DhcpRosGetAdapterInfo(DWORD AdapterIndex,
                                     PBOOL DhcpEnabled,
                                     PDWORD DhcpServer,
                                     time_t *LeaseObtained,
                                     time_t *LeaseExpires);

DWORD getDhcpInfoForAdapter(DWORD AdapterIndex,
                            PBOOL DhcpEnabled,
                            PDWORD DhcpServer,
                            time_t *LeaseObtained,
                            time_t *LeaseExpires)
{
    DWORD Status, Version = 0;

    Status = DhcpCApiInitialize(&Version);
    if (Status != ERROR_SUCCESS)
    {
        /* We assume that the DHCP service isn't running yet */
        *DhcpEnabled = FALSE;
        *DhcpServer = htonl(INADDR_NONE);
        *LeaseObtained = 0;
        *LeaseExpires = 0;
        return ERROR_SUCCESS;
    }

    Status = DhcpRosGetAdapterInfo(AdapterIndex, DhcpEnabled, DhcpServer,
                                   LeaseObtained, LeaseExpires);

    DhcpCApiCleanup();

    return Status;
}
