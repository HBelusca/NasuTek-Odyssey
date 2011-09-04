/* $Id: init.c 43790 2009-10-27 10:34:16Z dgorbachev $
 *
 * init.c - Odyssey/Win32 base enviroment subsystem server
 *
 * Odyssey Operating System
 *
 * --------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * --------------------------------------------------------------------
 */
#include "basesrv.h"

#define NDEBUG
#include <debug.h>

HANDLE BaseApiPort = (HANDLE) 0;

/**********************************************************************
 * NAME							PRIVATE
 * 	BaseStaticServerThread/1
 */
VOID WINAPI BaseStaticServerThread (PVOID x)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PPORT_MESSAGE Request = (PPORT_MESSAGE) x;
	PPORT_MESSAGE Reply = NULL;
	ULONG MessageType = 0;

	DPRINT("BASESRV: %s called\n", __FUNCTION__);

	MessageType = Request->u2.s2.Type;
	DPRINT("BASESRV: %s received a message (Type=%d)\n",
		__FUNCTION__, MessageType);
	switch (MessageType)
	{
		default:
			Reply = Request;
			Status = NtReplyPort (BaseApiPort, Reply);
			break;
	}
}


NTSTATUS WINAPI ServerDllInitialization (ULONG ArgumentCount, LPWSTR *Argument)
{
	NTSTATUS Status = STATUS_SUCCESS;

	DPRINT("BASSRV: %s(%ld,...) called\n", __FUNCTION__, ArgumentCount);

	BaseApiPort = CsrQueryApiPort ();
	Status = CsrAddStaticServerThread (BaseStaticServerThread);
	if (NT_SUCCESS(Status))
	{
		//TODO initialize the BASE server
	}
	return STATUS_SUCCESS;
}

/* EOF */
