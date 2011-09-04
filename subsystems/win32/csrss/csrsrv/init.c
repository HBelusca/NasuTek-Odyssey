/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey CSR Sub System
 * FILE:            subsystems/win32/csrss/csrsrv/init.c
 * PURPOSE:         CSR Server DLL Initialization
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include "srv.h"
#define NDEBUG
#include <debug.h>

/* DATA ***********************************************************************/

HANDLE CsrHeap = (HANDLE) 0;
HANDLE CsrObjectDirectory = (HANDLE) 0;
UNICODE_STRING CsrDirectoryName;
extern HANDLE CsrssApiHeap;
static unsigned ServerProcCount;
static CSRPLUGIN_SERVER_PROCS *ServerProcs = NULL;
HANDLE hSbApiPort = (HANDLE) 0;
HANDLE hBootstrapOk = (HANDLE) 0;
HANDLE hSmApiPort = (HANDLE) 0;
HANDLE hApiPort = (HANDLE) 0;

/* PRIVATE FUNCTIONS **********************************************************/

ULONG
InitializeVideoAddressSpace(VOID)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING PhysMemName = RTL_CONSTANT_STRING(L"\\Device\\PhysicalMemory");
    NTSTATUS Status;
    HANDLE PhysMemHandle;
    PVOID BaseAddress;
    LARGE_INTEGER Offset;
    SIZE_T ViewSize;
    CHAR IVTAndBda[1024+256];
    
    /* Free the 1MB pre-reserved region. In reality, Odyssey should simply support us mapping the view into the reserved area, but it doesn't. */
    BaseAddress = 0;
    ViewSize = 1024 * 1024;
    Status = ZwFreeVirtualMemory(NtCurrentProcess(), 
                                 &BaseAddress,
                                 &ViewSize,
                                 MEM_RELEASE);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Couldn't unmap reserved memory (%x)\n", Status);
        return 0;
    }
    
    /* Open the physical memory section */
    InitializeObjectAttributes(&ObjectAttributes,
                               &PhysMemName,
                               0,
                               NULL,
                               NULL);
    Status = ZwOpenSection(&PhysMemHandle,
                           SECTION_ALL_ACCESS,
                           &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Couldn't open \\Device\\PhysicalMemory\n");
        return 0;
    }

    /* Map the BIOS and device registers into the address space */
    Offset.QuadPart = 0xa0000;
    ViewSize = 0x100000 - 0xa0000;
    BaseAddress = (PVOID)0xa0000;
    Status = ZwMapViewOfSection(PhysMemHandle,
                                NtCurrentProcess(),
                                &BaseAddress,
                                0,
                                ViewSize,
                                &Offset,
                                &ViewSize,
                                ViewUnmap,
                                0,
                                PAGE_EXECUTE_READWRITE);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Couldn't map physical memory (%x)\n", Status);
        ZwClose(PhysMemHandle);
        return 0;
    }

    /* Close physical memory section handle */
    ZwClose(PhysMemHandle);

    if (BaseAddress != (PVOID)0xa0000)
    {
        DPRINT1("Couldn't map physical memory at the right address (was %x)\n",
                BaseAddress);
        return 0;
    }

    /* Allocate some low memory to use for the non-BIOS
     * parts of the v86 mode address space
     */
    BaseAddress = (PVOID)0x1;
    ViewSize = 0xa0000 - 0x1000;
    Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
                                     &BaseAddress,
                                     0,
                                     &ViewSize,
                                     MEM_RESERVE | MEM_COMMIT,
                                     PAGE_EXECUTE_READWRITE);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to allocate virtual memory (Status %x)\n", Status);
        return 0;
    }
    if (BaseAddress != (PVOID)0x0)
    {
        DPRINT1("Failed to allocate virtual memory at right address (was %x)\n",
                BaseAddress);
        return 0;
    }

    /* Get the real mode IVT and BDA from the kernel */
    Status = NtVdmControl(VdmInitialize, IVTAndBda);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("NtVdmControl failed (status %x)\n", Status);
        return 0;
    }

    /* Return success */
    return 1;
}


static NTSTATUS FASTCALL
CsrpAddServerProcs(CSRPLUGIN_SERVER_PROCS *Procs)
{
  CSRPLUGIN_SERVER_PROCS *NewProcs;

  DPRINT("CSR: %s called\n", __FUNCTION__);

  NewProcs = RtlAllocateHeap(CsrssApiHeap, 0,
                             (ServerProcCount + 1)
                             * sizeof(CSRPLUGIN_SERVER_PROCS));
  if (NULL == NewProcs)
    {
      return STATUS_NO_MEMORY;
    }
  if (0 != ServerProcCount)
    {
      RtlCopyMemory(NewProcs, ServerProcs,
                    ServerProcCount * sizeof(CSRPLUGIN_SERVER_PROCS));
      RtlFreeHeap(CsrssApiHeap, 0, ServerProcs);
    }
  NewProcs[ServerProcCount] = *Procs;
  ServerProcs = NewProcs;
  ServerProcCount++;

  return STATUS_SUCCESS;
}

/**********************************************************************
 * CallInitComplete/0
 */
static BOOL FASTCALL
CallInitComplete(void)
{
  BOOL Ok;
  unsigned i;

  DPRINT("CSR: %s called\n", __FUNCTION__);

  Ok = TRUE;
  for (i = 0; i < ServerProcCount && Ok; i++)
    {
      Ok = (*ServerProcs[i].InitCompleteProc)();
    }

  return Ok;
}

BOOL
CallHardError(IN PCSRSS_PROCESS_DATA ProcessData,
              IN PHARDERROR_MSG HardErrorMessage)
{
    BOOL Ok;
    unsigned i;

    DPRINT("CSR: %s called\n", __FUNCTION__);

    Ok = TRUE;
    for (i = 0; i < ServerProcCount && Ok; i++)
    {
        Ok = (*ServerProcs[i].HardErrorProc)(ProcessData, HardErrorMessage);
    }

    return Ok;
}

NTSTATUS
CallProcessInherit(IN PCSRSS_PROCESS_DATA SourceProcessData,
                   IN PCSRSS_PROCESS_DATA TargetProcessData)
{
    NTSTATUS Status = STATUS_SUCCESS;
    unsigned i;

    DPRINT("CSR: %s called\n", __FUNCTION__);

    for (i = 0; i < ServerProcCount && NT_SUCCESS(Status); i++)
        Status = (*ServerProcs[i].ProcessInheritProc)(SourceProcessData, TargetProcessData);

    return Status;
}

NTSTATUS
CallProcessDeleted(IN PCSRSS_PROCESS_DATA ProcessData)
{
    NTSTATUS Status = STATUS_SUCCESS;
    unsigned i;

    DPRINT("CSR: %s called\n", __FUNCTION__);

    for (i = 0; i < ServerProcCount && NT_SUCCESS(Status); i++)
        Status = (*ServerProcs[i].ProcessDeletedProc)(ProcessData);

    return Status;
}


ULONG
InitializeVideoAddressSpace(VOID);

/**********************************************************************
 * CsrpCreateObjectDirectory/3
 */
static NTSTATUS
CsrpCreateObjectDirectory (int argc, char ** argv, char ** envp)
{
   NTSTATUS Status;
   OBJECT_ATTRIBUTES Attributes;

  DPRINT("CSR: %s called\n", __FUNCTION__);


	/* create object directory ('\Windows') */
	RtlCreateUnicodeString (&CsrDirectoryName,
	                        L"\\Windows");

	InitializeObjectAttributes (&Attributes,
	                            &CsrDirectoryName,
	                            OBJ_OPENIF,
	                            NULL,
	                            NULL);

	Status = NtOpenDirectoryObject(&CsrObjectDirectory,
	                               DIRECTORY_ALL_ACCESS,
	                               &Attributes);
	return Status;
}

/**********************************************************************
 * CsrpInitVideo/3
 *
 * TODO: we need a virtual device for sessions other than
 * TODO: the console one
 */
static NTSTATUS
CsrpInitVideo (int argc, char ** argv, char ** envp)
{
  OBJECT_ATTRIBUTES ObjectAttributes;
  UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\??\\DISPLAY1");
  IO_STATUS_BLOCK Iosb;
  HANDLE VideoHandle = (HANDLE) 0;
  NTSTATUS Status = STATUS_SUCCESS;

  DPRINT("CSR: %s called\n", __FUNCTION__);

  InitializeVideoAddressSpace();

  InitializeObjectAttributes(&ObjectAttributes,
			     &DeviceName,
			     0,
			     NULL,
			     NULL);
  Status = NtOpenFile(&VideoHandle,
		      FILE_ALL_ACCESS,
		      &ObjectAttributes,
		      &Iosb,
		      0,
		      0);
  if (NT_SUCCESS(Status))
    {
      NtClose(VideoHandle);
    }
  return Status;
}

/**********************************************************************
 * CsrpInitWin32Csr/3
 *
 * TODO: this function should be turned more general to load an
 * TODO: hosted server DLL as received from the command line;
 * TODO: for instance: ServerDll=winsrv:ConServerDllInitialization,2
 * TODO:               ^method   ^dll   ^api                       ^sid
 * TODO:
 * TODO: CsrpHostServerDll (LPWSTR DllName,
 * TODO:                    LPWSTR ApiName,
 * TODO:                    DWORD  ServerId)
 */
static NTSTATUS
CsrpInitWin32Csr (int argc, char ** argv, char ** envp)
{
  NTSTATUS Status;
  UNICODE_STRING DllName;
  HINSTANCE hInst;
  ANSI_STRING ProcName;
  CSRPLUGIN_INITIALIZE_PROC InitProc;
  CSRSS_EXPORTED_FUNCS Exports;
  PCSRSS_API_DEFINITION ApiDefinitions;
  CSRPLUGIN_SERVER_PROCS ServerProcs;

  DPRINT("CSR: %s called\n", __FUNCTION__);

  RtlInitUnicodeString(&DllName, L"win32csr.dll");
  Status = LdrLoadDll(NULL, 0, &DllName, (PVOID *) &hInst);
  if (! NT_SUCCESS(Status))
    {
      return Status;
    }
  RtlInitAnsiString(&ProcName, "Win32CsrInitialization");
  Status = LdrGetProcedureAddress(hInst, &ProcName, 0, (PVOID *) &InitProc);
  if (! NT_SUCCESS(Status))
    {
      return Status;
    }
  Exports.CsrEnumProcessesProc = CsrEnumProcesses;
  if (! (*InitProc)(&ApiDefinitions, &ServerProcs, &Exports, CsrssApiHeap))
    {
      return STATUS_UNSUCCESSFUL;
    }

  Status = CsrApiRegisterDefinitions(ApiDefinitions);
  if (! NT_SUCCESS(Status))
    {
      return Status;
    }
  Status = CsrpAddServerProcs(&ServerProcs);
  return Status;
}

CSRSS_API_DEFINITION NativeDefinitions[] =
  {
    CSRSS_DEFINE_API(CREATE_PROCESS,               CsrCreateProcess),
    CSRSS_DEFINE_API(CREATE_THREAD,                CsrSrvCreateThread),
    CSRSS_DEFINE_API(TERMINATE_PROCESS,            CsrTerminateProcess),
    CSRSS_DEFINE_API(CONNECT_PROCESS,              CsrConnectProcess),
    CSRSS_DEFINE_API(REGISTER_SERVICES_PROCESS,    CsrRegisterServicesProcess),
    CSRSS_DEFINE_API(GET_SHUTDOWN_PARAMETERS,      CsrGetShutdownParameters),
    CSRSS_DEFINE_API(SET_SHUTDOWN_PARAMETERS,      CsrSetShutdownParameters),
    { 0, 0, NULL }
  };

static NTSTATUS WINAPI
CsrpCreateListenPort (IN     LPWSTR  Name,
		      IN OUT PHANDLE Port,
		      IN     PTHREAD_START_ROUTINE ListenThread)
{
	NTSTATUS           Status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES  PortAttributes;
	UNICODE_STRING     PortName;
    HANDLE ServerThread;
    CLIENT_ID ClientId;

	DPRINT("CSR: %s called\n", __FUNCTION__);

	RtlInitUnicodeString (& PortName, Name);
	InitializeObjectAttributes (& PortAttributes,
				    & PortName,
				    0,
				    NULL,
				    NULL);
	Status = NtCreatePort ( Port,
				& PortAttributes,
				LPC_MAX_DATA_LENGTH, /* TODO: make caller set it*/
				LPC_MAX_MESSAGE_LENGTH, /* TODO: make caller set it*/
				0); /* TODO: make caller set it*/
	if(!NT_SUCCESS(Status))
	{
		DPRINT1("CSR: %s: NtCreatePort failed (Status=%08lx)\n",
			__FUNCTION__, Status);
		return Status;
	}
	Status = RtlCreateUserThread(NtCurrentProcess(),
                               NULL,
                               TRUE,
                               0,
                               0,
                               0,
                               (PTHREAD_START_ROUTINE) ListenThread,
                               *Port,
                               &ServerThread,
                               &ClientId);
    
    if (ListenThread == (PVOID)ClientConnectionThread)
    {
        CsrAddStaticServerThread(ServerThread, &ClientId, 0);
    }
    
    NtResumeThread(ServerThread, NULL);
    NtClose(ServerThread);
	return Status;
}

/* === INIT ROUTINES === */

/**********************************************************************
 * CsrpCreateBNODirectory/3
 *
 * These used to be part of kernel32 startup, but that clearly wasn't a good
 * idea, as races were definately possible.  These are moved (as in the
 * previous fixmes).
 */
static NTSTATUS
CsrpCreateBNODirectory (int argc, char ** argv, char ** envp)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING Name = RTL_CONSTANT_STRING(L"\\BaseNamedObjects");
    UNICODE_STRING SymName = RTL_CONSTANT_STRING(L"Local");
    UNICODE_STRING SymName2 = RTL_CONSTANT_STRING(L"Global");
    HANDLE DirHandle, SymHandle;

    /* Seems like a good place to create these objects which are needed by
     * win32 processes */
    InitializeObjectAttributes(&ObjectAttributes,
                               &Name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtCreateDirectoryObject(&DirHandle,
                                     DIRECTORY_ALL_ACCESS,
                                     &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("NtCreateDirectoryObject() failed %08x\n", Status);
    }

    /* Create the "local" Symbolic Link.
     * FIXME: CSR should do this -- Fixed */
    InitializeObjectAttributes(&ObjectAttributes,
                               &SymName,
                               OBJ_CASE_INSENSITIVE,
                               DirHandle,
                               NULL);
    Status = NtCreateSymbolicLinkObject(&SymHandle,
                                        SYMBOLIC_LINK_ALL_ACCESS,
                                        &ObjectAttributes,
                                        &Name);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("NtCreateDirectoryObject() failed %08x\n", Status);
    }

    /* Create the "global" Symbolic Link. */
    InitializeObjectAttributes(&ObjectAttributes,
                               &SymName2,
                               OBJ_CASE_INSENSITIVE,
                               DirHandle,
                               NULL);
    Status = NtCreateSymbolicLinkObject(&SymHandle,
                                        SYMBOLIC_LINK_ALL_ACCESS,
                                        &ObjectAttributes,
                                        &Name);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("NtCreateDirectoryObject() failed %08x\n", Status);
    }

    return Status;
}


VOID
WINAPI
BasepFakeStaticServerData(VOID);

NTSTATUS
NTAPI
CsrSrvCreateSharedSection(IN PCHAR ParameterValue);

/**********************************************************************
 * CsrpCreateHeap/3
 */
static NTSTATUS
CsrpCreateHeap (int argc, char ** argv, char ** envp)
{
    CHAR Value[] = "1024,3072,512";
    NTSTATUS Status;
	DPRINT("CSR: %s called\n", __FUNCTION__);

	CsrssApiHeap = RtlCreateHeap(HEAP_GROWABLE,
        	                       NULL,
                	               65536,
                        	       65536,
	                               NULL,
        	                       NULL);
	if (CsrssApiHeap == NULL)
	{
		return STATUS_UNSUCCESSFUL;
	}
    
    
    Status = CsrSrvCreateSharedSection(Value);
    if (Status != STATUS_SUCCESS)
    {
        DPRINT1("CsrSrvCreateSharedSection failed with status 0x%08lx\n", Status);
        ASSERT(FALSE);
    }

    BasepFakeStaticServerData();
	return STATUS_SUCCESS;
}

/**********************************************************************
 * CsrpCreateCallbackPort/3
 */
static NTSTATUS
CsrpCreateCallbackPort (int argc, char ** argv, char ** envp)
{
	DPRINT("CSR: %s called\n", __FUNCTION__);

	return CsrpCreateListenPort (L"\\Windows\\SbApiPort",
				     & hSbApiPort,
				     ServerSbApiPortThread);
}

/**********************************************************************
 * CsrpRegisterSubsystem/3
 */
static NTSTATUS
CsrpRegisterSubsystem (int argc, char ** argv, char ** envp)
{
	NTSTATUS           Status = STATUS_SUCCESS;
	OBJECT_ATTRIBUTES  BootstrapOkAttributes;
	UNICODE_STRING     Name;

	DPRINT("CSR: %s called\n", __FUNCTION__);

	/*
	 * Create the event object the callback port
	 * thread will signal *if* the SM will
	 * authorize us to bootstrap.
	 */
	RtlInitUnicodeString (& Name, L"\\CsrssBooting");
	InitializeObjectAttributes(& BootstrapOkAttributes,
				   & Name,
				   0, NULL, NULL);
	Status = NtCreateEvent (& hBootstrapOk,
				EVENT_ALL_ACCESS,
				& BootstrapOkAttributes,
				SynchronizationEvent,
				FALSE);
	if(!NT_SUCCESS(Status))
	{
		DPRINT("CSR: %s: NtCreateEvent failed (Status=0x%08lx)\n",
			__FUNCTION__, Status);
		return Status;
	}
	/*
	 * Let's tell the SM a new environment
	 * subsystem server is in the system.
	 */
	RtlInitUnicodeString (& Name, L"\\Windows\\SbApiPort");
	DPRINT("CSR: %s: registering with SM for\n  IMAGE_SUBSYSTEM_WINDOWS_CUI == 3\n", __FUNCTION__);
	Status = SmConnectApiPort (& Name,
				   hSbApiPort,
				   IMAGE_SUBSYSTEM_WINDOWS_CUI,
				   & hSmApiPort);
	if(!NT_SUCCESS(Status))
	{
		DPRINT("CSR: %s unable to connect to the SM (Status=0x%08lx)\n",
			__FUNCTION__, Status);
		NtClose (hBootstrapOk);
		return Status;
	}
	/*
	 *  Wait for SM to reply OK... If the SM
	 *  won't answer, we hang here forever!
	 */
	DPRINT("CSR: %s: waiting for SM to OK boot...\n", __FUNCTION__);
	Status = NtWaitForSingleObject (hBootstrapOk,
					FALSE,
					NULL);
	NtClose (hBootstrapOk);
	return Status;
}

/**********************************************************************
 * 	CsrpLoadKernelModeDriver/3
 */
static NTSTATUS
CsrpLoadKernelModeDriver (int argc, char ** argv, char ** envp)
{
	NTSTATUS        Status = STATUS_SUCCESS;
	WCHAR           Data [MAX_PATH + 1];
	ULONG           DataLength = sizeof Data;
	ULONG           DataType = 0;
	//UNICODE_STRING  Environment;


	DPRINT1("SM: %s called\n", __FUNCTION__);


	//EnvpToUnicodeString (envp, & Environment);
	Status = SmLookupSubsystem (L"Kmode",
				    Data,
				    & DataLength,
				    & DataType,
				    NULL);
	//RtlFreeUnicodeString (& Environment);
	if((STATUS_SUCCESS == Status) && (DataLength > sizeof Data[0]))
	{
		WCHAR                      ImagePath [MAX_PATH + 1] = {0};
		UNICODE_STRING             ModuleName;

		wcscpy (ImagePath, L"\\SYSTEMROOT\\system32\\win32k.sys");
//		wcscat (ImagePath, Data);
		RtlInitUnicodeString (& ModuleName, ImagePath);
		Status = NtSetSystemInformation(/* FIXME: SystemLoadAndCallImage */
		                                SystemExtendServiceTableInformation,
						& ModuleName,
						sizeof ModuleName);
		if(!NT_SUCCESS(Status))
		{
			DPRINT1("WIN: %s: loading Kmode failed (Status=0x%08lx)\n",
				__FUNCTION__, Status);
		}
	}
	return Status;
}

/**********************************************************************
 * CsrpCreateApiPort/2
 */
static NTSTATUS
CsrpCreateApiPort (int argc, char ** argv, char ** envp)
{
	DPRINT("CSR: %s called\n", __FUNCTION__);

	CsrInitProcessData();

	return CsrpCreateListenPort(L"\\Windows\\ApiPort", &hApiPort,
		(PTHREAD_START_ROUTINE)ClientConnectionThread);
}

/**********************************************************************
 * CsrpApiRegisterDef/0
 */
static NTSTATUS
CsrpApiRegisterDef (int argc, char ** argv, char ** envp)
{
	return CsrApiRegisterDefinitions(NativeDefinitions);
}

/**********************************************************************
 * CsrpCCTS/2
 */
static NTSTATUS
CsrpCCTS (int argc, char ** argv, char ** envp)
{
    ULONG Dummy;
    ULONG DummyLength = sizeof(Dummy);
	return CsrClientConnectToServer(L"\\Windows",
			0, &Dummy, &DummyLength, NULL);
}

/**********************************************************************
 * CsrpRunWinlogon/0
 *
 * Start the logon process (winlogon.exe).
 *
 * TODO: this should be moved in CsrpCreateSession/x (one per session)
 * TODO: in its own desktop (one logon desktop per winstation).
 */
static NTSTATUS
CsrpRunWinlogon (int argc, char ** argv, char ** envp)
{
	NTSTATUS                      Status = STATUS_SUCCESS;
	UNICODE_STRING                ImagePath;
	UNICODE_STRING                CommandLine;
	PRTL_USER_PROCESS_PARAMETERS  ProcessParameters = NULL;
	RTL_USER_PROCESS_INFORMATION  ProcessInfo;


	DPRINT("CSR: %s called\n", __FUNCTION__);

	/* initialize the process parameters */
	RtlInitUnicodeString (& ImagePath, L"\\SystemRoot\\system32\\winlogon.exe");
	RtlInitUnicodeString (& CommandLine, L"");
	RtlCreateProcessParameters(& ProcessParameters,
				   & ImagePath,
				   NULL,
				   NULL,
				   & CommandLine,
				   NULL,
				   NULL,
				   NULL,
				   NULL,
				   NULL);
	/* Create the winlogon process */
	Status = RtlCreateUserProcess (& ImagePath,
				       OBJ_CASE_INSENSITIVE,
				       ProcessParameters,
				       NULL,
				       NULL,
				       NULL,
				       FALSE,
				       NULL,
				       NULL,
				       & ProcessInfo);
	/* Cleanup */
	RtlDestroyProcessParameters (ProcessParameters);
	if (!NT_SUCCESS(Status))
	{
		DPRINT1("SM: %s: loading winlogon.exe failed (Status=%08lx)\n",
				__FUNCTION__, Status);
	}

   ZwResumeThread(ProcessInfo.ThreadHandle, NULL);
	return Status;
}

static NTSTATUS
CsrpCreateHardErrorPort (int argc, char ** argv, char ** envp)
{
    return NtSetDefaultHardErrorPort(hApiPort);
}

typedef NTSTATUS (* CSR_INIT_ROUTINE)(int,char**,char**);

struct {
	BOOL Required;
	CSR_INIT_ROUTINE EntryPoint;
	PCHAR ErrorMessage;
} InitRoutine [] = {
        {TRUE, CsrpCreateBNODirectory,   "create base named objects directory"},
	{TRUE, CsrpCreateCallbackPort,   "create the callback port \\Windows\\SbApiPort"},
	{TRUE, CsrpRegisterSubsystem,    "register with SM"},
	{TRUE, CsrpCreateHeap,           "create the CSR heap"},
	{TRUE, CsrpCreateApiPort,        "create the api port \\Windows\\ApiPort"},
    {TRUE, CsrpCreateHardErrorPort,  "create the hard error port"},
	{TRUE, CsrpCreateObjectDirectory,"create the object directory \\Windows"},
	{TRUE, CsrpLoadKernelModeDriver, "load Kmode driver"},
	{TRUE, CsrpInitVideo,            "initialize video"},
	{TRUE, CsrpApiRegisterDef,       "initialize api definitions"},
	{TRUE, CsrpCCTS,                 "connect client to server"},
	{TRUE, CsrpInitWin32Csr,         "load usermode dll"},
	{TRUE, CsrpRunWinlogon,          "run WinLogon"},
};

/* PUBLIC FUNCTIONS ***********************************************************/

NTSTATUS
NTAPI
CsrServerInitialization(ULONG ArgumentCount,
                        PCHAR Arguments[])
{
	UINT       i = 0;
	NTSTATUS  Status = STATUS_SUCCESS;

	DPRINT("CSR: %s called\n", __FUNCTION__);

	for (i=0; i < (sizeof InitRoutine / sizeof InitRoutine[0]); i++)
	{
		Status = InitRoutine[i].EntryPoint(ArgumentCount,Arguments,NULL);
		if(!NT_SUCCESS(Status))
		{
			DPRINT1("CSR: %s: failed to %s (Status=%08lx)\n",
				__FUNCTION__,
				InitRoutine[i].ErrorMessage,
				Status);
			if (InitRoutine[i].Required)
			{
				return FALSE;
			}
		}
	}
	if (CallInitComplete())
	{
		Status = SmCompleteSession (hSmApiPort,hSbApiPort,hApiPort);
		return STATUS_SUCCESS;
	}
    
	return STATUS_UNSUCCESSFUL;
}

BOOL
NTAPI
DllMain(HANDLE hDll,
        DWORD dwReason,
        LPVOID lpReserved)
{
    /* We don't do much */
    UNREFERENCED_PARAMETER(hDll);
    UNREFERENCED_PARAMETER(dwReason);
    UNREFERENCED_PARAMETER(lpReserved);
    return TRUE;
}

/* EOF */
