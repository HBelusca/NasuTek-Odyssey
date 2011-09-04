/*
 * PROJECT:     Odyssey advapi32
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        dll/win32/advapi32/service/sctrl.c
 * PURPOSE:     Service control manager functions
 * COPYRIGHT:   Copyright 1999 Emanuele Aliberti
 *              Copyright 2007 Ged Murphy <gedmurphy@odyssey.org>
 *                             Gregor Brunmar <gregor.brunmar@home.se>
 *
 */


/* INCLUDES ******************************************************************/

#include <advapi32.h>
WINE_DEFAULT_DEBUG_CHANNEL(advapi);


/* TYPES *********************************************************************/

typedef struct _SERVICE_THREAD_PARAMSA
{
    LPSERVICE_MAIN_FUNCTIONA lpServiceMain;
    DWORD dwArgCount;
    LPSTR *lpArgVector;
} SERVICE_THREAD_PARAMSA, *PSERVICE_THREAD_PARAMSA;


typedef struct _SERVICE_THREAD_PARAMSW
{
    LPSERVICE_MAIN_FUNCTIONW lpServiceMain;
    DWORD dwArgCount;
    LPWSTR *lpArgVector;
} SERVICE_THREAD_PARAMSW, *PSERVICE_THREAD_PARAMSW;


typedef struct _ACTIVE_SERVICE
{
    SERVICE_STATUS_HANDLE hServiceStatus;
    UNICODE_STRING ServiceName;
    union
    {
        SERVICE_THREAD_PARAMSA A;
        SERVICE_THREAD_PARAMSW W;
    } ThreadParams;
    LPHANDLER_FUNCTION HandlerFunction;
    LPHANDLER_FUNCTION_EX HandlerFunctionEx;
    LPVOID HandlerContext;
    BOOL bUnicode;
} ACTIVE_SERVICE, *PACTIVE_SERVICE;


/* GLOBALS *******************************************************************/

static DWORD dwActiveServiceCount = 0;
static PACTIVE_SERVICE lpActiveServices = NULL;
static handle_t hStatusBinding = NULL;


/* FUNCTIONS *****************************************************************/

handle_t __RPC_USER
RPC_SERVICE_STATUS_HANDLE_bind(RPC_SERVICE_STATUS_HANDLE hServiceStatus)
{
    return hStatusBinding;
}


void __RPC_USER
RPC_SERVICE_STATUS_HANDLE_unbind(RPC_SERVICE_STATUS_HANDLE hServiceStatus,
                                 handle_t hBinding)
{
}


static RPC_STATUS
ScCreateStatusBinding(VOID)
{
    LPWSTR pszStringBinding;
    RPC_STATUS status;

    TRACE("ScCreateStatusBinding() called\n");

    status = RpcStringBindingComposeW(NULL,
                                      L"ncacn_np",
                                      NULL,
                                      L"\\pipe\\ntsvcs",
                                      NULL,
                                      &pszStringBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcStringBindingCompose returned 0x%x\n", status);
        return status;
    }

    /* Set the binding handle that will be used to bind to the server. */
    status = RpcBindingFromStringBindingW(pszStringBinding,
                                          &hStatusBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcBindingFromStringBinding returned 0x%x\n", status);
    }

    status = RpcStringFreeW(&pszStringBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcStringFree returned 0x%x\n", status);
    }

    return status;
}


static RPC_STATUS
ScDestroyStatusBinding(VOID)
{
    RPC_STATUS status;

    TRACE("ScDestroyStatusBinding() called\n");

    if (hStatusBinding == NULL)
        return RPC_S_OK;

    status = RpcBindingFree(&hStatusBinding);
    if (status != RPC_S_OK)
    {
        ERR("RpcBindingFree returned 0x%x\n", status);
    }
    else
    {
        hStatusBinding = NULL;
    }

    return status;
}


static PACTIVE_SERVICE
ScLookupServiceByServiceName(LPCWSTR lpServiceName)
{
    DWORD i;

    TRACE("ScLookupServiceByServiceName(%S) called\n", lpServiceName);

    for (i = 0; i < dwActiveServiceCount; i++)
    {
        TRACE("Checking %S\n", lpActiveServices[i].ServiceName.Buffer);
        if (_wcsicmp(lpActiveServices[i].ServiceName.Buffer, lpServiceName) == 0)
        {
            TRACE("Found!\n");
            return &lpActiveServices[i];
        }
    }

    TRACE("No service found!\n");

    SetLastError(ERROR_SERVICE_DOES_NOT_EXIST);

    return NULL;
}


static DWORD WINAPI
ScServiceMainStub(LPVOID Context)
{
    PACTIVE_SERVICE lpService = (PACTIVE_SERVICE)Context;

    TRACE("ScServiceMainStub() called\n");

    /* Call the main service routine and free the arguments vector */
    if (lpService->bUnicode)
    {
        (lpService->ThreadParams.W.lpServiceMain)(lpService->ThreadParams.W.dwArgCount,
                                                  lpService->ThreadParams.W.lpArgVector);

        if (lpService->ThreadParams.A.lpArgVector != NULL)
        {
            HeapFree(GetProcessHeap(),
                     0,
                     lpService->ThreadParams.W.lpArgVector);

            lpService->ThreadParams.W.lpArgVector = NULL;
            lpService->ThreadParams.W.dwArgCount = 0;
        }
    }
    else
    {
        (lpService->ThreadParams.A.lpServiceMain)(lpService->ThreadParams.A.dwArgCount,
                                                  lpService->ThreadParams.A.lpArgVector);

        if (lpService->ThreadParams.A.lpArgVector != NULL)
        {
            HeapFree(GetProcessHeap(),
                     0,
                     lpService->ThreadParams.A.lpArgVector);

            lpService->ThreadParams.A.lpArgVector = NULL;
            lpService->ThreadParams.A.dwArgCount = 0;
        }
    }

    return ERROR_SUCCESS;
}


static DWORD
ScConnectControlPipe(HANDLE *hPipe)
{
    DWORD dwBytesWritten;
    DWORD dwState;
    DWORD dwServiceCurrent = 0;
    NTSTATUS Status;
    WCHAR NtControlPipeName[MAX_PATH + 1];
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    DWORD dwProcessId;

    /* Get the service number and create the named pipe */
    RtlZeroMemory(&QueryTable,
                  sizeof(QueryTable));

    QueryTable[0].Name = L"";
    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
    QueryTable[0].EntryContext = &dwServiceCurrent;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
                                    L"ServiceCurrent",
                                    QueryTable,
                                    NULL,
                                    NULL);

    if (!NT_SUCCESS(Status))
    {
        ERR("RtlQueryRegistryValues() failed (Status %lx)\n", Status);
        return RtlNtStatusToDosError(Status);
    }

    swprintf(NtControlPipeName, L"\\\\.\\pipe\\net\\NtControlPipe%u", dwServiceCurrent);

    if (!WaitNamedPipeW(NtControlPipeName, 15000))
    {
        ERR("WaitNamedPipe(%S) failed (Error %lu)\n", NtControlPipeName, GetLastError());
        return ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
    }

    *hPipe = CreateFileW(NtControlPipeName,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
    if (*hPipe == INVALID_HANDLE_VALUE)
    {
        ERR("CreateFileW() failed for pipe %S (Error %lu)\n", NtControlPipeName, GetLastError());
        return ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
    }

    dwState = PIPE_READMODE_MESSAGE;
    if (!SetNamedPipeHandleState(*hPipe, &dwState, NULL, NULL))
    {
        CloseHandle(*hPipe);
        *hPipe = INVALID_HANDLE_VALUE;
        return ERROR_FAILED_SERVICE_CONTROLLER_CONNECT;
    }

    /* Pass the ProcessId to the SCM */
    dwProcessId = GetCurrentProcessId();
    WriteFile(*hPipe,
              &dwProcessId,
              sizeof(DWORD),
              &dwBytesWritten,
              NULL);

    TRACE("Sent Process ID %lu\n", dwProcessId);

    return ERROR_SUCCESS;
}


static DWORD
ScStartService(PACTIVE_SERVICE lpService,
               PSCM_CONTROL_PACKET ControlPacket)
{
    HANDLE ThreadHandle;
    DWORD ThreadId;
    LPWSTR *lpArgW;
    DWORD i;

    TRACE("ScStartService() called\n");
    TRACE("Size: %lu\n", ControlPacket->dwSize);
    TRACE("Service: %S\n", (PWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset));

    /* Set the service status handle */
    lpService->hServiceStatus = ControlPacket->hServiceStatus;

    if (lpService->bUnicode == TRUE)
    {
        lpService->ThreadParams.W.dwArgCount = ControlPacket->dwArgumentsCount;
        lpService->ThreadParams.W.lpArgVector = NULL;

        if (ControlPacket->dwArgumentsOffset > 0)
        {
            lpService->ThreadParams.W.lpArgVector =
                HeapAlloc(GetProcessHeap(),
                          HEAP_ZERO_MEMORY,
                          ControlPacket->dwSize - ControlPacket->dwArgumentsOffset);
            if (lpService->ThreadParams.W.lpArgVector == NULL)
                return ERROR_OUTOFMEMORY;

            memcpy(lpService->ThreadParams.W.lpArgVector,
                   ((PBYTE)ControlPacket + ControlPacket->dwArgumentsOffset),
                   ControlPacket->dwSize - ControlPacket->dwArgumentsOffset);

            lpArgW = lpService->ThreadParams.W.lpArgVector;
            for (i = 0; i < lpService->ThreadParams.W.dwArgCount; i++)
            {
                *lpArgW = (LPWSTR)((ULONG_PTR)lpArgW + (ULONG_PTR)*lpArgW);
                lpArgW++;
            }
        }
    }
    else
    {
        /* FIXME */
        lpService->ThreadParams.A.dwArgCount = 0;
        lpService->ThreadParams.A.lpArgVector = NULL;

#if 0
        LPSTR *lpArgVector;
        LPSTR Ptr;
        LPSTR AnsiString;
        DWORD AnsiLength;

        AnsiLength = WideCharToMultiByte(CP_ACP,
                                         0,
                                         lpService->Arguments,
                                         dwLength,
                                         NULL,
                                         0,
                                         NULL,
                                         NULL);
        if (AnsiLength == 0)
            return ERROR_INVALID_PARAMETER; /* ? */

        AnsiString = HeapAlloc(GetProcessHeap(),
                               0,
                               AnsiLength + 1);
        if (AnsiString == NULL)
            return ERROR_OUTOFMEMORY;

        WideCharToMultiByte(CP_ACP,
                            0,
                            lpService->Arguments,
                            dwLength,
                            AnsiString,
                            AnsiLength,
                            NULL,
                            NULL);

        AnsiString[AnsiLength] = ANSI_NULL;

        lpArgVector = HeapAlloc(GetProcessHeap(),
                                0,
                                (dwArgCount + 1) * sizeof(LPSTR));
        if (lpArgVector == NULL)
        {
            HeapFree(GetProcessHeap(),
                        0,
                        AnsiString);
            return ERROR_OUTOFMEMORY;
        }

        dwArgCount = 0;
        Ptr = AnsiString;
        while (*Ptr)
        {
            lpArgVector[dwArgCount] = Ptr;

            dwArgCount++;
            Ptr += (strlen(Ptr) + 1);
        }
        lpArgVector[dwArgCount] = NULL;

        (lpService->ThreadParams.A.lpServiceMain)(dwArgCount, lpArgVector);

        HeapFree(GetProcessHeap(),
                 0,
                 lpArgVector);
        HeapFree(GetProcessHeap(),
                 0,
                 AnsiString);
#endif
    }

    /* Invoke the services entry point and implement the command loop */
    ThreadHandle = CreateThread(NULL,
                                0,
                                ScServiceMainStub,
                                lpService,
                                CREATE_SUSPENDED,
                                &ThreadId);
    if (ThreadHandle == NULL)
    {
        /* Free the arguments vector */
        if (lpService->bUnicode)
        {
            if (lpService->ThreadParams.W.lpArgVector != NULL)
            {
                HeapFree(GetProcessHeap(),
                         0,
                         lpService->ThreadParams.W.lpArgVector);
                lpService->ThreadParams.W.lpArgVector = NULL;
                lpService->ThreadParams.W.dwArgCount = 0;
            }
        }
        else
        {
            if (lpService->ThreadParams.A.lpArgVector != NULL)
            {
                HeapFree(GetProcessHeap(),
                         0,
                         lpService->ThreadParams.A.lpArgVector);
                lpService->ThreadParams.A.lpArgVector = NULL;
                lpService->ThreadParams.A.dwArgCount = 0;
            }
        }

        return ERROR_SERVICE_NO_THREAD;
    }

    ResumeThread(ThreadHandle);
    CloseHandle(ThreadHandle);

    return ERROR_SUCCESS;
}


static DWORD
ScControlService(PACTIVE_SERVICE lpService,
                 PSCM_CONTROL_PACKET ControlPacket)
{
    TRACE("ScControlService() called\n");
    TRACE("Size: %lu\n", ControlPacket->dwSize);
    TRACE("Service: %S\n", (PWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset));

    if (lpService->HandlerFunction)
    {
        (lpService->HandlerFunction)(ControlPacket->dwControl);
    }
    else if (lpService->HandlerFunctionEx)
    {
        /* FIXME: send correct params */
        (lpService->HandlerFunctionEx)(ControlPacket->dwControl, 0, NULL, NULL);
    }

    TRACE("ScControlService() done\n");

    return ERROR_SUCCESS;
}


static BOOL
ScServiceDispatcher(HANDLE hPipe,
                    PUCHAR lpBuffer,
                    DWORD dwBufferSize)
{
    PSCM_CONTROL_PACKET ControlPacket;
    DWORD Count;
    BOOL bResult;
    DWORD dwRunningServices = 0;
    LPWSTR lpServiceName;
    PACTIVE_SERVICE lpService;
    SCM_REPLY_PACKET ReplyPacket;
    DWORD dwError;

    TRACE("ScDispatcherLoop() called\n");

    ControlPacket = HeapAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY,
                              1024);
    if (ControlPacket == NULL)
        return FALSE;

    while (TRUE)
    {
        /* Read command from the control pipe */
        bResult = ReadFile(hPipe,
                           ControlPacket,
                           1024,
                           &Count,
                           NULL);
        if (bResult == FALSE)
        {
            ERR("Pipe read failed (Error: %lu)\n", GetLastError());
            return FALSE;
        }

        lpServiceName = (LPWSTR)((PBYTE)ControlPacket + ControlPacket->dwServiceNameOffset);
        TRACE("Service: %S\n", lpServiceName);

        lpService = ScLookupServiceByServiceName(lpServiceName);
        if (lpService != NULL)
        {
            /* Execute command */
            switch (ControlPacket->dwControl)
            {
                case SERVICE_CONTROL_START:
                    TRACE("Start command - recieved SERVICE_CONTROL_START\n");
                    dwError = ScStartService(lpService, ControlPacket);
                    if (dwError == ERROR_SUCCESS)
                        dwRunningServices++;
                    break;

                case SERVICE_CONTROL_STOP:
                    TRACE("Stop command - recieved SERVICE_CONTROL_STOP\n");
                    dwError = ScControlService(lpService, ControlPacket);
                    if (dwError == ERROR_SUCCESS)
                        dwRunningServices--;
                    break;

                default:
                    TRACE("Command %lu received", ControlPacket->dwControl);
                    dwError = ScControlService(lpService, ControlPacket);
                    break;
            }
        }
        else
        {
            dwError = ERROR_SERVICE_DOES_NOT_EXIST;
        }

        ReplyPacket.dwError = dwError;

        /* Send the reply packet */
        bResult = WriteFile(hPipe,
                            &ReplyPacket,
                            sizeof(ReplyPacket),
                            &Count,
                            NULL);
        if (bResult == FALSE)
        {
            ERR("Pipe write failed (Error: %lu)\n", GetLastError());
            return FALSE;
        }

        if (dwRunningServices == 0)
            break;
    }

    HeapFree(GetProcessHeap(),
             0,
             ControlPacket);

    return TRUE;
}


/**********************************************************************
 *	RegisterServiceCtrlHandlerA
 *
 * @implemented
 */
SERVICE_STATUS_HANDLE WINAPI
RegisterServiceCtrlHandlerA(LPCSTR lpServiceName,
                            LPHANDLER_FUNCTION lpHandlerProc)
{
    ANSI_STRING ServiceNameA;
    UNICODE_STRING ServiceNameU;
    SERVICE_STATUS_HANDLE SHandle;

    RtlInitAnsiString(&ServiceNameA, (LPSTR)lpServiceName);
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&ServiceNameU, &ServiceNameA, TRUE)))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (SERVICE_STATUS_HANDLE)0;
    }

    SHandle = RegisterServiceCtrlHandlerW(ServiceNameU.Buffer,
                                          lpHandlerProc);

    RtlFreeUnicodeString(&ServiceNameU);

    return SHandle;
}


/**********************************************************************
 *	RegisterServiceCtrlHandlerW
 *
 * @implemented
 */
SERVICE_STATUS_HANDLE WINAPI
RegisterServiceCtrlHandlerW(LPCWSTR lpServiceName,
                            LPHANDLER_FUNCTION lpHandlerProc)
{
    PACTIVE_SERVICE Service;

    Service = ScLookupServiceByServiceName((LPWSTR)lpServiceName);
    if (Service == NULL)
    {
        return (SERVICE_STATUS_HANDLE)NULL;
    }

    Service->HandlerFunction = lpHandlerProc;
    Service->HandlerFunctionEx = NULL;

    TRACE("RegisterServiceCtrlHandler returning %lu\n", Service->hServiceStatus);

    return Service->hServiceStatus;
}


/**********************************************************************
 *	RegisterServiceCtrlHandlerExA
 *
 * @implemented
 */
SERVICE_STATUS_HANDLE WINAPI
RegisterServiceCtrlHandlerExA(LPCSTR lpServiceName,
                              LPHANDLER_FUNCTION_EX lpHandlerProc,
                              LPVOID lpContext)
{
    ANSI_STRING ServiceNameA;
    UNICODE_STRING ServiceNameU;
    SERVICE_STATUS_HANDLE SHandle;

    RtlInitAnsiString(&ServiceNameA, (LPSTR)lpServiceName);
    if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&ServiceNameU, &ServiceNameA, TRUE)))
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (SERVICE_STATUS_HANDLE)0;
    }

    SHandle = RegisterServiceCtrlHandlerExW(ServiceNameU.Buffer,
                                            lpHandlerProc,
                                            lpContext);

    RtlFreeUnicodeString(&ServiceNameU);

    return SHandle;
}


/**********************************************************************
 *	RegisterServiceCtrlHandlerExW
 *
 * @implemented
 */
SERVICE_STATUS_HANDLE WINAPI
RegisterServiceCtrlHandlerExW(LPCWSTR lpServiceName,
                              LPHANDLER_FUNCTION_EX lpHandlerProc,
                              LPVOID lpContext)
{
    PACTIVE_SERVICE Service;

    Service = ScLookupServiceByServiceName(lpServiceName);
    if (Service == NULL)
    {
        return (SERVICE_STATUS_HANDLE)NULL;
    }

    Service->HandlerFunction = NULL;
    Service->HandlerFunctionEx = lpHandlerProc;
    Service->HandlerContext = lpContext;

    TRACE("RegisterServiceCtrlHandlerEx returning %lu\n", Service->hServiceStatus);

    return Service->hServiceStatus;
}


/**********************************************************************
 *	I_ScSetServiceBitsA
 *
 * Undocumented
 *
 * @implemented
 */
BOOL WINAPI
I_ScSetServiceBitsA(SERVICE_STATUS_HANDLE hServiceStatus,
                    DWORD dwServiceBits,
                    BOOL bSetBitsOn,
                    BOOL bUpdateImmediately,
                    LPSTR lpString)
{
    BOOL bResult;

    RpcTryExcept
    {
        /* Call to services.exe using RPC */
        bResult = RI_ScSetServiceBitsA((RPC_SERVICE_STATUS_HANDLE)hServiceStatus,
                                       dwServiceBits,
                                       bSetBitsOn,
                                       bUpdateImmediately,
                                       lpString);
    }
    RpcExcept(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(ScmRpcStatusToWinError(RpcExceptionCode()));
        bResult = FALSE;
    }
    RpcEndExcept;

    return bResult;
}


/**********************************************************************
 *	I_ScSetServiceBitsW
 *
 * Undocumented
 *
 * @implemented
 */
BOOL WINAPI
I_ScSetServiceBitsW(SERVICE_STATUS_HANDLE hServiceStatus,
                    DWORD dwServiceBits,
                    BOOL bSetBitsOn,
                    BOOL bUpdateImmediately,
                    LPWSTR lpString)
{
    BOOL bResult;

    RpcTryExcept
    {
        /* Call to services.exe using RPC */
        bResult = RI_ScSetServiceBitsW((RPC_SERVICE_STATUS_HANDLE)hServiceStatus,
                                       dwServiceBits,
                                       bSetBitsOn,
                                       bUpdateImmediately,
                                       lpString);
    }
    RpcExcept(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError(ScmRpcStatusToWinError(RpcExceptionCode()));
        bResult = FALSE;
    }
    RpcEndExcept;

    return bResult;
}


/**********************************************************************
 *	SetServiceBits
 *
 * @implemented
 */
BOOL WINAPI
SetServiceBits(SERVICE_STATUS_HANDLE hServiceStatus,
               DWORD dwServiceBits,
               BOOL bSetBitsOn,
               BOOL bUpdateImmediately)
{
    return I_ScSetServiceBitsW(hServiceStatus,
                               dwServiceBits,
                               bSetBitsOn,
                               bUpdateImmediately,
                               NULL);
}


/**********************************************************************
 *	SetServiceStatus
 *
 * @implemented
 */
BOOL WINAPI
SetServiceStatus(SERVICE_STATUS_HANDLE hServiceStatus,
                 LPSERVICE_STATUS lpServiceStatus)
{
    DWORD dwError;

    TRACE("SetServiceStatus() called\n");
    TRACE("hServiceStatus %lu\n", hServiceStatus);

    RpcTryExcept
    {
        /* Call to services.exe using RPC */
        dwError = RSetServiceStatus((RPC_SERVICE_STATUS_HANDLE)hServiceStatus,
                                    lpServiceStatus);
    }
    RpcExcept(EXCEPTION_EXECUTE_HANDLER)
    {
        dwError = ScmRpcStatusToWinError(RpcExceptionCode());
    }
    RpcEndExcept;

    if (dwError != ERROR_SUCCESS)
    {
        ERR("ScmrSetServiceStatus() failed (Error %lu)\n", dwError);
        SetLastError(dwError);
        return FALSE;
    }

    TRACE("SetServiceStatus() done (ret %lu)\n", dwError);

    return TRUE;
}


/**********************************************************************
 *	StartServiceCtrlDispatcherA
 *
 * @implemented
 */
BOOL WINAPI
StartServiceCtrlDispatcherA(const SERVICE_TABLE_ENTRYA * lpServiceStartTable)
{
    ULONG i;
    HANDLE hPipe;
    DWORD dwError;
    PUCHAR lpMessageBuffer;

    TRACE("StartServiceCtrlDispatcherA() called\n");

    i = 0;
    while (lpServiceStartTable[i].lpServiceProc != NULL)
    {
        i++;
    }

    dwActiveServiceCount = i;
    lpActiveServices = RtlAllocateHeap(RtlGetProcessHeap(),
                                       HEAP_ZERO_MEMORY,
                                       dwActiveServiceCount * sizeof(ACTIVE_SERVICE));
    if (lpActiveServices == NULL)
    {
        return FALSE;
    }

    /* Copy service names and start procedure */
    for (i = 0; i < dwActiveServiceCount; i++)
    {
        RtlCreateUnicodeStringFromAsciiz(&lpActiveServices[i].ServiceName,
                                         lpServiceStartTable[i].lpServiceName);
        lpActiveServices[i].ThreadParams.A.lpServiceMain = lpServiceStartTable[i].lpServiceProc;
        lpActiveServices[i].hServiceStatus = 0;
        lpActiveServices[i].bUnicode = FALSE;
    }

    dwError = ScConnectControlPipe(&hPipe);
    if (dwError != ERROR_SUCCESS)
    {
        /* Free the service table */
        for (i = 0; i < dwActiveServiceCount; i++)
        {
            RtlFreeUnicodeString(&lpActiveServices[i].ServiceName);
        }
        RtlFreeHeap(RtlGetProcessHeap(), 0, lpActiveServices);
        lpActiveServices = NULL;
        dwActiveServiceCount = 0;
        return FALSE;
    }

    lpMessageBuffer = RtlAllocateHeap(RtlGetProcessHeap(),
                                      HEAP_ZERO_MEMORY,
                                      256);
    if (lpMessageBuffer == NULL)
    {
        /* Free the service table */
        for (i = 0; i < dwActiveServiceCount; i++)
        {
            RtlFreeUnicodeString(&lpActiveServices[i].ServiceName);
        }
        RtlFreeHeap(RtlGetProcessHeap(), 0, lpActiveServices);
        lpActiveServices = NULL;
        dwActiveServiceCount = 0;
        CloseHandle(hPipe);
        return FALSE;
    }

    ScCreateStatusBinding();

    ScServiceDispatcher(hPipe, lpMessageBuffer, 256);

    ScDestroyStatusBinding();

    CloseHandle(hPipe);

    /* Free the message buffer */
    RtlFreeHeap(RtlGetProcessHeap(), 0, lpMessageBuffer);

    /* Free the service table */
    for (i = 0; i < dwActiveServiceCount; i++)
    {
        RtlFreeUnicodeString(&lpActiveServices[i].ServiceName);
    }
    RtlFreeHeap(RtlGetProcessHeap(), 0, lpActiveServices);
    lpActiveServices = NULL;
    dwActiveServiceCount = 0;

    return TRUE;
}


/**********************************************************************
 *	StartServiceCtrlDispatcherW
 *
 * @implemented
 */
BOOL WINAPI
StartServiceCtrlDispatcherW(const SERVICE_TABLE_ENTRYW * lpServiceStartTable)
{
    ULONG i;
    HANDLE hPipe;
    DWORD dwError;
    PUCHAR lpMessageBuffer;

    TRACE("StartServiceCtrlDispatcherW() called\n");

    i = 0;
    while (lpServiceStartTable[i].lpServiceProc != NULL)
    {
        i++;
    }

    dwActiveServiceCount = i;
    lpActiveServices = RtlAllocateHeap(RtlGetProcessHeap(),
                                       HEAP_ZERO_MEMORY,
                                       dwActiveServiceCount * sizeof(ACTIVE_SERVICE));
    if (lpActiveServices == NULL)
    {
        return FALSE;
    }

    /* Copy service names and start procedure */
    for (i = 0; i < dwActiveServiceCount; i++)
    {
        RtlCreateUnicodeString(&lpActiveServices[i].ServiceName,
                               lpServiceStartTable[i].lpServiceName);
        lpActiveServices[i].ThreadParams.W.lpServiceMain = lpServiceStartTable[i].lpServiceProc;
        lpActiveServices[i].hServiceStatus = 0;
        lpActiveServices[i].bUnicode = TRUE;
    }

    dwError = ScConnectControlPipe(&hPipe);
    if (dwError != ERROR_SUCCESS)
    {
        /* Free the service table */
        for (i = 0; i < dwActiveServiceCount; i++)
        {
            RtlFreeUnicodeString(&lpActiveServices[i].ServiceName);
        }
        RtlFreeHeap(RtlGetProcessHeap(), 0, lpActiveServices);
        lpActiveServices = NULL;
        dwActiveServiceCount = 0;
        return FALSE;
    }

    lpMessageBuffer = RtlAllocateHeap(RtlGetProcessHeap(),
                                      HEAP_ZERO_MEMORY,
                                      256);
    if (lpMessageBuffer == NULL)
    {
        /* Free the service table */
        for (i = 0; i < dwActiveServiceCount; i++)
        {
            RtlFreeUnicodeString(&lpActiveServices[i].ServiceName);
        }
        RtlFreeHeap(RtlGetProcessHeap(), 0, lpActiveServices);
        lpActiveServices = NULL;
        dwActiveServiceCount = 0;
        CloseHandle(hPipe);
        return FALSE;
    }

    ScCreateStatusBinding();

    ScServiceDispatcher(hPipe, lpMessageBuffer, 256);

    ScDestroyStatusBinding();

    CloseHandle(hPipe);

    /* Free the message buffer */
    RtlFreeHeap(RtlGetProcessHeap(), 0, lpMessageBuffer);

    /* Free the service table */
    for (i = 0; i < dwActiveServiceCount; i++)
    {
        RtlFreeUnicodeString(&lpActiveServices[i].ServiceName);
    }
    RtlFreeHeap(RtlGetProcessHeap(), 0, lpActiveServices);
    lpActiveServices = NULL;
    dwActiveServiceCount = 0;

    return TRUE;
}

/* EOF */
