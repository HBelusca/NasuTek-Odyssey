#pragma once

typedef struct _DESKTOP
{
    PDESKTOPINFO pDeskInfo;
    LIST_ENTRY ListEntry;
    /* Pointer to the associated window station. */
    struct _WINSTATION_OBJECT *rpwinstaParent;
    DWORD dwDTFlags;
    PWND spwndForeground;
    PWND spwndTray;
    PWND spwndMessage;
    PWND spwndTooltip;
    PSECTION_OBJECT hsectionDesktop;
    PWIN32HEAP pheapDesktop;
    ULONG_PTR ulHeapSize;
    LIST_ENTRY PtiList;
    /* use for tracking mouse moves. */
    PWND spwndTrack;
    DWORD htEx;
    RECT rcMouseHover;
    DWORD dwMouseHoverTime;

    /* Odyssey */
    /* Pointer to the active queue. */
    PVOID ActiveMessageQueue;
    /* Handle of the desktop window. */
    HWND DesktopWindow;
    /* Thread blocking input */
    PVOID BlockInputThread;
    LIST_ENTRY ShellHookWindows;
} DESKTOP;

// Desktop flags
#define DF_TME_HOVER        0x00000400
#define DF_TME_LEAVE        0x00000800
#define DF_DESTROYED        0x00008000
#define DF_DESKWNDDESTROYED 0x00010000
#define DF_DYING            0x00020000


extern PDESKTOP InputDesktop;
extern HDESK InputDesktopHandle;
extern PCLS DesktopWindowClass;
extern HDC ScreenDeviceContext;
extern BOOL g_PaintDesktopVersion;

typedef struct _SHELL_HOOK_WINDOW
{
  LIST_ENTRY ListEntry;
  HWND hWnd;
} SHELL_HOOK_WINDOW, *PSHELL_HOOK_WINDOW;

INIT_FUNCTION
NTSTATUS
NTAPI
InitDesktopImpl(VOID);

NTSTATUS FASTCALL
CleanupDesktopImpl(VOID);

NTSTATUS
APIENTRY
IntDesktopObjectParse(IN PVOID ParseObject,
                      IN PVOID ObjectType,
                      IN OUT PACCESS_STATE AccessState,
                      IN KPROCESSOR_MODE AccessMode,
                      IN ULONG Attributes,
                      IN OUT PUNICODE_STRING CompleteName,
                      IN OUT PUNICODE_STRING RemainingName,
                      IN OUT PVOID Context OPTIONAL,
                      IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
                      OUT PVOID *Object);

VOID APIENTRY
IntDesktopObjectDelete(PWIN32_DELETEMETHOD_PARAMETERS Parameters);

NTSTATUS NTAPI 
IntDesktopOkToClose(PWIN32_OKAYTOCLOSEMETHOD_PARAMETERS Parameters);

HDC FASTCALL
IntGetScreenDC(VOID);

HWND FASTCALL
IntGetDesktopWindow (VOID);

PWND FASTCALL
UserGetDesktopWindow(VOID);

HWND FASTCALL
IntGetCurrentThreadDesktopWindow(VOID);

PUSER_MESSAGE_QUEUE FASTCALL
IntGetFocusMessageQueue(VOID);

VOID FASTCALL
IntSetFocusMessageQueue(PUSER_MESSAGE_QUEUE NewQueue);

PDESKTOP FASTCALL
IntGetActiveDesktop(VOID);

NTSTATUS FASTCALL
co_IntShowDesktop(PDESKTOP Desktop, ULONG Width, ULONG Height);

NTSTATUS FASTCALL
IntHideDesktop(PDESKTOP Desktop);

BOOL IntSetThreadDesktop(IN HDESK hDesktop,
                         IN BOOL FreeOnFailure);

NTSTATUS FASTCALL
IntValidateDesktopHandle(
   HDESK Desktop,
   KPROCESSOR_MODE AccessMode,
   ACCESS_MASK DesiredAccess,
   PDESKTOP *Object);

NTSTATUS FASTCALL
IntParseDesktopPath(PEPROCESS Process,
                    PUNICODE_STRING DesktopPath,
                    HWINSTA *hWinSta,
                    HDESK *hDesktop);

BOOL FASTCALL IntDesktopUpdatePerUserSettings(BOOL bEnable);
VOID APIENTRY UserRedrawDesktop(VOID);
BOOL IntRegisterShellHookWindow(HWND hWnd);
BOOL IntDeRegisterShellHookWindow(HWND hWnd);
VOID co_IntShellHookNotify(WPARAM Message, LPARAM lParam);
HDC FASTCALL UserGetDesktopDC(ULONG,BOOL,BOOL);

#define IntIsActiveDesktop(Desktop) \
  ((Desktop)->rpwinstaParent->ActiveDesktop == (Desktop))

#define GET_DESKTOP_NAME(d)                                             \
    OBJECT_HEADER_TO_NAME_INFO(OBJECT_TO_OBJECT_HEADER(d)) ?            \
    &(OBJECT_HEADER_TO_NAME_INFO(OBJECT_TO_OBJECT_HEADER(d))->Name) :   \
    NULL

HWND FASTCALL IntGetMessageWindow(VOID);

static __inline PVOID
DesktopHeapAlloc(IN PDESKTOP Desktop,
                 IN SIZE_T Bytes)
{
    return RtlAllocateHeap(Desktop->pheapDesktop,
                           HEAP_NO_SERIALIZE,
                           Bytes);
}

static __inline BOOL
DesktopHeapFree(IN PDESKTOP Desktop,
                IN PVOID lpMem)
{
    return RtlFreeHeap(Desktop->pheapDesktop,
                       HEAP_NO_SERIALIZE,
                       lpMem);
}

static __inline PVOID
DesktopHeapReAlloc(IN PDESKTOP Desktop,
                   IN PVOID lpMem,
                   IN SIZE_T Bytes)
{
#if 0
    /* NOTE: ntoskrnl doesn't export RtlReAllocateHeap... */
    return RtlReAllocateHeap(Desktop->pheapDesktop,
                             HEAP_NO_SERIALIZE,
                             lpMem,
                             Bytes);
#else
    SIZE_T PrevSize;
    PVOID pNew;

    PrevSize = RtlSizeHeap(Desktop->pheapDesktop,
                           HEAP_NO_SERIALIZE,
                           lpMem);

    if (PrevSize == Bytes)
        return lpMem;

    pNew = RtlAllocateHeap(Desktop->pheapDesktop,
                           HEAP_NO_SERIALIZE,
                           Bytes);
    if (pNew != NULL)
    {
        if (PrevSize < Bytes)
            Bytes = PrevSize;

        RtlCopyMemory(pNew,
                      lpMem,
                      Bytes);

        RtlFreeHeap(Desktop->pheapDesktop,
                    HEAP_NO_SERIALIZE,
                    lpMem);
    }

    return pNew;
#endif
}

static __inline ULONG_PTR
DesktopHeapGetUserDelta(VOID)
{
    PW32HEAP_USER_MAPPING Mapping;
    PTHREADINFO pti;
    PPROCESSINFO W32Process;
    PWIN32HEAP pheapDesktop;
    ULONG_PTR Delta = 0;

    pti = PsGetCurrentThreadWin32Thread();
    if (!pti->rpdesk)
        return 0;

    pheapDesktop = pti->rpdesk->pheapDesktop;

    W32Process = PsGetCurrentProcessWin32Process();
    Mapping = W32Process->HeapMappings.Next;
    while (Mapping != NULL)
    {
        if (Mapping->KernelMapping == (PVOID)pheapDesktop)
        {
            Delta = (ULONG_PTR)Mapping->KernelMapping - (ULONG_PTR)Mapping->UserMapping;
            break;
        }

        Mapping = Mapping->Next;
    }

    return Delta;
}

static __inline PVOID
DesktopHeapAddressToUser(PVOID lpMem)
{
    PW32HEAP_USER_MAPPING Mapping;
    PPROCESSINFO W32Process;

    W32Process = PsGetCurrentProcessWin32Process();
    Mapping = W32Process->HeapMappings.Next;
    while (Mapping != NULL)
    {
        if ((ULONG_PTR)lpMem >= (ULONG_PTR)Mapping->KernelMapping &&
            (ULONG_PTR)lpMem < (ULONG_PTR)Mapping->KernelMapping + Mapping->Limit)
        {
            return (PVOID)(((ULONG_PTR)lpMem - (ULONG_PTR)Mapping->KernelMapping) +
                           (ULONG_PTR)Mapping->UserMapping);
        }

        Mapping = Mapping->Next;
    }

    return NULL;
}

/* EOF */
