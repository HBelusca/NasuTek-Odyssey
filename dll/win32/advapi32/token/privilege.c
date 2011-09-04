/* $Id: privilege.c 37763 2008-11-30 11:42:05Z sginsberg $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/advapi32/token/privilege.c
 * PURPOSE:         advapi32.dll token's privilege handling
 * PROGRAMMER:      E.Aliberti
 * UPDATE HISTORY:
 *	20010317 ea	stubs
 */

#include <advapi32.h>


/**********************************************************************
 *	PrivilegeCheck					EXPORTED
 *
 * @implemented
 */
BOOL WINAPI
PrivilegeCheck(HANDLE ClientToken,
               PPRIVILEGE_SET RequiredPrivileges,
               LPBOOL pfResult)
{
    BOOLEAN Result;
    NTSTATUS Status;

    Status = NtPrivilegeCheck(ClientToken,
                              RequiredPrivileges,
                              &Result);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    *pfResult = (BOOL)Result;

    return TRUE;
}

/* EOF */
