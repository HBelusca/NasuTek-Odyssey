/* $Id: init.c 43790 2009-10-27 10:34:16Z dgorbachev $
 *
 * init.c - Odyssey/Win32 Console+User Enviroment Subsystem Server - Initialization
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
#include "winsrv.h"

//#define NDEBUG
#include <debug.h>

HANDLE WinSrvApiPort = NULL;

/**********************************************************************
 * NAME							PRIVATE
 * 	ConStaticServerThread/1
 */
VOID WINAPI ConStaticServerThread (PVOID x)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PPORT_MESSAGE Request = (PPORT_MESSAGE) x;
	PPORT_MESSAGE Reply = NULL;
	ULONG MessageType = 0;

	DPRINT("WINSRV: %s(%08lx) called\n", __FUNCTION__, x);

	MessageType = Request->u2.s2.Type;
	DPRINT("WINSRV: %s(%08lx) received a message (Type=%d)\n",
		__FUNCTION__, x, MessageType);
	switch (MessageType)
	{
		default:
			Reply = Request;
			Status = NtReplyPort (WinSrvApiPort, Reply);
			break;
	}
}

/**********************************************************************
 * NAME							PRIVATE
 * 	UserStaticServerThread/1
 */
VOID WINAPI UserStaticServerThread (PVOID x)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PPORT_MESSAGE Request = (PPORT_MESSAGE) x;
	PPORT_MESSAGE Reply = NULL;
	ULONG MessageType = 0;

	DPRINT("WINSRV: %s(%08lx) called\n", __FUNCTION__, x);

	MessageType = Request->u2.s2.Type;
	DPRINT("WINSRV: %s(%08lx) received a message (Type=%d)\n",
		__FUNCTION__, x, MessageType);
	switch (MessageType)
	{
		default:
			Reply = Request;
			Status = NtReplyPort (WinSrvApiPort, Reply);
			break;
	}
}

/*=====================================================================
 * 	PUBLIC API
 *===================================================================*/

NTSTATUS WINAPI ConServerDllInitialization (ULONG ArgumentCount,
					     LPWSTR *Argument)
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	DPRINT("WINSRV: %s called\n", __FUNCTION__);

	// Get the listening port from csrsrv.dll
	WinSrvApiPort = CsrQueryApiPort ();
	if (NULL == WinSrvApiPort)
	{
		return STATUS_UNSUCCESSFUL;
	}
	// Register our message dispatcher
	Status = CsrAddStaticServerThread (ConStaticServerThread);
	if (NT_SUCCESS(Status))
	{
		//TODO: perform the real console server internal initialization here
	}
	return Status;
}

NTSTATUS WINAPI UserServerDllInitialization (ULONG ArgumentCount,
					      LPWSTR *Argument)
{
	NTSTATUS Status = STATUS_SUCCESS;
	
	DPRINT("WINSRV: %s called\n", __FUNCTION__);

	// Get the listening port from csrsrv.dll
	WinSrvApiPort = CsrQueryApiPort ();
	if (NULL == WinSrvApiPort)
	{
		return STATUS_UNSUCCESSFUL;
	}
	// Register our message dispatcher
	Status = CsrAddStaticServerThread (UserStaticServerThread);
	if (NT_SUCCESS(Status))
	{
		//TODO: perform the real user server internal initialization here
	}
	return Status;
}

/* EOF */
