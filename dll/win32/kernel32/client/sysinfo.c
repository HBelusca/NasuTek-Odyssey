/*
 * PROJECT:         Odyssey Win32 Base API
 * LICENSE:         See COPYING in the top level directory
 * FILE:            dll/win32/kernel32/client/sysinfo.c
 * PURPOSE:         System Information Functions
 * PROGRAMMERS:     Emanuele Aliberti
 *                  Christoph von Wittich
 *                  Thomas Weidenmueller
 *                  Gunnar Andre Dalsnes
 */

/* INCLUDES *******************************************************************/

#include <k32.h>

#define NDEBUG
#include <debug.h>

#define PV_NT351 0x00030033

/* PRIVATE FUNCTIONS **********************************************************/

VOID
WINAPI
GetSystemInfoInternal(IN PSYSTEM_BASIC_INFORMATION BasicInfo,
                      IN PSYSTEM_PROCESSOR_INFORMATION ProcInfo,
                      OUT LPSYSTEM_INFO SystemInfo)
{
    RtlZeroMemory(SystemInfo, sizeof (SYSTEM_INFO));
    SystemInfo->wProcessorArchitecture = ProcInfo->ProcessorArchitecture;
    SystemInfo->wReserved = 0;
    SystemInfo->dwPageSize = BasicInfo->PageSize;
    SystemInfo->lpMinimumApplicationAddress = (PVOID)BasicInfo->MinimumUserModeAddress;
    SystemInfo->lpMaximumApplicationAddress = (PVOID)BasicInfo->MaximumUserModeAddress;
    SystemInfo->dwActiveProcessorMask = BasicInfo->ActiveProcessorsAffinityMask;
    SystemInfo->dwNumberOfProcessors = BasicInfo->NumberOfProcessors;
    SystemInfo->wProcessorLevel = ProcInfo->ProcessorLevel;
    SystemInfo->wProcessorRevision = ProcInfo->ProcessorRevision;
    SystemInfo->dwAllocationGranularity = BasicInfo->AllocationGranularity;

    switch (ProcInfo->ProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_INTEL:
            switch (ProcInfo->ProcessorLevel)
            {
                case 3:
                    SystemInfo->dwProcessorType = PROCESSOR_INTEL_386;
                    break;
                case 4:
                    SystemInfo->dwProcessorType = PROCESSOR_INTEL_486;
                    break;
                default:
                    SystemInfo->dwProcessorType = PROCESSOR_INTEL_PENTIUM;
            }
            break;

        case PROCESSOR_ARCHITECTURE_AMD64:
            SystemInfo->dwProcessorType = PROCESSOR_AMD_X8664;
            break;

        case PROCESSOR_ARCHITECTURE_IA64:
            SystemInfo->dwProcessorType = PROCESSOR_INTEL_IA64;
            break;

        default:
            SystemInfo->dwProcessorType = 0;
            break;
    }

    if (PV_NT351 > GetProcessVersion(0))
    {
        SystemInfo->wProcessorLevel = 0;
        SystemInfo->wProcessorRevision = 0;
    }
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * @implemented
 */
SIZE_T
WINAPI
GetLargePageMinimum(VOID)
{
    return SharedUserData->LargePageMinimum;
}

/*
 * @implemented
 */
VOID
WINAPI
GetSystemInfo(IN LPSYSTEM_INFO lpSystemInfo)
{
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcInfo;
    NTSTATUS Status;

    Status = NtQuerySystemInformation(SystemBasicInformation,
                                      &BasicInfo,
                                      sizeof(BasicInfo),
                                      0);
    if (!NT_SUCCESS(Status)) return;
                                  
    Status = NtQuerySystemInformation(SystemProcessorInformation,
                                      &ProcInfo,
                                      sizeof(ProcInfo),
                                      0);
    if (!NT_SUCCESS(Status)) return;
    
    GetSystemInfoInternal(&BasicInfo, &ProcInfo, lpSystemInfo);
}

/*
 * @implemented
 */
BOOL
WINAPI
IsProcessorFeaturePresent(IN DWORD ProcessorFeature)
{
    if (ProcessorFeature >= PROCESSOR_FEATURE_MAX) return FALSE;
    return ((BOOL)SharedUserData->ProcessorFeatures[ProcessorFeature]);
}

/*
 * @implemented
 */
BOOL
WINAPI
GetSystemRegistryQuota(OUT PDWORD pdwQuotaAllowed,
                       OUT PDWORD pdwQuotaUsed)
{
    SYSTEM_REGISTRY_QUOTA_INFORMATION QuotaInfo;
    ULONG BytesWritten;
    NTSTATUS Status;

    Status = NtQuerySystemInformation(SystemRegistryQuotaInformation,
                                      &QuotaInfo,
                                      sizeof(QuotaInfo),
                                      &BytesWritten);
    if (NT_SUCCESS(Status))
    {
      if (pdwQuotaAllowed) *pdwQuotaAllowed = QuotaInfo.RegistryQuotaAllowed;
      if (pdwQuotaUsed) *pdwQuotaUsed = QuotaInfo.RegistryQuotaUsed;
      return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}

/*
 * @implemented
 */
VOID
WINAPI
GetNativeSystemInfo(IN LPSYSTEM_INFO lpSystemInfo)
{
    SYSTEM_BASIC_INFORMATION BasicInfo;
    SYSTEM_PROCESSOR_INFORMATION ProcInfo;
    NTSTATUS Status;

    /* FIXME: Should be SystemNativeBasicInformation */
    Status = NtQuerySystemInformation(SystemBasicInformation,
                                      &BasicInfo,
                                      sizeof(BasicInfo),
                                      0);
    if (!NT_SUCCESS(Status)) return;
                                  
    /* FIXME: Should be SystemNativeProcessorInformation */
    Status = NtQuerySystemInformation(SystemProcessorInformation,
                                      &ProcInfo,
                                      sizeof(ProcInfo),
                                      0);
    if (!NT_SUCCESS(Status)) return;
    
    GetSystemInfoInternal(&BasicInfo, &ProcInfo, lpSystemInfo);
}

/*
 * @implemented
 */
BOOL
WINAPI
GetLogicalProcessorInformation(OUT PSYSTEM_LOGICAL_PROCESSOR_INFORMATION Buffer,
                               IN OUT PDWORD ReturnLength)
{
    NTSTATUS Status;

    if (!ReturnLength)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Status = NtQuerySystemInformation(SystemLogicalProcessorInformation,
                                      Buffer,
                                      *ReturnLength,
                                      ReturnLength);

    /* Normalize the error to what Win32 expects */
    if (Status == STATUS_INFO_LENGTH_MISMATCH) Status = STATUS_BUFFER_TOO_SMALL;
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
GetNumaHighestNodeNumber(OUT PULONG HighestNodeNumber)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
GetNumaNodeProcessorMask(IN UCHAR Node,
                         OUT PULONGLONG ProcessorMask)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
GetNumaProcessorNode(IN UCHAR Processor,
                     OUT PUCHAR NodeNumber)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
GetNumaAvailableMemoryNode(IN UCHAR Node,
                           OUT PULONGLONG AvailableBytes)
{
    STUB;
    return FALSE;
}

/*
 * @unimplemented
 */
DWORD
WINAPI
GetFirmwareEnvironmentVariableW(IN LPCWSTR lpName,
                                IN LPCWSTR lpGuid,
                                IN PVOID pValue,
                                IN DWORD nSize)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
SetFirmwareEnvironmentVariableW(IN LPCWSTR lpName,
                                IN LPCWSTR lpGuid,
                                IN PVOID pValue,
                                IN DWORD nSize)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
DWORD
WINAPI
GetFirmwareEnvironmentVariableA(IN LPCSTR lpName,
                                IN LPCSTR lpGuid,
                                IN PVOID pValue,
                                IN DWORD nSize)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
SetFirmwareEnvironmentVariableA(IN LPCSTR lpName,
                                IN LPCSTR lpGuid,
                                IN PVOID pValue,
                                IN DWORD nSize)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
UINT
WINAPI
EnumSystemFirmwareTables(IN DWORD FirmwareTableProviderSignature,
                         OUT PVOID pFirmwareTableBuffer,
                         IN DWORD BufferSize)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
UINT
WINAPI
GetSystemFirmwareTable(IN DWORD FirmwareTableProviderSignature,
                       IN DWORD FirmwareTableID,
                       OUT PVOID pFirmwareTableBuffer,
                       IN DWORD BufferSize)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
GetSystemFileCacheSize(OUT PSIZE_T lpMinimumFileCacheSize,
                       OUT PSIZE_T lpMaximumFileCacheSize,
                       OUT PDWORD lpFlags)
{
    STUB;
    return FALSE;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
SetSystemFileCacheSize(IN SIZE_T MinimumFileCacheSize,
                       IN SIZE_T MaximumFileCacheSize,
                       IN DWORD Flags)
{
    STUB;
    return FALSE;
}
