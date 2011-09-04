/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey NT User-Mode Library
 * FILE:            dll/ntdll/ldr/ldrutils.c
 * PURPOSE:         Internal Loader Utility Functions
 * PROGRAMMERS:     Alex Ionescu (alex@relsoft.net)
 *                  Aleksey Bragin (aleksey@odyssey.org)
 */

/* INCLUDES *****************************************************************/

#include <ntdll.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS *******************************************************************/

PLDR_DATA_TABLE_ENTRY LdrpLoadedDllHandleCache, LdrpGetModuleHandleCache;
BOOLEAN g_ShimsEnabled;

/* FUNCTIONS *****************************************************************/

NTSTATUS
NTAPI
LdrpAllocateUnicodeString(IN OUT PUNICODE_STRING StringOut,
                          IN ULONG Length)
{
    /* Sanity checks */
    ASSERT(StringOut);
    ASSERT(Length <= UNICODE_STRING_MAX_BYTES);

    /* Assume failure */
    StringOut->Length = 0;

    /* Make sure it's not mis-aligned */
    if (Length & 1)
    {
        /* Fail */
        StringOut->Buffer = NULL;
        StringOut->MaximumLength = 0;
        return STATUS_INVALID_PARAMETER;
    }

    /* Allocate the string*/
    StringOut->Buffer = RtlAllocateHeap(RtlGetProcessHeap(),
                                        0,
                                        StringOut->Length + sizeof(WCHAR));
    if (!StringOut->Buffer)
    {
        /* Fail */
        StringOut->MaximumLength = 0;
        return STATUS_NO_MEMORY;
    }

    /* Null-terminate it */
    StringOut->Buffer[StringOut->Length / sizeof(WCHAR)] = UNICODE_NULL;

    /* Check if this is a maximum-sized string */
    if (StringOut->Length != UNICODE_STRING_MAX_BYTES)
    {
        /* It's not, so set the maximum length to be one char more */
        StringOut->MaximumLength = StringOut->Length + sizeof(UNICODE_NULL);
    }
    else
    {
        /* The length is already the maximum possible */
        StringOut->MaximumLength = UNICODE_STRING_MAX_BYTES;
    }

    /* Return success */
    return STATUS_SUCCESS;
}

VOID
NTAPI
LdrpFreeUnicodeString(IN PUNICODE_STRING StringIn)
{
    ASSERT(StringIn != NULL);

    /* If Buffer is not NULL - free it */
    if (StringIn->Buffer)
    {
        RtlFreeHeap(RtlGetProcessHeap(), 0, StringIn->Buffer);
    }

    /* Zero it out */
    RtlInitEmptyUnicodeString(StringIn, NULL, 0);
}
BOOLEAN
NTAPI
LdrpCallInitRoutine(IN PDLL_INIT_ROUTINE EntryPoint,
                    IN PVOID BaseAddress,
                    IN ULONG Reason,
                    IN PVOID Context)
{
    /* Call the entry */
    return EntryPoint(BaseAddress, Reason, Context);
}

/* NOTE: This function is broken */
VOID
NTAPI
LdrpUpdateLoadCount3(IN PLDR_DATA_TABLE_ENTRY LdrEntry,
                     IN ULONG Flags,
                     OUT PUNICODE_STRING UpdateString)
{
    PIMAGE_BOUND_FORWARDER_REF NewImportForwarder;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR BoundEntry;
    PIMAGE_IMPORT_DESCRIPTOR ImportEntry;
    PIMAGE_THUNK_DATA FirstThunk;
    PLDR_DATA_TABLE_ENTRY Entry;
    PUNICODE_STRING ImportNameUnic;
    ANSI_STRING ImportNameAnsi;
    LPSTR ImportName;
    ULONG ImportSize;
    NTSTATUS Status;
    ULONG i;

    /* Check the action we need to perform */
    if (Flags == LDRP_UPDATE_REFCOUNT)
    {
        /* Make sure entry is not being loaded already */
        if (LdrEntry->Flags & LDRP_LOAD_IN_PROGRESS)
            return;

        LdrEntry->Flags |= LDRP_LOAD_IN_PROGRESS;
    }
    else if (Flags == LDRP_UPDATE_DEREFCOUNT)
    {
        /* Make sure the entry is not being unloaded already */
        if (LdrEntry->Flags & LDRP_UNLOAD_IN_PROGRESS)
            return;

        LdrEntry->Flags |= LDRP_UNLOAD_IN_PROGRESS;
    }

    /* Go through all bound DLLs and dereference them */
    ImportNameUnic = &NtCurrentTeb()->StaticUnicodeString;

    /* Try to get the new import entry */
    BoundEntry = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(LdrEntry->DllBase,
                                                                              TRUE,
                                                                              IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT,
                                                                              &ImportSize);

    if (BoundEntry)
    {
        /* Set entry flags if refing/derefing */
        if (Flags == LDRP_UPDATE_REFCOUNT)
            LdrEntry->Flags |= LDRP_LOAD_IN_PROGRESS;
        else if (Flags == LDRP_UPDATE_DEREFCOUNT)
            LdrEntry->Flags |= LDRP_UNLOAD_IN_PROGRESS;

        while (BoundEntry->OffsetModuleName)
        {
            /* Get pointer to the current import name */
            ImportName = (PCHAR)BoundEntry + BoundEntry->OffsetModuleName;

            RtlInitAnsiString(&ImportNameAnsi, ImportName);
            Status = RtlAnsiStringToUnicodeString(ImportNameUnic, &ImportNameAnsi, FALSE);

            if (NT_SUCCESS(Status))
            {
                if (LdrpCheckForLoadedDll(NULL,
                                          ImportNameUnic,
                                          TRUE,
                                          FALSE,
                                          &Entry))
                {
                    if (Entry->LoadCount != 0xFFFF)
                    {
                        /* Perform the required action */
                        switch (Flags)
                        {
                        case LDRP_UPDATE_REFCOUNT:
                            Entry->LoadCount++;
                            break;
                        case LDRP_UPDATE_DEREFCOUNT:
                            Entry->LoadCount--;
                            break;
                        case LDRP_UPDATE_PIN:
                            Entry->LoadCount = 0xFFFF;
                            break;
                        }

                        /* Show snaps */
                        if (ShowSnaps)
                        {
                            DPRINT1("LDR: Flags %d  %wZ (%lx)\n", Flags, ImportNameUnic, Entry->LoadCount);
                        }
                    }

                    /* Recurse into this entry */
                    LdrpUpdateLoadCount3(Entry, Flags, UpdateString);
                }
            }

            /* Go through forwarders */
            NewImportForwarder = (PIMAGE_BOUND_FORWARDER_REF)(BoundEntry + 1);
            for (i=0; i<BoundEntry->NumberOfModuleForwarderRefs; i++)
            {
                ImportName = (PCHAR)BoundEntry + NewImportForwarder->OffsetModuleName;

                RtlInitAnsiString(&ImportNameAnsi, ImportName);
                Status = RtlAnsiStringToUnicodeString(ImportNameUnic, &ImportNameAnsi, FALSE);
                if (NT_SUCCESS(Status))
                {
                    if (LdrpCheckForLoadedDll(NULL,
                                              ImportNameUnic,
                                              TRUE,
                                              FALSE,
                                              &Entry))
                    {
                        if (Entry->LoadCount != 0xFFFF)
                        {
                            /* Perform the required action */
                            switch (Flags)
                            {
                            case LDRP_UPDATE_REFCOUNT:
                                Entry->LoadCount++;
                                break;
                            case LDRP_UPDATE_DEREFCOUNT:
                                Entry->LoadCount--;
                                break;
                            case LDRP_UPDATE_PIN:
                                Entry->LoadCount = 0xFFFF;
                                break;
                            }

                            /* Show snaps */
                            if (ShowSnaps)
                            {
                                DPRINT1("LDR: Flags %d  %wZ (%lx)\n", Flags, ImportNameUnic, Entry->LoadCount);
                            }
                        }

                        /* Recurse into this entry */
                        LdrpUpdateLoadCount3(Entry, Flags, UpdateString);
                    }
                }

                NewImportForwarder++;
            }

            BoundEntry = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)NewImportForwarder;
        }

        /* We're done */
        return;
    }

    /* Check oldstyle import descriptor */
    ImportEntry = (PIMAGE_IMPORT_DESCRIPTOR)RtlImageDirectoryEntryToData(LdrEntry->DllBase,
                                                                         TRUE,
                                                                         IMAGE_DIRECTORY_ENTRY_IMPORT,
                                                                         &ImportSize);
    if (ImportEntry)
    {
        /* There is old one, so go through its entries */
        while (ImportEntry->Name && ImportEntry->FirstThunk)
        {
            FirstThunk = (PIMAGE_THUNK_DATA)((ULONG_PTR)LdrEntry->DllBase + ImportEntry->FirstThunk);

            /* Skip this entry if needed */
            if (!FirstThunk->u1.Function)
            {
                ImportEntry++;
                continue;
            }

            ImportName = (PSZ)((ULONG_PTR)LdrEntry->DllBase + ImportEntry->Name);

            RtlInitAnsiString(&ImportNameAnsi, ImportName);
            Status = RtlAnsiStringToUnicodeString(ImportNameUnic, &ImportNameAnsi, FALSE);
            if (NT_SUCCESS(Status))
            {
                if (LdrpCheckForLoadedDll(NULL,
                                          ImportNameUnic,
                                          TRUE,
                                          FALSE,
                                          &Entry))
                {
                    if (Entry->LoadCount != 0xFFFF)
                    {
                        /* Perform the required action */
                        switch (Flags)
                        {
                        case LDRP_UPDATE_REFCOUNT:
                            Entry->LoadCount++;
                            break;
                        case LDRP_UPDATE_DEREFCOUNT:
                            Entry->LoadCount--;
                            break;
                        case LDRP_UPDATE_PIN:
                            Entry->LoadCount = 0xFFFF;
                            break;
                        }

                        /* Show snaps */
                        if (ShowSnaps)
                        {
                            DPRINT1("LDR: Flags %d  %wZ (%lx)\n", Flags, ImportNameUnic, Entry->LoadCount);
                        }
                    }

                    /* Recurse */
                    LdrpUpdateLoadCount3(Entry, Flags, UpdateString);
                }
            }

            /* Go to the next entry */
            ImportEntry++;
        }
    }
}

VOID
NTAPI
LdrpUpdateLoadCount2(IN PLDR_DATA_TABLE_ENTRY LdrEntry,
                     IN ULONG Flags)
{
    WCHAR Buffer[MAX_PATH];
    UNICODE_STRING UpdateString;

    /* Setup the string and call the extended API */
    RtlInitEmptyUnicodeString(&UpdateString, Buffer, sizeof(Buffer));
    LdrpUpdateLoadCount3(LdrEntry, Flags, &UpdateString);
}

VOID
NTAPI
LdrpCallTlsInitializers(IN PVOID BaseAddress,
                        IN ULONG Reason)
{
    PIMAGE_TLS_DIRECTORY TlsDirectory;
    PIMAGE_TLS_CALLBACK *Array, Callback;
    ULONG Size;

    /* Get the TLS Directory */
    TlsDirectory = RtlImageDirectoryEntryToData(BaseAddress,
                                                TRUE,
                                                IMAGE_DIRECTORY_ENTRY_TLS,
                                                &Size);

    /* Protect against invalid pointers */
    _SEH2_TRY
    {
        /* Make sure it's valid */
        if (TlsDirectory)
        {
            /* Get the array */
            Array = (PIMAGE_TLS_CALLBACK *)TlsDirectory->AddressOfCallBacks;
            if (Array)
            {
                /* Display debug */
                if (ShowSnaps)
                {
                    DPRINT1("LDR: Tls Callbacks Found. Imagebase %p Tls %p CallBacks %p\n",
                            BaseAddress, TlsDirectory, Array);
                }

                /* Loop the array */
                while (*Array)
                {
                    /* Get the TLS Entrypoint */
                    Callback = *Array++;

                    /* Display debug */
                    if (ShowSnaps)
                    {
                        DPRINT1("LDR: Calling Tls Callback Imagebase %p Function %p\n",
                                BaseAddress, Callback);
                    }

                    /* Call it */
                    LdrpCallInitRoutine((PDLL_INIT_ROUTINE)Callback,
                                        BaseAddress,
                                        Reason,
                                        NULL);
                }
            }
        }
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Do nothing */
    }
    _SEH2_END;
}

NTSTATUS
NTAPI
LdrpCodeAuthzCheckDllAllowed(IN PUNICODE_STRING FullName,
                             IN HANDLE DllHandle)
{
    /* Not implemented */
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
LdrpCreateDllSection(IN PUNICODE_STRING FullName,
                     IN HANDLE DllHandle,
                     IN PULONG DllCharacteristics OPTIONAL,
                     OUT PHANDLE SectionHandle)
{
    HANDLE FileHandle;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG_PTR HardErrorParameters[1];
    ULONG Response;
    SECTION_IMAGE_INFORMATION SectionImageInfo;

    /* Check if we don't already have a handle */
    if (!DllHandle)
    {
        /* Create the object attributes */
        InitializeObjectAttributes(&ObjectAttributes,
                                   FullName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        /* Open the DLL */
        Status = NtOpenFile(&FileHandle,
                            SYNCHRONIZE | FILE_EXECUTE | FILE_READ_DATA,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            FILE_SHARE_READ | FILE_SHARE_DELETE,
                            FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

        /* Check if we failed */
        if (!NT_SUCCESS(Status))
        {
            /* Attempt to open for execute only */
            Status = NtOpenFile(&FileHandle,
                                SYNCHRONIZE | FILE_EXECUTE,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_DELETE,
                                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

            /* Check if this failed too */
            if (!NT_SUCCESS(Status))
            {
                /* Show debug message */
                if (ShowSnaps)
                {
                    DPRINT1("LDR: LdrpCreateDllSection - NtOpenFile failed; status = %x\n",
                            Status);
                }

                /* Make sure to return an expected status code */
                if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                {
                    /* Callers expect this instead */
                    Status = STATUS_DLL_NOT_FOUND;
                }

                /* Return an empty section handle */
                *SectionHandle = NULL;
                return Status;
            }
        }
    }
    else
    {
        /* Use the handle we already have */
        FileHandle = DllHandle;
    }

    /* Create a section for the DLL */
    Status = NtCreateSection(SectionHandle,
                             SECTION_MAP_READ | SECTION_MAP_EXECUTE |
                             SECTION_MAP_WRITE | SECTION_QUERY,
                             NULL,
                             NULL,
                             PAGE_EXECUTE,
                             SEC_IMAGE,
                             FileHandle);

    /* If mapping failed, raise a hard error */
    if (!NT_SUCCESS(Status))
    {
        /* Forget the handle */
        *SectionHandle = NULL;

        /* Give the DLL name */
        HardErrorParameters[0] = (ULONG_PTR)FullName;

        /* Raise the error */
        ZwRaiseHardError(STATUS_INVALID_IMAGE_FORMAT,
                         1,
                         1,
                         HardErrorParameters,
                         OptionOk,
                         &Response);

        /* Increment the error count */
        if (LdrpInLdrInit) LdrpFatalHardErrorCount++;
    }

    /* Check for Safer restrictions */
    if (DllCharacteristics &&
        !(*DllCharacteristics & IMAGE_FILE_SYSTEM))
    {
        /* Make sure it's executable */
        Status = ZwQuerySection(*SectionHandle,
                                SectionImageInformation,
                                &SectionImageInfo,
                                sizeof(SECTION_IMAGE_INFORMATION),
                                NULL);
        if (NT_SUCCESS(Status))
        {
            /* Bypass the check for .NET images */
            if (!(SectionImageInfo.LoaderFlags & IMAGE_LOADER_FLAGS_COMPLUS))
            {
                /* Check with Safer */
                Status = LdrpCodeAuthzCheckDllAllowed(FullName, DllHandle);
                if (!NT_SUCCESS(Status) && (Status != STATUS_NOT_FOUND))
                {
                    /* Show debug message */
                    if (ShowSnaps)
                    {
                        DPRINT1("LDR: Loading of (%wZ) blocked by Winsafer\n",
                                &FullName);
                    }

                    /* Failure case, close section handle */
                    NtClose(*SectionHandle);
                    *SectionHandle = NULL;
                }
            }
        }
        else
        {
            /* Failure case, close section handle */
            NtClose(*SectionHandle);
            *SectionHandle = NULL;
        }
    }

    /* Close the file handle, we don't need it */
    NtClose(FileHandle);

    /* Return status */
    return Status;
}

/* NOTE: This function is totally b0rked and doesn't handle the parameters/functionality it should */
BOOLEAN
NTAPI
LdrpResolveDllName(PWSTR DllPath,
                   PWSTR DllName,
                   PUNICODE_STRING FullDllName,
                   PUNICODE_STRING BaseDllName)
{
    PWCHAR NameBuffer, p1, p2 = 0;
    ULONG Length;
    ULONG BufSize = 500;

    /* Allocate space for full DLL name */
    FullDllName->Buffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, BufSize + sizeof(UNICODE_NULL));
    if (!FullDllName->Buffer) return FALSE;

    Length = RtlDosSearchPath_U(DllPath ? DllPath : LdrpDefaultPath.Buffer,
                                DllName,
                                NULL,
                                BufSize,
                                FullDllName->Buffer,
                                &BaseDllName->Buffer);

    if (!Length || Length > BufSize)
    {
        if (ShowSnaps)
        {
            DPRINT1("LDR: LdrResolveDllName - Unable to find ");
            DPRINT1("%ws from %ws\n", DllName, DllPath ? DllPath : LdrpDefaultPath.Buffer);
        }

        RtlFreeUnicodeString(FullDllName);
        return FALSE;
    }

    /* Construct full DLL name */
    FullDllName->Length = Length;
    FullDllName->MaximumLength = FullDllName->Length + sizeof(UNICODE_NULL);

    /* Allocate a new buffer */
    NameBuffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, FullDllName->MaximumLength);
    if (!NameBuffer)
    {
        RtlFreeHeap(RtlGetProcessHeap(), 0, FullDllName->Buffer);
        return FALSE;
    }

    /* Copy over the contents from the previous one and free it */
    RtlCopyMemory(NameBuffer, FullDllName->Buffer, FullDllName->MaximumLength);
    RtlFreeHeap(RtlGetProcessHeap(), 0, FullDllName->Buffer);
    FullDllName->Buffer = NameBuffer;

    /* Find last backslash */
    p1 = FullDllName->Buffer;
    while (*p1)
    {
        if (*p1++ == L'\\')
        {
            p2 = p1;
        }
    }

    /* If found, set p1 to it, otherwise p1 points to the beginning of DllName */
    if (p2)
        p1 = p2;
    else
        p1 = DllName;

    p2 = p1;

    /* Calculate remaining length */
    while (*p1) ++p1;

    /* Construct base DLL name */
    BaseDllName->Length = (ULONG_PTR)p1 - (ULONG_PTR)p2;
    BaseDllName->MaximumLength = BaseDllName->Length + sizeof(UNICODE_NULL);
    BaseDllName->Buffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, BaseDllName->MaximumLength);

    if (!BaseDllName->Buffer)
    {
        RtlFreeHeap(RtlGetProcessHeap(), 0, NameBuffer);
        return FALSE;
    }

    /* Copy base dll name to the new buffer */
    RtlMoveMemory(BaseDllName->Buffer,
                  p2,
                  BaseDllName->Length);

    /* Null-terminate the string */
    BaseDllName->Buffer[BaseDllName->Length / sizeof(WCHAR)] = 0;

    return TRUE;
}

PVOID
NTAPI
LdrpFetchAddressOfEntryPoint(IN PVOID ImageBase)
{
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG_PTR EntryPoint = 0;

    /* Get entry point offset from NT headers */
    NtHeaders = RtlImageNtHeader(ImageBase);
    if (NtHeaders)
    {
        /* Add image base */
        EntryPoint = NtHeaders->OptionalHeader.AddressOfEntryPoint;
        if (EntryPoint) EntryPoint += (ULONG_PTR)ImageBase;
    }

    /* Return calculated pointer (or zero in case of failure) */
    return (PVOID)EntryPoint;
}

/* NOTE: This function is broken, wrong number of parameters, no SxS, etc */
HANDLE
NTAPI
LdrpCheckForKnownDll(PWSTR DllName,
                     PUNICODE_STRING FullDllName,
                     PUNICODE_STRING BaseDllName)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Section = NULL;
    UNICODE_STRING DllNameUnic;
    NTSTATUS Status;
    PCHAR p1;
    PWCHAR p2;

    /* Upgrade DllName to a unicode string */
    RtlInitUnicodeString(&DllNameUnic, DllName);

    /* Get the activation context */
    Status = RtlFindActivationContextSectionString(0,
                                                   NULL,
                                                   ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION,
                                                   &DllNameUnic,
                                                   NULL);

    /* Check if it's a SxS or not */
    if (Status == STATUS_SXS_SECTION_NOT_FOUND ||
        Status == STATUS_SXS_KEY_NOT_FOUND)
    {
        /* Set up BaseDllName */
        BaseDllName->Length = DllNameUnic.Length;
        BaseDllName->MaximumLength = DllNameUnic.MaximumLength;
        BaseDllName->Buffer = RtlAllocateHeap(RtlGetProcessHeap(),
                                              0,
                                              DllNameUnic.MaximumLength);
        if (!BaseDllName->Buffer) return NULL;

        /* Copy the contents there */
        RtlMoveMemory(BaseDllName->Buffer, DllNameUnic.Buffer, DllNameUnic.MaximumLength);

        /* Set up FullDllName */
        FullDllName->Length = LdrpKnownDllPath.Length + BaseDllName->Length + sizeof(WCHAR);
        FullDllName->MaximumLength = FullDllName->Length + sizeof(UNICODE_NULL);
        FullDllName->Buffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, FullDllName->MaximumLength);
        if (!FullDllName->Buffer)
        {
            /* Free base name and fail */
            RtlFreeHeap(RtlGetProcessHeap(), 0, BaseDllName->Buffer);
            return NULL;
        }

        RtlMoveMemory(FullDllName->Buffer, LdrpKnownDllPath.Buffer, LdrpKnownDllPath.Length);

        /* Put a slash there */
        p1 = (PCHAR)FullDllName->Buffer + LdrpKnownDllPath.Length;
        p2 = (PWCHAR)p1;
        *p2++ = (WCHAR)'\\';
        p1 = (PCHAR)p2;

        /* Set up DllNameUnic for a relative path */
        DllNameUnic.Buffer = (PWSTR)p1;
        DllNameUnic.Length = BaseDllName->Length;
        DllNameUnic.MaximumLength = DllNameUnic.Length + sizeof(UNICODE_NULL);

        /* Copy the contents */
        RtlMoveMemory(p1, BaseDllName->Buffer, BaseDllName->MaximumLength);

        /* There are all names, init attributes and open the section */
        InitializeObjectAttributes(&ObjectAttributes,
                                   &DllNameUnic,
                                   OBJ_CASE_INSENSITIVE,
                                   LdrpKnownDllObjectDirectory,
                                   NULL);

        Status = NtOpenSection(&Section,
                               SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE,
                               &ObjectAttributes);
        if (!NT_SUCCESS(Status))
        {
            /* Opening failed, free resources */
            Section = NULL;
            RtlFreeHeap(RtlGetProcessHeap(), 0, BaseDllName->Buffer);
            RtlFreeHeap(RtlGetProcessHeap(), 0, FullDllName->Buffer);
        }
    }
    else
    {
        if (!NT_SUCCESS(Status)) Section = NULL;
    }

    /* Return section handle */
    return Section;
}

NTSTATUS
NTAPI
LdrpSetProtection(PVOID ViewBase,
                  BOOLEAN Restore)
{
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER Section;
    NTSTATUS Status;
    PVOID SectionBase;
    SIZE_T SectionSize;
    ULONG NewProtection, OldProtection, i;

    /* Get the NT headers */
    NtHeaders = RtlImageNtHeader(ViewBase);
    if (!NtHeaders) return STATUS_INVALID_IMAGE_FORMAT;

    /* Compute address of the first section header */
    Section = IMAGE_FIRST_SECTION(NtHeaders);

    /* Go through all sections */
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i++)
    {
        /* Check for read-only non-zero section */
        if ((Section->SizeOfRawData) &&
            !(Section->Characteristics & IMAGE_SCN_MEM_WRITE))
        {
            /* Check if we are setting or restoring protection */
            if (Restore)
            {
                /* Set it to either EXECUTE or READONLY */
                if (Section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
                {
                    NewProtection = PAGE_EXECUTE;
                }
                else
                {
                    NewProtection = PAGE_READONLY;
                }

                /* Add PAGE_NOCACHE if needed */
                if (Section->Characteristics & IMAGE_SCN_MEM_NOT_CACHED)
                {
                    NewProtection |= PAGE_NOCACHE;
                }
            }
            else
            {
                /* Enable write access */
                NewProtection = PAGE_READWRITE;
            }

            /* Get the section VA */
            SectionBase = (PVOID)((ULONG_PTR)ViewBase + Section->VirtualAddress);
            SectionSize = Section->SizeOfRawData;
            if (SectionSize)
            {
                /* Set protection */
                Status = ZwProtectVirtualMemory(NtCurrentProcess(),
                                                &SectionBase,
                                                &SectionSize,
                                                NewProtection,
                                                &OldProtection);
                if (!NT_SUCCESS(Status)) return Status;
            }
        }

        /* Move to the next section */
        Section++;
    }

    /* Flush instruction cache if necessary */
    if (Restore) ZwFlushInstructionCache(NtCurrentProcess(), NULL, 0);
    return STATUS_SUCCESS;
}

/* NOTE: Not yet reviewed */
NTSTATUS
NTAPI
LdrpMapDll(IN PWSTR SearchPath OPTIONAL,
           IN PWSTR DllPath2,
           IN PWSTR DllName OPTIONAL,
           IN PULONG DllCharacteristics,
           IN BOOLEAN Static,
           IN BOOLEAN Redirect,
           OUT PLDR_DATA_TABLE_ENTRY *DataTableEntry)
{
    PTEB Teb = NtCurrentTeb();
    PPEB Peb = NtCurrentPeb();
    PWCHAR p1 = DllName;
    WCHAR TempChar;
    BOOLEAN KnownDll = FALSE;
    UNICODE_STRING FullDllName, BaseDllName;
    HANDLE SectionHandle = NULL, DllHandle = 0;
    UNICODE_STRING NtPathDllName;
    ULONG_PTR HardErrorParameters[2];
    UNICODE_STRING HardErrorDllName, HardErrorDllPath;
    ULONG Response;
    SIZE_T ViewSize = 0;
    PVOID ViewBase = NULL;
    PVOID ArbitraryUserPointer;
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS HardErrorStatus, Status;
    BOOLEAN OverlapDllFound = FALSE;
    ULONG_PTR ImageBase, ImageEnd;
    PLIST_ENTRY ListHead, NextEntry;
    PLDR_DATA_TABLE_ENTRY CandidateEntry, LdrEntry;
    ULONG_PTR CandidateBase, CandidateEnd;
    UNICODE_STRING OverlapDll;
    BOOLEAN RelocatableDll = TRUE;
    UNICODE_STRING IllegalDll;
    PVOID RelocData;
    ULONG RelocDataSize = 0;

    // FIXME: AppCompat stuff is missing

    if (ShowSnaps)
    {
        DPRINT1("LDR: LdrpMapDll: Image Name %ws, Search Path %ws\n",
                DllName,
                SearchPath ? SearchPath : L"");
    }

    /* Check if we have a known dll directory */
    if (LdrpKnownDllObjectDirectory)
    {
        /* Check if the path is full */
        while (*p1)
        {
            TempChar = *p1++;
            if (TempChar == '\\' || TempChar == '/' )
            {
                /* Complete path, don't do Known Dll lookup */
                goto SkipCheck;
            }
        }

        /* Try to find a Known DLL */
        SectionHandle = LdrpCheckForKnownDll(DllName,
                                             &FullDllName,
                                             &BaseDllName);
    }

SkipCheck:

    /* Check if the Known DLL Check returned something */
    if (!SectionHandle)
    {
        /* It didn't, so try to resolve the name now */
        if (LdrpResolveDllName(SearchPath,
                               DllName,
                               &FullDllName,
                               &BaseDllName))
        {
            /* Got a name, display a message */
            if (ShowSnaps)
            {
                DPRINT1("LDR: Loading (%s) %wZ\n",
                        Static ? "STATIC" : "DYNAMIC",
                        &FullDllName);
            }

            /* Convert to NT Name */
            if (!RtlDosPathNameToNtPathName_U(FullDllName.Buffer,
                                              &NtPathDllName,
                                              NULL,
                                              NULL))
            {
                /* Path was invalid */
                return STATUS_OBJECT_PATH_SYNTAX_BAD;
            }

            /* Create a section for this dLL */
            Status = LdrpCreateDllSection(&NtPathDllName,
                                          DllHandle,
                                          DllCharacteristics,
                                          &SectionHandle);

            /* Free the NT Name */
            RtlFreeHeap(RtlGetProcessHeap(), 0, NtPathDllName.Buffer);

            /* If we failed */
            if (!NT_SUCCESS(Status))
            {
                /* Free the name strings and return */
                RtlFreeUnicodeString(&FullDllName);
                RtlFreeUnicodeString(&BaseDllName);
                return Status;
            }
        }
        else
        {
            /* We couldn't resolve the name, is this a static load? */
            if (Static)
            {
                /*
                 * This is BAD! Static loads are CRITICAL. Bugcheck!
                 * Initialize the strings for the error
                 */
                RtlInitUnicodeString(&HardErrorDllName, DllName);
                RtlInitUnicodeString(&HardErrorDllPath,
                                     DllPath2 ? DllPath2 : LdrpDefaultPath.Buffer);

                /* Set them as error parameters */
                HardErrorParameters[0] = (ULONG_PTR)&HardErrorDllName;
                HardErrorParameters[1] = (ULONG_PTR)&HardErrorDllPath;

                /* Raise the hard error */
                NtRaiseHardError(STATUS_DLL_NOT_FOUND,
                                 2,
                                 0x00000003,
                                 HardErrorParameters,
                                 OptionOk,
                                 &Response);

                /* We're back, where we initializing? */
                if (LdrpInLdrInit) LdrpFatalHardErrorCount++;
            }

            /* Return failure */
            return STATUS_DLL_NOT_FOUND;
        }
    }
    else
    {
        /* We have a section handle, so this is a known dll */
        KnownDll = TRUE;
    }

    /* Stuff the image name in the TIB, for the debugger */
    ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
    Teb->NtTib.ArbitraryUserPointer = FullDllName.Buffer;

    /* Map the DLL */
    ViewBase = NULL;
    ViewSize = 0;
    Status = NtMapViewOfSection(SectionHandle,
                                NtCurrentProcess(),
                                &ViewBase,
                                0,
                                0,
                                NULL,
                                &ViewSize,
                                ViewShare,
                                0,
                                PAGE_READWRITE);

    /* Restore */
    Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

    /* Fail if we couldn't map it */
    if (!NT_SUCCESS(Status))
    {
        /* Close and return */
        NtClose(SectionHandle);
        return Status;
    }

    /* Get the NT Header */
    if (!(NtHeaders = RtlImageNtHeader(ViewBase)))
    {
        /* Invalid image, unmap, close handle and fail */
        NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
        NtClose(SectionHandle);
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    // FIXME: .NET support is missing

    /* Allocate an entry */
    if (!(LdrEntry = LdrpAllocateDataTableEntry(ViewBase)))
    {
        /* Invalid image, unmap, close handle and fail */
        NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
        NtClose(SectionHandle);
        return STATUS_NO_MEMORY;
    }

    /* Setup the entry */
    LdrEntry->Flags = Static ? LDRP_STATIC_LINK : 0;
    if (Redirect) LdrEntry->Flags |= LDRP_REDIRECTED;
    LdrEntry->LoadCount = 0;
    LdrEntry->FullDllName = FullDllName;
    LdrEntry->BaseDllName = BaseDllName;
    LdrEntry->EntryPoint = LdrpFetchAddressOfEntryPoint(LdrEntry->DllBase);

    /* Show debug message */
    if (ShowSnaps)
    {
        DPRINT1("LDR: LdrpMapDll: Full Name %wZ, Base Name %wZ\n",
                &FullDllName,
                &BaseDllName);
    }

    /* Insert this entry */
    LdrpInsertMemoryTableEntry(LdrEntry);

    // LdrpSendDllNotifications(LdrEntry, TRUE, Status == STATUS_IMAGE_NOT_AT_BASE)

    /* Check for invalid CPU Image */
    if (Status == STATUS_IMAGE_MACHINE_TYPE_MISMATCH)
    {
        /* Load our header */
        PIMAGE_NT_HEADERS ImageNtHeader = RtlImageNtHeader(Peb->ImageBaseAddress);

        /* Assume defaults if we don't have to run the Hard Error path */
        HardErrorStatus = STATUS_SUCCESS;
        Response = ResponseCancel;

        /* Are we an NT 3.0 image? [Do these still exist? LOL -- IAI] */
        if (ImageNtHeader->OptionalHeader.MajorSubsystemVersion <= 3)
        {
            /* Reset the entrypoint, save our Dll Name */
            LdrEntry->EntryPoint = 0;
            HardErrorParameters[0] = (ULONG_PTR)&FullDllName;

            /* Raise the error */
            HardErrorStatus = ZwRaiseHardError(STATUS_IMAGE_MACHINE_TYPE_MISMATCH,
                                               1,
                                               1,
                                               HardErrorParameters,
                                               OptionOkCancel,
                                               &Response);
        }

        /* Check if the user pressed cancel */
        if (NT_SUCCESS(HardErrorStatus) && Response == ResponseCancel)
        {
            /* Remove the DLL from the lists */
            RemoveEntryList(&LdrEntry->InLoadOrderLinks);
            RemoveEntryList(&LdrEntry->InMemoryOrderModuleList);
            RemoveEntryList(&LdrEntry->HashLinks);

            /* Remove the LDR Entry */
            RtlFreeHeap(RtlGetProcessHeap(), 0, LdrEntry );

            /* Unmap and close section */
            NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
            NtClose(SectionHandle);

            /* Did we do a hard error? */
            if (ImageNtHeader->OptionalHeader.MajorSubsystemVersion <= 3)
            {
                /* Yup, so increase fatal error count if we are initializing */
                if (LdrpInLdrInit) LdrpFatalHardErrorCount++;
            }

            /* Return failure */
            return STATUS_INVALID_IMAGE_FORMAT;
        }
    }
    else
    {
        /* The image was valid. Is it a DLL? */
        if (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DLL)
        {
            /* Set the DLL Flag */
            LdrEntry->Flags |= LDRP_IMAGE_DLL;
        }

        /* If we're not a DLL, clear the entrypoint */
        if (!(LdrEntry->Flags & LDRP_IMAGE_DLL))
        {
            LdrEntry->EntryPoint = 0;
        }
    }

    /* Return it for the caller */
    *DataTableEntry = LdrEntry;

    /* Check if we loaded somewhere else */
    if (Status == STATUS_IMAGE_NOT_AT_BASE)
    {
        /* Write the flag */
        LdrEntry->Flags |= LDRP_IMAGE_NOT_AT_BASE;

        /* Find our region */
        ImageBase = (ULONG_PTR)NtHeaders->OptionalHeader.ImageBase;
        ImageEnd = ImageBase + ViewSize;

        DPRINT1("LDR: LdrpMapDll Relocating Image Name %ws (%p -> %p)\n", DllName, ImageBase, ViewBase);

        /* Scan all the modules */
        ListHead = &Peb->Ldr->InLoadOrderModuleList;
        NextEntry = ListHead->Flink;
        while (NextEntry != ListHead)
        {
            /* Get the entry */
            CandidateEntry = CONTAINING_RECORD(NextEntry,
                                               LDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);
            NextEntry = NextEntry->Flink;

            /* Get the entry's bounds */
            CandidateBase = (ULONG_PTR)CandidateEntry->DllBase;
            CandidateEnd = CandidateBase + CandidateEntry->SizeOfImage;

            /* Make sure this entry isn't unloading */
            if (!CandidateEntry->InMemoryOrderModuleList.Flink) continue;

            /* Check if our regions are colliding */
            if ((ImageBase >= CandidateBase && ImageBase <= CandidateEnd) ||
                (ImageEnd >= CandidateBase && ImageEnd <= CandidateEnd) ||
                (CandidateBase >= ImageBase && CandidateBase <= ImageEnd))
            {
                /* Found who is overlapping */
                OverlapDllFound = TRUE;
                OverlapDll = CandidateEntry->FullDllName;
                break;
            }
        }

        /* Check if we found the DLL overlapping with us */
        if (!OverlapDllFound)
        {
            /* It's not another DLL, it's memory already here */
            RtlInitUnicodeString(&OverlapDll, L"Dynamically Allocated Memory");
        }

        DPRINT1("Overlapping DLL: %wZ\n", &OverlapDll);

        /* Are we dealing with a DLL? */
        if (LdrEntry->Flags & LDRP_IMAGE_DLL)
        {
            /* Check if relocs were stripped */
            if (!(NtHeaders->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED))
            {
                /* Get the relocation data */
                RelocData = RtlImageDirectoryEntryToData(ViewBase,
                                                         TRUE,
                                                         IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                                         &RelocDataSize);

                /* Does the DLL not have any? */
                if (!RelocData && !RelocDataSize)
                {
                    /* We'll allow this and simply continue */
                    goto NoRelocNeeded;
                }
            }

            /* See if this is an Illegal DLL - IE: user32 and kernel32 */
            RtlInitUnicodeString(&IllegalDll,L"user32.dll");
            if (RtlEqualUnicodeString(&BaseDllName, &IllegalDll, TRUE))
            {
                /* Can't relocate user32 */
                RelocatableDll = FALSE;
            }
            else
            {
                RtlInitUnicodeString(&IllegalDll, L"kernel32.dll");
                if (RtlEqualUnicodeString(&BaseDllName, &IllegalDll, TRUE))
                {
                    /* Can't relocate kernel32 */
                    RelocatableDll = FALSE;
                }
            }

            /* Check if this was a non-relocatable DLL or a known dll */
            if (!RelocatableDll && KnownDll)
            {
                /* Setup for hard error */
                HardErrorParameters[0] = (ULONG_PTR)&IllegalDll;
                HardErrorParameters[1] = (ULONG_PTR)&OverlapDll;

                /* Raise the error */
                ZwRaiseHardError(STATUS_ILLEGAL_DLL_RELOCATION,
                                 2,
                                 3,
                                 HardErrorParameters,
                                 OptionOk,
                                 &Response);

                /* If initializing, increase the error count */
                if (LdrpInLdrInit) LdrpFatalHardErrorCount++;

                /* Don't do relocation */
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto NoRelocNeeded;
            }

            /* Change the protection to prepare for relocation */
            Status = LdrpSetProtection(ViewBase, FALSE);

            /* Make sure we changed the protection */
            if (NT_SUCCESS(Status))
            {
                /* Do the relocation */
                Status = LdrRelocateImageWithBias(ViewBase, 0LL, NULL, STATUS_SUCCESS,
                    STATUS_CONFLICTING_ADDRESSES, STATUS_INVALID_IMAGE_FORMAT);

                if (NT_SUCCESS(Status))
                {
                    /* Stuff the image name in the TIB, for the debugger */
                    ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
                    Teb->NtTib.ArbitraryUserPointer = FullDllName.Buffer;

                    /* Map the DLL */
                    Status = NtMapViewOfSection(SectionHandle,
                                                NtCurrentProcess(),
                                                &ViewBase,
                                                0,
                                                0,
                                                NULL,
                                                &ViewSize,
                                                ViewShare,
                                                0,
                                                PAGE_READWRITE);

                    /* Restore */
                    Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

                    /* Return the protection */
                    Status = LdrpSetProtection(ViewBase, TRUE);
                }
            }
//FailRelocate:
            /* Handle any kind of failure */
            if (!NT_SUCCESS(Status))
            {
                /* Remove it from the lists */
                RemoveEntryList(&LdrEntry->InLoadOrderLinks);
                RemoveEntryList(&LdrEntry->InMemoryOrderModuleList);
                RemoveEntryList(&LdrEntry->HashLinks);

                /* Unmap it, clear the entry */
                NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
                LdrEntry = NULL;
            }

            /* Show debug message */
            if (ShowSnaps)
            {
                DPRINT1("LDR: Fixups %successfully re-applied @ %p\n",
                        NT_SUCCESS(Status) ? "s" : "uns", ViewBase);
            }
        }
        else
        {
NoRelocNeeded:
            /* Not a DLL, or no relocation needed */
            Status = STATUS_SUCCESS;

            /* Stuff the image name in the TIB, for the debugger */
            ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
            Teb->NtTib.ArbitraryUserPointer = FullDllName.Buffer;

            /* Map the DLL */
            Status = NtMapViewOfSection(SectionHandle,
                                        NtCurrentProcess(),
                                        &ViewBase,
                                        0,
                                        0,
                                        NULL,
                                        &ViewSize,
                                        ViewShare,
                                        0,
                                        PAGE_READWRITE);

            /* Restore */
            Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;

            /* Show debug message */
            if (ShowSnaps)
            {
                DPRINT1("LDR: Fixups won't be re-applied to non-Dll @ %p\n", ViewBase);
            }
        }
    }

    // FIXME: LdrpCheckCorImage() is missing

    /* Check if this is an SMP Machine and a DLL */
    if ((LdrpNumberOfProcessors > 1) &&
        (LdrEntry && (LdrEntry->Flags & LDRP_IMAGE_DLL)))
    {
        /* Validate the image for MP */
        LdrpValidateImageForMp(LdrEntry);
    }

    // FIXME: LdrpCorUnloadImage() is missing

    /* Close section and return status */
    NtClose(SectionHandle);
    return Status;
}

PLDR_DATA_TABLE_ENTRY
NTAPI
LdrpAllocateDataTableEntry(IN PVOID BaseAddress)
{
    PLDR_DATA_TABLE_ENTRY LdrEntry = NULL;
    PIMAGE_NT_HEADERS NtHeader;

    /* Make sure the header is valid */
    NtHeader = RtlImageNtHeader(BaseAddress);
    DPRINT("LdrpAllocateDataTableEntry(%p), NtHeader %p\n", BaseAddress, NtHeader);

    if (NtHeader)
    {
        /* Allocate an entry */
        LdrEntry = RtlAllocateHeap(RtlGetProcessHeap(),
                                   HEAP_ZERO_MEMORY,
                                   sizeof(LDR_DATA_TABLE_ENTRY));

        /* Make sure we got one */
        if (LdrEntry)
        {
            /* Set it up */
            LdrEntry->DllBase = BaseAddress;
            LdrEntry->SizeOfImage = NtHeader->OptionalHeader.SizeOfImage;
            LdrEntry->TimeDateStamp = NtHeader->FileHeader.TimeDateStamp;
            LdrEntry->PatchInformation = NULL;
        }
    }

    /* Return the entry */
    return LdrEntry;
}

VOID
NTAPI
LdrpInsertMemoryTableEntry(IN PLDR_DATA_TABLE_ENTRY LdrEntry)
{
    PPEB_LDR_DATA PebData = NtCurrentPeb()->Ldr;
    ULONG i;

    /* Insert into hash table */
    i = LDR_GET_HASH_ENTRY(LdrEntry->BaseDllName.Buffer[0]);
    InsertTailList(&LdrpHashTable[i], &LdrEntry->HashLinks);

    /* Insert into other lists */
    InsertTailList(&PebData->InLoadOrderModuleList, &LdrEntry->InLoadOrderLinks);
    InsertTailList(&PebData->InMemoryOrderModuleList, &LdrEntry->InMemoryOrderModuleList);
}

VOID
NTAPI
LdrpFinalizeAndDeallocateDataTableEntry(IN PLDR_DATA_TABLE_ENTRY Entry)
{
    /* Sanity check */
    ASSERT(Entry != NULL);

    /* Release the activation context if it exists and wasn't already released */
    if ((Entry->EntryPointActivationContext) &&
        (Entry->EntryPointActivationContext != INVALID_HANDLE_VALUE))
    {
        /* Mark it as invalid */
        RtlReleaseActivationContext(Entry->EntryPointActivationContext);
        Entry->EntryPointActivationContext = INVALID_HANDLE_VALUE;
    }

    /* Release the full dll name string */
    if (Entry->FullDllName.Buffer) LdrpFreeUnicodeString(&Entry->FullDllName);

    /* Finally free the entry's memory */
    RtlFreeHeap(RtlGetProcessHeap(), 0, Entry);
}

BOOLEAN
NTAPI
LdrpCheckForLoadedDllHandle(IN PVOID Base,
                            OUT PLDR_DATA_TABLE_ENTRY *LdrEntry)
{
    PLDR_DATA_TABLE_ENTRY Current;
    PLIST_ENTRY ListHead, Next;

    /* Check the cache first */
    if ((LdrpLoadedDllHandleCache) &&
        (LdrpLoadedDllHandleCache->DllBase == Base))
    {
        /* We got lucky, return the cached entry */
        *LdrEntry = LdrpLoadedDllHandleCache;
        return TRUE;
    }

    /* Time for a lookup */
    ListHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    Next = ListHead->Flink;
    while (Next != ListHead)
    {
        /* Get the current entry */
        Current = CONTAINING_RECORD(Next,
                                    LDR_DATA_TABLE_ENTRY,
                                    InLoadOrderLinks);

        /* Make sure it's not unloading and check for a match */
        if ((Current->InMemoryOrderModuleList.Flink) && (Base == Current->DllBase))
        {
            /* Save in cache */
            LdrpLoadedDllHandleCache = Current;

            /* Return it */
            *LdrEntry = Current;
            return TRUE;
        }

        /* Move to the next one */
        Next = Next->Flink;
    }

    /* Nothing found */
    return FALSE;
}

NTSTATUS
NTAPI
LdrpResolveFullName(IN PUNICODE_STRING OriginalName,
                    IN PUNICODE_STRING PathName,
                    IN PUNICODE_STRING FullPathName,
                    IN PUNICODE_STRING *ExpandedName)
{
    NTSTATUS Status = STATUS_SUCCESS;
//    RTL_PATH_TYPE PathType;
//    BOOLEAN InvalidName;
    ULONG Length;

    /* Display debug output if snaps are on */
    if (ShowSnaps)
    {
        DbgPrintEx(81, //DPFLTR_LDR_ID,
                   0,
                   "LDR: %s - Expanding full name of %wZ\n",
                   __FUNCTION__,
                   OriginalName);
    }

    /* FIXME: Lock the PEB */
    //RtlEnterCriticalSection(&FastPebLock);
#if 0
    /* Get the path name */
    Length = RtlGetFullPathName_Ustr(OriginalName,
                                     PathName->Length,
                                     PathName->Buffer,
                                     NULL,
                                     &InvalidName,
                                     &PathType);
#else
    Length = 0;
#endif
    if (!(Length) || (Length > UNICODE_STRING_MAX_BYTES))
    {
        /* Fail */
        Status = STATUS_NAME_TOO_LONG;
        goto Quickie;
    }

    /* Check if the length hasn't changed */
    if (Length <= PathName->Length)
    {
        /* Return the same thing */
        *ExpandedName = PathName;
        PathName->Length = (USHORT)Length;
        goto Quickie;
    }

    /* Sanity check */
    ASSERT(Length >= sizeof(WCHAR));

    /* Allocate a string */
    Status = LdrpAllocateUnicodeString(FullPathName, Length - sizeof(WCHAR));
    if (!NT_SUCCESS(Status)) goto Quickie;

    /* Now get the full path again */
#if 0
    Length = RtlGetFullPathName_Ustr(OriginalName,
                                     FullPathName->Length,
                                     FullPathName->Buffer,
                                     NULL,
                                     &InvalidName,
                                     &PathType);
#else
    Length = 0;
#endif
    if (!(Length) || (Length > FullPathName->Length))
    {
        /* Fail */
        LdrpFreeUnicodeString(FullPathName);
        Status = STATUS_NAME_TOO_LONG;
    }
    else
    {
        /* Return the expanded name */
        *ExpandedName = FullPathName;
        FullPathName->Length = (USHORT)Length;
    }

Quickie:
    /* FIXME: Unlock the PEB */
    //RtlLeaveCriticalSection(&FastPebLock);

    /* Display debug output if snaps are on */
    if (ShowSnaps)
    {
        /* Check which output to use -- failure or success */
        if (NT_SUCCESS(Status))
        {
            DbgPrintEx(81, //DPFLTR_LDR_ID,
                       0,
                       "LDR: %s - Expanded to %wZ\n",
                       __FUNCTION__,
                       *ExpandedName);
        }
        else
        {
            DbgPrintEx(81, //DPFLTR_LDR_ID,
                       0,
                       "LDR: %s - Failed to expand %wZ; 0x%08x\n",
                       __FUNCTION__,
                       OriginalName,
                       Status);
        }
    }

    /* If we failed, return NULL */
    if (!NT_SUCCESS(Status)) *ExpandedName = NULL;

    /* Return status */
    return Status;
}

NTSTATUS
NTAPI
LdrpSearchPath(IN PWCHAR *SearchPath,
               IN PWCHAR DllName,
               IN PUNICODE_STRING PathName,
               IN PUNICODE_STRING FullPathName,
               IN PUNICODE_STRING *ExpandedName)
{
    BOOLEAN TryAgain = FALSE;
    PWCHAR ActualSearchPath = *SearchPath;
    UNICODE_STRING TestName;
    NTSTATUS Status;
    PWCHAR Buffer, BufEnd = NULL;
    ULONG Length = 0;
    WCHAR p;
    PWCHAR pp;

    /* Check if we don't have a search path */
    if (!ActualSearchPath) *SearchPath = LdrpDefaultPath.Buffer;

    /* Display debug output if snaps are on */
    if (ShowSnaps)
    {
        DbgPrintEx(81, //DPFLTR_LDR_ID,
                   0,
                   "LDR: %s - Looking for %ws in %ws\n",
                   __FUNCTION__,
                   DllName,
                   *SearchPath);
    }

    /* Check if we're dealing with a relative path */
    if (RtlDetermineDosPathNameType_U(DllName) != RtlPathTypeRelative)
    {
        /* Good, we're not. Create the name string */
        Status = RtlInitUnicodeStringEx(&TestName, DllName);
        if (!NT_SUCCESS(Status)) goto Quickie;

        /* Make sure it exists */
        #if 0
        if (!RtlDoesFileExists_UstrEx(&TestName, TRUE))
        {
            /* It doesn't, fail */
            Status = STATUS_DLL_NOT_FOUND;
            goto Quickie;
        }
        #endif

        /* Resolve the full name */
        Status = LdrpResolveFullName(&TestName,
                                     PathName,
                                     FullPathName,
                                     ExpandedName);
        goto Quickie;
    }

    /* FIXME: Handle relative case semicolon-lookup here */

    /* Calculate length */
    Length += (ULONG)wcslen(DllName) + sizeof(UNICODE_NULL);
    if (Length > UNICODE_STRING_MAX_CHARS)
    {
        /* Too long, fail */
        Status = STATUS_NAME_TOO_LONG;
        goto Quickie;
    }

    /* Allocate buffer */
    Buffer = RtlAllocateHeap(RtlGetProcessHeap(), 0, Length * sizeof(WCHAR));
    if (!Buffer)
    {
        /* Fail */
        Status = STATUS_NO_MEMORY;
        goto Quickie;
    }

    /* FIXME: Setup TestName here */
    Status = STATUS_NOT_FOUND;

    /* Start loop */
    do
    {
        /* Get character */
        p = *ActualSearchPath;
        if (!(p) || (p == ';'))
        {
            /* FIXME: We don't have a character, or is a semicolon.*/

            /* Display debug output if snaps are on */
            if (ShowSnaps)
            {
                DbgPrintEx(81, //DPFLTR_LDR_ID,
                           0,
                           "LDR: %s - Looking for %ws\n",
                           __FUNCTION__,
                           Buffer);
            }

            /* Sanity check */
            TestName.Length = (USHORT)ALIGN_DOWN((BufEnd - Buffer), WCHAR);
            ASSERT(TestName.Length < TestName.MaximumLength);

            /* Check if the file exists */
            #if 0
            if (RtlDoesFileExists_UstrEx(&TestName, FALSE))
            #endif
            {
                /* It does. Reallocate the buffer */
                TestName.MaximumLength = (USHORT)ALIGN_DOWN((BufEnd - Buffer), WCHAR) + sizeof(WCHAR);
                TestName.Buffer = RtlReAllocateHeap(RtlGetProcessHeap(),
                                                    0,
                                                    Buffer,
                                                    TestName.MaximumLength);
                if (!TestName.Buffer)
                {
                    /* Keep the old one */
                    TestName.Buffer = Buffer;
                }
                else
                {
                    /* Update buffer */
                    Buffer = TestName.Buffer;
                }

                /* Make sure we have a buffer at least */
                ASSERT(TestName.Buffer);

                /* Resolve the name */
                *SearchPath = ActualSearchPath++;
                Status = LdrpResolveFullName(&TestName,
                                             PathName,
                                             FullPathName,
                                             ExpandedName);
                break;
            }

            /* Update buffer end */
            BufEnd = Buffer;

            /* Update string position */
            pp = ActualSearchPath++;
        }
        else
        {
            /* Otherwise, write the character */
            *BufEnd = p;
            BufEnd++;
        }

        /* Check if the string is empty, meaning we're done */
        if (!(*ActualSearchPath)) TryAgain = TRUE;

        /* Advance in the string */
        ActualSearchPath++;
    } while (!TryAgain);

    /* Check if we had a buffer and free it */
    if (Buffer) RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);

Quickie:
    /* Check if we got here through failure */
    if (!NT_SUCCESS(Status)) *ExpandedName = NULL;

    /* Display debug output if snaps are on */
    if (ShowSnaps)
    {
        /* Check which output to use -- failure or success */
        if (NT_SUCCESS(Status))
        {
            DbgPrintEx(81, //DPFLTR_LDR_ID,
                       0,
                       "LDR: %s - Returning %wZ\n",
                       __FUNCTION__,
                       *ExpandedName);
        }
        else
        {
            DbgPrintEx(81, //DPFLTR_LDR_ID,
                       0,
                       "LDR: %s -  Unable to locate %ws in %ws: 0x%08x\n",
                       __FUNCTION__,
                       DllName,
                       ActualSearchPath,
                       Status);
        }
    }

    /* Return status */
    return Status;
}


/* NOTE: This function is b0rked and in the process of being slowly unf*cked */
BOOLEAN
NTAPI
LdrpCheckForLoadedDll(IN PWSTR DllPath,
                      IN PUNICODE_STRING DllName,
                      IN BOOLEAN Flag,
                      IN BOOLEAN RedirectedDll,
                      OUT PLDR_DATA_TABLE_ENTRY *LdrEntry)
{
    ULONG HashIndex;
    PLIST_ENTRY ListHead, ListEntry;
    PLDR_DATA_TABLE_ENTRY CurEntry;
    BOOLEAN FullPath = FALSE;
    PWCHAR wc;
    WCHAR NameBuf[266];
    UNICODE_STRING FullDllName, NtPathName;
    ULONG Length;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE FileHandle, SectionHandle;
    IO_STATUS_BLOCK Iosb;
    PVOID ViewBase = NULL;
    SIZE_T ViewSize = 0;
    PIMAGE_NT_HEADERS NtHeader, NtHeader2;
    DPRINT("LdrpCheckForLoadedDll('%S' '%wZ' %d %d %p)\n", DllPath, DllName, Flag, RedirectedDll, LdrEntry);

    /* Check if a dll name was provided */
    if (!(DllName->Buffer) || !(DllName->Buffer[0])) return FALSE;

    /* FIXME: Warning, "Flag" is used as magic instead of "Static" */
    /* FIXME: Warning, code does not support redirection at all */
    
    /* Look in the hash table if flag was set */
lookinhash:
    if (Flag)
    {
        /* Get hash index */
        HashIndex = LDR_GET_HASH_ENTRY(DllName->Buffer[0]);

        /* Traverse that list */
        ListHead = &LdrpHashTable[HashIndex];
        ListEntry = ListHead->Flink;
        while (ListEntry != ListHead)
        {
            /* Get the current entry */
            CurEntry = CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, HashLinks);

            /* Check base name of that module */
            if (RtlEqualUnicodeString(DllName, &CurEntry->BaseDllName, TRUE))
            {
                /* It matches, return it */
                *LdrEntry = CurEntry;
                return TRUE;
            }

            /* Advance to the next entry */
            ListEntry = ListEntry->Flink;
        }

        /* Module was not found, return failure */
        return FALSE;
    }

    /* Check if there is a full path in this DLL */
    wc = DllName->Buffer;
    while (*wc)
    {
        /* Check for a slash in the current position*/
        if ((*wc == L'\\') || (*wc == L'/'))
        {
            /* Found the slash, so dll name contains path */
            FullPath = TRUE;

            /* Setup full dll name string */
            FullDllName.Buffer = NameBuf;

            /* FIXME: This is from the Windows 2000 loader, not XP/2003, we should call LdrpSearchPath */
            Length = RtlDosSearchPath_U(DllPath ? DllPath : LdrpDefaultPath.Buffer,
                                        DllName->Buffer,
                                        NULL,
                                        sizeof(NameBuf) - sizeof(UNICODE_NULL),
                                        FullDllName.Buffer,
                                        NULL);

            /* Check if that was successful */
            if (!(Length) || (Length > (sizeof(NameBuf) - sizeof(UNICODE_NULL))))
            {
                if (ShowSnaps)
                {
                    DPRINT1("LDR: LdrpCheckForLoadedDll - Unable To Locate %ws: 0x%08x\n",
                        DllName->Buffer, Length);
                }

                /* Return failure */
                return FALSE;
            }

            /* Full dll name is found */
            FullDllName.Length = Length;
            FullDllName.MaximumLength = FullDllName.Length + sizeof(UNICODE_NULL);
            break;
        }

        wc++;
    }

    /* Go check the hash table */
    if (!FullPath)
    {
        Flag = TRUE;
        goto lookinhash;
    }
    
    /* FIXME: Warning, activation context missing */
    /* NOTE: From here on down, everything looks good */

    /* Loop the module list */
    ListHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead)
    {
        /* Get the current entry and advance to the next one */
        CurEntry = CONTAINING_RECORD(ListEntry,
                                     LDR_DATA_TABLE_ENTRY,
                                     InLoadOrderLinks);
        ListEntry = ListEntry->Flink;

        /* Check if it's being unloaded */
        if (!CurEntry->InMemoryOrderModuleList.Flink) continue;

        /* Check if name matches */
        if (RtlEqualUnicodeString(&FullDllName,
                                  &CurEntry->FullDllName,
                                  TRUE))
        {
            /* Found it */
            *LdrEntry = CurEntry;
            return TRUE;
        }
    }

    /* Convert given path to NT path */
    if (!RtlDosPathNameToNtPathName_U(FullDllName.Buffer,
                                      &NtPathName,
                                      NULL,
                                      NULL))
    {
        /* Fail if conversion failed */
        return FALSE;
    }

    /* Initialize object attributes and open it */
    InitializeObjectAttributes(&ObjectAttributes,
                               &NtPathName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    Status = NtOpenFile(&FileHandle,
                        SYNCHRONIZE | FILE_EXECUTE,
                        &ObjectAttributes,
                        &Iosb,
                        FILE_SHARE_READ | FILE_SHARE_DELETE,
                        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    /* Free NT path name */
    RtlFreeHeap(RtlGetProcessHeap(), 0, NtPathName.Buffer);

    /* If opening the file failed - return failure */
    if (!NT_SUCCESS(Status)) return FALSE;

    /* Create a section for this file */
    Status = NtCreateSection(&SectionHandle,
                             SECTION_MAP_READ |
                             SECTION_MAP_EXECUTE |
                             SECTION_MAP_WRITE,
                             NULL,
                             NULL,
                             PAGE_EXECUTE,
                             SEC_COMMIT,
                             FileHandle);

    /* Close file handle */
    NtClose(FileHandle);

    /* If creating section failed - return failure */
    if (!NT_SUCCESS(Status)) return FALSE;

    /* Map view of this section */
    Status = ZwMapViewOfSection(SectionHandle,
                                NtCurrentProcess(),
                                &ViewBase,
                                0,
                                0,
                                NULL,
                                &ViewSize,
                                ViewShare,
                                0,
                                PAGE_EXECUTE);

    /* Close section handle */
    NtClose(SectionHandle);

    /* If section mapping failed - return failure */
    if (!NT_SUCCESS(Status)) return FALSE;

    /* Get pointer to the NT header of this section */
    Status = RtlImageNtHeaderEx(0, ViewBase, ViewSize, &NtHeader);
    if (!(NT_SUCCESS(Status)) || !(NtHeader))
    {
        /* Unmap the section and fail */
        NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
        return FALSE;
    }

    /* Go through the list of modules again */
    ListHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    ListEntry = ListHead->Flink;
    while (ListEntry != ListHead)
    {
        /* Get the current entry and advance to the next one */
        CurEntry = CONTAINING_RECORD(ListEntry,
                                     LDR_DATA_TABLE_ENTRY,
                                     InLoadOrderLinks);
        ListEntry = ListEntry->Flink;

        /* Check if it's in the process of being unloaded */
        if (!CurEntry->InMemoryOrderModuleList.Flink) continue;
        
        /* The header is untrusted, use SEH */
        _SEH2_TRY
        {
            /* Check if timedate stamp and sizes match */
            if ((CurEntry->TimeDateStamp == NtHeader->FileHeader.TimeDateStamp) &&
                (CurEntry->SizeOfImage == NtHeader->OptionalHeader.SizeOfImage))
            {
                /* Time, date and size match. Let's compare their headers */
                NtHeader2 = RtlImageNtHeader(CurEntry->DllBase);
                if (RtlCompareMemory(NtHeader2, NtHeader, sizeof(IMAGE_NT_HEADERS)))
                {
                    /* Headers match too! Finally ask the kernel to compare mapped files */
                    Status = ZwAreMappedFilesTheSame(CurEntry->DllBase, ViewBase);
                    if (!NT_SUCCESS(Status))
                    {
                        /* Almost identical, but not quite, keep trying */
                        _SEH2_YIELD(continue;)
                    }
                    else
                    {
                        /* This is our entry!, unmap and return success */
                        *LdrEntry = CurEntry;
                        NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
                        _SEH2_YIELD(return TRUE;)
                    }
                }
            }
        }
        _SEH2_EXCEPT (EXCEPTION_EXECUTE_HANDLER)
        {
            _SEH2_YIELD(break;)
        }
        _SEH2_END;
    }

    /* Unmap the section and fail */
    NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
    return FALSE;
}

NTSTATUS
NTAPI
LdrpGetProcedureAddress(IN PVOID BaseAddress,
                        IN PANSI_STRING Name,
                        IN ULONG Ordinal,
                        OUT PVOID *ProcedureAddress,
                        IN BOOLEAN ExecuteInit)
{
    NTSTATUS Status = STATUS_SUCCESS;
    UCHAR ImportBuffer[64];
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    IMAGE_THUNK_DATA Thunk;
    PVOID ImageBase;
    PIMAGE_IMPORT_BY_NAME ImportName = NULL;
    PIMAGE_EXPORT_DIRECTORY ExportDir;
    ULONG ExportDirSize, Length;
    PLIST_ENTRY Entry;

    /* Show debug message */
    if (ShowSnaps) DPRINT1("LDR: LdrGetProcedureAddress by ");

    /* Check if we got a name */
    if (Name)
    {
        /* Show debug message */
        if (ShowSnaps) DbgPrint("NAME - %s\n", Name->Buffer);

        /* Make sure it's not too long */
        Length = Name->Length +
                 sizeof(CHAR) +
                 FIELD_OFFSET(IMAGE_IMPORT_BY_NAME, Name);
        if (Length > UNICODE_STRING_MAX_BYTES)
        {
            /* Won't have enough space to add the hint */
            return STATUS_NAME_TOO_LONG;
        }

        /* Check if our buffer is large enough */
        if (Name->Length > sizeof(ImportBuffer))
        {
            /* Allocate from heap, plus 2 bytes for the Hint */
            ImportName = RtlAllocateHeap(RtlGetProcessHeap(),
                                         0,
                                         Length);
        }
        else
        {
            /* Use our internal buffer */
            ImportName = (PIMAGE_IMPORT_BY_NAME)ImportBuffer;
        }

        /* Clear the hint */
        ImportName->Hint = 0;

        /* Copy the name and null-terminate it */
        RtlCopyMemory(ImportName->Name, Name->Buffer, Name->Length);
        ImportName->Name[Name->Length] = ANSI_NULL;

        /* Clear the high bit */
        ImageBase = ImportName;
        Thunk.u1.AddressOfData = 0;
    }
    else
    {
        /* Do it by ordinal */
        ImageBase = NULL;

        /* Show debug message */
        if (ShowSnaps) DbgPrint("ORDINAL - %lx\n", Ordinal);

        /* Make sure an ordinal was given */
        if (!Ordinal)
        {
            /* No ordinal */
            DPRINT1("No ordinal and no name\n");
            return STATUS_INVALID_PARAMETER;
        }

        /* Set the orginal flag in the thunk */
        Thunk.u1.Ordinal = Ordinal | IMAGE_ORDINAL_FLAG;
    }

    /* Acquire lock unless we are initting */
    if (!LdrpInLdrInit) RtlEnterCriticalSection(&LdrpLoaderLock);

    _SEH2_TRY
    {
        /* Try to find the loaded DLL */
        if (!LdrpCheckForLoadedDllHandle(BaseAddress, &LdrEntry))
        {
            /* Invalid base */
            DPRINT1("Invalid base address %p\n", BaseAddress);
            Status = STATUS_DLL_NOT_FOUND;
            _SEH2_YIELD(goto Quickie;)
        }

        /* Get the pointer to the export directory */
        ExportDir = RtlImageDirectoryEntryToData(LdrEntry->DllBase,
                                                 TRUE,
                                                 IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                 &ExportDirSize);

        if (!ExportDir)
        {
            DPRINT1("Image %wZ has no exports, but were trying to get procedure %s. BaseAddress asked %p, got entry BA %p\n", &LdrEntry->BaseDllName, Name ? Name->Buffer : NULL, BaseAddress, LdrEntry->DllBase);
            Status = STATUS_PROCEDURE_NOT_FOUND;
            _SEH2_YIELD(goto Quickie;)
        }

        /* Now get the thunk */
        Status = LdrpSnapThunk(LdrEntry->DllBase,
                               ImageBase,
                               &Thunk,
                               &Thunk,
                               ExportDir,
                               ExportDirSize,
                               FALSE,
                               NULL);

        /* Finally, see if we're supposed to run the init routines */
        if ((NT_SUCCESS(Status)) && (ExecuteInit))
        {
            /*
            * It's possible a forwarded entry had us load the DLL. In that case,
            * then we will call its DllMain. Use the last loaded DLL for this.
            */
            Entry = NtCurrentPeb()->Ldr->InInitializationOrderModuleList.Blink;
            LdrEntry = CONTAINING_RECORD(Entry,
                                         LDR_DATA_TABLE_ENTRY,
                                         InInitializationOrderModuleList);

            /* Make sure we didn't process it yet*/
            if (!(LdrEntry->Flags & LDRP_ENTRY_PROCESSED))
            {
                /* Call the init routine */
                _SEH2_TRY
                {
                    Status = LdrpRunInitializeRoutines(NULL);
                }
                _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
                {
                    /* Get the exception code */
                    Status = _SEH2_GetExceptionCode();
                }
                _SEH2_END;
            }
        }

        /* Make sure we're OK till here */
        if (NT_SUCCESS(Status))
        {
            /* Return the address */
            *ProcedureAddress = (PVOID)Thunk.u1.Function;
        }
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Just ignore exceptions */
    }
    _SEH2_END;

Quickie:
    /* Cleanup */
    if (ImportName && (ImportName != (PIMAGE_IMPORT_BY_NAME)ImportBuffer))
    {
        /* We allocated from heap, free it */
        RtlFreeHeap(RtlGetProcessHeap(), 0, ImportName);
    }

    /* Release the CS if we entered it */
    if (!LdrpInLdrInit) RtlLeaveCriticalSection(&LdrpLoaderLock);

    /* We're done */
    return Status;
}

NTSTATUS
NTAPI
LdrpLoadDll(IN BOOLEAN Redirected,
            IN PWSTR DllPath OPTIONAL,
            IN PULONG DllCharacteristics OPTIONAL,
            IN PUNICODE_STRING DllName,
            OUT PVOID *BaseAddress,
            IN BOOLEAN CallInit)
{
    PPEB Peb = NtCurrentPeb();
    NTSTATUS Status = STATUS_SUCCESS;
    PWCHAR p1, p2;
    WCHAR c;
    WCHAR NameBuffer[266];
    LPWSTR RawDllName;
    UNICODE_STRING RawDllNameString;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    BOOLEAN InInit = LdrpInLdrInit;

    /* Find the name without the extension */
    p1 = DllName->Buffer;
    p2 = NULL;
    while (*p1)
    {
        c = *p1++;
        if (c == L'.')
        {
            p2 = p1;
        }
        else if (c == L'\\')
        {
            p2 = NULL;
        }
    }

    /* Save the Raw DLL Name */
    RawDllName = NameBuffer;
    if (DllName->Length >= sizeof(NameBuffer)) return STATUS_NAME_TOO_LONG;
    RtlMoveMemory(RawDllName, DllName->Buffer, DllName->Length);

    /* Check if no extension was found or if we got a slash */
    if (!(p2) || (*p2 == '\\'))
    {
        /* Check that we have space to add one */
        if ((DllName->Length + LdrApiDefaultExtension.Length + sizeof(UNICODE_NULL)) >=
            sizeof(NameBuffer))
        {
            /* No space to add the extension */
            DbgPrintEx(81, //DPFLTR_LDR_ID,
                       0,
                       "LDR: %s - Dll name missing extension; with extension "
                       "added the name is too long\n"
                       "   DllName: (@ %p) \"%wZ\"\n"
                       "   DllName->Length: %u\n",
                       __FUNCTION__,
                       DllName,
                       DllName,
                       DllName->Length);
            return STATUS_NAME_TOO_LONG;
        }

        /* FIXME: CLEAN THIS UP WITH Rtl String Functions */
        /* Add it */
        RtlMoveMemory((PVOID)((ULONG_PTR)RawDllName + DllName->Length),
                      LdrApiDefaultExtension.Buffer,
                      LdrApiDefaultExtension.Length);

        /* Save the length to a unicode string */
        RawDllNameString.Length = DllName->Length + LdrApiDefaultExtension.Length;

        /* Null terminate it */
        RawDllName[RawDllNameString.Length / sizeof(WCHAR)] = 0;
    }
    else
    {
        /* Null terminate it */
        RawDllName[DllName->Length / sizeof(WCHAR)] = 0;

        /* Save the length to a unicode string */
        RawDllNameString.Length = DllName->Length;
    }

    /* Now create a unicode string for the DLL's name */
    RawDllNameString.MaximumLength = sizeof(NameBuffer);
    RawDllNameString.Buffer = NameBuffer;

    /* Check for init flag and acquire lock */
    if (!InInit) RtlEnterCriticalSection(&LdrpLoaderLock);

    /* Show debug message */
    if (ShowSnaps)
    {
        DPRINT1("LDR: LdrLoadDll, loading %ws from %ws\n",
                 RawDllName,
                 DllPath ? DllPath : L"");
    }

    /* Check if the DLL is already loaded */
    if (!LdrpCheckForLoadedDll(DllPath,
                               &RawDllNameString,
                               FALSE,
                               Redirected,
                               &LdrEntry))
    {
        /* Map it */
        Status = LdrpMapDll(DllPath,
                            DllPath,
                            NameBuffer,
                            DllCharacteristics,
                            FALSE,
                            Redirected,
                            &LdrEntry);
        if (!NT_SUCCESS(Status)) goto Quickie;
        
        /* FIXME: Need to mark the DLL range for the stack DB */
        //RtlpStkMarkDllRange(LdrEntry);

        /* Check if IMAGE_FILE_EXECUTABLE_IMAGE was provided */
        if ((DllCharacteristics) &&
            (*DllCharacteristics & IMAGE_FILE_EXECUTABLE_IMAGE))
        {
            /* This is not a DLL, so remove such data */
            LdrEntry->EntryPoint = NULL;
            LdrEntry->Flags &= ~LDRP_IMAGE_DLL;
        }

        /* Make sure it's a DLL */
        if (LdrEntry->Flags & LDRP_IMAGE_DLL)
        {
            /* Check if this is a .NET Image */
            if (!(LdrEntry->Flags & LDRP_COR_IMAGE))
            {
                /* Walk the Import Descriptor */
                Status = LdrpWalkImportDescriptor(DllPath, LdrEntry);
            }

            /* Update load count, unless it's locked */
            if (LdrEntry->LoadCount != 0xFFFF) LdrEntry->LoadCount++;
            LdrpUpdateLoadCount2(LdrEntry, LDRP_UPDATE_REFCOUNT);

            /* Check if we failed */
            if (!NT_SUCCESS(Status))
            {
                /* Clear entrypoint, and insert into list */
                LdrEntry->EntryPoint = NULL;
                InsertTailList(&Peb->Ldr->InInitializationOrderModuleList,
                               &LdrEntry->InInitializationOrderModuleList);

                /* Cancel the load */
                LdrpClearLoadInProgress();
                
                /* Unload the DLL */
                if (ShowSnaps)
                {
                    DbgPrint("LDR: Unloading %wZ due to error %x walking "
                             "import descriptors",
                             DllName,
                             Status);
                }
                LdrUnloadDll(LdrEntry->DllBase);

                /* Return the error */
                goto Quickie;
            }
        }
        else if (LdrEntry->LoadCount != 0xFFFF)
        {
            /* Increase load count */
            LdrEntry->LoadCount++;
        }

        /* Insert it into the list */
        InsertTailList(&Peb->Ldr->InInitializationOrderModuleList,
                       &LdrEntry->InInitializationOrderModuleList);

        /* If we have to run the entrypoint, make sure the DB is ready */
        if (CallInit && LdrpLdrDatabaseIsSetup)
        {
            /* FIXME: Notify Shim Engine */
            if (g_ShimsEnabled)
            {
                /* Call it */
                //ShimLoadCallback = RtlDecodeSystemPointer(g_pfnSE_DllLoaded);
                //ShimLoadCallback(LdrEntry);
            }
            
            /* Run the init routine */
            Status = LdrpRunInitializeRoutines(NULL);
            if (!NT_SUCCESS(Status))
            {
                /* Failed, unload the DLL */
                if (ShowSnaps)
                {
                    DbgPrint("LDR: Unloading %wZ because either its init "
                             "routine or one of its static imports failed; "
                             "status = 0x%08lx\n",
                             DllName,
                             Status);
                }
                LdrUnloadDll(LdrEntry->DllBase);
            }
        }
        else
        {
            /* The DB isn't ready, which means we were loaded because of a forwarder */
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        /* We were already loaded. Are we a DLL? */
        if ((LdrEntry->Flags & LDRP_IMAGE_DLL) && (LdrEntry->LoadCount != 0xFFFF))
        {
            /* Increase load count */
            LdrEntry->LoadCount++;
            LdrpUpdateLoadCount2(LdrEntry, LDRP_UPDATE_REFCOUNT);

            /* Clear the load in progress */
            LdrpClearLoadInProgress();
        }
        else
        {
            /* Not a DLL, just increase the load count */
            if (LdrEntry->LoadCount != 0xFFFF) LdrEntry->LoadCount++;
        }
    }

Quickie:
    /* Release the lock */
    if (!InInit) RtlLeaveCriticalSection(Peb->LoaderLock);

    /* Check for success */
    if (NT_SUCCESS(Status))
    {
        /* Return the base address */
        *BaseAddress = LdrEntry->DllBase;
    }
    else
    {
        /* Nothing found */
        *BaseAddress = NULL;
    }

    /* Return status */
    return Status;
}

ULONG
NTAPI
LdrpClearLoadInProgress(VOID)
{
    PLIST_ENTRY ListHead, Entry;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    ULONG ModulesCount = 0;

    /* Traverse the init list */
    ListHead = &NtCurrentPeb()->Ldr->InInitializationOrderModuleList;
    Entry = ListHead->Flink;
    while (Entry != ListHead)
    {
        /* Get the loader entry */
        LdrEntry = CONTAINING_RECORD(Entry,
                                     LDR_DATA_TABLE_ENTRY,
                                     InInitializationOrderModuleList);

        /* Clear load in progress flag */
        LdrEntry->Flags &= ~LDRP_LOAD_IN_PROGRESS;

        /* Check for modules with entry point count but not processed yet */
        if ((LdrEntry->EntryPoint) &&
            !(LdrEntry->Flags & LDRP_ENTRY_PROCESSED))
        {
            /* Increase counter */
            ModulesCount++;
        }

        /* Advance to the next entry */
        Entry = Entry->Flink;
    }

    /* Return final count */
    return ModulesCount;
}

/* EOF */
