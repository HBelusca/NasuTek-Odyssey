/*
* COPYRIGHT:        See COPYING in the top level directory
* PROJECT:          Odyssey kernel
* PURPOSE:          Messages
* FILE:             subsystems/win32/win32k/ntuser/message.c
* PROGRAMER:        Casper S. Hornstrup (chorns@users.sourceforge.net)
* REVISION HISTORY:
*       06-06-2001  CSH  Created
*/

/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserMsg);

BOOLEAN NTAPI PsGetProcessExitProcessCalled(PEPROCESS Process);

#define PM_BADMSGFLAGS ~((QS_RAWINPUT << 16)|PM_QS_SENDMESSAGE|PM_QS_PAINT|PM_QS_POSTMESSAGE|PM_QS_INPUT|PM_NOYIELD|PM_REMOVE)

/* FUNCTIONS *****************************************************************/

NTSTATUS FASTCALL
IntInitMessageImpl(VOID)
{
    return STATUS_SUCCESS;
}

NTSTATUS FASTCALL
IntCleanupMessageImpl(VOID)
{
    return STATUS_SUCCESS;
}

/* From wine: */
/* flag for messages that contain pointers */
/* 32 messages per entry, messages 0..31 map to bits 0..31 */

#define SET(msg) (1 << ((msg) & 31))

static const unsigned int message_pointer_flags[] =
{
    /* 0x00 - 0x1f */
    SET(WM_CREATE) | SET(WM_SETTEXT) | SET(WM_GETTEXT) |
    SET(WM_WININICHANGE) | SET(WM_DEVMODECHANGE),
    /* 0x20 - 0x3f */
    SET(WM_GETMINMAXINFO) | SET(WM_DRAWITEM) | SET(WM_MEASUREITEM) | SET(WM_DELETEITEM) |
    SET(WM_COMPAREITEM),
    /* 0x40 - 0x5f */
    SET(WM_WINDOWPOSCHANGING) | SET(WM_WINDOWPOSCHANGED) | SET(WM_COPYDATA) |
    SET(WM_COPYGLOBALDATA) | SET(WM_NOTIFY) | SET(WM_HELP),
    /* 0x60 - 0x7f */
    SET(WM_STYLECHANGING) | SET(WM_STYLECHANGED),
    /* 0x80 - 0x9f */
    SET(WM_NCCREATE) | SET(WM_NCCALCSIZE) | SET(WM_GETDLGCODE),
    /* 0xa0 - 0xbf */
    SET(EM_GETSEL) | SET(EM_GETRECT) | SET(EM_SETRECT) | SET(EM_SETRECTNP),
    /* 0xc0 - 0xdf */
    SET(EM_REPLACESEL) | SET(EM_GETLINE) | SET(EM_SETTABSTOPS),
    /* 0xe0 - 0xff */
    SET(SBM_GETRANGE) | SET(SBM_SETSCROLLINFO) | SET(SBM_GETSCROLLINFO) | SET(SBM_GETSCROLLBARINFO),
    /* 0x100 - 0x11f */
    0,
    /* 0x120 - 0x13f */
    0,
    /* 0x140 - 0x15f */
    SET(CB_GETEDITSEL) | SET(CB_ADDSTRING) | SET(CB_DIR) | SET(CB_GETLBTEXT) |
    SET(CB_INSERTSTRING) | SET(CB_FINDSTRING) | SET(CB_SELECTSTRING) |
    SET(CB_GETDROPPEDCONTROLRECT) | SET(CB_FINDSTRINGEXACT),
    /* 0x160 - 0x17f */
    0,
    /* 0x180 - 0x19f */
    SET(LB_ADDSTRING) | SET(LB_INSERTSTRING) | SET(LB_GETTEXT) | SET(LB_SELECTSTRING) |
    SET(LB_DIR) | SET(LB_FINDSTRING) |
    SET(LB_GETSELITEMS) | SET(LB_SETTABSTOPS) | SET(LB_ADDFILE) | SET(LB_GETITEMRECT),
    /* 0x1a0 - 0x1bf */
    SET(LB_FINDSTRINGEXACT),
    /* 0x1c0 - 0x1df */
    0,
    /* 0x1e0 - 0x1ff */
    0,
    /* 0x200 - 0x21f */
    SET(WM_NEXTMENU) | SET(WM_SIZING) | SET(WM_MOVING) | SET(WM_DEVICECHANGE),
    /* 0x220 - 0x23f */
    SET(WM_MDICREATE) | SET(WM_MDIGETACTIVE) | SET(WM_DROPOBJECT) |
    SET(WM_QUERYDROPOBJECT) | SET(WM_DRAGLOOP) | SET(WM_DRAGSELECT) | SET(WM_DRAGMOVE),
    /* 0x240 - 0x25f */
    0,
    /* 0x260 - 0x27f */
    0,
    /* 0x280 - 0x29f */
    0,
    /* 0x2a0 - 0x2bf */
    0,
    /* 0x2c0 - 0x2df */
    0,
    /* 0x2e0 - 0x2ff */
    0,
    /* 0x300 - 0x31f */
    SET(WM_ASKCBFORMATNAME)
};

/* check whether a given message type includes pointers */
static inline int is_pointer_message( UINT message )
{
    if (message >= 8*sizeof(message_pointer_flags)) return FALSE;
        return (message_pointer_flags[message / 32] & SET(message)) != 0;
}

#define MMS_SIZE_WPARAM      -1
#define MMS_SIZE_WPARAMWCHAR -2
#define MMS_SIZE_LPARAMSZ    -3
#define MMS_SIZE_SPECIAL     -4
#define MMS_FLAG_READ        0x01
#define MMS_FLAG_WRITE       0x02
#define MMS_FLAG_READWRITE   (MMS_FLAG_READ | MMS_FLAG_WRITE)
typedef struct tagMSGMEMORY
{
    UINT Message;
    UINT Size;
    INT Flags;
}
MSGMEMORY, *PMSGMEMORY;

static MSGMEMORY MsgMemory[] =
{
    { WM_CREATE, MMS_SIZE_SPECIAL, MMS_FLAG_READWRITE },
    { WM_DDE_ACK, sizeof(KMDDELPARAM), MMS_FLAG_READ },
    { WM_DDE_EXECUTE, MMS_SIZE_WPARAM, MMS_FLAG_READ },
    { WM_GETMINMAXINFO, sizeof(MINMAXINFO), MMS_FLAG_READWRITE },
    { WM_GETTEXT, MMS_SIZE_WPARAMWCHAR, MMS_FLAG_WRITE },
    { WM_NCCALCSIZE, MMS_SIZE_SPECIAL, MMS_FLAG_READWRITE },
    { WM_NCCREATE, MMS_SIZE_SPECIAL, MMS_FLAG_READWRITE },
    { WM_SETTEXT, MMS_SIZE_LPARAMSZ, MMS_FLAG_READ },
    { WM_STYLECHANGED, sizeof(STYLESTRUCT), MMS_FLAG_READ },
    { WM_STYLECHANGING, sizeof(STYLESTRUCT), MMS_FLAG_READWRITE },
    { WM_COPYDATA, MMS_SIZE_SPECIAL, MMS_FLAG_READ },
    { WM_COPYGLOBALDATA, MMS_SIZE_WPARAM, MMS_FLAG_READ },
    { WM_WINDOWPOSCHANGED, sizeof(WINDOWPOS), MMS_FLAG_READ },
    { WM_WINDOWPOSCHANGING, sizeof(WINDOWPOS), MMS_FLAG_READWRITE },
};

static PMSGMEMORY FASTCALL
FindMsgMemory(UINT Msg)
{
    PMSGMEMORY MsgMemoryEntry;

    /* See if this message type is present in the table */
    for (MsgMemoryEntry = MsgMemory;
    MsgMemoryEntry < MsgMemory + sizeof(MsgMemory) / sizeof(MSGMEMORY);
    MsgMemoryEntry++)
    {
        if (Msg == MsgMemoryEntry->Message)
        {
            return MsgMemoryEntry;
        }
    }

    return NULL;
}

static UINT FASTCALL
MsgMemorySize(PMSGMEMORY MsgMemoryEntry, WPARAM wParam, LPARAM lParam)
{
    CREATESTRUCTW *Cs;
    PUNICODE_STRING WindowName;
    PUNICODE_STRING ClassName;
    UINT Size = 0;

    _SEH2_TRY
    {
        if (MMS_SIZE_WPARAM == MsgMemoryEntry->Size)
        {
            Size = (UINT)wParam;
        }
        else if (MMS_SIZE_WPARAMWCHAR == MsgMemoryEntry->Size)
        {
            Size = (UINT) (wParam * sizeof(WCHAR));
        }
        else if (MMS_SIZE_LPARAMSZ == MsgMemoryEntry->Size)
        {
            Size = (UINT) ((wcslen((PWSTR) lParam) + 1) * sizeof(WCHAR));
        }
        else if (MMS_SIZE_SPECIAL == MsgMemoryEntry->Size)
        {
            switch(MsgMemoryEntry->Message)
            {
            case WM_CREATE:
            case WM_NCCREATE:
                Cs = (CREATESTRUCTW *) lParam;
                WindowName = (PUNICODE_STRING) Cs->lpszName;
                ClassName = (PUNICODE_STRING) Cs->lpszClass;
                Size = sizeof(CREATESTRUCTW) + WindowName->Length + sizeof(WCHAR);
                if (IS_ATOM(ClassName->Buffer))
                {
                    Size += sizeof(WCHAR) + sizeof(ATOM);
                }
                else
                {
                    Size += sizeof(WCHAR) + ClassName->Length + sizeof(WCHAR);
                }
                break;

            case WM_NCCALCSIZE:
                Size = wParam ? sizeof(NCCALCSIZE_PARAMS) + sizeof(WINDOWPOS) : sizeof(RECT);
                break;

            case WM_COPYDATA:
                Size = sizeof(COPYDATASTRUCT) + ((PCOPYDATASTRUCT)lParam)->cbData;
                break;

            default:
                ASSERT(FALSE);
                Size = 0;
                break;
            }
        }
        else
        {
            Size = MsgMemoryEntry->Size;
        }
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        ERR("Exception caught in MsgMemorySize()! Status: 0x%x\n", _SEH2_GetExceptionCode());
        Size = 0;
    }
    _SEH2_END;
    return Size;
}

static NTSTATUS
PackParam(LPARAM *lParamPacked, UINT Msg, WPARAM wParam, LPARAM lParam, BOOL NonPagedPoolNeeded)
{
    NCCALCSIZE_PARAMS *UnpackedNcCalcsize;
    NCCALCSIZE_PARAMS *PackedNcCalcsize;
    CREATESTRUCTW *UnpackedCs;
    CREATESTRUCTW *PackedCs;
    PLARGE_STRING WindowName;
    PUNICODE_STRING ClassName;
    POOL_TYPE PoolType;
    UINT Size;
    PCHAR CsData;

    *lParamPacked = lParam;

    if (NonPagedPoolNeeded)
        PoolType = NonPagedPool;
    else
        PoolType = PagedPool;

    if (WM_NCCALCSIZE == Msg && wParam)
    {

        UnpackedNcCalcsize = (NCCALCSIZE_PARAMS *) lParam;
        PackedNcCalcsize = ExAllocatePoolWithTag(PoolType,
        sizeof(NCCALCSIZE_PARAMS) + sizeof(WINDOWPOS),
        TAG_MSG);

        if (NULL == PackedNcCalcsize)
        {
            ERR("Not enough memory to pack lParam\n");
            return STATUS_NO_MEMORY;
        }
        RtlCopyMemory(PackedNcCalcsize, UnpackedNcCalcsize, sizeof(NCCALCSIZE_PARAMS));
        PackedNcCalcsize->lppos = (PWINDOWPOS) (PackedNcCalcsize + 1);
        RtlCopyMemory(PackedNcCalcsize->lppos, UnpackedNcCalcsize->lppos, sizeof(WINDOWPOS));
        *lParamPacked = (LPARAM) PackedNcCalcsize;
    }
    else if (WM_CREATE == Msg || WM_NCCREATE == Msg)
    {
        UnpackedCs = (CREATESTRUCTW *) lParam;
        WindowName = (PLARGE_STRING) UnpackedCs->lpszName;
        ClassName = (PUNICODE_STRING) UnpackedCs->lpszClass;
        Size = sizeof(CREATESTRUCTW) + WindowName->Length + sizeof(WCHAR);
        if (IS_ATOM(ClassName->Buffer))
        {
            Size += sizeof(WCHAR) + sizeof(ATOM);
        }
        else
        {
            Size += sizeof(WCHAR) + ClassName->Length + sizeof(WCHAR);
        }
        PackedCs = ExAllocatePoolWithTag(PoolType, Size, TAG_MSG);
        if (NULL == PackedCs)
        {
            ERR("Not enough memory to pack lParam\n");
            return STATUS_NO_MEMORY;
        }
        RtlCopyMemory(PackedCs, UnpackedCs, sizeof(CREATESTRUCTW));
        CsData = (PCHAR) (PackedCs + 1);
        PackedCs->lpszName = (LPCWSTR) (CsData - (PCHAR) PackedCs);
        RtlCopyMemory(CsData, WindowName->Buffer, WindowName->Length);
        CsData += WindowName->Length;
        *((WCHAR *) CsData) = L'\0';
        CsData += sizeof(WCHAR);
        PackedCs->lpszClass = (LPCWSTR) (CsData - (PCHAR) PackedCs);
        if (IS_ATOM(ClassName->Buffer))
        {
            *((WCHAR *) CsData) = L'A';
            CsData += sizeof(WCHAR);
            *((ATOM *) CsData) = (ATOM)(DWORD_PTR) ClassName->Buffer;
            CsData += sizeof(ATOM);
        }
        else
        {
            *((WCHAR *) CsData) = L'S';
            CsData += sizeof(WCHAR);
            RtlCopyMemory(CsData, ClassName->Buffer, ClassName->Length);
            CsData += ClassName->Length;
            *((WCHAR *) CsData) = L'\0';
            CsData += sizeof(WCHAR);
        }
        ASSERT(CsData == (PCHAR) PackedCs + Size);
        *lParamPacked = (LPARAM) PackedCs;
    }
    else if (PoolType == NonPagedPool)
    {
        PMSGMEMORY MsgMemoryEntry;
        PVOID PackedData;
        SIZE_T size;

        MsgMemoryEntry = FindMsgMemory(Msg);

        if ((!MsgMemoryEntry) || (MsgMemoryEntry->Size < 0))
        {
            /* Keep previous behavior */
            return STATUS_SUCCESS;
        }
        size = MsgMemorySize(MsgMemoryEntry, wParam, lParam);
        if (!size)
        {
           ERR("No size for lParamPacked\n");
           return STATUS_SUCCESS;
        }
        PackedData = ExAllocatePoolWithTag(NonPagedPool, size, TAG_MSG);
        RtlCopyMemory(PackedData, (PVOID)lParam, MsgMemorySize(MsgMemoryEntry, wParam, lParam));
        *lParamPacked = (LPARAM)PackedData;
    }

    return STATUS_SUCCESS;
}

static NTSTATUS
UnpackParam(LPARAM lParamPacked, UINT Msg, WPARAM wParam, LPARAM lParam, BOOL NonPagedPoolUsed)
{
    NCCALCSIZE_PARAMS *UnpackedParams;
    NCCALCSIZE_PARAMS *PackedParams;
    PWINDOWPOS UnpackedWindowPos;

    if (lParamPacked == lParam)
    {
        return STATUS_SUCCESS;
    }

    if (WM_NCCALCSIZE == Msg && wParam)
    {
        PackedParams = (NCCALCSIZE_PARAMS *) lParamPacked;
        UnpackedParams = (NCCALCSIZE_PARAMS *) lParam;
        UnpackedWindowPos = UnpackedParams->lppos;
        RtlCopyMemory(UnpackedParams, PackedParams, sizeof(NCCALCSIZE_PARAMS));
        UnpackedParams->lppos = UnpackedWindowPos;
        RtlCopyMemory(UnpackedWindowPos, PackedParams + 1, sizeof(WINDOWPOS));
        ExFreePool((PVOID) lParamPacked);

        return STATUS_SUCCESS;
    }
    else if (WM_CREATE == Msg || WM_NCCREATE == Msg)
    {
        ExFreePool((PVOID) lParamPacked);

        return STATUS_SUCCESS;
    }
    else if (NonPagedPoolUsed)
    {
        PMSGMEMORY MsgMemoryEntry;
        MsgMemoryEntry = FindMsgMemory(Msg);
        if (MsgMemoryEntry->Size < 0)
        {
            /* Keep previous behavior */
            return STATUS_INVALID_PARAMETER;
        }

        if (MsgMemory->Flags == MMS_FLAG_READWRITE)
        {
            //RtlCopyMemory((PVOID)lParam, (PVOID)lParamPacked, MsgMemory->Size);
        }
        ExFreePool((PVOID) lParamPacked);
        return STATUS_SUCCESS;
    }

    ASSERT(FALSE);

    return STATUS_INVALID_PARAMETER;
}

static NTSTATUS FASTCALL
CopyMsgToKernelMem(MSG *KernelModeMsg, MSG *UserModeMsg, PMSGMEMORY MsgMemoryEntry)
{
    NTSTATUS Status;

    PVOID KernelMem;
    UINT Size;

    *KernelModeMsg = *UserModeMsg;

    /* See if this message type is present in the table */
    if (NULL == MsgMemoryEntry)
    {
        /* Not present, no copying needed */
        return STATUS_SUCCESS;
    }

    /* Determine required size */
    Size = MsgMemorySize(MsgMemoryEntry, UserModeMsg->wParam, UserModeMsg->lParam);

    if (0 != Size)
    {
        /* Allocate kernel mem */
        KernelMem = ExAllocatePoolWithTag(PagedPool, Size, TAG_MSG);
        if (NULL == KernelMem)
        {
            ERR("Not enough memory to copy message to kernel mem\n");
            return STATUS_NO_MEMORY;
        }
        KernelModeMsg->lParam = (LPARAM) KernelMem;

        /* Copy data if required */
        if (0 != (MsgMemoryEntry->Flags & MMS_FLAG_READ))
        {
            Status = MmCopyFromCaller(KernelMem, (PVOID) UserModeMsg->lParam, Size);
            if (! NT_SUCCESS(Status))
            {
                ERR("Failed to copy message to kernel: invalid usermode buffer\n");
                ExFreePoolWithTag(KernelMem, TAG_MSG);
                return Status;
            }
        }
        else
        {
            /* Make sure we don't pass any secrets to usermode */
            RtlZeroMemory(KernelMem, Size);
        }
    }
    else
    {
        KernelModeMsg->lParam = 0;
    }

    return STATUS_SUCCESS;
}

static NTSTATUS FASTCALL
CopyMsgToUserMem(MSG *UserModeMsg, MSG *KernelModeMsg)
{
    NTSTATUS Status;
    PMSGMEMORY MsgMemoryEntry;
    UINT Size;

    /* See if this message type is present in the table */
    MsgMemoryEntry = FindMsgMemory(UserModeMsg->message);
    if (NULL == MsgMemoryEntry)
    {
        /* Not present, no copying needed */
        return STATUS_SUCCESS;
    }

    /* Determine required size */
    Size = MsgMemorySize(MsgMemoryEntry, UserModeMsg->wParam, UserModeMsg->lParam);

    if (0 != Size)
    {
        /* Copy data if required */
        if (0 != (MsgMemoryEntry->Flags & MMS_FLAG_WRITE))
        {
            Status = MmCopyToCaller((PVOID) UserModeMsg->lParam, (PVOID) KernelModeMsg->lParam, Size);
            if (! NT_SUCCESS(Status))
            {
                ERR("Failed to copy message from kernel: invalid usermode buffer\n");
                ExFreePool((PVOID) KernelModeMsg->lParam);
                return Status;
            }
        }

        ExFreePool((PVOID) KernelModeMsg->lParam);
    }

    return STATUS_SUCCESS;
}

//
// Wakeup any thread/process waiting on idle input.
//
VOID FASTCALL
IdlePing(VOID)
{
   PPROCESSINFO ppi = PsGetCurrentProcessWin32Process();
   PUSER_MESSAGE_QUEUE ForegroundQueue;
   PTHREADINFO pti, ptiForeground = NULL;

   ForegroundQueue = IntGetFocusMessageQueue();

   if (ForegroundQueue)
      ptiForeground = ForegroundQueue->Thread->Tcb.Win32Thread;

   pti = PsGetCurrentThreadWin32Thread();

   if ( pti )
   {
      pti->pClientInfo->cSpins = 0; // Reset spins.

      if ( pti->pDeskInfo && pti == ptiForeground )
      {
         if ( pti->fsHooks & HOOKID_TO_FLAG(WH_FOREGROUNDIDLE) ||
              pti->pDeskInfo->fsHooks & HOOKID_TO_FLAG(WH_FOREGROUNDIDLE) )
         {
            co_HOOK_CallHooks(WH_FOREGROUNDIDLE,HC_ACTION,0,0);
         }
      }
   }

   TRACE("IdlePing ppi 0x%x\n",ppi);
   if ( ppi && ppi->InputIdleEvent )
   {
      TRACE("InputIdleEvent\n");
      KeSetEvent( ppi->InputIdleEvent, IO_NO_INCREMENT, FALSE);
   }
}

VOID FASTCALL
IdlePong(VOID)
{
   PPROCESSINFO ppi = PsGetCurrentProcessWin32Process();

   TRACE("IdlePong ppi 0x%x\n",ppi);
   if ( ppi && ppi->InputIdleEvent )
   {
      KeClearEvent(ppi->InputIdleEvent);
   }
}

UINT FASTCALL
GetWakeMask(UINT first, UINT last )
{
    UINT mask = QS_POSTMESSAGE | QS_SENDMESSAGE;  /* Always selected */

    if (first || last)
    {
        if ((first <= WM_KEYLAST) && (last >= WM_KEYFIRST)) mask |= QS_KEY;
        if ( ((first <= WM_MOUSELAST) && (last >= WM_MOUSEFIRST)) ||
             ((first <= WM_NCMOUSELAST) && (last >= WM_NCMOUSEFIRST)) ) mask |= QS_MOUSE;
        if ((first <= WM_TIMER) && (last >= WM_TIMER)) mask |= QS_TIMER;
        if ((first <= WM_SYSTIMER) && (last >= WM_SYSTIMER)) mask |= QS_TIMER;
        if ((first <= WM_PAINT) && (last >= WM_PAINT)) mask |= QS_PAINT;
    }
    else mask = QS_ALLINPUT;

    return mask;
}

static VOID FASTCALL
IntCallWndProc( PWND Window, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    BOOL SameThread = FALSE;
    CWPSTRUCT CWP;

    if (Window->head.pti == ((PTHREADINFO)PsGetCurrentThreadWin32Thread()))
        SameThread = TRUE;

    CWP.hwnd    = hWnd;
    CWP.message = Msg;
    CWP.wParam  = wParam;
    CWP.lParam  = lParam;
    co_HOOK_CallHooks( WH_CALLWNDPROC, HC_ACTION, SameThread, (LPARAM)&CWP );
}

static VOID FASTCALL
IntCallWndProcRet ( PWND Window, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam, LRESULT *uResult)
{
    BOOL SameThread = FALSE;
    CWPRETSTRUCT CWPR;

    if (Window->head.pti == ((PTHREADINFO)PsGetCurrentThreadWin32Thread()))
        SameThread = TRUE;

    CWPR.hwnd    = hWnd;
    CWPR.message = Msg;
    CWPR.wParam  = wParam;
    CWPR.lParam  = lParam;
    CWPR.lResult = *uResult;
    co_HOOK_CallHooks( WH_CALLWNDPROCRET, HC_ACTION, SameThread, (LPARAM)&CWPR );
}

LRESULT FASTCALL
IntDispatchMessage(PMSG pMsg)
{
    LARGE_INTEGER TickCount;
    LONG Time;
    LRESULT retval = 0;
    PTHREADINFO pti;
    PWND Window = NULL;

    if (pMsg->hwnd)
    {
        Window = UserGetWindowObject(pMsg->hwnd);
        if (!Window) return 0;
    }

    pti = PsGetCurrentThreadWin32Thread();

    if ( Window->head.pti != pti)
    {
       EngSetLastError( ERROR_MESSAGE_SYNC_ONLY );
       return 0;
    }

    if (((pMsg->message == WM_SYSTIMER) ||
         (pMsg->message == WM_TIMER)) &&
         (pMsg->lParam) )
    {
        if (pMsg->message == WM_TIMER)
        {
            if (ValidateTimerCallback(pti,pMsg->lParam))
            {
                KeQueryTickCount(&TickCount);
                Time = MsqCalculateMessageTime(&TickCount);
                retval = co_IntCallWindowProc((WNDPROC)pMsg->lParam,
                                              TRUE,
                                              pMsg->hwnd,
                                              WM_TIMER,
                                              pMsg->wParam,
                                              (LPARAM)Time,
                                              0);
            }
            return retval;
        }
        else
        {
            PTIMER pTimer = FindSystemTimer(pMsg);
            if (pTimer && pTimer->pfn)
            {
                KeQueryTickCount(&TickCount);
                Time = MsqCalculateMessageTime(&TickCount);
                pTimer->pfn(pMsg->hwnd, WM_SYSTIMER, (UINT)pMsg->wParam, Time);
            }
            return 0;
        }
    }
    // Need a window!
    if ( !Window ) return 0;

    /* Since we are doing a callback on the same thread right away, there is
       no need to copy the lparam to kernel mode and then back to usermode.
       We just pretend it isn't a pointer */

    retval = co_IntCallWindowProc( Window->lpfnWndProc,
                                   !Window->Unicode,
                                   pMsg->hwnd,
                                   pMsg->message,
                                   pMsg->wParam,
                                   pMsg->lParam,
                                   0);

    if (pMsg->message == WM_PAINT)
    {
        /* send a WM_NCPAINT and WM_ERASEBKGND if the non-client area is still invalid */
        HRGN hrgn = IntSysCreateRectRgn( 0, 0, 0, 0 );
        co_UserGetUpdateRgn( Window, hrgn, TRUE );
        GreDeleteObject(hrgn);
    }

    return retval;
}

/*
 * Internal version of PeekMessage() doing all the work
 */
BOOL FASTCALL
co_IntPeekMessage( PMSG Msg,
                   PWND Window,
                   UINT MsgFilterMin,
                   UINT MsgFilterMax,
                   UINT RemoveMsg,
                   BOOL bGMSG )
{
    PTHREADINFO pti;
    PCLIENTINFO pci;
    LARGE_INTEGER LargeTickCount;
    PUSER_MESSAGE_QUEUE ThreadQueue;
    BOOL RemoveMessages;
    UINT ProcessMask;
    BOOL Hit = FALSE;

    pti = PsGetCurrentThreadWin32Thread();
    ThreadQueue = pti->MessageQueue;
    pci = pti->pClientInfo;

    RemoveMessages = RemoveMsg & PM_REMOVE;
    ProcessMask = HIWORD(RemoveMsg);

 /* Hint, "If wMsgFilterMin and wMsgFilterMax are both zero, PeekMessage returns
    all available messages (that is, no range filtering is performed)".        */
    if (!ProcessMask) ProcessMask = (QS_ALLPOSTMESSAGE|QS_ALLINPUT);

    IdlePong();

    do
    {
        KeQueryTickCount(&LargeTickCount);
        ThreadQueue->LastMsgRead = LargeTickCount.u.LowPart;
        pti->pcti->tickLastMsgChecked = LargeTickCount.u.LowPart;

        /* Dispatch sent messages here. */
        while ( co_MsqDispatchOneSentMessage(ThreadQueue) )
        {
           /* if some PM_QS* flags were specified, only handle sent messages from now on */
           if (HIWORD(RemoveMsg) && !bGMSG) Hit = TRUE; // wine does this; ProcessMask = QS_SENDMESSAGE;
        }
        if (Hit) return FALSE;

        /* Clear changed bits so we can wait on them if we don't find a message */
        if (ProcessMask & QS_POSTMESSAGE)
        {
           pti->pcti->fsChangeBits &= ~(QS_POSTMESSAGE | QS_HOTKEY | QS_TIMER);
           if (MsgFilterMin == 0 && MsgFilterMax == 0) // wine hack does this; ~0U)
           {
              pti->pcti->fsChangeBits &= ~QS_ALLPOSTMESSAGE;
           }
        }

        if (ProcessMask & QS_INPUT)
        {
           pti->pcti->fsChangeBits &= ~QS_INPUT;
        }

        /* Now check for normal messages. */
        if (( (ProcessMask & QS_POSTMESSAGE) ||
              (ProcessMask & QS_HOTKEY) ) &&
            MsqPeekMessage( ThreadQueue,
                            RemoveMessages,
                            Window,
                            MsgFilterMin,
                            MsgFilterMax,
                            ProcessMask,
                            Msg ))
        {
               return TRUE;
        }

        /* Now look for a quit message. */
        if (ThreadQueue->QuitPosted)
        {
            /* According to the PSDK, WM_QUIT messages are always returned, regardless
               of the filter specified */
            Msg->hwnd = NULL;
            Msg->message = WM_QUIT;
            Msg->wParam = ThreadQueue->QuitExitCode;
            Msg->lParam = 0;
            if (RemoveMessages)
            {
                ThreadQueue->QuitPosted = FALSE;
                ClearMsgBitsMask(ThreadQueue, QS_POSTMESSAGE);
                pti->pcti->fsWakeBits &= ~QS_ALLPOSTMESSAGE;
                pti->pcti->fsChangeBits &= ~QS_ALLPOSTMESSAGE;
            }
            return TRUE;
        }

        /* Check for hardware events. */
        if ((ProcessMask & QS_MOUSE) &&
            co_MsqPeekMouseMove( ThreadQueue,
                                 RemoveMessages,
                                 Window,
                                 MsgFilterMin,
                                 MsgFilterMax,
                                 Msg ))
        {
            return TRUE;
        }

        if ((ProcessMask & QS_INPUT) &&
            co_MsqPeekHardwareMessage( ThreadQueue,
                                       RemoveMessages,
                                       Window,
                                       MsgFilterMin,
                                       MsgFilterMax,
                                       ProcessMask,
                                       Msg))
        {
            return TRUE;
        }

        /* Check for sent messages again. */
        while ( co_MsqDispatchOneSentMessage(ThreadQueue) )
        {
           if (HIWORD(RemoveMsg) && !bGMSG) Hit = TRUE;
        }
        if (Hit) return FALSE;

        /* Check for paint messages. */
        if ((ProcessMask & QS_PAINT) &&
            pti->cPaintsReady &&
            IntGetPaintMessage( Window,
                                MsgFilterMin,
                                MsgFilterMax,
                                pti,
                                Msg,
                                RemoveMessages))
        {
            return TRUE;
        }

       /* This is correct, check for the current threads timers waiting to be
          posted to this threads message queue. If any we loop again.
        */
        if ((ProcessMask & QS_TIMER) &&
            PostTimerMessages(Window))
        {
            continue;
        }

        return FALSE;
    }
    while (TRUE);

    return TRUE;
}

static BOOL FASTCALL
co_IntWaitMessage( PWND Window,
                   UINT MsgFilterMin,
                   UINT MsgFilterMax )
{
    PTHREADINFO pti;
    PUSER_MESSAGE_QUEUE ThreadQueue;
    NTSTATUS Status = STATUS_SUCCESS;
    MSG Msg;

    pti = PsGetCurrentThreadWin32Thread();
    ThreadQueue = pti->MessageQueue;

    do
    {
        if ( co_IntPeekMessage( &Msg,       // Dont reenter!
                                 Window,
                                 MsgFilterMin,
                                 MsgFilterMax,
                                 MAKELONG( PM_NOREMOVE, GetWakeMask( MsgFilterMin, MsgFilterMax)),
                                 TRUE ) )   // act like GetMessage.
        {
            return TRUE;
        }

        /* Nothing found. Wait for new messages. */
        Status = co_MsqWaitForNewMessages( ThreadQueue,
                                           Window,
                                           MsgFilterMin,
                                           MsgFilterMax);
        if (!NT_SUCCESS(Status))
        {
            SetLastNtError(Status);
            ERR("Exit co_IntWaitMessage on error!\n");
            return FALSE;
        }
        if (Status == STATUS_USER_APC || Status == STATUS_TIMEOUT)
        {
           return FALSE;
        }
    }
    while ( TRUE );

    return FALSE;
}

BOOL FASTCALL
co_IntGetPeekMessage( PMSG pMsg,
                      HWND hWnd,
                      UINT MsgFilterMin,
                      UINT MsgFilterMax,
                      UINT RemoveMsg,
                      BOOL bGMSG )
{
    PWND Window;
    PTHREADINFO pti;
    BOOL Present = FALSE;
    NTSTATUS Status;

    if ( hWnd == HWND_TOPMOST || hWnd == HWND_BROADCAST )
        hWnd = HWND_BOTTOM;

    /* Validate input */
    if (hWnd && hWnd != HWND_BOTTOM)
    {
        if (!(Window = UserGetWindowObject(hWnd)))
        {
            if (bGMSG)
                return -1;
            else
                return FALSE;
        }
    }
    else
    {
        Window = (PWND)hWnd;
    }

    if (MsgFilterMax < MsgFilterMin)
    {
        MsgFilterMin = 0;
        MsgFilterMax = 0;
    }

    if (bGMSG)
    {
       RemoveMsg |= ((GetWakeMask( MsgFilterMin, MsgFilterMax ))<< 16);
    }

    pti = PsGetCurrentThreadWin32Thread();
    pti->pClientInfo->cSpins++; // Bump up the spin count.

    do
    {
        Present = co_IntPeekMessage( pMsg,
                                     Window,
                                     MsgFilterMin,
                                     MsgFilterMax,
                                     RemoveMsg,
                                     bGMSG );
        if (Present)
        {
           /* GetMessage or PostMessage must never get messages that contain pointers */
           ASSERT(FindMsgMemory(pMsg->message) == NULL);

           if (pMsg->message != WM_PAINT && pMsg->message != WM_QUIT)
           {
              pti->timeLast = pMsg->time;
              pti->ptLast   = pMsg->pt;
           }

           // The WH_GETMESSAGE hook enables an application to monitor messages about to
           // be returned by the GetMessage or PeekMessage function.

           co_HOOK_CallHooks( WH_GETMESSAGE, HC_ACTION, RemoveMsg & PM_REMOVE, (LPARAM)pMsg);

           if ( bGMSG ) break;
        }

        if ( bGMSG )
        {
            Status = co_MsqWaitForNewMessages( pti->MessageQueue,
                                               Window,
                                               MsgFilterMin,
                                               MsgFilterMax);
           if ( !NT_SUCCESS(Status) ||
                Status == STATUS_USER_APC ||
                Status == STATUS_TIMEOUT )
           {
              Present = -1;
              break;
           }
        }
        else
        {
           if (!(RemoveMsg & PM_NOYIELD))
           {
              IdlePing();
              // Yield this thread!
              UserLeave();
              ZwYieldExecution();
              UserEnterExclusive();
              // Fall through to exit.
              IdlePong();
           }
           break;
        }
    }
    while( bGMSG && !Present );

    // Been spinning, time to swap vinyl...
    if (pti->pClientInfo->cSpins >= 100)
    {
       // Clear the spin cycle to fix the mix.
       pti->pClientInfo->cSpins = 0;
       //if (!(pti->TIF_flags & TIF_SPINNING)) FIXME need to swap vinyl..
    }
    return Present;
}

BOOL FASTCALL
UserPostThreadMessage( DWORD idThread,
                       UINT Msg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    MSG Message;
    PETHREAD peThread;
    PTHREADINFO pThread;
    LARGE_INTEGER LargeTickCount;
    NTSTATUS Status;

    if (is_pointer_message(Msg))
    {
        EngSetLastError(ERROR_MESSAGE_SYNC_ONLY );
        return FALSE;
    }

    Status = PsLookupThreadByThreadId((HANDLE)idThread,&peThread);

    if( Status == STATUS_SUCCESS )
    {
        pThread = (PTHREADINFO)peThread->Tcb.Win32Thread;
        if( !pThread ||
            !pThread->MessageQueue ||
            (pThread->TIF_flags & TIF_INCLEANUP))
        {
            ObDereferenceObject( peThread );
            return FALSE;
        }

        Message.hwnd = NULL;
        Message.message = Msg;
        Message.wParam = wParam;
        Message.lParam = lParam;
        Message.pt = gpsi->ptCursor;

        KeQueryTickCount(&LargeTickCount);
        Message.time = MsqCalculateMessageTime(&LargeTickCount);
        MsqPostMessage(pThread->MessageQueue, &Message, FALSE, QS_POSTMESSAGE);
        ObDereferenceObject( peThread );
        return TRUE;
    }
    else
    {
        SetLastNtError( Status );
    }
    return FALSE;
}

BOOL FASTCALL
UserPostMessage( HWND Wnd,
                 UINT Msg,
                 WPARAM wParam,
                 LPARAM lParam )
{
    PTHREADINFO pti;
    MSG Message, KernelModeMsg;
    LARGE_INTEGER LargeTickCount;

    Message.hwnd = Wnd;
    Message.message = Msg;
    Message.wParam = wParam;
    Message.lParam = lParam;
    Message.pt = gpsi->ptCursor;
    KeQueryTickCount(&LargeTickCount);
    Message.time = MsqCalculateMessageTime(&LargeTickCount);

    if (is_pointer_message(Message.message))
    {
        EngSetLastError(ERROR_MESSAGE_SYNC_ONLY );
        return FALSE;
    }

    if( Msg >= WM_DDE_FIRST && Msg <= WM_DDE_LAST )
    {
        NTSTATUS Status;
        PMSGMEMORY MsgMemoryEntry;

        MsgMemoryEntry = FindMsgMemory(Message.message);

        Status = CopyMsgToKernelMem(&KernelModeMsg, &Message, MsgMemoryEntry);
        if (! NT_SUCCESS(Status))
        {
            EngSetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
        co_IntSendMessageNoWait(KernelModeMsg.hwnd,
                                KernelModeMsg.message,
                                KernelModeMsg.wParam,
                                KernelModeMsg.lParam);

        if (MsgMemoryEntry && KernelModeMsg.lParam)
            ExFreePool((PVOID) KernelModeMsg.lParam);

        return TRUE;
    }

    if (!Wnd)
    {
        return UserPostThreadMessage( PtrToInt(PsGetCurrentThreadId()),
                                      Msg,
                                      wParam,
                                      lParam);
    }
    if (Wnd == HWND_BROADCAST)
    {
        HWND *List;
        PWND DesktopWindow;
        ULONG i;

        DesktopWindow = UserGetWindowObject(IntGetDesktopWindow());
        List = IntWinListChildren(DesktopWindow);

        if (List != NULL)
        {
            UserPostMessage(DesktopWindow->head.h, Msg, wParam, lParam);
            for (i = 0; List[i]; i++)
            {
                UserPostMessage(List[i], Msg, wParam, lParam);
            }
            ExFreePoolWithTag(List, USERTAG_WINDOWLIST);
        }
    }
    else
    {
        PWND Window;

        Window = UserGetWindowObject(Wnd);
        if ( !Window )
        {
            return FALSE;
        }

        pti = Window->head.pti;
        if ( pti->TIF_flags & TIF_INCLEANUP )
        {
            ERR("Attempted to post message to window 0x%x when the thread is in cleanup!\n", Wnd);
            return FALSE;
        }

        if ( Window->state & WNDS_DESTROYED )
        {
            ERR("Attempted to post message to window 0x%x that is being destroyed!\n", Wnd);
            /* FIXME - last error code? */
            return FALSE;
        }

        if (WM_QUIT == Msg)
        {
            MsqPostQuitMessage(Window->head.pti->MessageQueue, wParam);
        }
        else
        {
            MsqPostMessage(Window->head.pti->MessageQueue, &Message, FALSE, QS_POSTMESSAGE);
        }
    }
    return TRUE;
}


LRESULT FASTCALL
co_IntSendMessage( HWND hWnd,
                   UINT Msg,
                   WPARAM wParam,
                   LPARAM lParam )
{
    ULONG_PTR Result = 0;
    if(co_IntSendMessageTimeout(hWnd, Msg, wParam, lParam, SMTO_NORMAL, 0, &Result))
    {
        return (LRESULT)Result;
    }
    return 0;
}

static LRESULT FASTCALL
co_IntSendMessageTimeoutSingle( HWND hWnd,
                                UINT Msg,
                                WPARAM wParam,
                                LPARAM lParam,
                                UINT uFlags,
                                UINT uTimeout,
                                ULONG_PTR *uResult )
{
    NTSTATUS Status;
    PWND Window = NULL;
    PMSGMEMORY MsgMemoryEntry;
    INT lParamBufferSize;
    LPARAM lParamPacked;
    PTHREADINFO Win32Thread;
    ULONG_PTR Result = 0;
    DECLARE_RETURN(LRESULT);
    USER_REFERENCE_ENTRY Ref;

    if (!(Window = UserGetWindowObject(hWnd)))
    {
        RETURN( FALSE);
    }

    UserRefObjectCo(Window, &Ref);

    Win32Thread = PsGetCurrentThreadWin32Thread();

    IntCallWndProc( Window, hWnd, Msg, wParam, lParam);

    if ( NULL != Win32Thread &&
         Window->head.pti->MessageQueue == Win32Thread->MessageQueue)
    {
        if (Win32Thread->TIF_flags & TIF_INCLEANUP)
        {
            /* Never send messages to exiting threads */
            RETURN( FALSE);
        }

        /* See if this message type is present in the table */
        MsgMemoryEntry = FindMsgMemory(Msg);
        if (NULL == MsgMemoryEntry)
        {
            lParamBufferSize = -1;
        }
        else
        {
            lParamBufferSize = MsgMemorySize(MsgMemoryEntry, wParam, lParam);
        }

        if (! NT_SUCCESS(PackParam(&lParamPacked, Msg, wParam, lParam, FALSE)))
        {
            ERR("Failed to pack message parameters\n");
            RETURN( FALSE);
        }

        Result = (ULONG_PTR)co_IntCallWindowProc( Window->lpfnWndProc,
                                                  !Window->Unicode,
                                                  hWnd,
                                                  Msg,
                                                  wParam,
                                                  lParamPacked,
                                                  lParamBufferSize );
        if(uResult)
        {
            *uResult = Result;
        }

        IntCallWndProcRet( Window, hWnd, Msg, wParam, lParam, (LRESULT *)uResult);

        if (! NT_SUCCESS(UnpackParam(lParamPacked, Msg, wParam, lParam, FALSE)))
        {
            ERR("Failed to unpack message parameters\n");
            RETURN( TRUE);
        }

        RETURN( TRUE);
    }

    if (uFlags & SMTO_ABORTIFHUNG && MsqIsHung(Window->head.pti->MessageQueue))
    {
        // FIXME - Set window hung and add to a list.
        /* FIXME - Set a LastError? */
        RETURN( FALSE);
    }

    if (Window->state & WNDS_DESTROYED)
    {
        /* FIXME - last error? */
        ERR("Attempted to send message to window 0x%x that is being destroyed!\n", hWnd);
        RETURN( FALSE);
    }

    do
    {
        Status = co_MsqSendMessage( Window->head.pti->MessageQueue,
                                    hWnd,
                                    Msg,
                                    wParam,
                                    lParam,
                                    uTimeout,
                                    (uFlags & SMTO_BLOCK),
                                    MSQ_NORMAL,
                                    uResult );
    }
    while ((STATUS_TIMEOUT == Status) &&
           (uFlags & SMTO_NOTIMEOUTIFNOTHUNG) &&
           !MsqIsHung(Window->head.pti->MessageQueue)); // FIXME - Set window hung and add to a list.

    IntCallWndProcRet( Window, hWnd, Msg, wParam, lParam, (LRESULT *)uResult);

    if (STATUS_TIMEOUT == Status)
    {
/*
    MSDN says:
    Microsoft Windows 2000: If GetLastError returns zero, then the function
    timed out.
    XP+ : If the function fails or times out, the return value is zero.
    To get extended error information, call GetLastError. If GetLastError
    returns ERROR_TIMEOUT, then the function timed out.
*/
        EngSetLastError(ERROR_TIMEOUT);
        RETURN( FALSE);
    }
    else if (! NT_SUCCESS(Status))
    {
        SetLastNtError(Status);
        RETURN( FALSE);
    }

    RETURN( TRUE);

CLEANUP:
    if (Window) UserDerefObjectCo(Window);
    END_CLEANUP;
}

LRESULT FASTCALL
co_IntSendMessageTimeout( HWND hWnd,
                          UINT Msg,
                          WPARAM wParam,
                          LPARAM lParam,
                          UINT uFlags,
                          UINT uTimeout,
                          ULONG_PTR *uResult )
{
    PWND DesktopWindow;
    HWND *Children;
    HWND *Child;

    if (HWND_BROADCAST != hWnd)
    {
        return co_IntSendMessageTimeoutSingle(hWnd, Msg, wParam, lParam, uFlags, uTimeout, uResult);
    }

    DesktopWindow = UserGetWindowObject(IntGetDesktopWindow());
    if (NULL == DesktopWindow)
    {
        EngSetLastError(ERROR_INTERNAL_ERROR);
        return 0;
    }

    /* Send message to the desktop window too! */
    co_IntSendMessageTimeoutSingle(DesktopWindow->head.h, Msg, wParam, lParam, uFlags, uTimeout, uResult);

    Children = IntWinListChildren(DesktopWindow);
    if (NULL == Children)
    {
        return 0;
    }

    for (Child = Children; NULL != *Child; Child++)
    {
        co_IntSendMessageTimeoutSingle(*Child, Msg, wParam, lParam, uFlags, uTimeout, uResult);
    }

    ExFreePool(Children);

    return (LRESULT) TRUE;
}

LRESULT FASTCALL
co_IntSendMessageNoWait(HWND hWnd,
                        UINT Msg,
                        WPARAM wParam,
                        LPARAM lParam)
{
    ULONG_PTR Result = 0;
    co_IntSendMessageWithCallBack(hWnd,
                                  Msg,
                                  wParam,
                                  lParam,
                                  NULL,
                                  0,
                                  &Result);
    return Result;
}
/* MSDN:
   If you send a message in the range below WM_USER to the asynchronous message
   functions (PostMessage, SendNotifyMessage, and SendMessageCallback), its
   message parameters cannot include pointers. Otherwise, the operation will fail.
   The functions will return before the receiving thread has had a chance to
   process the message and the sender will free the memory before it is used.
*/
LRESULT FASTCALL
co_IntSendMessageWithCallBack( HWND hWnd,
                              UINT Msg,
                              WPARAM wParam,
                              LPARAM lParam,
                              SENDASYNCPROC CompletionCallback,
                              ULONG_PTR CompletionCallbackContext,
                              ULONG_PTR *uResult)
{
    ULONG_PTR Result;
    PWND Window = NULL;
    PMSGMEMORY MsgMemoryEntry;
    INT lParamBufferSize;
    LPARAM lParamPacked;
    PTHREADINFO Win32Thread;
    DECLARE_RETURN(LRESULT);
    USER_REFERENCE_ENTRY Ref;
    PUSER_SENT_MESSAGE Message;

    if (!(Window = UserGetWindowObject(hWnd)))
    {
        RETURN(FALSE);
    }

    UserRefObjectCo(Window, &Ref);

    if (Window->state & WNDS_DESTROYED)
    {
        /* FIXME - last error? */
        ERR("Attempted to send message to window 0x%x that is being destroyed!\n", hWnd);
        RETURN(FALSE);
    }

    Win32Thread = PsGetCurrentThreadWin32Thread();

    if (Win32Thread == NULL)
    {
        RETURN(FALSE);
    }

    /* See if this message type is present in the table */
    MsgMemoryEntry = FindMsgMemory(Msg);
    if (NULL == MsgMemoryEntry)
    {
        lParamBufferSize = -1;
    }
    else
    {
        lParamBufferSize = MsgMemorySize(MsgMemoryEntry, wParam, lParam);
    }

    if (! NT_SUCCESS(PackParam(&lParamPacked, Msg, wParam, lParam, Window->head.pti->MessageQueue != Win32Thread->MessageQueue)))
    {
        ERR("Failed to pack message parameters\n");
        RETURN( FALSE);
    }

    /* If it can be sent now, then send it. */
    if (Window->head.pti->MessageQueue == Win32Thread->MessageQueue)
    {
        if (Win32Thread->TIF_flags & TIF_INCLEANUP)
        {
            UnpackParam(lParamPacked, Msg, wParam, lParam, FALSE);
            /* Never send messages to exiting threads */
            RETURN(FALSE);
        }

        IntCallWndProc( Window, hWnd, Msg, wParam, lParam);

        Result = (ULONG_PTR)co_IntCallWindowProc( Window->lpfnWndProc,
                                                  !Window->Unicode,
                                                  hWnd,
                                                  Msg,
                                                  wParam,
                                                  lParamPacked,
                                                  lParamBufferSize );
        if(uResult)
        {
            *uResult = Result;
        }

        IntCallWndProcRet( Window, hWnd, Msg, wParam, lParam, (LRESULT *)uResult);

        if (CompletionCallback)
        {
            co_IntCallSentMessageCallback(CompletionCallback,
                                          hWnd,
                                          Msg,
                                          CompletionCallbackContext,
                                          Result);
        }
    }



    if (Window->head.pti->MessageQueue == Win32Thread->MessageQueue)
    {
        if (! NT_SUCCESS(UnpackParam(lParamPacked, Msg, wParam, lParam, FALSE)))
        {
            ERR("Failed to unpack message parameters\n");
        }
        RETURN(TRUE);
    }

    if(!(Message = ExAllocatePoolWithTag(NonPagedPool, sizeof(USER_SENT_MESSAGE), TAG_USRMSG)))
    {
        ERR("MsqSendMessage(): Not enough memory to allocate a message");
        RETURN( FALSE);
    }

    IntReferenceMessageQueue(Window->head.pti->MessageQueue);
    /* Take reference on this MessageQueue if its a callback. It will be released
       when message is processed or removed from target hwnd MessageQueue */
    if (CompletionCallback)
       IntReferenceMessageQueue(Win32Thread->MessageQueue);

    Message->Msg.hwnd = hWnd;
    Message->Msg.message = Msg;
    Message->Msg.wParam = wParam;
    Message->Msg.lParam = lParamPacked;
    Message->CompletionEvent = NULL;
    Message->Result = 0;
    Message->lResult = 0;
    Message->QS_Flags = 0;
    Message->SenderQueue = NULL; // mjmartin, you are right! This is null.
    Message->CallBackSenderQueue = Win32Thread->MessageQueue;
    Message->DispatchingListEntry.Flink = NULL;
    Message->CompletionCallback = CompletionCallback;
    Message->CompletionCallbackContext = CompletionCallbackContext;
    Message->HookMessage = MSQ_NORMAL;
    Message->HasPackedLParam = (lParamBufferSize > 0);
    Message->QS_Flags = QS_SENDMESSAGE;

    InsertTailList(&Window->head.pti->MessageQueue->SentMessagesListHead, &Message->ListEntry);
    MsqWakeQueue(Window->head.pti->MessageQueue, QS_SENDMESSAGE, TRUE);
    IntDereferenceMessageQueue(Window->head.pti->MessageQueue);

    RETURN(TRUE);

CLEANUP:
    if (Window) UserDerefObjectCo(Window);
    END_CLEANUP;
}

/* This function posts a message if the destination's message queue belongs to
another thread, otherwise it sends the message. It does not support broadcast
messages! */
LRESULT FASTCALL
co_IntPostOrSendMessage( HWND hWnd,
                         UINT Msg,
                         WPARAM wParam,
                         LPARAM lParam )
{
    ULONG_PTR Result;
    PTHREADINFO pti;
    PWND Window;

    if ( hWnd == HWND_BROADCAST )
    {
        return 0;
    }

    if(!(Window = UserGetWindowObject(hWnd)))
    {
        return 0;
    }

    pti = PsGetCurrentThreadWin32Thread();

    if ( Window->head.pti->MessageQueue != pti->MessageQueue &&
         FindMsgMemory(Msg) == 0 )
    {
        Result = UserPostMessage(hWnd, Msg, wParam, lParam);
    }
    else
    {
        if ( !co_IntSendMessageTimeoutSingle(hWnd, Msg, wParam, lParam, SMTO_NORMAL, 0, &Result) )
        {
            Result = 0;
        }
    }

    return (LRESULT)Result;
}

LRESULT FASTCALL
co_IntDoSendMessage( HWND hWnd,
                     UINT Msg,
                     WPARAM wParam,
                     LPARAM lParam,
                     PDOSENDMESSAGE dsm)
{
    PTHREADINFO pti;
    LRESULT Result = TRUE;
    NTSTATUS Status;
    PWND Window = NULL;
    MSG UserModeMsg;
    MSG KernelModeMsg;
    PMSGMEMORY MsgMemoryEntry;

    if (HWND_BROADCAST != hWnd)
    {
        Window = UserGetWindowObject(hWnd);
        if ( !Window )
        {
            return 0;
        }
    }

    /* Check for an exiting window. */
    if (Window && Window->state & WNDS_DESTROYED)
    {
        ERR("co_IntDoSendMessage Window Exiting!\n");
    }

    /* See if the current thread can handle the message */
    pti = PsGetCurrentThreadWin32Thread();

    UserModeMsg.hwnd = hWnd;
    UserModeMsg.message = Msg;
    UserModeMsg.wParam = wParam;
    UserModeMsg.lParam = lParam;
    MsgMemoryEntry = FindMsgMemory(UserModeMsg.message);

    Status = CopyMsgToKernelMem(&KernelModeMsg, &UserModeMsg, MsgMemoryEntry);
    if (! NT_SUCCESS(Status))
    {
       EngSetLastError(ERROR_INVALID_PARAMETER);
       return (dsm ? 0 : -1);
    }

    if (!dsm)
    {
       Result = co_IntSendMessage( KernelModeMsg.hwnd,
                                   KernelModeMsg.message,
                                   KernelModeMsg.wParam,
                                   KernelModeMsg.lParam );
    }
    else
    {
       Result = co_IntSendMessageTimeout( KernelModeMsg.hwnd,
                                          KernelModeMsg.message,
                                          KernelModeMsg.wParam,
                                          KernelModeMsg.lParam,
                                          dsm->uFlags,
                                          dsm->uTimeout,
                                         &dsm->Result );
    }

    Status = CopyMsgToUserMem(&UserModeMsg, &KernelModeMsg);
    if (! NT_SUCCESS(Status))
    {
       EngSetLastError(ERROR_INVALID_PARAMETER);
       return(dsm ? 0 : -1);
    }

    return (LRESULT)Result;
}

BOOL FASTCALL
UserSendNotifyMessage( HWND hWnd,
                       UINT Msg,
                       WPARAM wParam,
                       LPARAM lParam )
{
    BOOL Ret = TRUE;

    if (is_pointer_message(Msg))
    {
        EngSetLastError(ERROR_MESSAGE_SYNC_ONLY );
        return FALSE;
    }

    // Basicly the same as IntPostOrSendMessage
    if (hWnd == HWND_BROADCAST) //Handle Broadcast
    {
        HWND *List;
        PWND DesktopWindow;
        ULONG i;

        DesktopWindow = UserGetWindowObject(IntGetDesktopWindow());
        List = IntWinListChildren(DesktopWindow);

        if (List != NULL)
        {
            UserSendNotifyMessage(DesktopWindow->head.h, Msg, wParam, lParam);
            for (i = 0; List[i]; i++)
            {
                Ret = UserSendNotifyMessage(List[i], Msg, wParam, lParam);
            }
            ExFreePool(List);
        }
    }
    else
    {
        ULONG_PTR lResult = 0;
        Ret = co_IntSendMessageWithCallBack( hWnd,
                                             Msg,
                                             wParam,
                                             lParam,
                                             NULL,
                                             0,
                                            &lResult);
    }
    return Ret;
}


DWORD APIENTRY
IntGetQueueStatus(DWORD Changes)
{
    PTHREADINFO pti;
    PUSER_MESSAGE_QUEUE Queue;
    DWORD Result;

    pti = PsGetCurrentThreadWin32Thread();
    Queue = pti->MessageQueue;
// wine:
    Changes &= (QS_ALLINPUT|QS_ALLPOSTMESSAGE|QS_SMRESULT);

    /* High word, types of messages currently in the queue.
       Low  word, types of messages that have been added to the queue and that
                  are still in the queue
     */
    Result = MAKELONG(pti->pcti->fsChangeBits & Changes, pti->pcti->fsWakeBits & Changes);

    pti->pcti->fsChangeBits &= ~Changes;

    return Result;
}

BOOL APIENTRY
IntInitMessagePumpHook()
{
    PTHREADINFO pti = PsGetCurrentThreadWin32Thread();

    if (pti->pcti)
    {
        pti->pcti->dwcPumpHook++;
        return TRUE;
    }
    return FALSE;
}

BOOL APIENTRY
IntUninitMessagePumpHook()
{
    PTHREADINFO pti = PsGetCurrentThreadWin32Thread();

    if (pti->pcti)
    {
        if (pti->pcti->dwcPumpHook <= 0)
        {
            return FALSE;
        }
        pti->pcti->dwcPumpHook--;
        return TRUE;
    }
    return FALSE;
}

/** Functions ******************************************************************/

BOOL
APIENTRY
NtUserDragDetect(
   HWND hWnd,
   POINT pt) // Just like the User call.
{
    MSG msg;
    RECT rect;
    WORD wDragWidth, wDragHeight;
    DECLARE_RETURN(BOOL);

    TRACE("Enter NtUserDragDetect(%x)\n", hWnd);
    UserEnterExclusive();

    wDragWidth = UserGetSystemMetrics(SM_CXDRAG);
    wDragHeight= UserGetSystemMetrics(SM_CYDRAG);

    rect.left = pt.x - wDragWidth;
    rect.right = pt.x + wDragWidth;

    rect.top = pt.y - wDragHeight;
    rect.bottom = pt.y + wDragHeight;

    co_UserSetCapture(hWnd);

    for (;;)
    {
        while (co_IntGetPeekMessage( &msg, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE, FALSE ) ||
               co_IntGetPeekMessage( &msg, 0, WM_QUEUESYNC,  WM_QUEUESYNC, PM_REMOVE, FALSE ) ||
               co_IntGetPeekMessage( &msg, 0, WM_KEYFIRST,   WM_KEYLAST,   PM_REMOVE, FALSE ) )
        {
            if ( msg.message == WM_LBUTTONUP )
            {
                co_UserSetCapture(NULL);
                RETURN( FALSE);
            }
            if ( msg.message == WM_MOUSEMOVE )
            {
                POINT tmp;
                tmp.x = (short)LOWORD(msg.lParam);
                tmp.y = (short)HIWORD(msg.lParam);
                if( !RECTL_bPointInRect( &rect, tmp.x, tmp.y ) )
                {
                    co_UserSetCapture(NULL);
                    RETURN( TRUE);
                }
            }
            if ( msg.message == WM_KEYDOWN )
            {
                if ( msg.wParam == VK_ESCAPE )
                {
                   co_UserSetCapture(NULL);
                   RETURN( TRUE);
                }
            }
            if ( msg.message == WM_QUEUESYNC )
            {
                co_HOOK_CallHooks( WH_CBT, HCBT_QS, 0, 0 );
            }
        }
        co_IntWaitMessage(NULL, 0, 0);
    }
    RETURN( FALSE);

CLEANUP:
   TRACE("Leave NtUserDragDetect, ret=%i\n",_ret_);
   UserLeave();
   END_CLEANUP;
}

BOOL APIENTRY
NtUserPostMessage(HWND hWnd,
                  UINT Msg,
                  WPARAM wParam,
                  LPARAM lParam)
{
    BOOL ret;

    UserEnterExclusive();

    ret = UserPostMessage(hWnd, Msg, wParam, lParam);

    UserLeave();

    return ret;
}

BOOL APIENTRY
NtUserPostThreadMessage(DWORD idThread,
                        UINT Msg,
                        WPARAM wParam,
                        LPARAM lParam)
{
    BOOL ret;

    UserEnterExclusive();

    ret = UserPostThreadMessage( idThread, Msg, wParam, lParam);

    UserLeave();

    return ret;
}

BOOL APIENTRY
NtUserWaitMessage(VOID)
{
    BOOL ret;

    UserEnterExclusive();
    TRACE("NtUserWaitMessage Enter\n");
    ret = co_IntWaitMessage(NULL, 0, 0);
    TRACE("NtUserWaitMessage Leave\n");
    UserLeave();

    return ret;
}

BOOL APIENTRY
NtUserGetMessage(PMSG pMsg,
                  HWND hWnd,
                  UINT MsgFilterMin,
                  UINT MsgFilterMax )
{
    MSG Msg;
    BOOL Ret;

    if ( (MsgFilterMin|MsgFilterMax) & ~WM_MAXIMUM )
    {
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    UserEnterExclusive();

    RtlZeroMemory(&Msg, sizeof(MSG));

    Ret = co_IntGetPeekMessage(&Msg, hWnd, MsgFilterMin, MsgFilterMax, PM_REMOVE, TRUE);

    UserLeave();

    if (Ret == TRUE)
    {
        _SEH2_TRY
        {
            ProbeForWrite(pMsg, sizeof(MSG), 1);
            RtlCopyMemory(pMsg, &Msg, sizeof(MSG));
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            SetLastNtError(_SEH2_GetExceptionCode());
            Ret = FALSE;
        }
        _SEH2_END;
    }

    if ((INT)Ret != -1)
       Ret = Ret ? (WM_QUIT != pMsg->message) : FALSE;

    return Ret;
}

BOOL APIENTRY
NtUserPeekMessage( PMSG pMsg,
                  HWND hWnd,
                  UINT MsgFilterMin,
                  UINT MsgFilterMax,
                  UINT RemoveMsg)
{
    MSG Msg;
    BOOL Ret;

    if ( RemoveMsg & PM_BADMSGFLAGS )
    {
        EngSetLastError(ERROR_INVALID_FLAGS);
        return FALSE;
    }

    UserEnterExclusive();

    RtlZeroMemory(&Msg, sizeof(MSG));

    Ret = co_IntGetPeekMessage(&Msg, hWnd, MsgFilterMin, MsgFilterMax, RemoveMsg, FALSE);

    UserLeave();

    if (Ret)
    {
        _SEH2_TRY
        {
            ProbeForWrite(pMsg, sizeof(MSG), 1);
            RtlCopyMemory(pMsg, &Msg, sizeof(MSG));
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            SetLastNtError(_SEH2_GetExceptionCode());
            Ret = FALSE;
        }
        _SEH2_END;
    }

    return Ret;
}

BOOL APIENTRY
NtUserCallMsgFilter( LPMSG lpmsg, INT code)
{
    BOOL Ret = FALSE;
    MSG Msg;

    _SEH2_TRY
    {
        ProbeForRead(lpmsg, sizeof(MSG), 1);
        RtlCopyMemory( &Msg, lpmsg, sizeof(MSG));
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        _SEH2_YIELD(return FALSE);
    }
    _SEH2_END;

    UserEnterExclusive();

    if ( co_HOOK_CallHooks( WH_SYSMSGFILTER, code, 0, (LPARAM)&Msg))
    {
        Ret = TRUE;
    }
    else
    {
        Ret = co_HOOK_CallHooks( WH_MSGFILTER, code, 0, (LPARAM)&Msg);
    }

    UserLeave();

    _SEH2_TRY
    {
        ProbeForWrite(lpmsg, sizeof(MSG), 1);
        RtlCopyMemory(lpmsg, &Msg, sizeof(MSG));
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        Ret = FALSE;
    }
    _SEH2_END;

    return Ret;
}

LRESULT APIENTRY
NtUserDispatchMessage(PMSG UnsafeMsgInfo)
{
    LRESULT Res = 0;
    MSG SafeMsg;

    _SEH2_TRY
    {
        ProbeForRead(UnsafeMsgInfo, sizeof(MSG), 1);
        RtlCopyMemory(&SafeMsg, UnsafeMsgInfo, sizeof(MSG));
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastNtError(_SEH2_GetExceptionCode());
        _SEH2_YIELD(return FALSE);
    }
    _SEH2_END;

    UserEnterExclusive();

    Res = IntDispatchMessage(&SafeMsg);

    UserLeave();
    return Res;
}


BOOL APIENTRY
NtUserTranslateMessage(LPMSG lpMsg, UINT flags)
{
    MSG SafeMsg;
    BOOL Ret;

    _SEH2_TRY
    {
        ProbeForRead(lpMsg, sizeof(MSG), 1);
        RtlCopyMemory(&SafeMsg, lpMsg, sizeof(MSG));
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastNtError(_SEH2_GetExceptionCode());
        _SEH2_YIELD(return FALSE);
    }
    _SEH2_END;

    UserEnterExclusive();

    Ret = IntTranslateKbdMessage(&SafeMsg, flags);

    UserLeave();

    return Ret;
}

BOOL APIENTRY
NtUserMessageCall( HWND hWnd,
                   UINT Msg,
                   WPARAM wParam,
                   LPARAM lParam,
                   ULONG_PTR ResultInfo,
                   DWORD dwType, // fnID?
                   BOOL Ansi)
{
    LRESULT lResult = 0;
    BOOL Ret = FALSE;
    PWND Window = NULL;
    USER_REFERENCE_ENTRY Ref;

    UserEnterExclusive();

    switch(dwType)
    {
    case FNID_DEFWINDOWPROC:
        /* Validate input */
        if (hWnd && (hWnd != INVALID_HANDLE_VALUE))
        {
           Window = UserGetWindowObject(hWnd);
           if (!Window)
           {
               UserLeave();
               return FALSE;
           }
        }
        UserRefObjectCo(Window, &Ref);
        lResult = IntDefWindowProc(Window, Msg, wParam, lParam, Ansi);
        Ret = TRUE;
        UserDerefObjectCo(Window);
        break;
    case FNID_SENDNOTIFYMESSAGE:
        Ret = UserSendNotifyMessage(hWnd, Msg, wParam, lParam);
        break;
    case FNID_BROADCASTSYSTEMMESSAGE:
        {
            BROADCASTPARM parm;
            DWORD_PTR RetVal = 0;

            if (ResultInfo)
            {
                _SEH2_TRY
                {
                    ProbeForWrite((PVOID)ResultInfo, sizeof(BROADCASTPARM), 1);
                    RtlCopyMemory(&parm, (PVOID)ResultInfo, sizeof(BROADCASTPARM));
                }
                _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
                {
                    _SEH2_YIELD(break);
                }
                _SEH2_END;
            }
            else
                break;

            if ( parm.recipients & BSM_ALLDESKTOPS ||
                    parm.recipients == BSM_ALLCOMPONENTS )
            {
            }
            else if (parm.recipients & BSM_APPLICATIONS)
            {
                if (parm.flags & BSF_QUERY)
                {
                    if (parm.flags & BSF_FORCEIFHUNG || parm.flags & BSF_NOHANG)
                    {
                        co_IntSendMessageTimeout( HWND_BROADCAST,
                                                  Msg,
                                                  wParam,
                                                  lParam,
                                                  SMTO_ABORTIFHUNG,
                                                  2000,
                                                 &RetVal);
                    }
                    else if (parm.flags & BSF_NOTIMEOUTIFNOTHUNG)
                    {
                        co_IntSendMessageTimeout( HWND_BROADCAST,
                                                  Msg,
                                                  wParam,
                                                  lParam,
                                                  SMTO_NOTIMEOUTIFNOTHUNG,
                                                  2000,
                                                 &RetVal);
                    }
                    else
                    {
                        co_IntSendMessageTimeout( HWND_BROADCAST,
                                                  Msg,
			                          wParam,
                                                  lParam,
                                                  SMTO_NORMAL,
                                                  2000,
                                                 &RetVal);
                    }
                    Ret = RetVal;
                }
                else if (parm.flags & BSF_POSTMESSAGE)
                {
                    Ret = UserPostMessage(HWND_BROADCAST, Msg, wParam, lParam);
                }
                else //Everything else,,,, if ( parm.flags & BSF_SENDNOTIFYMESSAGE)
                {
                    Ret = UserSendNotifyMessage(HWND_BROADCAST, Msg, wParam, lParam);
                }
            }
        }
        break;
    case FNID_SENDMESSAGECALLBACK:
        {
            CALL_BACK_INFO CallBackInfo;
            ULONG_PTR uResult;

            _SEH2_TRY
            {
                ProbeForRead((PVOID)ResultInfo, sizeof(CALL_BACK_INFO), 1);
                RtlCopyMemory(&CallBackInfo, (PVOID)ResultInfo, sizeof(CALL_BACK_INFO));
            }
            _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
            {
                _SEH2_YIELD(break);
            }
            _SEH2_END;

            if (is_pointer_message(Msg))
            {
               EngSetLastError(ERROR_MESSAGE_SYNC_ONLY );
               break;
            }

            if (!(Ret = co_IntSendMessageWithCallBack(hWnd, Msg, wParam, lParam,
                        CallBackInfo.CallBack, CallBackInfo.Context, &uResult)))
            {
                ERR("Callback failure!\n");
            }
        }
        break;
    case FNID_SENDMESSAGE:
        {
            Ret = co_IntDoSendMessage(hWnd, Msg, wParam, lParam, 0);

            if (ResultInfo)
            {
                _SEH2_TRY
                {
                    ProbeForWrite((PVOID)ResultInfo, sizeof(ULONG_PTR), 1);
                    RtlCopyMemory((PVOID)ResultInfo, &Ret, sizeof(ULONG_PTR));
                }
                _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
                {
                    Ret = FALSE;
                    _SEH2_YIELD(break);
                }
                _SEH2_END;
            }
            break;
        }
    case FNID_SENDMESSAGETIMEOUT:
        {
            DOSENDMESSAGE dsm, *pdsm = (PDOSENDMESSAGE)ResultInfo;
            if (ResultInfo)
            {
                _SEH2_TRY
                {
                    ProbeForRead(pdsm, sizeof(DOSENDMESSAGE), 1);
                    RtlCopyMemory(&dsm, pdsm, sizeof(DOSENDMESSAGE));
                }
                _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
                {
                    _SEH2_YIELD(break);
                }
                _SEH2_END;
            }

            Ret = co_IntDoSendMessage( hWnd, Msg, wParam, lParam, &dsm );

            if (pdsm)
            {
                _SEH2_TRY
                {
                    ProbeForWrite(pdsm, sizeof(DOSENDMESSAGE), 1);
                    RtlCopyMemory(pdsm, &dsm, sizeof(DOSENDMESSAGE));
                }
                _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
                {
                    Ret = FALSE;
                    _SEH2_YIELD(break);
                }
                _SEH2_END;
            }
            break;
        }
        // CallNextHook bypass.
    case FNID_CALLWNDPROC:
    case FNID_CALLWNDPROCRET:
        {
            PTHREADINFO pti;
            PCLIENTINFO ClientInfo;
            PHOOK NextObj, Hook;

            pti = GetW32ThreadInfo();

            Hook = pti->sphkCurrent;

            if (!Hook) break;

            NextObj = Hook->phkNext;
            ClientInfo = pti->pClientInfo;
            _SEH2_TRY
            {
                ClientInfo->phkCurrent = NextObj;
            }
            _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
            {
                ClientInfo = NULL;
            }
            _SEH2_END;

            if (!ClientInfo || !NextObj) break;

            NextObj->phkNext = IntGetNextHook(NextObj);

            if ( Hook->HookId == WH_CALLWNDPROC)
            {
                CWPSTRUCT CWP;
                CWP.hwnd    = hWnd;
                CWP.message = Msg;
                CWP.wParam  = wParam;
                CWP.lParam  = lParam;
                TRACE("WH_CALLWNDPROC: Hook %x NextHook %x\n", Hook, NextObj );

                lResult = co_IntCallHookProc( Hook->HookId,
                                              HC_ACTION,
                                              ((ClientInfo->CI_flags & CI_CURTHPRHOOK) ? 1 : 0),
                                              (LPARAM)&CWP,
                                              Hook->Proc,
                                              Hook->Ansi,
                                              &Hook->ModuleName);
            }
            else
            {
                CWPRETSTRUCT CWPR;
                CWPR.hwnd    = hWnd;
                CWPR.message = Msg;
                CWPR.wParam  = wParam;
                CWPR.lParam  = lParam;
                CWPR.lResult = ClientInfo->dwHookData;

                lResult = co_IntCallHookProc( Hook->HookId,
                                              HC_ACTION,
                                              ((ClientInfo->CI_flags & CI_CURTHPRHOOK) ? 1 : 0),
                                              (LPARAM)&CWPR,
                                              Hook->Proc,
                                              Hook->Ansi,
                                              &Hook->ModuleName);
            }
        }
        break;
    }

    switch(dwType)
    {
    case FNID_DEFWINDOWPROC:
    case FNID_CALLWNDPROC:
    case FNID_CALLWNDPROCRET:
        if (ResultInfo)
        {
            _SEH2_TRY
            {
                ProbeForWrite((PVOID)ResultInfo, sizeof(LRESULT), 1);
                RtlCopyMemory((PVOID)ResultInfo, &lResult, sizeof(LRESULT));
            }
            _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
            {
                Ret = FALSE;
            }
            _SEH2_END;
        }
        break;
    default:
        break;
    }

    UserLeave();

    return Ret;
}

#define INFINITE 0xFFFFFFFF
#define WAIT_FAILED ((DWORD)0xFFFFFFFF)

DWORD
APIENTRY
NtUserWaitForInputIdle( IN HANDLE hProcess,
                        IN DWORD dwMilliseconds,
                        IN BOOL Unknown2)
{
    PEPROCESS Process;
    PPROCESSINFO W32Process;
    PTHREADINFO pti;
    NTSTATUS Status;
    HANDLE Handles[3];
    LARGE_INTEGER Timeout;

    UserEnterExclusive();

    Status = ObReferenceObjectByHandle(hProcess,
                                       PROCESS_QUERY_INFORMATION,
                                       PsProcessType,
                                       UserMode,
                                       (PVOID*)&Process,
                                       NULL);

    if (!NT_SUCCESS(Status))
    {
        UserLeave();
        SetLastNtError(Status);
        return WAIT_FAILED;
    }

    pti = PsGetCurrentThreadWin32Thread();

    W32Process = (PPROCESSINFO)Process->Win32Process;

    if ( PsGetProcessExitProcessCalled(Process) ||
         !W32Process ||
         pti->ppi == W32Process)
    {
        ObDereferenceObject(Process);
        UserLeave();
        EngSetLastError(ERROR_INVALID_PARAMETER);
        return WAIT_FAILED;
    }

    Handles[0] = Process;
    Handles[1] = W32Process->InputIdleEvent;
    Handles[2] = pti->MessageQueue->NewMessages; // pEventQueueServer; IntMsqSetWakeMask returns hEventQueueClient

    if (!Handles[1])
    {
        ObDereferenceObject(Process);
        UserLeave();
        return STATUS_SUCCESS;  /* no event to wait on */
    }

    if (dwMilliseconds != INFINITE)
       Timeout.QuadPart = (LONGLONG) dwMilliseconds * (LONGLONG) -10000;

    W32Process->W32PF_flags |= W32PF_WAITFORINPUTIDLE;
    for (pti = W32Process->ptiList; pti; pti = pti->ptiSibling)
    {
       pti->TIF_flags |= TIF_WAITFORINPUTIDLE;
       pti->pClientInfo->dwTIFlags = pti->TIF_flags;
    }

    TRACE("WFII: ppi 0x%x\n",W32Process);
    TRACE("WFII: waiting for %p\n", Handles[1] );
    do
    {
        UserLeave();
        Status = KeWaitForMultipleObjects( 3,
                                           Handles,
                                           WaitAny,
                                           UserRequest,
                                           UserMode,
                                           FALSE,
                                           dwMilliseconds == INFINITE ? NULL : &Timeout,
                                           NULL);
        UserEnterExclusive();

        if (!NT_SUCCESS(Status))
        {
            SetLastNtError(Status);
            Status = WAIT_FAILED;
            goto WaitExit;
        }

        switch (Status)
        {
        case STATUS_WAIT_0:
            goto WaitExit;

        case STATUS_WAIT_2:
            {
               MSG Msg;
               co_IntGetPeekMessage( &Msg, 0, 0, 0, PM_REMOVE | PM_QS_SENDMESSAGE, FALSE);
               ERR("WFII: WAIT 2\n");
            }
            break;

        case STATUS_TIMEOUT:
            ERR("WFII: timeout\n");
        case WAIT_FAILED:
            goto WaitExit;

        default:
            ERR("WFII: finished\n");
            Status = STATUS_SUCCESS;
            goto WaitExit;
        }
    }
    while (TRUE);

WaitExit:
    for (pti = W32Process->ptiList; pti; pti = pti->ptiSibling)
    {
       pti->TIF_flags &= ~TIF_WAITFORINPUTIDLE;
       pti->pClientInfo->dwTIFlags = pti->TIF_flags;
    }
    W32Process->W32PF_flags &= ~W32PF_WAITFORINPUTIDLE;
    ObDereferenceObject(Process);
    UserLeave();
    return Status;
}

/* EOF */
