/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey WinSock 2 API
 * FILE:        spinstal.c
 * PURPOSE:     Transport Service Provider Installation
 * PROGRAMMER:  Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/
#include "ws2_32.h"

//#define NDEBUG
#include <debug.h>

/* DATA **********************************************************************/

/* FUNCTIONS *****************************************************************/

/*
 * @unimplemented
 */
INT
WSPAPI
WSCInstallProvider(IN LPGUID lpProviderId,
                   IN CONST WCHAR * lpszProviderDllPath,
                   IN CONST LPWSAPROTOCOL_INFOW lpProtocolInfoList,
                   IN DWORD dwNumberOfEntries,
                   OUT LPINT lpErrno)
{
    UNIMPLEMENTED;
    SetLastError(WSAEINVAL);
    return SOCKET_ERROR;
}

/*
 * @unimplemented
 */
INT
WSPAPI
WSCDeinstallProvider(IN LPGUID lpProviderId,
                     OUT LPINT lpErrno)
{
    UNIMPLEMENTED;
    SetLastError(WSAEINVAL);
    return SOCKET_ERROR;
}

/*
 * @unimplemented
 */
INT
WSPAPI
WSCWriteProviderOrder(IN LPDWORD lpwdCatalogEntryId,
                      IN DWORD dwNumberOfEntries)
{
    UNIMPLEMENTED;
    SetLastError(WSAEINVAL);
    return SOCKET_ERROR;
}
