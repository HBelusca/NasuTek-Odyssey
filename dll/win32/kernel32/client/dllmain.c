/* $Id: dllmain.c 53068 2011-08-04 22:18:01Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/kernel32/misc/dllmain.c
 * PURPOSE:         Initialization
 * PROGRAMMER:      Ariadne ( ariadne@xs4all.nl)
 *                  Aleksey Bragin (aleksey@odyssey.org)
 * UPDATE HISTORY:
 *                  Created 01/11/98
 */

/* INCLUDES ******************************************************************/

#include <k32.h>

#define NDEBUG
#include <debug.h>

/* GLOBALS *******************************************************************/

extern UNICODE_STRING SystemDirectory;
extern UNICODE_STRING WindowsDirectory;

PBASE_STATIC_SERVER_DATA BaseStaticServerData;

BOOLEAN BaseRunningInServerProcess;

WCHAR BaseDefaultPathBuffer[6140];

HANDLE BaseNamedObjectDirectory;
HMODULE hCurrentModule = NULL;
HMODULE kernel32_handle = NULL;
HANDLE hBaseDir = NULL;
PPEB Peb;
ULONG SessionId;
BOOL ConsoleInitialized = FALSE;
UNICODE_STRING BaseWindowsDirectory, BaseWindowsSystemDirectory;
static BOOL DllInitialized = FALSE;

BOOL WINAPI
DllMain(HANDLE hInst,
	DWORD dwReason,
	LPVOID lpReserved);

/* Critical section for various kernel32 data structures */
RTL_CRITICAL_SECTION BaseDllDirectoryLock;
RTL_CRITICAL_SECTION ConsoleLock;

extern BOOL WINAPI DefaultConsoleCtrlHandler(DWORD Event);
extern __declspec(noreturn) VOID CALLBACK ConsoleControlDispatcher(DWORD CodeAndFlag);
extern PHANDLER_ROUTINE InitialHandler[1];
extern PHANDLER_ROUTINE* CtrlHandlers;
extern ULONG NrCtrlHandlers;
extern ULONG NrAllocatedHandlers;
extern BOOL FASTCALL NlsInit(VOID);
extern VOID FASTCALL NlsUninit(VOID);
BOOLEAN InWindows = FALSE;

#define WIN_OBJ_DIR L"\\Windows"
#define SESSION_DIR L"\\Sessions"

/* FUNCTIONS *****************************************************************/

NTSTATUS
WINAPI
BaseGetNamedObjectDirectory(VOID)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjectAttributes,
                               &BaseStaticServerData->NamedObjectDirectory,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenDirectoryObject(&BaseNamedObjectDirectory,
                                   DIRECTORY_ALL_ACCESS &
                                   ~(DELETE | WRITE_DAC | WRITE_OWNER),
                                   &ObjectAttributes);
    if (!NT_SUCCESS(Status)) return Status;

    DPRINT("Opened BNO: %lx\n", BaseNamedObjectDirectory);
    return Status;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
BaseQueryModuleData(IN LPSTR ModuleName,
                    IN LPSTR Unknown,
                    IN PVOID Unknown2,
                    IN PVOID Unknown3,
                    IN PVOID Unknown4)
{
    DPRINT1("BaseQueryModuleData called: %s %s %x %x %x\n",
            ModuleName,
            Unknown,
            Unknown2,
            Unknown3,
            Unknown4);
    return FALSE;
}

/*
 * @unimplemented
 */
NTSTATUS
WINAPI
BaseProcessInitPostImport(VOID)
{
    /* FIXME: Initialize TS pointers */
    return STATUS_SUCCESS;
}

BOOL
WINAPI
BasepInitConsole(VOID)
{
    CSR_API_MESSAGE Request;
    ULONG CsrRequest;
    NTSTATUS Status;
    BOOLEAN NotConsole = FALSE;
    PRTL_USER_PROCESS_PARAMETERS Parameters = NtCurrentPeb()->ProcessParameters;
    LPCWSTR ExeName;

    WCHAR lpTest[MAX_PATH];
    GetModuleFileNameW(NULL, lpTest, MAX_PATH);
    DPRINT("BasepInitConsole for : %S\n", lpTest);
    DPRINT("Our current console handles are: %lx, %lx, %lx %lx\n",
           Parameters->ConsoleHandle, Parameters->StandardInput,
           Parameters->StandardOutput, Parameters->StandardError);

    /* We have nothing to do if this isn't a console app... */
    if (RtlImageNtHeader(GetModuleHandle(NULL))->OptionalHeader.Subsystem !=
        IMAGE_SUBSYSTEM_WINDOWS_CUI)
    {
        DPRINT("Image is not a console application\n");
        Parameters->ConsoleHandle = NULL;
        Request.Data.AllocConsoleRequest.ConsoleNeeded = FALSE;
    }
    else
    {
        /* Assume one is needed */
        Request.Data.AllocConsoleRequest.ConsoleNeeded = TRUE;
        Request.Data.AllocConsoleRequest.Visible = TRUE;

        /* Handle the special flags given to us by BasepInitializeEnvironment */
        if (Parameters->ConsoleHandle == HANDLE_DETACHED_PROCESS)
        {
            /* No console to create */
            DPRINT("No console to create\n");
            Parameters->ConsoleHandle = NULL;
            Request.Data.AllocConsoleRequest.ConsoleNeeded = FALSE;
        }
        else if (Parameters->ConsoleHandle == HANDLE_CREATE_NEW_CONSOLE)
        {
            /* We'll get the real one soon */
            DPRINT("Creating new console\n");
            Parameters->ConsoleHandle = NULL;
        }
        else if (Parameters->ConsoleHandle == HANDLE_CREATE_NO_WINDOW)
        {
            /* We'll get the real one soon */
            DPRINT("Creating new invisible console\n");
            Parameters->ConsoleHandle = NULL;
            Request.Data.AllocConsoleRequest.Visible = FALSE;
        }
        else
        {
            if (Parameters->ConsoleHandle == INVALID_HANDLE_VALUE)
            {
                Parameters->ConsoleHandle = 0;
            }
            DPRINT("Using existing console: %x\n", Parameters->ConsoleHandle);
        }
    }

    /* Initialize Console Ctrl Handler and input EXE name */
    ConsoleInitialized = TRUE;
    RtlInitializeCriticalSection(&ConsoleLock);
    NrAllocatedHandlers = 1;
    NrCtrlHandlers = 1;
    CtrlHandlers = InitialHandler;
    CtrlHandlers[0] = DefaultConsoleCtrlHandler;

    ExeName = wcsrchr(Parameters->ImagePathName.Buffer, L'\\');
    if (ExeName)
        SetConsoleInputExeNameW(ExeName + 1);

    /* Now use the proper console handle */
    Request.Data.AllocConsoleRequest.Console = Parameters->ConsoleHandle;

    /*
     * Normally, we should be connecting to the Console CSR Server...
     * but we don't have one yet, so we will instead simply send a create
     * console message to the Base Server. When we finally have a Console
     * Server, this code should be changed to send connection data instead.
     */
    CsrRequest = MAKE_CSR_API(ALLOC_CONSOLE, CSR_CONSOLE);
    Request.Data.AllocConsoleRequest.CtrlDispatcher = ConsoleControlDispatcher;
    Status = CsrClientCallServer(&Request,
                                 NULL,
                                 CsrRequest,
                                 sizeof(CSR_API_MESSAGE));
    if(!NT_SUCCESS(Status) || !NT_SUCCESS(Status = Request.Status))
    {
        DPRINT1("CSR Failed to give us a console\n");
        /* We're lying here, so at least the process can load... */
        return TRUE;
    }

    if (NotConsole) return TRUE;

    /* We got the handles, let's set them */
    if ((Parameters->ConsoleHandle = Request.Data.AllocConsoleRequest.Console))
    {
        /* If we already had some, don't use the new ones */
        if (!Parameters->StandardInput)
        {
            Parameters->StandardInput = Request.Data.AllocConsoleRequest.InputHandle;
        }
        if (!Parameters->StandardOutput)
        {
            Parameters->StandardOutput = Request.Data.AllocConsoleRequest.OutputHandle;
        }
        if (!Parameters->StandardError)
        {
            Parameters->StandardError = Request.Data.AllocConsoleRequest.OutputHandle;
        }
    }

    DPRINT("Console setup: %lx, %lx, %lx, %lx\n",
            Parameters->ConsoleHandle,
            Parameters->StandardInput,
            Parameters->StandardOutput,
            Parameters->StandardError);
    return TRUE;
}

BOOL
WINAPI
DllMain(HANDLE hDll,
        DWORD dwReason,
        LPVOID lpReserved)
{
    NTSTATUS Status;
    ULONG Dummy;
    ULONG DummySize = sizeof(Dummy);
    WCHAR SessionDir[256];

    DPRINT("DllMain(hInst %lx, dwReason %lu)\n",
           hDll, dwReason);

    Basep8BitStringToUnicodeString = RtlAnsiStringToUnicodeString;

    /* Cache the PEB and Session ID */
    Peb = NtCurrentPeb();
    SessionId = Peb->SessionId;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

        /* Don't bother us for each thread */
        LdrDisableThreadCalloutsForDll((PVOID)hDll);

        /* Initialize default path to NULL */
        RtlInitUnicodeString(&BaseDefaultPath, NULL);

        /* Setup the right Object Directory path */
        if (!SessionId)
        {
            /* Use the raw path */
            wcscpy(SessionDir, WIN_OBJ_DIR);
        }
        else
        {
            /* Use the session path */
            swprintf(SessionDir,
                     L"%ws\\%ld%ws",
                     SESSION_DIR,
                     SessionId,
                     WIN_OBJ_DIR);
        }

        /* Connect to the base server */
        DPRINT("Connecting to CSR...\n");
        Status = CsrClientConnectToServer(SessionDir,
                                          InWindows ? 1 : 0,
                                          &Dummy,
                                          &DummySize,
                                          &BaseRunningInServerProcess);
        if (!NT_SUCCESS(Status))
        {
            DPRINT1("Failed to connect to CSR (Status %lx)\n", Status);
            ZwTerminateProcess(NtCurrentProcess(), Status);
            return FALSE;
        }

        /* Get the server data */
        ASSERT(Peb->ReadOnlyStaticServerData);
        BaseStaticServerData = Peb->ReadOnlyStaticServerData[CSR_CONSOLE];
        ASSERT(BaseStaticServerData);

        /* Check if we are running a CSR Server */
        if (!BaseRunningInServerProcess)
        {
            /* Set the termination port for the thread */
            DPRINT("Creating new thread for CSR\n");
            CsrNewThread();
        }

        /* Initialize heap handle table */
        BaseDllInitializeMemoryManager();

        /* Set HMODULE for our DLL */
        kernel32_handle = hCurrentModule = hDll;

        /* Set the directories */
        BaseWindowsDirectory = BaseStaticServerData->WindowsDirectory;
        BaseWindowsSystemDirectory = BaseStaticServerData->WindowsSystemDirectory;
        SystemDirectory = BaseWindowsSystemDirectory;
        WindowsDirectory = BaseWindowsDirectory;

        /* Construct the default path (using the static buffer) */
        _snwprintf(BaseDefaultPathBuffer, sizeof(BaseDefaultPathBuffer) / sizeof(WCHAR),
            L".;%wZ;%wZ\\system;%wZ;", &SystemDirectory, &WindowsDirectory, &WindowsDirectory);

        BaseDefaultPath.Buffer = BaseDefaultPathBuffer;
        BaseDefaultPath.Length = wcslen(BaseDefaultPathBuffer) * sizeof(WCHAR);
        BaseDefaultPath.MaximumLength = sizeof(BaseDefaultPathBuffer);

        /* Use remaining part of the default path buffer for the append path */
        BaseDefaultPathAppend.Buffer = (PWSTR)((ULONG_PTR)BaseDefaultPathBuffer + BaseDefaultPath.Length);
        BaseDefaultPathAppend.Length = 0;
        BaseDefaultPathAppend.MaximumLength = BaseDefaultPath.MaximumLength - BaseDefaultPath.Length;

        /* Initialize command line */
        InitCommandLines();

        /* Open object base directory */
        Status = BaseGetNamedObjectDirectory();
        hBaseDir = BaseNamedObjectDirectory;
        if (!NT_SUCCESS(Status))
        {
            DPRINT1("Failed to open object base directory (Status %lx)\n", Status);
            return FALSE;
        }

        /* Initialize the DLL critical section */
        RtlInitializeCriticalSection(&BaseDllDirectoryLock);

        /* Initialize the National Language Support routines */
        if (!NlsInit())
        {
            DPRINT1("NLS Init failed\n");
            return FALSE;
        }

        /* Initialize Console Support */
        if (!BasepInitConsole())
        {
            DPRINT1("Failure to set up console\n");
            return FALSE;
        }

        /* Insert more dll attach stuff here! */
        DllInitialized = TRUE;
        DPRINT("Initialization complete\n");
        break;

        case DLL_PROCESS_DETACH:

            DPRINT("DLL_PROCESS_DETACH\n");
            if (DllInitialized == TRUE)
            {
                /* Insert more dll detach stuff here! */
                NlsUninit();

                /* Delete DLL critical section */
                if (ConsoleInitialized == TRUE)
                {
                    ConsoleInitialized = FALSE;
                    RtlDeleteCriticalSection (&ConsoleLock);
                }
                RtlDeleteCriticalSection (&BaseDllDirectoryLock);

                /* Close object base directory */
                NtClose(hBaseDir);
            }
            break;

        default:
            break;
    }

    return TRUE;
}

#undef InterlockedIncrement
#undef InterlockedDecrement
#undef InterlockedExchange
#undef InterlockedExchangeAdd
#undef InterlockedCompareExchange

LONG
WINAPI
InterlockedIncrement(IN OUT LONG volatile *lpAddend)
{
    return _InterlockedIncrement(lpAddend);
}

LONG
WINAPI
InterlockedDecrement(IN OUT LONG volatile *lpAddend)
{
    return _InterlockedDecrement(lpAddend);
}

#undef InterlockedExchange
LONG
WINAPI
InterlockedExchange(IN OUT LONG volatile *Target,
                    IN LONG Value)
{
    return _InterlockedExchange(Target, Value);
}

LONG
WINAPI
InterlockedExchangeAdd(IN OUT LONG volatile *Addend,
                       IN LONG Value)
{
    return _InterlockedExchangeAdd(Addend, Value);
}

LONG
WINAPI
InterlockedCompareExchange(IN OUT LONG volatile *Destination,
                           IN LONG Exchange,
                           IN LONG Comperand)
{
    return _InterlockedCompareExchange(Destination, Exchange, Comperand);
}

/* EOF */
