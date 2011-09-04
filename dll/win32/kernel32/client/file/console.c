/* $Id: console.c 52819 2011-07-23 18:54:29Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            dll/win32/kernel32/misc/console.c
 * PURPOSE:         Win32 server console functions
 * PROGRAMMER:      James Tabor
 *            <jimtabor@adsl-64-217-116-74.dsl.hstntx.swbell.net>
 * UPDATE HISTORY:
 *    199901?? ??    Created
 *    19990204 EA    SetConsoleTitleA
 *      19990306 EA    Stubs
 */

/* INCLUDES ******************************************************************/

#include <k32.h>

#define NDEBUG
#include <debug.h>

extern RTL_CRITICAL_SECTION ConsoleLock;
extern BOOL ConsoleInitialized;
extern BOOL WINAPI IsDebuggerPresent(VOID);

/* GLOBALS *******************************************************************/

PHANDLER_ROUTINE InitialHandler[1];
PHANDLER_ROUTINE* CtrlHandlers;
ULONG NrCtrlHandlers;
ULONG NrAllocatedHandlers;

#define INPUTEXENAME_BUFLEN 256
static WCHAR InputExeName[INPUTEXENAME_BUFLEN] = L"";

/* Default Console Control Handler *******************************************/

BOOL
WINAPI
DefaultConsoleCtrlHandler(DWORD Event)
{
    DPRINT("Default handler called: %lx\n", Event);
    switch(Event)
    {
        case CTRL_C_EVENT:
            DPRINT("Ctrl-C Event\n");
            break;

        case CTRL_BREAK_EVENT:
            DPRINT("Ctrl-Break Event\n");
            break;

        case CTRL_SHUTDOWN_EVENT:
            DPRINT("Ctrl Shutdown Event\n");
            break;

        case CTRL_CLOSE_EVENT:
            DPRINT("Ctrl Close Event\n");
            break;

        case CTRL_LOGOFF_EVENT:
            DPRINT("Ctrl Logoff Event\n");
            break;
    }

    ExitProcess(CONTROL_C_EXIT);
    return TRUE;
}

__declspec(noreturn)
VOID
CALLBACK
ConsoleControlDispatcher(DWORD CodeAndFlag)
{
    DWORD nExitCode = 0;
    DWORD nCode = CodeAndFlag & MAXLONG;
    UINT i;
    EXCEPTION_RECORD erException;
    
    DPRINT("Console Dispatcher Active: %lx %lx\n", CodeAndFlag, nCode);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    switch(nCode)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        {
            if (IsDebuggerPresent())
            {
                erException.ExceptionCode = (nCode == CTRL_C_EVENT ?
                                             DBG_CONTROL_C : DBG_CONTROL_BREAK);
                erException.ExceptionFlags = 0;
                erException.ExceptionRecord = NULL;
                erException.ExceptionAddress = DefaultConsoleCtrlHandler;
                erException.NumberParameters = 0;
                
                _SEH2_TRY
                {
                    RtlRaiseException(&erException);
                }
                _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
                {
                    RtlEnterCriticalSection(&ConsoleLock);
                    
                    if ((nCode != CTRL_C_EVENT) ||
                        (NtCurrentPeb()->ProcessParameters->ConsoleFlags != 1))
                    {
                        for (i = NrCtrlHandlers; i > 0; i--)
                        {
                            if (CtrlHandlers[i - 1](nCode)) break;
                        }
                    }
                    
                    RtlLeaveCriticalSection(&ConsoleLock);
                }
                _SEH2_END;
                
                ExitThread(0);
            }
            
            break;
        }

        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            break;
            
        case 3:
        
            ExitThread(0);
            break;
        
        case 4:
        
            ExitProcess(CONTROL_C_EXIT);
            break;

        default:
        
            ASSERT(FALSE);
            break;
    }
    
    ASSERT(ConsoleInitialized);
    
    RtlEnterCriticalSection(&ConsoleLock);
    nExitCode = 0;
    if ((nCode != CTRL_C_EVENT) || (NtCurrentPeb()->ProcessParameters->ConsoleFlags != 1))
    {
        for (i = NrCtrlHandlers; i > 0; i--)
        {
            if ((i == 1) &&
                (CodeAndFlag & MINLONG) &&
                ((nCode == CTRL_LOGOFF_EVENT) || (nCode == CTRL_SHUTDOWN_EVENT)))
            {
                DPRINT("Skipping system/service apps\n");
                break;
            }

            if (CtrlHandlers[i - 1](nCode))
            {
                switch(nCode)
                {
                    case CTRL_CLOSE_EVENT:
                    case CTRL_LOGOFF_EVENT:
                    case CTRL_SHUTDOWN_EVENT:
                    case 3:
                        nExitCode = CodeAndFlag;
                        break;
                }
                break;
            }
        }
    }
    
    RtlLeaveCriticalSection(&ConsoleLock);
    ExitThread(nExitCode);
}

/* Get the size needed to copy a string to a capture buffer, including alignment */
static ULONG
IntStringSize(LPCVOID String,
              BOOL Unicode)
{
    ULONG Size = (Unicode ? wcslen(String) : strlen(String)) * sizeof(WCHAR);
    return (Size + 3) & -4;
}

/* Copy a string to a capture buffer */
static VOID
IntCaptureMessageString(PCSR_CAPTURE_BUFFER CaptureBuffer,
                        LPCVOID String,
                        BOOL Unicode,
                        PUNICODE_STRING RequestString)
{
    ULONG Size;
    if (Unicode)
    {
        Size = wcslen(String) * sizeof(WCHAR);
        CsrCaptureMessageBuffer(CaptureBuffer, (PVOID)String, Size, (PVOID *)&RequestString->Buffer);
    }
    else
    {
        Size = strlen(String);
        CsrAllocateMessagePointer(CaptureBuffer, Size * sizeof(WCHAR), (PVOID *)&RequestString->Buffer);
        Size = MultiByteToWideChar(CP_ACP, 0, String, Size, RequestString->Buffer, Size * sizeof(WCHAR))
               * sizeof(WCHAR);
    }
    RequestString->Length = RequestString->MaximumLength = Size;
}

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
BOOL
WINAPI
AddConsoleAliasA(LPCSTR lpSource,
                 LPCSTR lpTarget,
                 LPCSTR lpExeName)
{
    LPWSTR lpSourceW = NULL;
    LPWSTR lpTargetW = NULL;
    LPWSTR lpExeNameW = NULL;
    BOOL bRetVal;

    if (lpSource)
        BasepAnsiStringToHeapUnicodeString(lpSource, (LPWSTR*) &lpSourceW);
    if (lpTarget)
        BasepAnsiStringToHeapUnicodeString(lpTarget, (LPWSTR*) &lpTargetW);
    if (lpExeName)
        BasepAnsiStringToHeapUnicodeString(lpExeName, (LPWSTR*) &lpExeNameW);

    bRetVal = AddConsoleAliasW(lpSourceW, lpTargetW, lpExeNameW);

    /* Clean up */
    if (lpSourceW)
        RtlFreeHeap(GetProcessHeap(), 0, (LPWSTR*) lpSourceW);
    if (lpTargetW)
        RtlFreeHeap(GetProcessHeap(), 0, (LPWSTR*) lpTargetW);
    if (lpExeNameW)
        RtlFreeHeap(GetProcessHeap(), 0, (LPWSTR*) lpExeNameW);

    return bRetVal;
}


/*
 * @unimplemented
 */
BOOL
WINAPI
AddConsoleAliasW(LPCWSTR lpSource,
                 LPCWSTR lpTarget,
                 LPCWSTR lpExeName)
{
    PCSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    ULONG SourceLength;
    ULONG TargetLength = 0;
    ULONG ExeLength;
    ULONG Size;
    ULONG RequestLength;
    WCHAR * Ptr;

    DPRINT("AddConsoleAliasW enterd with lpSource %S lpTarget %S lpExeName %S\n", lpSource, lpTarget, lpExeName);

    ExeLength = wcslen(lpExeName) + 1;
    SourceLength = wcslen(lpSource)+ 1;
    if (lpTarget)
        TargetLength = wcslen(lpTarget) + 1;

    Size = (ExeLength + SourceLength + TargetLength) * sizeof(WCHAR);
    RequestLength = sizeof(CSR_API_MESSAGE) + Size;

    Request = RtlAllocateHeap(GetProcessHeap(), HEAP_ZERO_MEMORY, RequestLength);
    Ptr = (WCHAR*)(((ULONG_PTR)Request) + sizeof(CSR_API_MESSAGE));

    wcscpy(Ptr, lpSource);
    Request->Data.AddConsoleAlias.SourceLength = SourceLength;
    Ptr = (WCHAR*)(((ULONG_PTR)Request) + sizeof(CSR_API_MESSAGE) + SourceLength * sizeof(WCHAR));

    wcscpy(Ptr, lpExeName);
    Request->Data.AddConsoleAlias.ExeLength = ExeLength;
    Ptr = (WCHAR*)(((ULONG_PTR)Request) + sizeof(CSR_API_MESSAGE) + (ExeLength + SourceLength)* sizeof(WCHAR));

    if (lpTarget) /* target can be optional */
        wcscpy(Ptr, lpTarget);

    Request->Data.AddConsoleAlias.TargetLength = TargetLength;

    CsrRequest = MAKE_CSR_API(ADD_CONSOLE_ALIAS, CSR_CONSOLE);
    Status = CsrClientCallServer(Request,
                                 NULL,
                                 CsrRequest,
                                 RequestLength);

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request->Status))
    {
        BaseSetLastNTError(Status);
        RtlFreeHeap(GetProcessHeap(), 0, Request);
        return FALSE;
    }

    RtlFreeHeap(GetProcessHeap(), 0, Request);
    return TRUE;
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
ConsoleMenuControl(HANDLE hConsole,
                   DWORD Unknown1,
                   DWORD Unknown2)
{
    DPRINT1("ConsoleMenuControl(0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", hConsole, Unknown1, Unknown2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @implemented
 */
HANDLE
WINAPI
DuplicateConsoleHandle(HANDLE hConsole,
                       DWORD dwDesiredAccess,
                       BOOL bInheritHandle,
                       DWORD dwOptions)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    if (dwOptions & ~(DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)
        || (!(dwOptions & DUPLICATE_SAME_ACCESS)
        && dwDesiredAccess & ~(GENERIC_READ | GENERIC_WRITE)))
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    CsrRequest = MAKE_CSR_API(DUPLICATE_HANDLE, CSR_NATIVE);
    Request.Data.DuplicateHandleRequest.Handle = hConsole;
    Request.Data.DuplicateHandleRequest.Access = dwDesiredAccess;
    Request.Data.DuplicateHandleRequest.Inheritable = bInheritHandle;
    Request.Data.DuplicateHandleRequest.Options = dwOptions;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status=Request.Status))
    {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    return Request.Data.DuplicateHandleRequest.Handle;
}


static BOOL
IntExpungeConsoleCommandHistory(LPCVOID lpExeName, BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(EXPUNGE_COMMAND_HISTORY, CSR_CONSOLE);
    NTSTATUS Status;

    if (lpExeName == NULL || !(bUnicode ? *(PWCHAR)lpExeName : *(PCHAR)lpExeName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer(1, IntStringSize(lpExeName, bUnicode));
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
                            &Request.Data.ExpungeCommandHistory.ExeName);
    Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
    CsrFreeCaptureBuffer(CaptureBuffer);
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
ExpungeConsoleCommandHistoryW(LPCWSTR lpExeName)
{
    return IntExpungeConsoleCommandHistory(lpExeName, TRUE);
}

/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
ExpungeConsoleCommandHistoryA(LPCSTR lpExeName)
{
    return IntExpungeConsoleCommandHistory(lpExeName, FALSE);
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasW(LPWSTR lpSource,
                 LPWSTR lpTargetBuffer,
                 DWORD TargetBufferLength,
                 LPWSTR lpExeName)
{
    PCSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest;
    NTSTATUS Status;
    ULONG Size;
    ULONG ExeLength;
    ULONG SourceLength;
    ULONG RequestLength;
    WCHAR * Ptr;

    DPRINT("GetConsoleAliasW entered lpSource %S lpExeName %S\n", lpSource, lpExeName);

    if (lpTargetBuffer == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_ALIAS, CSR_CONSOLE);

    ExeLength = wcslen(lpExeName) + 1;
    SourceLength = wcslen(lpSource) + 1;

    Size = (ExeLength + SourceLength) * sizeof(WCHAR);

    RequestLength = Size + sizeof(CSR_API_MESSAGE);
    Request = RtlAllocateHeap(GetProcessHeap(), 0, RequestLength);
    if (Request == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer(1, TargetBufferLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        RtlFreeHeap(GetProcessHeap(), 0, Request);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    Request->Data.GetConsoleAlias.TargetBuffer = NULL;

    CsrCaptureMessageBuffer(CaptureBuffer,
                            NULL,
                            TargetBufferLength,
                            (PVOID*)&Request->Data.GetConsoleAlias.TargetBuffer);

    Request->Data.GetConsoleAlias.TargetBufferLength = TargetBufferLength;

    Ptr = (LPWSTR)((ULONG_PTR)Request + sizeof(CSR_API_MESSAGE));
    wcscpy(Ptr, lpSource);
    Ptr += SourceLength;
    wcscpy(Ptr, lpExeName);

    Request->Data.GetConsoleAlias.ExeLength = ExeLength;
    Request->Data.GetConsoleAlias.SourceLength = SourceLength;

    Status = CsrClientCallServer(Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE) + Size);

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request->Status))
    {
        RtlFreeHeap(GetProcessHeap(), 0, Request);
        CsrFreeCaptureBuffer(CaptureBuffer);
        BaseSetLastNTError(Status);
        return 0;
    }

    wcscpy(lpTargetBuffer, Request->Data.GetConsoleAlias.TargetBuffer);
    RtlFreeHeap(GetProcessHeap(), 0, Request);
    CsrFreeCaptureBuffer(CaptureBuffer);

    return Request->Data.GetConsoleAlias.BytesWritten;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasA(LPSTR lpSource,
                 LPSTR lpTargetBuffer,
                 DWORD TargetBufferLength,
                 LPSTR lpExeName)
{
    LPWSTR lpwSource;
    LPWSTR lpwExeName;
    LPWSTR lpwTargetBuffer;
    UINT dwSourceSize;
    UINT dwExeNameSize;
    UINT dwResult;

    DPRINT("GetConsoleAliasA entered\n");

    if (lpTargetBuffer == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    dwSourceSize = (strlen(lpSource)+1) * sizeof(WCHAR);
    lpwSource = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSourceSize);
    if (lpwSource == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }
    MultiByteToWideChar(CP_ACP, 0, lpSource, -1, lpwSource, dwSourceSize);

    dwExeNameSize = (strlen(lpExeName)+1) * sizeof(WCHAR);
    lpwExeName = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwExeNameSize);
    if (lpwExeName == NULL)
    {
        HeapFree(GetProcessHeap(), 0, lpwSource);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }
    MultiByteToWideChar(CP_ACP, 0, lpExeName, -1, lpwExeName, dwExeNameSize);

    lpwTargetBuffer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, TargetBufferLength * sizeof(WCHAR));
    if (lpwTargetBuffer == NULL)
    {
        HeapFree(GetProcessHeap(), 0, lpwSource);
        HeapFree(GetProcessHeap(), 0, lpwExeName);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    dwResult = GetConsoleAliasW(lpwSource, lpwTargetBuffer, TargetBufferLength * sizeof(WCHAR), lpwExeName);

    HeapFree(GetProcessHeap(), 0, lpwSource);
    HeapFree(GetProcessHeap(), 0, lpwExeName);

    if (dwResult)
        dwResult = WideCharToMultiByte(CP_ACP, 0, lpwTargetBuffer, dwResult / sizeof(WCHAR), lpTargetBuffer, TargetBufferLength, NULL, NULL);

    HeapFree(GetProcessHeap(), 0, lpwTargetBuffer);

    return dwResult;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasExesW(LPWSTR lpExeNameBuffer,
                     DWORD ExeNameBufferLength)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest;
    NTSTATUS Status;

    DPRINT("GetConsoleAliasExesW entered\n");

    CaptureBuffer = CsrAllocateCaptureBuffer(1, ExeNameBufferLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_ALIASES_EXES, CSR_CONSOLE);
    CsrAllocateMessagePointer(CaptureBuffer,
                              ExeNameBufferLength,
                              (PVOID*)&Request.Data.GetConsoleAliasesExes.ExeNames);
    Request.Data.GetConsoleAliasesExes.Length = ExeNameBufferLength;

    Status = CsrClientCallServer(&Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        CsrFreeCaptureBuffer(CaptureBuffer);
        return 0;
    }

    memcpy(lpExeNameBuffer,
           Request.Data.GetConsoleAliasesExes.ExeNames,
           Request.Data.GetConsoleAliasesExes.BytesWritten);

    CsrFreeCaptureBuffer(CaptureBuffer);
    return Request.Data.GetConsoleAliasesExes.BytesWritten;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasExesA(LPSTR lpExeNameBuffer,
                     DWORD ExeNameBufferLength)
{
    LPWSTR lpwExeNameBuffer;
    DWORD dwResult;

    DPRINT("GetConsoleAliasExesA entered\n");

    lpwExeNameBuffer = HeapAlloc(GetProcessHeap(), 0, ExeNameBufferLength * sizeof(WCHAR));

    dwResult = GetConsoleAliasExesW(lpwExeNameBuffer, ExeNameBufferLength * sizeof(WCHAR));

    if (dwResult)
        dwResult = WideCharToMultiByte(CP_ACP, 0, lpwExeNameBuffer, dwResult / sizeof(WCHAR), lpExeNameBuffer, ExeNameBufferLength, NULL, NULL);

    HeapFree(GetProcessHeap(), 0, lpwExeNameBuffer);
    return dwResult;
}

/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasExesLengthW(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    DPRINT("GetConsoleAliasExesLengthW entered\n");

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_ALIASES_EXES_LENGTH, CSR_CONSOLE);
    Request.Data.GetConsoleAliasesExesLength.Length = 0;


    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return 0;
    }

    return Request.Data.GetConsoleAliasesExesLength.Length;
}

/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasExesLengthA(VOID)
{
    DWORD dwLength;

    DPRINT("GetConsoleAliasExesLengthA entered\n");

    dwLength = GetConsoleAliasExesLengthW();

    if (dwLength)
        dwLength /= sizeof(WCHAR);

    return dwLength;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasesW(LPWSTR AliasBuffer,
                   DWORD AliasBufferLength,
                   LPWSTR ExeName)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    DWORD dwLength;

    DPRINT("GetConsoleAliasesW entered\n");

    dwLength = GetConsoleAliasesLengthW(ExeName);
    if (!dwLength || dwLength > AliasBufferLength)
        return 0;

    CsrRequest = MAKE_CSR_API(GET_ALL_CONSOLE_ALIASES, CSR_CONSOLE);
    Request.Data.GetAllConsoleAlias.AliasBuffer = AliasBuffer;
    Request.Data.GetAllConsoleAlias.AliasBufferLength = AliasBufferLength;
    Request.Data.GetAllConsoleAlias.lpExeName = ExeName;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return 0;
    }

    return Request.Data.GetAllConsoleAlias.BytesWritten / sizeof(WCHAR);
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasesA(LPSTR AliasBuffer,
                   DWORD AliasBufferLength,
                   LPSTR ExeName)
{
    DWORD dwRetVal = 0;
    LPWSTR lpwExeName = NULL;
    LPWSTR lpwAliasBuffer;

    DPRINT("GetConsoleAliasesA entered\n");

    if (ExeName)
        BasepAnsiStringToHeapUnicodeString(ExeName, (LPWSTR*) &lpwExeName);

    lpwAliasBuffer = HeapAlloc(GetProcessHeap(), 0, AliasBufferLength * sizeof(WCHAR));

    dwRetVal = GetConsoleAliasesW(lpwAliasBuffer, AliasBufferLength, lpwExeName);

    if (lpwExeName)
        RtlFreeHeap(GetProcessHeap(), 0, (LPWSTR*) lpwExeName);

    if (dwRetVal)
        dwRetVal = WideCharToMultiByte(CP_ACP, 0, lpwAliasBuffer, dwRetVal, AliasBuffer, AliasBufferLength, NULL, NULL);

    HeapFree(GetProcessHeap(), 0, lpwAliasBuffer);
    return dwRetVal;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasesLengthW(LPWSTR lpExeName)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    DPRINT("GetConsoleAliasesLengthW entered\n");

    CsrRequest = MAKE_CSR_API(GET_ALL_CONSOLE_ALIASES_LENGTH, CSR_CONSOLE);
    Request.Data.GetAllConsoleAliasesLength.lpExeName = lpExeName;
    Request.Data.GetAllConsoleAliasesLength.Length = 0;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return 0;
    }

    return Request.Data.GetAllConsoleAliasesLength.Length;
}


/*
 * @implemented
 */
DWORD
WINAPI
GetConsoleAliasesLengthA(LPSTR lpExeName)
{
    DWORD dwRetVal = 0;
    LPWSTR lpExeNameW = NULL;

    if (lpExeName)
        BasepAnsiStringToHeapUnicodeString(lpExeName, (LPWSTR*) &lpExeNameW);

    dwRetVal = GetConsoleAliasesLengthW(lpExeNameW);
    if (dwRetVal)
        dwRetVal /= sizeof(WCHAR);

    /* Clean up */
    if (lpExeNameW)
        RtlFreeHeap(GetProcessHeap(), 0, (LPWSTR*) lpExeNameW);

    return dwRetVal;
}


static DWORD
IntGetConsoleCommandHistory(LPVOID lpHistory, DWORD cbHistory, LPCVOID lpExeName, BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(GET_COMMAND_HISTORY, CSR_CONSOLE);
    NTSTATUS Status;
    DWORD HistoryLength = cbHistory * (bUnicode ? 1 : sizeof(WCHAR));

    if (lpExeName == NULL || !(bUnicode ? *(PWCHAR)lpExeName : *(PCHAR)lpExeName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer(2, IntStringSize(lpExeName, bUnicode) +
                                                HistoryLength);
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }
    IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
                            &Request.Data.GetCommandHistory.ExeName);
    Request.Data.GetCommandHistory.Length = HistoryLength;
    CsrAllocateMessagePointer(CaptureBuffer, HistoryLength,
                              (PVOID*)&Request.Data.GetCommandHistory.History);

    Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        CsrFreeCaptureBuffer(CaptureBuffer);
        BaseSetLastNTError(Status);
        return 0;
    }

    if (bUnicode)
    {
        memcpy(lpHistory,
               Request.Data.GetCommandHistory.History,
               Request.Data.GetCommandHistory.Length);
    }
    else
    {
        WideCharToMultiByte(CP_ACP, 0,
                            Request.Data.GetCommandHistory.History,
                            Request.Data.GetCommandHistory.Length / sizeof(WCHAR),
                            lpHistory,
                            cbHistory,
                            NULL, NULL);
    }
    CsrFreeCaptureBuffer(CaptureBuffer);
    return Request.Data.GetCommandHistory.Length;
}

/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryW(LPWSTR lpHistory,
                          DWORD cbHistory,
                          LPCWSTR lpExeName)
{
    return IntGetConsoleCommandHistory(lpHistory, cbHistory, lpExeName, TRUE);
}

/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryA(LPSTR lpHistory,
                          DWORD cbHistory,
                          LPCSTR lpExeName)
{
    return IntGetConsoleCommandHistory(lpHistory, cbHistory, lpExeName, FALSE);
}


static DWORD
IntGetConsoleCommandHistoryLength(LPCVOID lpExeName, BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(GET_COMMAND_HISTORY_LENGTH, CSR_CONSOLE);
    NTSTATUS Status;

    if (lpExeName == NULL || !(bUnicode ? *(PWCHAR)lpExeName : *(PCHAR)lpExeName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer(1, IntStringSize(lpExeName, bUnicode));
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }
    IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
                            &Request.Data.GetCommandHistoryLength.ExeName);
    Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
    CsrFreeCaptureBuffer(CaptureBuffer);
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return 0;
    }
    return Request.Data.GetCommandHistoryLength.Length;
}

/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryLengthW(LPCWSTR lpExeName)
{
    return IntGetConsoleCommandHistoryLength(lpExeName, TRUE);
}

/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleCommandHistoryLengthA(LPCSTR lpExeName)
{
    return IntGetConsoleCommandHistoryLength(lpExeName, FALSE) / sizeof(WCHAR);
}


/*
 * @unimplemented
 */
INT
WINAPI
GetConsoleDisplayMode(LPDWORD lpdwMode)
     /*
      * FUNCTION: Get the console display mode
      * ARGUMENTS:
      *      lpdwMode - Address of variable that receives the current value
      *                 of display mode
      * STATUS: Undocumented
      */
{
    DPRINT1("GetConsoleDisplayMode(0x%x) UNIMPLEMENTED!\n", lpdwMode);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * @unimplemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleFontInfo(DWORD Unknown0,
                   DWORD Unknown1,
                   DWORD Unknown2,
                   DWORD Unknown3)
{
    DPRINT1("GetConsoleFontInfo(0x%x, 0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1, Unknown2, Unknown3);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * @unimplemented
 */
COORD
WINAPI
GetConsoleFontSize(HANDLE hConsoleOutput,
                   DWORD nFont)
{
    COORD Empty = {0, 0};
    DPRINT1("GetConsoleFontSize(0x%x, 0x%x) UNIMPLEMENTED!\n", hConsoleOutput, nFont);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return Empty;
}


/*
 * @implemented (Undocumented)
 */
DWORD
WINAPI
GetConsoleHardwareState(HANDLE hConsole,
                        DWORD Flags,
                        PDWORD State)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SETGET_CONSOLE_HW_STATE, CSR_CONSOLE);
    Request.Data.ConsoleHardwareStateRequest.ConsoleHandle = hConsole;
    Request.Data.ConsoleHardwareStateRequest.SetGet = CONSOLE_HARDWARE_STATE_GET;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *State = Request.Data.ConsoleHardwareStateRequest.State;
    return TRUE;
}


/*
 * @implemented (Undocumented)
 */
HANDLE
WINAPI
GetConsoleInputWaitHandle(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(GET_INPUT_WAIT_HANDLE, CSR_CONSOLE);

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return 0;
    }

    return Request.Data.GetConsoleInputWaitHandle.InputWaitHandle;
}


/*
 * @unimplemented
 */
INT
WINAPI
GetCurrentConsoleFont(HANDLE hConsoleOutput,
                      BOOL bMaximumWindow,
                      PCONSOLE_FONT_INFO lpConsoleCurrentFont)
{
    DPRINT1("GetCurrentConsoleFont(0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", hConsoleOutput, bMaximumWindow, lpConsoleCurrentFont);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * @unimplemented (Undocumented)
 */
ULONG
WINAPI
GetNumberOfConsoleFonts(VOID)
{
    DPRINT1("GetNumberOfConsoleFonts() UNIMPLEMENTED!\n");
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 1; /* FIXME: call csrss.exe */
}


/*
 * @unimplemented (Undocumented)
 */
DWORD
WINAPI
InvalidateConsoleDIBits(DWORD Unknown0,
                        DWORD Unknown1)
{
    DPRINT1("InvalidateConsoleDIBits(0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * @unimplemented (Undocumented)
 */
HANDLE
WINAPI
OpenConsoleW(LPCWSTR wsName,
             DWORD dwDesiredAccess,
             BOOL bInheritHandle,
             DWORD dwShareMode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status = STATUS_SUCCESS;

    if (wsName && 0 == _wcsicmp(wsName, L"CONIN$"))
    {
        CsrRequest = MAKE_CSR_API(GET_INPUT_HANDLE, CSR_NATIVE);
    }
    else if (wsName && 0 == _wcsicmp(wsName, L"CONOUT$"))
    {
        CsrRequest = MAKE_CSR_API(GET_OUTPUT_HANDLE, CSR_NATIVE);
    }
    else
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(INVALID_HANDLE_VALUE);
    }

    if (dwDesiredAccess & ~(GENERIC_READ|GENERIC_WRITE))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(INVALID_HANDLE_VALUE);
    }

    if (dwShareMode & ~(FILE_SHARE_READ|FILE_SHARE_WRITE))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(INVALID_HANDLE_VALUE);
    }

    /* Structures for GET_INPUT_HANDLE and GET_OUTPUT_HANDLE requests are identical */
    Request.Data.GetInputHandleRequest.Access = dwDesiredAccess;
    Request.Data.GetInputHandleRequest.Inheritable = bInheritHandle;
    Request.Data.GetInputHandleRequest.ShareMode = dwShareMode;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    return Request.Data.GetInputHandleRequest.Handle;
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleCursor(DWORD Unknown0,
                 DWORD Unknown1)
{
    DPRINT1("SetConsoleCursor(0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL
WINAPI
SetConsoleDisplayMode(HANDLE hOut,
                      DWORD dwNewMode,
                      PCOORD lpdwOldMode)
     /*
      * FUNCTION: Set the console display mode.
      * ARGUMENTS:
      *       hOut - Standard output handle.
      *       dwNewMode - New mode.
      *       lpdwOldMode - Address of a variable that receives the old mode.
      */
{
    DPRINT1("SetConsoleDisplayMode(0x%x, 0x%x, 0x%p) UNIMPLEMENTED!\n", hOut, dwNewMode, lpdwOldMode);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleFont(DWORD Unknown0,
               DWORD Unknown1)
{
    DPRINT1("SetConsoleFont(0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleHardwareState(HANDLE hConsole,
                        DWORD Flags,
                        DWORD State)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SETGET_CONSOLE_HW_STATE, CSR_CONSOLE);
    Request.Data.ConsoleHardwareStateRequest.ConsoleHandle = hConsole;
    Request.Data.ConsoleHardwareStateRequest.SetGet = CONSOLE_HARDWARE_STATE_SET;
    Request.Data.ConsoleHardwareStateRequest.State = State;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleKeyShortcuts(DWORD Unknown0,
                       DWORD Unknown1,
                       DWORD Unknown2,
                       DWORD Unknown3)
{
    DPRINT1("SetConsoleKeyShortcuts(0x%x, 0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1, Unknown2, Unknown3);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleMaximumWindowSize(DWORD Unknown0,
                            DWORD Unknown1)
{
    DPRINT1("SetConsoleMaximumWindowSize(0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleMenuClose(DWORD Unknown0)
{
    DPRINT1("SetConsoleMenuClose(0x%x) UNIMPLEMENTED!\n", Unknown0);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


static BOOL
IntSetConsoleNumberOfCommands(DWORD dwNumCommands,
                              LPCVOID lpExeName,
                              BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(SET_HISTORY_NUMBER_COMMANDS, CSR_CONSOLE);
    NTSTATUS Status;

    if (lpExeName == NULL || !(bUnicode ? *(PWCHAR)lpExeName : *(PCHAR)lpExeName))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer(1, IntStringSize(lpExeName, bUnicode));
    if (!CaptureBuffer)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    IntCaptureMessageString(CaptureBuffer, lpExeName, bUnicode,
                            &Request.Data.SetHistoryNumberCommands.ExeName);
    Request.Data.SetHistoryNumberCommands.NumCommands = dwNumCommands;
    Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
    CsrFreeCaptureBuffer(CaptureBuffer);
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleNumberOfCommandsA(DWORD dwNumCommands,
                            LPCWSTR lpExeName)
{
    return IntSetConsoleNumberOfCommands(dwNumCommands, lpExeName, FALSE);
}

/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
SetConsoleNumberOfCommandsW(DWORD dwNumCommands,
                            LPCSTR lpExeName)
{
    return IntSetConsoleNumberOfCommands(dwNumCommands, lpExeName, TRUE);
}


/*
 * @unimplemented (Undocumented)
 */
BOOL
WINAPI
SetConsolePalette(DWORD Unknown0,
                  DWORD Unknown1,
                  DWORD Unknown2)
{
    DPRINT1("SetConsolePalette(0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1, Unknown2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}

/*
 * @unimplemented (Undocumented)
 */
DWORD
WINAPI
ShowConsoleCursor(DWORD Unknown0,
                  DWORD Unknown1)
{
    DPRINT1("ShowConsoleCursor(0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * FUNCTION: Checks whether the given handle is a valid console handle.
 * ARGUMENTS:
 *      Handle - Handle to be checked
 * RETURNS:
 *      TRUE: Handle is a valid console handle
 *      FALSE: Handle is not a valid console handle.
 * STATUS: Officially undocumented
 *
 * @implemented
 */
BOOL
WINAPI
VerifyConsoleIoHandle(HANDLE Handle)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(VERIFY_HANDLE, CSR_NATIVE);
    Request.Data.VerifyHandleRequest.Handle = Handle;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return (BOOL)NT_SUCCESS(Request.Status);
}


/*
 * @unimplemented
 */
DWORD
WINAPI
WriteConsoleInputVDMA(DWORD Unknown0,
                      DWORD Unknown1,
                      DWORD Unknown2,
                      DWORD Unknown3)
{
    DPRINT1("WriteConsoleInputVDMA(0x%x, 0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1, Unknown2, Unknown3);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * @unimplemented
 */
DWORD
WINAPI
WriteConsoleInputVDMW(DWORD Unknown0,
                      DWORD Unknown1,
                      DWORD Unknown2,
                      DWORD Unknown3)
{
    DPRINT1("WriteConsoleInputVDMW(0x%x, 0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", Unknown0, Unknown1, Unknown2, Unknown3);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return 0;
}


/*
 * @implemented (Undocumented)
 */
BOOL
WINAPI
CloseConsoleHandle(HANDLE Handle)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(CLOSE_HANDLE, CSR_NATIVE);
    Request.Data.CloseHandleRequest.Handle = Handle;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

/*
 * @implemented
 */
HANDLE
WINAPI
GetStdHandle(DWORD nStdHandle)
     /*
      * FUNCTION: Get a handle for the standard input, standard output
      * and a standard error device.
      * ARGUMENTS:
      *       nStdHandle - Specifies the device for which to return the handle.
      * RETURNS: If the function succeeds, the return value is the handle
      * of the specified device. Otherwise the value is INVALID_HANDLE_VALUE.
      */
{
    PRTL_USER_PROCESS_PARAMETERS Ppb;

    Ppb = NtCurrentPeb()->ProcessParameters;
    switch (nStdHandle)
    {
        case STD_INPUT_HANDLE:
            return Ppb->StandardInput;

        case STD_OUTPUT_HANDLE:
            return Ppb->StandardOutput;

        case STD_ERROR_HANDLE:
            return Ppb->StandardError;
    }

    SetLastError (ERROR_INVALID_PARAMETER);
    return INVALID_HANDLE_VALUE;
}


/*
 * @implemented
 */
BOOL
WINAPI
SetStdHandle(DWORD nStdHandle,
             HANDLE hHandle)
     /*
      * FUNCTION: Set the handle for the standard input, standard output or
      * the standard error device.
      * ARGUMENTS:
      *        nStdHandle - Specifies the handle to be set.
      *        hHandle - The handle to set.
      * RETURNS: TRUE if the function succeeds, FALSE otherwise.
      */
{
    PRTL_USER_PROCESS_PARAMETERS Ppb;

    /* no need to check if hHandle == INVALID_HANDLE_VALUE */

    Ppb = NtCurrentPeb()->ProcessParameters;

    switch (nStdHandle)
    {
        case STD_INPUT_HANDLE:
            Ppb->StandardInput = hHandle;
            return TRUE;

        case STD_OUTPUT_HANDLE:
            Ppb->StandardOutput = hHandle;
            return TRUE;

        case STD_ERROR_HANDLE:
            Ppb->StandardError = hHandle;
            return TRUE;
    }

    /* windows for whatever reason sets the last error to ERROR_INVALID_HANDLE here */
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
}


static
BOOL
IntWriteConsole(HANDLE hConsoleOutput,
                PVOID lpBuffer,
                DWORD nNumberOfCharsToWrite,
                LPDWORD lpNumberOfCharsWritten,
                LPVOID lpReserved,
                BOOL bUnicode)
{
    PCSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    USHORT nChars;
    ULONG SizeBytes, CharSize;
    DWORD Written = 0;

    CharSize = (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));
    Request = RtlAllocateHeap(RtlGetProcessHeap(),
                              0,
                              max(sizeof(CSR_API_MESSAGE),
                              CSR_API_MESSAGE_HEADER_SIZE(CSRSS_WRITE_CONSOLE) + min(nNumberOfCharsToWrite,
                              CSRSS_MAX_WRITE_CONSOLE / CharSize) * CharSize));
    if (Request == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(WRITE_CONSOLE, CSR_CONSOLE);

    while (nNumberOfCharsToWrite > 0)
    {
        Request->Data.WriteConsoleRequest.ConsoleHandle = hConsoleOutput;
        Request->Data.WriteConsoleRequest.Unicode = bUnicode;

        nChars = (USHORT)min(nNumberOfCharsToWrite, CSRSS_MAX_WRITE_CONSOLE / CharSize);
        Request->Data.WriteConsoleRequest.NrCharactersToWrite = nChars;

        SizeBytes = nChars * CharSize;

        memcpy(Request->Data.WriteConsoleRequest.Buffer, lpBuffer, SizeBytes);

        Status = CsrClientCallServer(Request,
                                     NULL,
                                     CsrRequest,
                                     max(sizeof(CSR_API_MESSAGE),
                                     CSR_API_MESSAGE_HEADER_SIZE(CSRSS_WRITE_CONSOLE) + SizeBytes));

        if (Status == STATUS_PENDING)
        {
            WaitForSingleObject(Request->Data.WriteConsoleRequest.UnpauseEvent, INFINITE);
            CloseHandle(Request->Data.WriteConsoleRequest.UnpauseEvent);
            continue;
        }
        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request->Status))
        {
            RtlFreeHeap(RtlGetProcessHeap(), 0, Request);
            BaseSetLastNTError(Status);
            return FALSE;
        }

        nNumberOfCharsToWrite -= nChars;
        lpBuffer = (PVOID)((ULONG_PTR)lpBuffer + (ULONG_PTR)SizeBytes);
        Written += Request->Data.WriteConsoleRequest.NrCharactersWritten;
    }

    if (lpNumberOfCharsWritten != NULL)
    {
        *lpNumberOfCharsWritten = Written;
    }
    RtlFreeHeap(RtlGetProcessHeap(), 0, Request);

    return TRUE;
}


/*--------------------------------------------------------------
 *    WriteConsoleA
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleA(HANDLE hConsoleOutput,
              CONST VOID *lpBuffer,
              DWORD nNumberOfCharsToWrite,
              LPDWORD lpNumberOfCharsWritten,
              LPVOID lpReserved)
{
    return IntWriteConsole(hConsoleOutput,
                           (PVOID)lpBuffer,
                           nNumberOfCharsToWrite,
                           lpNumberOfCharsWritten,
                           lpReserved,
                           FALSE);
}


/*--------------------------------------------------------------
 *    WriteConsoleW
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleW(HANDLE hConsoleOutput,
              CONST VOID *lpBuffer,
              DWORD nNumberOfCharsToWrite,
              LPDWORD lpNumberOfCharsWritten,
              LPVOID lpReserved)
{
    return IntWriteConsole(hConsoleOutput,
                           (PVOID)lpBuffer,
                           nNumberOfCharsToWrite,
                           lpNumberOfCharsWritten,
                           lpReserved,
                           TRUE);
}


static
BOOL
IntReadConsole(HANDLE hConsoleInput,
               PVOID lpBuffer,
               DWORD nNumberOfCharsToRead,
               LPDWORD lpNumberOfCharsRead,
               PCONSOLE_READCONSOLE_CONTROL pInputControl,
               BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(READ_CONSOLE, CSR_CONSOLE);
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CharSize = (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    CaptureBuffer = CsrAllocateCaptureBuffer(1, nNumberOfCharsToRead * CharSize);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrAllocateMessagePointer(CaptureBuffer,
                              nNumberOfCharsToRead * CharSize,
                              &Request.Data.ReadConsoleRequest.Buffer);

    Request.Data.ReadConsoleRequest.ConsoleHandle = hConsoleInput;
    Request.Data.ReadConsoleRequest.Unicode = bUnicode;
    Request.Data.ReadConsoleRequest.NrCharactersToRead = (WORD)nNumberOfCharsToRead;
    Request.Data.ReadConsoleRequest.NrCharactersRead = 0;
    Request.Data.ReadConsoleRequest.CtrlWakeupMask = 0;
    if (pInputControl && pInputControl->nLength == sizeof(CONSOLE_READCONSOLE_CONTROL))
    {
        Request.Data.ReadConsoleRequest.NrCharactersRead = pInputControl->nInitialChars;
        memcpy(Request.Data.ReadConsoleRequest.Buffer,
               lpBuffer,
               pInputControl->nInitialChars * sizeof(WCHAR));
        Request.Data.ReadConsoleRequest.CtrlWakeupMask = pInputControl->dwCtrlWakeupMask;
    }

    do
    {
        if (Status == STATUS_PENDING)
        {
            Status = NtWaitForSingleObject(Request.Data.ReadConsoleRequest.EventHandle,
                                           FALSE,
                                           0);
            if (!NT_SUCCESS(Status))
            {
                DPRINT1("Wait for console input failed!\n");
                break;
            }
        }

        Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
        {
            DPRINT1("CSR returned error in ReadConsole\n");
            CsrFreeCaptureBuffer(CaptureBuffer);
            BaseSetLastNTError(Status);
            return FALSE;
        }
    }
    while (Status == STATUS_PENDING);

    memcpy(lpBuffer,
           Request.Data.ReadConsoleRequest.Buffer,
           Request.Data.ReadConsoleRequest.NrCharactersRead * CharSize);

    if (lpNumberOfCharsRead != NULL)
        *lpNumberOfCharsRead = Request.Data.ReadConsoleRequest.NrCharactersRead;

    if (pInputControl && pInputControl->nLength == sizeof(CONSOLE_READCONSOLE_CONTROL))
        pInputControl->dwControlKeyState = Request.Data.ReadConsoleRequest.ControlKeyState;

    CsrFreeCaptureBuffer(CaptureBuffer);

    return TRUE;
}


/*--------------------------------------------------------------
 *    ReadConsoleA
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleA(HANDLE hConsoleInput,
             LPVOID lpBuffer,
             DWORD nNumberOfCharsToRead,
             LPDWORD lpNumberOfCharsRead,
             PCONSOLE_READCONSOLE_CONTROL pInputControl)
{
    return IntReadConsole(hConsoleInput,
                          lpBuffer,
                          nNumberOfCharsToRead,
                          lpNumberOfCharsRead,
                          NULL,
                          FALSE);
}


/*--------------------------------------------------------------
 *    ReadConsoleW
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleW(HANDLE hConsoleInput,
             LPVOID lpBuffer,
             DWORD nNumberOfCharsToRead,
             LPDWORD lpNumberOfCharsRead,
             PCONSOLE_READCONSOLE_CONTROL pInputControl)
{
    return IntReadConsole(hConsoleInput,
                          lpBuffer,
                          nNumberOfCharsToRead,
                          lpNumberOfCharsRead,
                          pInputControl,
                          TRUE);
}


/*--------------------------------------------------------------
 *    AllocConsole
 *
 * @implemented
 */
BOOL
WINAPI
AllocConsole(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    HANDLE hStdError;

    if (NtCurrentPeb()->ProcessParameters->ConsoleHandle)
    {
        DPRINT("AllocConsole: Allocate duplicate console to the same Process\n");
        BaseSetLastNTError (STATUS_OBJECT_NAME_EXISTS);
        return FALSE;
    }

    Request.Data.AllocConsoleRequest.CtrlDispatcher = ConsoleControlDispatcher;
    Request.Data.AllocConsoleRequest.ConsoleNeeded = TRUE;
    Request.Data.AllocConsoleRequest.Visible = TRUE;

    CsrRequest = MAKE_CSR_API(ALLOC_CONSOLE, CSR_CONSOLE);

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    NtCurrentPeb()->ProcessParameters->ConsoleHandle = Request.Data.AllocConsoleRequest.Console;

    SetStdHandle(STD_INPUT_HANDLE, Request.Data.AllocConsoleRequest.InputHandle);
    SetStdHandle(STD_OUTPUT_HANDLE, Request.Data.AllocConsoleRequest.OutputHandle);

    hStdError = DuplicateConsoleHandle(Request.Data.AllocConsoleRequest.OutputHandle,
                                       0,
                                       TRUE,
                                       DUPLICATE_SAME_ACCESS);

    SetStdHandle(STD_ERROR_HANDLE, hStdError);
    return TRUE;
}


/*--------------------------------------------------------------
 *    FreeConsole
 *
 * @implemented
 */
BOOL
WINAPI
FreeConsole(VOID)
{
    // AG: I'm not sure if this is correct (what happens to std handles?)
    // but I just tried to reverse what AllocConsole() does...

    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(FREE_CONSOLE, CSR_CONSOLE);

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    NtCurrentPeb()->ProcessParameters->ConsoleHandle = NULL;
    return TRUE;
}


/*--------------------------------------------------------------
 *    GetConsoleScreenBufferInfo
 *
 * @implemented
 */
BOOL
WINAPI
GetConsoleScreenBufferInfo(HANDLE hConsoleOutput,
                           PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SCREEN_BUFFER_INFO, CSR_CONSOLE);
    Request.Data.ScreenBufferInfoRequest.ConsoleHandle = hConsoleOutput;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    *lpConsoleScreenBufferInfo = Request.Data.ScreenBufferInfoRequest.Info;
    return TRUE;
}


/*--------------------------------------------------------------
 *    SetConsoleCursorPosition
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleCursorPosition(HANDLE hConsoleOutput,
                         COORD dwCursorPosition)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_CURSOR, CSR_CONSOLE);
    Request.Data.SetCursorRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.SetCursorRequest.Position = dwCursorPosition;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if(!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


static
BOOL
IntFillConsoleOutputCharacter(HANDLE hConsoleOutput,
                              PVOID cCharacter,
                              DWORD nLength,
                              COORD dwWriteCoord,
                              LPDWORD lpNumberOfCharsWritten,
                              BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(FILL_OUTPUT, CSR_CONSOLE);
    Request.Data.FillOutputRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.FillOutputRequest.Unicode = bUnicode;

    if(bUnicode)
        Request.Data.FillOutputRequest.Char.UnicodeChar = *((WCHAR*)cCharacter);
    else
        Request.Data.FillOutputRequest.Char.AsciiChar = *((CHAR*)cCharacter);

    Request.Data.FillOutputRequest.Position = dwWriteCoord;
    Request.Data.FillOutputRequest.Length = (WORD)nLength;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if(lpNumberOfCharsWritten != NULL)
    {
        *lpNumberOfCharsWritten = Request.Data.FillOutputRequest.NrCharactersWritten;
    }

    return TRUE;
}

/*--------------------------------------------------------------
 *    FillConsoleOutputCharacterA
 *
 * @implemented
 */
BOOL
WINAPI
FillConsoleOutputCharacterA(HANDLE hConsoleOutput,
                            CHAR cCharacter,
                            DWORD nLength,
                            COORD dwWriteCoord,
                            LPDWORD lpNumberOfCharsWritten)
{
    return IntFillConsoleOutputCharacter(hConsoleOutput,
                                         &cCharacter,
                                         nLength,
                                         dwWriteCoord,
                                         lpNumberOfCharsWritten,
                                         FALSE);
}


/*--------------------------------------------------------------
 *    FillConsoleOutputCharacterW
 *
 * @implemented
 */
BOOL
WINAPI
FillConsoleOutputCharacterW(HANDLE hConsoleOutput,
                            WCHAR cCharacter,
                            DWORD nLength,
                            COORD dwWriteCoord,
                            LPDWORD lpNumberOfCharsWritten)
{
    return IntFillConsoleOutputCharacter(hConsoleOutput,
                                         &cCharacter,
                                         nLength,
                                         dwWriteCoord,
                                         lpNumberOfCharsWritten,
                                         TRUE);
}


static
BOOL
IntPeekConsoleInput(HANDLE hConsoleInput,
                    PINPUT_RECORD lpBuffer,
                    DWORD nLength,
                    LPDWORD lpNumberOfEventsRead,
                    BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    NTSTATUS Status;
    ULONG Size;

    if (lpBuffer == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Size = nLength * sizeof(INPUT_RECORD);

    /* Allocate a Capture Buffer */
    DPRINT("IntPeekConsoleInput: %lx %p\n", Size, lpNumberOfEventsRead);
    CaptureBuffer = CsrAllocateCaptureBuffer(1, Size);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    /* Allocate space in the Buffer */
    CsrCaptureMessageBuffer(CaptureBuffer,
                            NULL,
                            Size,
                            (PVOID*)&Request.Data.PeekConsoleInputRequest.InputRecord);

    /* Set up the data to send to the Console Server */
    CsrRequest = MAKE_CSR_API(PEEK_CONSOLE_INPUT, CSR_CONSOLE);
    Request.Data.PeekConsoleInputRequest.ConsoleHandle = hConsoleInput;
    Request.Data.PeekConsoleInputRequest.Unicode = bUnicode;
    Request.Data.PeekConsoleInputRequest.Length = nLength;

    /* Call the server */
    Status = CsrClientCallServer(&Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    DPRINT("Server returned: %x\n", Request.Status);

    /* Check for success*/
    if (NT_SUCCESS(Request.Status))
    {
        /* Return the number of events read */
        DPRINT("Events read: %lx\n", Request.Data.PeekConsoleInputRequest.Length);
        *lpNumberOfEventsRead = Request.Data.PeekConsoleInputRequest.Length;

        /* Copy into the buffer */
        DPRINT("Copying to buffer\n");
        RtlCopyMemory(lpBuffer,
                      Request.Data.PeekConsoleInputRequest.InputRecord,
                      sizeof(INPUT_RECORD) * *lpNumberOfEventsRead);
    }
    else
    {
        /* Error out */
       *lpNumberOfEventsRead = 0;
       BaseSetLastNTError(Request.Status);
    }

    /* Release the capture buffer */
    CsrFreeCaptureBuffer(CaptureBuffer);

    /* Return TRUE or FALSE */
    return NT_SUCCESS(Request.Status);
}

/*--------------------------------------------------------------
 *     PeekConsoleInputA
 *
 * @implemented
 */
BOOL
WINAPI
PeekConsoleInputA(HANDLE hConsoleInput,
                  PINPUT_RECORD lpBuffer,
                  DWORD nLength,
                  LPDWORD lpNumberOfEventsRead)
{
    return IntPeekConsoleInput(hConsoleInput,
                               lpBuffer,
                               nLength,
                               lpNumberOfEventsRead,
                               FALSE);
}


/*--------------------------------------------------------------
 *     PeekConsoleInputW
 *
 * @implemented
 */
BOOL
WINAPI
PeekConsoleInputW(HANDLE hConsoleInput,
                  PINPUT_RECORD lpBuffer,
                  DWORD nLength,
                  LPDWORD lpNumberOfEventsRead)
{
    return IntPeekConsoleInput(hConsoleInput,
                               lpBuffer, nLength,
                               lpNumberOfEventsRead,
                               TRUE);
}


static
BOOL
IntReadConsoleInput(HANDLE hConsoleInput,
                    PINPUT_RECORD lpBuffer,
                    DWORD nLength,
                    LPDWORD lpNumberOfEventsRead,
                    BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    ULONG Read;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(READ_INPUT, CSR_CONSOLE);
    Read = 0;

    while (nLength > 0)
    {
        Request.Data.ReadInputRequest.ConsoleHandle = hConsoleInput;
        Request.Data.ReadInputRequest.Unicode = bUnicode;

        Status = CsrClientCallServer(&Request,
                                     NULL,
                                     CsrRequest,
                                     sizeof(CSR_API_MESSAGE));
        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
        {
            if (Read == 0)
            {
                /* we couldn't read a single record, fail */
                BaseSetLastNTError(Status);
                return FALSE;
            }
            else
            {
                /* FIXME - fail gracefully in case we already read at least one record? */
                break;
            }
        }
        else if (Status == STATUS_PENDING)
        {
            if (Read == 0)
            {
                Status = NtWaitForSingleObject(Request.Data.ReadInputRequest.Event, FALSE, 0);
                if (!NT_SUCCESS(Status))
                {
                    BaseSetLastNTError(Status);
                    break;
                }
            }
            else
            {
                /* nothing more to read (waiting for more input??), let's just bail */
                break;
            }
        }
        else
        {
            lpBuffer[Read++] = Request.Data.ReadInputRequest.Input;
            nLength--;

            if (!Request.Data.ReadInputRequest.MoreEvents)
            {
                /* nothing more to read, bail */
                break;
            }
        }
    }

    if (lpNumberOfEventsRead != NULL)
    {
        *lpNumberOfEventsRead = Read;
    }

    return (Read > 0);
}


/*--------------------------------------------------------------
 *     ReadConsoleInputA
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleInputA(HANDLE hConsoleInput,
                  PINPUT_RECORD lpBuffer,
                  DWORD nLength,
                  LPDWORD lpNumberOfEventsRead)
{
    return IntReadConsoleInput(hConsoleInput,
                               lpBuffer,
                               nLength,
                               lpNumberOfEventsRead,
                               FALSE);
}


/*--------------------------------------------------------------
 *     ReadConsoleInputW
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleInputW(HANDLE hConsoleInput,
                  PINPUT_RECORD lpBuffer,
                  DWORD nLength,
                  LPDWORD lpNumberOfEventsRead)
{
    return IntReadConsoleInput(hConsoleInput,
                               lpBuffer,
                               nLength,
                               lpNumberOfEventsRead,
                               TRUE);
}


static
BOOL
IntWriteConsoleInput(HANDLE hConsoleInput,
                     PINPUT_RECORD lpBuffer,
                     DWORD nLength,
                     LPDWORD lpNumberOfEventsWritten,
                     BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    NTSTATUS Status;
    DWORD Size;

    if (lpBuffer == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Size = nLength * sizeof(INPUT_RECORD);

    /* Allocate a Capture Buffer */
    DPRINT("IntWriteConsoleInput: %lx %p\n", Size, lpNumberOfEventsWritten);
    CaptureBuffer = CsrAllocateCaptureBuffer(1, Size);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    /* Allocate space in the Buffer */
    CsrCaptureMessageBuffer(CaptureBuffer,
                            lpBuffer,
                            Size,
                            (PVOID*)&Request.Data.WriteConsoleInputRequest.InputRecord);

    /* Set up the data to send to the Console Server */
    CsrRequest = MAKE_CSR_API(WRITE_CONSOLE_INPUT, CSR_CONSOLE);
    Request.Data.WriteConsoleInputRequest.ConsoleHandle = hConsoleInput;
    Request.Data.WriteConsoleInputRequest.Unicode = bUnicode;
    Request.Data.WriteConsoleInputRequest.Length = nLength;

    /* Call the server */
    Status = CsrClientCallServer(&Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    DPRINT("Server returned: %x\n", Request.Status);

    /* Check for success*/
    if (NT_SUCCESS(Request.Status))
    {
        /* Return the number of events read */
        DPRINT("Events read: %lx\n", Request.Data.WriteConsoleInputRequest.Length);
        *lpNumberOfEventsWritten = Request.Data.WriteConsoleInputRequest.Length;
    }
    else
    {
        /* Error out */
        *lpNumberOfEventsWritten = 0;
        BaseSetLastNTError(Request.Status);
    }

    /* Release the capture buffer */
    CsrFreeCaptureBuffer(CaptureBuffer);

    /* Return TRUE or FALSE */
    return NT_SUCCESS(Request.Status);
}


/*--------------------------------------------------------------
 *     WriteConsoleInputA
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleInputA(HANDLE hConsoleInput,
                   CONST INPUT_RECORD *lpBuffer,
                   DWORD nLength,
                   LPDWORD lpNumberOfEventsWritten)
{
    return IntWriteConsoleInput(hConsoleInput,
                                (PINPUT_RECORD)lpBuffer,
                                nLength,
                                lpNumberOfEventsWritten,
                                FALSE);
}


/*--------------------------------------------------------------
 *     WriteConsoleInputW
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleInputW(HANDLE hConsoleInput,
                   CONST INPUT_RECORD *lpBuffer,
                   DWORD nLength,
                   LPDWORD lpNumberOfEventsWritten)
{
    return IntWriteConsoleInput(hConsoleInput,
                                (PINPUT_RECORD)lpBuffer,
                                nLength,
                                lpNumberOfEventsWritten,
                                TRUE);
}


static
BOOL
IntReadConsoleOutput(HANDLE hConsoleOutput,
                     PCHAR_INFO lpBuffer,
                     COORD dwBufferSize,
                     COORD dwBufferCoord,
                     PSMALL_RECT lpReadRegion,
                     BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    NTSTATUS Status;
    DWORD Size, SizeX, SizeY;

    if (lpBuffer == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    Size = dwBufferSize.X * dwBufferSize.Y * sizeof(CHAR_INFO);

    /* Allocate a Capture Buffer */
    DPRINT("IntReadConsoleOutput: %lx %p\n", Size, lpReadRegion);
    CaptureBuffer = CsrAllocateCaptureBuffer(1, Size);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed with size 0x%x!\n", Size);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    /* Allocate space in the Buffer */
    CsrCaptureMessageBuffer(CaptureBuffer,
                            NULL,
                            Size,
                            (PVOID*)&Request.Data.ReadConsoleOutputRequest.CharInfo);

    /* Set up the data to send to the Console Server */
    CsrRequest = MAKE_CSR_API(READ_CONSOLE_OUTPUT, CSR_CONSOLE);
    Request.Data.ReadConsoleOutputRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.ReadConsoleOutputRequest.Unicode = bUnicode;
    Request.Data.ReadConsoleOutputRequest.BufferSize = dwBufferSize;
    Request.Data.ReadConsoleOutputRequest.BufferCoord = dwBufferCoord;
    Request.Data.ReadConsoleOutputRequest.ReadRegion = *lpReadRegion;

    /* Call the server */
    Status = CsrClientCallServer(&Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    DPRINT("Server returned: %x\n", Request.Status);

    /* Check for success*/
    if (NT_SUCCESS(Request.Status))
    {
        /* Copy into the buffer */
        DPRINT("Copying to buffer\n");
        SizeX = Request.Data.ReadConsoleOutputRequest.ReadRegion.Right -
                Request.Data.ReadConsoleOutputRequest.ReadRegion.Left + 1;
        SizeY = Request.Data.ReadConsoleOutputRequest.ReadRegion.Bottom -
                Request.Data.ReadConsoleOutputRequest.ReadRegion.Top + 1;
        RtlCopyMemory(lpBuffer,
                      Request.Data.ReadConsoleOutputRequest.CharInfo,
                      sizeof(CHAR_INFO) * SizeX * SizeY);
    }
    else
    {
        /* Error out */
        BaseSetLastNTError(Request.Status);
    }

    /* Return the read region */
    DPRINT("read region: %lx\n", Request.Data.ReadConsoleOutputRequest.ReadRegion);
    *lpReadRegion = Request.Data.ReadConsoleOutputRequest.ReadRegion;

    /* Release the capture buffer */
    CsrFreeCaptureBuffer(CaptureBuffer);

    /* Return TRUE or FALSE */
    return NT_SUCCESS(Request.Status);
}

/*--------------------------------------------------------------
 *     ReadConsoleOutputA
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleOutputA(HANDLE hConsoleOutput,
                   PCHAR_INFO lpBuffer,
                   COORD dwBufferSize,
                   COORD dwBufferCoord,
                   PSMALL_RECT lpReadRegion)
{
    return IntReadConsoleOutput(hConsoleOutput,
                                lpBuffer,
                                dwBufferSize,
                                dwBufferCoord,
                                lpReadRegion,
                                FALSE);
}


/*--------------------------------------------------------------
 *     ReadConsoleOutputW
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleOutputW(HANDLE hConsoleOutput,
                   PCHAR_INFO lpBuffer,
                   COORD dwBufferSize,
                   COORD dwBufferCoord,
                   PSMALL_RECT lpReadRegion)
{
    return IntReadConsoleOutput(hConsoleOutput,
                                lpBuffer,
                                dwBufferSize,
                                dwBufferCoord,
                                lpReadRegion,
                                TRUE);
}


static
BOOL
IntWriteConsoleOutput(HANDLE hConsoleOutput,
                      CONST CHAR_INFO *lpBuffer,
                      COORD dwBufferSize,
                      COORD dwBufferCoord,
                      PSMALL_RECT lpWriteRegion,
                      BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    NTSTATUS Status;
    ULONG Size;

    Size = dwBufferSize.Y * dwBufferSize.X * sizeof(CHAR_INFO);

    /* Allocate a Capture Buffer */
    DPRINT("IntWriteConsoleOutput: %lx %p\n", Size, lpWriteRegion);
    CaptureBuffer = CsrAllocateCaptureBuffer(1, Size);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    /* Allocate space in the Buffer */
    CsrCaptureMessageBuffer(CaptureBuffer,
                            NULL,
                            Size,
                            (PVOID*)&Request.Data.WriteConsoleOutputRequest.CharInfo);

    /* Copy from the buffer */
    RtlCopyMemory(Request.Data.WriteConsoleOutputRequest.CharInfo, lpBuffer, Size);

    /* Set up the data to send to the Console Server */
    CsrRequest = MAKE_CSR_API(WRITE_CONSOLE_OUTPUT, CSR_CONSOLE);
    Request.Data.WriteConsoleOutputRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.WriteConsoleOutputRequest.Unicode = bUnicode;
    Request.Data.WriteConsoleOutputRequest.BufferSize = dwBufferSize;
    Request.Data.WriteConsoleOutputRequest.BufferCoord = dwBufferCoord;
    Request.Data.WriteConsoleOutputRequest.WriteRegion = *lpWriteRegion;

    /* Call the server */
    Status = CsrClientCallServer(&Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    DPRINT("Server returned: %x\n", Request.Status);

    /* Check for success*/
    if (!NT_SUCCESS(Request.Status))
    {
        /* Error out */
        BaseSetLastNTError(Request.Status);
    }

    /* Return the read region */
    DPRINT("read region: %lx\n", Request.Data.WriteConsoleOutputRequest.WriteRegion);
    *lpWriteRegion = Request.Data.WriteConsoleOutputRequest.WriteRegion;

    /* Release the capture buffer */
    CsrFreeCaptureBuffer(CaptureBuffer);

    /* Return TRUE or FALSE */
    return NT_SUCCESS(Request.Status);
}

/*--------------------------------------------------------------
 *     WriteConsoleOutputA
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleOutputA(HANDLE hConsoleOutput,
                    CONST CHAR_INFO *lpBuffer,
                    COORD dwBufferSize,
                    COORD dwBufferCoord,
                    PSMALL_RECT lpWriteRegion)
{
    return IntWriteConsoleOutput(hConsoleOutput,
                                 lpBuffer,
                                 dwBufferSize,
                                 dwBufferCoord,
                                 lpWriteRegion,
                                 FALSE);
}


/*--------------------------------------------------------------
 *     WriteConsoleOutputW
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleOutputW(HANDLE hConsoleOutput,
                    CONST CHAR_INFO *lpBuffer,
                    COORD dwBufferSize,
                    COORD dwBufferCoord,
                    PSMALL_RECT lpWriteRegion)
{
    return IntWriteConsoleOutput(hConsoleOutput,
                                 lpBuffer,
                                 dwBufferSize,
                                 dwBufferCoord,
                                 lpWriteRegion,
                                 TRUE);
}


static
BOOL
IntReadConsoleOutputCharacter(HANDLE hConsoleOutput,
                              PVOID lpCharacter,
                              DWORD nLength,
                              COORD dwReadCoord,
                              LPDWORD lpNumberOfCharsRead,
                              BOOL bUnicode)
{
    PCSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    ULONG nChars, SizeBytes, CharSize;
    DWORD CharsRead = 0;

    CharSize = (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    nChars = min(nLength, CSRSS_MAX_READ_CONSOLE_OUTPUT_CHAR) / CharSize;
    SizeBytes = nChars * CharSize;

    Request = RtlAllocateHeap(RtlGetProcessHeap(), 0,
                              max(sizeof(CSR_API_MESSAGE),
                              CSR_API_MESSAGE_HEADER_SIZE(CSRSS_READ_CONSOLE_OUTPUT_CHAR)
                                  + min (nChars, CSRSS_MAX_READ_CONSOLE_OUTPUT_CHAR / CharSize) * CharSize));
    if (Request == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(READ_CONSOLE_OUTPUT_CHAR, CSR_CONSOLE);
    Request->Data.ReadConsoleOutputCharRequest.ReadCoord = dwReadCoord;

    while (nLength > 0)
    {
        DWORD BytesRead;

        Request->Data.ReadConsoleOutputCharRequest.ConsoleHandle = hConsoleOutput;
        Request->Data.ReadConsoleOutputCharRequest.Unicode = bUnicode;
        Request->Data.ReadConsoleOutputCharRequest.NumCharsToRead = min(nLength, nChars);
        SizeBytes = Request->Data.ReadConsoleOutputCharRequest.NumCharsToRead * CharSize;

        Status = CsrClientCallServer(Request,
                                     NULL,
                                     CsrRequest,
                                     max(sizeof(CSR_API_MESSAGE),
                                     CSR_API_MESSAGE_HEADER_SIZE(CSRSS_READ_CONSOLE_OUTPUT_CHAR) + SizeBytes));
        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Request->Status))
        {
            RtlFreeHeap(RtlGetProcessHeap(), 0, Request);
            BaseSetLastNTError(Status);
            break;
        }

        BytesRead = Request->Data.ReadConsoleOutputCharRequest.CharsRead * CharSize;
        memcpy(lpCharacter, Request->Data.ReadConsoleOutputCharRequest.String, BytesRead);
        lpCharacter = (PVOID)((ULONG_PTR)lpCharacter + (ULONG_PTR)BytesRead);
        CharsRead += Request->Data.ReadConsoleOutputCharRequest.CharsRead;
        nLength -= Request->Data.ReadConsoleOutputCharRequest.CharsRead;

        Request->Data.ReadConsoleOutputCharRequest.ReadCoord = Request->Data.ReadConsoleOutputCharRequest.EndCoord;
    }

    if (lpNumberOfCharsRead != NULL)
    {
        *lpNumberOfCharsRead = CharsRead;
    }

    RtlFreeHeap(RtlGetProcessHeap(), 0, Request);

    return TRUE;
}


/*--------------------------------------------------------------
 *     ReadConsoleOutputCharacterA
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleOutputCharacterA(HANDLE hConsoleOutput,
                            LPSTR lpCharacter,
                            DWORD nLength,
                            COORD dwReadCoord,
                            LPDWORD lpNumberOfCharsRead)
{
    return IntReadConsoleOutputCharacter(hConsoleOutput,
                                         (PVOID)lpCharacter,
                                         nLength,
                                         dwReadCoord,
                                         lpNumberOfCharsRead,
                                         FALSE);
}


/*--------------------------------------------------------------
 *      ReadConsoleOutputCharacterW
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleOutputCharacterW(HANDLE hConsoleOutput,
                            LPWSTR lpCharacter,
                            DWORD nLength,
                            COORD dwReadCoord,
                            LPDWORD lpNumberOfCharsRead)
{
    return IntReadConsoleOutputCharacter(hConsoleOutput,
                                         (PVOID)lpCharacter,
                                         nLength,
                                         dwReadCoord,
                                         lpNumberOfCharsRead,
                                         TRUE);
}


/*--------------------------------------------------------------
 *     ReadConsoleOutputAttribute
 *
 * @implemented
 */
BOOL
WINAPI
ReadConsoleOutputAttribute(HANDLE hConsoleOutput,
                           LPWORD lpAttribute,
                           DWORD nLength,
                           COORD dwReadCoord,
                           LPDWORD lpNumberOfAttrsRead)
{
    PCSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    DWORD Size;

    if (lpNumberOfAttrsRead != NULL)
        *lpNumberOfAttrsRead = nLength;

    Request = RtlAllocateHeap(RtlGetProcessHeap(),
                              0,
                              max(sizeof(CSR_API_MESSAGE),
                              CSR_API_MESSAGE_HEADER_SIZE(CSRSS_READ_CONSOLE_OUTPUT_ATTRIB)
                                  + min (nLength, CSRSS_MAX_READ_CONSOLE_OUTPUT_ATTRIB / sizeof(WORD)) * sizeof(WORD)));
    if (Request == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(READ_CONSOLE_OUTPUT_ATTRIB, CSR_CONSOLE);

    while (nLength != 0)
    {
        Request->Data.ReadConsoleOutputAttribRequest.ConsoleHandle = hConsoleOutput;
        Request->Data.ReadConsoleOutputAttribRequest.ReadCoord = dwReadCoord;

        if (nLength > CSRSS_MAX_READ_CONSOLE_OUTPUT_ATTRIB / sizeof(WORD))
            Size = CSRSS_MAX_READ_CONSOLE_OUTPUT_ATTRIB / sizeof(WCHAR);
        else
            Size = nLength;

        Request->Data.ReadConsoleOutputAttribRequest.NumAttrsToRead = Size;

        Status = CsrClientCallServer(Request,
                                     NULL,
                                     CsrRequest,
                                     max(sizeof(CSR_API_MESSAGE),
                                     CSR_API_MESSAGE_HEADER_SIZE(CSRSS_READ_CONSOLE_OUTPUT_ATTRIB) + Size * sizeof(WORD)));
        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Request->Status))
        {
            RtlFreeHeap(RtlGetProcessHeap(), 0, Request);
            BaseSetLastNTError(Status);
            return FALSE;
        }

        memcpy(lpAttribute, Request->Data.ReadConsoleOutputAttribRequest.Attribute, Size * sizeof(WORD));
        lpAttribute += Size;
        nLength -= Size;
        Request->Data.ReadConsoleOutputAttribRequest.ReadCoord = Request->Data.ReadConsoleOutputAttribRequest.EndCoord;
    }

    RtlFreeHeap(RtlGetProcessHeap(), 0, Request);

    return TRUE;
}


static
BOOL
IntWriteConsoleOutputCharacter(HANDLE hConsoleOutput,
                               PVOID lpCharacter,
                               DWORD nLength,
                               COORD dwWriteCoord,
                               LPDWORD lpNumberOfCharsWritten,
                               BOOL bUnicode)
{
    PCSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    ULONG SizeBytes, CharSize, nChars;
    DWORD Written = 0;

    CharSize = (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    nChars = min(nLength, CSRSS_MAX_WRITE_CONSOLE_OUTPUT_CHAR / CharSize);
    SizeBytes = nChars * CharSize;

    Request = RtlAllocateHeap(RtlGetProcessHeap(), 0,
                              max(sizeof(CSR_API_MESSAGE),
                              CSR_API_MESSAGE_HEADER_SIZE(CSRSS_WRITE_CONSOLE_OUTPUT_CHAR)
                                + min (nChars, CSRSS_MAX_WRITE_CONSOLE_OUTPUT_CHAR / CharSize) * CharSize));
    if (Request == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(WRITE_CONSOLE_OUTPUT_CHAR, CSR_CONSOLE);
    Request->Data.WriteConsoleOutputCharRequest.Coord = dwWriteCoord;

    while (nLength > 0)
    {
        DWORD BytesWrite;

        Request->Data.WriteConsoleOutputCharRequest.ConsoleHandle = hConsoleOutput;
        Request->Data.WriteConsoleOutputCharRequest.Unicode = bUnicode;
        Request->Data.WriteConsoleOutputCharRequest.Length = (WORD)min(nLength, nChars);
        BytesWrite = Request->Data.WriteConsoleOutputCharRequest.Length * CharSize;

        memcpy(Request->Data.WriteConsoleOutputCharRequest.String, lpCharacter, BytesWrite);

        Status = CsrClientCallServer(Request,
                                     NULL,
                                     CsrRequest,
                                     max(sizeof(CSR_API_MESSAGE),
                                     CSR_API_MESSAGE_HEADER_SIZE(CSRSS_WRITE_CONSOLE_OUTPUT_CHAR) + BytesWrite));

        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request->Status))
        {
            RtlFreeHeap(RtlGetProcessHeap(), 0, Request);
            BaseSetLastNTError(Status);
            return FALSE;
        }

        nLength -= Request->Data.WriteConsoleOutputCharRequest.NrCharactersWritten;
        lpCharacter = (PVOID)((ULONG_PTR)lpCharacter + (ULONG_PTR)(Request->Data.WriteConsoleOutputCharRequest.NrCharactersWritten * CharSize));
        Written += Request->Data.WriteConsoleOutputCharRequest.NrCharactersWritten;

        Request->Data.WriteConsoleOutputCharRequest.Coord = Request->Data.WriteConsoleOutputCharRequest.EndCoord;
    }

    if (lpNumberOfCharsWritten != NULL)
    {
        *lpNumberOfCharsWritten = Written;
    }

    RtlFreeHeap(RtlGetProcessHeap(), 0, Request);

    return TRUE;
}


/*--------------------------------------------------------------
 *     WriteConsoleOutputCharacterA
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleOutputCharacterA(HANDLE hConsoleOutput,
                             LPCSTR lpCharacter,
                             DWORD nLength,
                             COORD dwWriteCoord,
                             LPDWORD lpNumberOfCharsWritten)
{
    return IntWriteConsoleOutputCharacter(hConsoleOutput,
                                          (PVOID)lpCharacter,
                                          nLength,
                                          dwWriteCoord,
                                          lpNumberOfCharsWritten,
                                          FALSE);
}


/*--------------------------------------------------------------
 *     WriteConsoleOutputCharacterW
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleOutputCharacterW(HANDLE hConsoleOutput,
                             LPCWSTR lpCharacter,
                             DWORD nLength,
                             COORD dwWriteCoord,
                             LPDWORD lpNumberOfCharsWritten)
{
    return IntWriteConsoleOutputCharacter(hConsoleOutput,
                                          (PVOID)lpCharacter,
                                          nLength,
                                          dwWriteCoord,
                                          lpNumberOfCharsWritten,
                                          TRUE);
}


/*--------------------------------------------------------------
 *     WriteConsoleOutputAttribute
 *
 * @implemented
 */
BOOL
WINAPI
WriteConsoleOutputAttribute(HANDLE hConsoleOutput,
                            CONST WORD *lpAttribute,
                            DWORD nLength,
                            COORD dwWriteCoord,
                            LPDWORD lpNumberOfAttrsWritten)
{
    PCSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    WORD Size;

    Request = RtlAllocateHeap(RtlGetProcessHeap(),
                              0,
                              max(sizeof(CSR_API_MESSAGE),
                              CSR_API_MESSAGE_HEADER_SIZE(CSRSS_WRITE_CONSOLE_OUTPUT_ATTRIB)
                                + min(nLength, CSRSS_MAX_WRITE_CONSOLE_OUTPUT_ATTRIB / sizeof(WORD)) * sizeof(WORD)));
    if (Request == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(WRITE_CONSOLE_OUTPUT_ATTRIB, CSR_CONSOLE);
    Request->Data.WriteConsoleOutputAttribRequest.Coord = dwWriteCoord;

    if (lpNumberOfAttrsWritten)
        *lpNumberOfAttrsWritten = nLength;
    while (nLength)
    {
        Size = (WORD)min(nLength, CSRSS_MAX_WRITE_CONSOLE_OUTPUT_ATTRIB / sizeof(WORD));
        Request->Data.WriteConsoleOutputAttribRequest.ConsoleHandle = hConsoleOutput;
        Request->Data.WriteConsoleOutputAttribRequest.Length = Size;
        memcpy(Request->Data.WriteConsoleOutputAttribRequest.Attribute, lpAttribute, Size * sizeof(WORD));

        Status = CsrClientCallServer(Request,
                                     NULL,
                                     CsrRequest,
                                     max(sizeof(CSR_API_MESSAGE),
                                     CSR_API_MESSAGE_HEADER_SIZE(CSRSS_WRITE_CONSOLE_OUTPUT_ATTRIB) + Size * sizeof(WORD)));

        if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request->Status))
        {
            RtlFreeHeap(RtlGetProcessHeap(), 0, Request);
            BaseSetLastNTError (Status);
            return FALSE;
        }
        nLength -= Size;
        lpAttribute += Size;
        Request->Data.WriteConsoleOutputAttribRequest.Coord = Request->Data.WriteConsoleOutputAttribRequest.EndCoord;
    }

    RtlFreeHeap(RtlGetProcessHeap(), 0, Request);

    return TRUE;
}


/*--------------------------------------------------------------
 *     FillConsoleOutputAttribute
 *
 * @implemented
 */
BOOL
WINAPI
FillConsoleOutputAttribute(HANDLE hConsoleOutput,
                           WORD wAttribute,
                           DWORD nLength,
                           COORD dwWriteCoord,
                           LPDWORD lpNumberOfAttrsWritten)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(FILL_OUTPUT_ATTRIB, CSR_CONSOLE);
    Request.Data.FillOutputAttribRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.FillOutputAttribRequest.Attribute = (CHAR)wAttribute;
    Request.Data.FillOutputAttribRequest.Coord = dwWriteCoord;
    Request.Data.FillOutputAttribRequest.Length = (WORD)nLength;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError ( Status );
        return FALSE;
    }

    if (lpNumberOfAttrsWritten)
        *lpNumberOfAttrsWritten = nLength;

    return TRUE;
}


/*--------------------------------------------------------------
 *     GetConsoleMode
 *
 * @implemented
 */
BOOL
WINAPI
GetConsoleMode(HANDLE hConsoleHandle,
               LPDWORD lpMode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_MODE, CSR_CONSOLE);
    Request.Data.GetConsoleModeRequest.ConsoleHandle = hConsoleHandle;

    Status = CsrClientCallServer( &Request, NULL, CsrRequest, sizeof( CSR_API_MESSAGE ) );
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError ( Status );
        return FALSE;
    }
    *lpMode = Request.Data.GetConsoleModeRequest.ConsoleMode;

    return TRUE;
}


/*--------------------------------------------------------------
 *     GetNumberOfConsoleInputEvents
 *
 * @implemented
 */
BOOL
WINAPI
GetNumberOfConsoleInputEvents(HANDLE hConsoleInput,
                              LPDWORD lpNumberOfEvents)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    if (lpNumberOfEvents == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(GET_NUM_INPUT_EVENTS, CSR_CONSOLE);
    Request.Data.GetNumInputEventsRequest.ConsoleHandle = hConsoleInput;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if(!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *lpNumberOfEvents = Request.Data.GetNumInputEventsRequest.NumInputEvents;

    return TRUE;
}


/*--------------------------------------------------------------
 *     GetLargestConsoleWindowSize
 *
 * @unimplemented
 */
COORD
WINAPI
GetLargestConsoleWindowSize(HANDLE hConsoleOutput)
{
    COORD Coord = {80,25};
    DPRINT1("GetLargestConsoleWindowSize(0x%x) UNIMPLEMENTED!\n", hConsoleOutput);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return Coord;
}


/*--------------------------------------------------------------
 *    GetConsoleCursorInfo
 *
 * @implemented
 */
BOOL
WINAPI
GetConsoleCursorInfo(HANDLE hConsoleOutput,
                     PCONSOLE_CURSOR_INFO lpConsoleCursorInfo)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    if (!lpConsoleCursorInfo)
    {
        if (!hConsoleOutput)
            SetLastError(ERROR_INVALID_HANDLE);
        else
            SetLastError(ERROR_INVALID_ACCESS);

        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(GET_CURSOR_INFO, CSR_CONSOLE);
    Request.Data.GetCursorInfoRequest.ConsoleHandle = hConsoleOutput;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    *lpConsoleCursorInfo = Request.Data.GetCursorInfoRequest.Info;

    return TRUE;
}


/*--------------------------------------------------------------
 *     GetNumberOfConsoleMouseButtons
 *
 * @unimplemented
 */
BOOL
WINAPI
GetNumberOfConsoleMouseButtons(LPDWORD lpNumberOfMouseButtons)
{
    DPRINT1("GetNumberOfConsoleMouseButtons(0x%x) UNIMPLEMENTED!\n", lpNumberOfMouseButtons);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*--------------------------------------------------------------
 *     SetConsoleMode
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleMode(HANDLE hConsoleHandle,
               DWORD dwMode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_CONSOLE_MODE, CSR_CONSOLE);
    Request.Data.SetConsoleModeRequest.ConsoleHandle = hConsoleHandle;
    Request.Data.SetConsoleModeRequest.Mode = dwMode;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if(!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError ( Status );
        return FALSE;
    }

    return TRUE;
}


/*--------------------------------------------------------------
 *     SetConsoleActiveScreenBuffer
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleActiveScreenBuffer(HANDLE hConsoleOutput)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_SCREEN_BUFFER, CSR_CONSOLE);
    Request.Data.SetScreenBufferRequest.OutputHandle = hConsoleOutput;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*--------------------------------------------------------------
 *     FlushConsoleInputBuffer
 *
 * @implemented
 */
BOOL
WINAPI
FlushConsoleInputBuffer(HANDLE hConsoleInput)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(FLUSH_INPUT_BUFFER, CSR_CONSOLE);
    Request.Data.FlushInputBufferRequest.ConsoleInput = hConsoleInput;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*--------------------------------------------------------------
 *     SetConsoleScreenBufferSize
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleScreenBufferSize(HANDLE hConsoleOutput,
                           COORD dwSize)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_SCREEN_BUFFER_SIZE, CSR_CONSOLE);
    Request.Data.SetScreenBufferSize.OutputHandle = hConsoleOutput;
    Request.Data.SetScreenBufferSize.Size = dwSize;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------
 *     SetConsoleCursorInfo
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleCursorInfo(HANDLE hConsoleOutput,
                     CONST CONSOLE_CURSOR_INFO *lpConsoleCursorInfo)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_CURSOR_INFO, CSR_CONSOLE);
    Request.Data.SetCursorInfoRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.SetCursorInfoRequest.Info = *lpConsoleCursorInfo;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if(!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


static
BOOL
IntScrollConsoleScreenBuffer(HANDLE hConsoleOutput,
                             const SMALL_RECT *lpScrollRectangle,
                             const SMALL_RECT *lpClipRectangle,
                             COORD dwDestinationOrigin,
                             const CHAR_INFO *lpFill,
                             BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SCROLL_CONSOLE_SCREEN_BUFFER, CSR_CONSOLE);
    Request.Data.ScrollConsoleScreenBufferRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.ScrollConsoleScreenBufferRequest.Unicode = bUnicode;
    Request.Data.ScrollConsoleScreenBufferRequest.ScrollRectangle = *lpScrollRectangle;

    if (lpClipRectangle != NULL)
    {
        Request.Data.ScrollConsoleScreenBufferRequest.UseClipRectangle = TRUE;
        Request.Data.ScrollConsoleScreenBufferRequest.ClipRectangle = *lpClipRectangle;
    }
    else
    {
        Request.Data.ScrollConsoleScreenBufferRequest.UseClipRectangle = FALSE;
    }

    Request.Data.ScrollConsoleScreenBufferRequest.DestinationOrigin = dwDestinationOrigin;
    Request.Data.ScrollConsoleScreenBufferRequest.Fill = *lpFill;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));

    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*--------------------------------------------------------------
 *    ScrollConsoleScreenBufferA
 *
 * @implemented
 */
BOOL
WINAPI
ScrollConsoleScreenBufferA(HANDLE hConsoleOutput,
                           CONST SMALL_RECT *lpScrollRectangle,
                           CONST SMALL_RECT *lpClipRectangle,
                           COORD dwDestinationOrigin,
                           CONST CHAR_INFO *lpFill)
{
    return IntScrollConsoleScreenBuffer(hConsoleOutput,
                                        (PSMALL_RECT)lpScrollRectangle,
                                        (PSMALL_RECT)lpClipRectangle,
                                        dwDestinationOrigin,
                                        (PCHAR_INFO)lpFill,
                                        FALSE);
}


/*--------------------------------------------------------------
 *     ScrollConsoleScreenBufferW
 *
 * @implemented
 */
BOOL
WINAPI
ScrollConsoleScreenBufferW(HANDLE hConsoleOutput,
                           CONST SMALL_RECT *lpScrollRectangle,
                           CONST SMALL_RECT *lpClipRectangle,
                           COORD dwDestinationOrigin,
                           CONST CHAR_INFO *lpFill)
{
    return IntScrollConsoleScreenBuffer(hConsoleOutput,
                                        lpScrollRectangle,
                                        lpClipRectangle,
                                        dwDestinationOrigin,
                                        lpFill,
                                        TRUE);
}


/*--------------------------------------------------------------
 *     SetConsoleWindowInfo
 *
 * @unimplemented
 */
BOOL
WINAPI
SetConsoleWindowInfo(HANDLE hConsoleOutput,
                     BOOL bAbsolute,
                     CONST SMALL_RECT *lpConsoleWindow)
{
    DPRINT1("SetConsoleWindowInfo(0x%x, 0x%x, 0x%x) UNIMPLEMENTED!\n", hConsoleOutput, bAbsolute, lpConsoleWindow);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*--------------------------------------------------------------
 *      SetConsoleTextAttribute
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleTextAttribute(HANDLE hConsoleOutput,
                        WORD wAttributes)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_ATTRIB, CSR_CONSOLE);
    Request.Data.SetAttribRequest.ConsoleHandle = hConsoleOutput;
    Request.Data.SetAttribRequest.Attrib = wAttributes;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


static
BOOL
AddConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine)
{
    PHANDLER_ROUTINE* NewCtrlHandlers = NULL;

    if (HandlerRoutine == NULL)
    {
        NtCurrentPeb()->ProcessParameters->ConsoleFlags = TRUE;
        return TRUE;
    }
    
    if (NrCtrlHandlers == NrAllocatedHandlers)
    {
        NewCtrlHandlers = RtlAllocateHeap(RtlGetProcessHeap(),
                                          0,
                                          (NrCtrlHandlers + 4) * sizeof(PHANDLER_ROUTINE));
        if (NewCtrlHandlers == NULL)   
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
        
        memmove(NewCtrlHandlers, CtrlHandlers, sizeof(PHANDLER_ROUTINE) * NrCtrlHandlers);
        
        if (NrAllocatedHandlers > 1) RtlFreeHeap(RtlGetProcessHeap(), 0, CtrlHandlers);
        
        CtrlHandlers = NewCtrlHandlers;
        NrAllocatedHandlers += 4;
    }
    
    ASSERT(NrCtrlHandlers < NrAllocatedHandlers);

    CtrlHandlers[NrCtrlHandlers++] = HandlerRoutine;
    return TRUE;
}


static
BOOL
RemoveConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine)
{
    ULONG i;

    if (HandlerRoutine == NULL)
    {
        NtCurrentPeb()->ProcessParameters->ConsoleFlags = FALSE;
        return TRUE;
    }

    for (i = 0; i < NrCtrlHandlers; i++)
    {
        if (CtrlHandlers[i] == HandlerRoutine)
        {
            if (i < (NrCtrlHandlers - 1))
            {
                memmove(&CtrlHandlers[i],
                        &CtrlHandlers[i+1],
                        (NrCtrlHandlers - i + 1) * sizeof(PHANDLER_ROUTINE));
            }

            NrCtrlHandlers--;
            return TRUE;
        }
    }

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}


/*
 * @implemented
 */
BOOL
WINAPI
SetConsoleCtrlHandler(PHANDLER_ROUTINE HandlerRoutine,
                      BOOL Add)
{
    BOOL Ret;

    RtlEnterCriticalSection(&BaseDllDirectoryLock);
    if (Add)
    {
        Ret = AddConsoleCtrlHandler(HandlerRoutine);
    }
    else
    {
        Ret = RemoveConsoleCtrlHandler(HandlerRoutine);
    }

    RtlLeaveCriticalSection(&BaseDllDirectoryLock);
    return(Ret);
}


/*--------------------------------------------------------------
 *     GenerateConsoleCtrlEvent
 *
 * @implemented
 */
BOOL
WINAPI
GenerateConsoleCtrlEvent(DWORD dwCtrlEvent,
                         DWORD dwProcessGroupId)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    if (dwCtrlEvent != CTRL_C_EVENT && dwCtrlEvent != CTRL_BREAK_EVENT)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(GENERATE_CTRL_EVENT, CSR_CONSOLE);
    Request.Data.GenerateCtrlEvent.Event = dwCtrlEvent;
    Request.Data.GenerateCtrlEvent.ProcessGroup = dwProcessGroupId;

    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if(!NT_SUCCESS(Status) || !(NT_SUCCESS(Status = Request.Status)))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


static DWORD
IntGetConsoleTitle(LPVOID lpConsoleTitle, DWORD nSize, BOOL bUnicode)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(GET_TITLE, CSR_CONSOLE);
    NTSTATUS Status;

    if (nSize == 0)
        return 0;

    Request.Data.GetTitleRequest.Length = nSize * (bUnicode ? 1 : sizeof(WCHAR));
    CaptureBuffer = CsrAllocateCaptureBuffer(1, Request.Data.GetTitleRequest.Length);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    CsrAllocateMessagePointer(CaptureBuffer,
                              Request.Data.GetTitleRequest.Length,
                              (PVOID*)&Request.Data.GetTitleRequest.Title);

    Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !(NT_SUCCESS(Status = Request.Status)))
    {
        CsrFreeCaptureBuffer(CaptureBuffer);
        BaseSetLastNTError(Status);
        return 0;
    }

    if (bUnicode)
    {
        if (nSize >= sizeof(WCHAR))
            wcscpy((LPWSTR)lpConsoleTitle, Request.Data.GetTitleRequest.Title);
    }
    else
    {
        if (nSize < Request.Data.GetTitleRequest.Length / sizeof(WCHAR) ||
            !WideCharToMultiByte(CP_ACP, // ANSI code page
                                 0, // performance and mapping flags
                                 Request.Data.GetTitleRequest.Title, // address of wide-character string
                                 -1, // number of characters in string
                                 (LPSTR)lpConsoleTitle, // address of buffer for new string
                                 nSize, // size of buffer
                                 NULL, // FAST
                                 NULL))
        {
            /* Yes, if the buffer isn't big enough, it returns 0... Bad API */
            *(LPSTR)lpConsoleTitle = '\0';
            Request.Data.GetTitleRequest.Length = 0;
        }
    }
    CsrFreeCaptureBuffer(CaptureBuffer);

    return Request.Data.GetTitleRequest.Length / sizeof(WCHAR);
}

/*--------------------------------------------------------------
 *    GetConsoleTitleW
 *
 * @implemented
 */
DWORD
WINAPI
GetConsoleTitleW(LPWSTR lpConsoleTitle,
                 DWORD nSize)
{
    return IntGetConsoleTitle(lpConsoleTitle, nSize, TRUE);
}

/*--------------------------------------------------------------
 *     GetConsoleTitleA
 *
 *     19990306 EA
 *
 * @implemented
 */
DWORD
WINAPI
GetConsoleTitleA(LPSTR lpConsoleTitle,
                 DWORD nSize)
{
    return IntGetConsoleTitle(lpConsoleTitle, nSize, FALSE);
}


/*--------------------------------------------------------------
 *    SetConsoleTitleW
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleTitleW(LPCWSTR lpConsoleTitle)
{
    CSR_API_MESSAGE Request;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    ULONG CsrRequest = MAKE_CSR_API(SET_TITLE, CSR_CONSOLE);
    NTSTATUS Status;

    Request.Data.SetTitleRequest.Length = wcslen(lpConsoleTitle) * sizeof(WCHAR);

    CaptureBuffer = CsrAllocateCaptureBuffer(1, Request.Data.SetTitleRequest.Length);
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrCaptureMessageBuffer(CaptureBuffer,
                            (PVOID)lpConsoleTitle,
                            Request.Data.SetTitleRequest.Length,
                            (PVOID*)&Request.Data.SetTitleRequest.Title);

    Status = CsrClientCallServer(&Request, CaptureBuffer, CsrRequest, sizeof(CSR_API_MESSAGE));
    CsrFreeCaptureBuffer(CaptureBuffer);
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*--------------------------------------------------------------
 *    SetConsoleTitleA
 *
 *     19990204 EA    Added
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleTitleA(LPCSTR lpConsoleTitle)
{
    ULONG Length = strlen(lpConsoleTitle) + 1;
    LPWSTR WideTitle = HeapAlloc(GetProcessHeap(), 0, Length * sizeof(WCHAR));
    BOOL Ret;
    if (!WideTitle)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    MultiByteToWideChar(CP_ACP, 0, lpConsoleTitle, -1, WideTitle, Length);
    Ret = SetConsoleTitleW(WideTitle);
    HeapFree(GetProcessHeap(), 0, WideTitle);
    return Ret;
}


/*--------------------------------------------------------------
 *    CreateConsoleScreenBuffer
 *
 * @implemented
 */
HANDLE
WINAPI
CreateConsoleScreenBuffer(DWORD dwDesiredAccess,
                          DWORD dwShareMode,
                          CONST SECURITY_ATTRIBUTES *lpSecurityAttributes,
                          DWORD dwFlags,
                          LPVOID lpScreenBufferData)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    if (dwDesiredAccess & ~(GENERIC_READ | GENERIC_WRITE)
        || dwShareMode & ~(FILE_SHARE_READ | FILE_SHARE_WRITE)
        || dwFlags != CONSOLE_TEXTMODE_BUFFER)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
    }

    Request.Data.CreateScreenBufferRequest.Access = dwDesiredAccess;
    Request.Data.CreateScreenBufferRequest.ShareMode = dwShareMode;
    Request.Data.CreateScreenBufferRequest.Inheritable =
        lpSecurityAttributes ? lpSecurityAttributes->bInheritHandle : FALSE;

    CsrRequest = MAKE_CSR_API(CREATE_SCREEN_BUFFER, CSR_CONSOLE);
    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }
    return Request.Data.CreateScreenBufferRequest.OutputHandle;
}


/*--------------------------------------------------------------
 *    GetConsoleCP
 *
 * @implemented
 */
UINT
WINAPI
GetConsoleCP(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_CP, CSR_CONSOLE);
    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError (Status);
        return 0;
    }

    return Request.Data.GetConsoleCodePage.CodePage;
}


/*--------------------------------------------------------------
 *    SetConsoleCP
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleCP(UINT wCodePageID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_CONSOLE_CP, CSR_CONSOLE);
    Request.Data.SetConsoleCodePage.CodePage = wCodePageID;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
    }

    return NT_SUCCESS(Status);
}


/*--------------------------------------------------------------
 *    GetConsoleOutputCP
 *
 * @implemented
 */
UINT
WINAPI
GetConsoleOutputCP(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_OUTPUT_CP, CSR_CONSOLE);
    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError (Status);
        return 0;
    }

    return Request.Data.GetConsoleOutputCodePage.CodePage;
}


/*--------------------------------------------------------------
 *    SetConsoleOutputCP
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleOutputCP(UINT wCodePageID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_CONSOLE_OUTPUT_CP, CSR_CONSOLE);
    Request.Data.SetConsoleOutputCodePage.CodePage = wCodePageID;
    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
    }

    return NT_SUCCESS(Status);
}


/*--------------------------------------------------------------
 *     GetConsoleProcessList
 *
 * @implemented
 */
DWORD
WINAPI
GetConsoleProcessList(LPDWORD lpdwProcessList,
                      DWORD dwProcessCount)
{
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    ULONG nProcesses;
    NTSTATUS Status;

    if (lpdwProcessList == NULL || dwProcessCount == 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    CaptureBuffer = CsrAllocateCaptureBuffer(1, dwProcessCount * sizeof(DWORD));
    if (CaptureBuffer == NULL)
    {
        DPRINT1("CsrAllocateCaptureBuffer failed!\n");
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    CsrRequest = MAKE_CSR_API(GET_PROCESS_LIST, CSR_CONSOLE);
    Request.Data.GetProcessListRequest.nMaxIds = dwProcessCount;
    CsrAllocateMessagePointer(CaptureBuffer,
                              dwProcessCount * sizeof(DWORD),
                              (PVOID*)&Request.Data.GetProcessListRequest.ProcessId);

    Status = CsrClientCallServer(&Request,
                                 CaptureBuffer,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError (Status);
        nProcesses = 0;
    }
    else
    {
        nProcesses = Request.Data.GetProcessListRequest.nProcessIdsTotal;
        if (dwProcessCount >= nProcesses)
        {
            memcpy(lpdwProcessList, Request.Data.GetProcessListRequest.ProcessId, nProcesses * sizeof(DWORD));
        }
    }

    CsrFreeCaptureBuffer(CaptureBuffer);
    return nProcesses;
}



/*--------------------------------------------------------------
 *     GetConsoleSelectionInfo
 *
 * @implemented
 */
BOOL
WINAPI
GetConsoleSelectionInfo(PCONSOLE_SELECTION_INFO lpConsoleSelectionInfo)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest = MAKE_CSR_API(GET_CONSOLE_SELECTION_INFO, CSR_CONSOLE);
    NTSTATUS Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    *lpConsoleSelectionInfo = Request.Data.GetConsoleSelectionInfo.Info;
    return TRUE;
}



/*--------------------------------------------------------------
 *     AttachConsole
 *
 * @unimplemented
 */
BOOL
WINAPI
AttachConsole(DWORD dwProcessId)
{
    DPRINT1("AttachConsole(0x%x) UNIMPLEMENTED!\n", dwProcessId);
    return TRUE;
}

/*--------------------------------------------------------------
 *     GetConsoleWindow
 *
 * @implemented
 */
HWND
WINAPI
GetConsoleWindow(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(GET_CONSOLE_WINDOW, CSR_CONSOLE);
    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status ) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return (HWND) NULL;
    }

    return Request.Data.GetConsoleWindowRequest.WindowHandle;
}


/*--------------------------------------------------------------
 *     SetConsoleIcon
 *
 * @implemented
 */
BOOL
WINAPI
SetConsoleIcon(HICON hicon)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;

    CsrRequest = MAKE_CSR_API(SET_CONSOLE_ICON, CSR_CONSOLE);
    Request.Data.SetConsoleIconRequest.WindowIcon = hicon;

    Status = CsrClientCallServer(&Request, NULL, CsrRequest, sizeof(CSR_API_MESSAGE));
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return NT_SUCCESS(Status);
}


/******************************************************************************
 * \name SetConsoleInputExeNameW
 * \brief Sets the console input file name from a unicode string.
 * \param lpInputExeName Pointer to a unicode string with the name.
 * \return TRUE if successful, FALSE if unsuccsedful.
 * \remarks If lpInputExeName is 0 or the string length is 0 or greater than 255,
 *          the function fails and sets last error to ERROR_INVALID_PARAMETER.
 */
BOOL
WINAPI
SetConsoleInputExeNameW(LPCWSTR lpInputExeName)
{
    int lenName;

    if (!lpInputExeName
        || (lenName = lstrlenW(lpInputExeName)) == 0
        || lenName > INPUTEXENAME_BUFLEN - 1)
    {
        /* Fail if string is empty or too long */
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    RtlEnterCriticalSection(&ConsoleLock);
    _SEH2_TRY
    {
        RtlCopyMemory(InputExeName, lpInputExeName, lenName * sizeof(WCHAR));
        InputExeName[lenName] = L'\0';
    }
    _SEH2_FINALLY
    {
        RtlLeaveCriticalSection(&ConsoleLock);
    }
    _SEH2_END;

    return TRUE;
}


/******************************************************************************
 * \name SetConsoleInputExeNameA
 * \brief Sets the console input file name from an ansi string.
 * \param lpInputExeName Pointer to an ansi string with the name.
 * \return TRUE if successful, FALSE if unsuccsedful.
 * \remarks If lpInputExeName is 0 or the string length is 0 or greater than 255,
 *          the function fails and sets last error to ERROR_INVALID_PARAMETER.
 */
BOOL
WINAPI
SetConsoleInputExeNameA(LPCSTR lpInputExeName)
{
    WCHAR Buffer[INPUTEXENAME_BUFLEN];
    ANSI_STRING InputExeNameA;
    UNICODE_STRING InputExeNameU;
    NTSTATUS Status;
    BOOL Ret;

    RtlInitAnsiString(&InputExeNameA, lpInputExeName);

    if(InputExeNameA.Length == 0 || 
       InputExeNameA.Length > INPUTEXENAME_BUFLEN - 1)
    {
        /* Fail if string is empty or too long */
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    InputExeNameU.Buffer = Buffer;
    InputExeNameU.MaximumLength = sizeof(Buffer);
    InputExeNameU.Length = 0;
    Status = RtlAnsiStringToUnicodeString(&InputExeNameU, &InputExeNameA, FALSE);
    if(NT_SUCCESS(Status))
    {
        Ret = SetConsoleInputExeNameW(InputExeNameU.Buffer);
    }
    else
    {
        BaseSetLastNTError(Status);
        Ret = FALSE;
    }

    return Ret;
}


/******************************************************************************
 * \name GetConsoleInputExeNameW
 * \brief Retrieves the console input file name as unicode string.
 * \param nBufferLength Length of the buffer in WCHARs.
 *        Specify 0 to recieve the needed buffer length.
 * \param lpBuffer Pointer to a buffer that recieves the string.
 * \return Needed buffer size if \p nBufferLength is 0.
 *         Otherwise 1 if successful, 2 if buffer is too small.
 * \remarks Sets last error value to ERROR_BUFFER_OVERFLOW if the buffer
 *          is not big enough.
 */
DWORD
WINAPI
GetConsoleInputExeNameW(DWORD nBufferLength, LPWSTR lpBuffer)
{
    int lenName = lstrlenW(InputExeName);

    if (nBufferLength == 0)
    {
        /* Buffer size is requested, return it */
        return lenName + 1;
    }

    if(lenName + 1 > nBufferLength)
    {
        /* Buffer is not large enough! */
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return 2;
    }

    RtlEnterCriticalSection(&ConsoleLock);
    _SEH2_TRY
    {
        RtlCopyMemory(lpBuffer, InputExeName, lenName * sizeof(WCHAR));
        lpBuffer[lenName] = '\0';
    }
    _SEH2_FINALLY
    {
        RtlLeaveCriticalSection(&ConsoleLock);
    }
    _SEH2_END;

    /* Success, return 1 */
    return 1;
}


/******************************************************************************
 * \name GetConsoleInputExeNameA
 * \brief Retrieves the console input file name as ansi string.
 * \param nBufferLength Length of the buffer in CHARs.
 * \param lpBuffer Pointer to a buffer that recieves the string.
 * \return 1 if successful, 2 if buffer is too small.
 * \remarks Sets last error value to ERROR_BUFFER_OVERFLOW if the buffer
 *          is not big enough. The buffer recieves as much characters as fit.
 */
DWORD
WINAPI
GetConsoleInputExeNameA(DWORD nBufferLength, LPSTR lpBuffer)
{
    WCHAR Buffer[INPUTEXENAME_BUFLEN];
    DWORD Ret;
    UNICODE_STRING BufferU;
    ANSI_STRING BufferA;

    /* Get the unicode name */
    Ret = GetConsoleInputExeNameW(sizeof(Buffer) / sizeof(Buffer[0]), Buffer);

    /* Initialize strings for conversion */
    RtlInitUnicodeString(&BufferU, Buffer);
    BufferA.Length = 0;
    BufferA.MaximumLength = nBufferLength;
    BufferA.Buffer = lpBuffer;

    /* Convert unicode name to ansi, copying as much chars as fit */
    RtlUnicodeStringToAnsiString(&BufferA, &BufferU, FALSE);

    /* Error handling */
    if(nBufferLength <= BufferU.Length / sizeof(WCHAR))
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return 2;
    }

    return Ret;
}

BOOL
WINAPI
GetConsoleCharType(HANDLE hConsole, COORD Coord, PDWORD Type)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
GetConsoleCursorMode(HANDLE hConsole, PBOOL pUnknown1, PBOOL pUnknown2)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
GetConsoleNlsMode(HANDLE hConsole, LPDWORD lpMode)
{
    STUB;
    return FALSE;
}


BOOL
WINAPI
ReadConsoleInputExA(HANDLE hConsole, LPVOID lpBuffer, DWORD dwLen, LPDWORD Unknown1, DWORD Unknown2)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
ReadConsoleInputExW(HANDLE hConsole, LPVOID lpBuffer, DWORD dwLen, LPDWORD Unknown1, DWORD Unknown2)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
RegisterConsoleIME(HWND hWnd, LPDWORD ThreadId)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
RegisterConsoleOS2(BOOL bUnknown)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
SetConsoleCursorMode(HANDLE hConsole, BOOL Unknown1, BOOL Unknown2)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
SetConsoleLocalEUDC(DWORD Unknown1, DWORD Unknown2, DWORD Unknown3, DWORD Unknown4)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
SetConsoleNlsMode(HANDLE hConsole, DWORD dwMode)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
SetConsoleOS2OemFormat(BOOL bUnknown)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
UnregisterConsoleIME(VOID)
{
    STUB;
    return FALSE;
}


/*
 * @unimplemented
 */
BOOL WINAPI GetConsoleKeyboardLayoutNameA(LPSTR name)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL WINAPI GetConsoleKeyboardLayoutNameW(LPWSTR name)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
SetLastConsoleEventActive(VOID)
{
    STUB;
    return FALSE;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
SetConsoleCommandHistoryMode(IN DWORD dwMode)
{
    STUB;
    return FALSE;
}


/* EOF */
