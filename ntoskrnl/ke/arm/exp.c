/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/ke/arm/exp.c
 * PURPOSE:         Implements exception helper routines for ARM machines
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#include <internal/arm/ksarm.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS ******************************************************************/

VOID
NTAPI
KeContextToTrapFrame(IN PCONTEXT Context,
                     IN OUT PKEXCEPTION_FRAME ExceptionFrame,
                     IN OUT PKTRAP_FRAME TrapFrame,
                     IN ULONG ContextFlags,
                     IN KPROCESSOR_MODE PreviousMode)
{
    KIRQL OldIrql;
    
    //
    // Do this at APC_LEVEL
    //
    OldIrql = KeGetCurrentIrql();
    if (OldIrql < APC_LEVEL) KeRaiseIrql(APC_LEVEL, &OldIrql);
    
    //
    // Start with the Control flags
    //
    if ((Context->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL)
    {
        //
        // So this basically means all the special stuff
        //
        if (PreviousMode == UserMode)
        {
            //
            // ARM has register banks
            //
            TrapFrame->UserSp = Context->Sp;
            TrapFrame->UserLr = Context->Lr;
        }
        else
        {
            //
            // ARM has register banks
            //
            TrapFrame->SvcSp = Context->Sp;
            TrapFrame->SvcLr = Context->Lr;
        }
        
        //
        // The rest is already in the right mode
        //
        TrapFrame->Pc = Context->Pc;
        TrapFrame->Spsr = Context->Psr;
    }
    
    //
    // Now do the integers
    //
    if ((Context->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER)
    {
        //
        // Basically everything else but FPU
        //
        TrapFrame->R0 = Context->R0;
        TrapFrame->R1 = Context->R1;
        TrapFrame->R2 = Context->R2;
        TrapFrame->R3 = Context->R3;
        TrapFrame->R4 = Context->R4;
        TrapFrame->R5 = Context->R5;
        TrapFrame->R6 = Context->R6;
        TrapFrame->R7 = Context->R7;
        TrapFrame->R8 = Context->R8;
        TrapFrame->R0 = Context->R9;
        TrapFrame->R10 = Context->R10;
        TrapFrame->R11 = Context->R11;
        TrapFrame->R12 = Context->R12;
    }
    
    //
    // Restore IRQL
    //
    if (OldIrql < APC_LEVEL) KeLowerIrql(OldIrql);  
}

VOID
NTAPI
KeTrapFrameToContext(IN PKTRAP_FRAME TrapFrame,
                     IN PKEXCEPTION_FRAME ExceptionFrame,
                     IN OUT PCONTEXT Context)
{
    KIRQL OldIrql;
    
    //
    // Do this at APC_LEVEL
    //
    OldIrql = KeGetCurrentIrql();
    if (OldIrql < APC_LEVEL) KeRaiseIrql(APC_LEVEL, &OldIrql);
    
    //
    // Start with the Control flags
    //
    if ((Context->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL)
    {
        //
        // So this basically means all the special stuff
        //
        if (KiGetPreviousMode(TrapFrame) == UserMode)
        {
            //
            // ARM has register banks
            //
            Context->Sp = TrapFrame->UserSp;
            Context->Lr = TrapFrame->UserLr;
        }
        else
        {
            //
            // ARM has register banks
            //
            Context->Sp = TrapFrame->SvcSp;
            Context->Lr = TrapFrame->SvcLr;
        }
        
        //
        // The rest is already in the right mode
        //
        Context->Pc = TrapFrame->Pc;
        Context->Psr = TrapFrame->Spsr;
    }
    
    //
    // Now do the integers
    //
    if ((Context->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER)
    {
        //
        // Basically everything else but FPU
        //
        Context->R0 = TrapFrame->R0;
        Context->R1 = TrapFrame->R1;
        Context->R2 = TrapFrame->R2;
        Context->R3 = TrapFrame->R3;
        Context->R4 = TrapFrame->R4;
        Context->R5 = TrapFrame->R5;
        Context->R6 = TrapFrame->R6;
        Context->R7 = TrapFrame->R7;
        Context->R8 = TrapFrame->R8;
        Context->R0 = TrapFrame->R9;
        Context->R10 = TrapFrame->R10;
        Context->R11 = TrapFrame->R11;
        Context->R12 = TrapFrame->R12;
    }
           
    //
    // Restore IRQL
    //
    if (OldIrql < APC_LEVEL) KeLowerIrql(OldIrql);    
}

VOID
NTAPI
KiDispatchException(IN PEXCEPTION_RECORD ExceptionRecord,
                    IN PKEXCEPTION_FRAME ExceptionFrame,
                    IN PKTRAP_FRAME TrapFrame,
                    IN KPROCESSOR_MODE PreviousMode,
                    IN BOOLEAN FirstChance)
{
    CONTEXT Context;

    /* Increase number of Exception Dispatches */
    KeGetCurrentPrcb()->KeExceptionDispatchCount++;

    /* Set the context flags */
    Context.ContextFlags = CONTEXT_FULL;
    
    /* Check if User Mode or if the kernel debugger is enabled */
    if ((PreviousMode == UserMode) || (KeGetPcr()->KdVersionBlock))
    {
        /* FIXME-V6: VFP Support */
    }

    /* Get a Context */
    KeTrapFrameToContext(TrapFrame, ExceptionFrame, &Context);

    /* Look at our exception code */
    switch (ExceptionRecord->ExceptionCode)
    {
        /* Breakpoint */
        case STATUS_BREAKPOINT:

            /* Decrement PC by four */
            Context.Pc -= sizeof(ULONG);
            break;

        /* Internal exception */
        case KI_EXCEPTION_ACCESS_VIOLATION:

            /* Set correct code */
            ExceptionRecord->ExceptionCode = STATUS_ACCESS_VIOLATION;
            if (PreviousMode == UserMode)
            {
                /* FIXME: Handle no execute */
            }
            break;
    }

    /* Handle kernel-mode first, it's simpler */
    if (PreviousMode == KernelMode)
    {
        /* Check if this is a first-chance exception */
        if (FirstChance == TRUE)
        {
            /* Break into the debugger for the first time */
            if (KiDebugRoutine(TrapFrame,
                               ExceptionFrame,
                               ExceptionRecord,
                               &Context,
                               PreviousMode,
                               FALSE))
            {
                /* Exception was handled */
                goto Handled;
            }

            /* If the Debugger couldn't handle it, dispatch the exception */
            if (RtlDispatchException(ExceptionRecord, &Context)) goto Handled;
        }

        /* This is a second-chance exception, only for the debugger */
        if (KiDebugRoutine(TrapFrame,
                           ExceptionFrame,
                           ExceptionRecord,
                           &Context,
                           PreviousMode,
                           TRUE))
        {
            /* Exception was handled */
            goto Handled;
        }

        /* Third strike; you're out */
        KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                     ExceptionRecord->ExceptionCode,
                     (ULONG_PTR)ExceptionRecord->ExceptionAddress,
                     (ULONG_PTR)TrapFrame,
                     0);
    }
    else
    {
        /* FIXME: TODO */
        /* 3rd strike, kill the process */
        DPRINT1("Kill %.16s, ExceptionCode: %lx, ExceptionAddress: %lx\n",
                PsGetCurrentProcess()->ImageFileName,
                ExceptionRecord->ExceptionCode,
                ExceptionRecord->ExceptionAddress);

        ZwTerminateProcess(NtCurrentProcess(), ExceptionRecord->ExceptionCode);
        KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                     ExceptionRecord->ExceptionCode,
                     (ULONG_PTR)ExceptionRecord->ExceptionAddress,
                     (ULONG_PTR)TrapFrame,
                     0);
    }

Handled:
    /* Convert the context back into Trap/Exception Frames */
    KeContextToTrapFrame(&Context,
                         ExceptionFrame,
                         TrapFrame,
                         Context.ContextFlags,
                         PreviousMode);
    return;
}
