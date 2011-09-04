/* $Id: shutdown.c 53225 2011-08-14 11:31:23Z akhaldi $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:     Odyssey system libraries
 * FILE:        lib/advapi32/misc/shutdown.c
 * PURPOSE:     System shutdown functions
 * PROGRAMMER:      Emanuele Aliberti
 * UPDATE HISTORY:
 *      19990413 EA     created
 *      19990515 EA
 */

#include <advapi32.h>
WINE_DEFAULT_DEBUG_CHANNEL(advapi);

#define USZ {0,0,0}

/**********************************************************************
 *      AbortSystemShutdownW
 *
 * @unimplemented
 */
BOOL WINAPI
AbortSystemShutdownW(LPCWSTR lpMachineName)
{
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/**********************************************************************
 *      AbortSystemShutdownA
 *
 * @unimplemented
 */
BOOL WINAPI
AbortSystemShutdownA(LPCSTR lpMachineName)
{
    ANSI_STRING MachineNameA;
    UNICODE_STRING MachineNameW;
    NTSTATUS Status;
    BOOL rv;

    RtlInitAnsiString(&MachineNameA, (LPSTR)lpMachineName);
    Status = RtlAnsiStringToUnicodeString(&MachineNameW, &MachineNameA, TRUE);
    if (STATUS_SUCCESS != Status)
    {
            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
    }

    rv = AbortSystemShutdownW(MachineNameW.Buffer);
    RtlFreeUnicodeString(&MachineNameW);
    SetLastError(ERROR_SUCCESS);
    return rv;
}


/**********************************************************************
 *      InitiateSystemShutdownW
 *
 * @unimplemented
 */
BOOL WINAPI
InitiateSystemShutdownW(LPWSTR lpMachineName,
                        LPWSTR lpMessage,
                        DWORD dwTimeout,
                        BOOL bForceAppsClosed,
                        BOOL bRebootAfterShutdown)
{
    SHUTDOWN_ACTION Action = ShutdownNoReboot;
    NTSTATUS Status;

    if (lpMachineName)
    {
        /* FIXME: remote machine shutdown not supported yet */
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (dwTimeout)
    {
    }

    Status = NtShutdownSystem(Action);
    SetLastError(RtlNtStatusToDosError(Status));
    return FALSE;
}


/**********************************************************************
 *      InitiateSystemShutdownA
 *
 * @unimplemented
 */
BOOL
WINAPI
InitiateSystemShutdownA(LPSTR lpMachineName,
                        LPSTR lpMessage,
                        DWORD dwTimeout,
                        BOOL bForceAppsClosed,
                        BOOL bRebootAfterShutdown)
{
    ANSI_STRING     MachineNameA;
    ANSI_STRING     MessageA;
    UNICODE_STRING  MachineNameW;
    UNICODE_STRING  MessageW;
    NTSTATUS        Status;
    INT         LastError;
    BOOL        rv;

	MachineNameW.Buffer = NULL;
	MessageW.Buffer = NULL;

    if (lpMachineName)
    {
        RtlInitAnsiString(&MachineNameA, lpMachineName);
        Status = RtlAnsiStringToUnicodeString(&MachineNameW, &MachineNameA, TRUE);
        if (STATUS_SUCCESS != Status)
        {
            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
        }
    }

    if (lpMessage)
    {
        RtlInitAnsiString(&MessageA, lpMessage);
        Status = RtlAnsiStringToUnicodeString(&MessageW, &MessageA, TRUE);
        if (STATUS_SUCCESS != Status)
        {
            if (MachineNameW.Buffer)
            {
                RtlFreeUnicodeString(&MachineNameW);
            }

            SetLastError(RtlNtStatusToDosError(Status));
            return FALSE;
        }
    }

    rv = InitiateSystemShutdownW(MachineNameW.Buffer,
                                 MessageW.Buffer,
                                 dwTimeout,
                                 bForceAppsClosed,
                                 bRebootAfterShutdown);
    LastError = GetLastError();
    if (lpMachineName)
    {
        RtlFreeUnicodeString(&MachineNameW);
    }

    if (lpMessage)
    {
        RtlFreeUnicodeString(&MessageW);
    }

    SetLastError(LastError);
    return rv;
}

/******************************************************************************
 * InitiateSystemShutdownExW [ADVAPI32.@]
 *
 * see InitiateSystemShutdownExA
 */
BOOL WINAPI
InitiateSystemShutdownExW(LPWSTR lpMachineName,
                          LPWSTR lpMessage,
                          DWORD dwTimeout,
                          BOOL bForceAppsClosed,
                          BOOL bRebootAfterShutdown,
                          DWORD dwReason)
{
     UNIMPLEMENTED;
     return TRUE;
}

BOOL WINAPI
InitiateSystemShutdownExA(LPSTR lpMachineName,
                          LPSTR lpMessage,
                          DWORD dwTimeout,
                          BOOL bForceAppsClosed,
                          BOOL bRebootAfterShutdown,
                          DWORD dwReason)
{
     UNIMPLEMENTED;
     return TRUE;
}

/* EOF */
