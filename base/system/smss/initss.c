/*
 * PROJECT:         Odyssey Session Manager
 * LICENSE:         GPL v2 or later - See COPYING in the top level directory
 * FILE:            base/system/smss/initss.c
 * PURPOSE:         Load the subsystems.
 * PROGRAMMERS:     Odyssey Development Team
 */

/* INCLUDES ******************************************************************/
#include "smss.h"

#define NDEBUG
#include <debug.h>

/* SM handle for its own \SmApiPort */
HANDLE hSmApiPort = (HANDLE) 0;


/* TODO:
 *
 * a) look if a special option is set for smss.exe in
 *    HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options
 */

/**********************************************************************
 *	SmRegisterInternalSubsystem/3
 *
 * DESCRIPTION
 *	Register with itself for ImageSubsystemId
 *	(programmatically).
 */
NTSTATUS NTAPI SmRegisterInternalSubsystem (LPWSTR PgmName,
					      USHORT ImageSubsystemId,
					      PHANDLE ApiPort)
{
	NTSTATUS                      Status = STATUS_SUCCESS;
	RTL_USER_PROCESS_INFORMATION  ProcessInfo;


	DPRINT("SM: %s(%S,%d) called\n",__FUNCTION__, PgmName, ImageSubsystemId);

	RtlZeroMemory (& ProcessInfo, sizeof ProcessInfo);
	ProcessInfo.Size = sizeof ProcessInfo;
	ProcessInfo.ProcessHandle = (HANDLE) UlongToPtr(SmSsProcessId);
	ProcessInfo.ClientId.UniqueProcess = (HANDLE) UlongToPtr(SmSsProcessId);
	DPRINT("SM: %s: ProcessInfo.ProcessHandle=%p\n",
		__FUNCTION__,ProcessInfo.ProcessHandle);
	Status = SmCreateClient (& ProcessInfo, PgmName);
	if (NT_SUCCESS(Status))
	{
		UNICODE_STRING SbApiPortName = {0,0,NULL};

		RtlInitUnicodeString (& SbApiPortName, L"");
		Status = SmConnectApiPort(& SbApiPortName,
					  (HANDLE) -1, /* internal SS have no SB port */
					  ImageSubsystemId,
					  ApiPort);
		if(!NT_SUCCESS(Status))
		{
			DPRINT("SM: %s: SMLIB!SmConnectApiPort failed (Status=0x%08lx)\n",
				__FUNCTION__,Status);
			return Status;
		}
		DPRINT("SM:%s: %S self registered\n", __FUNCTION__, PgmName);
	}
	else
	{
		DPRINT1("SM: %s: SmCreateClient(%S) failed (Status=0x%08lx)\n",
			__FUNCTION__, PgmName, Status);
	}
	/*
	 * Note that you don't need to call complete session
	 * here because connection handling code autocompletes
	 * the client structure for IMAGE_SUBSYSTEM_NATIVE and
	 * -1.
	 */
	return Status;
}
/**********************************************************************
 * 	SmpLoadRequiredSubsystems/0
 */
static NTSTATUS
SmpLoadRequiredSubsystems (VOID)
{
	NTSTATUS  Status = STATUS_SUCCESS;
	WCHAR     Data [MAX_PATH + 1];
	ULONG     DataLength = sizeof Data;
	ULONG     DataType = 0;


	DPRINT("SM: %s called\n", __FUNCTION__);

	RtlZeroMemory (Data, DataLength);
	Status = SmLookupSubsystem (L"Required",
				    Data,
				    & DataLength,
				    & DataType,
				    NULL);
	if((STATUS_SUCCESS == Status) && (DataLength > sizeof Data[0]))
	{
		PWCHAR Name = NULL;
		ULONG Offset = 0;

		for (Name = Data; (Offset < DataLength); )
		{
			if(L'\0' != *Name)
			{
				UNICODE_STRING Program;

				/* Run the current program */
				RtlInitUnicodeString (& Program, Name);
				Status = SmExecuteProgram (hSmApiPort, & Program);
				if(!NT_SUCCESS(Status))
				{
					DPRINT1("SM: %s failed to run '%S' program (Status=0x%08lx)\n",
						__FUNCTION__, Name, Status);
				}
				/* Look for the next program */
				while ((L'\0' != *Name) && (Offset < DataLength))
				{
					++ Name;
					++ Offset;
				}
			}
			++ Name;
			++ Offset;
		}
	}

	return Status;
}

/**********************************************************************
 * 	SmLoadSubsystems/0
 */
NTSTATUS
SmLoadSubsystems(VOID)
{
	NTSTATUS  Status = STATUS_SUCCESS;


	DPRINT("SM: loading subsystems...\n");

	/*
	 *  SM self registers: this also opens hSmApiPort to be used
	 * in loading required subsystems.
	 */
	Status = SmRegisterInternalSubsystem (L"Session Manager", IMAGE_SUBSYSTEM_NATIVE, & hSmApiPort);
	if(!NT_SUCCESS(Status))
	{
		DPRINT1("SM: %s SmRegisterInternalSubsystem failed Status=%08lx\n", __FUNCTION__, Status);
		return Status;
	}
	/* Load Required subsystems (Debug Windows) */
	Status = SmpLoadRequiredSubsystems();
	if(!NT_SUCCESS(Status))
	{
		DPRINT1("SM: %s SmpLoadRequiredSubsystems failed Status=%08lx\n", __FUNCTION__, Status);
		return Status;
	}
	/* done */
	DPRINT("SM: done loading subsystems\n");
	return Status;
}

/* EOF */
