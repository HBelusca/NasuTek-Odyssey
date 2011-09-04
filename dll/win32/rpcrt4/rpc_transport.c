/*
 * RPC transport layer
 *
 * Copyright 2001 Ove Kåven, TransGaming Technologies
 * Copyright 2003 Mike Hearn
 * Copyright 2004 Filip Navara
 * Copyright 2006 Mike McCormack
 * Copyright 2006 Damjan Jovanovic
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(__MINGW32__) || defined (_MSC_VER)
# include <ws2tcpip.h>
# ifndef EADDRINUSE
#  define EADDRINUSE WSAEADDRINUSE
# endif
# ifndef EAGAIN
#  define EAGAIN WSAEWOULDBLOCK
# endif
# undef errno
# define errno WSAGetLastError()
#else
# include <errno.h>
# ifdef HAVE_UNISTD_H
#  include <unistd.h>
# endif
# include <fcntl.h>
# ifdef HAVE_SYS_SOCKET_H
#  include <sys/socket.h>
# endif
# ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
# endif
# ifdef HAVE_NETINET_TCP_H
#  include <netinet/tcp.h>
# endif
# ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
# endif
# ifdef HAVE_NETDB_H
#  include <netdb.h>
# endif
# ifdef HAVE_SYS_POLL_H
#  include <sys/poll.h>
# endif
# ifdef HAVE_SYS_FILIO_H
#  include <sys/filio.h>
# endif
# ifdef HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
# endif
# define closesocket close
# define ioctlsocket ioctl
#endif /* defined(__MINGW32__) || defined (_MSC_VER) */

#include "windef.h"
#include "winbase.h"
#include "winnls.h"
#include "winerror.h"
#include "wininet.h"
#include "winternl.h"
#include "wine/unicode.h"

#include "rpc.h"
#include "rpcndr.h"

#include "wine/debug.h"

#include "rpc_binding.h"
#include "rpc_assoc.h"
#include "rpc_message.h"
#include "rpc_server.h"
#include "epm_towers.h"

#ifndef SOL_TCP
# define SOL_TCP IPPROTO_TCP
#endif

#define DEFAULT_NCACN_HTTP_TIMEOUT (60 * 1000)

WINE_DEFAULT_DEBUG_CHANNEL(rpc);

static RPC_STATUS RPCRT4_SpawnConnection(RpcConnection** Connection, RpcConnection* OldConnection);

/**** ncacn_np support ****/

typedef struct _RpcConnection_np
{
  RpcConnection common;
  HANDLE pipe;
  OVERLAPPED ovl;
  BOOL listening;
} RpcConnection_np;

static RpcConnection *rpcrt4_conn_np_alloc(void)
{
  RpcConnection_np *npc = HeapAlloc(GetProcessHeap(), 0, sizeof(RpcConnection_np));
  if (npc)
  {
    npc->pipe = NULL;
    memset(&npc->ovl, 0, sizeof(npc->ovl));
    npc->listening = FALSE;
  }
  return &npc->common;
}

static RPC_STATUS rpcrt4_conn_listen_pipe(RpcConnection_np *npc)
{
  if (npc->listening)
    return RPC_S_OK;

  npc->listening = TRUE;
  for (;;)
  {
      if (ConnectNamedPipe(npc->pipe, &npc->ovl))
          return RPC_S_OK;

      switch(GetLastError())
      {
      case ERROR_PIPE_CONNECTED:
          SetEvent(npc->ovl.hEvent);
          return RPC_S_OK;
      case ERROR_IO_PENDING:
          /* will be completed in rpcrt4_protseq_np_wait_for_new_connection */
          return RPC_S_OK;
      case ERROR_NO_DATA_DETECTED:
          /* client has disconnected, retry */
          DisconnectNamedPipe( npc->pipe );
          break;
      default:
          npc->listening = FALSE;
          WARN("Couldn't ConnectNamedPipe (error was %d)\n", GetLastError());
          return RPC_S_OUT_OF_RESOURCES;
      }
  }
}

static RPC_STATUS rpcrt4_conn_create_pipe(RpcConnection *Connection, LPCSTR pname)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  TRACE("listening on %s\n", pname);

  npc->pipe = CreateNamedPipeA(pname, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                               PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                               PIPE_UNLIMITED_INSTANCES,
                               RPC_MAX_PACKET_SIZE, RPC_MAX_PACKET_SIZE, 5000, NULL);
  if (npc->pipe == INVALID_HANDLE_VALUE) {
    WARN("CreateNamedPipe failed with error %d\n", GetLastError());
    if (GetLastError() == ERROR_FILE_EXISTS)
      return RPC_S_DUPLICATE_ENDPOINT;
    else
      return RPC_S_CANT_CREATE_ENDPOINT;
  }

  memset(&npc->ovl, 0, sizeof(npc->ovl));
  npc->ovl.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

  /* Note: we don't call ConnectNamedPipe here because it must be done in the
   * server thread as the thread must be alertable */
  return RPC_S_OK;
}

static RPC_STATUS rpcrt4_conn_open_pipe(RpcConnection *Connection, LPCSTR pname, BOOL wait)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  HANDLE pipe;
  DWORD err, dwMode;

  TRACE("connecting to %s\n", pname);

  while (TRUE) {
    DWORD dwFlags = 0;
    if (Connection->QOS)
    {
        dwFlags = SECURITY_SQOS_PRESENT;
        switch (Connection->QOS->qos->ImpersonationType)
        {
            case RPC_C_IMP_LEVEL_DEFAULT:
                /* FIXME: what to do here? */
                break;
            case RPC_C_IMP_LEVEL_ANONYMOUS:
                dwFlags |= SECURITY_ANONYMOUS;
                break;
            case RPC_C_IMP_LEVEL_IDENTIFY:
                dwFlags |= SECURITY_IDENTIFICATION;
                break;
            case RPC_C_IMP_LEVEL_IMPERSONATE:
                dwFlags |= SECURITY_IMPERSONATION;
                break;
            case RPC_C_IMP_LEVEL_DELEGATE:
                dwFlags |= SECURITY_DELEGATION;
                break;
        }
        if (Connection->QOS->qos->IdentityTracking == RPC_C_QOS_IDENTITY_DYNAMIC)
            dwFlags |= SECURITY_CONTEXT_TRACKING;
    }
    pipe = CreateFileA(pname, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                       OPEN_EXISTING, dwFlags, 0);
    if (pipe != INVALID_HANDLE_VALUE) break;
    err = GetLastError();
    if (err == ERROR_PIPE_BUSY) {
      TRACE("connection failed, error=%x\n", err);
      return RPC_S_SERVER_TOO_BUSY;
    }
    if (!wait || !WaitNamedPipeA(pname, NMPWAIT_WAIT_FOREVER)) {
      err = GetLastError();
      WARN("connection failed, error=%x\n", err);
      return RPC_S_SERVER_UNAVAILABLE;
    }
  }

  /* success */
  memset(&npc->ovl, 0, sizeof(npc->ovl));
  /* pipe is connected; change to message-read mode. */
  dwMode = PIPE_READMODE_MESSAGE;
  SetNamedPipeHandleState(pipe, &dwMode, NULL, NULL);
  npc->ovl.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
  npc->pipe = pipe;

  return RPC_S_OK;
}

static RPC_STATUS rpcrt4_ncalrpc_open(RpcConnection* Connection)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  static const char prefix[] = "\\\\.\\pipe\\lrpc\\";
  RPC_STATUS r;
  LPSTR pname;

  /* already connected? */
  if (npc->pipe)
    return RPC_S_OK;

  /* protseq=ncalrpc: supposed to use NT LPC ports,
   * but we'll implement it with named pipes for now */
  pname = I_RpcAllocate(strlen(prefix) + strlen(Connection->Endpoint) + 1);
  strcat(strcpy(pname, prefix), Connection->Endpoint);
  r = rpcrt4_conn_open_pipe(Connection, pname, TRUE);
  I_RpcFree(pname);

  return r;
}

static RPC_STATUS rpcrt4_protseq_ncalrpc_open_endpoint(RpcServerProtseq* protseq, const char *endpoint)
{
  static const char prefix[] = "\\\\.\\pipe\\lrpc\\";
  RPC_STATUS r;
  LPSTR pname;
  RpcConnection *Connection;
  char generated_endpoint[22];

  if (!endpoint)
  {
    static LONG lrpc_nameless_id;
    DWORD process_id = GetCurrentProcessId();
    ULONG id = InterlockedIncrement(&lrpc_nameless_id);
    snprintf(generated_endpoint, sizeof(generated_endpoint),
             "LRPC%08x.%08x", process_id, id);
    endpoint = generated_endpoint;
  }

  r = RPCRT4_CreateConnection(&Connection, TRUE, protseq->Protseq, NULL,
                              endpoint, NULL, NULL, NULL);
  if (r != RPC_S_OK)
      return r;

  /* protseq=ncalrpc: supposed to use NT LPC ports,
   * but we'll implement it with named pipes for now */
  pname = I_RpcAllocate(strlen(prefix) + strlen(Connection->Endpoint) + 1);
  strcat(strcpy(pname, prefix), Connection->Endpoint);
  r = rpcrt4_conn_create_pipe(Connection, pname);
  I_RpcFree(pname);

  EnterCriticalSection(&protseq->cs);
  Connection->Next = protseq->conn;
  protseq->conn = Connection;
  LeaveCriticalSection(&protseq->cs);

  return r;
}

static RPC_STATUS rpcrt4_ncacn_np_open(RpcConnection* Connection)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  static const char prefix[] = "\\\\.";
  RPC_STATUS r;
  LPSTR pname;

  /* already connected? */
  if (npc->pipe)
    return RPC_S_OK;

  /* protseq=ncacn_np: named pipes */
  pname = I_RpcAllocate(strlen(prefix) + strlen(Connection->Endpoint) + 1);
  strcat(strcpy(pname, prefix), Connection->Endpoint);
  r = rpcrt4_conn_open_pipe(Connection, pname, TRUE);
  I_RpcFree(pname);

  return r;
}

static RPC_STATUS rpcrt4_protseq_ncacn_np_open_endpoint(RpcServerProtseq *protseq, const char *endpoint)
{
  static const char prefix[] = "\\\\.";
  RPC_STATUS r;
  LPSTR pname;
  RpcConnection *Connection;
  char generated_endpoint[21];

  if (!endpoint)
  {
    static LONG np_nameless_id;
    DWORD process_id = GetCurrentProcessId();
    ULONG id = InterlockedExchangeAdd(&np_nameless_id, 1 );
    snprintf(generated_endpoint, sizeof(generated_endpoint),
             "\\\\pipe\\\\%08x.%03x", process_id, id);
    endpoint = generated_endpoint;
  }

  r = RPCRT4_CreateConnection(&Connection, TRUE, protseq->Protseq, NULL,
                              endpoint, NULL, NULL, NULL);
  if (r != RPC_S_OK)
    return r;

  /* protseq=ncacn_np: named pipes */
  pname = I_RpcAllocate(strlen(prefix) + strlen(Connection->Endpoint) + 1);
  strcat(strcpy(pname, prefix), Connection->Endpoint);
  r = rpcrt4_conn_create_pipe(Connection, pname);
  I_RpcFree(pname);

  EnterCriticalSection(&protseq->cs);
  Connection->Next = protseq->conn;
  protseq->conn = Connection;
  LeaveCriticalSection(&protseq->cs);

  return r;
}

static void rpcrt4_conn_np_handoff(RpcConnection_np *old_npc, RpcConnection_np *new_npc)
{    
  /* because of the way named pipes work, we'll transfer the connected pipe
   * to the child, then reopen the server binding to continue listening */

  new_npc->pipe = old_npc->pipe;
  new_npc->ovl = old_npc->ovl;
  old_npc->pipe = 0;
  memset(&old_npc->ovl, 0, sizeof(old_npc->ovl));
  old_npc->listening = FALSE;
}

static RPC_STATUS rpcrt4_ncacn_np_handoff(RpcConnection *old_conn, RpcConnection *new_conn)
{
  RPC_STATUS status;
  LPSTR pname;
  static const char prefix[] = "\\\\.";

  rpcrt4_conn_np_handoff((RpcConnection_np *)old_conn, (RpcConnection_np *)new_conn);

  pname = I_RpcAllocate(strlen(prefix) + strlen(old_conn->Endpoint) + 1);
  strcat(strcpy(pname, prefix), old_conn->Endpoint);
  status = rpcrt4_conn_create_pipe(old_conn, pname);
  I_RpcFree(pname);

  return status;
}

static RPC_STATUS rpcrt4_ncalrpc_handoff(RpcConnection *old_conn, RpcConnection *new_conn)
{
  RPC_STATUS status;
  LPSTR pname;
  static const char prefix[] = "\\\\.\\pipe\\lrpc\\";

  TRACE("%s\n", old_conn->Endpoint);

  rpcrt4_conn_np_handoff((RpcConnection_np *)old_conn, (RpcConnection_np *)new_conn);

  pname = I_RpcAllocate(strlen(prefix) + strlen(old_conn->Endpoint) + 1);
  strcat(strcpy(pname, prefix), old_conn->Endpoint);
  status = rpcrt4_conn_create_pipe(old_conn, pname);
  I_RpcFree(pname);
    
  return status;
}

static int rpcrt4_conn_np_read(RpcConnection *Connection,
                        void *buffer, unsigned int count)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  char *buf = buffer;
  BOOL ret = TRUE;
  unsigned int bytes_left = count;
  OVERLAPPED ovl;
  
  ZeroMemory(&ovl, sizeof(ovl));
  ovl.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

  while (bytes_left)
  {
    DWORD bytes_read;
    ret = ReadFile(npc->pipe, buf, bytes_left, &bytes_read, &ovl);
	if ((!ret || !bytes_read) && (GetLastError() != ERROR_IO_PENDING))
        break;
    ret = GetOverlappedResult(npc->pipe, &ovl, &bytes_read, TRUE);
    if (!ret && (GetLastError() != ERROR_MORE_DATA))
        break;
    bytes_left -= bytes_read;
    buf += bytes_read;
  }
  CloseHandle(ovl.hEvent);
  return ret ? count : -1;
}

static int rpcrt4_conn_np_write(RpcConnection *Connection,
                             const void *buffer, unsigned int count)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  const char *buf = buffer;
  BOOL ret = TRUE;
  unsigned int bytes_left = count;
  OVERLAPPED ovl;
  
  ZeroMemory(&ovl, sizeof(ovl));
  ovl.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

  while (bytes_left)
  {
    DWORD bytes_written;
    ret = WriteFile(npc->pipe, buf, bytes_left, &bytes_written, &ovl);
    if ((!ret || !bytes_written) && (GetLastError() != ERROR_IO_PENDING))
        break;

    ret = GetOverlappedResult(npc->pipe, &ovl, &bytes_written, TRUE);
    if (!ret && (GetLastError() != ERROR_MORE_DATA))
        break;
    bytes_left -= bytes_written;
    buf += bytes_written;
  }
  CloseHandle(ovl.hEvent);
  return ret ? count : -1;
}

static int rpcrt4_conn_np_close(RpcConnection *Connection)
{
  RpcConnection_np *npc = (RpcConnection_np *) Connection;
  if (npc->pipe) {
    FlushFileBuffers(npc->pipe);
    CloseHandle(npc->pipe);
    npc->pipe = 0;
  }
  if (npc->ovl.hEvent) {
    CloseHandle(npc->ovl.hEvent);
    npc->ovl.hEvent = 0;
  }
  return 0;
}

static void rpcrt4_conn_np_cancel_call(RpcConnection *Connection)
{
    /* FIXME: implement when named pipe writes use overlapped I/O */
}

static int rpcrt4_conn_np_wait_for_incoming_data(RpcConnection *Connection)
{
    /* FIXME: implement when named pipe writes use overlapped I/O */
    return -1;
}

static size_t rpcrt4_ncacn_np_get_top_of_tower(unsigned char *tower_data,
                                               const char *networkaddr,
                                               const char *endpoint)
{
    twr_empty_floor_t *smb_floor;
    twr_empty_floor_t *nb_floor;
    size_t size;
    size_t networkaddr_size;
    size_t endpoint_size;

    TRACE("(%p, %s, %s)\n", tower_data, networkaddr, endpoint);

    networkaddr_size = networkaddr ? strlen(networkaddr) + 1 : 1;
    endpoint_size = endpoint ? strlen(endpoint) + 1 : 1;
    size = sizeof(*smb_floor) + endpoint_size + sizeof(*nb_floor) + networkaddr_size;

    if (!tower_data)
        return size;

    smb_floor = (twr_empty_floor_t *)tower_data;

    tower_data += sizeof(*smb_floor);

    smb_floor->count_lhs = sizeof(smb_floor->protid);
    smb_floor->protid = EPM_PROTOCOL_SMB;
    smb_floor->count_rhs = endpoint_size;

    if (endpoint)
        memcpy(tower_data, endpoint, endpoint_size);
    else
        tower_data[0] = 0;
    tower_data += endpoint_size;

    nb_floor = (twr_empty_floor_t *)tower_data;

    tower_data += sizeof(*nb_floor);

    nb_floor->count_lhs = sizeof(nb_floor->protid);
    nb_floor->protid = EPM_PROTOCOL_NETBIOS;
    nb_floor->count_rhs = networkaddr_size;

    if (networkaddr)
        memcpy(tower_data, networkaddr, networkaddr_size);
    else
        tower_data[0] = 0;

    return size;
}

static RPC_STATUS rpcrt4_ncacn_np_parse_top_of_tower(const unsigned char *tower_data,
                                                     size_t tower_size,
                                                     char **networkaddr,
                                                     char **endpoint)
{
    const twr_empty_floor_t *smb_floor = (const twr_empty_floor_t *)tower_data;
    const twr_empty_floor_t *nb_floor;

    TRACE("(%p, %d, %p, %p)\n", tower_data, (int)tower_size, networkaddr, endpoint);

    if (tower_size < sizeof(*smb_floor))
        return EPT_S_NOT_REGISTERED;

    tower_data += sizeof(*smb_floor);
    tower_size -= sizeof(*smb_floor);

    if ((smb_floor->count_lhs != sizeof(smb_floor->protid)) ||
        (smb_floor->protid != EPM_PROTOCOL_SMB) ||
        (smb_floor->count_rhs > tower_size) ||
        (tower_data[smb_floor->count_rhs - 1] != '\0'))
        return EPT_S_NOT_REGISTERED;

    if (endpoint)
    {
        *endpoint = I_RpcAllocate(smb_floor->count_rhs);
        if (!*endpoint)
            return RPC_S_OUT_OF_RESOURCES;
        memcpy(*endpoint, tower_data, smb_floor->count_rhs);
    }
    tower_data += smb_floor->count_rhs;
    tower_size -= smb_floor->count_rhs;

    if (tower_size < sizeof(*nb_floor))
        return EPT_S_NOT_REGISTERED;

    nb_floor = (const twr_empty_floor_t *)tower_data;

    tower_data += sizeof(*nb_floor);
    tower_size -= sizeof(*nb_floor);

    if ((nb_floor->count_lhs != sizeof(nb_floor->protid)) ||
        (nb_floor->protid != EPM_PROTOCOL_NETBIOS) ||
        (nb_floor->count_rhs > tower_size) ||
        (tower_data[nb_floor->count_rhs - 1] != '\0'))
        return EPT_S_NOT_REGISTERED;

    if (networkaddr)
    {
        *networkaddr = I_RpcAllocate(nb_floor->count_rhs);
        if (!*networkaddr)
        {
            if (endpoint)
            {
                I_RpcFree(*endpoint);
                *endpoint = NULL;
            }
            return RPC_S_OUT_OF_RESOURCES;
        }
        memcpy(*networkaddr, tower_data, nb_floor->count_rhs);
    }

    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_conn_np_impersonate_client(RpcConnection *conn)
{
    RpcConnection_np *npc = (RpcConnection_np *)conn;
    BOOL ret;

    TRACE("(%p)\n", conn);

    if (conn->AuthInfo && SecIsValidHandle(&conn->ctx))
        return RPCRT4_default_impersonate_client(conn);

    ret = ImpersonateNamedPipeClient(npc->pipe);
    if (!ret)
    {
        DWORD error = GetLastError();
        WARN("ImpersonateNamedPipeClient failed with error %u\n", error);
        switch (error)
        {
        case ERROR_CANNOT_IMPERSONATE:
            return RPC_S_NO_CONTEXT_AVAILABLE;
        }
    }
    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_conn_np_revert_to_self(RpcConnection *conn)
{
    BOOL ret;

    TRACE("(%p)\n", conn);

    if (conn->AuthInfo && SecIsValidHandle(&conn->ctx))
        return RPCRT4_default_revert_to_self(conn);

    ret = RevertToSelf();
    if (!ret)
    {
        WARN("RevertToSelf failed with error %u\n", GetLastError());
        return RPC_S_NO_CONTEXT_AVAILABLE;
    }
    return RPC_S_OK;
}

typedef struct _RpcServerProtseq_np
{
    RpcServerProtseq common;
    HANDLE mgr_event;
} RpcServerProtseq_np;

static RpcServerProtseq *rpcrt4_protseq_np_alloc(void)
{
    RpcServerProtseq_np *ps = HeapAlloc(GetProcessHeap(), 0, sizeof(*ps));
    if (ps)
        ps->mgr_event = CreateEventW(NULL, FALSE, FALSE, NULL);
    return &ps->common;
}

static void rpcrt4_protseq_np_signal_state_changed(RpcServerProtseq *protseq)
{
    RpcServerProtseq_np *npps = CONTAINING_RECORD(protseq, RpcServerProtseq_np, common);
    SetEvent(npps->mgr_event);
}

static void *rpcrt4_protseq_np_get_wait_array(RpcServerProtseq *protseq, void *prev_array, unsigned int *count)
{
    HANDLE *objs = prev_array;
    RpcConnection_np *conn;
    RpcServerProtseq_np *npps = CONTAINING_RECORD(protseq, RpcServerProtseq_np, common);
    
    EnterCriticalSection(&protseq->cs);
    
    /* open and count connections */
    *count = 1;
    conn = CONTAINING_RECORD(protseq->conn, RpcConnection_np, common);
    while (conn) {
        rpcrt4_conn_listen_pipe(conn);
        if (conn->ovl.hEvent)
            (*count)++;
        conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_np, common);
    }
    
    /* make array of connections */
    if (objs)
        objs = HeapReAlloc(GetProcessHeap(), 0, objs, *count*sizeof(HANDLE));
    else
        objs = HeapAlloc(GetProcessHeap(), 0, *count*sizeof(HANDLE));
    if (!objs)
    {
        ERR("couldn't allocate objs\n");
        LeaveCriticalSection(&protseq->cs);
        return NULL;
    }
    
    objs[0] = npps->mgr_event;
    *count = 1;
    conn = CONTAINING_RECORD(protseq->conn, RpcConnection_np, common);
    while (conn) {
        if ((objs[*count] = conn->ovl.hEvent))
            (*count)++;
        conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_np, common);
    }
    LeaveCriticalSection(&protseq->cs);
    return objs;
}

static void rpcrt4_protseq_np_free_wait_array(RpcServerProtseq *protseq, void *array)
{
    HeapFree(GetProcessHeap(), 0, array);
}

static int rpcrt4_protseq_np_wait_for_new_connection(RpcServerProtseq *protseq, unsigned int count, void *wait_array)
{
    HANDLE b_handle;
    HANDLE *objs = wait_array;
    DWORD res;
    RpcConnection *cconn;
    RpcConnection_np *conn;
    
    if (!objs)
        return -1;

    do
    {
        /* an alertable wait isn't strictly necessary, but due to our
         * overlapped I/O implementation in Wine we need to free some memory
         * by the file user APC being called, even if no completion routine was
         * specified at the time of starting the async operation */
        res = WaitForMultipleObjectsEx(count, objs, FALSE, INFINITE, TRUE);
    } while (res == WAIT_IO_COMPLETION);

    if (res == WAIT_OBJECT_0)
        return 0;
    else if (res == WAIT_FAILED)
    {
        ERR("wait failed with error %d\n", GetLastError());
        return -1;
    }
    else
    {
        b_handle = objs[res - WAIT_OBJECT_0];
        /* find which connection got a RPC */
        EnterCriticalSection(&protseq->cs);
        conn = CONTAINING_RECORD(protseq->conn, RpcConnection_np, common);
        while (conn) {
            if (b_handle == conn->ovl.hEvent) break;
            conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_np, common);
        }
        cconn = NULL;
        if (conn)
            RPCRT4_SpawnConnection(&cconn, &conn->common);
        else
            ERR("failed to locate connection for handle %p\n", b_handle);
        LeaveCriticalSection(&protseq->cs);
        if (cconn)
        {
            RPCRT4_new_client(cconn);
            return 1;
        }
        else return -1;
    }
}

static size_t rpcrt4_ncalrpc_get_top_of_tower(unsigned char *tower_data,
                                              const char *networkaddr,
                                              const char *endpoint)
{
    twr_empty_floor_t *pipe_floor;
    size_t size;
    size_t endpoint_size;

    TRACE("(%p, %s, %s)\n", tower_data, networkaddr, endpoint);

    endpoint_size = strlen(endpoint) + 1;
    size = sizeof(*pipe_floor) + endpoint_size;

    if (!tower_data)
        return size;

    pipe_floor = (twr_empty_floor_t *)tower_data;

    tower_data += sizeof(*pipe_floor);

    pipe_floor->count_lhs = sizeof(pipe_floor->protid);
    pipe_floor->protid = EPM_PROTOCOL_PIPE;
    pipe_floor->count_rhs = endpoint_size;

    memcpy(tower_data, endpoint, endpoint_size);

    return size;
}

static RPC_STATUS rpcrt4_ncalrpc_parse_top_of_tower(const unsigned char *tower_data,
                                                    size_t tower_size,
                                                    char **networkaddr,
                                                    char **endpoint)
{
    const twr_empty_floor_t *pipe_floor = (const twr_empty_floor_t *)tower_data;

    TRACE("(%p, %d, %p, %p)\n", tower_data, (int)tower_size, networkaddr, endpoint);

    if (tower_size < sizeof(*pipe_floor))
        return EPT_S_NOT_REGISTERED;

    tower_data += sizeof(*pipe_floor);
    tower_size -= sizeof(*pipe_floor);

    if ((pipe_floor->count_lhs != sizeof(pipe_floor->protid)) ||
        (pipe_floor->protid != EPM_PROTOCOL_PIPE) ||
        (pipe_floor->count_rhs > tower_size) ||
        (tower_data[pipe_floor->count_rhs - 1] != '\0'))
        return EPT_S_NOT_REGISTERED;

    if (networkaddr)
        *networkaddr = NULL;

    if (endpoint)
    {
        *endpoint = I_RpcAllocate(pipe_floor->count_rhs);
        if (!*endpoint)
            return RPC_S_OUT_OF_RESOURCES;
        memcpy(*endpoint, tower_data, pipe_floor->count_rhs);
    }

    return RPC_S_OK;
}

static BOOL rpcrt4_ncalrpc_is_authorized(RpcConnection *conn)
{
    return FALSE;
}

static RPC_STATUS rpcrt4_ncalrpc_authorize(RpcConnection *conn, BOOL first_time,
                                           unsigned char *in_buffer,
                                           unsigned int in_size,
                                           unsigned char *out_buffer,
                                           unsigned int *out_size)
{
    /* since this protocol is local to the machine there is no need to
     * authenticate the caller */
    *out_size = 0;
    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_ncalrpc_secure_packet(RpcConnection *conn,
    enum secure_packet_direction dir,
    RpcPktHdr *hdr, unsigned int hdr_size,
    unsigned char *stub_data, unsigned int stub_data_size,
    RpcAuthVerifier *auth_hdr,
    unsigned char *auth_value, unsigned int auth_value_size)
{
    /* since this protocol is local to the machine there is no need to secure
     * the packet */
    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_ncalrpc_inquire_auth_client(
    RpcConnection *conn, RPC_AUTHZ_HANDLE *privs, RPC_WSTR *server_princ_name,
    ULONG *authn_level, ULONG *authn_svc, ULONG *authz_svc, ULONG flags)
{
    TRACE("(%p, %p, %p, %p, %p, %p, 0x%x)\n", conn, privs,
          server_princ_name, authn_level, authn_svc, authz_svc, flags);

    if (privs)
    {
        FIXME("privs not implemented\n");
        *privs = NULL;
    }
    if (server_princ_name)
    {
        FIXME("server_princ_name not implemented\n");
        *server_princ_name = NULL;
    }
    if (authn_level) *authn_level = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
    if (authn_svc) *authn_svc = RPC_C_AUTHN_WINNT;
    if (authz_svc)
    {
        FIXME("authorization service not implemented\n");
        *authz_svc = RPC_C_AUTHZ_NONE;
    }
    if (flags)
        FIXME("flags 0x%x not implemented\n", flags);

    return RPC_S_OK;
}

/**** ncacn_ip_tcp support ****/

static size_t rpcrt4_ip_tcp_get_top_of_tower(unsigned char *tower_data,
                                             const char *networkaddr,
                                             unsigned char tcp_protid,
                                             const char *endpoint)
{
    twr_tcp_floor_t *tcp_floor;
    twr_ipv4_floor_t *ipv4_floor;
    struct addrinfo *ai;
    struct addrinfo hints;
    int ret;
    size_t size = sizeof(*tcp_floor) + sizeof(*ipv4_floor);

    TRACE("(%p, %s, %s)\n", tower_data, networkaddr, endpoint);

    if (!tower_data)
        return size;

    tcp_floor = (twr_tcp_floor_t *)tower_data;
    tower_data += sizeof(*tcp_floor);

    ipv4_floor = (twr_ipv4_floor_t *)tower_data;

    tcp_floor->count_lhs = sizeof(tcp_floor->protid);
    tcp_floor->protid = tcp_protid;
    tcp_floor->count_rhs = sizeof(tcp_floor->port);

    ipv4_floor->count_lhs = sizeof(ipv4_floor->protid);
    ipv4_floor->protid = EPM_PROTOCOL_IP;
    ipv4_floor->count_rhs = sizeof(ipv4_floor->ipv4addr);

    hints.ai_flags          = AI_NUMERICHOST;
    /* FIXME: only support IPv4 at the moment. how is IPv6 represented by the EPM? */
    hints.ai_family         = PF_INET;
    hints.ai_socktype       = SOCK_STREAM;
    hints.ai_protocol       = IPPROTO_TCP;
    hints.ai_addrlen        = 0;
    hints.ai_addr           = NULL;
    hints.ai_canonname      = NULL;
    hints.ai_next           = NULL;

    ret = getaddrinfo(networkaddr, endpoint, &hints, &ai);
    if (ret)
    {
        ret = getaddrinfo("0.0.0.0", endpoint, &hints, &ai);
        if (ret)
        {
            ERR("getaddrinfo failed: %s\n", gai_strerror(ret));
            return 0;
        }
    }

    if (ai->ai_family == PF_INET)
    {
        const struct sockaddr_in *sin = (const struct sockaddr_in *)ai->ai_addr;
        tcp_floor->port = sin->sin_port;
        ipv4_floor->ipv4addr = sin->sin_addr.s_addr;
    }
    else
    {
        ERR("unexpected protocol family %d\n", ai->ai_family);
        return 0;
    }

    freeaddrinfo(ai);

    return size;
}

static RPC_STATUS rpcrt4_ip_tcp_parse_top_of_tower(const unsigned char *tower_data,
                                                   size_t tower_size,
                                                   char **networkaddr,
                                                   unsigned char tcp_protid,
                                                   char **endpoint)
{
    const twr_tcp_floor_t *tcp_floor = (const twr_tcp_floor_t *)tower_data;
    const twr_ipv4_floor_t *ipv4_floor;
    struct in_addr in_addr;

    TRACE("(%p, %d, %p, %p)\n", tower_data, (int)tower_size, networkaddr, endpoint);

    if (tower_size < sizeof(*tcp_floor))
        return EPT_S_NOT_REGISTERED;

    tower_data += sizeof(*tcp_floor);
    tower_size -= sizeof(*tcp_floor);

    if (tower_size < sizeof(*ipv4_floor))
        return EPT_S_NOT_REGISTERED;

    ipv4_floor = (const twr_ipv4_floor_t *)tower_data;

    if ((tcp_floor->count_lhs != sizeof(tcp_floor->protid)) ||
        (tcp_floor->protid != tcp_protid) ||
        (tcp_floor->count_rhs != sizeof(tcp_floor->port)) ||
        (ipv4_floor->count_lhs != sizeof(ipv4_floor->protid)) ||
        (ipv4_floor->protid != EPM_PROTOCOL_IP) ||
        (ipv4_floor->count_rhs != sizeof(ipv4_floor->ipv4addr)))
        return EPT_S_NOT_REGISTERED;

    if (endpoint)
    {
        *endpoint = I_RpcAllocate(6 /* sizeof("65535") + 1 */);
        if (!*endpoint)
            return RPC_S_OUT_OF_RESOURCES;
        sprintf(*endpoint, "%u", ntohs(tcp_floor->port));
    }

    if (networkaddr)
    {
        *networkaddr = I_RpcAllocate(INET_ADDRSTRLEN);
        if (!*networkaddr)
        {
            if (endpoint)
            {
                I_RpcFree(*endpoint);
                *endpoint = NULL;
            }
            return RPC_S_OUT_OF_RESOURCES;
        }
        in_addr.s_addr = ipv4_floor->ipv4addr;
        if (!inet_ntop(AF_INET, &in_addr, *networkaddr, INET_ADDRSTRLEN))
        {
            ERR("inet_ntop: %s\n", strerror(errno));
            I_RpcFree(*networkaddr);
            *networkaddr = NULL;
            if (endpoint)
            {
                I_RpcFree(*endpoint);
                *endpoint = NULL;
            }
            return EPT_S_NOT_REGISTERED;
        }
    }

    return RPC_S_OK;
}

typedef struct _RpcConnection_tcp
{
  RpcConnection common;
  int sock;
#ifdef HAVE_SOCKETPAIR
  int cancel_fds[2];
#else
  HANDLE sock_event;
  HANDLE cancel_event;
#endif
} RpcConnection_tcp;

#ifdef HAVE_SOCKETPAIR

static BOOL rpcrt4_sock_wait_init(RpcConnection_tcp *tcpc)
{
  if (socketpair(PF_UNIX, SOCK_STREAM, 0, tcpc->cancel_fds) < 0)
  {
    ERR("socketpair() failed: %s\n", strerror(errno));
    return FALSE;
  }
  return TRUE;
}

static BOOL rpcrt4_sock_wait_for_recv(RpcConnection_tcp *tcpc)
{
  struct pollfd pfds[2];
  pfds[0].fd = tcpc->sock;
  pfds[0].events = POLLIN;
  pfds[1].fd = tcpc->cancel_fds[0];
  pfds[1].events = POLLIN;
  if (poll(pfds, 2, -1 /* infinite */) == -1 && errno != EINTR)
  {
    ERR("poll() failed: %s\n", strerror(errno));
    return FALSE;
  }
  if (pfds[1].revents & POLLIN) /* canceled */
  {
    char dummy;
    read(pfds[1].fd, &dummy, sizeof(dummy));
    return FALSE;
  }
  return TRUE;
}

static BOOL rpcrt4_sock_wait_for_send(RpcConnection_tcp *tcpc)
{
  struct pollfd pfd;
  pfd.fd = tcpc->sock;
  pfd.events = POLLOUT;
  if (poll(&pfd, 1, -1 /* infinite */) == -1 && errno != EINTR)
  {
    ERR("poll() failed: %s\n", strerror(errno));
    return FALSE;
  }
  return TRUE;
}

static void rpcrt4_sock_wait_cancel(RpcConnection_tcp *tcpc)
{
  char dummy = 1;

  write(tcpc->cancel_fds[1], &dummy, 1);
}

static void rpcrt4_sock_wait_destroy(RpcConnection_tcp *tcpc)
{
  close(tcpc->cancel_fds[0]);
  close(tcpc->cancel_fds[1]);
}

#else /* HAVE_SOCKETPAIR */

static BOOL rpcrt4_sock_wait_init(RpcConnection_tcp *tcpc)
{
  static BOOL wsa_inited;
  if (!wsa_inited)
  {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    /* Note: WSAStartup can be called more than once so we don't bother with
     * making accesses to wsa_inited thread-safe */
    wsa_inited = TRUE;
  }
  tcpc->sock_event = CreateEventW(NULL, FALSE, FALSE, NULL);
  tcpc->cancel_event = CreateEventW(NULL, FALSE, FALSE, NULL);
  if (!tcpc->sock_event || !tcpc->cancel_event)
  {
    ERR("event creation failed\n");
    if (tcpc->sock_event) CloseHandle(tcpc->sock_event);
    return FALSE;
  }
  return TRUE;
}

static BOOL rpcrt4_sock_wait_for_recv(RpcConnection_tcp *tcpc)
{
  HANDLE wait_handles[2];
  DWORD res;
  if (WSAEventSelect(tcpc->sock, tcpc->sock_event, FD_READ | FD_CLOSE) == SOCKET_ERROR)
  {
    ERR("WSAEventSelect() failed with error %d\n", WSAGetLastError());
    return FALSE;
  }
  wait_handles[0] = tcpc->sock_event;
  wait_handles[1] = tcpc->cancel_event;
  res = WaitForMultipleObjects(2, wait_handles, FALSE, INFINITE);
  switch (res)
  {
  case WAIT_OBJECT_0:
    return TRUE;
  case WAIT_OBJECT_0 + 1:
    return FALSE;
  default:
    ERR("WaitForMultipleObjects() failed with error %d\n", GetLastError());
    return FALSE;
  }
}

static BOOL rpcrt4_sock_wait_for_send(RpcConnection_tcp *tcpc)
{
  DWORD res;
  if (WSAEventSelect(tcpc->sock, tcpc->sock_event, FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
  {
    ERR("WSAEventSelect() failed with error %d\n", WSAGetLastError());
    return FALSE;
  }
  res = WaitForSingleObject(tcpc->sock_event, INFINITE);
  switch (res)
  {
  case WAIT_OBJECT_0:
    return TRUE;
  default:
    ERR("WaitForMultipleObjects() failed with error %d\n", GetLastError());
    return FALSE;
  }
}

static void rpcrt4_sock_wait_cancel(RpcConnection_tcp *tcpc)
{
  SetEvent(tcpc->cancel_event);
}

static void rpcrt4_sock_wait_destroy(RpcConnection_tcp *tcpc)
{
  CloseHandle(tcpc->sock_event);
  CloseHandle(tcpc->cancel_event);
}

#endif

static RpcConnection *rpcrt4_conn_tcp_alloc(void)
{
  RpcConnection_tcp *tcpc;
  tcpc = HeapAlloc(GetProcessHeap(), 0, sizeof(RpcConnection_tcp));
  if (tcpc == NULL)
    return NULL;
  tcpc->sock = -1;
  if (!rpcrt4_sock_wait_init(tcpc))
  {
    HeapFree(GetProcessHeap(), 0, tcpc);
    return NULL;
  }
  return &tcpc->common;
}

static RPC_STATUS rpcrt4_ncacn_ip_tcp_open(RpcConnection* Connection)
{
  RpcConnection_tcp *tcpc = (RpcConnection_tcp *) Connection;
  int sock;
  int ret;
  struct addrinfo *ai;
  struct addrinfo *ai_cur;
  struct addrinfo hints;

  TRACE("(%s, %s)\n", Connection->NetworkAddr, Connection->Endpoint);

  if (tcpc->sock != -1)
    return RPC_S_OK;

  hints.ai_flags          = 0;
  hints.ai_family         = PF_UNSPEC;
  hints.ai_socktype       = SOCK_STREAM;
  hints.ai_protocol       = IPPROTO_TCP;
  hints.ai_addrlen        = 0;
  hints.ai_addr           = NULL;
  hints.ai_canonname      = NULL;
  hints.ai_next           = NULL;

  ret = getaddrinfo(Connection->NetworkAddr, Connection->Endpoint, &hints, &ai);
  if (ret)
  {
    ERR("getaddrinfo for %s:%s failed: %s\n", Connection->NetworkAddr,
      Connection->Endpoint, gai_strerror(ret));
    return RPC_S_SERVER_UNAVAILABLE;
  }

  for (ai_cur = ai; ai_cur; ai_cur = ai_cur->ai_next)
  {
    int val;
    u_long nonblocking;

    if (ai_cur->ai_family != AF_INET && ai_cur->ai_family != AF_INET6)
    {
      TRACE("skipping non-IP/IPv6 address family\n");
      continue;
    }

    if (TRACE_ON(rpc))
    {
      char host[256];
      char service[256];
      getnameinfo(ai_cur->ai_addr, ai_cur->ai_addrlen,
        host, sizeof(host), service, sizeof(service),
        NI_NUMERICHOST | NI_NUMERICSERV);
      TRACE("trying %s:%s\n", host, service);
    }

    sock = socket(ai_cur->ai_family, ai_cur->ai_socktype, ai_cur->ai_protocol);
    if (sock == -1)
    {
      WARN("socket() failed: %s\n", strerror(errno));
      continue;
    }

    if (0>connect(sock, ai_cur->ai_addr, ai_cur->ai_addrlen))
    {
      WARN("connect() failed: %s\n", strerror(errno));
      closesocket(sock);
      continue;
    }

    /* RPC depends on having minimal latency so disable the Nagle algorithm */
    val = 1;
    setsockopt(sock, SOL_TCP, TCP_NODELAY, (char *)&val, sizeof(val));
    nonblocking = 1;
    ioctlsocket(sock, FIONBIO, &nonblocking);

    tcpc->sock = sock;

    freeaddrinfo(ai);
    TRACE("connected\n");
    return RPC_S_OK;
  }

  freeaddrinfo(ai);
  ERR("couldn't connect to %s:%s\n", Connection->NetworkAddr, Connection->Endpoint);
  return RPC_S_SERVER_UNAVAILABLE;
}

static RPC_STATUS rpcrt4_protseq_ncacn_ip_tcp_open_endpoint(RpcServerProtseq *protseq, const char *endpoint)
{
    RPC_STATUS status = RPC_S_CANT_CREATE_ENDPOINT;
    int sock;
    int ret;
    struct addrinfo *ai;
    struct addrinfo *ai_cur;
    struct addrinfo hints;
    RpcConnection *first_connection = NULL;

    TRACE("(%p, %s)\n", protseq, endpoint);

    hints.ai_flags          = AI_PASSIVE /* for non-localhost addresses */;
    hints.ai_family         = PF_UNSPEC;
    hints.ai_socktype       = SOCK_STREAM;
    hints.ai_protocol       = IPPROTO_TCP;
    hints.ai_addrlen        = 0;
    hints.ai_addr           = NULL;
    hints.ai_canonname      = NULL;
    hints.ai_next           = NULL;

    ret = getaddrinfo(NULL, endpoint ? endpoint : "0", &hints, &ai);
    if (ret)
    {
        ERR("getaddrinfo for port %s failed: %s\n", endpoint,
            gai_strerror(ret));
        if ((ret == EAI_SERVICE) || (ret == EAI_NONAME))
            return RPC_S_INVALID_ENDPOINT_FORMAT;
        return RPC_S_CANT_CREATE_ENDPOINT;
    }

    for (ai_cur = ai; ai_cur; ai_cur = ai_cur->ai_next)
    {
        RpcConnection_tcp *tcpc;
        RPC_STATUS create_status;
        struct sockaddr_storage sa;
        socklen_t sa_len;
        char service[NI_MAXSERV];
        u_long nonblocking;

        if (ai_cur->ai_family != AF_INET && ai_cur->ai_family != AF_INET6)
        {
            TRACE("skipping non-IP/IPv6 address family\n");
            continue;
        }

        if (TRACE_ON(rpc))
        {
            char host[256];
            getnameinfo(ai_cur->ai_addr, ai_cur->ai_addrlen,
                        host, sizeof(host), service, sizeof(service),
                        NI_NUMERICHOST | NI_NUMERICSERV);
            TRACE("trying %s:%s\n", host, service);
        }

        sock = socket(ai_cur->ai_family, ai_cur->ai_socktype, ai_cur->ai_protocol);
        if (sock == -1)
        {
            WARN("socket() failed: %s\n", strerror(errno));
            status = RPC_S_CANT_CREATE_ENDPOINT;
            continue;
        }

        ret = bind(sock, ai_cur->ai_addr, ai_cur->ai_addrlen);
        if (ret < 0)
        {
            WARN("bind failed: %s\n", strerror(errno));
            closesocket(sock);
            if (errno == EADDRINUSE)
              status = RPC_S_DUPLICATE_ENDPOINT;
            else
              status = RPC_S_CANT_CREATE_ENDPOINT;
            continue;
        }

        sa_len = sizeof(sa);
        if (getsockname(sock, (struct sockaddr *)&sa, &sa_len))
        {
            WARN("getsockname() failed: %s\n", strerror(errno));
            status = RPC_S_CANT_CREATE_ENDPOINT;
            continue;
        }

        ret = getnameinfo((struct sockaddr *)&sa, sa_len,
                          NULL, 0, service, sizeof(service),
                          NI_NUMERICSERV);
        if (ret)
        {
            WARN("getnameinfo failed: %s\n", gai_strerror(ret));
            status = RPC_S_CANT_CREATE_ENDPOINT;
            continue;
        }

        create_status = RPCRT4_CreateConnection((RpcConnection **)&tcpc, TRUE,
                                                protseq->Protseq, NULL,
                                                service, NULL, NULL, NULL);
        if (create_status != RPC_S_OK)
        {
            closesocket(sock);
            status = create_status;
            continue;
        }

        tcpc->sock = sock;
        ret = listen(sock, protseq->MaxCalls);
        if (ret < 0)
        {
            WARN("listen failed: %s\n", strerror(errno));
            RPCRT4_DestroyConnection(&tcpc->common);
            status = RPC_S_OUT_OF_RESOURCES;
            continue;
        }
        /* need a non-blocking socket, otherwise accept() has a potential
         * race-condition (poll() says it is readable, connection drops,
         * and accept() blocks until the next connection comes...)
         */
        nonblocking = 1;
        ret = ioctlsocket(sock, FIONBIO, &nonblocking);
        if (ret < 0)
        {
            WARN("couldn't make socket non-blocking, error %d\n", ret);
            RPCRT4_DestroyConnection(&tcpc->common);
            status = RPC_S_OUT_OF_RESOURCES;
            continue;
        }

        tcpc->common.Next = first_connection;
        first_connection = &tcpc->common;

        /* since IPv4 and IPv6 share the same port space, we only need one
         * successful bind to listen for both */
        break;
    }

    freeaddrinfo(ai);

    /* if at least one connection was created for an endpoint then
     * return success */
    if (first_connection)
    {
        RpcConnection *conn;

        /* find last element in list */
        for (conn = first_connection; conn->Next; conn = conn->Next)
            ;

        EnterCriticalSection(&protseq->cs);
        conn->Next = protseq->conn;
        protseq->conn = first_connection;
        LeaveCriticalSection(&protseq->cs);
        
        TRACE("listening on %s\n", endpoint);
        return RPC_S_OK;
    }

    ERR("couldn't listen on port %s\n", endpoint);
    return status;
}

static RPC_STATUS rpcrt4_conn_tcp_handoff(RpcConnection *old_conn, RpcConnection *new_conn)
{
  int ret;
  struct sockaddr_in address;
  socklen_t addrsize;
  RpcConnection_tcp *server = (RpcConnection_tcp*) old_conn;
  RpcConnection_tcp *client = (RpcConnection_tcp*) new_conn;
  u_long nonblocking;

  addrsize = sizeof(address);
  ret = accept(server->sock, (struct sockaddr*) &address, &addrsize);
  if (ret < 0)
  {
    ERR("Failed to accept a TCP connection: error %d\n", ret);
    return RPC_S_OUT_OF_RESOURCES;
  }
  nonblocking = 1;
  ioctlsocket(ret, FIONBIO, &nonblocking);
  client->sock = ret;
  TRACE("Accepted a new TCP connection\n");
  return RPC_S_OK;
}

static int rpcrt4_conn_tcp_read(RpcConnection *Connection,
                                void *buffer, unsigned int count)
{
  RpcConnection_tcp *tcpc = (RpcConnection_tcp *) Connection;
  int bytes_read = 0;
  while (bytes_read != count)
  {
    int r = recv(tcpc->sock, (char *)buffer + bytes_read, count - bytes_read, 0);
    if (!r)
      return -1;
    else if (r > 0)
      bytes_read += r;
    else if (errno != EAGAIN)
    {
      WARN("recv() failed: %s\n", strerror(errno));
      return -1;
    }
    else
    {
      if (!rpcrt4_sock_wait_for_recv(tcpc))
        return -1;
    }
  }
  TRACE("%d %p %u -> %d\n", tcpc->sock, buffer, count, bytes_read);
  return bytes_read;
}

static int rpcrt4_conn_tcp_write(RpcConnection *Connection,
                                 const void *buffer, unsigned int count)
{
  RpcConnection_tcp *tcpc = (RpcConnection_tcp *) Connection;
  int bytes_written = 0;
  while (bytes_written != count)
  {
    int r = send(tcpc->sock, (const char *)buffer + bytes_written, count - bytes_written, 0);
    if (r >= 0)
      bytes_written += r;
    else if (errno != EAGAIN)
      return -1;
    else
    {
      if (!rpcrt4_sock_wait_for_send(tcpc))
        return -1;
    }
  }
  TRACE("%d %p %u -> %d\n", tcpc->sock, buffer, count, bytes_written);
  return bytes_written;
}

static int rpcrt4_conn_tcp_close(RpcConnection *Connection)
{
  RpcConnection_tcp *tcpc = (RpcConnection_tcp *) Connection;

  TRACE("%d\n", tcpc->sock);

  if (tcpc->sock != -1)
    closesocket(tcpc->sock);
  tcpc->sock = -1;
  rpcrt4_sock_wait_destroy(tcpc);
  return 0;
}

static void rpcrt4_conn_tcp_cancel_call(RpcConnection *Connection)
{
    RpcConnection_tcp *tcpc = (RpcConnection_tcp *) Connection;
    TRACE("%p\n", Connection);
    rpcrt4_sock_wait_cancel(tcpc);
}

static int rpcrt4_conn_tcp_wait_for_incoming_data(RpcConnection *Connection)
{
    RpcConnection_tcp *tcpc = (RpcConnection_tcp *) Connection;

    TRACE("%p\n", Connection);

    if (!rpcrt4_sock_wait_for_recv(tcpc))
        return -1;
    return 0;
}

static size_t rpcrt4_ncacn_ip_tcp_get_top_of_tower(unsigned char *tower_data,
                                                   const char *networkaddr,
                                                   const char *endpoint)
{
    return rpcrt4_ip_tcp_get_top_of_tower(tower_data, networkaddr,
                                          EPM_PROTOCOL_TCP, endpoint);
}

#ifdef HAVE_SOCKETPAIR

typedef struct _RpcServerProtseq_sock
{
    RpcServerProtseq common;
    int mgr_event_rcv;
    int mgr_event_snd;
} RpcServerProtseq_sock;

static RpcServerProtseq *rpcrt4_protseq_sock_alloc(void)
{
    RpcServerProtseq_sock *ps = HeapAlloc(GetProcessHeap(), 0, sizeof(*ps));
    if (ps)
    {
        int fds[2];
        if (!socketpair(PF_UNIX, SOCK_DGRAM, 0, fds))
        {
            fcntl(fds[0], F_SETFL, O_NONBLOCK);
            fcntl(fds[1], F_SETFL, O_NONBLOCK);
            ps->mgr_event_rcv = fds[0];
            ps->mgr_event_snd = fds[1];
        }
        else
        {
            ERR("socketpair failed with error %s\n", strerror(errno));
            HeapFree(GetProcessHeap(), 0, ps);
            return NULL;
        }
    }
    return &ps->common;
}

static void rpcrt4_protseq_sock_signal_state_changed(RpcServerProtseq *protseq)
{
    RpcServerProtseq_sock *sockps = CONTAINING_RECORD(protseq, RpcServerProtseq_sock, common);
    char dummy = 1;
    write(sockps->mgr_event_snd, &dummy, sizeof(dummy));
}

static void *rpcrt4_protseq_sock_get_wait_array(RpcServerProtseq *protseq, void *prev_array, unsigned int *count)
{
    struct pollfd *poll_info = prev_array;
    RpcConnection_tcp *conn;
    RpcServerProtseq_sock *sockps = CONTAINING_RECORD(protseq, RpcServerProtseq_sock, common);

    EnterCriticalSection(&protseq->cs);
    
    /* open and count connections */
    *count = 1;
    conn = (RpcConnection_tcp *)protseq->conn;
    while (conn) {
        if (conn->sock != -1)
            (*count)++;
        conn = (RpcConnection_tcp *)conn->common.Next;
    }
    
    /* make array of connections */
    if (poll_info)
        poll_info = HeapReAlloc(GetProcessHeap(), 0, poll_info, *count*sizeof(*poll_info));
    else
        poll_info = HeapAlloc(GetProcessHeap(), 0, *count*sizeof(*poll_info));
    if (!poll_info)
    {
        ERR("couldn't allocate poll_info\n");
        LeaveCriticalSection(&protseq->cs);
        return NULL;
    }

    poll_info[0].fd = sockps->mgr_event_rcv;
    poll_info[0].events = POLLIN;
    *count = 1;
    conn =  CONTAINING_RECORD(protseq->conn, RpcConnection_tcp, common);
    while (conn) {
        if (conn->sock != -1)
        {
            poll_info[*count].fd = conn->sock;
            poll_info[*count].events = POLLIN;
            (*count)++;
        }
        conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_tcp, common);
    }
    LeaveCriticalSection(&protseq->cs);
    return poll_info;
}

static void rpcrt4_protseq_sock_free_wait_array(RpcServerProtseq *protseq, void *array)
{
    HeapFree(GetProcessHeap(), 0, array);
}

static int rpcrt4_protseq_sock_wait_for_new_connection(RpcServerProtseq *protseq, unsigned int count, void *wait_array)
{
    struct pollfd *poll_info = wait_array;
    int ret;
    unsigned int i;
    RpcConnection *cconn;
    RpcConnection_tcp *conn;
    
    if (!poll_info)
        return -1;
    
    ret = poll(poll_info, count, -1);
    if (ret < 0)
    {
        ERR("poll failed with error %d\n", ret);
        return -1;
    }

    for (i = 0; i < count; i++)
        if (poll_info[i].revents & POLLIN)
        {
            /* RPC server event */
            if (i == 0)
            {
                char dummy;
                read(poll_info[0].fd, &dummy, sizeof(dummy));
                return 0;
            }

            /* find which connection got a RPC */
            EnterCriticalSection(&protseq->cs);
            conn = CONTAINING_RECORD(protseq->conn, RpcConnection_tcp, common);
            while (conn) {
                if (poll_info[i].fd == conn->sock) break;
                conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_tcp, common);
            }
            cconn = NULL;
            if (conn)
                RPCRT4_SpawnConnection(&cconn, &conn->common);
            else
                ERR("failed to locate connection for fd %d\n", poll_info[i].fd);
            LeaveCriticalSection(&protseq->cs);
            if (cconn)
                RPCRT4_new_client(cconn);
            else
                return -1;
        }

    return 1;
}

#else /* HAVE_SOCKETPAIR */

typedef struct _RpcServerProtseq_sock
{
    RpcServerProtseq common;
    HANDLE mgr_event;
} RpcServerProtseq_sock;

static RpcServerProtseq *rpcrt4_protseq_sock_alloc(void)
{
    RpcServerProtseq_sock *ps = HeapAlloc(GetProcessHeap(), 0, sizeof(*ps));
    if (ps)
    {
        static BOOL wsa_inited;
        if (!wsa_inited)
        {
            WSADATA wsadata;
            WSAStartup(MAKEWORD(2, 2), &wsadata);
            /* Note: WSAStartup can be called more than once so we don't bother with
             * making accesses to wsa_inited thread-safe */
            wsa_inited = TRUE;
        }
        ps->mgr_event = CreateEventW(NULL, FALSE, FALSE, NULL);
    }
    return &ps->common;
}

static void rpcrt4_protseq_sock_signal_state_changed(RpcServerProtseq *protseq)
{
    RpcServerProtseq_sock *sockps = CONTAINING_RECORD(protseq, RpcServerProtseq_sock, common);
    SetEvent(sockps->mgr_event);
}

static void *rpcrt4_protseq_sock_get_wait_array(RpcServerProtseq *protseq, void *prev_array, unsigned int *count)
{
    HANDLE *objs = prev_array;
    RpcConnection_tcp *conn;
    RpcServerProtseq_sock *sockps = CONTAINING_RECORD(protseq, RpcServerProtseq_sock, common);

    EnterCriticalSection(&protseq->cs);

    /* open and count connections */
    *count = 1;
    conn = CONTAINING_RECORD(protseq->conn, RpcConnection_tcp, common);
    while (conn)
    {
        if (conn->sock != -1)
            (*count)++;
        conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_tcp, common);
    }

    /* make array of connections */
    if (objs)
        objs = HeapReAlloc(GetProcessHeap(), 0, objs, *count*sizeof(HANDLE));
    else
        objs = HeapAlloc(GetProcessHeap(), 0, *count*sizeof(HANDLE));
    if (!objs)
    {
        ERR("couldn't allocate objs\n");
        LeaveCriticalSection(&protseq->cs);
        return NULL;
    }

    objs[0] = sockps->mgr_event;
    *count = 1;
    conn = CONTAINING_RECORD(protseq->conn, RpcConnection_tcp, common);
    while (conn)
    {
        if (conn->sock != -1)
        {
            int res = WSAEventSelect(conn->sock, conn->sock_event, FD_ACCEPT);
            if (res == SOCKET_ERROR)
                ERR("WSAEventSelect() failed with error %d\n", WSAGetLastError());
            else
            {
                objs[*count] = conn->sock_event;
                (*count)++;
            }
        }
        conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_tcp, common);
    }
    LeaveCriticalSection(&protseq->cs);
    return objs;
}

static void rpcrt4_protseq_sock_free_wait_array(RpcServerProtseq *protseq, void *array)
{
    HeapFree(GetProcessHeap(), 0, array);
}

static int rpcrt4_protseq_sock_wait_for_new_connection(RpcServerProtseq *protseq, unsigned int count, void *wait_array)
{
    HANDLE b_handle;
    HANDLE *objs = wait_array;
    DWORD res;
    RpcConnection *cconn;
    RpcConnection_tcp *conn;

    if (!objs)
        return -1;

    do
    {
        /* an alertable wait isn't strictly necessary, but due to our
         * overlapped I/O implementation in Wine we need to free some memory
         * by the file user APC being called, even if no completion routine was
         * specified at the time of starting the async operation */
        res = WaitForMultipleObjectsEx(count, objs, FALSE, INFINITE, TRUE);
    } while (res == WAIT_IO_COMPLETION);

    if (res == WAIT_OBJECT_0)
        return 0;
    else if (res == WAIT_FAILED)
    {
        ERR("wait failed with error %d\n", GetLastError());
        return -1;
    }
    else
    {
        b_handle = objs[res - WAIT_OBJECT_0];
        /* find which connection got a RPC */
        EnterCriticalSection(&protseq->cs);
        conn = CONTAINING_RECORD(protseq->conn, RpcConnection_tcp, common);
        while (conn)
        {
            if (b_handle == conn->sock_event) break;
            conn = CONTAINING_RECORD(conn->common.Next, RpcConnection_tcp, common);
        }
        cconn = NULL;
        if (conn)
            RPCRT4_SpawnConnection(&cconn, &conn->common);
        else
            ERR("failed to locate connection for handle %p\n", b_handle);
        LeaveCriticalSection(&protseq->cs);
        if (cconn)
        {
            RPCRT4_new_client(cconn);
            return 1;
        }
        else return -1;
    }
}

#endif  /* HAVE_SOCKETPAIR */

static RPC_STATUS rpcrt4_ncacn_ip_tcp_parse_top_of_tower(const unsigned char *tower_data,
                                                         size_t tower_size,
                                                         char **networkaddr,
                                                         char **endpoint)
{
    return rpcrt4_ip_tcp_parse_top_of_tower(tower_data, tower_size,
                                            networkaddr, EPM_PROTOCOL_TCP,
                                            endpoint);
}

/**** ncacn_http support ****/

/* 60 seconds is the period native uses */
#define HTTP_IDLE_TIME 60000

/* reference counted to avoid a race between a cancelled call's connection
 * being destroyed and the asynchronous InternetReadFileEx call being
 * completed */
typedef struct _RpcHttpAsyncData
{
    LONG refs;
    HANDLE completion_event;
    INTERNET_BUFFERSA inet_buffers;
    void *destination_buffer; /* the address that inet_buffers.lpvBuffer will be
                               * copied into when the call completes */
    CRITICAL_SECTION cs;
} RpcHttpAsyncData;

static ULONG RpcHttpAsyncData_AddRef(RpcHttpAsyncData *data)
{
    return InterlockedIncrement(&data->refs);
}

static ULONG RpcHttpAsyncData_Release(RpcHttpAsyncData *data)
{
    ULONG refs = InterlockedDecrement(&data->refs);
    if (!refs)
    {
        TRACE("destroying async data %p\n", data);
        CloseHandle(data->completion_event);
        HeapFree(GetProcessHeap(), 0, data->inet_buffers.lpvBuffer);
        DeleteCriticalSection(&data->cs);
        HeapFree(GetProcessHeap(), 0, data);
    }
    return refs;
}

typedef struct _RpcConnection_http
{
    RpcConnection common;
    HINTERNET app_info;
    HINTERNET session;
    HINTERNET in_request;
    HINTERNET out_request;
    HANDLE timer_cancelled;
    HANDLE cancel_event;
    DWORD last_sent_time;
    ULONG bytes_received;
    ULONG flow_control_mark; /* send a control packet to the server when this many bytes received */
    ULONG flow_control_increment; /* number of bytes to increment flow_control_mark by */
    UUID connection_uuid;
    UUID in_pipe_uuid;
    UUID out_pipe_uuid;
    RpcHttpAsyncData *async_data;
} RpcConnection_http;

static RpcConnection *rpcrt4_ncacn_http_alloc(void)
{
    RpcConnection_http *httpc;
    httpc = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*httpc));
    if (!httpc) return NULL;
    httpc->async_data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RpcHttpAsyncData));
    if (!httpc->async_data)
    {
        HeapFree(GetProcessHeap(), 0, httpc);
        return NULL;
    }
    TRACE("async data = %p\n", httpc->async_data);
    httpc->cancel_event = CreateEventW(NULL, FALSE, FALSE, NULL);
    httpc->async_data->refs = 1;
    httpc->async_data->inet_buffers.dwStructSize = sizeof(INTERNET_BUFFERSA);
    httpc->async_data->inet_buffers.lpvBuffer = NULL;
    httpc->async_data->destination_buffer = NULL;
    InitializeCriticalSection(&httpc->async_data->cs);
    return &httpc->common;
}

typedef struct _HttpTimerThreadData
{
    PVOID timer_param;
    DWORD *last_sent_time;
    HANDLE timer_cancelled;
} HttpTimerThreadData;

static VOID rpcrt4_http_keep_connection_active_timer_proc(PVOID param, BOOLEAN dummy)
{
    HINTERNET in_request = param;
    RpcPktHdr *idle_pkt;

    idle_pkt = RPCRT4_BuildHttpHeader(NDR_LOCAL_DATA_REPRESENTATION, 0x0001,
                                      0, 0);
    if (idle_pkt)
    {
        DWORD bytes_written;
        InternetWriteFile(in_request, idle_pkt, idle_pkt->common.frag_len, &bytes_written);
        RPCRT4_FreeHeader(idle_pkt);
    }
}

static inline DWORD rpcrt4_http_timer_calc_timeout(DWORD *last_sent_time)
{
    DWORD cur_time = GetTickCount();
    DWORD cached_last_sent_time = *last_sent_time;
    return HTTP_IDLE_TIME - (cur_time - cached_last_sent_time > HTTP_IDLE_TIME ? 0 : cur_time - cached_last_sent_time);
}

static DWORD CALLBACK rpcrt4_http_timer_thread(PVOID param)
{
    HttpTimerThreadData *data_in = param;
    HttpTimerThreadData data;
    DWORD timeout;

    data = *data_in;
    HeapFree(GetProcessHeap(), 0, data_in);

    for (timeout = HTTP_IDLE_TIME;
         WaitForSingleObject(data.timer_cancelled, timeout) == WAIT_TIMEOUT;
         timeout = rpcrt4_http_timer_calc_timeout(data.last_sent_time))
    {
        /* are we too soon after last send? */
        if (GetTickCount() - HTTP_IDLE_TIME < *data.last_sent_time)
            continue;
        rpcrt4_http_keep_connection_active_timer_proc(data.timer_param, TRUE);
    }

    CloseHandle(data.timer_cancelled);
    return 0;
}

static VOID WINAPI rpcrt4_http_internet_callback(
     HINTERNET hInternet,
     DWORD_PTR dwContext,
     DWORD dwInternetStatus,
     LPVOID lpvStatusInformation,
     DWORD dwStatusInformationLength)
{
    RpcHttpAsyncData *async_data = (RpcHttpAsyncData *)dwContext;

    switch (dwInternetStatus)
    {
    case INTERNET_STATUS_REQUEST_COMPLETE:
        TRACE("INTERNET_STATUS_REQUEST_COMPLETED\n");
        if (async_data)
        {
            if (async_data->inet_buffers.lpvBuffer)
            {
                EnterCriticalSection(&async_data->cs);
                if (async_data->destination_buffer)
                {
                    memcpy(async_data->destination_buffer,
                           async_data->inet_buffers.lpvBuffer,
                           async_data->inet_buffers.dwBufferLength);
                    async_data->destination_buffer = NULL;
                }
                LeaveCriticalSection(&async_data->cs);
            }
            HeapFree(GetProcessHeap(), 0, async_data->inet_buffers.lpvBuffer);
            async_data->inet_buffers.lpvBuffer = NULL;
            SetEvent(async_data->completion_event);
            RpcHttpAsyncData_Release(async_data);
        }
        break;
    }
}

static RPC_STATUS rpcrt4_http_check_response(HINTERNET hor)
{
    BOOL ret;
    DWORD status_code;
    DWORD size;
    DWORD index;
    WCHAR buf[32];
    WCHAR *status_text = buf;
    TRACE("\n");

    index = 0;
    size = sizeof(status_code);
    ret = HttpQueryInfoW(hor, HTTP_QUERY_STATUS_CODE|HTTP_QUERY_FLAG_NUMBER, &status_code, &size, &index);
    if (!ret)
        return GetLastError();
    if (status_code < 400)
        return RPC_S_OK;
    index = 0;
    size = sizeof(buf);
    ret = HttpQueryInfoW(hor, HTTP_QUERY_STATUS_TEXT, status_text, &size, &index);
    if (!ret && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        status_text = HeapAlloc(GetProcessHeap(), 0, size);
        ret = HttpQueryInfoW(hor, HTTP_QUERY_STATUS_TEXT, status_text, &size, &index);
    }

    ERR("server returned: %d %s\n", status_code, ret ? debugstr_w(status_text) : "<status text unavailable>");
    if(status_text != buf) HeapFree(GetProcessHeap(), 0, status_text);

    if (status_code == HTTP_STATUS_DENIED)
        return ERROR_ACCESS_DENIED;
    return RPC_S_SERVER_UNAVAILABLE;
}

static RPC_STATUS rpcrt4_http_internet_connect(RpcConnection_http *httpc)
{
    static const WCHAR wszUserAgent[] = {'M','S','R','P','C',0};
    LPWSTR proxy = NULL;
    LPWSTR user = NULL;
    LPWSTR password = NULL;
    LPWSTR servername = NULL;
    const WCHAR *option;
    INTERNET_PORT port = INTERNET_INVALID_PORT_NUMBER; /* use default port */

    if (httpc->common.QOS &&
        (httpc->common.QOS->qos->AdditionalSecurityInfoType == RPC_C_AUTHN_INFO_TYPE_HTTP))
    {
        const RPC_HTTP_TRANSPORT_CREDENTIALS_W *http_cred = httpc->common.QOS->qos->u.HttpCredentials;
        if (http_cred->TransportCredentials)
        {
            WCHAR *p;
            const SEC_WINNT_AUTH_IDENTITY_W *cred = http_cred->TransportCredentials;
            ULONG len = cred->DomainLength + 1 + cred->UserLength;
            user = HeapAlloc(GetProcessHeap(), 0, (len + 1) * sizeof(WCHAR));
            if (!user)
                return RPC_S_OUT_OF_RESOURCES;
            p = user;
            if (cred->DomainLength)
            {
                memcpy(p, cred->Domain, cred->DomainLength * sizeof(WCHAR));
                p += cred->DomainLength;
                *p = '\\';
                p++;
            }
            memcpy(p, cred->User, cred->UserLength * sizeof(WCHAR));
            p[cred->UserLength] = 0;

            password = RPCRT4_strndupW(cred->Password, cred->PasswordLength);
        }
    }

    for (option = httpc->common.NetworkOptions; option;
         option = (strchrW(option, ',') ? strchrW(option, ',')+1 : NULL))
    {
        static const WCHAR wszRpcProxy[] = {'R','p','c','P','r','o','x','y','=',0};
        static const WCHAR wszHttpProxy[] = {'H','t','t','p','P','r','o','x','y','=',0};

        if (!strncmpiW(option, wszRpcProxy, sizeof(wszRpcProxy)/sizeof(wszRpcProxy[0])-1))
        {
            const WCHAR *value_start = option + sizeof(wszRpcProxy)/sizeof(wszRpcProxy[0])-1;
            const WCHAR *value_end;
            const WCHAR *p;

            value_end = strchrW(option, ',');
            if (!value_end)
                value_end = value_start + strlenW(value_start);
            for (p = value_start; p < value_end; p++)
                if (*p == ':')
                {
                    port = atoiW(p+1);
                    value_end = p;
                    break;
                }
            TRACE("RpcProxy value is %s\n", debugstr_wn(value_start, value_end-value_start));
            servername = RPCRT4_strndupW(value_start, value_end-value_start);
        }
        else if (!strncmpiW(option, wszHttpProxy, sizeof(wszHttpProxy)/sizeof(wszHttpProxy[0])-1))
        {
            const WCHAR *value_start = option + sizeof(wszHttpProxy)/sizeof(wszHttpProxy[0])-1;
            const WCHAR *value_end;

            value_end = strchrW(option, ',');
            if (!value_end)
                value_end = value_start + strlenW(value_start);
            TRACE("HttpProxy value is %s\n", debugstr_wn(value_start, value_end-value_start));
            proxy = RPCRT4_strndupW(value_start, value_end-value_start);
        }
        else
            FIXME("unhandled option %s\n", debugstr_w(option));
    }

    httpc->app_info = InternetOpenW(wszUserAgent, proxy ? INTERNET_OPEN_TYPE_PROXY : INTERNET_OPEN_TYPE_PRECONFIG,
                                    NULL, NULL, INTERNET_FLAG_ASYNC);
    if (!httpc->app_info)
    {
        HeapFree(GetProcessHeap(), 0, password);
        HeapFree(GetProcessHeap(), 0, user);
        ERR("InternetOpenW failed with error %d\n", GetLastError());
        return RPC_S_SERVER_UNAVAILABLE;
    }
    InternetSetStatusCallbackW(httpc->app_info, rpcrt4_http_internet_callback);

    /* if no RpcProxy option specified, set the HTTP server address to the
     * RPC server address */
    if (!servername)
    {
        servername = HeapAlloc(GetProcessHeap(), 0, (strlen(httpc->common.NetworkAddr) + 1)*sizeof(WCHAR));
        if (!servername)
        {
            HeapFree(GetProcessHeap(), 0, password);
            HeapFree(GetProcessHeap(), 0, user);
            return RPC_S_OUT_OF_RESOURCES;
        }
        MultiByteToWideChar(CP_ACP, 0, httpc->common.NetworkAddr, -1, servername, strlen(httpc->common.NetworkAddr) + 1);
    }

    httpc->session = InternetConnectW(httpc->app_info, servername, port, user, password,
                                      INTERNET_SERVICE_HTTP, 0, 0);

    HeapFree(GetProcessHeap(), 0, password);
    HeapFree(GetProcessHeap(), 0, user);
    HeapFree(GetProcessHeap(), 0, servername);

    if (!httpc->session)
    {
        ERR("InternetConnectW failed with error %d\n", GetLastError());
        return RPC_S_SERVER_UNAVAILABLE;
    }

    return RPC_S_OK;
}

/* prepare the in pipe for use by RPC packets */
static RPC_STATUS rpcrt4_http_prepare_in_pipe(HINTERNET in_request, RpcHttpAsyncData *async_data,
                                              const UUID *connection_uuid,
                                              const UUID *in_pipe_uuid,
                                              const UUID *association_uuid)
{
    BYTE packet[44];
    BOOL ret;
    RPC_STATUS status;
    RpcPktHdr *hdr;
    INTERNET_BUFFERSW buffers_in;
    DWORD bytes_read, bytes_written;

    /* prepare in pipe */
    ResetEvent(async_data->completion_event);
    RpcHttpAsyncData_AddRef(async_data);
    ret = HttpSendRequestW(in_request, NULL, 0, NULL, 0);
    if (!ret)
    {
        if (GetLastError() == ERROR_IO_PENDING)
            WaitForSingleObject(async_data->completion_event, INFINITE);
        else
        {
            RpcHttpAsyncData_Release(async_data);
            ERR("HttpSendRequestW failed with error %d\n", GetLastError());
            return RPC_S_SERVER_UNAVAILABLE;
        }
    }
    status = rpcrt4_http_check_response(in_request);
    if (status != RPC_S_OK) return status;

    InternetReadFile(in_request, packet, 20, &bytes_read);
    /* FIXME: do something with retrieved data */

    memset(&buffers_in, 0, sizeof(buffers_in));
    buffers_in.dwStructSize = sizeof(buffers_in);
    /* FIXME: get this from the registry */
    buffers_in.dwBufferTotal = 1024 * 1024 * 1024; /* 1Gb */
    ResetEvent(async_data->completion_event);
    RpcHttpAsyncData_AddRef(async_data);
    ret = HttpSendRequestExW(in_request, &buffers_in, NULL, 0, 0);
    if (!ret)
    {
        if (GetLastError() == ERROR_IO_PENDING)
            WaitForSingleObject(async_data->completion_event, INFINITE);
        else
        {
            RpcHttpAsyncData_Release(async_data);
            ERR("HttpSendRequestExW failed with error %d\n", GetLastError());
            return RPC_S_SERVER_UNAVAILABLE;
        }
    }

    TRACE("sending HTTP connect header to server\n");
    hdr = RPCRT4_BuildHttpConnectHeader(0, FALSE, connection_uuid, in_pipe_uuid, association_uuid);
    if (!hdr) return RPC_S_OUT_OF_RESOURCES;
    ret = InternetWriteFile(in_request, hdr, hdr->common.frag_len, &bytes_written);
    RPCRT4_FreeHeader(hdr);
    if (!ret)
    {
        ERR("InternetWriteFile failed with error %d\n", GetLastError());
        return RPC_S_SERVER_UNAVAILABLE;
    }

    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_http_read_http_packet(HINTERNET request, RpcPktHdr *hdr, BYTE **data)
{
    BOOL ret;
    DWORD bytes_read;
    unsigned short data_len;

    ret = InternetReadFile(request, hdr, sizeof(hdr->common), &bytes_read);
    if (!ret)
        return RPC_S_SERVER_UNAVAILABLE;
    if (hdr->common.ptype != PKT_HTTP || hdr->common.frag_len < sizeof(hdr->http))
    {
        ERR("wrong packet type received %d or wrong frag_len %d\n",
            hdr->common.ptype, hdr->common.frag_len);
        return RPC_S_PROTOCOL_ERROR;
    }

    ret = InternetReadFile(request, &hdr->common + 1, sizeof(hdr->http) - sizeof(hdr->common), &bytes_read);
    if (!ret)
        return RPC_S_SERVER_UNAVAILABLE;

    data_len = hdr->common.frag_len - sizeof(hdr->http);
    if (data_len)
    {
        *data = HeapAlloc(GetProcessHeap(), 0, data_len);
        if (!*data)
            return RPC_S_OUT_OF_RESOURCES;
        ret = InternetReadFile(request, *data, data_len, &bytes_read);
        if (!ret)
        {
            HeapFree(GetProcessHeap(), 0, *data);
            return RPC_S_SERVER_UNAVAILABLE;
        }
    }
    else
        *data = NULL;

    if (!RPCRT4_IsValidHttpPacket(hdr, *data, data_len))
    {
        ERR("invalid http packet\n");
        return RPC_S_PROTOCOL_ERROR;
    }

    return RPC_S_OK;
}

/* prepare the out pipe for use by RPC packets */
static RPC_STATUS rpcrt4_http_prepare_out_pipe(HINTERNET out_request,
                                               RpcHttpAsyncData *async_data,
                                               const UUID *connection_uuid,
                                               const UUID *out_pipe_uuid,
                                               ULONG *flow_control_increment)
{
    BYTE packet[20];
    BOOL ret;
    RPC_STATUS status;
    RpcPktHdr *hdr;
    DWORD bytes_read;
    BYTE *data_from_server;
    RpcPktHdr pkt_from_server;
    ULONG field1, field3;

    ResetEvent(async_data->completion_event);
    RpcHttpAsyncData_AddRef(async_data);
    ret = HttpSendRequestW(out_request, NULL, 0, NULL, 0);
    if (!ret)
    {
        if (GetLastError() == ERROR_IO_PENDING)
            WaitForSingleObject(async_data->completion_event, INFINITE);
        else
        {
            RpcHttpAsyncData_Release(async_data);
            ERR("HttpSendRequestW failed with error %d\n", GetLastError());
            return RPC_S_SERVER_UNAVAILABLE;
        }
    }
    status = rpcrt4_http_check_response(out_request);
    if (status != RPC_S_OK) return status;

    InternetReadFile(out_request, packet, 20, &bytes_read);
    /* FIXME: do something with retrieved data */

    hdr = RPCRT4_BuildHttpConnectHeader(0, TRUE, connection_uuid, out_pipe_uuid, NULL);
    if (!hdr) return RPC_S_OUT_OF_RESOURCES;
    ResetEvent(async_data->completion_event);
    RpcHttpAsyncData_AddRef(async_data);
    ret = HttpSendRequestW(out_request, NULL, 0, hdr, hdr->common.frag_len);
    if (!ret)
    {
        if (GetLastError() == ERROR_IO_PENDING)
            WaitForSingleObject(async_data->completion_event, INFINITE);
        else
        {
            RpcHttpAsyncData_Release(async_data);
            ERR("HttpSendRequestW failed with error %d\n", GetLastError());
            RPCRT4_FreeHeader(hdr);
            return RPC_S_SERVER_UNAVAILABLE;
        }
    }
    RPCRT4_FreeHeader(hdr);
    status = rpcrt4_http_check_response(out_request);
    if (status != RPC_S_OK) return status;

    status = rpcrt4_http_read_http_packet(out_request, &pkt_from_server,
                                          &data_from_server);
    if (status != RPC_S_OK) return status;
    status = RPCRT4_ParseHttpPrepareHeader1(&pkt_from_server, data_from_server,
                                            &field1);
    HeapFree(GetProcessHeap(), 0, data_from_server);
    if (status != RPC_S_OK) return status;
    TRACE("received (%d) from first prepare header\n", field1);

    status = rpcrt4_http_read_http_packet(out_request, &pkt_from_server,
                                          &data_from_server);
    if (status != RPC_S_OK) return status;
    status = RPCRT4_ParseHttpPrepareHeader2(&pkt_from_server, data_from_server,
                                            &field1, flow_control_increment,
                                            &field3);
    HeapFree(GetProcessHeap(), 0, data_from_server);
    if (status != RPC_S_OK) return status;
    TRACE("received (0x%08x 0x%08x %d) from second prepare header\n", field1, *flow_control_increment, field3);

    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_ncacn_http_open(RpcConnection* Connection)
{
    RpcConnection_http *httpc = (RpcConnection_http *)Connection;
    static const WCHAR wszVerbIn[] = {'R','P','C','_','I','N','_','D','A','T','A',0};
    static const WCHAR wszVerbOut[] = {'R','P','C','_','O','U','T','_','D','A','T','A',0};
    static const WCHAR wszRpcProxyPrefix[] = {'/','r','p','c','/','r','p','c','p','r','o','x','y','.','d','l','l','?',0};
    static const WCHAR wszColon[] = {':',0};
    static const WCHAR wszAcceptType[] = {'a','p','p','l','i','c','a','t','i','o','n','/','r','p','c',0};
    LPCWSTR wszAcceptTypes[] = { wszAcceptType, NULL };
    WCHAR *url;
    RPC_STATUS status;
    BOOL secure;
    HttpTimerThreadData *timer_data;
    HANDLE thread;

    TRACE("(%s, %s)\n", Connection->NetworkAddr, Connection->Endpoint);

    if (Connection->server)
    {
        ERR("ncacn_http servers not supported yet\n");
        return RPC_S_SERVER_UNAVAILABLE;
    }

    if (httpc->in_request)
        return RPC_S_OK;

    httpc->async_data->completion_event = CreateEventW(NULL, FALSE, FALSE, NULL);

    status = UuidCreate(&httpc->connection_uuid);
    status = UuidCreate(&httpc->in_pipe_uuid);
    status = UuidCreate(&httpc->out_pipe_uuid);

    status = rpcrt4_http_internet_connect(httpc);
    if (status != RPC_S_OK)
        return status;

    url = HeapAlloc(GetProcessHeap(), 0, sizeof(wszRpcProxyPrefix) + (strlen(Connection->NetworkAddr) + 1 + strlen(Connection->Endpoint))*sizeof(WCHAR));
    if (!url)
        return RPC_S_OUT_OF_MEMORY;
    memcpy(url, wszRpcProxyPrefix, sizeof(wszRpcProxyPrefix));
    MultiByteToWideChar(CP_ACP, 0, Connection->NetworkAddr, -1, url+sizeof(wszRpcProxyPrefix)/sizeof(wszRpcProxyPrefix[0])-1, strlen(Connection->NetworkAddr)+1);
    strcatW(url, wszColon);
    MultiByteToWideChar(CP_ACP, 0, Connection->Endpoint, -1, url+strlenW(url), strlen(Connection->Endpoint)+1);

    secure = httpc->common.QOS &&
             (httpc->common.QOS->qos->AdditionalSecurityInfoType == RPC_C_AUTHN_INFO_TYPE_HTTP) &&
             (httpc->common.QOS->qos->u.HttpCredentials->Flags & RPC_C_HTTP_FLAG_USE_SSL);

    httpc->in_request = HttpOpenRequestW(httpc->session, wszVerbIn, url, NULL, NULL,
                                         wszAcceptTypes,
                                         (secure ? INTERNET_FLAG_SECURE : 0)|INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_PRAGMA_NOCACHE,
                                         (DWORD_PTR)httpc->async_data);
    if (!httpc->in_request)
    {
        ERR("HttpOpenRequestW failed with error %d\n", GetLastError());
        return RPC_S_SERVER_UNAVAILABLE;
    }
    httpc->out_request = HttpOpenRequestW(httpc->session, wszVerbOut, url, NULL, NULL,
                                          wszAcceptTypes,
                                          (secure ? INTERNET_FLAG_SECURE : 0)|INTERNET_FLAG_KEEP_CONNECTION|INTERNET_FLAG_PRAGMA_NOCACHE,
                                          (DWORD_PTR)httpc->async_data);
    if (!httpc->out_request)
    {
        ERR("HttpOpenRequestW failed with error %d\n", GetLastError());
        return RPC_S_SERVER_UNAVAILABLE;
    }

    status = rpcrt4_http_prepare_in_pipe(httpc->in_request,
                                         httpc->async_data,
                                         &httpc->connection_uuid,
                                         &httpc->in_pipe_uuid,
                                         &Connection->assoc->http_uuid);
    if (status != RPC_S_OK)
        return status;

    status = rpcrt4_http_prepare_out_pipe(httpc->out_request,
                                          httpc->async_data,
                                          &httpc->connection_uuid,
                                          &httpc->out_pipe_uuid,
                                          &httpc->flow_control_increment);
    if (status != RPC_S_OK)
        return status;

    httpc->flow_control_mark = httpc->flow_control_increment / 2;
    httpc->last_sent_time = GetTickCount();
    httpc->timer_cancelled = CreateEventW(NULL, FALSE, FALSE, NULL);

    timer_data = HeapAlloc(GetProcessHeap(), 0, sizeof(*timer_data));
    if (!timer_data)
        return ERROR_OUTOFMEMORY;
    timer_data->timer_param = httpc->in_request;
    timer_data->last_sent_time = &httpc->last_sent_time;
    timer_data->timer_cancelled = httpc->timer_cancelled;
    /* FIXME: should use CreateTimerQueueTimer when implemented */
    thread = CreateThread(NULL, 0, rpcrt4_http_timer_thread, timer_data, 0, NULL);
    if (!thread)
    {
        HeapFree(GetProcessHeap(), 0, timer_data);
        return GetLastError();
    }
    CloseHandle(thread);

    return RPC_S_OK;
}

static RPC_STATUS rpcrt4_ncacn_http_handoff(RpcConnection *old_conn, RpcConnection *new_conn)
{
    assert(0);
    return RPC_S_SERVER_UNAVAILABLE;
}

static int rpcrt4_ncacn_http_read(RpcConnection *Connection,
                                void *buffer, unsigned int count)
{
  RpcConnection_http *httpc = (RpcConnection_http *) Connection;
  char *buf = buffer;
  BOOL ret = TRUE;
  unsigned int bytes_left = count;

  ResetEvent(httpc->async_data->completion_event);
  while (bytes_left)
  {
    RpcHttpAsyncData_AddRef(httpc->async_data);
    httpc->async_data->inet_buffers.dwBufferLength = bytes_left;
    httpc->async_data->inet_buffers.lpvBuffer = HeapAlloc(GetProcessHeap(), 0, bytes_left);
    httpc->async_data->destination_buffer = buf;
    ret = InternetReadFileExA(httpc->out_request, &httpc->async_data->inet_buffers, IRF_ASYNC, 0);
    if (ret)
    {
        /* INTERNET_STATUS_REQUEST_COMPLETED won't be sent, so release our
         * async ref now */
        RpcHttpAsyncData_Release(httpc->async_data);
        memcpy(buf, httpc->async_data->inet_buffers.lpvBuffer,
               httpc->async_data->inet_buffers.dwBufferLength);
        HeapFree(GetProcessHeap(), 0, httpc->async_data->inet_buffers.lpvBuffer);
        httpc->async_data->inet_buffers.lpvBuffer = NULL;
        httpc->async_data->destination_buffer = NULL;
    }
    else
    {
        if (GetLastError() == ERROR_IO_PENDING)
        {
            HANDLE handles[2] = { httpc->async_data->completion_event, httpc->cancel_event };
            DWORD result = WaitForMultipleObjects(2, handles, FALSE, DEFAULT_NCACN_HTTP_TIMEOUT);
            if (result == WAIT_OBJECT_0)
                ret = TRUE;
            else
            {
                TRACE("call cancelled\n");
                EnterCriticalSection(&httpc->async_data->cs);
                httpc->async_data->destination_buffer = NULL;
                LeaveCriticalSection(&httpc->async_data->cs);
                break;
            }
        }
        else
        {
            HeapFree(GetProcessHeap(), 0, httpc->async_data->inet_buffers.lpvBuffer);
            httpc->async_data->inet_buffers.lpvBuffer = NULL;
            httpc->async_data->destination_buffer = NULL;
            RpcHttpAsyncData_Release(httpc->async_data);
            break;
        }
    }
    if (!httpc->async_data->inet_buffers.dwBufferLength)
        break;
    bytes_left -= httpc->async_data->inet_buffers.dwBufferLength;
    buf += httpc->async_data->inet_buffers.dwBufferLength;
  }
  TRACE("%p %p %u -> %s\n", httpc->out_request, buffer, count, ret ? "TRUE" : "FALSE");
  return ret ? count : -1;
}

static RPC_STATUS rpcrt4_ncacn_http_receive_fragment(RpcConnection *Connection, RpcPktHdr **Header, void **Payload)
{
  RpcConnection_http *httpc = (RpcConnection_http *) Connection;
  RPC_STATUS status;
  DWORD hdr_length;
  LONG dwRead;
  RpcPktCommonHdr common_hdr;

  *Header = NULL;

  TRACE("(%p, %p, %p)\n", Connection, Header, Payload);

again:
  /* read packet common header */
  dwRead = rpcrt4_ncacn_http_read(Connection, &common_hdr, sizeof(common_hdr));
  if (dwRead != sizeof(common_hdr)) {
    WARN("Short read of header, %d bytes\n", dwRead);
    status = RPC_S_PROTOCOL_ERROR;
    goto fail;
  }
  if (!memcmp(&common_hdr, "HTTP/1.1", sizeof("HTTP/1.1")) ||
      !memcmp(&common_hdr, "HTTP/1.0", sizeof("HTTP/1.0")))
  {
    FIXME("server returned %s\n", debugstr_a((const char *)&common_hdr));
    status = RPC_S_PROTOCOL_ERROR;
    goto fail;
  }

  status = RPCRT4_ValidateCommonHeader(&common_hdr);
  if (status != RPC_S_OK) goto fail;

  hdr_length = RPCRT4_GetHeaderSize((RpcPktHdr*)&common_hdr);
  if (hdr_length == 0) {
    WARN("header length == 0\n");
    status = RPC_S_PROTOCOL_ERROR;
    goto fail;
  }

  *Header = HeapAlloc(GetProcessHeap(), 0, hdr_length);
  if (!*Header)
  {
    status = RPC_S_OUT_OF_RESOURCES;
    goto fail;
  }
  memcpy(*Header, &common_hdr, sizeof(common_hdr));

  /* read the rest of packet header */
  dwRead = rpcrt4_ncacn_http_read(Connection, &(*Header)->common + 1, hdr_length - sizeof(common_hdr));
  if (dwRead != hdr_length - sizeof(common_hdr)) {
    WARN("bad header length, %d bytes, hdr_length %d\n", dwRead, hdr_length);
    status = RPC_S_PROTOCOL_ERROR;
    goto fail;
  }

  if (common_hdr.frag_len - hdr_length)
  {
    *Payload = HeapAlloc(GetProcessHeap(), 0, common_hdr.frag_len - hdr_length);
    if (!*Payload)
    {
      status = RPC_S_OUT_OF_RESOURCES;
      goto fail;
    }

    dwRead = rpcrt4_ncacn_http_read(Connection, *Payload, common_hdr.frag_len - hdr_length);
    if (dwRead != common_hdr.frag_len - hdr_length)
    {
      WARN("bad data length, %d/%d\n", dwRead, common_hdr.frag_len - hdr_length);
      status = RPC_S_PROTOCOL_ERROR;
      goto fail;
    }
  }
  else
    *Payload = NULL;

  if ((*Header)->common.ptype == PKT_HTTP)
  {
    if (!RPCRT4_IsValidHttpPacket(*Header, *Payload, common_hdr.frag_len - hdr_length))
    {
      ERR("invalid http packet of length %d bytes\n", (*Header)->common.frag_len);
      status = RPC_S_PROTOCOL_ERROR;
      goto fail;
    }
    if ((*Header)->http.flags == 0x0001)
    {
      TRACE("http idle packet, waiting for real packet\n");
      if ((*Header)->http.num_data_items != 0)
      {
        ERR("HTTP idle packet should have no data items instead of %d\n", (*Header)->http.num_data_items);
        status = RPC_S_PROTOCOL_ERROR;
        goto fail;
      }
    }
    else if ((*Header)->http.flags == 0x0002)
    {
      ULONG bytes_transmitted;
      ULONG flow_control_increment;
      UUID pipe_uuid;
      status = RPCRT4_ParseHttpFlowControlHeader(*Header, *Payload,
                                                 Connection->server,
                                                 &bytes_transmitted,
                                                 &flow_control_increment,
                                                 &pipe_uuid);
      if (status != RPC_S_OK)
        goto fail;
      TRACE("received http flow control header (0x%x, 0x%x, %s)\n",
            bytes_transmitted, flow_control_increment, debugstr_guid(&pipe_uuid));
      /* FIXME: do something with parsed data */
    }
    else
    {
      FIXME("unrecognised http packet with flags 0x%04x\n", (*Header)->http.flags);
      status = RPC_S_PROTOCOL_ERROR;
      goto fail;
    }
    RPCRT4_FreeHeader(*Header);
    *Header = NULL;
    HeapFree(GetProcessHeap(), 0, *Payload);
    *Payload = NULL;
    goto again;
  }

  /* success */
  status = RPC_S_OK;

  httpc->bytes_received += common_hdr.frag_len;

  TRACE("httpc->bytes_received = 0x%x\n", httpc->bytes_received);

  if (httpc->bytes_received > httpc->flow_control_mark)
  {
    RpcPktHdr *hdr = RPCRT4_BuildHttpFlowControlHeader(httpc->common.server,
                                                       httpc->bytes_received,
                                                       httpc->flow_control_increment,
                                                       &httpc->out_pipe_uuid);
    if (hdr)
    {
      DWORD bytes_written;
      BOOL ret2;
      TRACE("sending flow control packet at 0x%x\n", httpc->bytes_received);
      ret2 = InternetWriteFile(httpc->in_request, hdr, hdr->common.frag_len, &bytes_written);
      RPCRT4_FreeHeader(hdr);
      if (ret2)
        httpc->flow_control_mark = httpc->bytes_received + httpc->flow_control_increment / 2;
    }
  }

fail:
  if (status != RPC_S_OK) {
    RPCRT4_FreeHeader(*Header);
    *Header = NULL;
    HeapFree(GetProcessHeap(), 0, *Payload);
    *Payload = NULL;
  }
  return status;
}

static int rpcrt4_ncacn_http_write(RpcConnection *Connection,
                                 const void *buffer, unsigned int count)
{
  RpcConnection_http *httpc = (RpcConnection_http *) Connection;
  DWORD bytes_written;
  BOOL ret;

  httpc->last_sent_time = ~0U; /* disable idle packet sending */
  ret = InternetWriteFile(httpc->in_request, buffer, count, &bytes_written);
  httpc->last_sent_time = GetTickCount();
  TRACE("%p %p %u -> %s\n", httpc->in_request, buffer, count, ret ? "TRUE" : "FALSE");
  return ret ? bytes_written : -1;
}

static int rpcrt4_ncacn_http_close(RpcConnection *Connection)
{
  RpcConnection_http *httpc = (RpcConnection_http *) Connection;

  TRACE("\n");

  SetEvent(httpc->timer_cancelled);
  if (httpc->in_request)
    InternetCloseHandle(httpc->in_request);
  httpc->in_request = NULL;
  if (httpc->out_request)
    InternetCloseHandle(httpc->out_request);
  httpc->out_request = NULL;
  if (httpc->app_info)
    InternetCloseHandle(httpc->app_info);
  httpc->app_info = NULL;
  if (httpc->session)
    InternetCloseHandle(httpc->session);
  httpc->session = NULL;
  RpcHttpAsyncData_Release(httpc->async_data);
  if (httpc->cancel_event)
    CloseHandle(httpc->cancel_event);

  return 0;
}

static void rpcrt4_ncacn_http_cancel_call(RpcConnection *Connection)
{
  RpcConnection_http *httpc = (RpcConnection_http *) Connection;

  SetEvent(httpc->cancel_event);
}

static int rpcrt4_ncacn_http_wait_for_incoming_data(RpcConnection *Connection)
{
  BOOL ret;
  RpcConnection_http *httpc = (RpcConnection_http *) Connection;

  RpcHttpAsyncData_AddRef(httpc->async_data);
  ret = InternetQueryDataAvailable(httpc->out_request,
    &httpc->async_data->inet_buffers.dwBufferLength, IRF_ASYNC, 0);
  if (ret)
  {
      /* INTERNET_STATUS_REQUEST_COMPLETED won't be sent, so release our
       * async ref now */
      RpcHttpAsyncData_Release(httpc->async_data);
  }
  else
  {
    if (GetLastError() == ERROR_IO_PENDING)
    {
      HANDLE handles[2] = { httpc->async_data->completion_event, httpc->cancel_event };
      DWORD result = WaitForMultipleObjects(2, handles, FALSE, DEFAULT_NCACN_HTTP_TIMEOUT);
      if (result != WAIT_OBJECT_0)
      {
        TRACE("call cancelled\n");
        return -1;
      }
    }
    else
    {
      RpcHttpAsyncData_Release(httpc->async_data);
      return -1;
    }
  }

  /* success */
  return 0;
}

static size_t rpcrt4_ncacn_http_get_top_of_tower(unsigned char *tower_data,
                                                 const char *networkaddr,
                                                 const char *endpoint)
{
    return rpcrt4_ip_tcp_get_top_of_tower(tower_data, networkaddr,
                                          EPM_PROTOCOL_HTTP, endpoint);
}

static RPC_STATUS rpcrt4_ncacn_http_parse_top_of_tower(const unsigned char *tower_data,
                                                       size_t tower_size,
                                                       char **networkaddr,
                                                       char **endpoint)
{
    return rpcrt4_ip_tcp_parse_top_of_tower(tower_data, tower_size,
                                            networkaddr, EPM_PROTOCOL_HTTP,
                                            endpoint);
}

static const struct connection_ops conn_protseq_list[] = {
  { "ncacn_np",
    { EPM_PROTOCOL_NCACN, EPM_PROTOCOL_SMB },
    rpcrt4_conn_np_alloc,
    rpcrt4_ncacn_np_open,
    rpcrt4_ncacn_np_handoff,
    rpcrt4_conn_np_read,
    rpcrt4_conn_np_write,
    rpcrt4_conn_np_close,
    rpcrt4_conn_np_cancel_call,
    rpcrt4_conn_np_wait_for_incoming_data,
    rpcrt4_ncacn_np_get_top_of_tower,
    rpcrt4_ncacn_np_parse_top_of_tower,
    NULL,
    RPCRT4_default_is_authorized,
    RPCRT4_default_authorize,
    RPCRT4_default_secure_packet,
    rpcrt4_conn_np_impersonate_client,
    rpcrt4_conn_np_revert_to_self,
    RPCRT4_default_inquire_auth_client,
  },
  { "ncalrpc",
    { EPM_PROTOCOL_NCALRPC, EPM_PROTOCOL_PIPE },
    rpcrt4_conn_np_alloc,
    rpcrt4_ncalrpc_open,
    rpcrt4_ncalrpc_handoff,
    rpcrt4_conn_np_read,
    rpcrt4_conn_np_write,
    rpcrt4_conn_np_close,
    rpcrt4_conn_np_cancel_call,
    rpcrt4_conn_np_wait_for_incoming_data,
    rpcrt4_ncalrpc_get_top_of_tower,
    rpcrt4_ncalrpc_parse_top_of_tower,
    NULL,
    rpcrt4_ncalrpc_is_authorized,
    rpcrt4_ncalrpc_authorize,
    rpcrt4_ncalrpc_secure_packet,
    rpcrt4_conn_np_impersonate_client,
    rpcrt4_conn_np_revert_to_self,
    rpcrt4_ncalrpc_inquire_auth_client,
  },
  { "ncacn_ip_tcp",
    { EPM_PROTOCOL_NCACN, EPM_PROTOCOL_TCP },
    rpcrt4_conn_tcp_alloc,
    rpcrt4_ncacn_ip_tcp_open,
    rpcrt4_conn_tcp_handoff,
    rpcrt4_conn_tcp_read,
    rpcrt4_conn_tcp_write,
    rpcrt4_conn_tcp_close,
    rpcrt4_conn_tcp_cancel_call,
    rpcrt4_conn_tcp_wait_for_incoming_data,
    rpcrt4_ncacn_ip_tcp_get_top_of_tower,
    rpcrt4_ncacn_ip_tcp_parse_top_of_tower,
    NULL,
    RPCRT4_default_is_authorized,
    RPCRT4_default_authorize,
    RPCRT4_default_secure_packet,
    RPCRT4_default_impersonate_client,
    RPCRT4_default_revert_to_self,
    RPCRT4_default_inquire_auth_client,
  },
  { "ncacn_http",
    { EPM_PROTOCOL_NCACN, EPM_PROTOCOL_HTTP },
    rpcrt4_ncacn_http_alloc,
    rpcrt4_ncacn_http_open,
    rpcrt4_ncacn_http_handoff,
    rpcrt4_ncacn_http_read,
    rpcrt4_ncacn_http_write,
    rpcrt4_ncacn_http_close,
    rpcrt4_ncacn_http_cancel_call,
    rpcrt4_ncacn_http_wait_for_incoming_data,
    rpcrt4_ncacn_http_get_top_of_tower,
    rpcrt4_ncacn_http_parse_top_of_tower,
    rpcrt4_ncacn_http_receive_fragment,
    RPCRT4_default_is_authorized,
    RPCRT4_default_authorize,
    RPCRT4_default_secure_packet,
    RPCRT4_default_impersonate_client,
    RPCRT4_default_revert_to_self,
    RPCRT4_default_inquire_auth_client,
  },
};


static const struct protseq_ops protseq_list[] =
{
    {
        "ncacn_np",
        rpcrt4_protseq_np_alloc,
        rpcrt4_protseq_np_signal_state_changed,
        rpcrt4_protseq_np_get_wait_array,
        rpcrt4_protseq_np_free_wait_array,
        rpcrt4_protseq_np_wait_for_new_connection,
        rpcrt4_protseq_ncacn_np_open_endpoint,
    },
    {
        "ncalrpc",
        rpcrt4_protseq_np_alloc,
        rpcrt4_protseq_np_signal_state_changed,
        rpcrt4_protseq_np_get_wait_array,
        rpcrt4_protseq_np_free_wait_array,
        rpcrt4_protseq_np_wait_for_new_connection,
        rpcrt4_protseq_ncalrpc_open_endpoint,
    },
    {
        "ncacn_ip_tcp",
        rpcrt4_protseq_sock_alloc,
        rpcrt4_protseq_sock_signal_state_changed,
        rpcrt4_protseq_sock_get_wait_array,
        rpcrt4_protseq_sock_free_wait_array,
        rpcrt4_protseq_sock_wait_for_new_connection,
        rpcrt4_protseq_ncacn_ip_tcp_open_endpoint,
    },
};

#define ARRAYSIZE(a) (sizeof((a)) / sizeof((a)[0]))

const struct protseq_ops *rpcrt4_get_protseq_ops(const char *protseq)
{
  unsigned int i;
  for(i=0; i<ARRAYSIZE(protseq_list); i++)
    if (!strcmp(protseq_list[i].name, protseq))
      return &protseq_list[i];
  return NULL;
}

static const struct connection_ops *rpcrt4_get_conn_protseq_ops(const char *protseq)
{
    unsigned int i;
    for(i=0; i<ARRAYSIZE(conn_protseq_list); i++)
        if (!strcmp(conn_protseq_list[i].name, protseq))
            return &conn_protseq_list[i];
    return NULL;
}

/**** interface to rest of code ****/

RPC_STATUS RPCRT4_OpenClientConnection(RpcConnection* Connection)
{
  TRACE("(Connection == ^%p)\n", Connection);

  assert(!Connection->server);
  return Connection->ops->open_connection_client(Connection);
}

RPC_STATUS RPCRT4_CloseConnection(RpcConnection* Connection)
{
  TRACE("(Connection == ^%p)\n", Connection);
  if (SecIsValidHandle(&Connection->ctx))
  {
    DeleteSecurityContext(&Connection->ctx);
    SecInvalidateHandle(&Connection->ctx);
  }
  rpcrt4_conn_close(Connection);
  return RPC_S_OK;
}

RPC_STATUS RPCRT4_CreateConnection(RpcConnection** Connection, BOOL server,
    LPCSTR Protseq, LPCSTR NetworkAddr, LPCSTR Endpoint,
    LPCWSTR NetworkOptions, RpcAuthInfo* AuthInfo, RpcQualityOfService *QOS)
{
  static LONG next_id;
  const struct connection_ops *ops;
  RpcConnection* NewConnection;

  ops = rpcrt4_get_conn_protseq_ops(Protseq);
  if (!ops)
  {
    FIXME("not supported for protseq %s\n", Protseq);
    return RPC_S_PROTSEQ_NOT_SUPPORTED;
  }

  NewConnection = ops->alloc();
  NewConnection->Next = NULL;
  NewConnection->server_binding = NULL;
  NewConnection->server = server;
  NewConnection->ops = ops;
  NewConnection->NetworkAddr = RPCRT4_strdupA(NetworkAddr);
  NewConnection->Endpoint = RPCRT4_strdupA(Endpoint);
  NewConnection->NetworkOptions = RPCRT4_strdupW(NetworkOptions);
  NewConnection->MaxTransmissionSize = RPC_MAX_PACKET_SIZE;
  memset(&NewConnection->ActiveInterface, 0, sizeof(NewConnection->ActiveInterface));
  NewConnection->NextCallId = 1;

  SecInvalidateHandle(&NewConnection->ctx);
  memset(&NewConnection->exp, 0, sizeof(NewConnection->exp));
  NewConnection->attr = 0;
  if (AuthInfo) RpcAuthInfo_AddRef(AuthInfo);
  NewConnection->AuthInfo = AuthInfo;
  NewConnection->auth_context_id = InterlockedIncrement( &next_id );
  NewConnection->encryption_auth_len = 0;
  NewConnection->signature_auth_len = 0;
  if (QOS) RpcQualityOfService_AddRef(QOS);
  NewConnection->QOS = QOS;

  list_init(&NewConnection->conn_pool_entry);
  NewConnection->async_state = NULL;

  TRACE("connection: %p\n", NewConnection);
  *Connection = NewConnection;

  return RPC_S_OK;
}

static RPC_STATUS RPCRT4_SpawnConnection(RpcConnection** Connection, RpcConnection* OldConnection)
{
  RPC_STATUS err;

  err = RPCRT4_CreateConnection(Connection, OldConnection->server,
                                rpcrt4_conn_get_name(OldConnection),
                                OldConnection->NetworkAddr,
                                OldConnection->Endpoint, NULL,
                                OldConnection->AuthInfo, OldConnection->QOS);
  if (err == RPC_S_OK)
    rpcrt4_conn_handoff(OldConnection, *Connection);
  return err;
}

RPC_STATUS RPCRT4_DestroyConnection(RpcConnection* Connection)
{
  TRACE("connection: %p\n", Connection);

  RPCRT4_CloseConnection(Connection);
  RPCRT4_strfree(Connection->Endpoint);
  RPCRT4_strfree(Connection->NetworkAddr);
  HeapFree(GetProcessHeap(), 0, Connection->NetworkOptions);
  if (Connection->AuthInfo) RpcAuthInfo_Release(Connection->AuthInfo);
  if (Connection->QOS) RpcQualityOfService_Release(Connection->QOS);

  /* server-only */
  if (Connection->server_binding) RPCRT4_ReleaseBinding(Connection->server_binding);

  HeapFree(GetProcessHeap(), 0, Connection);
  return RPC_S_OK;
}

RPC_STATUS RpcTransport_GetTopOfTower(unsigned char *tower_data,
                                      size_t *tower_size,
                                      const char *protseq,
                                      const char *networkaddr,
                                      const char *endpoint)
{
    twr_empty_floor_t *protocol_floor;
    const struct connection_ops *protseq_ops = rpcrt4_get_conn_protseq_ops(protseq);

    *tower_size = 0;

    if (!protseq_ops)
        return RPC_S_INVALID_RPC_PROTSEQ;

    if (!tower_data)
    {
        *tower_size = sizeof(*protocol_floor);
        *tower_size += protseq_ops->get_top_of_tower(NULL, networkaddr, endpoint);
        return RPC_S_OK;
    }

    protocol_floor = (twr_empty_floor_t *)tower_data;
    protocol_floor->count_lhs = sizeof(protocol_floor->protid);
    protocol_floor->protid = protseq_ops->epm_protocols[0];
    protocol_floor->count_rhs = 0;

    tower_data += sizeof(*protocol_floor);

    *tower_size = protseq_ops->get_top_of_tower(tower_data, networkaddr, endpoint);
    if (!*tower_size)
        return EPT_S_NOT_REGISTERED;

    *tower_size += sizeof(*protocol_floor);

    return RPC_S_OK;
}

RPC_STATUS RpcTransport_ParseTopOfTower(const unsigned char *tower_data,
                                        size_t tower_size,
                                        char **protseq,
                                        char **networkaddr,
                                        char **endpoint)
{
    const twr_empty_floor_t *protocol_floor;
    const twr_empty_floor_t *floor4;
    const struct connection_ops *protseq_ops = NULL;
    RPC_STATUS status;
    unsigned int i;

    if (tower_size < sizeof(*protocol_floor))
        return EPT_S_NOT_REGISTERED;

    protocol_floor = (const twr_empty_floor_t *)tower_data;
    tower_data += sizeof(*protocol_floor);
    tower_size -= sizeof(*protocol_floor);
    if ((protocol_floor->count_lhs != sizeof(protocol_floor->protid)) ||
        (protocol_floor->count_rhs > tower_size))
        return EPT_S_NOT_REGISTERED;
    tower_data += protocol_floor->count_rhs;
    tower_size -= protocol_floor->count_rhs;

    floor4 = (const twr_empty_floor_t *)tower_data;
    if ((tower_size < sizeof(*floor4)) ||
        (floor4->count_lhs != sizeof(floor4->protid)))
        return EPT_S_NOT_REGISTERED;

    for(i = 0; i < ARRAYSIZE(conn_protseq_list); i++)
        if ((protocol_floor->protid == conn_protseq_list[i].epm_protocols[0]) &&
            (floor4->protid == conn_protseq_list[i].epm_protocols[1]))
        {
            protseq_ops = &conn_protseq_list[i];
            break;
        }

    if (!protseq_ops)
        return EPT_S_NOT_REGISTERED;

    status = protseq_ops->parse_top_of_tower(tower_data, tower_size, networkaddr, endpoint);

    if ((status == RPC_S_OK) && protseq)
    {
        *protseq = I_RpcAllocate(strlen(protseq_ops->name) + 1);
        strcpy(*protseq, protseq_ops->name);
    }

    return status;
}

/***********************************************************************
 *             RpcNetworkIsProtseqValidW (RPCRT4.@)
 *
 * Checks if the given protocol sequence is known by the RPC system.
 * If it is, returns RPC_S_OK, otherwise RPC_S_PROTSEQ_NOT_SUPPORTED.
 *
 */
RPC_STATUS WINAPI RpcNetworkIsProtseqValidW(RPC_WSTR protseq)
{
  char ps[0x10];

  WideCharToMultiByte(CP_ACP, 0, protseq, -1,
                      ps, sizeof ps, NULL, NULL);
  if (rpcrt4_get_conn_protseq_ops(ps))
    return RPC_S_OK;

  FIXME("Unknown protseq %s\n", debugstr_w(protseq));

  return RPC_S_INVALID_RPC_PROTSEQ;
}

/***********************************************************************
 *             RpcNetworkIsProtseqValidA (RPCRT4.@)
 */
RPC_STATUS WINAPI RpcNetworkIsProtseqValidA(RPC_CSTR protseq)
{
  UNICODE_STRING protseqW;

  if (RtlCreateUnicodeStringFromAsciiz(&protseqW, (char*)protseq))
  {
    RPC_STATUS ret = RpcNetworkIsProtseqValidW(protseqW.Buffer);
    RtlFreeUnicodeString(&protseqW);
    return ret;
  }
  return RPC_S_OUT_OF_MEMORY;
}

/***********************************************************************
 *             RpcProtseqVectorFreeA (RPCRT4.@)
 */
RPC_STATUS WINAPI RpcProtseqVectorFreeA(RPC_PROTSEQ_VECTORA **protseqs)
{
  TRACE("(%p)\n", protseqs);

  if (*protseqs)
  {
    int i;
    for (i = 0; i < (*protseqs)->Count; i++)
      HeapFree(GetProcessHeap(), 0, (*protseqs)->Protseq[i]);
    HeapFree(GetProcessHeap(), 0, *protseqs);
    *protseqs = NULL;
  }
  return RPC_S_OK;
}

/***********************************************************************
 *             RpcProtseqVectorFreeW (RPCRT4.@)
 */
RPC_STATUS WINAPI RpcProtseqVectorFreeW(RPC_PROTSEQ_VECTORW **protseqs)
{
  TRACE("(%p)\n", protseqs);

  if (*protseqs)
  {
    int i;
    for (i = 0; i < (*protseqs)->Count; i++)
      HeapFree(GetProcessHeap(), 0, (*protseqs)->Protseq[i]);
    HeapFree(GetProcessHeap(), 0, *protseqs);
    *protseqs = NULL;
  }
  return RPC_S_OK;
}

/***********************************************************************
 *             RpcNetworkInqProtseqsW (RPCRT4.@)
 */
RPC_STATUS WINAPI RpcNetworkInqProtseqsW( RPC_PROTSEQ_VECTORW** protseqs )
{
  RPC_PROTSEQ_VECTORW *pvector;
  int i = 0;
  RPC_STATUS status = RPC_S_OUT_OF_MEMORY;

  TRACE("(%p)\n", protseqs);

  *protseqs = HeapAlloc(GetProcessHeap(), 0, sizeof(RPC_PROTSEQ_VECTORW)+(sizeof(unsigned short*)*ARRAYSIZE(protseq_list)));
  if (!*protseqs)
    goto end;
  pvector = *protseqs;
  pvector->Count = 0;
  for (i = 0; i < ARRAYSIZE(protseq_list); i++)
  {
    pvector->Protseq[i] = HeapAlloc(GetProcessHeap(), 0, (strlen(protseq_list[i].name)+1)*sizeof(unsigned short));
    if (pvector->Protseq[i] == NULL)
      goto end;
    MultiByteToWideChar(CP_ACP, 0, (CHAR*)protseq_list[i].name, -1,
      (WCHAR*)pvector->Protseq[i], strlen(protseq_list[i].name) + 1);
    pvector->Count++;
  }
  status = RPC_S_OK;

end:
  if (status != RPC_S_OK)
    RpcProtseqVectorFreeW(protseqs);
  return status;
}

/***********************************************************************
 *             RpcNetworkInqProtseqsA (RPCRT4.@)
 */
RPC_STATUS WINAPI RpcNetworkInqProtseqsA(RPC_PROTSEQ_VECTORA** protseqs)
{
  RPC_PROTSEQ_VECTORA *pvector;
  int i = 0;
  RPC_STATUS status = RPC_S_OUT_OF_MEMORY;

  TRACE("(%p)\n", protseqs);

  *protseqs = HeapAlloc(GetProcessHeap(), 0, sizeof(RPC_PROTSEQ_VECTORW)+(sizeof(unsigned char*)*ARRAYSIZE(protseq_list)));
  if (!*protseqs)
    goto end;
  pvector = *protseqs;
  pvector->Count = 0;
  for (i = 0; i < ARRAYSIZE(protseq_list); i++)
  {
    pvector->Protseq[i] = HeapAlloc(GetProcessHeap(), 0, strlen(protseq_list[i].name)+1);
    if (pvector->Protseq[i] == NULL)
      goto end;
    strcpy((char*)pvector->Protseq[i], protseq_list[i].name);
    pvector->Count++;
  }
  status = RPC_S_OK;

end:
  if (status != RPC_S_OK)
    RpcProtseqVectorFreeA(protseqs);
  return status;
}
