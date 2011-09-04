/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/kd/kdio.c
 * PURPOSE:         NT Kernel Debugger Input/Output Functions
 *
 * PROGRAMMERS:     Alex Ionescu (alex@relsoft.net)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>
#include <odyssey/buildno.h>
#include <debug.h>

/* GLOBALS *******************************************************************/

#define KdpBufferSize  (1024 * 512)
BOOLEAN KdpLoggingEnabled = FALSE;
PCHAR KdpDebugBuffer = NULL;
volatile ULONG KdpCurrentPosition = 0;
volatile ULONG KdpFreeBytes = 0;
KSPIN_LOCK KdpDebugLogSpinLock;
KEVENT KdpLoggerThreadEvent;
HANDLE KdpLogFileHandle;

KSPIN_LOCK KdpSerialSpinLock;
KD_PORT_INFORMATION SerialPortInfo = { DEFAULT_DEBUG_PORT, DEFAULT_DEBUG_BAUD_RATE, 0 };

/* Current Port in use. FIXME: Do we support more then one? */
ULONG KdpPort;

#define KdpScreenLineLenght 80
CHAR KdpScreenLineBuffer[KdpScreenLineLenght + 1] = "";
ULONG KdpScreenLineBufferPos = 0, KdpScreenLineLength = 0;

const ULONG KdpDmesgBufferSize = 128 * 1024; // 512*1024; // 5*1024*1024;
PCHAR KdpDmesgBuffer = NULL;
volatile ULONG KdpDmesgCurrentPosition = 0;
volatile ULONG KdpDmesgFreeBytes = 0;
volatile ULONG KdbDmesgTotalWritten = 0;
KSPIN_LOCK KdpDmesgLogSpinLock;
volatile BOOLEAN KdbpIsInDmesgMode = FALSE;

/* FILE DEBUG LOG FUNCTIONS **************************************************/

VOID
NTAPI
KdpLoggerThread(PVOID Context)
{
    ULONG beg, end, num;
    IO_STATUS_BLOCK Iosb;

    KdpLoggingEnabled = TRUE;

    while (TRUE)
    {
        KeWaitForSingleObject(&KdpLoggerThreadEvent, 0, KernelMode, FALSE, NULL);

        /* Bug */
        /* Keep KdpCurrentPosition and KdpFreeBytes values in local
         * variables to avoid their possible change from Producer part,
         * KdpPrintToLogFile function
         */
        end = KdpCurrentPosition;
        num = KdpFreeBytes;

        /* Now securely calculate values, based on local variables */
        beg = (end + num) % KdpBufferSize;
        num = KdpBufferSize - num;

        /* Nothing to do? */
        if (num == 0)
            continue;

        if (end > beg)
        {
            NtWriteFile(KdpLogFileHandle, NULL, NULL, NULL, &Iosb,
                        KdpDebugBuffer + beg, num, NULL, NULL);
        }
        else
        {
            NtWriteFile(KdpLogFileHandle, NULL, NULL, NULL, &Iosb,
                        KdpDebugBuffer + beg, KdpBufferSize - beg, NULL, NULL);

            NtWriteFile(KdpLogFileHandle, NULL, NULL, NULL, &Iosb,
                        KdpDebugBuffer, end, NULL, NULL);
        }

        (VOID)InterlockedExchangeAddUL(&KdpFreeBytes, num);
    }
}

VOID
NTAPI
KdpPrintToLogFile(PCH String,
                  ULONG StringLength)
{
    ULONG beg, end, num;
    KIRQL OldIrql;

    if (KdpDebugBuffer == NULL) return;

    /* Acquire the printing spinlock without waiting at raised IRQL */
    while (TRUE)
    {
        /* Wait when the spinlock becomes available */
        while (!KeTestSpinLock(&KdpDebugLogSpinLock));

        /* Spinlock was free, raise IRQL */
        KeRaiseIrql(HIGH_LEVEL, &OldIrql);

        /* Try to get the spinlock */
        if (KeTryToAcquireSpinLockAtDpcLevel(&KdpDebugLogSpinLock))
            break;

        /* Someone else got the spinlock, lower IRQL back */
        KeLowerIrql(OldIrql);
    }

    beg = KdpCurrentPosition;
    num = KdpFreeBytes;
    if (StringLength < num)
        num = StringLength;

    if (num != 0)
    {
        end = (beg + num) % KdpBufferSize;
        KdpCurrentPosition = end;
        KdpFreeBytes -= num;

        if (end > beg)
        {
            RtlCopyMemory(KdpDebugBuffer + beg, String, num);
        }
        else
        {
            RtlCopyMemory(KdpDebugBuffer + beg, String, KdpBufferSize - beg);
            RtlCopyMemory(KdpDebugBuffer, String + KdpBufferSize - beg, end);
        }
    }

    /* Release spinlock */
    KiReleaseSpinLock(&KdpDebugLogSpinLock);

    /* Lower IRQL */
    KeLowerIrql(OldIrql);

    /* Signal the logger thread */
    if (OldIrql <= DISPATCH_LEVEL && KdpLoggingEnabled)
        KeSetEvent(&KdpLoggerThreadEvent, 0, FALSE);
}

VOID
NTAPI
INIT_FUNCTION
KdpInitDebugLog(PKD_DISPATCH_TABLE DispatchTable,
                ULONG BootPhase)
{
    NTSTATUS Status;
    UNICODE_STRING FileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK Iosb;
    HANDLE ThreadHandle;
    KPRIORITY Priority;
    SIZE_T MemSizeMBs;

    if (!KdpDebugMode.File) return;

    if (BootPhase == 0)
    {
        KdComPortInUse = NULL;

        /* Write out the functions that we support for now */
        DispatchTable->KdpInitRoutine = KdpInitDebugLog;
        DispatchTable->KdpPrintRoutine = KdpPrintToLogFile;

        /* Register as a Provider */
        InsertTailList(&KdProviders, &DispatchTable->KdProvidersList);

    }
    else if (BootPhase == 1)
    {
        /* Allocate a buffer for debug log */
        KdpDebugBuffer = ExAllocatePool(NonPagedPool, KdpBufferSize);
        KdpFreeBytes = KdpBufferSize;

        /* Initialize spinlock */
        KeInitializeSpinLock(&KdpDebugLogSpinLock);

        /* Display separator + Odyssey version at start of the debug log */
        DPRINT1("---------------------------------------------------------------\n");
        DPRINT1("Odyssey "KERNEL_VERSION_STR" (Build "KERNEL_VERSION_BUILD_STR")\n");
        MemSizeMBs = MmNumberOfPhysicalPages * PAGE_SIZE / 1024 / 1024;
        DPRINT1("%u System Processor [%u MB Memory]\n", KeNumberProcessors, MemSizeMBs);
    }
    else if (BootPhase == 2)
    {
        HalDisplayString("\n   File log debugging enabled\n\n");
    }
    else if (BootPhase == 3)
    {
        /* Setup the log name */
        RtlInitUnicodeString(&FileName, L"\\SystemRoot\\debug.log");
        InitializeObjectAttributes(&ObjectAttributes,
                                   &FileName,
                                   0,
                                   NULL,
                                   NULL);

        /* Create the log file */
        Status = NtCreateFile(&KdpLogFileHandle,
                              FILE_APPEND_DATA | SYNCHRONIZE,
                              &ObjectAttributes,
                              &Iosb,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ,
                              FILE_SUPERSEDE,
                              FILE_WRITE_THROUGH | FILE_SYNCHRONOUS_IO_NONALERT,
                              NULL,
                              0);

        if (!NT_SUCCESS(Status)) return;

        KeInitializeEvent(&KdpLoggerThreadEvent, SynchronizationEvent, TRUE);

        /* Create the logger thread */
        Status = PsCreateSystemThread(&ThreadHandle,
                                      THREAD_ALL_ACCESS,
                                      NULL,
                                      NULL,
                                      NULL,
                                      KdpLoggerThread,
                                      NULL);

        if (!NT_SUCCESS(Status)) return;

        Priority = 7;
        NtSetInformationThread(ThreadHandle,
                               ThreadPriority,
                               &Priority,
                               sizeof(Priority));
    }
}

/* SERIAL FUNCTIONS **********************************************************/

VOID
NTAPI
KdpSerialDebugPrint(LPSTR Message,
                    ULONG Length)
{
    KIRQL OldIrql;
    PCHAR pch = (PCHAR) Message;

    /* Acquire the printing spinlock without waiting at raised IRQL */
    while (TRUE)
    {
        /* Wait when the spinlock becomes available */
        while (!KeTestSpinLock(&KdpSerialSpinLock));

        /* Spinlock was free, raise IRQL */
        KeRaiseIrql(HIGH_LEVEL, &OldIrql);

        /* Try to get the spinlock */
        if (KeTryToAcquireSpinLockAtDpcLevel(&KdpSerialSpinLock))
            break;

        /* Someone else got the spinlock, lower IRQL back */
        KeLowerIrql(OldIrql);
    }

    /* Output the message */
    while (*pch != 0)
    {
        if (*pch == '\n')
        {
            KdPortPutByteEx(&SerialPortInfo, '\r');
        }
        KdPortPutByteEx(&SerialPortInfo, *pch);
        pch++;
    }

    /* Release spinlock */
    KiReleaseSpinLock(&KdpSerialSpinLock);

    /* Lower IRQL */
    KeLowerIrql(OldIrql);
}

VOID
NTAPI
KdpSerialInit(PKD_DISPATCH_TABLE DispatchTable,
              ULONG BootPhase)
{
    SIZE_T MemSizeMBs;
    if (!KdpDebugMode.Serial) return;

    if (BootPhase == 0)
    {
        /* Write out the functions that we support for now */
        DispatchTable->KdpInitRoutine = KdpSerialInit;
        DispatchTable->KdpPrintRoutine = KdpSerialDebugPrint;

        /* Initialize the Port */
        if (!KdPortInitializeEx(&SerialPortInfo, 0, 0))
        {
            KdpDebugMode.Serial = FALSE;
            return;
        }
        KdComPortInUse = (PUCHAR)(ULONG_PTR)SerialPortInfo.BaseAddress;

        /* Initialize spinlock */
        KeInitializeSpinLock(&KdpSerialSpinLock);

        /* Register as a Provider */
        InsertTailList(&KdProviders, &DispatchTable->KdProvidersList);

        /* Display separator + Odyssey version at start of the debug log */
        DPRINT1("-----------------------------------------------------\n");
        DPRINT1("Odyssey "KERNEL_VERSION_STR" (Build "KERNEL_VERSION_BUILD_STR")\n");
        MemSizeMBs = MmNumberOfPhysicalPages * PAGE_SIZE / 1024 / 1024;
        DPRINT1("%u System Processor [%u MB Memory]\n", KeNumberProcessors, MemSizeMBs);
        DPRINT1("Command Line: %s\n", KeLoaderBlock->LoadOptions);
        DPRINT1("ARC Paths: %s %s %s %s\n", KeLoaderBlock->ArcBootDeviceName,
                                            KeLoaderBlock->NtHalPathName,
                                            KeLoaderBlock->ArcHalDeviceName,
                                            KeLoaderBlock->NtBootPathName);
    }
    else if (BootPhase == 2)
    {
        HalDisplayString("\n   Serial debugging enabled\n\n");
    }
}

/* SCREEN FUNCTIONS **********************************************************/

/*
 * Screen debug logger function KdpScreenPrint() writes text messages into
 * KdpDmesgBuffer, using it as a circular buffer. KdpDmesgBuffer contents could
 * be later (re)viewed using dmesg command of kdbg. KdpScreenPrint() protects
 * KdpDmesgBuffer from simultaneous writes by use of KdpDmesgLogSpinLock.
 */
VOID
NTAPI
KdpScreenPrint(LPSTR Message,
               ULONG Length)
{
    ULONG beg, end, num;
    KIRQL OldIrql;
    PCHAR pch = (PCHAR) Message;

    while (*pch)
    {
        if(*pch == '\b')
        {
            /* HalDisplayString does not support '\b'. Workaround it and use '\r' */
            if(KdpScreenLineLength > 0)
            {
                /* Remove last character from buffer */
                KdpScreenLineBuffer[--KdpScreenLineLength] = '\0';
                KdpScreenLineBufferPos = KdpScreenLineLength;

                /* Clear row and print line again */
                HalDisplayString("\r");
                HalDisplayString(KdpScreenLineBuffer);
            }
        }
        else
        {
            KdpScreenLineBuffer[KdpScreenLineLength++] = *pch;
            KdpScreenLineBuffer[KdpScreenLineLength] = '\0';
        }

        if(*pch == '\n' || KdpScreenLineLength == KdpScreenLineLenght)
        {
            /* Print buffered characters */
            if(KdpScreenLineBufferPos != KdpScreenLineLength)
                HalDisplayString(KdpScreenLineBuffer + KdpScreenLineBufferPos);

            /* Clear line buffer */
            KdpScreenLineBuffer[0] = '\0';
            KdpScreenLineLength = KdpScreenLineBufferPos = 0;
        }

        ++pch;
    }

    /* Print buffered characters */
    if(KdpScreenLineBufferPos != KdpScreenLineLength)
    {
        HalDisplayString(KdpScreenLineBuffer + KdpScreenLineBufferPos);
        KdpScreenLineBufferPos = KdpScreenLineLength;
    }

    /* Dmesg: store Message in the buffer to show it later */
    if (KdbpIsInDmesgMode)
       return;

    if (KdpDmesgBuffer == NULL)
      return;

    /* Acquire the printing spinlock without waiting at raised IRQL */
    while (TRUE)
    {
        /* Wait when the spinlock becomes available */
        while (!KeTestSpinLock(&KdpDmesgLogSpinLock));

        /* Spinlock was free, raise IRQL */
        KeRaiseIrql(HIGH_LEVEL, &OldIrql);

        /* Try to get the spinlock */
        if (KeTryToAcquireSpinLockAtDpcLevel(&KdpDmesgLogSpinLock))
            break;

        /* Someone else got the spinlock, lower IRQL back */
        KeLowerIrql(OldIrql);
    }

    /* Invariant: always_true(KdpDmesgFreeBytes == KdpDmesgBufferSize);
     * set num to min(KdpDmesgFreeBytes, Length).
     */
    num = (Length < KdpDmesgFreeBytes) ? Length : KdpDmesgFreeBytes; 
    beg = KdpDmesgCurrentPosition;
    if (num != 0)
    {
        end = (beg + num) % KdpDmesgBufferSize;
        if (end > beg)
        {
            RtlCopyMemory(KdpDmesgBuffer + beg, Message, Length);
        }
        else
        {
            RtlCopyMemory(KdpDmesgBuffer + beg, Message, KdpDmesgBufferSize - beg);
            RtlCopyMemory(KdpDmesgBuffer, Message + (KdpDmesgBufferSize - beg), end);
        }
        KdpDmesgCurrentPosition = end;

        /* Counting the total bytes written */
        KdbDmesgTotalWritten += num;
    }

    /* Release spinlock */
    KiReleaseSpinLock(&KdpDmesgLogSpinLock);

    /* Lower IRQL */
    KeLowerIrql(OldIrql);

    /* Optional step(?): find out a way to notify about buffer exhaustion,
     * and possibly fall into kbd to use dmesg command: user will read
     * debug messages before they will be wiped over by next writes.
     */
}

VOID
NTAPI
KdpScreenInit(PKD_DISPATCH_TABLE DispatchTable,
              ULONG BootPhase)
{
    SIZE_T MemSizeMBs;
    if (!KdpDebugMode.Screen) return;

    if (BootPhase == 0)
    {
        /* Write out the functions that we support for now */
        DispatchTable->KdpInitRoutine = KdpScreenInit;
        DispatchTable->KdpPrintRoutine = KdpScreenPrint;

        /* Register as a Provider */
        InsertTailList(&KdProviders, &DispatchTable->KdProvidersList);
    }
    else if (BootPhase == 1)
    {
      /* Allocate a buffer for dmesg log buffer. +1 for terminating null,
       * see kdbp_cli.c:KdbpCmdDmesg()/2
       */
      KdpDmesgBuffer = ExAllocatePool(NonPagedPool, KdpDmesgBufferSize + 1);
      RtlZeroMemory(KdpDmesgBuffer, KdpDmesgBufferSize + 1);
      KdpDmesgFreeBytes = KdpDmesgBufferSize;
      KdbDmesgTotalWritten = 0;

      /* Initialize spinlock */
      KeInitializeSpinLock(&KdpDmesgLogSpinLock);

      /* Display separator + Odyssey version at start of the debug log */
      DPRINT1("-----------------------------------------------------\n");
      DPRINT1("Odyssey "KERNEL_VERSION_STR" (Build "KERNEL_VERSION_BUILD_STR")\n");
      MemSizeMBs = MmNumberOfPhysicalPages * PAGE_SIZE / 1024 / 1024;
      DPRINT1("%u System Processor [%u MB Memory]\n", KeNumberProcessors, MemSizeMBs);
      DPRINT1("Command Line: %s\n", KeLoaderBlock->LoadOptions);
      DPRINT1("ARC Paths: %s %s %s %s\n", KeLoaderBlock->ArcBootDeviceName,
                                          KeLoaderBlock->NtHalPathName,
                                          KeLoaderBlock->ArcHalDeviceName,
                                          KeLoaderBlock->NtBootPathName);
    }
    else if (BootPhase == 2)
    {
        HalDisplayString("\n   Screen debugging enabled\n\n");
    }
}

/* GENERAL FUNCTIONS *********************************************************/

ULONG
NTAPI
KdpPrintString(LPSTR String,
               ULONG Length)
{
    PLIST_ENTRY CurrentEntry;
    PKD_DISPATCH_TABLE CurrentTable;

    if (!KdpDebugMode.Value) return 0;

    /* Call the registered handlers */
    CurrentEntry = KdProviders.Flink;
    while (CurrentEntry != &KdProviders)
    {
        /* Get the current table */
        CurrentTable = CONTAINING_RECORD(CurrentEntry,
                                         KD_DISPATCH_TABLE,
                                         KdProvidersList);

        /* Call it */
        CurrentTable->KdpPrintRoutine(String, Length);

        /* Next Table */
        CurrentEntry = CurrentEntry->Flink;
    }

    /* Call the Wrapper Routine */
    if (WrapperTable.KdpPrintRoutine)
        WrapperTable.KdpPrintRoutine(String, Length);

    /* Return the Length */
    return Length;
}

/* EOF */
