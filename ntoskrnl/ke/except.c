/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/ke/except.c
 * PURPOSE:         Platform independent exception handling
 * PROGRAMMERS:     Odyssey Portable Systems Group
 *                  Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
KiContinuePreviousModeUser(IN PCONTEXT Context,
                           IN PKEXCEPTION_FRAME ExceptionFrame,
                           IN PKTRAP_FRAME TrapFrame)
{
    CONTEXT LocalContext;

    /* We'll have to make a copy and probe it */
    ProbeForRead(Context, sizeof(CONTEXT), sizeof(ULONG));
    RtlCopyMemory(&LocalContext, Context, sizeof(CONTEXT));
    Context = &LocalContext;

    /* Convert the context into Exception/Trap Frames */
    KeContextToTrapFrame(&LocalContext,
                         ExceptionFrame,
                         TrapFrame,
                         LocalContext.ContextFlags,
                         UserMode);
}

NTSTATUS
NTAPI
KiContinue(IN PCONTEXT Context,
           IN PKEXCEPTION_FRAME ExceptionFrame,
           IN PKTRAP_FRAME TrapFrame)
{
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL OldIrql = APC_LEVEL;
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();

    /* Raise to APC_LEVEL, only if needed */
    if (KeGetCurrentIrql() < APC_LEVEL) KeRaiseIrql(APC_LEVEL, &OldIrql);

    /* Set up SEH to validate the context */
    _SEH2_TRY
    {
        /* Check the previous mode */
        if (PreviousMode != KernelMode)
        {
            /* Validate from user-mode */
            KiContinuePreviousModeUser(Context,
                                       ExceptionFrame,
                                       TrapFrame);
        }
        else
        {
            /* Convert the context into Exception/Trap Frames */
            KeContextToTrapFrame(Context,
                                 ExceptionFrame,
                                 TrapFrame,
                                 Context->ContextFlags,
                                 KernelMode);
        }
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Save the exception code */
        Status = _SEH2_GetExceptionCode();
    }
    _SEH2_END;

    /* Lower the IRQL if needed */
    if (OldIrql < APC_LEVEL) KeLowerIrql(OldIrql);

    /* Return status */
    return Status;
}

NTSTATUS
NTAPI
KiRaiseException(IN PEXCEPTION_RECORD ExceptionRecord,
                 IN PCONTEXT Context,
                 IN PKEXCEPTION_FRAME ExceptionFrame,
                 IN PKTRAP_FRAME TrapFrame,
                 IN BOOLEAN SearchFrames)
{
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    CONTEXT LocalContext;
    EXCEPTION_RECORD LocalExceptionRecord;
    ULONG ParameterCount, Size;

    /* Check if we need to probe */
    if (PreviousMode != KernelMode)
    {
        /* Set up SEH */
        _SEH2_TRY
        {
            /* Probe the context */
            ProbeForRead(Context, sizeof(CONTEXT), sizeof(ULONG));

            /* Probe the Exception Record */
            ProbeForRead(ExceptionRecord,
                         FIELD_OFFSET(EXCEPTION_RECORD, NumberParameters) +
                         sizeof(ULONG),
                         sizeof(ULONG));

            /* Validate the maximum parameters */
            if ((ParameterCount = ExceptionRecord->NumberParameters) >
                EXCEPTION_MAXIMUM_PARAMETERS)
            {
                /* Too large */
                _SEH2_YIELD(return STATUS_INVALID_PARAMETER);
            }

            /* Probe the entire parameters now*/
            Size = (sizeof(EXCEPTION_RECORD) - 
                    ((EXCEPTION_MAXIMUM_PARAMETERS - ParameterCount) * sizeof(ULONG)));
            ProbeForRead(ExceptionRecord, Size, sizeof(ULONG));

            /* Now make copies in the stack */
            RtlCopyMemory(&LocalContext, Context, sizeof(CONTEXT));
            RtlCopyMemory(&LocalExceptionRecord, ExceptionRecord, Size);
            Context = &LocalContext;
            ExceptionRecord = &LocalExceptionRecord;

            /* Update the parameter count */
            ExceptionRecord->NumberParameters = ParameterCount;
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            /* Don't fail silently */
            DPRINT1("KiRaiseException: Failed to Probe\n");
            DbgBreakPoint();

            /* Return the exception code */
            _SEH2_YIELD(return _SEH2_GetExceptionCode());
        }
        _SEH2_END;
    }

    /* Convert the context record */
    KeContextToTrapFrame(Context,
                         ExceptionFrame,
                         TrapFrame,
                         Context->ContextFlags,
                         PreviousMode);

    /* Dispatch the exception */
    ExceptionRecord->ExceptionCode &= ~KI_EXCEPTION_INTERNAL;
    KiDispatchException(ExceptionRecord,
                        ExceptionFrame,
                        TrapFrame,
                        PreviousMode,
                        SearchFrames);

    /* We are done */
    return STATUS_SUCCESS;
}

/* SYSTEM CALLS ***************************************************************/

NTSTATUS
NTAPI
NtRaiseException(IN PEXCEPTION_RECORD ExceptionRecord,
                 IN PCONTEXT Context,
                 IN BOOLEAN FirstChance)
{
    NTSTATUS Status;
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;

    /* Get trap frame and link previous one*/
    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    Thread->TrapFrame = KiGetLinkedTrapFrame(TrapFrame);
    
    /* Set exception list */
#ifdef _M_IX86
    KeGetPcr()->NtTib.ExceptionList = TrapFrame->ExceptionList;
#endif
    
    /* Raise the exception */
    Status = KiRaiseException(ExceptionRecord,
                              Context,
                              NULL,
                              TrapFrame,
                              FirstChance);
    if (NT_SUCCESS(Status))
    {
        /* It was handled, so exit restoring all state */
        KiServiceExit2(TrapFrame);
    }
    else
    {
        /* Exit with error */
        KiServiceExit(TrapFrame, Status);
    }
    
    /* We don't actually make it here */
    return Status;
}

NTSTATUS
NTAPI
NtContinue(IN PCONTEXT Context,
           IN BOOLEAN TestAlert)
{
    PKTHREAD Thread;
    NTSTATUS Status;
    PKTRAP_FRAME TrapFrame;
    
    /* Get trap frame and link previous one*/
    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    Thread->TrapFrame = KiGetLinkedTrapFrame(TrapFrame);
    
    /* Continue from this point on */
    Status = KiContinue(Context, NULL, TrapFrame);
    if (NT_SUCCESS(Status))
    {
        /* Check if alert was requested */
        if (TestAlert) KeTestAlertThread(Thread->PreviousMode);
        
        /* Exit to new trap frame */
        KiServiceExit2(TrapFrame);
    }
    else
    {
        /* Exit with an error */
        KiServiceExit(TrapFrame, Status);
    }
    
    /* We don't actually make it here */
    return Status;
}

/* EOF */
