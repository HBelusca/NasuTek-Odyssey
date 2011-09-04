/*
 * PROJECT:     Odyssey Service Control Manager
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        base/system/services/database.c
 * PURPOSE:     Database control interface
 * COPYRIGHT:   Copyright 2002-2006 Eric Kohl
 *              Copyright 2006 Herv� Poussineau <hpoussin@odyssey.org>
 *              Copyright 2007 Ged Murphy <gedmurphy@odyssey.org>
 *                             Gregor Brunmar <gregor.brunmar@home.se>
 *
 */

/* INCLUDES *****************************************************************/

#include "services.h"

#define NDEBUG
#include <debug.h>

/*
 * Uncomment the line below to start services
 *  using the SERVICE_START_PENDING state
 */
// #define USE_SERVICE_START_PENDING

/* GLOBALS *******************************************************************/

LIST_ENTRY ImageListHead;
LIST_ENTRY ServiceListHead;

static RTL_RESOURCE DatabaseLock;
static DWORD dwResumeCount = 1;

static CRITICAL_SECTION ControlServiceCriticalSection;

/* FUNCTIONS *****************************************************************/

static DWORD
ScmCreateNewControlPipe(PSERVICE_IMAGE pServiceImage)
{
    WCHAR szControlPipeName[MAX_PATH + 1];
    HKEY hServiceCurrentKey = INVALID_HANDLE_VALUE;
    DWORD ServiceCurrent = 0;
    DWORD KeyDisposition;
    DWORD dwKeySize;
    DWORD dwError;

    /* Get the service number */
    /* TODO: Create registry entry with correct write access */
    dwError = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                              L"SYSTEM\\CurrentControlSet\\Control\\ServiceCurrent", 0, NULL,
                              REG_OPTION_VOLATILE,
                              KEY_WRITE | KEY_READ,
                              NULL,
                              &hServiceCurrentKey,
                              &KeyDisposition);
    if (dwError != ERROR_SUCCESS)
    {
        DPRINT1("RegCreateKeyEx() failed with error %lu\n", dwError);
        return dwError;
    }

    if (KeyDisposition == REG_OPENED_EXISTING_KEY)
    {
        dwKeySize = sizeof(DWORD);
        dwError = RegQueryValueExW(hServiceCurrentKey,
                                   L"", 0, NULL, (BYTE*)&ServiceCurrent, &dwKeySize);

        if (dwError != ERROR_SUCCESS)
        {
            RegCloseKey(hServiceCurrentKey);
            DPRINT1("RegQueryValueEx() failed with error %lu\n", dwError);
            return dwError;
        }

        ServiceCurrent++;
    }

    dwError = RegSetValueExW(hServiceCurrentKey, L"", 0, REG_DWORD, (BYTE*)&ServiceCurrent, sizeof(ServiceCurrent));

    RegCloseKey(hServiceCurrentKey);

    if (dwError != ERROR_SUCCESS)
    {
        DPRINT1("RegSetValueExW() failed (Error %lu)\n", dwError);
        return dwError;
    }

    /* Create '\\.\pipe\net\NtControlPipeXXX' instance */
    swprintf(szControlPipeName, L"\\\\.\\pipe\\net\\NtControlPipe%u", ServiceCurrent);

    DPRINT("PipeName: %S\n", szControlPipeName);

    pServiceImage->hControlPipe = CreateNamedPipeW(szControlPipeName,
                                                   PIPE_ACCESS_DUPLEX,
                                                   PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                                                   100,
                                                   8000,
                                                   4,
                                                   30000,
                                                   NULL);
    DPRINT("CreateNamedPipeW(%S) done\n", szControlPipeName);
    if (pServiceImage->hControlPipe == INVALID_HANDLE_VALUE)
    {
        DPRINT1("Failed to create control pipe!\n");
        return GetLastError();
    }

    return ERROR_SUCCESS;
}


static PSERVICE_IMAGE
ScmGetServiceImageByImagePath(LPWSTR lpImagePath)
{
    PLIST_ENTRY ImageEntry;
    PSERVICE_IMAGE CurrentImage;

    DPRINT("ScmGetServiceImageByImagePath(%S) called\n", lpImagePath);

    ImageEntry = ImageListHead.Flink;
    while (ImageEntry != &ImageListHead)
    {
        CurrentImage = CONTAINING_RECORD(ImageEntry,
                                         SERVICE_IMAGE,
                                         ImageListEntry);
        if (_wcsicmp(CurrentImage->szImagePath, lpImagePath) == 0)
        {
            DPRINT("Found image: '%S'\n", CurrentImage->szImagePath);
            return CurrentImage;
        }

        ImageEntry = ImageEntry->Flink;
    }

    DPRINT1("Couldn't find a matching image\n");

    return NULL;

}


static DWORD
ScmCreateOrReferenceServiceImage(PSERVICE pService)
{
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    UNICODE_STRING ImagePath;
    PSERVICE_IMAGE pServiceImage = NULL;
    NTSTATUS Status;
    DWORD dwError = ERROR_SUCCESS;

    DPRINT("ScmCreateOrReferenceServiceImage(%p)\n", pService);

    RtlInitUnicodeString(&ImagePath, NULL);

    /* Get service data */
    RtlZeroMemory(&QueryTable,
                  sizeof(QueryTable));

    QueryTable[0].Name = L"ImagePath";
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].EntryContext = &ImagePath;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                    pService->lpServiceName,
                                    QueryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("RtlQueryRegistryValues() failed (Status %lx)\n", Status);
        return RtlNtStatusToDosError(Status);
    }

    DPRINT("ImagePath: '%wZ'\n", &ImagePath);

    pServiceImage = ScmGetServiceImageByImagePath(ImagePath.Buffer);
    if (pServiceImage == NULL)
    {
        /* Create a new service image */
        pServiceImage = (PSERVICE_IMAGE)HeapAlloc(GetProcessHeap(),
                                                  HEAP_ZERO_MEMORY,
                                                  sizeof(SERVICE_IMAGE) + ((wcslen(ImagePath.Buffer) + 1) * sizeof(WCHAR)));
        if (pServiceImage == NULL)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            goto done;
        }

        pServiceImage->dwImageRunCount = 1;
        pServiceImage->hControlPipe = INVALID_HANDLE_VALUE;
        pServiceImage->hProcess = INVALID_HANDLE_VALUE;

        /* Set the image path */
        wcscpy(pServiceImage->szImagePath,
               ImagePath.Buffer);

        RtlFreeUnicodeString(&ImagePath);

        /* Create the control pipe */
        dwError = ScmCreateNewControlPipe(pServiceImage);
        if (dwError != ERROR_SUCCESS)
        {
            HeapFree(GetProcessHeap(), 0, pServiceImage);
            goto done;
        }

        /* FIXME: Add more initialization code here */


        /* Append service record */
        InsertTailList(&ImageListHead,
                       &pServiceImage->ImageListEntry);
    }
    else
    {
        /* Increment the run counter */
        pServiceImage->dwImageRunCount++;
    }

    DPRINT("pServiceImage->dwImageRunCount: %lu\n", pServiceImage->dwImageRunCount);

    /* Link the service image to the service */
    pService->lpImage = pServiceImage;

done:;
    RtlFreeUnicodeString(&ImagePath);

    DPRINT("ScmCreateOrReferenceServiceImage() done (Error: %lu)\n", dwError);

    return dwError;
}


static VOID
ScmDereferenceServiceImage(PSERVICE_IMAGE pServiceImage)
{
    DPRINT1("ScmDereferenceServiceImage() called\n");

    pServiceImage->dwImageRunCount--;

    if (pServiceImage->dwImageRunCount == 0)
    {
        DPRINT1("dwImageRunCount == 0\n");

        /* FIXME: Terminate the process */

        /* Remove the service image from the list */
        RemoveEntryList(&pServiceImage->ImageListEntry);

        /* Close the control pipe */
        if (pServiceImage->hControlPipe != INVALID_HANDLE_VALUE)
            CloseHandle(pServiceImage->hControlPipe);

        /* Close the process handle */
        if (pServiceImage->hProcess != INVALID_HANDLE_VALUE)
            CloseHandle(pServiceImage->hProcess);

        /* Release the service image */
        HeapFree(GetProcessHeap(), 0, pServiceImage);
    }
}


PSERVICE
ScmGetServiceEntryByName(LPCWSTR lpServiceName)
{
    PLIST_ENTRY ServiceEntry;
    PSERVICE CurrentService;

    DPRINT("ScmGetServiceEntryByName() called\n");

    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry,
                                           SERVICE,
                                           ServiceListEntry);
        if (_wcsicmp(CurrentService->lpServiceName, lpServiceName) == 0)
        {
            DPRINT("Found service: '%S'\n", CurrentService->lpServiceName);
            return CurrentService;
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    DPRINT("Couldn't find a matching service\n");

    return NULL;
}


PSERVICE
ScmGetServiceEntryByDisplayName(LPCWSTR lpDisplayName)
{
    PLIST_ENTRY ServiceEntry;
    PSERVICE CurrentService;

    DPRINT("ScmGetServiceEntryByDisplayName() called\n");

    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry,
                                           SERVICE,
                                           ServiceListEntry);
        if (_wcsicmp(CurrentService->lpDisplayName, lpDisplayName) == 0)
        {
            DPRINT("Found service: '%S'\n", CurrentService->lpDisplayName);
            return CurrentService;
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    DPRINT("Couldn't find a matching service\n");

    return NULL;
}


PSERVICE
ScmGetServiceEntryByResumeCount(DWORD dwResumeCount)
{
    PLIST_ENTRY ServiceEntry;
    PSERVICE CurrentService;

    DPRINT("ScmGetServiceEntryByResumeCount() called\n");

    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry,
                                           SERVICE,
                                           ServiceListEntry);
        if (CurrentService->dwResumeCount > dwResumeCount)
        {
            DPRINT("Found service: '%S'\n", CurrentService->lpDisplayName);
            return CurrentService;
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    DPRINT("Couldn't find a matching service\n");

    return NULL;
}


DWORD
ScmCreateNewServiceRecord(LPCWSTR lpServiceName,
                          PSERVICE *lpServiceRecord)
{
    PSERVICE lpService = NULL;

    DPRINT("Service: '%S'\n", lpServiceName);

    /* Allocate service entry */
    lpService = (SERVICE*)HeapAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY,
                          sizeof(SERVICE) + ((wcslen(lpServiceName) + 1) * sizeof(WCHAR)));
    if (lpService == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    *lpServiceRecord = lpService;

    /* Copy service name */
    wcscpy(lpService->szServiceName, lpServiceName);
    lpService->lpServiceName = lpService->szServiceName;
    lpService->lpDisplayName = lpService->lpServiceName;

    /* Set the resume count */
    lpService->dwResumeCount = dwResumeCount++;

    /* Append service record */
    InsertTailList(&ServiceListHead,
                   &lpService->ServiceListEntry);

    /* Initialize the service status */
    lpService->Status.dwCurrentState = SERVICE_STOPPED;
    lpService->Status.dwControlsAccepted = 0;
    lpService->Status.dwWin32ExitCode = ERROR_SERVICE_NEVER_STARTED;
    lpService->Status.dwServiceSpecificExitCode = 0;
    lpService->Status.dwCheckPoint = 0;
    lpService->Status.dwWaitHint = 2000; /* 2 seconds */

    return ERROR_SUCCESS;
}


VOID
ScmDeleteServiceRecord(PSERVICE lpService)
{
    DPRINT("Deleting Service %S\n", lpService->lpServiceName);

    /* Delete the display name */
    if (lpService->lpDisplayName != NULL &&
        lpService->lpDisplayName != lpService->lpServiceName)
        HeapFree(GetProcessHeap(), 0, lpService->lpDisplayName);

    /* Dereference the service image */
    if (lpService->lpImage)
        ScmDereferenceServiceImage(lpService->lpImage);

    /* Decrement the group reference counter */
    if (lpService->lpGroup)
        lpService->lpGroup->dwRefCount--;

    /* FIXME: SecurityDescriptor */


    /* Remove the Service from the List */
    RemoveEntryList(&lpService->ServiceListEntry);

    DPRINT("Deleted Service %S\n", lpService->lpServiceName);

    /* Delete the service record */
    HeapFree(GetProcessHeap(), 0, lpService);

    DPRINT("Done\n");
}


static DWORD
CreateServiceListEntry(LPCWSTR lpServiceName,
                       HKEY hServiceKey)
{
    PSERVICE lpService = NULL;
    LPWSTR lpDisplayName = NULL;
    LPWSTR lpGroup = NULL;
    DWORD dwSize;
    DWORD dwError;
    DWORD dwServiceType;
    DWORD dwStartType;
    DWORD dwErrorControl;
    DWORD dwTagId;

    DPRINT("Service: '%S'\n", lpServiceName);
    if (*lpServiceName == L'{')
        return ERROR_SUCCESS;

    dwSize = sizeof(DWORD);
    dwError = RegQueryValueExW(hServiceKey,
                               L"Type",
                               NULL,
                               NULL,
                               (LPBYTE)&dwServiceType,
                               &dwSize);
    if (dwError != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    if (((dwServiceType & ~SERVICE_INTERACTIVE_PROCESS) != SERVICE_WIN32_OWN_PROCESS) &&
        ((dwServiceType & ~SERVICE_INTERACTIVE_PROCESS) != SERVICE_WIN32_SHARE_PROCESS) &&
        (dwServiceType != SERVICE_KERNEL_DRIVER) &&
        (dwServiceType != SERVICE_FILE_SYSTEM_DRIVER))
        return ERROR_SUCCESS;

    DPRINT("Service type: %lx\n", dwServiceType);

    dwSize = sizeof(DWORD);
    dwError = RegQueryValueExW(hServiceKey,
                               L"Start",
                               NULL,
                               NULL,
                               (LPBYTE)&dwStartType,
                               &dwSize);
    if (dwError != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    DPRINT("Start type: %lx\n", dwStartType);

    dwSize = sizeof(DWORD);
    dwError = RegQueryValueExW(hServiceKey,
                               L"ErrorControl",
                               NULL,
                               NULL,
                               (LPBYTE)&dwErrorControl,
                               &dwSize);
    if (dwError != ERROR_SUCCESS)
        return ERROR_SUCCESS;

    DPRINT("Error control: %lx\n", dwErrorControl);

    dwError = RegQueryValueExW(hServiceKey,
                               L"Tag",
                               NULL,
                               NULL,
                               (LPBYTE)&dwTagId,
                               &dwSize);
    if (dwError != ERROR_SUCCESS)
        dwTagId = 0;

    DPRINT("Tag: %lx\n", dwTagId);

    dwError = ScmReadString(hServiceKey,
                            L"Group",
                            &lpGroup);
    if (dwError != ERROR_SUCCESS)
        lpGroup = NULL;

    DPRINT("Group: %S\n", lpGroup);

    dwError = ScmReadString(hServiceKey,
                            L"DisplayName",
                            &lpDisplayName);
    if (dwError != ERROR_SUCCESS)
        lpDisplayName = NULL;

    DPRINT("Display name: %S\n", lpDisplayName);

    dwError = ScmCreateNewServiceRecord(lpServiceName,
                                        &lpService);
    if (dwError != ERROR_SUCCESS)
        goto done;

    lpService->Status.dwServiceType = dwServiceType;
    lpService->dwStartType = dwStartType;
    lpService->dwErrorControl = dwErrorControl;
    lpService->dwTag = dwTagId;

    if (lpGroup != NULL)
    {
        dwError = ScmSetServiceGroup(lpService, lpGroup);
        if (dwError != ERROR_SUCCESS)
            goto done;
    }

    if (lpDisplayName != NULL)
    {
        lpService->lpDisplayName = lpDisplayName;
        lpDisplayName = NULL;
    }

    DPRINT("ServiceName: '%S'\n", lpService->lpServiceName);
    if (lpService->lpGroup != NULL)
    {
        DPRINT("Group: '%S'\n", lpService->lpGroup->lpGroupName);
    }
    DPRINT("Start %lx  Type %lx  Tag %lx  ErrorControl %lx\n",
           lpService->dwStartType,
           lpService->Status.dwServiceType,
           lpService->dwTag,
           lpService->dwErrorControl);

    if (ScmIsDeleteFlagSet(hServiceKey))
        lpService->bDeleted = TRUE;

done:;
    if (lpGroup != NULL)
        HeapFree(GetProcessHeap(), 0, lpGroup);

    if (lpDisplayName != NULL)
        HeapFree(GetProcessHeap(), 0, lpDisplayName);

    if (lpService != NULL)
    {
        if (lpService->lpImage != NULL)
            ScmDereferenceServiceImage(lpService->lpImage);
    }

    return dwError;
}


DWORD
ScmDeleteRegKey(HKEY hKey, LPCWSTR lpszSubKey)
{
    DWORD dwRet, dwMaxSubkeyLen = 0, dwSize;
    WCHAR szNameBuf[MAX_PATH], *lpszName = szNameBuf;
    HKEY hSubKey = 0;

    dwRet = RegOpenKeyExW(hKey, lpszSubKey, 0, KEY_READ, &hSubKey);
    if (!dwRet)
    {
        /* Find the maximum subkey length so that we can allocate a buffer */
        dwRet = RegQueryInfoKeyW(hSubKey, NULL, NULL, NULL, NULL,
                                                         &dwMaxSubkeyLen, NULL, NULL, NULL, NULL, NULL, NULL);
        if (!dwRet)
        {
            dwMaxSubkeyLen++;
            if (dwMaxSubkeyLen > sizeof(szNameBuf)/sizeof(WCHAR))
                /* Name too big: alloc a buffer for it */
                lpszName = HeapAlloc(GetProcessHeap(), 0, dwMaxSubkeyLen*sizeof(WCHAR));

            if(!lpszName)
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
            else
            {
                while (dwRet == ERROR_SUCCESS)
                {
                    dwSize = dwMaxSubkeyLen;
                    dwRet = RegEnumKeyExW(hSubKey, 0, lpszName, &dwSize, NULL, NULL, NULL, NULL);
                    if (dwRet == ERROR_SUCCESS || dwRet == ERROR_MORE_DATA)
                        dwRet = ScmDeleteRegKey(hSubKey, lpszName);
                }
                if (dwRet == ERROR_NO_MORE_ITEMS)
                    dwRet = ERROR_SUCCESS;

                if (lpszName != szNameBuf)
                    HeapFree(GetProcessHeap(), 0, lpszName); /* Free buffer if allocated */
            }
        }

        RegCloseKey(hSubKey);
        if (!dwRet)
            dwRet = RegDeleteKeyW(hKey, lpszSubKey);
    }
    return dwRet;
}


VOID
ScmDeleteMarkedServices(VOID)
{
    PLIST_ENTRY ServiceEntry;
    PSERVICE CurrentService;
    HKEY hServicesKey;
    DWORD dwError;

    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

        ServiceEntry = ServiceEntry->Flink;

        if (CurrentService->bDeleted == TRUE)
        {
            dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                    L"System\\CurrentControlSet\\Services",
                                    0,
                                    DELETE,
                                    &hServicesKey);
            if (dwError == ERROR_SUCCESS)
            {
                dwError = ScmDeleteRegKey(hServicesKey, CurrentService->lpServiceName);
                RegCloseKey(hServicesKey);
                if (dwError == ERROR_SUCCESS)
                {
                    RemoveEntryList(&CurrentService->ServiceListEntry);
                    HeapFree(GetProcessHeap(), 0, CurrentService);
                }
            }
            
            if (dwError != ERROR_SUCCESS)
                DPRINT1("Delete service failed: %S\n", CurrentService->lpServiceName);
        }
    }
}


VOID
WaitForLSA(VOID)
{
    HANDLE hEvent;
    DWORD dwError;

    DPRINT("WaitForLSA() called\n");

    hEvent = CreateEventW(NULL,
                          TRUE,
                          FALSE,
                          L"LSA_RPC_SERVER_ACTIVE");
    if (hEvent == NULL)
    {
        dwError = GetLastError();
        DPRINT1("Failed to create the notication event (Error %lu)\n", dwError);

        if (dwError == ERROR_ALREADY_EXISTS)
        {
            hEvent = OpenEventW(SYNCHRONIZE,
                                FALSE,
                                L"LSA_RPC_SERVER_ACTIVE");
            if (hEvent != NULL)
            {
               DPRINT1("Could not open the notification event!\n");
               return;
            }
        }
    }

    DPRINT("Wait for LSA!\n");
    WaitForSingleObject(hEvent, INFINITE);
    DPRINT("LSA is available!\n");

    CloseHandle(hEvent);

    DPRINT("WaitForLSA() done\n");
}


DWORD
ScmCreateServiceDatabase(VOID)
{
    WCHAR szSubKey[MAX_PATH];
    HKEY hServicesKey;
    HKEY hServiceKey;
    DWORD dwSubKey;
    DWORD dwSubKeyLength;
    FILETIME ftLastChanged;
    DWORD dwError;

    DPRINT("ScmCreateServiceDatabase() called\n");

    dwError = ScmCreateGroupList();
    if (dwError != ERROR_SUCCESS)
        return dwError;

    /* Initialize basic variables */
    InitializeListHead(&ImageListHead);
    InitializeListHead(&ServiceListHead);

    /* Initialize the database lock */
    RtlInitializeResource(&DatabaseLock);

    dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                            L"System\\CurrentControlSet\\Services",
                            0,
                            KEY_READ,
                            &hServicesKey);
    if (dwError != ERROR_SUCCESS)
        return dwError;

    dwSubKey = 0;
    for (;;)
    {
        dwSubKeyLength = MAX_PATH;
        dwError = RegEnumKeyExW(hServicesKey,
                                dwSubKey,
                                szSubKey,
                                &dwSubKeyLength,
                                NULL,
                                NULL,
                                NULL,
                                &ftLastChanged);
        if (dwError == ERROR_SUCCESS &&
            szSubKey[0] != L'{')
        {
            DPRINT("SubKeyName: '%S'\n", szSubKey);

            dwError = RegOpenKeyExW(hServicesKey,
                                    szSubKey,
                                    0,
                                    KEY_READ,
                                    &hServiceKey);
            if (dwError == ERROR_SUCCESS)
            {
                dwError = CreateServiceListEntry(szSubKey,
                                                 hServiceKey);

                RegCloseKey(hServiceKey);
            }
        }

        if (dwError != ERROR_SUCCESS)
            break;

        dwSubKey++;
    }

    RegCloseKey(hServicesKey);

    /* Wait for LSA */
    WaitForLSA();

    /* Delete services that are marked for delete */
    ScmDeleteMarkedServices();

    DPRINT("ScmCreateServiceDatabase() done\n");

    return ERROR_SUCCESS;
}


VOID
ScmShutdownServiceDatabase(VOID)
{
    DPRINT("ScmShutdownServiceDatabase() called\n");

    ScmDeleteMarkedServices();
    RtlDeleteResource(&DatabaseLock);

    DPRINT("ScmShutdownServiceDatabase() done\n");
}


static NTSTATUS
ScmCheckDriver(PSERVICE Service)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DirName;
    HANDLE DirHandle;
    NTSTATUS Status;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    ULONG BufferLength;
    ULONG DataLength;
    ULONG Index;

    DPRINT("ScmCheckDriver(%S) called\n", Service->lpServiceName);

    if (Service->Status.dwServiceType == SERVICE_KERNEL_DRIVER)
    {
        RtlInitUnicodeString(&DirName,
                             L"\\Driver");
    }
    else
    {
        RtlInitUnicodeString(&DirName,
                             L"\\FileSystem");
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &DirName,
                               0,
                               NULL,
                               NULL);

    Status = NtOpenDirectoryObject(&DirHandle,
                                   DIRECTORY_QUERY | DIRECTORY_TRAVERSE,
                                   &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    BufferLength = sizeof(OBJECT_DIRECTORY_INFORMATION) +
                   2 * MAX_PATH * sizeof(WCHAR);
    DirInfo = (OBJECT_DIRECTORY_INFORMATION*) HeapAlloc(GetProcessHeap(),
                        HEAP_ZERO_MEMORY,
                        BufferLength);

    Index = 0;
    while (TRUE)
    {
        Status = NtQueryDirectoryObject(DirHandle,
                                        DirInfo,
                                        BufferLength,
                                        TRUE,
                                        FALSE,
                                        &Index,
                                        &DataLength);
        if (Status == STATUS_NO_MORE_ENTRIES)
        {
            /* FIXME: Add current service to 'failed service' list */
            DPRINT("Service '%S' failed\n", Service->lpServiceName);
            break;
        }

        if (!NT_SUCCESS(Status))
            break;

        DPRINT("Comparing: '%S'  '%wZ'\n", Service->lpServiceName, &DirInfo->Name);

        if (_wcsicmp(Service->lpServiceName, DirInfo->Name.Buffer) == 0)
        {
            DPRINT("Found: '%S'  '%wZ'\n",
                   Service->lpServiceName, &DirInfo->Name);

            /* Mark service as 'running' */
            Service->Status.dwCurrentState = SERVICE_RUNNING;

            /* Mark the service group as 'running' */
            if (Service->lpGroup != NULL)
            {
                Service->lpGroup->ServicesRunning = TRUE;
            }

            break;
        }
    }

    HeapFree(GetProcessHeap(),
             0,
             DirInfo);
    NtClose(DirHandle);

    return STATUS_SUCCESS;
}


VOID
ScmGetBootAndSystemDriverState(VOID)
{
    PLIST_ENTRY ServiceEntry;
    PSERVICE CurrentService;

    DPRINT("ScmGetBootAndSystemDriverState() called\n");

    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

        if (CurrentService->dwStartType == SERVICE_BOOT_START ||
            CurrentService->dwStartType == SERVICE_SYSTEM_START)
        {
            /* Check driver */
            DPRINT("  Checking service: %S\n", CurrentService->lpServiceName);

            ScmCheckDriver(CurrentService);
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    DPRINT("ScmGetBootAndSystemDriverState() done\n");
}


DWORD
ScmControlService(PSERVICE Service,
                  DWORD dwControl)
{
    PSCM_CONTROL_PACKET ControlPacket;
    SCM_REPLY_PACKET ReplyPacket;

    DWORD dwWriteCount = 0;
    DWORD dwReadCount = 0;
    DWORD PacketSize;
    PWSTR Ptr;
    DWORD dwError = ERROR_SUCCESS;

    DPRINT("ScmControlService() called\n");

    EnterCriticalSection(&ControlServiceCriticalSection);

    /* Calculate the total length of the start command line */
    PacketSize = sizeof(SCM_CONTROL_PACKET);
    PacketSize += (wcslen(Service->lpServiceName) + 1) * sizeof(WCHAR);

    ControlPacket = (SCM_CONTROL_PACKET*)HeapAlloc(GetProcessHeap(),
                                                   HEAP_ZERO_MEMORY,
                                                   PacketSize);
    if (ControlPacket == NULL)
    {
        LeaveCriticalSection(&ControlServiceCriticalSection);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ControlPacket->dwSize = PacketSize;
    ControlPacket->dwControl = dwControl;
    ControlPacket->hServiceStatus = (SERVICE_STATUS_HANDLE)Service;

    ControlPacket->dwServiceNameOffset = sizeof(SCM_CONTROL_PACKET);

    Ptr = (PWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset);
    wcscpy(Ptr, Service->lpServiceName);

    ControlPacket->dwArgumentsCount = 0;
    ControlPacket->dwArgumentsOffset = 0;

    /* Send the control packet */
    WriteFile(Service->lpImage->hControlPipe,
              ControlPacket,
              PacketSize,
              &dwWriteCount,
              NULL);

    /* Read the reply */
    ReadFile(Service->lpImage->hControlPipe,
             &ReplyPacket,
             sizeof(SCM_REPLY_PACKET),
             &dwReadCount,
             NULL);

    /* Release the contol packet */
    HeapFree(GetProcessHeap(),
             0,
             ControlPacket);

    if (dwReadCount == sizeof(SCM_REPLY_PACKET))
    {
        dwError = ReplyPacket.dwError;
    }

    if (dwError == ERROR_SUCCESS &&
        dwControl == SERVICE_CONTROL_STOP)
    {
        ScmDereferenceServiceImage(Service->lpImage);
    }

    LeaveCriticalSection(&ControlServiceCriticalSection);

    DPRINT("ScmControlService() done\n");

    return dwError;
}


static DWORD
ScmSendStartCommand(PSERVICE Service,
                    DWORD argc,
                    LPWSTR *argv)
{
    PSCM_CONTROL_PACKET ControlPacket;
    SCM_REPLY_PACKET ReplyPacket;
    DWORD PacketSize;
    PWSTR Ptr;
    DWORD dwWriteCount = 0;
    DWORD dwReadCount = 0;
    DWORD dwError = ERROR_SUCCESS;
    DWORD i;
    PWSTR *pOffPtr;
    PWSTR pArgPtr;

    DPRINT("ScmSendStartCommand() called\n");

    /* Calculate the total length of the start command line */
    PacketSize = sizeof(SCM_CONTROL_PACKET);
    PacketSize += (wcslen(Service->lpServiceName) + 1) * sizeof(WCHAR);

    /* Calculate the required packet size for the start arguments */
    if (argc > 0 && argv != NULL)
    {
        PacketSize = ALIGN_UP(PacketSize, LPWSTR);

        DPRINT("Argc: %lu\n", argc);
        for (i = 0; i < argc; i++)
        {
            DPRINT("Argv[%lu]: %S\n", i, argv[i]);
            PacketSize += (wcslen(argv[i]) + 1) * sizeof(WCHAR) + sizeof(PWSTR);
        }
    }

    /* Allocate a control packet */
    ControlPacket = (SCM_CONTROL_PACKET*)HeapAlloc(GetProcessHeap(),
                                                   HEAP_ZERO_MEMORY,
                                                   PacketSize);
    if (ControlPacket == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    ControlPacket->dwSize = PacketSize;
    ControlPacket->dwControl = SERVICE_CONTROL_START;
    ControlPacket->hServiceStatus = (SERVICE_STATUS_HANDLE)Service;
    ControlPacket->dwServiceNameOffset = sizeof(SCM_CONTROL_PACKET);

    Ptr = (PWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset);
    wcscpy(Ptr, Service->lpServiceName);

    ControlPacket->dwArgumentsCount = 0;
    ControlPacket->dwArgumentsOffset = 0;

    /* Copy argument list */
    if (argc > 0 && argv != NULL)
    {
        Ptr += wcslen(Service->lpServiceName) + 1;
        pOffPtr = (PWSTR*)ALIGN_UP_POINTER(Ptr, PWSTR);
        pArgPtr = (PWSTR)((ULONG_PTR)pOffPtr + argc * sizeof(PWSTR));

        ControlPacket->dwArgumentsCount = argc;
        ControlPacket->dwArgumentsOffset = (DWORD)((ULONG_PTR)pOffPtr - (ULONG_PTR)ControlPacket);

        DPRINT("dwArgumentsCount: %lu\n", ControlPacket->dwArgumentsCount);
        DPRINT("dwArgumentsOffset: %lu\n", ControlPacket->dwArgumentsOffset);

        for (i = 0; i < argc; i++)
        {
             wcscpy(pArgPtr, argv[i]);
             *pOffPtr = (PWSTR)((ULONG_PTR)pArgPtr - (ULONG_PTR)pOffPtr);
             DPRINT("offset: %p\n", *pOffPtr);

             pArgPtr += wcslen(argv[i]) + 1;
             pOffPtr++;
        }
    }

    /* Send the start command */
    WriteFile(Service->lpImage->hControlPipe,
              ControlPacket,
              PacketSize,
              &dwWriteCount,
              NULL);

    /* Read the reply */
    ReadFile(Service->lpImage->hControlPipe,
             &ReplyPacket,
             sizeof(SCM_REPLY_PACKET),
             &dwReadCount,
             NULL);

    /* Release the contol packet */
    HeapFree(GetProcessHeap(),
             0,
             ControlPacket);

    if (dwReadCount == sizeof(SCM_REPLY_PACKET))
    {
        dwError = ReplyPacket.dwError;
    }

    DPRINT("ScmSendStartCommand() done\n");

    return dwError;
}


static DWORD
ScmStartUserModeService(PSERVICE Service,
                        DWORD argc,
                        LPWSTR *argv)
{
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFOW StartupInfo;
    BOOL Result;
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwProcessId;

    DPRINT("ScmStartUserModeService(%p)\n", Service);

    /* If the image is already running ... */
    if (Service->lpImage->dwImageRunCount > 1)
    {
        /* ... just send a start command */
        return ScmSendStartCommand(Service, argc, argv);
    }

    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.lpReserved = NULL;
    StartupInfo.lpDesktop = NULL;
    StartupInfo.lpTitle = NULL;
    StartupInfo.dwFlags = 0;
    StartupInfo.cbReserved2 = 0;
    StartupInfo.lpReserved2 = 0;

    Result = CreateProcessW(NULL,
                            Service->lpImage->szImagePath,
                            NULL,
                            NULL,
                            FALSE,
                            DETACHED_PROCESS | CREATE_SUSPENDED,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInformation);
    if (!Result)
    {
        dwError = GetLastError();
        DPRINT1("Starting '%S' failed!\n", Service->lpServiceName);
        return dwError;
    }

    DPRINT("Process Id: %lu  Handle %lx\n",
           ProcessInformation.dwProcessId,
           ProcessInformation.hProcess);
    DPRINT("Thread Id: %lu  Handle %lx\n",
           ProcessInformation.dwThreadId,
           ProcessInformation.hThread);

    /* Get process handle and id */
    Service->lpImage->dwProcessId = ProcessInformation.dwProcessId;
    Service->lpImage->hProcess = ProcessInformation.hProcess;

    /* Resume Thread */
    ResumeThread(ProcessInformation.hThread);

    /* Connect control pipe */
    if (ConnectNamedPipe(Service->lpImage->hControlPipe, NULL) ?
        TRUE : (dwError = GetLastError()) == ERROR_PIPE_CONNECTED)
    {
        DWORD dwRead = 0;

        DPRINT("Control pipe connected!\n");

        /* Read SERVICE_STATUS_HANDLE from pipe */
        if (!ReadFile(Service->lpImage->hControlPipe,
                      (LPVOID)&dwProcessId,
                      sizeof(DWORD),
                      &dwRead,
                      NULL))
        {
            dwError = GetLastError();
            DPRINT1("Reading the service control pipe failed (Error %lu)\n",
                    dwError);
        }
        else
        {
            DPRINT("Received service process ID %lu\n", dwProcessId);

            /* Send start command */
            dwError = ScmSendStartCommand(Service, argc, argv);
        }
    }
    else
    {
        DPRINT1("Connecting control pipe failed! (Error %lu)\n", dwError);
    }

    /* Close thread handle */
    CloseHandle(ProcessInformation.hThread);

    return dwError;
}


DWORD
ScmStartService(PSERVICE Service, DWORD argc, LPWSTR *argv)
{
    PSERVICE_GROUP Group = Service->lpGroup;
    DWORD dwError = ERROR_SUCCESS;
    LPCWSTR ErrorLogStrings[2];

    DPRINT("ScmStartService() called\n");

    DPRINT("Start Service %p (%S)\n", Service, Service->lpServiceName);

    EnterCriticalSection(&ControlServiceCriticalSection);

    if (Service->Status.dwCurrentState != SERVICE_STOPPED)
    {
        DPRINT("Service %S is already running!\n", Service->lpServiceName);
        LeaveCriticalSection(&ControlServiceCriticalSection);
        return ERROR_SERVICE_ALREADY_RUNNING;
    }

    DPRINT("Service->Type: %lu\n", Service->Status.dwServiceType);

    if (Service->Status.dwServiceType & SERVICE_DRIVER)
    {
        /* Load driver */
        dwError = ScmLoadDriver(Service);
        if (dwError == ERROR_SUCCESS)
        {
            Service->Status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
            Service->Status.dwCurrentState = SERVICE_RUNNING;
        }
    }
    else
    {
        /* Start user-mode service */
        dwError = ScmCreateOrReferenceServiceImage(Service);
        if (dwError == ERROR_SUCCESS)
        {
            dwError = ScmStartUserModeService(Service, argc, argv);
            if (dwError == ERROR_SUCCESS)
            {
#ifdef USE_SERVICE_START_PENDING
                Service->Status.dwCurrentState = SERVICE_START_PENDING;
#else
                Service->Status.dwCurrentState = SERVICE_RUNNING;
#endif
            }
            else
            {
                ScmDereferenceServiceImage(Service->lpImage);
                Service->lpImage = NULL;
            }
        }
    }

    LeaveCriticalSection(&ControlServiceCriticalSection);

    DPRINT("ScmStartService() done (Error %lu)\n", dwError);

    if (dwError == ERROR_SUCCESS)
    {
        if (Group != NULL)
        {
            Group->ServicesRunning = TRUE;
        }
    }
    else
    {
        if (Service->dwErrorControl != SERVICE_ERROR_IGNORE)
        {
            ErrorLogStrings[0] = Service->lpServiceName;
            ErrorLogStrings[1] = L"Test";
            ScmLogError(EVENT_SERVICE_START_FAILED,
                        2,
                        ErrorLogStrings);
        }

#if 0
        switch (Service->dwErrorControl)
        {
            case SERVICE_ERROR_SEVERE:
                if (IsLastKnownGood == FALSE)
                {
                    /* FIXME: Boot last known good configuration */
                }
                break;

            case SERVICE_ERROR_CRITICAL:
                if (IsLastKnownGood == FALSE)
                {
                    /* FIXME: Boot last known good configuration */
                }
                else
                {
                    /* FIXME: BSOD! */
                }
                break;
        }
#endif
    }

    return dwError;
}


VOID
ScmAutoStartServices(VOID)
{
    PLIST_ENTRY GroupEntry;
    PLIST_ENTRY ServiceEntry;
    PSERVICE_GROUP CurrentGroup;
    PSERVICE CurrentService;
    WCHAR szSafeBootServicePath[MAX_PATH];
    DWORD dwError;
    HKEY hKey;
    ULONG i;

    /* Clear 'ServiceVisited' flag (or set if not to start in Safe Mode) */
    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

        /* Build the safe boot path */
        wcscpy(szSafeBootServicePath,
               L"SYSTEM\\CurrentControlSet\\Control\\SafeBoot");

        switch (GetSystemMetrics(SM_CLEANBOOT))
        {
            /* NOTE: Assumes MINIMAL (1) and DSREPAIR (3) load same items */
            case 1:
            case 3:
                wcscat(szSafeBootServicePath, L"\\Minimal\\");
                break;

            case 2:
                wcscat(szSafeBootServicePath, L"\\Network\\");
                break;
        }

        if (GetSystemMetrics(SM_CLEANBOOT))
        {
            /* If key does not exist then do not assume safe mode */
            dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                    szSafeBootServicePath,
                                    0,
                                    KEY_READ,
                                    &hKey);
            if (dwError == ERROR_SUCCESS)
            {
                RegCloseKey(hKey);

                /* Finish Safe Boot path off */
                wcsncat(szSafeBootServicePath,
                        CurrentService->lpServiceName,
                        MAX_PATH - wcslen(szSafeBootServicePath));

                /* Check that the key is in the Safe Boot path */
                dwError = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                        szSafeBootServicePath,
                                        0,
                                        KEY_READ,
                                        &hKey);
                if (dwError != ERROR_SUCCESS)
                {
                    /* Mark service as visited so it is not auto-started */
                    CurrentService->ServiceVisited = TRUE;
                }
                else
                {
                    /* Must be auto-started in safe mode - mark as unvisited */
                    RegCloseKey(hKey);
                    CurrentService->ServiceVisited = FALSE;
                }
            }
            else
            {
                DPRINT1("WARNING: Could not open the associated Safe Boot key!");
                CurrentService->ServiceVisited = FALSE;
            }
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    /* Start all services which are members of an existing group */
    GroupEntry = GroupListHead.Flink;
    while (GroupEntry != &GroupListHead)
    {
        CurrentGroup = CONTAINING_RECORD(GroupEntry, SERVICE_GROUP, GroupListEntry);

        DPRINT("Group '%S'\n", CurrentGroup->lpGroupName);

        /* Start all services witch have a valid tag */
        for (i = 0; i < CurrentGroup->TagCount; i++)
        {
            ServiceEntry = ServiceListHead.Flink;
            while (ServiceEntry != &ServiceListHead)
            {
                CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

                if ((CurrentService->lpGroup == CurrentGroup) &&
                    (CurrentService->dwStartType == SERVICE_AUTO_START) &&
                    (CurrentService->ServiceVisited == FALSE) &&
                    (CurrentService->dwTag == CurrentGroup->TagArray[i]))
                {
                    CurrentService->ServiceVisited = TRUE;
                    ScmStartService(CurrentService, 0, NULL);
                }

                ServiceEntry = ServiceEntry->Flink;
             }
        }

        /* Start all services which have an invalid tag or which do not have a tag */
        ServiceEntry = ServiceListHead.Flink;
        while (ServiceEntry != &ServiceListHead)
        {
            CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

            if ((CurrentService->lpGroup == CurrentGroup) &&
                (CurrentService->dwStartType == SERVICE_AUTO_START) &&
                (CurrentService->ServiceVisited == FALSE))
            {
                CurrentService->ServiceVisited = TRUE;
                ScmStartService(CurrentService, 0, NULL);
            }

            ServiceEntry = ServiceEntry->Flink;
        }

        GroupEntry = GroupEntry->Flink;
    }

    /* Start all services which are members of any non-existing group */
    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

        if ((CurrentService->lpGroup != NULL) &&
            (CurrentService->dwStartType == SERVICE_AUTO_START) &&
            (CurrentService->ServiceVisited == FALSE))
        {
            CurrentService->ServiceVisited = TRUE;
            ScmStartService(CurrentService, 0, NULL);
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    /* Start all services which are not a member of any group */
    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

        if ((CurrentService->lpGroup == NULL) &&
            (CurrentService->dwStartType == SERVICE_AUTO_START) &&
            (CurrentService->ServiceVisited == FALSE))
        {
            CurrentService->ServiceVisited = TRUE;
            ScmStartService(CurrentService, 0, NULL);
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    /* Clear 'ServiceVisited' flag again */
    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);
        CurrentService->ServiceVisited = FALSE;
        ServiceEntry = ServiceEntry->Flink;
    }
}


VOID
ScmAutoShutdownServices(VOID)
{
    PLIST_ENTRY ServiceEntry;
    PSERVICE CurrentService;

    DPRINT("ScmAutoShutdownServices() called\n");

    /* Lock the service database exclusively */
    ScmLockDatabaseExclusive();

    ServiceEntry = ServiceListHead.Flink;
    while (ServiceEntry != &ServiceListHead)
    {
        CurrentService = CONTAINING_RECORD(ServiceEntry, SERVICE, ServiceListEntry);

        if (CurrentService->Status.dwCurrentState == SERVICE_RUNNING ||
            CurrentService->Status.dwCurrentState == SERVICE_START_PENDING)
        {
            /* shutdown service */
            DPRINT("Shutdown service: %S\n", CurrentService->szServiceName);
            ScmControlService(CurrentService, SERVICE_CONTROL_SHUTDOWN);
        }

        ServiceEntry = ServiceEntry->Flink;
    }

    /* Unlock the service database */
    ScmUnlockDatabase();

    DPRINT("ScmAutoShutdownServices() done\n");
}


BOOL
ScmLockDatabaseExclusive(VOID)
{
    return RtlAcquireResourceExclusive(&DatabaseLock, TRUE);
}


BOOL
ScmLockDatabaseShared(VOID)
{
    return RtlAcquireResourceShared(&DatabaseLock, TRUE);
}


VOID
ScmUnlockDatabase(VOID)
{
    RtlReleaseResource(&DatabaseLock);
}


VOID
ScmInitNamedPipeCriticalSection(VOID)
{
    InitializeCriticalSection(&ControlServiceCriticalSection);
}


VOID
ScmDeleteNamedPipeCriticalSection(VOID)
{
    DeleteCriticalSection(&ControlServiceCriticalSection);
}

/* EOF */
