/* $Id: audit.c 53225 2011-08-14 11:31:23Z akhaldi $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/advapi32/sec/audit.c
 * PURPOSE:         Audit functions
 * PROGRAMMER:      Eric Kohl
 * UPDATE HISTORY:
 *                  Created 07/19/2003
 */

/* INCLUDES *****************************************************************/

#include <advapi32.h>
WINE_DEFAULT_DEBUG_CHANNEL(advapi);

/* FUNCTIONS ****************************************************************/

/*
 * @implemented
 */
BOOL WINAPI
AccessCheckAndAuditAlarmA(LPCSTR SubsystemName,
                          LPVOID HandleId,
                          LPSTR ObjectTypeName,
                          LPSTR ObjectName,
                          PSECURITY_DESCRIPTOR SecurityDescriptor,
                          DWORD DesiredAccess,
                          PGENERIC_MAPPING GenericMapping,
                          BOOL ObjectCreation,
                          LPDWORD GrantedAccess,
                          LPBOOL AccessStatus,
                          LPBOOL pfGenerateOnClose)
{
    UNICODE_STRING SubsystemNameU;
    UNICODE_STRING ObjectTypeNameU;
    UNICODE_STRING ObjectNameU;
    NTSTATUS LocalAccessStatus;
    BOOLEAN GenerateOnClose;
    NTSTATUS Status;

    RtlCreateUnicodeStringFromAsciiz(&SubsystemNameU,
                                     (PCHAR)SubsystemName);
    RtlCreateUnicodeStringFromAsciiz(&ObjectTypeNameU,
                                     (PCHAR)ObjectTypeName);
    RtlCreateUnicodeStringFromAsciiz(&ObjectNameU,
                                     (PCHAR)ObjectName);

    Status = NtAccessCheckAndAuditAlarm(&SubsystemNameU,
                                        HandleId,
                                        &ObjectTypeNameU,
                                        &ObjectNameU,
                                        SecurityDescriptor,
                                        DesiredAccess,
                                        GenericMapping,
                                        ObjectCreation,
                                        GrantedAccess,
                                        &LocalAccessStatus,
                                        &GenerateOnClose);
    RtlFreeUnicodeString(&SubsystemNameU);
    RtlFreeUnicodeString(&ObjectTypeNameU);
    RtlFreeUnicodeString(&ObjectNameU);

    *pfGenerateOnClose = (BOOL)GenerateOnClose;

    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    if (!NT_SUCCESS (LocalAccessStatus))
    {
        *AccessStatus = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    *AccessStatus = TRUE;

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
AccessCheckAndAuditAlarmW(LPCWSTR SubsystemName,
                          LPVOID HandleId,
                          LPWSTR ObjectTypeName,
                          LPWSTR ObjectName,
                          PSECURITY_DESCRIPTOR SecurityDescriptor,
                          DWORD DesiredAccess,
                          PGENERIC_MAPPING GenericMapping,
                          BOOL ObjectCreation,
                          LPDWORD GrantedAccess,
                          LPBOOL AccessStatus,
                          LPBOOL pfGenerateOnClose)
{
    UNICODE_STRING SubsystemNameU;
    UNICODE_STRING ObjectTypeNameU;
    UNICODE_STRING ObjectNameU;
    NTSTATUS LocalAccessStatus;
    BOOLEAN GenerateOnClose;
    NTSTATUS Status;

    RtlInitUnicodeString(&SubsystemNameU,
                         (PWSTR)SubsystemName);
    RtlInitUnicodeString(&ObjectTypeNameU,
                         (PWSTR)ObjectTypeName);
    RtlInitUnicodeString(&ObjectNameU,
                         (PWSTR)ObjectName);

    Status = NtAccessCheckAndAuditAlarm(&SubsystemNameU,
                                        HandleId,
                                        &ObjectTypeNameU,
                                        &ObjectNameU,
                                        SecurityDescriptor,
                                        DesiredAccess,
                                        GenericMapping,
                                        ObjectCreation,
                                        GrantedAccess,
                                        &LocalAccessStatus,
                                        &GenerateOnClose);

    *pfGenerateOnClose = (BOOL)GenerateOnClose;

    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    if (!NT_SUCCESS(LocalAccessStatus))
    {
        *AccessStatus = FALSE;
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    *AccessStatus = TRUE;

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectCloseAuditAlarmA(LPCSTR SubsystemName,
                       LPVOID HandleId,
                       BOOL GenerateOnClose)
{
    UNICODE_STRING Name;
    NTSTATUS Status;

    Status = RtlCreateUnicodeStringFromAsciiz(&Name,
                                              (PCHAR)SubsystemName);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    Status = NtCloseObjectAuditAlarm(&Name,
                                     HandleId,
                                     GenerateOnClose);
    RtlFreeUnicodeString(&Name);
    if (!NT_SUCCESS (Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectCloseAuditAlarmW(LPCWSTR SubsystemName,
                       LPVOID HandleId,
                       BOOL GenerateOnClose)
{
    UNICODE_STRING Name;
    NTSTATUS Status;

    RtlInitUnicodeString(&Name,
                         (PWSTR)SubsystemName);

    Status = NtCloseObjectAuditAlarm(&Name,
                                     HandleId,
                                     GenerateOnClose);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectDeleteAuditAlarmA(LPCSTR SubsystemName,
                        LPVOID HandleId,
                        BOOL GenerateOnClose)
{
    UNICODE_STRING Name;
    NTSTATUS Status;

    Status = RtlCreateUnicodeStringFromAsciiz(&Name,
                                              (PCHAR)SubsystemName);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    Status = NtDeleteObjectAuditAlarm(&Name,
                                      HandleId,
                                      GenerateOnClose);
    RtlFreeUnicodeString(&Name);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectDeleteAuditAlarmW(LPCWSTR SubsystemName,
                        LPVOID HandleId,
                        BOOL GenerateOnClose)
{
    UNICODE_STRING Name;
    NTSTATUS Status;

    RtlInitUnicodeString(&Name,
                         (PWSTR)SubsystemName);

    Status = NtDeleteObjectAuditAlarm(&Name,
                                      HandleId,
                                      GenerateOnClose);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectOpenAuditAlarmA(LPCSTR SubsystemName,
                      LPVOID HandleId,
                      LPSTR ObjectTypeName,
                      LPSTR ObjectName,
                      PSECURITY_DESCRIPTOR pSecurityDescriptor,
                      HANDLE ClientToken,
                      DWORD DesiredAccess,
                      DWORD GrantedAccess,
                      PPRIVILEGE_SET Privileges,
                      BOOL ObjectCreation,
                      BOOL AccessGranted,
                      LPBOOL GenerateOnClose)
{
    UNICODE_STRING SubsystemNameU;
    UNICODE_STRING ObjectTypeNameU;
    UNICODE_STRING ObjectNameU;
    NTSTATUS Status;

    RtlCreateUnicodeStringFromAsciiz(&SubsystemNameU,
                                     (PCHAR)SubsystemName);
    RtlCreateUnicodeStringFromAsciiz(&ObjectTypeNameU,
                                     (PCHAR)ObjectTypeName);
    RtlCreateUnicodeStringFromAsciiz(&ObjectNameU,
                                     (PCHAR)ObjectName);

    Status = NtOpenObjectAuditAlarm(&SubsystemNameU,
                                    HandleId,
                                    &ObjectTypeNameU,
                                    &ObjectNameU,
                                    pSecurityDescriptor,
                                    ClientToken,
                                    DesiredAccess,
                                    GrantedAccess,
                                    Privileges,
                                    ObjectCreation,
                                    AccessGranted,
                                    (PBOOLEAN)GenerateOnClose);
    RtlFreeUnicodeString(&SubsystemNameU);
    RtlFreeUnicodeString(&ObjectTypeNameU);
    RtlFreeUnicodeString(&ObjectNameU);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectOpenAuditAlarmW(LPCWSTR SubsystemName,
                      LPVOID HandleId,
                      LPWSTR ObjectTypeName,
                      LPWSTR ObjectName,
                      PSECURITY_DESCRIPTOR pSecurityDescriptor,
                      HANDLE ClientToken,
                      DWORD DesiredAccess,
                      DWORD GrantedAccess,
                      PPRIVILEGE_SET Privileges,
                      BOOL ObjectCreation,
                      BOOL AccessGranted,
                      LPBOOL GenerateOnClose)
{
    UNICODE_STRING SubsystemNameU;
    UNICODE_STRING ObjectTypeNameU;
    UNICODE_STRING ObjectNameU;
    NTSTATUS Status;

    RtlInitUnicodeString(&SubsystemNameU,
                         (PWSTR)SubsystemName);
    RtlInitUnicodeString(&ObjectTypeNameU,
                         (PWSTR)ObjectTypeName);
    RtlInitUnicodeString(&ObjectNameU,
                         (PWSTR)ObjectName);

    Status = NtOpenObjectAuditAlarm(&SubsystemNameU,
                                    HandleId,
                                    &ObjectTypeNameU,
                                    &ObjectNameU,
                                    pSecurityDescriptor,
                                    ClientToken,
                                    DesiredAccess,
                                    GrantedAccess,
                                    Privileges,
                                    ObjectCreation,
                                    AccessGranted,
                                    (PBOOLEAN)GenerateOnClose);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectPrivilegeAuditAlarmA(LPCSTR SubsystemName,
                           LPVOID HandleId,
                           HANDLE ClientToken,
                           DWORD DesiredAccess,
                           PPRIVILEGE_SET Privileges,
                           BOOL AccessGranted)
{
    UNICODE_STRING SubsystemNameU;
    NTSTATUS Status;

    RtlCreateUnicodeStringFromAsciiz(&SubsystemNameU,
                                     (PCHAR)SubsystemName);

    Status = NtPrivilegeObjectAuditAlarm(&SubsystemNameU,
                                         HandleId,
                                         ClientToken,
                                         DesiredAccess,
                                         Privileges,
                                         AccessGranted);
    RtlFreeUnicodeString (&SubsystemNameU);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
ObjectPrivilegeAuditAlarmW(LPCWSTR SubsystemName,
                           LPVOID HandleId,
                           HANDLE ClientToken,
                           DWORD DesiredAccess,
                           PPRIVILEGE_SET Privileges,
                           BOOL AccessGranted)
{
    UNICODE_STRING SubsystemNameU;
    NTSTATUS Status;

    RtlInitUnicodeString(&SubsystemNameU,
                         (PWSTR)SubsystemName);

    Status = NtPrivilegeObjectAuditAlarm(&SubsystemNameU,
                                         HandleId,
                                         ClientToken,
                                         DesiredAccess,
                                         Privileges,
                                         AccessGranted);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
PrivilegedServiceAuditAlarmA(LPCSTR SubsystemName,
                             LPCSTR ServiceName,
                             HANDLE ClientToken,
                             PPRIVILEGE_SET Privileges,
                             BOOL AccessGranted)
{
    UNICODE_STRING SubsystemNameU;
    UNICODE_STRING ServiceNameU;
    NTSTATUS Status;

    RtlCreateUnicodeStringFromAsciiz(&SubsystemNameU,
                                     (PCHAR)SubsystemName);
    RtlCreateUnicodeStringFromAsciiz(&ServiceNameU,
                                     (PCHAR)ServiceName);

    Status = NtPrivilegedServiceAuditAlarm(&SubsystemNameU,
                                           &ServiceNameU,
                                           ClientToken,
                                           Privileges,
                                           AccessGranted);
    RtlFreeUnicodeString(&SubsystemNameU);
    RtlFreeUnicodeString(&ServiceNameU);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
PrivilegedServiceAuditAlarmW(LPCWSTR SubsystemName,
                             LPCWSTR ServiceName,
                             HANDLE ClientToken,
                             PPRIVILEGE_SET Privileges,
                             BOOL AccessGranted)
{
    UNICODE_STRING SubsystemNameU;
    UNICODE_STRING ServiceNameU;
    NTSTATUS Status;

    RtlInitUnicodeString(&SubsystemNameU,
                         (PWSTR)SubsystemName);
    RtlInitUnicodeString(&ServiceNameU,
                         (PWSTR)ServiceName);

    Status = NtPrivilegedServiceAuditAlarm(&SubsystemNameU,
                                           &ServiceNameU,
                                           ClientToken,
                                           Privileges,
                                           AccessGranted);
    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}


/*
 * @unimplemented
 */
BOOL WINAPI
AccessCheckByTypeResultListAndAuditAlarmByHandleW(IN LPCWSTR SubsystemName,
                                                  IN LPVOID HandleId,
                                                  IN HANDLE ClientToken,
                                                  IN LPCWSTR ObjectTypeName,
                                                  IN LPCWSTR ObjectName,
                                                  IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                                  IN PSID PrincipalSelfSid,
                                                  IN DWORD DesiredAccess,
                                                  IN AUDIT_EVENT_TYPE AuditType,
                                                  IN DWORD Flags,
                                                  IN POBJECT_TYPE_LIST ObjectTypeList,
                                                  IN DWORD ObjectTypeListLength,
                                                  IN PGENERIC_MAPPING GenericMapping,
                                                  IN BOOL ObjectCreation,
                                                  OUT LPDWORD GrantedAccess,
                                                  OUT LPDWORD AccessStatusList,
                                                  OUT LPBOOL pfGenerateOnClose)
{
    FIXME("%s() not implemented!\n", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL WINAPI
AccessCheckByTypeResultListAndAuditAlarmByHandleA(IN LPCSTR SubsystemName,
                                                  IN LPVOID HandleId,
                                                  IN HANDLE ClientToken,
                                                  IN LPCSTR ObjectTypeName,
                                                  IN LPCSTR ObjectName,
                                                  IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                                  IN PSID PrincipalSelfSid,
                                                  IN DWORD DesiredAccess,
                                                  IN AUDIT_EVENT_TYPE AuditType,
                                                  IN DWORD Flags,
                                                  IN POBJECT_TYPE_LIST ObjectTypeList,
                                                  IN DWORD ObjectTypeListLength,
                                                  IN PGENERIC_MAPPING GenericMapping,
                                                  IN BOOL ObjectCreation,
                                                  OUT LPDWORD GrantedAccess,
                                                  OUT LPDWORD AccessStatusList,
                                                  OUT LPBOOL pfGenerateOnClose)
{
    FIXME("%s() not implemented!\n", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL WINAPI
AccessCheckByTypeResultListAndAuditAlarmW(IN LPCWSTR SubsystemName,
                                          IN LPVOID HandleId,
                                          IN LPCWSTR ObjectTypeName,
                                          IN LPCWSTR ObjectName,
                                          IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                          IN PSID PrincipalSelfSid,
                                          IN DWORD DesiredAccess,
                                          IN AUDIT_EVENT_TYPE AuditType,
                                          IN DWORD Flags,
                                          IN POBJECT_TYPE_LIST ObjectTypeList,
                                          IN DWORD ObjectTypeListLength,
                                          IN PGENERIC_MAPPING GenericMapping,
                                          IN BOOL ObjectCreation,
                                          OUT LPDWORD GrantedAccess,
                                          OUT LPDWORD AccessStatusList,
                                          OUT LPBOOL pfGenerateOnClose)
{
    FIXME("%s() not implemented!\n", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL WINAPI
AccessCheckByTypeResultListAndAuditAlarmA(IN LPCSTR SubsystemName,
                                          IN LPVOID HandleId,
                                          IN LPCSTR ObjectTypeName,
                                          IN LPCSTR ObjectName,
                                          IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                          IN PSID PrincipalSelfSid,
                                          IN DWORD DesiredAccess,
                                          IN AUDIT_EVENT_TYPE AuditType,
                                          IN DWORD Flags,
                                          IN POBJECT_TYPE_LIST ObjectTypeList,
                                          IN DWORD ObjectTypeListLength,
                                          IN PGENERIC_MAPPING GenericMapping,
                                          IN BOOL ObjectCreation,
                                          OUT LPDWORD GrantedAccess,
                                          OUT LPDWORD AccessStatusList,
                                          OUT LPBOOL pfGenerateOnClose)
{
    FIXME("%s() not implemented!\n", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL WINAPI
AccessCheckByTypeAndAuditAlarmW(IN LPCWSTR SubsystemName,
                                IN LPVOID HandleId,
                                IN LPCWSTR ObjectTypeName,
                                IN LPCWSTR ObjectName,
                                IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                IN PSID PrincipalSelfSid,
                                IN DWORD DesiredAccess,
                                IN AUDIT_EVENT_TYPE AuditType,
                                IN DWORD Flags,
                                IN POBJECT_TYPE_LIST ObjectTypeList,
                                IN DWORD ObjectTypeListLength,
                                IN PGENERIC_MAPPING GenericMapping,
                                IN BOOL ObjectCreation,
                                OUT LPDWORD GrantedAccess,
                                OUT LPBOOL AccessStatus,
                                OUT LPBOOL pfGenerateOnClose)
{
    FIXME("%s() not implemented!\n", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL WINAPI
AccessCheckByTypeAndAuditAlarmA(IN LPCSTR SubsystemName,
                                IN LPVOID HandleId,
                                IN LPCSTR ObjectTypeName,
                                IN LPCSTR ObjectName,
                                IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                IN PSID PrincipalSelfSid,
                                IN DWORD DesiredAccess,
                                IN AUDIT_EVENT_TYPE AuditType,
                                IN DWORD Flags,
                                IN POBJECT_TYPE_LIST ObjectTypeList,
                                IN DWORD ObjectTypeListLength,
                                IN PGENERIC_MAPPING GenericMapping,
                                IN BOOL ObjectCreation,
                                OUT LPDWORD GrantedAccess,
                                OUT LPBOOL AccessStatus,
                                OUT LPBOOL pfGenerateOnClose)
{
    FIXME("%s() not implemented!\n", __FUNCTION__);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/* EOF */
