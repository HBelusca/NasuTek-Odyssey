/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/ex/sysinfo.c
 * PURPOSE:         System information functions
 *
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 *                  Aleksey Bragin (aleksey@odyssey.org)
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

VOID MmPrintMemoryStatistic(VOID);

FAST_MUTEX ExpEnvironmentLock;
ERESOURCE ExpFirmwareTableResource;
LIST_ENTRY ExpFirmwareTableProviderListHead;

NTSTATUS
NTAPI
ExpQueryModuleInformation(IN PLIST_ENTRY KernelModeList,
                          IN PLIST_ENTRY UserModeList,
                          OUT PRTL_PROCESS_MODULES Modules,
                          IN ULONG Length,
                          OUT PULONG ReturnLength)
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG RequiredLength;
    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    ANSI_STRING ModuleName;
    ULONG ModuleCount = 0;
    PLIST_ENTRY NextEntry;
    PCHAR p;

    /* Setup defaults */
    RequiredLength = FIELD_OFFSET(RTL_PROCESS_MODULES, Modules);
    ModuleInfo = &Modules->Modules[0];

    /* Loop the kernel list */
    NextEntry = KernelModeList->Flink;
    while (NextEntry != KernelModeList)
    {
        /* Get the entry */
        LdrEntry = CONTAINING_RECORD(NextEntry,
                                     LDR_DATA_TABLE_ENTRY,
                                     InLoadOrderLinks);

        /* Update size and check if we can manage one more entry */
        RequiredLength += sizeof(RTL_PROCESS_MODULE_INFORMATION);
        if (Length >= RequiredLength)
        {
            /* Fill it out */
            ModuleInfo->MappedBase = NULL;
            ModuleInfo->ImageBase = LdrEntry->DllBase;
            ModuleInfo->ImageSize = LdrEntry->SizeOfImage;
            ModuleInfo->Flags = LdrEntry->Flags;
            ModuleInfo->LoadCount = LdrEntry->LoadCount;
            ModuleInfo->LoadOrderIndex = (USHORT)ModuleCount;
            ModuleInfo->InitOrderIndex = 0;

            /* Setup name */
            RtlInitEmptyAnsiString(&ModuleName,
                                   ModuleInfo->FullPathName,
                                   sizeof(ModuleInfo->FullPathName));

            /* Convert it */
            Status = RtlUnicodeStringToAnsiString(&ModuleName,
                                                  &LdrEntry->FullDllName,
                                                  FALSE);
            if ((NT_SUCCESS(Status)) || (Status == STATUS_BUFFER_OVERFLOW))
            {
                /* Calculate offset to name */
                p = ModuleName.Buffer + ModuleName.Length;
                while ((p > ModuleName.Buffer) && (*--p))
                {
                    /* Check if we found the separator */
                    if (*p == OBJ_NAME_PATH_SEPARATOR)
                    {
                        /* We did, break out */
                        p++;
                        break;
                    }
                }

                /* Set the offset */
                ModuleInfo->OffsetToFileName = p - ModuleName.Buffer;
            }
            else
            {
                /* Return empty name */
                ModuleInfo->FullPathName[0] = ANSI_NULL;
                ModuleInfo->OffsetToFileName = 0;
            }

            /* Go to the next module */
            ModuleInfo++;
        }
        else
        {
            /* Set error code */
            Status = STATUS_INFO_LENGTH_MISMATCH;
        }

        /* Update count and move to next entry */
        ModuleCount++;
        NextEntry = NextEntry->Flink;
    }

    /* Check if caller also wanted user modules */
    if (UserModeList)
    {
        /* FIXME: TODO */
        DPRINT1("User-mode list not yet supported in Odyssey!\n");
    }

    /* Update return length */
    if (ReturnLength) *ReturnLength = RequiredLength;

    /* Validate the length again */
    if (Length >= FIELD_OFFSET(RTL_PROCESS_MODULES, Modules))
    {
        /* Set the final count */
        Modules->NumberOfModules = ModuleCount;
    }
    else
    {
        /* Otherwise, we failed */
        Status = STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Done */
    return Status;
}

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
VOID
NTAPI
ExGetCurrentProcessorCpuUsage(PULONG CpuUsage)
{
    PKPRCB Prcb;
    ULONG TotalTime;
    ULONGLONG ScaledIdle;

    Prcb = KeGetCurrentPrcb();

    ScaledIdle = Prcb->IdleThread->KernelTime * 100;
    TotalTime = Prcb->KernelTime + Prcb->UserTime;
    if (TotalTime != 0)
        *CpuUsage = (ULONG)(100 - (ScaledIdle / TotalTime));
    else
        *CpuUsage = 0;
}

/*
 * @implemented
 */
VOID
NTAPI
ExGetCurrentProcessorCounts(PULONG ThreadKernelTime,
                            PULONG TotalCpuTime,
                            PULONG ProcessorNumber)
{
    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb();

    *ThreadKernelTime = Prcb->KernelTime + Prcb->UserTime;
    *TotalCpuTime = Prcb->CurrentThread->KernelTime;
    *ProcessorNumber = KeGetCurrentProcessorNumber();
}

/*
 * @implemented
 */
BOOLEAN
NTAPI
ExIsProcessorFeaturePresent(IN ULONG ProcessorFeature)
{
    /* Quick check to see if it exists at all */
    if (ProcessorFeature >= PROCESSOR_FEATURE_MAX) return(FALSE);

    /* Return our support for it */
    return(SharedUserData->ProcessorFeatures[ProcessorFeature]);
}

/*
 * @implemented
 */
BOOLEAN
NTAPI
ExVerifySuite(SUITE_TYPE SuiteType)
{
    if (SuiteType == Personal) return TRUE;
    return FALSE;
}

NTSTATUS
NTAPI
NtQuerySystemEnvironmentValue(IN PUNICODE_STRING VariableName,
                              OUT PWSTR ValueBuffer,
                              IN ULONG ValueBufferLength,
                              IN OUT PULONG ReturnLength OPTIONAL)
{
    ANSI_STRING AName;
    UNICODE_STRING WName;
    ARC_STATUS Result;
    PCH Value;
    ANSI_STRING AValue;
    UNICODE_STRING WValue;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PAGED_CODE();

    PreviousMode = ExGetPreviousMode();

    if (PreviousMode != KernelMode)
    {
        _SEH2_TRY
        {
            ProbeForRead(VariableName,
                         sizeof(UNICODE_STRING),
                         sizeof(ULONG));

            ProbeForWrite(ValueBuffer,
                          ValueBufferLength,
                          sizeof(WCHAR));

            if (ReturnLength != NULL) ProbeForWriteUlong(ReturnLength);
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            /* Return the exception code */
            _SEH2_YIELD(return _SEH2_GetExceptionCode());
        }
        _SEH2_END;
    }

    /*
     * Copy the name to kernel space if necessary and convert it to ANSI.
     */
    Status = ProbeAndCaptureUnicodeString(&WName,
                                          PreviousMode,
                                          VariableName);
    if (NT_SUCCESS(Status))
    {
        /*
         * according to ntinternals the SeSystemEnvironmentName privilege is required!
         */
        if (!SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                    PreviousMode))
        {
            ReleaseCapturedUnicodeString(&WName, PreviousMode);
            DPRINT1("NtQuerySystemEnvironmentValue: Caller requires the SeSystemEnvironmentPrivilege privilege!\n");
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        /*
         * convert the value name to ansi
         */
        Status = RtlUnicodeStringToAnsiString(&AName, &WName, TRUE);
        ReleaseCapturedUnicodeString(&WName, PreviousMode);

        if (!NT_SUCCESS(Status)) return Status;

        /*
         * Create a temporary buffer for the value
         */
        Value = ExAllocatePool(NonPagedPool, ValueBufferLength);
        if (Value == NULL)
        {
            RtlFreeAnsiString(&AName);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        /*
         * Get the environment variable
         */
        Result = HalGetEnvironmentVariable(AName.Buffer,
                                           (USHORT)ValueBufferLength,
                                           Value);
        if (!Result)
        {
            RtlFreeAnsiString(&AName);
            ExFreePool(Value);
            return STATUS_UNSUCCESSFUL;
        }

        /*
         * Convert the result to UNICODE, protect with SEH in case the value buffer
         * isn't NULL-terminated!
         */
        _SEH2_TRY
        {
            RtlInitAnsiString(&AValue, Value);
            Status = RtlAnsiStringToUnicodeString(&WValue, &AValue, TRUE);
        }
        _SEH2_EXCEPT(ExSystemExceptionFilter())
        {
            Status = _SEH2_GetExceptionCode();
        }
        _SEH2_END;

        if (NT_SUCCESS(Status))
        {
            /*
             * Copy the result back to the caller.
             */
            _SEH2_TRY
            {
                RtlCopyMemory(ValueBuffer, WValue.Buffer, WValue.Length);
                ValueBuffer[WValue.Length / sizeof(WCHAR)] = L'\0';
                if (ReturnLength != NULL)
                {
                    *ReturnLength = WValue.Length + sizeof(WCHAR);
                }

                Status = STATUS_SUCCESS;
            }
            _SEH2_EXCEPT(ExSystemExceptionFilter())
            {
                Status = _SEH2_GetExceptionCode();
            }
            _SEH2_END;
        }

        /*
         * Cleanup allocated resources.
         */
        RtlFreeAnsiString(&AName);
        ExFreePool(Value);
    }

    return Status;
}


NTSTATUS
NTAPI
NtSetSystemEnvironmentValue(IN PUNICODE_STRING VariableName,
                            IN PUNICODE_STRING Value)
{
    UNICODE_STRING CapturedName, CapturedValue;
    ANSI_STRING AName, AValue;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    PreviousMode = ExGetPreviousMode();

    /*
     * Copy the strings to kernel space if necessary
     */
    Status = ProbeAndCaptureUnicodeString(&CapturedName,
                                          PreviousMode,
                                          VariableName);
    if (NT_SUCCESS(Status))
    {
        Status = ProbeAndCaptureUnicodeString(&CapturedValue,
                                              PreviousMode,
                                              Value);
        if (NT_SUCCESS(Status))
        {
            /*
             * according to ntinternals the SeSystemEnvironmentName privilege is required!
             */
            if (SeSinglePrivilegeCheck(SeSystemEnvironmentPrivilege,
                                       PreviousMode))
            {
                /*
                 * convert the strings to ANSI
                 */
                Status = RtlUnicodeStringToAnsiString(&AName,
                                                      &CapturedName,
                                                      TRUE);
                if (NT_SUCCESS(Status))
                {
                    Status = RtlUnicodeStringToAnsiString(&AValue,
                                                          &CapturedValue,
                                                          TRUE);
                    if (NT_SUCCESS(Status))
                    {
                        ARC_STATUS Result = HalSetEnvironmentVariable(AName.Buffer,
                                                                      AValue.Buffer);

                        Status = (Result ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
                    }
                }
            }
            else
            {
                DPRINT1("NtSetSystemEnvironmentValue: Caller requires the SeSystemEnvironmentPrivilege privilege!\n");
                Status = STATUS_PRIVILEGE_NOT_HELD;
            }

            ReleaseCapturedUnicodeString(&CapturedValue,
                                         PreviousMode);
        }

        ReleaseCapturedUnicodeString(&CapturedName,
                                     PreviousMode);
    }

    return Status;
}

NTSTATUS
NTAPI
NtEnumerateSystemEnvironmentValuesEx(IN ULONG InformationClass,
                                     IN PVOID Buffer,
                                     IN ULONG BufferLength)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
NtQuerySystemEnvironmentValueEx(IN PUNICODE_STRING VariableName,
                                IN LPGUID VendorGuid,
                                IN PVOID Value,
                                IN OUT PULONG ReturnLength,
                                IN OUT PULONG Attributes)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
NTAPI
NtSetSystemEnvironmentValueEx(IN PUNICODE_STRING VariableName,
                              IN LPGUID VendorGuid)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/* --- Query/Set System Information --- */

/*
 * NOTE: QSI_DEF(n) and SSI_DEF(n) define _cdecl function symbols
 * so the stack is popped only in one place on x86 platform.
 */
#define QSI_USE(n) QSI##n
#define QSI_DEF(n) \
static NTSTATUS QSI_USE(n) (PVOID Buffer, ULONG Size, PULONG ReqSize)

#define SSI_USE(n) SSI##n
#define SSI_DEF(n) \
static NTSTATUS SSI_USE(n) (PVOID Buffer, ULONG Size)


/* Class 0 - Basic Information */
QSI_DEF(SystemBasicInformation)
{
    PSYSTEM_BASIC_INFORMATION Sbi
        = (PSYSTEM_BASIC_INFORMATION) Buffer;

    *ReqSize = sizeof(SYSTEM_BASIC_INFORMATION);

    /* Check user buffer's size */
    if (Size != sizeof(SYSTEM_BASIC_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    RtlZeroMemory(Sbi, Size);
    Sbi->Reserved = 0;
    Sbi->TimerResolution = KeMaximumIncrement;
    Sbi->PageSize = PAGE_SIZE;
    Sbi->NumberOfPhysicalPages = MmNumberOfPhysicalPages;
    Sbi->LowestPhysicalPageNumber = MmLowestPhysicalPage;
    Sbi->HighestPhysicalPageNumber = MmHighestPhysicalPage;
    Sbi->AllocationGranularity = MM_VIRTMEM_GRANULARITY; /* hard coded on Intel? */
    Sbi->MinimumUserModeAddress = 0x10000; /* Top of 64k */
    Sbi->MaximumUserModeAddress = (ULONG_PTR)MmHighestUserAddress;
    Sbi->ActiveProcessorsAffinityMask = KeActiveProcessors;
    Sbi->NumberOfProcessors = KeNumberProcessors;

    return STATUS_SUCCESS;
}

/* Class 1 - Processor Information */
QSI_DEF(SystemProcessorInformation)
{
    PSYSTEM_PROCESSOR_INFORMATION Spi
        = (PSYSTEM_PROCESSOR_INFORMATION) Buffer;

    *ReqSize = sizeof(SYSTEM_PROCESSOR_INFORMATION);

    /* Check user buffer's size */
    if (Size < sizeof(SYSTEM_PROCESSOR_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }
    Spi->ProcessorArchitecture = KeProcessorArchitecture;
    Spi->ProcessorLevel = KeProcessorLevel;
    Spi->ProcessorRevision = KeProcessorRevision;
    Spi->Reserved = 0;
    Spi->ProcessorFeatureBits = KeFeatureBits;

    DPRINT("Arch %d Level %d Rev 0x%x\n", Spi->ProcessorArchitecture,
        Spi->ProcessorLevel, Spi->ProcessorRevision);

    return STATUS_SUCCESS;
}

/* Class 2 - Performance Information */
QSI_DEF(SystemPerformanceInformation)
{
    ULONG IdleUser, IdleKernel;
    PSYSTEM_PERFORMANCE_INFORMATION Spi
        = (PSYSTEM_PERFORMANCE_INFORMATION) Buffer;

    PEPROCESS TheIdleProcess;

    *ReqSize = sizeof(SYSTEM_PERFORMANCE_INFORMATION);

    /* Check user buffer's size */
    if (Size < sizeof(SYSTEM_PERFORMANCE_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    TheIdleProcess = PsIdleProcess;

    IdleKernel = KeQueryRuntimeProcess(&TheIdleProcess->Pcb, &IdleUser);
    Spi->IdleProcessTime.QuadPart = UInt32x32To64(IdleKernel, KeMaximumIncrement);
    Spi->IoReadTransferCount = IoReadTransferCount;
    Spi->IoWriteTransferCount = IoWriteTransferCount;
    Spi->IoOtherTransferCount = IoOtherTransferCount;
    Spi->IoReadOperationCount = IoReadOperationCount;
    Spi->IoWriteOperationCount = IoWriteOperationCount;
    Spi->IoOtherOperationCount = IoOtherOperationCount;

    Spi->AvailablePages = MmAvailablePages;
    /*
     *   Add up all the used "Committed" memory + pagefile.
     *   Not sure this is right. 8^\
     */
    Spi->CommittedPages = MiMemoryConsumers[MC_SYSTEM].PagesUsed +
                          MiMemoryConsumers[MC_CACHE].PagesUsed +
                          MiMemoryConsumers[MC_USER].PagesUsed +
                          MiUsedSwapPages;
    /*
     *  Add up the full system total + pagefile.
     *  All this make Taskmgr happy but not sure it is the right numbers.
     *  This too, fixes some of GlobalMemoryStatusEx numbers.
     */
    Spi->CommitLimit = MmNumberOfPhysicalPages + MiFreeSwapPages + MiUsedSwapPages;

    Spi->PeakCommitment = 0; /* FIXME */
    Spi->PageFaultCount = 0; /* FIXME */
    Spi->CopyOnWriteCount = 0; /* FIXME */
    Spi->TransitionCount = 0; /* FIXME */
    Spi->CacheTransitionCount = 0; /* FIXME */
    Spi->DemandZeroCount = 0; /* FIXME */
    Spi->PageReadCount = 0; /* FIXME */
    Spi->PageReadIoCount = 0; /* FIXME */
    Spi->CacheReadCount = 0; /* FIXME */
    Spi->CacheIoCount = 0; /* FIXME */
    Spi->DirtyPagesWriteCount = 0; /* FIXME */
    Spi->DirtyWriteIoCount = 0; /* FIXME */
    Spi->MappedPagesWriteCount = 0; /* FIXME */
    Spi->MappedWriteIoCount = 0; /* FIXME */

    Spi->PagedPoolPages = 0; /* FIXME */
    Spi->PagedPoolAllocs = 0; /* FIXME */
    Spi->PagedPoolFrees = 0; /* FIXME */
    Spi->NonPagedPoolPages = 0; /* FIXME */
    Spi->NonPagedPoolAllocs = 0; /* FIXME */
    Spi->NonPagedPoolFrees = 0; /* FIXME */

    Spi->FreeSystemPtes = 0; /* FIXME */

    Spi->ResidentSystemCodePage = 0; /* FIXME */

    Spi->TotalSystemDriverPages = 0; /* FIXME */
    Spi->TotalSystemCodePages = 0; /* FIXME */
    Spi->NonPagedPoolLookasideHits = 0; /* FIXME */
    Spi->PagedPoolLookasideHits = 0; /* FIXME */
    Spi->Spare3Count = 0; /* FIXME */

    Spi->ResidentSystemCachePage = MiMemoryConsumers[MC_CACHE].PagesUsed;
    Spi->ResidentPagedPoolPage = 0; /* FIXME */

    Spi->ResidentSystemDriverPage = 0; /* FIXME */
    Spi->CcFastReadNoWait = 0; /* FIXME */
    Spi->CcFastReadWait = 0; /* FIXME */
    Spi->CcFastReadResourceMiss = 0; /* FIXME */
    Spi->CcFastReadNotPossible = 0; /* FIXME */

    Spi->CcFastMdlReadNoWait = 0; /* FIXME */
    Spi->CcFastMdlReadWait = 0; /* FIXME */
    Spi->CcFastMdlReadResourceMiss = 0; /* FIXME */
    Spi->CcFastMdlReadNotPossible = 0; /* FIXME */

    Spi->CcMapDataNoWait = 0; /* FIXME */
    Spi->CcMapDataWait = 0; /* FIXME */
    Spi->CcMapDataNoWaitMiss = 0; /* FIXME */
    Spi->CcMapDataWaitMiss = 0; /* FIXME */

    Spi->CcPinMappedDataCount = 0; /* FIXME */
    Spi->CcPinReadNoWait = 0; /* FIXME */
    Spi->CcPinReadWait = 0; /* FIXME */
    Spi->CcPinReadNoWaitMiss = 0; /* FIXME */
    Spi->CcPinReadWaitMiss = 0; /* FIXME */
    Spi->CcCopyReadNoWait = 0; /* FIXME */
    Spi->CcCopyReadWait = 0; /* FIXME */
    Spi->CcCopyReadNoWaitMiss = 0; /* FIXME */
    Spi->CcCopyReadWaitMiss = 0; /* FIXME */

    Spi->CcMdlReadNoWait = 0; /* FIXME */
    Spi->CcMdlReadWait = 0; /* FIXME */
    Spi->CcMdlReadNoWaitMiss = 0; /* FIXME */
    Spi->CcMdlReadWaitMiss = 0; /* FIXME */
    Spi->CcReadAheadIos = 0; /* FIXME */
    Spi->CcLazyWriteIos = 0; /* FIXME */
    Spi->CcLazyWritePages = 0; /* FIXME */
    Spi->CcDataFlushes = 0; /* FIXME */
    Spi->CcDataPages = 0; /* FIXME */
    Spi->ContextSwitches = 0; /* FIXME */
    Spi->FirstLevelTbFills = 0; /* FIXME */
    Spi->SecondLevelTbFills = 0; /* FIXME */
    Spi->SystemCalls = 0; /* FIXME */

    return STATUS_SUCCESS;
}

/* Class 3 - Time Of Day Information */
QSI_DEF(SystemTimeOfDayInformation)
{
    SYSTEM_TIMEOFDAY_INFORMATION Sti;
    LARGE_INTEGER CurrentTime;

    /* Set amount of written information to 0 */
    *ReqSize = 0;

    /* Check user buffer's size */
    if (Size > sizeof(SYSTEM_TIMEOFDAY_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Get current time */
    KeQuerySystemTime(&CurrentTime);

    /* Zero local buffer */
    RtlZeroMemory(&Sti, sizeof(SYSTEM_TIMEOFDAY_INFORMATION));

    /* Fill local time structure */
    Sti.BootTime= KeBootTime;
    Sti.CurrentTime = CurrentTime;
    Sti.TimeZoneBias.QuadPart = ExpTimeZoneBias.QuadPart;
    Sti.TimeZoneId = ExpTimeZoneId;
    Sti.Reserved = 0;

    /* Copy as much as requested by caller */
    RtlCopyMemory(Buffer, &Sti, Size);

    /* Set amount of information we copied */
    *ReqSize = Size;

    return STATUS_SUCCESS;
}

/* Class 4 - Path Information */
QSI_DEF(SystemPathInformation)
{
    /* FIXME: QSI returns STATUS_BREAKPOINT. Why? */
    DPRINT1("NtQuerySystemInformation - SystemPathInformation not implemented\n");

    return STATUS_BREAKPOINT;
}

/* Class 5 - Process Information */
QSI_DEF(SystemProcessInformation)
{
    PSYSTEM_PROCESS_INFORMATION SpiCurrent;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PEPROCESS Process = NULL, SystemProcess;
    PETHREAD CurrentThread;
    ANSI_STRING ImageName;
    ULONG CurrentSize;
    USHORT ImageNameMaximumLength; // image name len in bytes
    USHORT ImageNameLength;
    PLIST_ENTRY CurrentEntry;
    ULONG TotalSize = 0, ThreadsCount;
    ULONG TotalUser, TotalKernel;
    PUCHAR Current;
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING ProcessImageName;
    PWCHAR szSrc;
    BOOLEAN Overflow = FALSE;

    _SEH2_TRY
    {
        /* scan the process list */

        PSYSTEM_PROCESS_INFORMATION Spi
            = (PSYSTEM_PROCESS_INFORMATION) Buffer;

        *ReqSize = sizeof(SYSTEM_PROCESS_INFORMATION);

        /* Check for overflow */
        if (Size < sizeof(SYSTEM_PROCESS_INFORMATION))
            Overflow = TRUE;

        /* Zero user's buffer */
        if (!Overflow) RtlZeroMemory(Spi, Size);

        SystemProcess = PsIdleProcess;
        Process = SystemProcess;
        Current = (PUCHAR) Spi;

        do
        {
            SpiCurrent = (PSYSTEM_PROCESS_INFORMATION) Current;

            ThreadsCount = 0;
            CurrentEntry = Process->ThreadListHead.Flink;
            while (CurrentEntry != &Process->ThreadListHead)
            {
                ThreadsCount++;
                CurrentEntry = CurrentEntry->Flink;
            }

            // size of the structure for every process
            CurrentSize = sizeof(SYSTEM_PROCESS_INFORMATION) + sizeof(SYSTEM_THREAD_INFORMATION) * ThreadsCount;
            ImageNameLength = 0;
            Status = SeLocateProcessImageName(Process, &ProcessImageName);
            szSrc = NULL;
            if (NT_SUCCESS(Status) && (ProcessImageName->Length > 0))
            {
              szSrc = (PWCHAR)((PCHAR)ProcessImageName->Buffer + ProcessImageName->Length);
              /* Loop the file name*/
              while (szSrc > ProcessImageName->Buffer)
              {
                /* Make sure this isn't a backslash */
                if (*--szSrc == OBJ_NAME_PATH_SEPARATOR)
                {
                    szSrc++;
                    break;
                }
                else
                {
                    ImageNameLength += sizeof(WCHAR);
                }
              }
            }
            if (!ImageNameLength && Process != PsIdleProcess && Process->ImageFileName)
            {
              ImageNameLength = strlen(Process->ImageFileName) * sizeof(WCHAR);
            }

            /* Round up the image name length as NT does */
            if (ImageNameLength > 0)
                ImageNameMaximumLength = ROUND_UP(ImageNameLength + sizeof(WCHAR), 8);
            else
                ImageNameMaximumLength = 0;

            TotalSize += CurrentSize + ImageNameMaximumLength;

            /* Check for overflow */
            if (TotalSize > Size)
                Overflow = TRUE;

            /* Fill system information */
            if (!Overflow)
            {
                SpiCurrent->NextEntryOffset = CurrentSize + ImageNameMaximumLength; // relative offset to the beginnnig of the next structure
                SpiCurrent->NumberOfThreads = ThreadsCount;
                SpiCurrent->CreateTime = Process->CreateTime;
                SpiCurrent->ImageName.Length = ImageNameLength;
                SpiCurrent->ImageName.MaximumLength = ImageNameMaximumLength;
                SpiCurrent->ImageName.Buffer = (void*)(Current + CurrentSize);

                /* Copy name to the end of the struct */
                if(Process != PsIdleProcess)
                {
                    if (szSrc)
                    {
                        RtlCopyMemory(SpiCurrent->ImageName.Buffer, szSrc, SpiCurrent->ImageName.Length);

                        /* Release the memory allocated by SeLocateProcessImageName */
                        ExFreePool(ProcessImageName);
                    }
                    else if (Process->ImageFileName)
                    {
                        RtlInitAnsiString(&ImageName, Process->ImageFileName);
                        RtlAnsiStringToUnicodeString(&SpiCurrent->ImageName, &ImageName, FALSE);
                    }
                }
                else
                {
                    RtlInitUnicodeString(&SpiCurrent->ImageName, NULL);
                }

                SpiCurrent->BasePriority = Process->Pcb.BasePriority;
                SpiCurrent->UniqueProcessId = Process->UniqueProcessId;
                SpiCurrent->InheritedFromUniqueProcessId = Process->InheritedFromUniqueProcessId;
                SpiCurrent->HandleCount = ObGetProcessHandleCount(Process);
                SpiCurrent->PeakVirtualSize = Process->PeakVirtualSize;
                SpiCurrent->VirtualSize = Process->VirtualSize;
                SpiCurrent->PageFaultCount = Process->Vm.PageFaultCount;
                SpiCurrent->PeakWorkingSetSize = Process->Vm.PeakWorkingSetSize;
                SpiCurrent->WorkingSetSize = Process->Vm.WorkingSetSize;
                SpiCurrent->QuotaPeakPagedPoolUsage = Process->QuotaPeak[0];
                SpiCurrent->QuotaPagedPoolUsage = Process->QuotaUsage[0];
                SpiCurrent->QuotaPeakNonPagedPoolUsage = Process->QuotaPeak[1];
                SpiCurrent->QuotaNonPagedPoolUsage = Process->QuotaUsage[1];
                SpiCurrent->PagefileUsage = Process->QuotaUsage[2];
                SpiCurrent->PeakPagefileUsage = Process->QuotaPeak[2];
                SpiCurrent->PrivatePageCount = Process->CommitCharge;
                ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(SpiCurrent + 1);

                CurrentEntry = Process->ThreadListHead.Flink;
                while (CurrentEntry != &Process->ThreadListHead)
                {
                    CurrentThread = CONTAINING_RECORD(CurrentEntry, ETHREAD,
                        ThreadListEntry);

                    ThreadInfo->KernelTime.QuadPart = UInt32x32To64(CurrentThread->Tcb.KernelTime, KeMaximumIncrement);
                    ThreadInfo->UserTime.QuadPart = UInt32x32To64(CurrentThread->Tcb.UserTime, KeMaximumIncrement);
                    ThreadInfo->CreateTime.QuadPart = CurrentThread->CreateTime.QuadPart;
                    ThreadInfo->WaitTime = CurrentThread->Tcb.WaitTime;
                    ThreadInfo->StartAddress = (PVOID) CurrentThread->StartAddress;
                    ThreadInfo->ClientId = CurrentThread->Cid;
                    ThreadInfo->Priority = CurrentThread->Tcb.Priority;
                    ThreadInfo->BasePriority = CurrentThread->Tcb.BasePriority;
                    ThreadInfo->ContextSwitches = CurrentThread->Tcb.ContextSwitches;
                    ThreadInfo->ThreadState = CurrentThread->Tcb.State;
                    ThreadInfo->WaitReason = CurrentThread->Tcb.WaitReason;

                    ThreadInfo++;
                    CurrentEntry = CurrentEntry->Flink;
                }

                /* Query total user/kernel times of a process */
                TotalKernel = KeQueryRuntimeProcess(&Process->Pcb, &TotalUser);
                SpiCurrent->UserTime.QuadPart = UInt32x32To64(TotalUser, KeMaximumIncrement);
                SpiCurrent->KernelTime.QuadPart = UInt32x32To64(TotalKernel, KeMaximumIncrement);
            }

            /* Handle idle process entry */
            if (Process == PsIdleProcess) Process = NULL;

            Process = PsGetNextProcess(Process);
            ThreadsCount = 0;
            if ((Process == SystemProcess) || (Process == NULL))
            {
                if (!Overflow)
                    SpiCurrent->NextEntryOffset = 0;
                break;
            }
            else
                Current += CurrentSize + ImageNameMaximumLength;
          }  while ((Process != SystemProcess) && (Process != NULL));

          if(Process != NULL)
            ObDereferenceObject(Process);
          Status = STATUS_SUCCESS;
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        if(Process != NULL)
            ObDereferenceObject(Process);
        Status = _SEH2_GetExceptionCode();
    }
    _SEH2_END

    if (Overflow)
        Status = STATUS_INFO_LENGTH_MISMATCH;

    *ReqSize = TotalSize;
    return Status;
}

/* Class 6 - Call Count Information */
QSI_DEF(SystemCallCountInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemCallCountInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 7 - Device Information */
QSI_DEF(SystemDeviceInformation)
{
    PSYSTEM_DEVICE_INFORMATION Sdi
        = (PSYSTEM_DEVICE_INFORMATION) Buffer;
    PCONFIGURATION_INFORMATION ConfigInfo;

    *ReqSize = sizeof(SYSTEM_DEVICE_INFORMATION);

    /* Check user buffer's size */
    if (Size < sizeof(SYSTEM_DEVICE_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    ConfigInfo = IoGetConfigurationInformation();

    Sdi->NumberOfDisks = ConfigInfo->DiskCount;
    Sdi->NumberOfFloppies = ConfigInfo->FloppyCount;
    Sdi->NumberOfCdRoms = ConfigInfo->CdRomCount;
    Sdi->NumberOfTapes = ConfigInfo->TapeCount;
    Sdi->NumberOfSerialPorts = ConfigInfo->SerialCount;
    Sdi->NumberOfParallelPorts = ConfigInfo->ParallelCount;

    return STATUS_SUCCESS;
}

/* Class 8 - Processor Performance Information */
QSI_DEF(SystemProcessorPerformanceInformation)
{
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION Spi
        = (PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) Buffer;

    LONG i;
    ULONG TotalTime;
    PKPRCB Prcb;

    *ReqSize = KeNumberProcessors * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

    /* Check user buffer's size */
    if (Size < *ReqSize)
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    for (i = 0; i < KeNumberProcessors; i++)
    {
        /* Get the PRCB on this processor */
        Prcb = KiProcessorBlock[i];

        /* Calculate total user and kernel times */
        TotalTime = Prcb->IdleThread->KernelTime + Prcb->IdleThread->UserTime;
        Spi->IdleTime.QuadPart = UInt32x32To64(TotalTime, KeMaximumIncrement);
        Spi->KernelTime.QuadPart =  UInt32x32To64(Prcb->KernelTime, KeMaximumIncrement);
        Spi->UserTime.QuadPart = UInt32x32To64(Prcb->UserTime, KeMaximumIncrement);
        Spi->DpcTime.QuadPart = UInt32x32To64(Prcb->DpcTime, KeMaximumIncrement);
        Spi->InterruptTime.QuadPart = UInt32x32To64(Prcb->InterruptTime, KeMaximumIncrement);
        Spi->InterruptCount = Prcb->InterruptCount;
        Spi++;
    }

    return STATUS_SUCCESS;
}

/* Class 9 - Flags Information */
QSI_DEF(SystemFlagsInformation)
{
    if (sizeof(SYSTEM_FLAGS_INFORMATION) != Size)
    {
        *ReqSize = sizeof(SYSTEM_FLAGS_INFORMATION);
        return (STATUS_INFO_LENGTH_MISMATCH);
    }
    ((PSYSTEM_FLAGS_INFORMATION) Buffer)->Flags = NtGlobalFlag;
    return STATUS_SUCCESS;
}

SSI_DEF(SystemFlagsInformation)
{
    if (sizeof(SYSTEM_FLAGS_INFORMATION) != Size)
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }
    NtGlobalFlag = ((PSYSTEM_FLAGS_INFORMATION) Buffer)->Flags;
    return STATUS_SUCCESS;
}

/* Class 10 - Call Time Information */
QSI_DEF(SystemCallTimeInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemCallTimeInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 11 - Module Information */
QSI_DEF(SystemModuleInformation)
{
    NTSTATUS Status;

    /* Acquire system module list lock */
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&PsLoadedModuleResource, TRUE);

    /* Call the generic handler with the system module list */
    Status = ExpQueryModuleInformation(&PsLoadedModuleList,
                                       &MmLoadedUserImageList,
                                       (PRTL_PROCESS_MODULES)Buffer,
                                       Size,
                                       ReqSize);

    /* Release list lock and return status */
    ExReleaseResourceLite(&PsLoadedModuleResource);
    KeLeaveCriticalRegion();
    return Status;
}

/* Class 12 - Locks Information */
QSI_DEF(SystemLocksInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemLocksInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 13 - Stack Trace Information */
QSI_DEF(SystemStackTraceInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemStackTraceInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 14 - Paged Pool Information */
QSI_DEF(SystemPagedPoolInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemPagedPoolInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 15 - Non Paged Pool Information */
QSI_DEF(SystemNonPagedPoolInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemNonPagedPoolInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 16 - Handle Information */
QSI_DEF(SystemHandleInformation)
{
    PEPROCESS pr, syspr;
    ULONG curSize, i = 0;
    ULONG hCount = 0;

    PSYSTEM_HANDLE_INFORMATION Shi =
        (PSYSTEM_HANDLE_INFORMATION) Buffer;

    DPRINT("NtQuerySystemInformation - SystemHandleInformation\n");

    if (Size < sizeof(SYSTEM_HANDLE_INFORMATION))
    {
        *ReqSize = sizeof(SYSTEM_HANDLE_INFORMATION);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    DPRINT("SystemHandleInformation 1\n");

    /* First Calc Size from Count. */
    syspr = PsGetNextProcess(NULL);
    pr = syspr;

    do
    {
        hCount = hCount + ObGetProcessHandleCount(pr);
        pr = PsGetNextProcess(pr);

        if ((pr == syspr) || (pr == NULL)) break;
    }
    while ((pr != syspr) && (pr != NULL));

    if(pr != NULL)
    {
        ObDereferenceObject(pr);
    }

    DPRINT("SystemHandleInformation 2\n");

    curSize = sizeof(SYSTEM_HANDLE_INFORMATION) +
                     ((sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO) * hCount) -
                     (sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO)));

    Shi->NumberOfHandles = hCount;

    if (curSize > Size)
    {
        *ReqSize = curSize;
        return (STATUS_INFO_LENGTH_MISMATCH);
    }

    DPRINT("SystemHandleInformation 3\n");

    /* Now get Handles from all processs. */
    syspr = PsGetNextProcess(NULL);
    pr = syspr;

    do
    {
        int Count = 0, HandleCount;

        HandleCount = ObGetProcessHandleCount(pr);

        for (Count = 0; HandleCount > 0 ; HandleCount--)
        {
            Shi->Handles[i].UniqueProcessId = (USHORT)(ULONG_PTR)pr->UniqueProcessId;
            Count++;
            i++;
        }

        pr = PsGetNextProcess(pr);

        if ((pr == syspr) || (pr == NULL)) break;
    }
    while ((pr != syspr) && (pr != NULL));

    if(pr != NULL) ObDereferenceObject(pr);

    DPRINT("SystemHandleInformation 4\n");
    return STATUS_SUCCESS;

}
/*
SSI_DEF(SystemHandleInformation)
{

    return STATUS_SUCCESS;
}
*/

/* Class 17 -  Information */
QSI_DEF(SystemObjectInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemObjectInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 18 -  Information */
QSI_DEF(SystemPageFileInformation)
{
    UNICODE_STRING FileName; /* FIXME */
    SYSTEM_PAGEFILE_INFORMATION *Spfi = (SYSTEM_PAGEFILE_INFORMATION *) Buffer;

    if (Size < sizeof(SYSTEM_PAGEFILE_INFORMATION))
    {
        * ReqSize = sizeof(SYSTEM_PAGEFILE_INFORMATION);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    RtlInitUnicodeString(&FileName, NULL); /* FIXME */

    /* FIXME */
    Spfi->NextEntryOffset = 0;

    Spfi->TotalSize = MiFreeSwapPages + MiUsedSwapPages;
    Spfi->TotalInUse = MiUsedSwapPages;
    Spfi->PeakUsage = MiUsedSwapPages; /* FIXME */
    Spfi->PageFileName = FileName;
    return STATUS_SUCCESS;
}

/* Class 19 - Vdm Instemul Information */
QSI_DEF(SystemVdmInstemulInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemVdmInstemulInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 20 - Vdm Bop Information */
QSI_DEF(SystemVdmBopInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemVdmBopInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 21 - File Cache Information */
QSI_DEF(SystemFileCacheInformation)
{
    SYSTEM_FILECACHE_INFORMATION *Sci = (SYSTEM_FILECACHE_INFORMATION *) Buffer;

    if (Size < sizeof(SYSTEM_FILECACHE_INFORMATION))
    {
        *ReqSize = sizeof(SYSTEM_FILECACHE_INFORMATION);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    RtlZeroMemory(Sci, sizeof(SYSTEM_FILECACHE_INFORMATION));

    /* Return the Byte size not the page size. */
    Sci->CurrentSize =
        MiMemoryConsumers[MC_CACHE].PagesUsed * PAGE_SIZE;
    Sci->PeakSize =
            MiMemoryConsumers[MC_CACHE].PagesUsed * PAGE_SIZE; /* FIXME */
    /* Taskmgr multiplies this one by page size right away */
    Sci->CurrentSizeIncludingTransitionInPages =
        MiMemoryConsumers[MC_CACHE].PagesUsed; /* FIXME: Should be */
    /* system working set and standby pages. */
    Sci->PageFaultCount = 0; /* FIXME */
    Sci->MinimumWorkingSet = 0; /* FIXME */
    Sci->MaximumWorkingSet = 0; /* FIXME */

    return STATUS_SUCCESS;
}

SSI_DEF(SystemFileCacheInformation)
{
    if (Size < sizeof(SYSTEM_FILECACHE_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemFileCacheInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 22 - Pool Tag Information */
QSI_DEF(SystemPoolTagInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemPoolTagInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 23 - Interrupt Information for all processors */
QSI_DEF(SystemInterruptInformation)
{
    PKPRCB Prcb;
    LONG i;
    ULONG ti;
    PSYSTEM_INTERRUPT_INFORMATION sii = (PSYSTEM_INTERRUPT_INFORMATION)Buffer;

    if(Size < KeNumberProcessors * sizeof(SYSTEM_INTERRUPT_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    ti = KeQueryTimeIncrement();

    for (i = 0; i < KeNumberProcessors; i++)
    {
        Prcb = KiProcessorBlock[i];
        sii->ContextSwitches = KeGetContextSwitches(Prcb);
        sii->DpcCount = Prcb->DpcData[0].DpcCount;
        sii->DpcRate = Prcb->DpcRequestRate;
        sii->TimeIncrement = ti;
        sii->DpcBypassCount = 0;
        sii->ApcBypassCount = 0;
        sii++;
    }

    return STATUS_SUCCESS;
}

/* Class 24 - DPC Behaviour Information */
QSI_DEF(SystemDpcBehaviourInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemDpcBehaviourInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

SSI_DEF(SystemDpcBehaviourInformation)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemDpcBehaviourInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 25 - Full Memory Information */
QSI_DEF(SystemFullMemoryInformation)
{
    PULONG Spi = (PULONG) Buffer;

    PEPROCESS TheIdleProcess;

    *ReqSize = sizeof(ULONG);

    if (sizeof(ULONG) != Size)
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    DPRINT("SystemFullMemoryInformation\n");

    TheIdleProcess = PsIdleProcess;

    DPRINT("PID: %d, KernelTime: %u PFFree: %d PFUsed: %d\n",
           TheIdleProcess->UniqueProcessId,
           TheIdleProcess->Pcb.KernelTime,
           MiFreeSwapPages,
           MiUsedSwapPages);

#ifndef NDEBUG
    MmPrintMemoryStatistic();
#endif

    *Spi = MiMemoryConsumers[MC_USER].PagesUsed;

    return STATUS_SUCCESS;
}

/* Class 26 - Load Image */
SSI_DEF(SystemLoadGdiDriverInformation)
{
    PSYSTEM_GDI_DRIVER_INFORMATION DriverInfo = (PVOID)Buffer;
    UNICODE_STRING ImageName;
    PVOID ImageBase;
    PVOID SectionPointer;
    ULONG_PTR EntryPoint;
    NTSTATUS Status;
    ULONG DirSize;
    PIMAGE_NT_HEADERS NtHeader;

    /* Validate size */
    if (Size != sizeof(SYSTEM_GDI_DRIVER_INFORMATION))
    {
        /* Incorrect buffer length, fail */
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Only kernel mode can call this function */
    if (ExGetPreviousMode() != KernelMode) return STATUS_PRIVILEGE_NOT_HELD;

    /* Load the driver */
    ImageName = DriverInfo->DriverName;
    Status = MmLoadSystemImage(&ImageName,
                               NULL,
                               NULL,
                               0,
                               &SectionPointer,
                               &ImageBase);
    if (!NT_SUCCESS(Status)) return Status;

    /* Return the export pointer */
    DriverInfo->ExportSectionPointer =
        RtlImageDirectoryEntryToData(ImageBase,
                                     TRUE,
                                     IMAGE_DIRECTORY_ENTRY_EXPORT,
                                     &DirSize);

    /* Get the entrypoint */
    NtHeader = RtlImageNtHeader(ImageBase);
    EntryPoint = NtHeader->OptionalHeader.AddressOfEntryPoint;
    EntryPoint += (ULONG_PTR)ImageBase;

    /* Save other data */
    DriverInfo->ImageAddress = ImageBase;
    DriverInfo->SectionPointer = SectionPointer;
    DriverInfo->EntryPoint = (PVOID)EntryPoint;
    DriverInfo->ImageLength = NtHeader->OptionalHeader.SizeOfImage;

    /* All is good */
    return STATUS_SUCCESS;
}

/* Class 27 - Unload Image */
SSI_DEF(SystemUnloadGdiDriverInformation)
{
    PVOID SectionPointer = Buffer;

    /* Validate size */
    if (Size != sizeof(PVOID))
    {
        /* Incorrect length, fail */
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Only kernel mode can call this function */
    if (ExGetPreviousMode() != KernelMode) return STATUS_PRIVILEGE_NOT_HELD;

    /* Unload the image */
    MmUnloadSystemImage(SectionPointer);
    return STATUS_SUCCESS;
}

/* Class 28 - Time Adjustment Information */
QSI_DEF(SystemTimeAdjustmentInformation)
{
    PSYSTEM_QUERY_TIME_ADJUST_INFORMATION TimeInfo =
        (PSYSTEM_QUERY_TIME_ADJUST_INFORMATION)Buffer;

    /* Check if enough storage was provided */
    if (sizeof(SYSTEM_QUERY_TIME_ADJUST_INFORMATION) > Size)
    {
        * ReqSize = sizeof(SYSTEM_QUERY_TIME_ADJUST_INFORMATION);
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Give time values to our caller */
    TimeInfo->TimeIncrement = KeMaximumIncrement;
    TimeInfo->TimeAdjustment = KeTimeAdjustment;
    TimeInfo->Enable = TRUE;

    return STATUS_SUCCESS;
}

SSI_DEF(SystemTimeAdjustmentInformation)
{
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    /*PSYSTEM_SET_TIME_ADJUST_INFORMATION TimeInfo =
        (PSYSTEM_SET_TIME_ADJUST_INFORMATION)Buffer;*/

    /* Check size of a buffer, it must match our expectations */
    if (sizeof(SYSTEM_SET_TIME_ADJUST_INFORMATION) != Size)
        return STATUS_INFO_LENGTH_MISMATCH;

    /* Check who is calling */
    if (PreviousMode != KernelMode)
    {
        /* Check access rights */
        if (!SeSinglePrivilegeCheck(SeSystemtimePrivilege, PreviousMode))
        {
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

    /* TODO: Set time adjustment information */
    DPRINT1("Setting of SystemTimeAdjustmentInformation is not implemented yet!\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 29 - Summary Memory Information */
QSI_DEF(SystemSummaryMemoryInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemSummaryMemoryInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 30 - Next Event Id Information */
QSI_DEF(SystemNextEventIdInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemNextEventIdInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 31 - Event Ids Information */
QSI_DEF(SystemEventIdsInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemEventIdsInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 32 - Crash Dump Information */
QSI_DEF(SystemCrashDumpInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemCrashDumpInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 33 - Exception Information */
QSI_DEF(SystemExceptionInformation)
{
    PSYSTEM_EXCEPTION_INFORMATION ExceptionInformation =
        (PSYSTEM_EXCEPTION_INFORMATION)Buffer;
    PKPRCB Prcb;
    ULONG AlignmentFixupCount = 0, ExceptionDispatchCount = 0;
    ULONG FloatingEmulationCount = 0, ByteWordEmulationCount = 0;
    CHAR i;

    /* Check size of a buffer, it must match our expectations */
    if (sizeof(SYSTEM_EXCEPTION_INFORMATION) != Size)
        return STATUS_INFO_LENGTH_MISMATCH;

    /* Sum up exception count information from all processors */
    for (i = 0; i < KeNumberProcessors; i++)
    {
        Prcb = KiProcessorBlock[i];
        if (Prcb)
        {
            AlignmentFixupCount += Prcb->KeAlignmentFixupCount;
            ExceptionDispatchCount += Prcb->KeExceptionDispatchCount;
            FloatingEmulationCount += Prcb->KeFloatingEmulationCount;
        }
    }

    /* Save information in user's buffer */
    ExceptionInformation->AlignmentFixupCount = AlignmentFixupCount;
    ExceptionInformation->ExceptionDispatchCount = ExceptionDispatchCount;
    ExceptionInformation->FloatingEmulationCount = FloatingEmulationCount;
    ExceptionInformation->ByteWordEmulationCount = ByteWordEmulationCount;

    return STATUS_SUCCESS;
}

/* Class 34 - Crash Dump State Information */
QSI_DEF(SystemCrashDumpStateInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemCrashDumpStateInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 35 - Kernel Debugger Information */
QSI_DEF(SystemKernelDebuggerInformation)
{
    PSYSTEM_KERNEL_DEBUGGER_INFORMATION skdi = (PSYSTEM_KERNEL_DEBUGGER_INFORMATION) Buffer;

    *ReqSize = sizeof(SYSTEM_KERNEL_DEBUGGER_INFORMATION);
    if (Size < sizeof(SYSTEM_KERNEL_DEBUGGER_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    skdi->KernelDebuggerEnabled = KD_DEBUGGER_ENABLED;
    skdi->KernelDebuggerNotPresent = KD_DEBUGGER_NOT_PRESENT;

    return STATUS_SUCCESS;
}

/* Class 36 - Context Switch Information */
QSI_DEF(SystemContextSwitchInformation)
{
    PSYSTEM_CONTEXT_SWITCH_INFORMATION ContextSwitchInformation =
        (PSYSTEM_CONTEXT_SWITCH_INFORMATION)Buffer;
    ULONG ContextSwitches;
    PKPRCB Prcb;
    CHAR i;

    /* Check size of a buffer, it must match our expectations */
    if (sizeof(SYSTEM_CONTEXT_SWITCH_INFORMATION) != Size)
        return STATUS_INFO_LENGTH_MISMATCH;

    /* Calculate total value of context switches across all processors */
    ContextSwitches = 0;
    for (i = 0; i < KeNumberProcessors; i ++)
    {
        Prcb = KiProcessorBlock[i];
        if (Prcb)
        {
            ContextSwitches += KeGetContextSwitches(Prcb);
        }
    }

    ContextSwitchInformation->ContextSwitches = ContextSwitches;

    /* FIXME */
    ContextSwitchInformation->FindAny = 0;
    ContextSwitchInformation->FindLast = 0;
    ContextSwitchInformation->FindIdeal = 0;
    ContextSwitchInformation->IdleAny = 0;
    ContextSwitchInformation->IdleCurrent = 0;
    ContextSwitchInformation->IdleLast = 0;
    ContextSwitchInformation->IdleIdeal = 0;
    ContextSwitchInformation->PreemptAny = 0;
    ContextSwitchInformation->PreemptCurrent = 0;
    ContextSwitchInformation->PreemptLast = 0;
    ContextSwitchInformation->SwitchToIdle = 0;

    return STATUS_SUCCESS;
}

/* Class 37 - Registry Quota Information */
QSI_DEF(SystemRegistryQuotaInformation)
{
    PSYSTEM_REGISTRY_QUOTA_INFORMATION srqi = (PSYSTEM_REGISTRY_QUOTA_INFORMATION) Buffer;

    *ReqSize = sizeof(SYSTEM_REGISTRY_QUOTA_INFORMATION);
    if (Size < sizeof(SYSTEM_REGISTRY_QUOTA_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    DPRINT1("Faking max registry size of 32 MB\n");
    srqi->RegistryQuotaAllowed = 0x2000000;
    srqi->RegistryQuotaUsed = 0x200000;
    srqi->PagedPoolSize = 0x200000;

    return STATUS_SUCCESS;
}

SSI_DEF(SystemRegistryQuotaInformation)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemRegistryQuotaInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 38 - Load And Call Image */
SSI_DEF(SystemExtendServiceTableInformation)
{
    UNICODE_STRING ImageName;
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    PLDR_DATA_TABLE_ENTRY ModuleObject;
    NTSTATUS Status;
    PIMAGE_NT_HEADERS NtHeader;
    DRIVER_OBJECT Win32k;
    PDRIVER_INITIALIZE DriverInit;
    PVOID ImageBase;
    ULONG_PTR EntryPoint;

    /* Validate the size */
    if (Size != sizeof(UNICODE_STRING)) return STATUS_INFO_LENGTH_MISMATCH;

    /* Check who is calling */
    if (PreviousMode != KernelMode)
    {
        /* Make sure we can load drivers */
        if (!SeSinglePrivilegeCheck(SeLoadDriverPrivilege, UserMode))
        {
            /* FIXME: We can't, fail */
            //return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

    /* Probe and capture the driver name */
    ProbeAndCaptureUnicodeString(&ImageName, PreviousMode, Buffer);

    /* Load the image */
    Status = MmLoadSystemImage(&ImageName,
                               NULL,
                               NULL,
                               0,
                               (PVOID)&ModuleObject,
                               &ImageBase);

    /* Release String */
    ReleaseCapturedUnicodeString(&ImageName, PreviousMode);

    if (!NT_SUCCESS(Status)) return Status;

    /* Get the headers */
    NtHeader = RtlImageNtHeader(ImageBase);
    if (!NtHeader)
    {
        /* Fail */
        MmUnloadSystemImage(ModuleObject);
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    /* Get the entrypoint */
    EntryPoint = NtHeader->OptionalHeader.AddressOfEntryPoint;
    EntryPoint += (ULONG_PTR)ImageBase;
    DriverInit = (PDRIVER_INITIALIZE)EntryPoint;

    /* Create a dummy device */
    RtlZeroMemory(&Win32k, sizeof(Win32k));
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    Win32k.DriverStart = ImageBase;

    /* Call it */
    Status = (DriverInit)(&Win32k, NULL);
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    /* Unload if we failed */
    if (!NT_SUCCESS(Status)) MmUnloadSystemImage(ModuleObject);
    return Status;
}

/* Class 39 - Priority Separation */
SSI_DEF(SystemPrioritySeperation)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemPrioritySeperation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 40 - Plug Play Bus Information */
QSI_DEF(SystemPlugPlayBusInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemPlugPlayBusInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 41 - Dock Information */
QSI_DEF(SystemDockInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemDockInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 42 - Power Information */
QSI_DEF(SystemPowerInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemPowerInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 43 - Processor Speed Information */
QSI_DEF(SystemProcessorSpeedInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemProcessorSpeedInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}

/* Class 44 - Current Time Zone Information */
QSI_DEF(SystemCurrentTimeZoneInformation)
{
    *ReqSize = sizeof(TIME_ZONE_INFORMATION);

    if (sizeof(TIME_ZONE_INFORMATION) != Size)
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    /* Copy the time zone information struct */
    memcpy(Buffer,
           &ExpTimeZoneInfo,
           sizeof(TIME_ZONE_INFORMATION));

    return STATUS_SUCCESS;
}


SSI_DEF(SystemCurrentTimeZoneInformation)
{
    /* Check user buffer's size */
    if (Size < sizeof(TIME_ZONE_INFORMATION))
    {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    return ExpSetTimeZoneInformation((PTIME_ZONE_INFORMATION)Buffer);
}


/* Class 45 - Lookaside Information */
QSI_DEF(SystemLookasideInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemLookasideInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 46 - Set time slip event */
SSI_DEF(SystemSetTimeSlipEvent)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemSetTimSlipEvent not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 47 - Create a new session (TSE) */
SSI_DEF(SystemCreateSession)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemCreateSession not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 48 - Delete an existing session (TSE) */
SSI_DEF(SystemDeleteSession)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemDeleteSession not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 49 - UNKNOWN */
QSI_DEF(SystemInvalidInfoClass4)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemInvalidInfoClass4 not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 50 - System range start address */
QSI_DEF(SystemRangeStartInformation)
{
    /* Check user buffer's size */
    if (Size != sizeof(ULONG_PTR)) return STATUS_INFO_LENGTH_MISMATCH;

    *(PULONG_PTR)Buffer = (ULONG_PTR)MmSystemRangeStart;
    
    if (ReqSize) *ReqSize = sizeof(ULONG_PTR);
    
    return STATUS_SUCCESS;
}

/* Class 51 - Driver verifier information */
QSI_DEF(SystemVerifierInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemVerifierInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


SSI_DEF(SystemVerifierInformation)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemVerifierInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 52 - Add a driver verifier */
SSI_DEF(SystemAddVerifier)
{
    /* FIXME */
    DPRINT1("NtSetSystemInformation - SystemAddVerifier not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Class 53 - A session's processes  */
QSI_DEF(SystemSessionProcessesInformation)
{
    /* FIXME */
    DPRINT1("NtQuerySystemInformation - SystemSessionProcessInformation not implemented\n");
    return STATUS_NOT_IMPLEMENTED;
}


/* Query/Set Calls Table */
typedef
struct _QSSI_CALLS
{
    NTSTATUS (* Query) (PVOID,ULONG,PULONG);
    NTSTATUS (* Set) (PVOID,ULONG);

} QSSI_CALLS;

// QS    Query & Set
// QX    Query
// XS    Set
// XX    unknown behaviour
//
#define SI_QS(n) {QSI_USE(n),SSI_USE(n)}
#define SI_QX(n) {QSI_USE(n),NULL}
#define SI_XS(n) {NULL,SSI_USE(n)}
#define SI_XX(n) {NULL,NULL}

static
QSSI_CALLS
CallQS [] =
{
    SI_QX(SystemBasicInformation),
    SI_QX(SystemProcessorInformation),
    SI_QX(SystemPerformanceInformation),
    SI_QX(SystemTimeOfDayInformation),
    SI_QX(SystemPathInformation), /* should be SI_XX */
    SI_QX(SystemProcessInformation),
    SI_QX(SystemCallCountInformation),
    SI_QX(SystemDeviceInformation),
    SI_QX(SystemProcessorPerformanceInformation),
    SI_QS(SystemFlagsInformation),
    SI_QX(SystemCallTimeInformation), /* should be SI_XX */
    SI_QX(SystemModuleInformation),
    SI_QX(SystemLocksInformation),
    SI_QX(SystemStackTraceInformation), /* should be SI_XX */
    SI_QX(SystemPagedPoolInformation), /* should be SI_XX */
    SI_QX(SystemNonPagedPoolInformation), /* should be SI_XX */
    SI_QX(SystemHandleInformation),
    SI_QX(SystemObjectInformation),
    SI_QX(SystemPageFileInformation),
    SI_QX(SystemVdmInstemulInformation),
    SI_QX(SystemVdmBopInformation), /* it should be SI_XX */
    SI_QS(SystemFileCacheInformation),
    SI_QX(SystemPoolTagInformation),
    SI_QX(SystemInterruptInformation),
    SI_QS(SystemDpcBehaviourInformation),
    SI_QX(SystemFullMemoryInformation), /* it should be SI_XX */
    SI_XS(SystemLoadGdiDriverInformation),
    SI_XS(SystemUnloadGdiDriverInformation),
    SI_QS(SystemTimeAdjustmentInformation),
    SI_QX(SystemSummaryMemoryInformation), /* it should be SI_XX */
    SI_QX(SystemNextEventIdInformation), /* it should be SI_XX */
    SI_QX(SystemEventIdsInformation), /* it should be SI_XX */
    SI_QX(SystemCrashDumpInformation),
    SI_QX(SystemExceptionInformation),
    SI_QX(SystemCrashDumpStateInformation),
    SI_QX(SystemKernelDebuggerInformation),
    SI_QX(SystemContextSwitchInformation),
    SI_QS(SystemRegistryQuotaInformation),
    SI_XS(SystemExtendServiceTableInformation),
    SI_XS(SystemPrioritySeperation),
    SI_QX(SystemPlugPlayBusInformation), /* it should be SI_XX */
    SI_QX(SystemDockInformation), /* it should be SI_XX */
    SI_QX(SystemPowerInformation), /* it should be SI_XX */
    SI_QX(SystemProcessorSpeedInformation), /* it should be SI_XX */
    SI_QS(SystemCurrentTimeZoneInformation), /* it should be SI_QX */
    SI_QX(SystemLookasideInformation),
    SI_XS(SystemSetTimeSlipEvent),
    SI_XS(SystemCreateSession),
    SI_XS(SystemDeleteSession),
    SI_QX(SystemInvalidInfoClass4), /* it should be SI_XX */
    SI_QX(SystemRangeStartInformation),
    SI_QS(SystemVerifierInformation),
    SI_XS(SystemAddVerifier),
    SI_QX(SystemSessionProcessesInformation)
};

C_ASSERT(SystemBasicInformation == 0);
#define MIN_SYSTEM_INFO_CLASS (SystemBasicInformation)
#define MAX_SYSTEM_INFO_CLASS (sizeof(CallQS) / sizeof(CallQS[0]))

/*
 * @implemented
 */
NTSTATUS NTAPI
NtQuerySystemInformation(IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
                         OUT PVOID SystemInformation,
                         IN ULONG Length,
                         OUT PULONG UnsafeResultLength)
{
    KPROCESSOR_MODE PreviousMode;
    ULONG ResultLength;
    NTSTATUS FStatus = STATUS_NOT_IMPLEMENTED;

    PAGED_CODE();

    PreviousMode = ExGetPreviousMode();

    _SEH2_TRY
    {
        if (PreviousMode != KernelMode)
        {
            /* SystemKernelDebuggerInformation needs only BOOLEAN alignment */
            ProbeForWrite(SystemInformation, Length, 1);
            if (UnsafeResultLength != NULL)
                ProbeForWriteUlong(UnsafeResultLength);
        }

        /*
         * Check the request is valid.
         */
        if (SystemInformationClass >= MAX_SYSTEM_INFO_CLASS)
        {
            _SEH2_YIELD(return STATUS_INVALID_INFO_CLASS);
        }

        if (NULL != CallQS [SystemInformationClass].Query)
        {
            /*
             * Hand the request to a subhandler.
             */
            FStatus = CallQS [SystemInformationClass].Query(SystemInformation,
                                                            Length,
                                                            &ResultLength);
            if (UnsafeResultLength != NULL)
            {
                if (PreviousMode != KernelMode)
                {
                    *UnsafeResultLength = ResultLength;
                }
                else
                {
                    *UnsafeResultLength = ResultLength;
                }
            }
        }
    }
    _SEH2_EXCEPT(ExSystemExceptionFilter())
    {
        FStatus = _SEH2_GetExceptionCode();
    }
    _SEH2_END;

    return FStatus;
}


NTSTATUS
NTAPI
NtSetSystemInformation (IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
                        IN PVOID SystemInformation,
                        IN ULONG SystemInformationLength)
{
    PAGED_CODE();

    /*
     * If called from user mode, check
     * possible unsafe arguments.
     */
#if 0
    if (KernelMode != KeGetPreviousMode())
    {
        // Check arguments
        //ProbeForWrite(
        //    SystemInformation,
        //    Length
        //    );
        //ProbeForWrite(
        //    ResultLength,
        //    sizeof (ULONG)
        //    );
    }
#endif
    /*
     * Check the request is valid.
     */
    if ((SystemInformationClass >= MIN_SYSTEM_INFO_CLASS) &&
        (SystemInformationClass < MAX_SYSTEM_INFO_CLASS))
    {
        if (NULL != CallQS [SystemInformationClass].Set)
        {
            /*
             * Hand the request to a subhandler.
             */
            return CallQS [SystemInformationClass].Set(SystemInformation,
                                                       SystemInformationLength);
        }
    }

    return STATUS_INVALID_INFO_CLASS;
}

NTSTATUS
NTAPI
NtFlushInstructionCache(IN HANDLE ProcessHandle,
                        IN PVOID BaseAddress,
                        IN ULONG NumberOfBytesToFlush)
{
    PAGED_CODE();

#if defined(_M_IX86) || defined(_M_AMD64)
    __wbinvd();
#elif defined(_M_PPC)
    __asm__ __volatile__("tlbsync");
#elif defined(_M_MIPS)
    DPRINT1("NtFlushInstructionCache() is not implemented\n");
    for (;;);
#elif defined(_M_ARM)
    __asm__ __volatile__("mov r1, #0; mcr p15, 0, r1, c7, c5, 0");
#else
#error Unknown architecture
#endif
    return STATUS_SUCCESS;
}

ULONG
NTAPI
NtGetCurrentProcessorNumber(VOID)
{
    /* Just return the CPU */
    return KeGetCurrentProcessorNumber();
}

/*
 * @implemented
 */
#undef ExGetPreviousMode
KPROCESSOR_MODE
NTAPI
ExGetPreviousMode (VOID)
{
    return KeGetPreviousMode();
}
