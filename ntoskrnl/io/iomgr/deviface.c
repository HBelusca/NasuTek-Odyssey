/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/io/iomgr/deviface.c
 * PURPOSE:         Device interface functions
 *
 * PROGRAMMERS:     Filip Navara (xnavara@volny.cz)
 *                  Matthew Brace (ismarc@austin.rr.com)
 *                  Herv� Poussineau (hpoussin@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include <ntoskrnl.h>

#define NDEBUG
#include <debug.h>

/* FUNCTIONS *****************************************************************/

static PWCHAR BaseKeyString = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\DeviceClasses\\";

static
NTSTATUS
OpenRegistryHandlesFromSymbolicLink(IN PUNICODE_STRING SymbolicLinkName,
                                    IN ACCESS_MASK DesiredAccess,
                                    IN OPTIONAL PHANDLE GuidKey,
                                    IN OPTIONAL PHANDLE DeviceKey,
                                    IN OPTIONAL PHANDLE InstanceKey)
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR PathBuffer[MAX_PATH];
    UNICODE_STRING BaseKeyU;
    UNICODE_STRING GuidString, SubKeyName, ReferenceString;
    PWCHAR StartPosition, EndPosition;
    HANDLE ClassesKey;
    PHANDLE GuidKeyRealP, DeviceKeyRealP, InstanceKeyRealP;
    HANDLE GuidKeyReal, DeviceKeyReal, InstanceKeyReal;
    NTSTATUS Status;

    SubKeyName.Buffer = NULL;

    if (GuidKey != NULL)
        GuidKeyRealP = GuidKey;
    else
        GuidKeyRealP = &GuidKeyReal;

    if (DeviceKey != NULL)
        DeviceKeyRealP = DeviceKey;
    else
        DeviceKeyRealP = &DeviceKeyReal;

    if (InstanceKey != NULL)
        InstanceKeyRealP = InstanceKey;
    else
        InstanceKeyRealP = &InstanceKeyReal;

    *GuidKeyRealP = INVALID_HANDLE_VALUE;
    *DeviceKeyRealP = INVALID_HANDLE_VALUE;
    *InstanceKeyRealP = INVALID_HANDLE_VALUE;

    BaseKeyU.Buffer = PathBuffer;
    BaseKeyU.Length = 0;
    BaseKeyU.MaximumLength = MAX_PATH * sizeof(WCHAR);

    RtlAppendUnicodeToString(&BaseKeyU, BaseKeyString);

    /* Open the DeviceClasses key */
    InitializeObjectAttributes(&ObjectAttributes,
                               &BaseKeyU,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    Status = ZwOpenKey(&ClassesKey,
                       DesiredAccess | KEY_ENUMERATE_SUB_KEYS,
                       &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to open %wZ\n", &BaseKeyU);
        goto cleanup;
    }

    StartPosition = wcschr(SymbolicLinkName->Buffer, L'{');
    EndPosition = wcschr(SymbolicLinkName->Buffer, L'}');
    if (!StartPosition || !EndPosition || StartPosition > EndPosition)
    {
        DPRINT1("Bad symbolic link: %wZ\n", SymbolicLinkName);
        return STATUS_INVALID_PARAMETER_1;
    }
    GuidString.Buffer = StartPosition;
    GuidString.MaximumLength = GuidString.Length = (USHORT)((ULONG_PTR)(EndPosition + 1) - (ULONG_PTR)StartPosition);

    InitializeObjectAttributes(&ObjectAttributes,
                               &GuidString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               ClassesKey,
                               NULL);
    Status = ZwOpenKey(GuidKeyRealP,
                       DesiredAccess | KEY_ENUMERATE_SUB_KEYS,
                       &ObjectAttributes);
    ZwClose(ClassesKey);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to open %wZ%wZ (%x)\n", &BaseKeyU, &GuidString, Status);
        goto cleanup;
    }

    SubKeyName.MaximumLength = SymbolicLinkName->Length + sizeof(WCHAR);
    SubKeyName.Length = 0;
    SubKeyName.Buffer = ExAllocatePool(PagedPool, SubKeyName.MaximumLength);
    if (!SubKeyName.Buffer)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    RtlAppendUnicodeStringToString(&SubKeyName,
                                   SymbolicLinkName);

    SubKeyName.Buffer[SubKeyName.Length / sizeof(WCHAR)] = UNICODE_NULL;

    SubKeyName.Buffer[0] = L'#';
    SubKeyName.Buffer[1] = L'#';
    SubKeyName.Buffer[2] = L'?';
    SubKeyName.Buffer[3] = L'#';

    ReferenceString.Buffer = wcsrchr(SubKeyName.Buffer, '\\');
    if (ReferenceString.Buffer != NULL)
    {
        ReferenceString.Buffer[0] = L'#';

        SubKeyName.Length = (USHORT)((ULONG_PTR)(ReferenceString.Buffer) - (ULONG_PTR)SubKeyName.Buffer);
        ReferenceString.Length = SymbolicLinkName->Length - SubKeyName.Length;
    }
    else
    {
        RtlInitUnicodeString(&ReferenceString, L"#");
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &SubKeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               *GuidKeyRealP,
                               NULL);
    Status = ZwOpenKey(DeviceKeyRealP,
                       DesiredAccess | KEY_ENUMERATE_SUB_KEYS,
                       &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to open %wZ%wZ\\%wZ\n", &BaseKeyU, &GuidString, &SubKeyName);
        goto cleanup;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &ReferenceString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               *DeviceKeyRealP,
                               NULL);
    Status = ZwOpenKey(InstanceKeyRealP,
                       DesiredAccess,
                       &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to open %wZ%wZ\\%wZ%\\%wZ (%x)\n", &BaseKeyU, &GuidString, &SubKeyName, &ReferenceString, Status);
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

cleanup:
    if (SubKeyName.Buffer != NULL)
       ExFreePool(SubKeyName.Buffer);

    if (NT_SUCCESS(Status))
    {
       if (!GuidKey)
          ZwClose(*GuidKeyRealP);

       if (!DeviceKey)
          ZwClose(*DeviceKeyRealP);

       if (!InstanceKey)
          ZwClose(*InstanceKeyRealP);
    }
    else
    {
       if (*GuidKeyRealP != INVALID_HANDLE_VALUE)
          ZwClose(*GuidKeyRealP);

       if (*DeviceKeyRealP != INVALID_HANDLE_VALUE)
          ZwClose(*DeviceKeyRealP);

       if (*InstanceKeyRealP != INVALID_HANDLE_VALUE)
          ZwClose(*InstanceKeyRealP);
    }

    return Status;
}
/*++
 * @name IoOpenDeviceInterfaceRegistryKey
 * @unimplemented
 *
 * Provides a handle to the device's interface instance registry key.
 * Documented in WDK.
 *
 * @param SymbolicLinkName
 *        Pointer to a string which identifies the device interface instance
 *
 * @param DesiredAccess
 *        Desired ACCESS_MASK used to access the key (like KEY_READ,
 *        KEY_WRITE, etc)
 *
 * @param DeviceInterfaceKey
 *        If a call has been succesfull, a handle to the registry key
 *        will be stored there
 *
 * @return Three different NTSTATUS values in case of errors, and STATUS_SUCCESS
 *         otherwise (see WDK for details)
 *
 * @remarks Must be called at IRQL = PASSIVE_LEVEL in the context of a system thread
 *
 *--*/
NTSTATUS
NTAPI
IoOpenDeviceInterfaceRegistryKey(IN PUNICODE_STRING SymbolicLinkName,
                                 IN ACCESS_MASK DesiredAccess,
                                 OUT PHANDLE DeviceInterfaceKey)
{
   HANDLE InstanceKey, DeviceParametersKey;
   NTSTATUS Status;
   OBJECT_ATTRIBUTES ObjectAttributes;
   UNICODE_STRING DeviceParametersU = RTL_CONSTANT_STRING(L"Device Parameters");

   Status = OpenRegistryHandlesFromSymbolicLink(SymbolicLinkName,
                                                KEY_CREATE_SUB_KEY,
                                                NULL,
                                                NULL,
                                                &InstanceKey);
   if (!NT_SUCCESS(Status))
       return Status;

   InitializeObjectAttributes(&ObjectAttributes,
                              &DeviceParametersU,
                              OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                              InstanceKey,
                              NULL);
   Status = ZwCreateKey(&DeviceParametersKey,
                        DesiredAccess,
                        &ObjectAttributes,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        NULL);
   ZwClose(InstanceKey);

   if (NT_SUCCESS(Status))
       *DeviceInterfaceKey = DeviceParametersKey;

   return Status;
}

/*++
 * @name IoGetDeviceInterfaceAlias
 * @unimplemented
 *
 * Returns the alias device interface of the specified device interface
 * instance, if the alias exists.
 * Documented in WDK.
 *
 * @param SymbolicLinkName
 *        Pointer to a string which identifies the device interface instance
 *
 * @param AliasInterfaceClassGuid
 *        See WDK
 *
 * @param AliasSymbolicLinkName
 *        See WDK
 *
 * @return Three different NTSTATUS values in case of errors, and STATUS_SUCCESS
 *         otherwise (see WDK for details)
 *
 * @remarks Must be called at IRQL = PASSIVE_LEVEL in the context of a system thread
 *
 *--*/
NTSTATUS
NTAPI
IoGetDeviceInterfaceAlias(IN PUNICODE_STRING SymbolicLinkName,
                          IN CONST GUID *AliasInterfaceClassGuid,
                          OUT PUNICODE_STRING AliasSymbolicLinkName)
{
    return STATUS_NOT_IMPLEMENTED;
}

/*++
 * @name IopOpenInterfaceKey
 *
 * Returns the alias device interface of the specified device interface
 *
 * @param InterfaceClassGuid
 *        FILLME
 *
 * @param DesiredAccess
 *        FILLME
 *
 * @param pInterfaceKey
 *        FILLME
 *
 * @return Usual NTSTATUS
 *
 * @remarks None
 *
 *--*/
static NTSTATUS
IopOpenInterfaceKey(IN CONST GUID *InterfaceClassGuid,
                    IN ACCESS_MASK DesiredAccess,
                    OUT HANDLE *pInterfaceKey)
{
    UNICODE_STRING LocalMachine = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\");
    UNICODE_STRING GuidString;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE InterfaceKey = INVALID_HANDLE_VALUE;
    NTSTATUS Status;

    GuidString.Buffer = KeyName.Buffer = NULL;

    Status = RtlStringFromGUID(InterfaceClassGuid, &GuidString);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("RtlStringFromGUID() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }

    KeyName.Length = 0;
    KeyName.MaximumLength = LocalMachine.Length + (wcslen(REGSTR_PATH_DEVICE_CLASSES) + 1) * sizeof(WCHAR) + GuidString.Length;
    KeyName.Buffer = ExAllocatePool(PagedPool, KeyName.MaximumLength);
    if (!KeyName.Buffer)
    {
        DPRINT("ExAllocatePool() failed\n");
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    Status = RtlAppendUnicodeStringToString(&KeyName, &LocalMachine);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("RtlAppendUnicodeStringToString() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }
    Status = RtlAppendUnicodeToString(&KeyName, REGSTR_PATH_DEVICE_CLASSES);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("RtlAppendUnicodeToString() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }
    Status = RtlAppendUnicodeToString(&KeyName, L"\\");
    if (!NT_SUCCESS(Status))
    {
        DPRINT("RtlAppendUnicodeToString() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }
    Status = RtlAppendUnicodeStringToString(&KeyName, &GuidString);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("RtlAppendUnicodeStringToString() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);
    Status = ZwOpenKey(
        &InterfaceKey,
        DesiredAccess,
        &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwOpenKey() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }

    *pInterfaceKey = InterfaceKey;
    Status = STATUS_SUCCESS;

cleanup:
    if (!NT_SUCCESS(Status))
    {
        if (InterfaceKey != INVALID_HANDLE_VALUE)
            ZwClose(InterfaceKey);
    }
    RtlFreeUnicodeString(&GuidString);
    RtlFreeUnicodeString(&KeyName);
    return Status;
}

/*++
 * @name IoGetDeviceInterfaces
 * @implemented
 *
 * Returns a list of device interfaces of a particular device interface class.
 * Documented in WDK
 *
 * @param InterfaceClassGuid
 *        Points to a class GUID specifying the device interface class
 *
 * @param PhysicalDeviceObject
 *        Points to an optional PDO that narrows the search to only the
 *        device interfaces of the device represented by the PDO
 *
 * @param Flags
 *        Specifies flags that modify the search for device interfaces. The
 *        DEVICE_INTERFACE_INCLUDE_NONACTIVE flag specifies that the list of
 *        returned symbolic links should contain also disabled device
 *        interfaces in addition to the enabled ones.
 *
 * @param SymbolicLinkList
 *        Points to a character pointer that is filled in on successful return
 *        with a list of unicode strings identifying the device interfaces
 *        that match the search criteria. The newly allocated buffer contains
 *        a list of symbolic link names. Each unicode string in the list is
 *        null-terminated; the end of the whole list is marked by an additional
 *        NULL. The caller is responsible for freeing the buffer (ExFreePool)
 *        when it is no longer needed.
 *        If no device interfaces match the search criteria, this routine
 *        returns STATUS_SUCCESS and the string contains a single NULL
 *        character.
 *
 * @return Usual NTSTATUS
 *
 * @remarks None
 *
 *--*/
NTSTATUS
NTAPI
IoGetDeviceInterfaces(IN CONST GUID *InterfaceClassGuid,
                      IN PDEVICE_OBJECT PhysicalDeviceObject OPTIONAL,
                      IN ULONG Flags,
                      OUT PWSTR *SymbolicLinkList)
{
    UNICODE_STRING Control = RTL_CONSTANT_STRING(L"Control");
    UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"SymbolicLink");
    HANDLE InterfaceKey = INVALID_HANDLE_VALUE;
    HANDLE DeviceKey = INVALID_HANDLE_VALUE;
    HANDLE ReferenceKey = INVALID_HANDLE_VALUE;
    HANDLE ControlKey = INVALID_HANDLE_VALUE;
    PKEY_BASIC_INFORMATION DeviceBi = NULL;
    PKEY_BASIC_INFORMATION ReferenceBi = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION bip = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN FoundRightPDO = FALSE;
    ULONG i = 0, j, Size, NeededLength, ActualLength, LinkedValue;
    UNICODE_STRING ReturnBuffer = { 0, 0, NULL };
    NTSTATUS Status;

    PAGED_CODE();

    Status = IopOpenInterfaceKey(InterfaceClassGuid, KEY_ENUMERATE_SUB_KEYS, &InterfaceKey);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("IopOpenInterfaceKey() failed with status 0x%08lx\n", Status);
        goto cleanup;
    }

    /* Enumerate subkeys (ie the different device objets) */
    while (TRUE)
    {
        Status = ZwEnumerateKey(
            InterfaceKey,
            i,
            KeyBasicInformation,
            NULL,
            0,
            &Size);
        if (Status == STATUS_NO_MORE_ENTRIES)
        {
            break;
        }
        else if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL)
        {
            DPRINT("ZwEnumerateKey() failed with status 0x%08lx\n", Status);
            goto cleanup;
        }

        DeviceBi = ExAllocatePool(PagedPool, Size);
        if (!DeviceBi)
        {
            DPRINT("ExAllocatePool() failed\n");
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
        Status = ZwEnumerateKey(
            InterfaceKey,
            i++,
            KeyBasicInformation,
            DeviceBi,
            Size,
            &Size);
        if (!NT_SUCCESS(Status))
        {
            DPRINT("ZwEnumerateKey() failed with status 0x%08lx\n", Status);
            goto cleanup;
        }

        /* Open device key */
        KeyName.Length = KeyName.MaximumLength = (USHORT)DeviceBi->NameLength;
        KeyName.Buffer = DeviceBi->Name;
        InitializeObjectAttributes(
            &ObjectAttributes,
            &KeyName,
            OBJ_CASE_INSENSITIVE,
            InterfaceKey,
            NULL);
        Status = ZwOpenKey(
            &DeviceKey,
            KEY_ENUMERATE_SUB_KEYS,
            &ObjectAttributes);
        if (!NT_SUCCESS(Status))
        {
            DPRINT("ZwOpenKey() failed with status 0x%08lx\n", Status);
            goto cleanup;
        }

        if (PhysicalDeviceObject)
        {
            /* Check if we are on the right physical device object,
            * by reading the DeviceInstance string
            */
            DPRINT1("PhysicalDeviceObject != NULL. Case not implemented.\n");
            //FoundRightPDO = TRUE;
            Status = STATUS_NOT_IMPLEMENTED;
            goto cleanup;
        }

        /* Enumerate subkeys (ie the different reference strings) */
        j = 0;
        while (TRUE)
        {
            Status = ZwEnumerateKey(
                DeviceKey,
                j,
                KeyBasicInformation,
                NULL,
                0,
                &Size);
            if (Status == STATUS_NO_MORE_ENTRIES)
            {
                break;
            }
            else if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL)
            {
                DPRINT("ZwEnumerateKey() failed with status 0x%08lx\n", Status);
                goto cleanup;
            }

            ReferenceBi = ExAllocatePool(PagedPool, Size);
            if (!ReferenceBi)
            {
                DPRINT("ExAllocatePool() failed\n");
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
            Status = ZwEnumerateKey(
                DeviceKey,
                j++,
                KeyBasicInformation,
                ReferenceBi,
                Size,
                &Size);
            if (!NT_SUCCESS(Status))
            {
                DPRINT("ZwEnumerateKey() failed with status 0x%08lx\n", Status);
                goto cleanup;
            }

            KeyName.Length = KeyName.MaximumLength = (USHORT)ReferenceBi->NameLength;
            KeyName.Buffer = ReferenceBi->Name;
            if (RtlEqualUnicodeString(&KeyName, &Control, TRUE))
            {
                /* Skip Control subkey */
                goto NextReferenceString;
            }

            /* Open reference key */
            InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyName,
                OBJ_CASE_INSENSITIVE,
                DeviceKey,
                NULL);
            Status = ZwOpenKey(
                &ReferenceKey,
                KEY_QUERY_VALUE,
                &ObjectAttributes);
            if (!NT_SUCCESS(Status))
            {
                DPRINT("ZwOpenKey() failed with status 0x%08lx\n", Status);
                goto cleanup;
            }

            if (!(Flags & DEVICE_INTERFACE_INCLUDE_NONACTIVE))
            {
                /* We have to check if the interface is enabled, by
                * reading the Linked value in the Control subkey
                */
                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &Control,
                    OBJ_CASE_INSENSITIVE,
                    ReferenceKey,
                    NULL);
                Status = ZwOpenKey(
                    &ControlKey,
                    KEY_QUERY_VALUE,
                    &ObjectAttributes);
                if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
                {
                    /* That's OK. The key doesn't exist (yet) because
                    * the interface is not activated.
                    */
                    goto NextReferenceString;
                }
                else if (!NT_SUCCESS(Status))
                {
                    DPRINT1("ZwOpenKey() failed with status 0x%08lx\n", Status);
                    goto cleanup;
                }

                RtlInitUnicodeString(&KeyName, L"Linked");
                Status = ZwQueryValueKey(ControlKey,
                                         &KeyName,
                                         KeyValuePartialInformation,
                                         NULL,
                                         0,
                                         &NeededLength);
                if (Status == STATUS_BUFFER_TOO_SMALL)
                {
                    ActualLength = NeededLength;
                    PartialInfo = ExAllocatePool(NonPagedPool, ActualLength);
                    if (!PartialInfo)
                    {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto cleanup;
                    }

                    Status = ZwQueryValueKey(ControlKey,
                                             &KeyName,
                                             KeyValuePartialInformation,
                                             PartialInfo,
                                             ActualLength,
                                             &NeededLength);
                    if (!NT_SUCCESS(Status))
                    {
                        DPRINT1("ZwQueryValueKey #2 failed (%x)\n", Status);
                        ExFreePool(PartialInfo);
                        goto cleanup;
                    }

                    if (PartialInfo->Type != REG_DWORD || PartialInfo->DataLength != sizeof(ULONG))
                    {
                        DPRINT1("Bad registry read\n");
                        ExFreePool(PartialInfo);
                        goto cleanup;
                    }

                    RtlCopyMemory(&LinkedValue,
                                  PartialInfo->Data,
                                  PartialInfo->DataLength);

                    ExFreePool(PartialInfo);
                    if (LinkedValue == 0)
                    {
                        /* This interface isn't active */
                        goto NextReferenceString;
                    }
                }
                else
                {
                    DPRINT1("ZwQueryValueKey #1 failed (%x)\n", Status);
                    goto cleanup;
                }
            }

            /* Read the SymbolicLink string and add it into SymbolicLinkList */
            Status = ZwQueryValueKey(
                ReferenceKey,
                &SymbolicLink,
                KeyValuePartialInformation,
                NULL,
                0,
                &Size);
            if (!NT_SUCCESS(Status) && Status != STATUS_BUFFER_TOO_SMALL)
            {
                DPRINT("ZwQueryValueKey() failed with status 0x%08lx\n", Status);
                goto cleanup;
            }
            bip = ExAllocatePool(PagedPool, Size);
            if (!bip)
            {
                DPRINT("ExAllocatePool() failed\n");
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto cleanup;
            }
            Status = ZwQueryValueKey(
                ReferenceKey,
                &SymbolicLink,
                KeyValuePartialInformation,
                bip,
                Size,
                &Size);
            if (!NT_SUCCESS(Status))
            {
                DPRINT("ZwQueryValueKey() failed with status 0x%08lx\n", Status);
                goto cleanup;
            }
            else if (bip->Type != REG_SZ)
            {
                DPRINT("Unexpected registry type 0x%lx (expected 0x%lx)\n", bip->Type, REG_SZ);
                Status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }
            else if (bip->DataLength < 5 * sizeof(WCHAR))
            {
                DPRINT("Registry string too short (length %lu, expected %lu at least)\n", bip->DataLength < 5 * sizeof(WCHAR));
                Status = STATUS_UNSUCCESSFUL;
                goto cleanup;
            }
            KeyName.Length = KeyName.MaximumLength = (USHORT)bip->DataLength - 4 * sizeof(WCHAR);
            KeyName.Buffer = &((PWSTR)bip->Data)[4];

            /* Add new symbolic link to symbolic link list */
            if (ReturnBuffer.Length + KeyName.Length + sizeof(WCHAR) > ReturnBuffer.MaximumLength)
            {
                PWSTR NewBuffer;
                ReturnBuffer.MaximumLength = max(ReturnBuffer.MaximumLength * 2, ReturnBuffer.Length + KeyName.Length + 2 * sizeof(WCHAR));
                NewBuffer = ExAllocatePool(PagedPool, ReturnBuffer.MaximumLength);
                if (!NewBuffer)
                {
                    DPRINT("ExAllocatePool() failed\n");
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto cleanup;
                }
                RtlCopyMemory(NewBuffer, ReturnBuffer.Buffer, ReturnBuffer.Length);
                if (ReturnBuffer.Buffer)
                    ExFreePool(ReturnBuffer.Buffer);
                ReturnBuffer.Buffer = NewBuffer;
            }
            DPRINT("Adding symbolic link %wZ\n", &KeyName);
            Status = RtlAppendUnicodeStringToString(&ReturnBuffer, &KeyName);
            if (!NT_SUCCESS(Status))
            {
                DPRINT("RtlAppendUnicodeStringToString() failed with status 0x%08lx\n", Status);
                goto cleanup;
            }
            /* RtlAppendUnicodeStringToString added a NULL at the end of the 	 
	     * destination string, but didn't increase the Length field. 	 
	     * Do it for it. 	 
	     */ 	 
	    ReturnBuffer.Length += sizeof(WCHAR);

NextReferenceString:
            ExFreePool(ReferenceBi);
            ReferenceBi = NULL;
            if (bip)
                ExFreePool(bip);
            bip = NULL;
            if (ReferenceKey != INVALID_HANDLE_VALUE)
            {
                ZwClose(ReferenceKey);
                ReferenceKey = INVALID_HANDLE_VALUE;
            }
            if (ControlKey != INVALID_HANDLE_VALUE)
            {
                ZwClose(ControlKey);
                ControlKey = INVALID_HANDLE_VALUE;
            }
        }
        if (FoundRightPDO)
        {
            /* No need to go further, as we already have found what we searched */
            break;
        }

        ExFreePool(DeviceBi);
        DeviceBi = NULL;
        ZwClose(DeviceKey);
        DeviceKey = INVALID_HANDLE_VALUE;
    }

    /* Add final NULL to ReturnBuffer */
    if (ReturnBuffer.Length >= ReturnBuffer.MaximumLength)
    {
        PWSTR NewBuffer;
        ReturnBuffer.MaximumLength += sizeof(WCHAR);
        NewBuffer = ExAllocatePool(PagedPool, ReturnBuffer.MaximumLength);
        if (!NewBuffer)
        {
            DPRINT("ExAllocatePool() failed\n");
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }
        if (ReturnBuffer.Buffer)
        {
            RtlCopyMemory(NewBuffer, ReturnBuffer.Buffer, ReturnBuffer.Length);
            ExFreePool(ReturnBuffer.Buffer);
        }
        ReturnBuffer.Buffer = NewBuffer;
    }
    ReturnBuffer.Buffer[ReturnBuffer.Length / sizeof(WCHAR)] = UNICODE_NULL;
    *SymbolicLinkList = ReturnBuffer.Buffer;
    Status = STATUS_SUCCESS;

cleanup:
    if (!NT_SUCCESS(Status) && ReturnBuffer.Buffer)
        ExFreePool(ReturnBuffer.Buffer);
    if (InterfaceKey != INVALID_HANDLE_VALUE)
        ZwClose(InterfaceKey);
    if (DeviceKey != INVALID_HANDLE_VALUE)
        ZwClose(DeviceKey);
    if (ReferenceKey != INVALID_HANDLE_VALUE)
        ZwClose(ReferenceKey);
    if (ControlKey != INVALID_HANDLE_VALUE)
        ZwClose(ControlKey);
    if (DeviceBi)
        ExFreePool(DeviceBi);
    if (ReferenceBi)
        ExFreePool(ReferenceBi);
    if (bip)
        ExFreePool(bip);
    return Status;
}

/*++
 * @name IoRegisterDeviceInterface
 * @implemented
 *
 * Registers a device interface class, if it has not been previously registered,
 * and creates a new instance of the interface class, which a driver can
 * subsequently enable for use by applications or other system components.
 * Documented in WDK.
 *
 * @param PhysicalDeviceObject
 *        Points to an optional PDO that narrows the search to only the
 *        device interfaces of the device represented by the PDO
 *
 * @param InterfaceClassGuid
 *        Points to a class GUID specifying the device interface class
 *
 * @param ReferenceString
 *        Optional parameter, pointing to a unicode string. For a full
 *        description of this rather rarely used param (usually drivers
 *        pass NULL here) see WDK
 *
 * @param SymbolicLinkName
 *        Pointer to the resulting unicode string
 *
 * @return Usual NTSTATUS
 *
 * @remarks Must be called at IRQL = PASSIVE_LEVEL in the context of a
 *          system thread
 *
 *--*/
NTSTATUS
NTAPI
IoRegisterDeviceInterface(IN PDEVICE_OBJECT PhysicalDeviceObject,
                          IN CONST GUID *InterfaceClassGuid,
                          IN PUNICODE_STRING ReferenceString OPTIONAL,
                          OUT PUNICODE_STRING SymbolicLinkName)
{
    PUNICODE_STRING InstancePath;
    UNICODE_STRING GuidString;
    UNICODE_STRING SubKeyName;
    UNICODE_STRING InterfaceKeyName;
    UNICODE_STRING BaseKeyName;
    UCHAR PdoNameInfoBuffer[sizeof(OBJECT_NAME_INFORMATION) + (256 * sizeof(WCHAR))];
    POBJECT_NAME_INFORMATION PdoNameInfo = (POBJECT_NAME_INFORMATION)PdoNameInfoBuffer;
    UNICODE_STRING DeviceInstance = RTL_CONSTANT_STRING(L"DeviceInstance");
    UNICODE_STRING SymbolicLink = RTL_CONSTANT_STRING(L"SymbolicLink");
    HANDLE ClassKey;
    HANDLE InterfaceKey;
    HANDLE SubKey;
    ULONG StartIndex;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG i;
    NTSTATUS Status;
    PEXTENDED_DEVOBJ_EXTENSION DeviceObjectExtension;

    ASSERT_IRQL_EQUAL(PASSIVE_LEVEL);

    DPRINT("IoRegisterDeviceInterface(): PDO %p, RefString: %wZ\n",
        PhysicalDeviceObject, ReferenceString);

    /* Parameters must pass three border of checks */
    DeviceObjectExtension = (PEXTENDED_DEVOBJ_EXTENSION)PhysicalDeviceObject->DeviceObjectExtension;

    /* 1st level: Presence of a Device Node */
    if (DeviceObjectExtension->DeviceNode == NULL)
    {
        DPRINT("PhysicalDeviceObject 0x%p doesn't have a DeviceNode\n", PhysicalDeviceObject);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    /* 2nd level: Presence of an non-zero length InstancePath */
    if (DeviceObjectExtension->DeviceNode->InstancePath.Length == 0)
    {
        DPRINT("PhysicalDeviceObject 0x%p's DOE has zero-length InstancePath\n", PhysicalDeviceObject);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    /* 3rd level: Optional, based on WDK documentation */
    if (ReferenceString != NULL)
    {
        /* Reference string must not contain path-separator symbols */
        for (i = 0; i < ReferenceString->Length / sizeof(WCHAR); i++)
        {
            if ((ReferenceString->Buffer[i] == '\\') ||
                (ReferenceString->Buffer[i] == '/'))
                return STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    Status = RtlStringFromGUID(InterfaceClassGuid, &GuidString);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("RtlStringFromGUID() failed with status 0x%08lx\n", Status);
        return Status;
    }

    /* Create Pdo name: \Device\xxxxxxxx (unnamed device) */
    Status = ObQueryNameString(
        PhysicalDeviceObject,
        PdoNameInfo,
        sizeof(PdoNameInfoBuffer),
        &i);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("ObQueryNameString() failed with status 0x%08lx\n", Status);
        return Status;
    }
    ASSERT(PdoNameInfo->Name.Length);

    /* Create base key name for this interface: HKLM\SYSTEM\CurrentControlSet\Control\DeviceClasses\{GUID} */
    ASSERT(((PEXTENDED_DEVOBJ_EXTENSION)PhysicalDeviceObject->DeviceObjectExtension)->DeviceNode);
    InstancePath = &((PEXTENDED_DEVOBJ_EXTENSION)PhysicalDeviceObject->DeviceObjectExtension)->DeviceNode->InstancePath;
    BaseKeyName.Length = wcslen(BaseKeyString) * sizeof(WCHAR);
    BaseKeyName.MaximumLength = BaseKeyName.Length
        + GuidString.Length;
    BaseKeyName.Buffer = ExAllocatePool(
        PagedPool,
        BaseKeyName.MaximumLength);
    if (!BaseKeyName.Buffer)
    {
        DPRINT("ExAllocatePool() failed\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    wcscpy(BaseKeyName.Buffer, BaseKeyString);
    RtlAppendUnicodeStringToString(&BaseKeyName, &GuidString);

    /* Create BaseKeyName key in registry */
    InitializeObjectAttributes(
        &ObjectAttributes,
        &BaseKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_OPENIF,
        NULL, /* RootDirectory */
        NULL); /* SecurityDescriptor */

    Status = ZwCreateKey(
        &ClassKey,
        KEY_WRITE,
        &ObjectAttributes,
        0, /* TileIndex */
        NULL, /* Class */
        REG_OPTION_VOLATILE,
        NULL); /* Disposition */

    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwCreateKey() failed with status 0x%08lx\n", Status);
        ExFreePool(BaseKeyName.Buffer);
        return Status;
    }

    /* Create key name for this interface: ##?#ACPI#PNP0501#1#{GUID} */
    InterfaceKeyName.Length = 0;
    InterfaceKeyName.MaximumLength =
        4 * sizeof(WCHAR) + /* 4  = size of ##?# */
        InstancePath->Length +
        sizeof(WCHAR) +     /* 1  = size of # */
        GuidString.Length;
    InterfaceKeyName.Buffer = ExAllocatePool(
        PagedPool,
        InterfaceKeyName.MaximumLength);
    if (!InterfaceKeyName.Buffer)
    {
        DPRINT("ExAllocatePool() failed\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlAppendUnicodeToString(&InterfaceKeyName, L"##?#");
    StartIndex = InterfaceKeyName.Length / sizeof(WCHAR);
    RtlAppendUnicodeStringToString(&InterfaceKeyName, InstancePath);
    for (i = 0; i < InstancePath->Length / sizeof(WCHAR); i++)
    {
        if (InterfaceKeyName.Buffer[StartIndex + i] == '\\')
            InterfaceKeyName.Buffer[StartIndex + i] = '#';
    }
    RtlAppendUnicodeToString(&InterfaceKeyName, L"#");
    RtlAppendUnicodeStringToString(&InterfaceKeyName, &GuidString);

    /* Create the interface key in registry */
    InitializeObjectAttributes(
        &ObjectAttributes,
        &InterfaceKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_OPENIF,
        ClassKey,
        NULL); /* SecurityDescriptor */

    Status = ZwCreateKey(
        &InterfaceKey,
        KEY_WRITE,
        &ObjectAttributes,
        0, /* TileIndex */
        NULL, /* Class */
        REG_OPTION_VOLATILE,
        NULL); /* Disposition */

    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwCreateKey() failed with status 0x%08lx\n", Status);
        ZwClose(ClassKey);
        ExFreePool(BaseKeyName.Buffer);
        return Status;
    }

    /* Write DeviceInstance entry. Value is InstancePath */
    Status = ZwSetValueKey(
        InterfaceKey,
        &DeviceInstance,
        0, /* TileIndex */
        REG_SZ,
        InstancePath->Buffer,
        InstancePath->Length);
    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwSetValueKey() failed with status 0x%08lx\n", Status);
        ZwClose(InterfaceKey);
        ZwClose(ClassKey);
        ExFreePool(InterfaceKeyName.Buffer);
        ExFreePool(BaseKeyName.Buffer);
        return Status;
    }

    /* Create subkey. Name is #ReferenceString */
    SubKeyName.Length = 0;
    SubKeyName.MaximumLength = sizeof(WCHAR);
    if (ReferenceString && ReferenceString->Length)
        SubKeyName.MaximumLength += ReferenceString->Length;
    SubKeyName.Buffer = ExAllocatePool(
        PagedPool,
        SubKeyName.MaximumLength);
    if (!SubKeyName.Buffer)
    {
        DPRINT("ExAllocatePool() failed\n");
        ZwClose(InterfaceKey);
        ZwClose(ClassKey);
        ExFreePool(InterfaceKeyName.Buffer);
        ExFreePool(BaseKeyName.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlAppendUnicodeToString(&SubKeyName, L"#");
    if (ReferenceString && ReferenceString->Length)
        RtlAppendUnicodeStringToString(&SubKeyName, ReferenceString);

    /* Create SubKeyName key in registry */
    InitializeObjectAttributes(
        &ObjectAttributes,
        &SubKeyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        InterfaceKey, /* RootDirectory */
        NULL); /* SecurityDescriptor */

    Status = ZwCreateKey(
        &SubKey,
        KEY_WRITE,
        &ObjectAttributes,
        0, /* TileIndex */
        NULL, /* Class */
        REG_OPTION_VOLATILE,
        NULL); /* Disposition */

    if (!NT_SUCCESS(Status))
    {
        DPRINT("ZwCreateKey() failed with status 0x%08lx\n", Status);
        ZwClose(InterfaceKey);
        ZwClose(ClassKey);
        ExFreePool(InterfaceKeyName.Buffer);
        ExFreePool(BaseKeyName.Buffer);
        return Status;
    }

    /* Create symbolic link name: \??\ACPI#PNP0501#1#{GUID}\ReferenceString */
    SymbolicLinkName->Length = 0;
    SymbolicLinkName->MaximumLength = SymbolicLinkName->Length
        + 4 * sizeof(WCHAR) /* 4 = size of \??\ */
        + InstancePath->Length
        + sizeof(WCHAR)     /* 1  = size of # */
        + GuidString.Length
        + sizeof(WCHAR);    /* final NULL */
    if (ReferenceString && ReferenceString->Length)
        SymbolicLinkName->MaximumLength += sizeof(WCHAR) + ReferenceString->Length;
    SymbolicLinkName->Buffer = ExAllocatePool(
        PagedPool,
        SymbolicLinkName->MaximumLength);
    if (!SymbolicLinkName->Buffer)
    {
        DPRINT("ExAllocatePool() failed\n");
        ZwClose(SubKey);
        ZwClose(InterfaceKey);
        ZwClose(ClassKey);
        ExFreePool(InterfaceKeyName.Buffer);
        ExFreePool(SubKeyName.Buffer);
        ExFreePool(BaseKeyName.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlAppendUnicodeToString(SymbolicLinkName, L"\\??\\");
    StartIndex = SymbolicLinkName->Length / sizeof(WCHAR);
    RtlAppendUnicodeStringToString(SymbolicLinkName, InstancePath);
    for (i = 0; i < InstancePath->Length / sizeof(WCHAR); i++)
    {
        if (SymbolicLinkName->Buffer[StartIndex + i] == '\\')
            SymbolicLinkName->Buffer[StartIndex + i] = '#';
    }
    RtlAppendUnicodeToString(SymbolicLinkName, L"#");
    RtlAppendUnicodeStringToString(SymbolicLinkName, &GuidString);

    if (ReferenceString && ReferenceString->Length)
    {
        RtlAppendUnicodeToString(SymbolicLinkName, L"\\");
        RtlAppendUnicodeStringToString(SymbolicLinkName, ReferenceString);
    }
    SymbolicLinkName->Buffer[SymbolicLinkName->Length/sizeof(WCHAR)] = L'\0';

    /* Write symbolic link name in registry */
    SymbolicLinkName->Buffer[1] = '\\';
    Status = ZwSetValueKey(SubKey,
                           &SymbolicLink,
                           0, /* TileIndex */
                           REG_SZ,
                           SymbolicLinkName->Buffer,
                           SymbolicLinkName->Length);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("ZwSetValueKey() failed with status 0x%08lx\n", Status);
        ZwClose(SubKey);
        ZwClose(InterfaceKey);
        ZwClose(ClassKey);
        ExFreePool(SubKeyName.Buffer);
        ExFreePool(InterfaceKeyName.Buffer);
        ExFreePool(BaseKeyName.Buffer);
        ExFreePool(SymbolicLinkName->Buffer);
        return Status;
    }
    else
    {
        SymbolicLinkName->Buffer[1] = '?';
    }

    /* Create symbolic link */
    DPRINT("IoRegisterDeviceInterface(): creating symbolic link %wZ -> %wZ\n", SymbolicLinkName, &PdoNameInfo->Name);
    Status = IoCreateSymbolicLink(SymbolicLinkName, &PdoNameInfo->Name);

    /* If the symbolic link already exists, return an informational success status */
    if (Status == STATUS_OBJECT_NAME_COLLISION)
        Status = STATUS_OBJECT_NAME_EXISTS;

    /* Check if it really failed */
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("IoCreateSymbolicLink() failed with status 0x%08lx\n", Status);
        ExFreePool(SymbolicLinkName->Buffer);
    }

    ZwClose(SubKey);
    ZwClose(InterfaceKey);
    ZwClose(ClassKey);
    ExFreePool(SubKeyName.Buffer);
    ExFreePool(InterfaceKeyName.Buffer);
    ExFreePool(BaseKeyName.Buffer);

    return Status;
}

/*++
 * @name IoSetDeviceInterfaceState
 * @implemented
 *
 * Enables or disables an instance of a previously registered device
 * interface class.
 * Documented in WDK.
 *
 * @param SymbolicLinkName
 *        Pointer to the string identifying instance to enable or disable
 *
 * @param Enable
 *        TRUE = enable, FALSE = disable
 *
 * @return Usual NTSTATUS
 *
 * @remarks Must be called at IRQL = PASSIVE_LEVEL in the context of a
 *          system thread
 *
 *--*/
NTSTATUS
NTAPI
IoSetDeviceInterfaceState(IN PUNICODE_STRING SymbolicLinkName,
                          IN BOOLEAN Enable)
{
    PDEVICE_OBJECT PhysicalDeviceObject;
    PFILE_OBJECT FileObject;
    UNICODE_STRING GuidString;
    UNICODE_STRING SymLink;
    PWCHAR StartPosition;
    PWCHAR EndPosition;
    NTSTATUS Status;
    LPCGUID EventGuid;
    HANDLE InstanceHandle, ControlHandle;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG LinkedValue;
    GUID DeviceGuid;

    if (SymbolicLinkName == NULL)
        return STATUS_INVALID_PARAMETER_1;

    DPRINT("IoSetDeviceInterfaceState('%wZ', %d)\n", SymbolicLinkName, Enable);

    /* Symbolic link name is \??\ACPI#PNP0501#1#{GUID}\ReferenceString */
    /* Get GUID from SymbolicLinkName */
    StartPosition = wcschr(SymbolicLinkName->Buffer, L'{');
    EndPosition = wcschr(SymbolicLinkName->Buffer, L'}');
    if (!StartPosition ||!EndPosition || StartPosition > EndPosition)
    {
        DPRINT1("IoSetDeviceInterfaceState() returning STATUS_INVALID_PARAMETER_1\n");
        return STATUS_INVALID_PARAMETER_1;
    }
    GuidString.Buffer = StartPosition;
    GuidString.MaximumLength = GuidString.Length = (USHORT)((ULONG_PTR)(EndPosition + 1) - (ULONG_PTR)StartPosition);

    SymLink.Buffer = SymbolicLinkName->Buffer;
    SymLink.MaximumLength = SymLink.Length = (USHORT)((ULONG_PTR)(EndPosition + 1) - (ULONG_PTR)SymLink.Buffer);
    DPRINT("IoSetDeviceInterfaceState('%wZ', %d)\n", SymbolicLinkName, Enable);

    Status = OpenRegistryHandlesFromSymbolicLink(SymbolicLinkName,
                                                 KEY_CREATE_SUB_KEY,
                                                 NULL,
                                                 NULL,
                                                 &InstanceHandle);
    if (!NT_SUCCESS(Status))
        return Status;

    RtlInitUnicodeString(&KeyName, L"Control");
    InitializeObjectAttributes(&ObjectAttributes,
                               &KeyName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               InstanceHandle,
                               NULL);
    Status = ZwCreateKey(&ControlHandle,
                         KEY_SET_VALUE,
                         &ObjectAttributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         NULL);
    ZwClose(InstanceHandle);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to create the Control subkey\n");
        return Status;
    }

    LinkedValue = (Enable ? 1 : 0);

    RtlInitUnicodeString(&KeyName, L"Linked");
    Status = ZwSetValueKey(ControlHandle,
                           &KeyName,
                           0,
                           REG_DWORD,
                           &LinkedValue,
                           sizeof(ULONG));
    ZwClose(ControlHandle);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to write the Linked value\n");
        return Status;
    }

    /* Get pointer to the PDO */
    Status = IoGetDeviceObjectPointer(
        &SymLink,
        0, /* DesiredAccess */
        &FileObject,
        &PhysicalDeviceObject);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("IoGetDeviceObjectPointer() failed with status 0x%08lx\n", Status);
        return Status;
    }
    
    Status = RtlGUIDFromString(&GuidString, &DeviceGuid);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("RtlGUIDFromString() failed with status 0x%08lx\n", Status);
        return Status;
    }

    EventGuid = Enable ? &GUID_DEVICE_INTERFACE_ARRIVAL : &GUID_DEVICE_INTERFACE_REMOVAL;
    IopNotifyPlugPlayNotification(
        PhysicalDeviceObject,
        EventCategoryDeviceInterfaceChange,
        EventGuid,
        &DeviceGuid,
        (PVOID)SymbolicLinkName);

    ObDereferenceObject(FileObject);
    DPRINT("Status %x\n", Status);
    return STATUS_SUCCESS;
}

/* EOF */
