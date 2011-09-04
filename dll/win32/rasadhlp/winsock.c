/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey Winsock 2 SPI
 * FILE:        lib/mswsock/lib/init.c
 * PURPOSE:     DLL Initialization
 */

/* INCLUDES ******************************************************************/
#include "precomp.h"

/* DATA **********************************************************************/

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
BOOL
WINAPI
WSAttemptAutodialAddr(IN CONST SOCKADDR FAR *Name,
                      IN INT NameLength)
{
    PSOCKADDR_IN Ip = (PSOCKADDR_IN)Name;
    PSOCKADDR_NB NetBios = (PSOCKADDR_NB)Name;
    AUTODIAL_ADDR AutodialAdddress;

    /* Check the family type */
    switch (Name->sa_family)
    {
        case AF_INET:
            /* Normal IPv4, set the Autodial Address Data */
            AutodialAdddress.Family = AutoDialIp;
            AutodialAdddress.Ip4Address = Ip->sin_addr;
            break;

        case AF_NETBIOS:
            /* NetBIOS, set the Autodial Address Data*/
            AutodialAdddress.Family = AutoDialNetBios;
            RtlCopyMemory(&AutodialAdddress.NetBiosAddress,
                          NetBios->snb_name,
                          NETBIOS_NAME_LENGTH);
            break;

        default:
            /* Unsupported family type */
            return FALSE;
    }

    /* Call the public routine */
    return AcsHlpAttemptConnection(&AutodialAdddress);
}

/*
 * @implemented
 */
BOOL
WINAPI
WSAttemptAutodialName(IN CONST LPWSAQUERYSETW Restrictions)
{
    AUTODIAL_ADDR AutodialAdddress;
    CHAR AnsiIp[17];
    LPGUID Guid = Restrictions->lpServiceClassId;

    /* Make sure we actually have a name */
    if (!Restrictions->lpszServiceInstanceName) return FALSE;

    /* Check if this is the Hostname GUID */
    if (!memcmp(Guid, &HostnameGuid, sizeof(GUID)))
    {
        /* It is. Set up the Autodial Address Data */
        AutodialAdddress.Family = AutoDialIpHost;
        WideCharToMultiByte(CP_ACP,
                            0,
                            Restrictions->lpszServiceInstanceName,
                            -1,
                            AutodialAdddress.HostName,
                            INTERNET_MAX_PATH_LENGTH - 1,
                            0,
                            0);

        /* Call the public routine */
        return AcsHlpAttemptConnection(&AutodialAdddress);
    }
    else if (!memcmp(Guid, &AddressGuid, sizeof(GUID)))
    {
        /* It's actually the IP String GUID */
        AutodialAdddress.Family = AutoDialIp;

        /* Convert the IP String to ANSI and then convert it to IP */
        WideCharToMultiByte(CP_ACP,
                            0,
                            Restrictions->lpszServiceInstanceName,
                            -1,
                            AnsiIp,
                            sizeof(AnsiIp) - 1,
                            0,
                            0);
        _strlwr(AnsiIp);
        AutodialAdddress.Ip4Address.S_un.S_addr = inet_addr(AnsiIp);

        /* Make sure the IP is valid */
        if (AutodialAdddress.Ip4Address.S_un.S_addr == -1) return FALSE;

        /* Call the public routine */
        return AcsHlpAttemptConnection(&AutodialAdddress);
    }
    else
    {
        /* Unknown GUID type */
        return FALSE;
    }
}

/*
 * @implemented
 */
VOID
WINAPI
WSNoteSuccessfulHostentLookup(IN CONST CHAR FAR *Name,
                              IN CONST ULONG Address)
{
    AUTODIAL_ADDR AutodialAdddress;
    AUTODIAL_CONN AutodialConnection;

    /* Make sure there actually is a name */
    if (!(Name) || !strlen(Name)) return;

    /* Setup the Address */
    AutodialAdddress.Family = AutoDialIpHost;
    strcpy(AutodialAdddress.HostName, Name);

    /* Setup the new connection */
    AutodialConnection.Family = ConnectionIp;
    AutodialConnection.Ip4Address = Address;
    AcsHlpNoteNewConnection(&AutodialAdddress, &AutodialConnection);
}

