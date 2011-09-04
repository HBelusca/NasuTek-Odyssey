/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/mm/balance.c
 * PURPOSE:         kernel memory managment functions
 *
 * PROGRAMMERS:     David Welch (welch@cwcom.net)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

#if defined (ALLOC_PRAGMA)
#pragma alloc_text(INIT, MmInitializeBalancer)
#pragma alloc_text(INIT, MmInitializeMemoryConsumer)
#pragma alloc_text(INIT, MiInitBalancerThread)
#endif


/* TYPES ********************************************************************/
typedef struct _MM_ALLOCATION_REQUEST
{
   PFN_NUMBER Page;
   LIST_ENTRY ListEntry;
   KEVENT Event;
}
MM_ALLOCATION_REQUEST, *PMM_ALLOCATION_REQUEST;

/* GLOBALS ******************************************************************/

MM_MEMORY_CONSUMER MiMemoryConsumers[MC_MAXIMUM];
static ULONG MiMinimumAvailablePages;
static ULONG MiNrTotalPages;
static LIST_ENTRY AllocationListHead;
static KSPIN_LOCK AllocationListLock;
static ULONG MiPagesRequired = 0;
static ULONG MiMinimumPagesPerRun = 10;

static CLIENT_ID MiBalancerThreadId;
static HANDLE MiBalancerThreadHandle = NULL;
static KEVENT MiBalancerEvent;
static KTIMER MiBalancerTimer;
static LONG MiBalancerWork = 0;

/* FUNCTIONS ****************************************************************/

VOID
INIT_FUNCTION
NTAPI
MmInitializeBalancer(ULONG NrAvailablePages, ULONG NrSystemPages)
{
   memset(MiMemoryConsumers, 0, sizeof(MiMemoryConsumers));
   InitializeListHead(&AllocationListHead);
   KeInitializeSpinLock(&AllocationListLock);

   MiNrTotalPages = NrAvailablePages;

   /* Set up targets. */
   MiMinimumAvailablePages = 64;
    if ((NrAvailablePages + NrSystemPages) >= 8192)
    {
        MiMemoryConsumers[MC_CACHE].PagesTarget = NrAvailablePages / 4 * 3;   
    }
    else if ((NrAvailablePages + NrSystemPages) >= 4096)
    {
        MiMemoryConsumers[MC_CACHE].PagesTarget = NrAvailablePages / 3 * 2;
    }
    else
    {
        MiMemoryConsumers[MC_CACHE].PagesTarget = NrAvailablePages / 8;        
    }
   MiMemoryConsumers[MC_USER].PagesTarget = NrAvailablePages - MiMinimumAvailablePages;
}

VOID
INIT_FUNCTION
NTAPI
MmInitializeMemoryConsumer(ULONG Consumer,
                           NTSTATUS (*Trim)(ULONG Target, ULONG Priority,
                                            PULONG NrFreed))
{
   MiMemoryConsumers[Consumer].Trim = Trim;
}

VOID
NTAPI
MiZeroPhysicalPage(
    IN PFN_NUMBER PageFrameIndex
);

NTSTATUS
NTAPI
MmReleasePageMemoryConsumer(ULONG Consumer, PFN_NUMBER Page)
{
   PMM_ALLOCATION_REQUEST Request;
   PLIST_ENTRY Entry;
   KIRQL OldIrql;

   if (Page == 0)
   {
      DPRINT1("Tried to release page zero.\n");
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   KeAcquireSpinLock(&AllocationListLock, &OldIrql);
   if (MmGetReferenceCountPage(Page) == 1)
   {
      (void)InterlockedDecrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
      if (IsListEmpty(&AllocationListHead) || MmAvailablePages < MiMinimumAvailablePages)
      {
         KeReleaseSpinLock(&AllocationListLock, OldIrql);
         if(Consumer == MC_USER) MmRemoveLRUUserPage(Page);
         OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
         MmDereferencePage(Page);
         KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
      }
      else
      {
         Entry = RemoveHeadList(&AllocationListHead);
         Request = CONTAINING_RECORD(Entry, MM_ALLOCATION_REQUEST, ListEntry);
         KeReleaseSpinLock(&AllocationListLock, OldIrql);
         if(Consumer == MC_USER) MmRemoveLRUUserPage(Page);
         MiZeroPhysicalPage(Page);
         Request->Page = Page;
         KeSetEvent(&Request->Event, IO_NO_INCREMENT, FALSE);
      }
   }
   else
   {
      KeReleaseSpinLock(&AllocationListLock, OldIrql);
      if(Consumer == MC_USER) MmRemoveLRUUserPage(Page);
      OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
      MmDereferencePage(Page);
      KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
   }

   return(STATUS_SUCCESS);
}

VOID
NTAPI
MiTrimMemoryConsumer(ULONG Consumer)
{
   LONG Target;
   ULONG NrFreedPages;

   Target = MiMemoryConsumers[Consumer].PagesUsed -
            MiMemoryConsumers[Consumer].PagesTarget;
   if (Target < 1)
   {
      Target = 1;
   }

   if (MiMemoryConsumers[Consumer].Trim != NULL)
   {
      MiMemoryConsumers[Consumer].Trim(Target, 0, &NrFreedPages);
   }
}

NTSTATUS
MmTrimUserMemory(ULONG Target, ULONG Priority, PULONG NrFreedPages)
{
    PFN_NUMBER CurrentPage;
    PFN_NUMBER NextPage;
    NTSTATUS Status;
    
    (*NrFreedPages) = 0;
    
    CurrentPage = MmGetLRUFirstUserPage();
    while (CurrentPage != 0 && Target > 0)
    {
        NextPage = MmGetLRUNextUserPage(CurrentPage);
        
        Status = MmPageOutPhysicalAddress(CurrentPage);
        if (NT_SUCCESS(Status))
        {
            DPRINT("Succeeded\n");
            Target--;
            (*NrFreedPages)++;
        }
        
        CurrentPage = NextPage;
    }
    return(STATUS_SUCCESS);
}

VOID
NTAPI
MmRebalanceMemoryConsumers(VOID)
{
   LONG Target;
   ULONG i;
   ULONG NrFreedPages;
   NTSTATUS Status;

   Target = (MiMinimumAvailablePages - MmAvailablePages) + MiPagesRequired;
   Target = max(Target, (LONG) MiMinimumPagesPerRun);

   for (i = 0; i < MC_MAXIMUM && Target > 0; i++)
   {
      if (MiMemoryConsumers[i].Trim != NULL)
      {
         Status = MiMemoryConsumers[i].Trim(Target, 0, &NrFreedPages);
         if (!NT_SUCCESS(Status))
         {
            KeBugCheck(MEMORY_MANAGEMENT);
         }
         Target = Target - NrFreedPages;
      }
   }
}

static BOOLEAN
MiIsBalancerThread(VOID)
{
   return MiBalancerThreadHandle != NULL &&
          PsGetCurrentThread() == MiBalancerThreadId.UniqueThread;
}

NTSTATUS
NTAPI
MmRequestPageMemoryConsumer(ULONG Consumer, BOOLEAN CanWait,
                            PPFN_NUMBER AllocatedPage)
{
   ULONG OldUsed;
   PFN_NUMBER Page;
   KIRQL OldIrql;

   /*
    * Make sure we don't exceed our individual target.
    */
   OldUsed = InterlockedIncrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
   if (OldUsed >= (MiMemoryConsumers[Consumer].PagesTarget - 1) &&
         !MiIsBalancerThread())
   {
      if (!CanWait)
      {
         (void)InterlockedDecrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
         return(STATUS_NO_MEMORY);
      }
      MiTrimMemoryConsumer(Consumer);
   }

   /*
    * Allocate always memory for the non paged pool and for the pager thread.
    */
   if ((Consumer == MC_SYSTEM) || MiIsBalancerThread())
   {
      OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
      Page = MmAllocPage(Consumer);
      KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
      if (Page == 0)
      {
         KeBugCheck(NO_PAGES_AVAILABLE);
      }
      *AllocatedPage = Page;
      if (MmAvailablePages <= MiMinimumAvailablePages &&
            MiBalancerThreadHandle != NULL)
      {
         KeSetEvent(&MiBalancerEvent, IO_NO_INCREMENT, FALSE);
      }
      return(STATUS_SUCCESS);
   }

   /*
    * Make sure we don't exceed global targets.
    */
   if (MmAvailablePages <= MiMinimumAvailablePages)
   {
      MM_ALLOCATION_REQUEST Request;

      if (!CanWait)
      {
         (void)InterlockedDecrementUL(&MiMemoryConsumers[Consumer].PagesUsed);
         return(STATUS_NO_MEMORY);
      }

      /* Insert an allocation request. */
      Request.Page = 0;

      KeInitializeEvent(&Request.Event, NotificationEvent, FALSE);
      (void)InterlockedIncrementUL(&MiPagesRequired);

      KeAcquireSpinLock(&AllocationListLock, &OldIrql);

      if (MiBalancerThreadHandle != NULL)
      {
         KeSetEvent(&MiBalancerEvent, IO_NO_INCREMENT, FALSE);
      }
      InsertTailList(&AllocationListHead, &Request.ListEntry);
      KeReleaseSpinLock(&AllocationListLock, OldIrql);

      KeWaitForSingleObject(&Request.Event,
                            0,
                            KernelMode,
                            FALSE,
                            NULL);

      Page = Request.Page;
      if (Page == 0)
      {
         KeBugCheck(NO_PAGES_AVAILABLE);
      }
      /* Update the Consumer and make the page active */
      if(Consumer == MC_USER) MmInsertLRULastUserPage(Page);
      *AllocatedPage = Page;
      (void)InterlockedDecrementUL(&MiPagesRequired);
      return(STATUS_SUCCESS);
   }

   /*
    * Actually allocate the page.
    */
   OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);
   Page = MmAllocPage(Consumer);
   KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
   if (Page == 0)
   {
      KeBugCheck(NO_PAGES_AVAILABLE);
   }
   if(Consumer == MC_USER) MmInsertLRULastUserPage(Page);
   *AllocatedPage = Page;

   return(STATUS_SUCCESS);
}

VOID NTAPI
MiBalancerThread(PVOID Unused)
{
   PVOID WaitObjects[2];
   NTSTATUS Status;
   ULONG i;
   ULONG NrFreedPages;
   ULONG NrPagesUsed;
   ULONG Target;
   BOOLEAN ShouldRun;


   WaitObjects[0] = &MiBalancerEvent;
   WaitObjects[1] = &MiBalancerTimer;

   while (1)
   {
      Status = KeWaitForMultipleObjects(2,
                                        WaitObjects,
                                        WaitAny,
                                        Executive,
                                        KernelMode,
                                        FALSE,
                                        NULL,
                                        NULL);

      if (Status == STATUS_SUCCESS)
      {
         /* MiBalancerEvent */
         while (MmAvailablePages < MiMinimumAvailablePages + 5)
         {
            for (i = 0; i < MC_MAXIMUM; i++)
            {
               if (MiMemoryConsumers[i].Trim != NULL)
               {
                  NrFreedPages = 0;
                  Status = MiMemoryConsumers[i].Trim(MiMinimumPagesPerRun, 0, &NrFreedPages);
                  if (!NT_SUCCESS(Status))
                  {
                     KeBugCheck(MEMORY_MANAGEMENT);
                  }
               }
            }
         }
         InterlockedExchange(&MiBalancerWork, 0);
      }
      else if (Status == STATUS_SUCCESS + 1)
      {
         /* MiBalancerTimer */
         ShouldRun = MmAvailablePages < MiMinimumAvailablePages + 5 ? TRUE : FALSE;
         for (i = 0; i < MC_MAXIMUM; i++)
         {
            if (MiMemoryConsumers[i].Trim != NULL)
            {
               NrPagesUsed = MiMemoryConsumers[i].PagesUsed;
               if (NrPagesUsed > MiMemoryConsumers[i].PagesTarget || ShouldRun)
               {
                  if (NrPagesUsed > MiMemoryConsumers[i].PagesTarget)
                  {
                     Target = max (NrPagesUsed - MiMemoryConsumers[i].PagesTarget,
                                   MiMinimumPagesPerRun);
                  }
                  else
                  {
                     Target = MiMinimumPagesPerRun;
                  }
                  NrFreedPages = 0;
                  Status = MiMemoryConsumers[i].Trim(Target, 0, &NrFreedPages);
                  if (!NT_SUCCESS(Status))
                  {
                     KeBugCheck(MEMORY_MANAGEMENT);
                  }
               }
            }
         }
      }
      else
      {
         DPRINT1("KeWaitForMultipleObjects failed, status = %x\n", Status);
         KeBugCheck(MEMORY_MANAGEMENT);
      }
   }
}

VOID
INIT_FUNCTION
NTAPI
MiInitBalancerThread(VOID)
{
   KPRIORITY Priority;
   NTSTATUS Status;
#if !defined(__GNUC__)

   LARGE_INTEGER dummyJunkNeeded;
   dummyJunkNeeded.QuadPart = -20000000; /* 2 sec */
   ;
#endif


   KeInitializeEvent(&MiBalancerEvent, SynchronizationEvent, FALSE);
   KeInitializeTimerEx(&MiBalancerTimer, SynchronizationTimer);
   KeSetTimerEx(&MiBalancerTimer,
#if defined(__GNUC__)
                (LARGE_INTEGER)(LONGLONG)-20000000LL,     /* 2 sec */
#else
                dummyJunkNeeded,
#endif
                2000,         /* 2 sec */
                NULL);

   Status = PsCreateSystemThread(&MiBalancerThreadHandle,
                                 THREAD_ALL_ACCESS,
                                 NULL,
                                 NULL,
                                 &MiBalancerThreadId,
                                 (PKSTART_ROUTINE) MiBalancerThread,
                                 NULL);
   if (!NT_SUCCESS(Status))
   {
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   Priority = LOW_REALTIME_PRIORITY + 1;
   NtSetInformationThread(MiBalancerThreadHandle,
                          ThreadPriority,
                          &Priority,
                          sizeof(Priority));

}


/* EOF */
