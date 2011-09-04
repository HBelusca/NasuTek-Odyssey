#pragma once

//
// Kernel32 Filter IDs
//
#define kernel32file            200
#define kernel32ver             201
#define actctx                  202
#define resource                203
#define kernel32session         204


#if DBG
#define DEBUG_CHANNEL(ch) static ULONG gDebugChannel = ch;
#else
#define DEBUG_CHANNEL(ch)
#endif

#define TRACE(fmt, ...)         TRACE__(gDebugChannel, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)          WARN__(gDebugChannel, fmt, ##__VA_ARGS__)
#define FIXME(fmt, ...)         WARN__(gDebugChannel, fmt,## __VA_ARGS__)
#define ERR(fmt, ...)           ERR__(gDebugChannel, fmt, ##__VA_ARGS__)

#define STUB \
  SetLastError(ERROR_CALL_NOT_IMPLEMENTED); \
  DPRINT1("%s() is UNIMPLEMENTED!\n", __FUNCTION__)

#define debugstr_a  
#define debugstr_w
#define wine_dbgstr_w  
#define debugstr_guid

#include "wine/unicode.h"
#include "baseheap.h"

#define BINARY_UNKNOWN	(0)
#define BINARY_PE_EXE32	(1)
#define BINARY_PE_DLL32	(2)
#define BINARY_PE_EXE64	(3)
#define BINARY_PE_DLL64	(4)
#define BINARY_WIN16	(5)
#define BINARY_OS216	(6)
#define BINARY_DOS	(7)
#define BINARY_UNIX_EXE	(8)
#define BINARY_UNIX_LIB	(9)

#define  MAGIC(c1,c2,c3,c4)  ((c1) + ((c2)<<8) + ((c3)<<16) + ((c4)<<24))

#define  MAGIC_HEAP        MAGIC( 'H','E','A','P' )

#define ROUNDUP(a,b)	((((a)+(b)-1)/(b))*(b))
#define ROUNDDOWN(a,b)	(((a)/(b))*(b))

#define ROUND_DOWN(n, align) \
    (((ULONG)n) & ~((align) - 1l))

#define ROUND_UP(n, align) \
    ROUND_DOWN(((ULONG)n) + (align) - 1, (align))

#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type,fld)	((LONG)&(((type *)0)->fld))
#endif

#define IsConsoleHandle(h) \
  (((((ULONG_PTR)h) & 0x10000003) == 0x3) ? TRUE : FALSE)

#define HANDLE_DETACHED_PROCESS    (HANDLE)-2
#define HANDLE_CREATE_NEW_CONSOLE  (HANDLE)-3
#define HANDLE_CREATE_NO_WINDOW    (HANDLE)-4

/* Undocumented CreateProcess flag */
#define STARTF_SHELLPRIVATE         0x400
  
typedef struct _CODEPAGE_ENTRY
{
   LIST_ENTRY Entry;
   UINT CodePage;
   HANDLE SectionHandle;
   PBYTE SectionMapping;
   CPTABLEINFO CodePageTable;
} CODEPAGE_ENTRY, *PCODEPAGE_ENTRY;

extern PBASE_STATIC_SERVER_DATA BaseStaticServerData;

typedef
DWORD
(*WaitForInputIdleType)(
    HANDLE hProcess,
    DWORD dwMilliseconds);

/* GLOBAL VARIABLES **********************************************************/

extern BOOL bIsFileApiAnsi;
extern HANDLE hBaseDir;
extern HMODULE hCurrentModule;

extern RTL_CRITICAL_SECTION BaseDllDirectoryLock;

extern UNICODE_STRING BaseDllDirectory;
extern UNICODE_STRING BaseDefaultPath;
extern UNICODE_STRING BaseDefaultPathAppend;
extern PLDR_DATA_TABLE_ENTRY BasepExeLdrEntry;

extern LPTOP_LEVEL_EXCEPTION_FILTER GlobalTopLevelExceptionFilter;

extern SYSTEM_BASIC_INFORMATION BaseCachedSysInfo;

extern BOOLEAN BaseRunningInServerProcess;

/* FUNCTION PROTOTYPES *******************************************************/

VOID
NTAPI
BaseDllInitializeMemoryManager(VOID);

BOOL WINAPI VerifyConsoleIoHandle(HANDLE Handle);

BOOL WINAPI CloseConsoleHandle(HANDLE Handle);

HANDLE WINAPI
GetConsoleInputWaitHandle (VOID);

HANDLE WINAPI OpenConsoleW (LPCWSTR wsName,
			     DWORD  dwDesiredAccess,
			     BOOL   bInheritHandle,
			     DWORD  dwShareMode);

BOOL WINAPI SetConsoleInputExeNameW(LPCWSTR lpInputExeName);

PTEB GetTeb(VOID);

HANDLE FASTCALL TranslateStdHandle(HANDLE hHandle);

PWCHAR FilenameA2W(LPCSTR NameA, BOOL alloc);
DWORD FilenameW2A_N(LPSTR dest, INT destlen, LPCWSTR src, INT srclen);

DWORD FilenameW2A_FitOrFail(LPSTR  DestA, INT destLen, LPCWSTR SourceW, INT sourceLen);
DWORD FilenameU2A_FitOrFail(LPSTR  DestA, INT destLen, PUNICODE_STRING SourceU);

#define HeapAlloc RtlAllocateHeap
#define HeapReAlloc RtlReAllocateHeap
#define HeapFree RtlFreeHeap
#define _lread  (_readfun)_hread

PLARGE_INTEGER
WINAPI
BaseFormatTimeOut(OUT PLARGE_INTEGER Timeout,
                  IN DWORD dwMilliseconds);

POBJECT_ATTRIBUTES
WINAPI
BasepConvertObjectAttributes(OUT POBJECT_ATTRIBUTES ObjectAttributes,
                             IN PSECURITY_ATTRIBUTES SecurityAttributes OPTIONAL,
                             IN PUNICODE_STRING ObjectName);
                             
NTSTATUS
WINAPI
BasepCreateStack(HANDLE hProcess,
                 SIZE_T StackReserve,
                 SIZE_T StackCommit,
                 PINITIAL_TEB InitialTeb);
                 
VOID
WINAPI
BasepInitializeContext(IN PCONTEXT Context,
                       IN PVOID Parameter,
                       IN PVOID StartAddress,
                       IN PVOID StackAddress,
                       IN ULONG ContextType);
                
VOID
WINAPI
BaseThreadStartupThunk(VOID);

VOID
WINAPI
BaseProcessStartThunk(VOID);
        
__declspec(noreturn)
VOID
WINAPI
BaseThreadStartup(LPTHREAD_START_ROUTINE lpStartAddress,
                  LPVOID lpParameter);
                  
VOID
WINAPI
BasepFreeStack(HANDLE hProcess,
               PINITIAL_TEB InitialTeb);

__declspec(noreturn)
VOID
WINAPI
BaseFiberStartup(VOID);

typedef UINT (WINAPI *PPROCESS_START_ROUTINE)(VOID);

VOID
WINAPI
BaseProcessStartup(PPROCESS_START_ROUTINE lpStartAddress);
                  
BOOLEAN
WINAPI
BasepCheckRealTimePrivilege(VOID);

VOID
WINAPI
BasepAnsiStringToHeapUnicodeString(IN LPCSTR AnsiString,
                                   OUT LPWSTR* UnicodeString);
                                   
PUNICODE_STRING
WINAPI
Basep8BitStringToStaticUnicodeString(IN LPCSTR AnsiString);

BOOLEAN
WINAPI
Basep8BitStringToDynamicUnicodeString(OUT PUNICODE_STRING UnicodeString,
                                      IN LPCSTR String);
 
#define BasepUnicodeStringTo8BitString RtlUnicodeStringToAnsiString

typedef NTSTATUS (NTAPI *PRTL_CONVERT_STRING)(IN PUNICODE_STRING UnicodeString,
                                              IN PANSI_STRING AnsiString,
                                              IN BOOLEAN AllocateMemory);
                                                
extern PRTL_CONVERT_STRING Basep8BitStringToUnicodeString;

NTSTATUS
WINAPI
BasepMapFile(IN LPCWSTR lpApplicationName,
             OUT PHANDLE hSection,
             IN PUNICODE_STRING ApplicationName);

LPWSTR
WINAPI
BasepGetDllPath(LPWSTR FullPath,
                PVOID Environment);


PCODEPAGE_ENTRY FASTCALL
IntGetCodePageEntry(UINT CodePage);

LPWSTR
GetDllLoadPath(LPCWSTR lpModule);

VOID
WINAPI
InitCommandLines(VOID);

VOID
WINAPI
BaseSetLastNTError(IN NTSTATUS Status);

/* FIXME */
WCHAR WINAPI RtlAnsiCharToUnicodeChar(LPSTR *);

HANDLE
WINAPI
DuplicateConsoleHandle(HANDLE hConsole,
                       DWORD dwDesiredAccess,
                       BOOL	bInheritHandle,
                       DWORD dwOptions);

