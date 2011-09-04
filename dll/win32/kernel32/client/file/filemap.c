/*
 * PROJECT:         Odyssey Win32 Base API
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            dll/win32/kernel32/mem/section.c
 * PURPOSE:         Handles virtual memory APIs
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <k32.h>

#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
HANDLE
NTAPI
CreateFileMappingA(IN HANDLE hFile,
                   IN LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                   IN DWORD flProtect,
                   IN DWORD dwMaximumSizeHigh,
                   IN DWORD dwMaximumSizeLow,
                   IN LPCSTR lpName)
{
    /* Call the W(ide) function */
    ConvertWin32AnsiObjectApiToUnicodeApi(FileMapping,
                                          lpName,
                                          hFile,
                                          lpFileMappingAttributes,
                                          flProtect,
                                          dwMaximumSizeHigh,
                                          dwMaximumSizeLow);
}

/*
 * @implemented
 */
HANDLE
NTAPI
CreateFileMappingW(HANDLE hFile,
                   LPSECURITY_ATTRIBUTES lpFileMappingAttributes,
                   DWORD flProtect,
                   DWORD dwMaximumSizeHigh,
                   DWORD dwMaximumSizeLow,
                   LPCWSTR lpName)
{
    NTSTATUS Status;
    HANDLE SectionHandle;
    OBJECT_ATTRIBUTES LocalAttributes;
    POBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING SectionName;
    ACCESS_MASK DesiredAccess;
    LARGE_INTEGER LocalSize;
    PLARGE_INTEGER SectionSize = NULL;
    ULONG Attributes;

    /* Set default access */
    DesiredAccess = STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ;

    /* Get the attributes for the actual allocation and cleanup flProtect */
    Attributes = flProtect & (SEC_FILE | SEC_IMAGE | SEC_RESERVE | SEC_NOCACHE | SEC_COMMIT | SEC_LARGE_PAGES);
    flProtect ^= Attributes;

    /* If the caller didn't say anything, assume SEC_COMMIT */
    if (!Attributes) Attributes = SEC_COMMIT;

    /* Now check if the caller wanted write access */
    if (flProtect == PAGE_READWRITE)
    {
        /* Give it */
        DesiredAccess |= (SECTION_MAP_WRITE | SECTION_MAP_READ);
    }

    /* Now check if we got a name */
    if (lpName) RtlInitUnicodeString(&SectionName, lpName);

    /* Now convert the object attributes */
    ObjectAttributes = BasepConvertObjectAttributes(&LocalAttributes,
                                                    lpFileMappingAttributes,
                                                    lpName ? &SectionName : NULL);

    /* Check if we got a size */
    if (dwMaximumSizeLow || dwMaximumSizeHigh)
    {
        /* Use a LARGE_INTEGER and convert */
        SectionSize = &LocalSize;
        SectionSize->LowPart = dwMaximumSizeLow;
        SectionSize->HighPart = dwMaximumSizeHigh;
    }

    /* Make sure the handle is valid */
    if (hFile == INVALID_HANDLE_VALUE)
    {
        /* It's not, we'll only go on if we have a size */
        hFile = NULL;
        if (!SectionSize)
        {
            /* No size, so this isn't a valid non-mapped section */
            SetLastError(ERROR_INVALID_PARAMETER);
            return NULL;
        }
    }

    /* Now create the actual section */
    Status = NtCreateSection(&SectionHandle,
                             DesiredAccess,
                             ObjectAttributes,
                             SectionSize,
                             flProtect,
                             Attributes,
                             hFile);
    
    if (Status == STATUS_OBJECT_NAME_EXISTS)
    {
        SetLastError(ERROR_ALREADY_EXISTS);
        return SectionHandle;
    }

    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        BaseSetLastNTError(Status);
        return NULL;
    }

    SetLastError(ERROR_SUCCESS);
    /* Return the section */
    return SectionHandle;
}

/*
 * @implemented
 */
LPVOID
NTAPI
MapViewOfFileEx(HANDLE hFileMappingObject,
                DWORD dwDesiredAccess,
                DWORD dwFileOffsetHigh,
                DWORD dwFileOffsetLow,
                SIZE_T dwNumberOfBytesToMap,
                LPVOID lpBaseAddress)
{
    NTSTATUS Status;
    LARGE_INTEGER SectionOffset;
    SIZE_T ViewSize;
    ULONG Protect;
    LPVOID ViewBase;

    /* Convert the offset */
    SectionOffset.LowPart = dwFileOffsetLow;
    SectionOffset.HighPart = dwFileOffsetHigh;

    /* Save the size and base */
    ViewBase = lpBaseAddress;
    ViewSize = dwNumberOfBytesToMap;

    /* Convert flags to NT Protection Attributes */
    if (dwDesiredAccess & FILE_MAP_WRITE)
    {
        Protect  = PAGE_READWRITE;
    }
    else if (dwDesiredAccess & FILE_MAP_READ)
    {
        Protect = PAGE_READONLY;
    }
    else if (dwDesiredAccess & FILE_MAP_COPY)
    {
        Protect = PAGE_WRITECOPY;
    }
    else
    {
        Protect = PAGE_NOACCESS;
    }

    /* Map the section */
    Status = ZwMapViewOfSection(hFileMappingObject,
                                NtCurrentProcess(),
                                &ViewBase,
                                0,
                                0,
                                &SectionOffset,
                                &ViewSize,
                                ViewShare,
                                0,
                                Protect);
    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        BaseSetLastNTError(Status);
        return NULL;
    }

    /* Return the base */
    return ViewBase;
}

/*
 * @implemented
 */
LPVOID
NTAPI
MapViewOfFile(HANDLE hFileMappingObject,
              DWORD dwDesiredAccess,
              DWORD dwFileOffsetHigh,
              DWORD dwFileOffsetLow,
              SIZE_T dwNumberOfBytesToMap)
{
    /* Call the extended API */
    return MapViewOfFileEx(hFileMappingObject,
                           dwDesiredAccess,
                           dwFileOffsetHigh,
                           dwFileOffsetLow,
                           dwNumberOfBytesToMap,
                           NULL);
}

/*
 * @implemented
 */
BOOL
NTAPI
UnmapViewOfFile(LPCVOID lpBaseAddress)
{
    NTSTATUS Status;

    /* Unmap the section */
    Status = NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)lpBaseAddress);
    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        BaseSetLastNTError(Status);
        return FALSE;
    }

    /* Otherwise, return sucess */
    return TRUE;
}

/*
 * @implemented
 */
HANDLE
NTAPI
OpenFileMappingA(DWORD dwDesiredAccess,
                 BOOL bInheritHandle,
                 LPCSTR lpName)
{
    NTSTATUS Status;
    ANSI_STRING AnsiName;
    PUNICODE_STRING UnicodeCache;

    /* Check for a name */
    if (lpName)
    {
        /* Use TEB Cache */
        UnicodeCache = &NtCurrentTeb()->StaticUnicodeString;

        /* Convert to unicode */
        RtlInitAnsiString(&AnsiName, lpName);
        Status = RtlAnsiStringToUnicodeString(UnicodeCache, &AnsiName, FALSE);
        if (!NT_SUCCESS(Status))
        {
            /* Conversion failed */
            BaseSetLastNTError(Status);
            return NULL;
        }
    }
    else
    {
        /* We need a name */
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    /* Call the Unicode version */
    return OpenFileMappingW(dwDesiredAccess,
                            bInheritHandle,
                            (LPCWSTR)UnicodeCache->Buffer);
}

/*
 * @implemented
 */
HANDLE
NTAPI
OpenFileMappingW(DWORD dwDesiredAccess,
                 BOOL bInheritHandle,
                 LPCWSTR lpName)
{
    NTSTATUS Status;
    HANDLE SectionHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeName;

    /* We need a name */
    if (!lpName)
    {
        /* Otherwise, fail */
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    /* Convert attributes */
    RtlInitUnicodeString(&UnicodeName, lpName);
    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeName,
                               (bInheritHandle ? OBJ_INHERIT : 0),
                               hBaseDir,
                               NULL);

    /* Convert COPY to READ */
    if (dwDesiredAccess == FILE_MAP_COPY) dwDesiredAccess = FILE_MAP_READ;

    /* Open the section */
    Status = ZwOpenSection(&SectionHandle,
                           dwDesiredAccess,
                           &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        BaseSetLastNTError(Status);
        return NULL;
    }

    SetLastError(ERROR_SUCCESS);
    /* Otherwise, return the handle */
    return SectionHandle;
}

/*
 * @implemented
 */
BOOL
NTAPI
FlushViewOfFile(LPCVOID lpBaseAddress,
                SIZE_T dwNumberOfBytesToFlush)
{
    SIZE_T NumberOfBytesToFlush;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    /* Save amount of bytes to flush to a local var */
    NumberOfBytesToFlush = dwNumberOfBytesToFlush;

    /* Flush the view */
    Status = NtFlushVirtualMemory(NtCurrentProcess(),
                                  (LPVOID)lpBaseAddress,
                                  &NumberOfBytesToFlush,
                                  &IoStatusBlock);
    if (!NT_SUCCESS(Status))
    {
        /* We failed */
        BaseSetLastNTError(Status);
        return FALSE;
    }

    /* Return success */
    return TRUE;
}

/* EOF */
