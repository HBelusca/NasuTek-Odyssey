/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey System Libraries
 * FILE:        lib/kernel32/thread/fiber.c
 * PURPOSE:     Fiber Implementation
 * PROGRAMMERS: 
 *              Alex Ionescu (alex@relsoft.net)
 *              KJK::Hyperion <noog@libero.it>
 */
#include <k32.h>

#define NDEBUG
#include <debug.h>

typedef struct _FIBER                                      /* Field offsets:  */
{                                                          /* 32 bit   64 bit */
    /* this must be the first field */
    LPVOID Parameter;                                      /*   0x00     0x00 */
    struct _EXCEPTION_REGISTRATION_RECORD * ExceptionList; /*   0x04     0x08 */
    LPVOID StackBase;                                      /*   0x08     0x10 */
    LPVOID StackLimit;                                     /*   0x0C     0x18 */
    LPVOID DeallocationStack;                              /*   0x10     0x20 */
    CONTEXT Context;                                       /*   0x14     0x28 */
    ULONG GuaranteedStackBytes;                            /*   0x2E0         */
    PVOID FlsData;                                         /*   0x2E4         */
    PVOID ActivationContextStack;                          /*   0x2E8         */
} FIBER, *PFIBER;

/* PRIVATE FUNCTIONS **********************************************************/

VOID
WINAPI
BaseRundownFls(IN PVOID FlsData)
{
    /* No FLS support yet */
    
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * @implemented
 */
BOOL
WINAPI
ConvertFiberToThread(VOID)
{
    PTEB Teb;
    PFIBER FiberData;
    DPRINT1("Converting Fiber to Thread\n");

    /* Check if the thread is already not a fiber */
    Teb = NtCurrentTeb();
    if (!Teb->HasFiberData)
    {
        /* Fail */
        SetLastError(ERROR_ALREADY_THREAD);
        return FALSE;
    }

    /* this thread won't run a fiber anymore */
    Teb->HasFiberData = FALSE;
    FiberData = Teb->NtTib.FiberData;
    Teb->NtTib.FiberData = NULL;

    /* Free the fiber */
    ASSERT(FiberData != NULL);
    RtlFreeHeap(GetProcessHeap(), 0, FiberData);

    /* success */
    return TRUE;
}

/*
 * @implemented
 */
LPVOID
WINAPI
ConvertThreadToFiberEx(LPVOID lpParameter, 
                       DWORD dwFlags)
{
    PTEB pTeb = NtCurrentTeb();
    PFIBER pfCurFiber;
    DPRINT1("Converting Thread to Fiber\n");

    /* the current thread is already a fiber */
    if(pTeb->HasFiberData && pTeb->NtTib.FiberData) return pTeb->NtTib.FiberData;

    /* allocate the fiber */
    pfCurFiber = (PFIBER)RtlAllocateHeap(GetProcessHeap(), 
                                         0,
                                         sizeof(FIBER));

    /* failure */
    if (pfCurFiber == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    /* copy some contextual data from the thread to the fiber */
    pfCurFiber->Parameter = lpParameter;
    pfCurFiber->ExceptionList = pTeb->NtTib.ExceptionList;
    pfCurFiber->StackBase = pTeb->NtTib.StackBase;
    pfCurFiber->StackLimit = pTeb->NtTib.StackLimit;
    pfCurFiber->DeallocationStack = pTeb->DeallocationStack;
    pfCurFiber->FlsData = pTeb->FlsData;
    pfCurFiber->GuaranteedStackBytes = pTeb->GuaranteedStackBytes;
    pfCurFiber->ActivationContextStack = pTeb->ActivationContextStackPointer;
    pfCurFiber->Context.ContextFlags = CONTEXT_FULL;

    /* Save FPU State if requsted */
    if (dwFlags & FIBER_FLAG_FLOAT_SWITCH)
    {
        pfCurFiber->Context.ContextFlags |= CONTEXT_FLOATING_POINT;
    }

    /* associate the fiber to the current thread */
    pTeb->NtTib.FiberData = pfCurFiber;
    pTeb->HasFiberData = TRUE;

    /* success */
    return (LPVOID)pfCurFiber;
}

/*
 * @implemented
 */
LPVOID
WINAPI
ConvertThreadToFiber(LPVOID lpParameter)
{
    /* Call the newer function */
    return ConvertThreadToFiberEx(lpParameter, 0);
}

/*
 * @implemented
 */
LPVOID
WINAPI
CreateFiber(SIZE_T dwStackSize,
            LPFIBER_START_ROUTINE lpStartAddress,
            LPVOID lpParameter)
{
    /* Call the Newer Function */
    return CreateFiberEx(dwStackSize, 0, 0, lpStartAddress, lpParameter);
}

/*
 * @implemented
 */
LPVOID
WINAPI
CreateFiberEx(SIZE_T dwStackCommitSize,
              SIZE_T dwStackReserveSize,
              DWORD dwFlags,
              LPFIBER_START_ROUTINE lpStartAddress,
              LPVOID lpParameter)
{
    PFIBER Fiber;
    NTSTATUS Status;
    INITIAL_TEB InitialTeb;
    PVOID ActivationContextStack = NULL;
    DPRINT("Creating Fiber\n");

    /* Check for invalid flags */
    if (dwFlags &~ FIBER_FLAG_FLOAT_SWITCH)
    {
        /* Fail */
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    /* Allocate the Activation Context Stack */
    Status = RtlAllocateActivationContextStack(&ActivationContextStack);
    if (!NT_SUCCESS(Status))
    {
        /* Fail */
        BaseSetLastNTError(Status);
        return NULL;
    }

    /* Allocate the fiber */
    Fiber = RtlAllocateHeap(RtlGetProcessHeap(), 0, sizeof(FIBER));
    if (!Fiber)
    {
        /* Fail */
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    /* Create the stack for the fiber */
    Status = BasepCreateStack(NtCurrentProcess(),
                              dwStackCommitSize,
                              dwStackReserveSize,
                              &InitialTeb);
    if (!NT_SUCCESS(Status))
    {
        /* Free the fiber */
        RtlFreeHeap(GetProcessHeap(), 0, Fiber);

        /* Free the activation context */
        DPRINT1("Leaking activation stack because nobody implemented free");
        //RtlFreeActivationContextStack(&ActivationContextStack);

        /* Failure */
        BaseSetLastNTError(Status);
        return NULL;
    }

    /* Clear the context */
    RtlZeroMemory(&Fiber->Context, sizeof(CONTEXT));

    /* Copy the data into the fiber */
    Fiber->StackBase = InitialTeb.StackBase;
    Fiber->StackLimit = InitialTeb.StackLimit;
    Fiber->DeallocationStack = InitialTeb.AllocatedStackBase;
    Fiber->Parameter = lpParameter;
    Fiber->ExceptionList = EXCEPTION_CHAIN_END;
    Fiber->GuaranteedStackBytes = 0;
    Fiber->FlsData = NULL;
    Fiber->ActivationContextStack = ActivationContextStack;
    Fiber->Context.ContextFlags = CONTEXT_FULL;

    /* Save FPU State if requested */
    Fiber->Context.ContextFlags = (dwFlags & FIBER_FLAG_FLOAT_SWITCH) ? CONTEXT_FLOATING_POINT : 0;

    /* initialize the context for the fiber */
    BasepInitializeContext(&Fiber->Context,
                           lpParameter,
                           lpStartAddress,
                           InitialTeb.StackBase,
                           2);

    /* Return the Fiber */
    return Fiber;
}

/*
 * @implemented
 */
VOID
WINAPI
DeleteFiber(LPVOID lpFiber)
{
    SIZE_T Size = 0;
    PFIBER Fiber = (PFIBER)lpFiber;
    PTEB Teb;

    /* First, exit the thread */
    Teb = NtCurrentTeb();
    if ((Teb->HasFiberData) && (Teb->NtTib.FiberData == Fiber)) ExitThread(1);

    /* Now de-allocate the stack */
    NtFreeVirtualMemory(NtCurrentProcess(),
                        &Fiber->DeallocationStack,
                        &Size,
                        MEM_RELEASE);

    /* Get rid of FLS */
    if (Fiber->FlsData) BaseRundownFls(Fiber->FlsData);

    /* Get rid of the activation stack */
    DPRINT1("Leaking activation stack because nobody implemented free");
    //RtlFreeActivationContextStack(Fiber->ActivationContextStack);

    /* Free the fiber data */
    RtlFreeHeap(GetProcessHeap(), 0, lpFiber);
}

/*
 * @implemented
 */
BOOL
WINAPI
IsThreadAFiber(VOID)
{
    return NtCurrentTeb()->HasFiberData;
}

/*
 * @unimplemented
 */
DWORD
WINAPI
FlsAlloc(PFLS_CALLBACK_FUNCTION lpCallback)
{
   (void)lpCallback;

   UNIMPLEMENTED;
   SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
   return FLS_OUT_OF_INDEXES;
}


/*
 * @unimplemented
 */
BOOL
WINAPI
FlsFree(DWORD dwFlsIndex)
{
    (void)dwFlsIndex;

    UNIMPLEMENTED;
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
}


/*
 * @implemented
 */
PVOID
WINAPI
FlsGetValue(DWORD dwFlsIndex)
{
    PVOID *ppFlsSlots;
    PVOID pRetVal;

    if(dwFlsIndex >= 128) goto l_InvalidParam;

    ppFlsSlots = NtCurrentTeb()->FlsData;

    if(ppFlsSlots == NULL) goto l_InvalidParam;

    SetLastError(0);
    pRetVal = ppFlsSlots[dwFlsIndex + 2];

    return pRetVal;

l_InvalidParam:
    SetLastError(ERROR_INVALID_PARAMETER);
    return NULL;
}


/*
 * @implemented
 */
BOOL
WINAPI
FlsSetValue(DWORD dwFlsIndex, PVOID lpFlsData)
{
    PVOID *ppFlsSlots;
    TEB *pTeb = NtCurrentTeb();

    if(dwFlsIndex >= 128) goto l_InvalidParam;

    ppFlsSlots = pTeb->FlsData;

    if (ppFlsSlots == NULL)
    {
        PEB *pPeb = pTeb->ProcessEnvironmentBlock;

        ppFlsSlots = RtlAllocateHeap(pPeb->ProcessHeap,
                                     HEAP_ZERO_MEMORY,
                                     (128 + 2) * sizeof(PVOID));
        if(ppFlsSlots == NULL) goto l_OutOfMemory;

        pTeb->FlsData = ppFlsSlots;

        RtlAcquirePebLock();

        /* TODO: initialization */

        RtlReleasePebLock();
    }

    ppFlsSlots[dwFlsIndex + 2] = lpFlsData;

    return TRUE;

l_OutOfMemory:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    goto l_Fail;

l_InvalidParam:
    SetLastError(ERROR_INVALID_PARAMETER);

l_Fail:
    return FALSE;
}

/* EOF */
