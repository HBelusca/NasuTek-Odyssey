/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          Window timers messages
 * FILE:             subsystems/win32/win32k/ntuser/timer.c
 * PROGRAMER:        Gunnar
 *                   Thomas Weidenmueller (w3seek@users.sourceforge.net)
 *                   Michael Martin (michael.martin@odyssey.org)
 * REVISION HISTORY: 10/04/2003 Implemented System Timers
 *
 */

/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserTimer);

WORD FASTCALL get_key_state(void);

/* GLOBALS *******************************************************************/

static LIST_ENTRY TimersListHead;
static LONG TimeLast = 0;

#define MAX_ELAPSE_TIME 0x7FFFFFFF

/* Windows 2000 has room for 32768 window-less timers */
#define NUM_WINDOW_LESS_TIMERS   32768

static FAST_MUTEX     Mutex;
static RTL_BITMAP     WindowLessTimersBitMap;
static PVOID          WindowLessTimersBitMapBuffer;
static ULONG          HintIndex = 1;

ERESOURCE TimerLock;

#define IntLockWindowlessTimerBitmap() \
  ExEnterCriticalRegionAndAcquireFastMutexUnsafe(&Mutex)

#define IntUnlockWindowlessTimerBitmap() \
  ExReleaseFastMutexUnsafeAndLeaveCriticalRegion(&Mutex)

#define TimerEnterExclusive() \
{ \
  KeEnterCriticalRegion(); \
  ExAcquireResourceExclusiveLite(&TimerLock, TRUE); \
}

#define TimerLeave() \
{ \
  ExReleaseResourceLite(&TimerLock); \
  KeLeaveCriticalRegion(); \
}


/* FUNCTIONS *****************************************************************/
static
PTIMER
FASTCALL
CreateTimer(VOID)
{
  HANDLE Handle;
  PTIMER Ret = NULL;

  Ret = UserCreateObject(gHandleTable, NULL, &Handle, otTimer, sizeof(TIMER));
  if (Ret)
  {
     Ret->head.h = Handle;
     InsertTailList(&TimersListHead, &Ret->ptmrList);
  }

  return Ret;
}

static
BOOL
FASTCALL
RemoveTimer(PTIMER pTmr)
{
  BOOL Ret = FALSE;
  if (pTmr)
  {
     /* Set the flag, it will be removed when ready */
     RemoveEntryList(&pTmr->ptmrList);
     if ((pTmr->pWnd == NULL) && (!(pTmr->flags & TMRF_SYSTEM))) // System timers are reusable.
     {
        UINT_PTR IDEvent;

        IDEvent = NUM_WINDOW_LESS_TIMERS - pTmr->nID;
        IntLockWindowlessTimerBitmap();
        RtlClearBit(&WindowLessTimersBitMap, IDEvent);
        IntUnlockWindowlessTimerBitmap();
     }
     UserDereferenceObject(pTmr);
     Ret = UserDeleteObject( UserHMGetHandle(pTmr), otTimer);
  }
  if (!Ret) ERR("Warning: Unable to delete timer\n");

  return Ret;
}

PTIMER
FASTCALL
FindTimer(PWND Window,
          UINT_PTR nID,
          UINT flags)
{
  PLIST_ENTRY pLE;
  PTIMER pTmr, RetTmr = NULL;

  TimerEnterExclusive();
  pLE = TimersListHead.Flink;
  while (pLE != &TimersListHead)
  {
    pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);

    if ( pTmr->nID == nID &&
         pTmr->pWnd == Window &&
        (pTmr->flags & (TMRF_SYSTEM|TMRF_RIT)) == (flags & (TMRF_SYSTEM|TMRF_RIT)))
    {
       RetTmr = pTmr;
       break;
    }

    pLE = pLE->Flink;
  }
  TimerLeave();

  return RetTmr;
}

PTIMER
FASTCALL
FindSystemTimer(PMSG pMsg)
{
  PLIST_ENTRY pLE;
  PTIMER pTmr = NULL;

  TimerEnterExclusive();
  pLE = TimersListHead.Flink;
  while (pLE != &TimersListHead)
  {
    pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);

    if ( pMsg->lParam == (LPARAM)pTmr->pfn &&
         (pTmr->flags & TMRF_SYSTEM) )
       break;

    pLE = pLE->Flink;
  }
  TimerLeave();

  return pTmr;
}

BOOL
FASTCALL
ValidateTimerCallback(PTHREADINFO pti,
                      LPARAM lParam)
{
  PLIST_ENTRY pLE;
  BOOL Ret = FALSE;
  PTIMER pTmr;

  TimerEnterExclusive();
  pLE = TimersListHead.Flink;
  while (pLE != &TimersListHead)
  {
    pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);
    if ( (lParam == (LPARAM)pTmr->pfn) &&
        !(pTmr->flags & (TMRF_SYSTEM|TMRF_RIT)) &&
         (pTmr->pti->ppi == pti->ppi) )
    {
       Ret = TRUE;
       break;
    }
    pLE = pLE->Flink;
  }
  TimerLeave();

  return Ret;
}

UINT_PTR FASTCALL
IntSetTimer( PWND Window,
                  UINT_PTR IDEvent,
                  UINT Elapse,
                  TIMERPROC TimerFunc,
                  INT Type)
{
  PTIMER pTmr;
  UINT Ret = IDEvent;
  LARGE_INTEGER DueTime;
  DueTime.QuadPart = (LONGLONG)(-5000000);

#if 0
  /* Windows NT/2k/XP behaviour */
  if (Elapse > MAX_ELAPSE_TIME)
  {
     TRACE("Adjusting uElapse\n");
     Elapse = 1;
  }
#else
  /* Windows XP SP2 and Windows Server 2003 behaviour */
  if (Elapse > MAX_ELAPSE_TIME)
  {
     TRACE("Adjusting uElapse\n");
     Elapse = MAX_ELAPSE_TIME;
  }
#endif

  /* Windows 2k/XP and Windows Server 2003 SP1 behaviour */
  if (Elapse < 10)
  {
     TRACE("Adjusting uElapse\n");
     Elapse = 10;
  }

  /* Passing an IDEvent of 0 and the SetTimer returns 1.
     It will create the timer with an ID of 0 */
  if ((Window) && (IDEvent == 0))
     Ret = 1;

  pTmr = FindTimer(Window, IDEvent, Type);

  if ((!pTmr) && (Window == NULL) && (!(Type & TMRF_SYSTEM)))
  {
      IntLockWindowlessTimerBitmap();

      IDEvent = RtlFindClearBitsAndSet(&WindowLessTimersBitMap, 1, HintIndex);

      if (IDEvent == (UINT_PTR) -1)
      {
         IntUnlockWindowlessTimerBitmap();
         ERR("Unable to find a free window-less timer id\n");
         EngSetLastError(ERROR_NO_SYSTEM_RESOURCES);
         ASSERT(FALSE);
         return 0;
      }

      IDEvent = NUM_WINDOW_LESS_TIMERS - IDEvent;
      Ret = IDEvent;

      IntUnlockWindowlessTimerBitmap();
  }

  if (!pTmr)
  {
     pTmr = CreateTimer();
     if (!pTmr) return 0;

     if (Window && (Type & TMRF_TIFROMWND))
        pTmr->pti = Window->head.pti->pEThread->Tcb.Win32Thread;
     else
     {
        if (Type & TMRF_RIT)
           pTmr->pti = ptiRawInput;
        else
           pTmr->pti = PsGetCurrentThreadWin32Thread();
     }

     pTmr->pWnd    = Window;
     pTmr->cmsCountdown = Elapse;
     pTmr->cmsRate = Elapse;
     pTmr->pfn     = TimerFunc;
     pTmr->nID     = IDEvent;
     pTmr->flags   = Type|TMRF_INIT;
  }
  else
  {
     pTmr->cmsCountdown = Elapse;
     pTmr->cmsRate = Elapse;
  }

  ASSERT(MasterTimer != NULL);
  // Start the timer thread!
  if (TimersListHead.Flink == TimersListHead.Blink) // There is only one timer
     KeSetTimer(MasterTimer, DueTime, NULL);

  return Ret;
}

//
// Process win32k system timers.
//
VOID
CALLBACK
SystemTimerProc(HWND hwnd,
                UINT uMsg,
         UINT_PTR idEvent,
             DWORD dwTime)
{
  PDESKTOP pDesk;
  PWND pWnd = NULL;

  if (hwnd)
  {
     pWnd = UserGetWindowObject(hwnd);
     if (!pWnd)
     {
        ERR( "System Timer Proc has invalid window handle! 0x%x Id: %d\n", hwnd, idEvent);
        return;
     }
  }
  else
  {
     TRACE( "Windowless Timer Running!\n" );
     return;
  }

  switch (idEvent)
  {
/*
   Used in NtUserTrackMouseEvent.
 */
     case ID_EVENT_SYSTIMER_MOUSEHOVER:
       {
          POINT Point;
          UINT Msg;
          WPARAM wParam;

          pDesk = pWnd->head.rpdesk;
          if ( pDesk->dwDTFlags & DF_TME_HOVER &&
               pWnd == pDesk->spwndTrack )
          {
             Point = gpsi->ptCursor;
             if ( RECTL_bPointInRect(&pDesk->rcMouseHover, Point.x, Point.y) )
             {
                if (pDesk->htEx == HTCLIENT) // In a client area.
                {
                   wParam = get_key_state();
                   Msg = WM_MOUSEHOVER;

                   if (pWnd->ExStyle & WS_EX_LAYOUTRTL)
                   {
                      Point.x = pWnd->rcClient.right - Point.x - 1;
                   }
                   else
                      Point.x -= pWnd->rcClient.left;
                   Point.y -= pWnd->rcClient.top;
                }
                else
                {
                   wParam = pDesk->htEx; // Need to support all HTXYZ hits.
                   Msg = WM_NCMOUSEHOVER;
                }
                UserPostMessage(hwnd, Msg, wParam, MAKELPARAM(Point.x, Point.y));
                pDesk->dwDTFlags &= ~DF_TME_HOVER;
                break; // Kill this timer.
             }
          }
       }
       return; // Not this window so just return.

     default:
       ERR( "System Timer Proc invalid id %d!\n", idEvent );
       break;
  }
  IntKillTimer(pWnd, idEvent, TRUE);
}

VOID
FASTCALL
StartTheTimers(VOID)
{
  // Need to start gdi syncro timers then start timer with Hang App proc
  // that calles Idle process so the screen savers will know to run......    
  IntSetTimer(NULL, 0, 1000, HungAppSysTimerProc, TMRF_RIT);
// Test Timers
//  IntSetTimer(NULL, 0, 1000, SystemTimerProc, TMRF_RIT);
}

UINT_PTR
FASTCALL
SystemTimerSet( PWND Window,
                UINT_PTR nIDEvent,
                UINT uElapse,
                TIMERPROC lpTimerFunc)
{
  if (Window && Window->head.pti->pEThread->ThreadsProcess != PsGetCurrentProcess())
  {
     EngSetLastError(ERROR_ACCESS_DENIED);
     TRACE("SysemTimerSet: Access Denied!\n");
     return 0;
  }
  return IntSetTimer( Window, nIDEvent, uElapse, lpTimerFunc, TMRF_SYSTEM);
}

BOOL
FASTCALL
PostTimerMessages(PWND Window)
{
  PLIST_ENTRY pLE;
  PUSER_MESSAGE_QUEUE ThreadQueue;
  MSG Msg;
  PTHREADINFO pti;
  BOOL Hit = FALSE;
  PTIMER pTmr;

  pti = PsGetCurrentThreadWin32Thread();
  ThreadQueue = pti->MessageQueue;

  TimerEnterExclusive();
  pLE = TimersListHead.Flink;
  while(pLE != &TimersListHead)
  {
     pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);
     if ( (pTmr->flags & TMRF_READY) &&
          (pTmr->pti == pti) &&
          ((pTmr->pWnd == Window) || (Window == NULL)) )
        {
           Msg.hwnd    = (pTmr->pWnd) ? pTmr->pWnd->head.h : 0;
           Msg.message = (pTmr->flags & TMRF_SYSTEM) ? WM_SYSTIMER : WM_TIMER;
           Msg.wParam  = (WPARAM) pTmr->nID;
           Msg.lParam  = (LPARAM) pTmr->pfn;

           MsqPostMessage(ThreadQueue, &Msg, FALSE, QS_TIMER);
           pTmr->flags &= ~TMRF_READY;
           pti->cTimersReady++;
           Hit = TRUE;
           // Now move this entry to the end of the list so it will not be
           // called again in the next msg loop.
           if (pLE != &TimersListHead)
           {
              RemoveEntryList(&pTmr->ptmrList);
              InsertTailList(&TimersListHead, &pTmr->ptmrList);
           }
           break;
        }

     pLE = pLE->Flink;
  }

  TimerLeave();

  return Hit;
}

VOID
FASTCALL
ProcessTimers(VOID)
{
  LARGE_INTEGER TickCount, DueTime;
  LONG Time;
  PLIST_ENTRY pLE;
  PTIMER pTmr;
  LONG TimerCount = 0;

  TimerEnterExclusive();
  pLE = TimersListHead.Flink;
  KeQueryTickCount(&TickCount);
  Time = MsqCalculateMessageTime(&TickCount);

  DueTime.QuadPart = (LONGLONG)(-500000);

  while(pLE != &TimersListHead)
  {
    pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);
    TimerCount++;
    if (pTmr->flags & TMRF_WAITING)
    {
       pLE = pTmr->ptmrList.Flink;
       pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);
       continue;
    }

    if (pTmr->flags & TMRF_INIT)
    {
       pTmr->flags &= ~TMRF_INIT; // Skip this run.
    }
    else
    {
       if (pTmr->cmsCountdown < 0)
       {
          ASSERT(pTmr->pti);
          if ((!(pTmr->flags & TMRF_READY)) && (!(pTmr->pti->TIF_flags & TIF_INCLEANUP)))
          {
             if (pTmr->flags & TMRF_ONESHOT)
                pTmr->flags |= TMRF_WAITING;

             if (pTmr->flags & TMRF_RIT)
             {
                // Hard coded call here, inside raw input thread.
                pTmr->pfn(NULL, WM_SYSTIMER, pTmr->nID, (LPARAM)pTmr);
             }
             else
             {
                pTmr->flags |= TMRF_READY; // Set timer ready to be ran.
                // Set thread message queue for this timer.
                if (pTmr->pti->MessageQueue)
                {  // Wakeup thread
                   ASSERT(pTmr->pti->MessageQueue->NewMessages != NULL);
                   KeSetEvent(pTmr->pti->MessageQueue->NewMessages, IO_NO_INCREMENT, FALSE);
                }
             }
          }
          pTmr->cmsCountdown = pTmr->cmsRate;
       }
       else
          pTmr->cmsCountdown -= Time - TimeLast;
    }

    pLE = pLE->Flink;
  }

  // Restart the timer thread!
  ASSERT(MasterTimer != NULL);
  KeSetTimer(MasterTimer, DueTime, NULL);

  TimeLast = Time;

  TimerLeave();
  TRACE("TimerCount = %d\n", TimerCount);
}

BOOL FASTCALL
DestroyTimersForWindow(PTHREADINFO pti, PWND Window)
{
   PLIST_ENTRY pLE;
   PTIMER pTmr;
   BOOL TimersRemoved = FALSE;

   if ((Window == NULL))
      return FALSE;

   TimerEnterExclusive();
   pLE = TimersListHead.Flink;
   while(pLE != &TimersListHead)
   {
      pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);
      pLE = pLE->Flink; /* get next timer list entry before current timer is removed */
      if ((pTmr) && (pTmr->pti == pti) && (pTmr->pWnd == Window))
      {
         TimersRemoved = RemoveTimer(pTmr);
      }
   }

   TimerLeave();

   return TimersRemoved;
}

BOOL FASTCALL
DestroyTimersForThread(PTHREADINFO pti)
{
   PLIST_ENTRY pLE = TimersListHead.Flink;
   PTIMER pTmr;
   BOOL TimersRemoved = FALSE;

   TimerEnterExclusive();

   while(pLE != &TimersListHead)
   {
      pTmr = CONTAINING_RECORD(pLE, TIMER, ptmrList);
      pLE = pLE->Flink; /* get next timer list entry before current timer is removed */
      if ((pTmr) && (pTmr->pti == pti))
      {
         TimersRemoved = RemoveTimer(pTmr);
      }
   }

   TimerLeave();

   return TimersRemoved;
}

BOOL FASTCALL
IntKillTimer(PWND Window, UINT_PTR IDEvent, BOOL SystemTimer)
{
   PTIMER pTmr = NULL;
   TRACE("IntKillTimer Window %x id %p systemtimer %s\n",
          Window, IDEvent, SystemTimer ? "TRUE" : "FALSE");

   TimerEnterExclusive();
   pTmr = FindTimer(Window, IDEvent, SystemTimer ? TMRF_SYSTEM : 0);

   if (pTmr)
   {
      RemoveTimer(pTmr);
   }
   TimerLeave();

   return pTmr ? TRUE :  FALSE;
}

INIT_FUNCTION
NTSTATUS
NTAPI
InitTimerImpl(VOID)
{
   ULONG BitmapBytes;

   ExInitializeFastMutex(&Mutex);

   BitmapBytes = ROUND_UP(NUM_WINDOW_LESS_TIMERS, sizeof(ULONG) * 8) / 8;
   WindowLessTimersBitMapBuffer = ExAllocatePoolWithTag(NonPagedPool, BitmapBytes, TAG_TIMERBMP);
   if (WindowLessTimersBitMapBuffer == NULL)
   {
      return STATUS_UNSUCCESSFUL;
   }

   RtlInitializeBitMap(&WindowLessTimersBitMap,
                       WindowLessTimersBitMapBuffer,
                       BitmapBytes * 8);

   /* yes we need this, since ExAllocatePoolWithTag isn't supposed to zero out allocated memory */
   RtlClearAllBits(&WindowLessTimersBitMap);

   ExInitializeResourceLite(&TimerLock);
   InitializeListHead(&TimersListHead);

   return STATUS_SUCCESS;
}

UINT_PTR
APIENTRY
NtUserSetTimer
(
   HWND hWnd,
   UINT_PTR nIDEvent,
   UINT uElapse,
   TIMERPROC lpTimerFunc
)
{
   PWND Window;
   DECLARE_RETURN(UINT_PTR);

   TRACE("Enter NtUserSetTimer\n");
   UserEnterExclusive();
   Window = UserGetWindowObject(hWnd);
   UserLeave();

   RETURN(IntSetTimer(Window, nIDEvent, uElapse, lpTimerFunc, TMRF_TIFROMWND));

CLEANUP:
   TRACE("Leave NtUserSetTimer, ret=%i\n", _ret_);

   END_CLEANUP;
}


BOOL
APIENTRY
NtUserKillTimer
(
   HWND hWnd,
   UINT_PTR uIDEvent
)
{
   PWND Window;
   DECLARE_RETURN(BOOL);

   TRACE("Enter NtUserKillTimer\n");
   UserEnterExclusive();
   Window = UserGetWindowObject(hWnd);
   UserLeave();

   RETURN(IntKillTimer(Window, uIDEvent, FALSE));

CLEANUP:
   TRACE("Leave NtUserKillTimer, ret=%i\n", _ret_);
   END_CLEANUP;
}


UINT_PTR
APIENTRY
NtUserSetSystemTimer(
   HWND hWnd,
   UINT_PTR nIDEvent,
   UINT uElapse,
   TIMERPROC lpTimerFunc
)
{
   DECLARE_RETURN(UINT_PTR);

   TRACE("Enter NtUserSetSystemTimer\n");

   RETURN(IntSetTimer(UserGetWindowObject(hWnd), nIDEvent, uElapse, NULL, TMRF_SYSTEM));

CLEANUP:
   TRACE("Leave NtUserSetSystemTimer, ret=%i\n", _ret_);
   END_CLEANUP;
}

BOOL
APIENTRY
NtUserValidateTimerCallback(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam)
{
  BOOL Ret = FALSE;

  UserEnterShared();

  Ret = ValidateTimerCallback(PsGetCurrentThreadWin32Thread(), lParam);

  UserLeave();
  return Ret;
}


/* EOF */
