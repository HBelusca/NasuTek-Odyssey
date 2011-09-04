/*
 * iphlpapi dll implementation -- Setting and storing route information
 *
 * These are stubs for functions that set routing information on the target
 * operating system.  They are grouped here because their implementation will
 * vary widely by operating system.
 *
 * Copyright (C) 2004 Art Yerkes
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
 */

#include "config.h"
#include "iphlpapi_private.h"

#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
# include <arpa/nameser.h>
#endif
#ifdef HAVE_RESOLV_H
# include <resolv.h>
#endif

#include "windef.h"
#include "winbase.h"
#include "winreg.h"
#include "resinfo.h"
#include "iphlpapi.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(iphlpapi);

DWORD createIpForwardEntry( PMIB_IPFORWARDROW pRoute ) {
    TRACE("pRoute %p\n", pRoute);
  /* could use SIOCADDRT, not sure I want to */
  FIXME(":stub\n");
  return (DWORD) 0;
}

DWORD setIpForwardEntry( PMIB_IPFORWARDROW pRoute ) {
  FIXME(":stub\n");
  return (DWORD) 0;
}

DWORD deleteIpForwardEntry( PMIB_IPFORWARDROW pRoute ) {
  TRACE("pRoute %p\n", pRoute);
  /* could use SIOCDELRT, not sure I want to */
  FIXME(":stub\n");
  return (DWORD) 0;
}
