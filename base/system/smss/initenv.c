/*
 * PROJECT:         Odyssey Session Manager
 * LICENSE:         GPL v2 or later - See COPYING in the top level directory
 * FILE:            base/system/smss/initenv.c
 * PURPOSE:         Environment initialization.
 * PROGRAMMERS:     Odyssey Development Team
 */

/* INCLUDES ******************************************************************/
#include "smss.h"

#define NDEBUG
#include <debug.h>

/* GLOBALS */

PWSTR SmSystemEnvironment = NULL;


/* FUNCTIONS */

NTSTATUS
SmCreateEnvironment(VOID)
{
    return RtlCreateEnvironment(FALSE, &SmSystemEnvironment);
}


static NTSTATUS
SmpSetEnvironmentVariable(IN PVOID Context,
                          IN PWSTR ValueName,
                          IN PWSTR ValueData)
{
    UNICODE_STRING EnvVariable;
    UNICODE_STRING EnvValue;

    RtlInitUnicodeString(&EnvVariable,
                         ValueName);
    RtlInitUnicodeString(&EnvValue,
                         ValueData);
    return RtlSetEnvironmentVariable(Context,
                              &EnvVariable,
                              &EnvValue);
}


static NTSTATUS NTAPI
SmpEnvironmentQueryRoutine(IN PWSTR ValueName,
                           IN ULONG ValueType,
                           IN PVOID ValueData,
                           IN ULONG ValueLength,
                           IN PVOID Context,
                           IN PVOID EntryContext)
{
    DPRINT("ValueName '%S'  Type %lu  Length %lu\n", ValueName, ValueType, ValueLength);

    if (ValueType != REG_SZ && ValueType != REG_EXPAND_SZ)
        return STATUS_SUCCESS;

    DPRINT("ValueData '%S'\n", (PWSTR)ValueData);
    return SmpSetEnvironmentVariable(Context,ValueName,(PWSTR)ValueData);
}


NTSTATUS
SmSetEnvironmentVariables(VOID)
{
    SYSTEM_BASIC_INFORMATION BasicInformation;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInformation;
    RTL_QUERY_REGISTRY_TABLE QueryTable[3];
    UNICODE_STRING Identifier;
    UNICODE_STRING VendorIdentifier;
    WCHAR Buffer[256];
    UNICODE_STRING EnvironmentKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE EnvironmentKey;
    UNICODE_STRING VariableName;
    PWSTR VariableData;
    NTSTATUS Status;

    Status = NtQuerySystemInformation(SystemProcessorInformation,
                                      &ProcessorInformation,
                                      sizeof(SYSTEM_PROCESSOR_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to retrieve system processor information (Status %08lx)", Status);
        return Status;
    }

    Status = NtQuerySystemInformation(SystemBasicInformation,
                                      &BasicInformation,
                                      sizeof(SYSTEM_BASIC_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to retrieve system basic information (Status %08lx)", Status);
        return Status;
    }

    RtlInitUnicodeString(&EnvironmentKeyName,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Environment");
    InitializeObjectAttributes(&ObjectAttributes,
                               &EnvironmentKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    /* Open the system environment key */
    Status = NtOpenKey(&EnvironmentKey,
                       GENERIC_WRITE,
                       &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to open the environment key (Status %08lx)", Status);
        return Status;
    }

    /* Set the 'NUMBER_OF_PROCESSORS' system environment variable */
    RtlInitUnicodeString(&VariableName,
                         L"NUMBER_OF_PROCESSORS");

    swprintf(Buffer, L"%lu", BasicInformation.NumberOfProcessors);

    Status = NtSetValueKey(EnvironmentKey,
                           &VariableName,
                           0,
                           REG_SZ,
                           Buffer,
                           (wcslen(Buffer) + 1) * sizeof(WCHAR));
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to set the NUMBER_OF_PROCESSORS environment variable (Status %08lx)", Status);
        goto done;
    }

    /* Set the 'OS' system environment variable */
    RtlInitUnicodeString(&VariableName,
                         L"OS");

    VariableData = L"Odyssey";

    Status = NtSetValueKey(EnvironmentKey,
                           &VariableName,
                           0,
                           REG_SZ,
                           VariableData,
                           (wcslen(VariableData) + 1) * sizeof(WCHAR));
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to set the OS environment variable (Status %08lx)", Status);
        goto done;
    }

    /* Set the 'PROCESSOR_ARCHITECTURE' system environment variable */
    RtlInitUnicodeString(&VariableName,
                         L"PROCESSOR_ARCHITECTURE");

    switch (ProcessorInformation.ProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_INTEL:
            VariableData = L"x86";
            break;

        case PROCESSOR_ARCHITECTURE_PPC:
            VariableData = L"PPC";
            break;

        case PROCESSOR_ARCHITECTURE_ARM:
            VariableData = L"ARM";
            break;

        case PROCESSOR_ARCHITECTURE_AMD64:
            VariableData = L"AMD64";
            break;

        default:
            VariableData = L"Unknown";
            break;
    }

    Status = NtSetValueKey(EnvironmentKey,
                           &VariableName,
                           0,
                           REG_SZ,
                           VariableData,
                           (wcslen(VariableData) + 1) * sizeof(WCHAR));
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to set the PROCESSOR_ARCHITECTURE environment variable (Status %08lx)", Status);
        goto done;
    }

    /* Set the 'PROCESSOR_LEVEL' system environment variable */
    RtlInitUnicodeString(&VariableName,
                         L"PROCESSOR_LEVEL");

    swprintf(Buffer, L"%lu", ProcessorInformation.ProcessorLevel);

    Status = NtSetValueKey(EnvironmentKey,
                           &VariableName,
                           0,
                           REG_SZ,
                           Buffer,
                           (wcslen(Buffer) + 1) * sizeof(WCHAR));
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to set the PROCESSOR_LEVEL environment variable (Status %08lx)", Status);
        goto done;
    }

    /* Set the 'PROCESSOR_REVISION' system environment variable */
    RtlInitUnicodeString(&VariableName,
                         L"PROCESSOR_REVISION");

    swprintf(Buffer, L"%04x", ProcessorInformation.ProcessorRevision);

    Status = NtSetValueKey(EnvironmentKey,
                           &VariableName,
                           0,
                           REG_SZ,
                           Buffer,
                           (wcslen(Buffer) + 1) * sizeof(WCHAR));
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to set the PROCESSOR_REVISION environment variable (Status %08lx)", Status);
        goto done;
    }

    /* Set the 'PROCESSOR_IDENTIFIER' system environment variable */
    RtlInitUnicodeString(&Identifier, NULL);
    RtlInitUnicodeString(&VendorIdentifier, NULL);

    RtlZeroMemory(&QueryTable,
                  sizeof(QueryTable));

    QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[0].Name = L"Identifier";
    QueryTable[0].EntryContext = &Identifier;

    QueryTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryTable[1].Name = L"VendorIdentifier";
    QueryTable[1].EntryContext = &VendorIdentifier;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    L"\\Registry\\Machine\\Hardware\\Description\\System\\CentralProcessor\\0",
                                    QueryTable,
                                    NULL,
                                    NULL);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to retrieve processor Identifier and/or VendorIdentifier (Status %08lx)", Status);
        goto done;
    }

    DPRINT("SM: szIdentifier: %wZ\n"      , &Identifier);
    DPRINT("SM: szVendorIdentifier: %wZ\n", &VendorIdentifier);

    RtlInitUnicodeString(&VariableName, L"PROCESSOR_IDENTIFIER");
    swprintf(Buffer, L"%wZ, %wZ", &Identifier, &VendorIdentifier);
    RtlFreeUnicodeString(&VendorIdentifier);
    RtlFreeUnicodeString(&Identifier);

    Status = NtSetValueKey(EnvironmentKey,
                           &VariableName,
                           0,
                           REG_SZ,
                           Buffer,
                           (wcslen(Buffer) + 1) * sizeof(WCHAR));
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("SM: Failed to set the PROCESSOR_IDENTIFIER environment variable (Status %08lx)", Status);
        goto done;
    }

done:
    /* Close the handle */
    NtClose(EnvironmentKey);

    return Status;
}


/**********************************************************************
 *  Set environment variables from registry
 */
NTSTATUS
SmUpdateEnvironment(VOID)
{
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    WCHAR ValueBuffer[MAX_PATH];
    NTSTATUS Status;
#ifndef NDEBUG
    ULONG ii;
    PWSTR envp;
#endif

    /*
     * The following environment variables must be set prior to reading
     * other variables from the registry.
     *
     * Variables (example):
     *    SystemRoot = "C:\odyssey"
     *    SystemDrive = "C:"
     */

    /* Copy system root into value buffer */
    wcscpy(ValueBuffer,
           SharedUserData->NtSystemRoot);

    /* Set SystemRoot = "C:\odyssey" */
    SmpSetEnvironmentVariable(&SmSystemEnvironment, L"SystemRoot", ValueBuffer);

    /* Cut off trailing path */
    ValueBuffer[2] = 0;

    /* Set SystemDrive = "C:" */
    SmpSetEnvironmentVariable(&SmSystemEnvironment, L"SystemDrive", ValueBuffer);

    /* Read system environment from the registry. */
    RtlZeroMemory(&QueryTable,
                  sizeof(QueryTable));

    QueryTable[0].QueryRoutine = SmpEnvironmentQueryRoutine;

    Status = RtlQueryRegistryValues(RTL_REGISTRY_CONTROL,
                                    L"Session Manager\\Environment",
                                    QueryTable,
                                    &SmSystemEnvironment,
                                    SmSystemEnvironment);

#ifndef NDEBUG
    /* Print all environment varaibles */
    ii = 0;
    envp = SmSystemEnvironment;
    DbgPrint("SmUpdateEnvironment:\n");
    while (*envp)
    {
        DbgPrint("  %u: %S\n", ii++, envp);
        envp += wcslen(envp) + 1;
    }
#endif

    return Status;
}

/* EOF */
