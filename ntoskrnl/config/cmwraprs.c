/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/config/cmwraprs.c
 * PURPOSE:         Configuration Manager - Wrappers for Hive Operations
 * PROGRAMMERS:     Alex Ionescu (alex.ionescu@odyssey.org)
 */

/* INCLUDES ******************************************************************/

#include "ntoskrnl.h"
#define NDEBUG
#include "debug.h"

/* FUNCTIONS *****************************************************************/

NTSTATUS
NTAPI
CmpCreateEvent(IN EVENT_TYPE EventType,
               OUT PHANDLE EventHandle,
               OUT PKEVENT *Event)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;

    /* Create the event */
    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    Status = ZwCreateEvent(EventHandle,
                           EVENT_ALL_ACCESS,
                           &ObjectAttributes,
                           EventType,
                           FALSE);
    if (!NT_SUCCESS(Status)) return Status;

    /* Get a pointer to the object itself */
    Status = ObReferenceObjectByHandle(*EventHandle,
                                       EVENT_ALL_ACCESS,
                                       NULL,
                                       KernelMode,
                                       (PVOID*)Event,
                                       NULL);
    if (!NT_SUCCESS(Status)) ZwClose(*EventHandle);

    /* Return status */
    return Status;
}

PVOID
NTAPI
CmpAllocate(IN SIZE_T Size,
            IN BOOLEAN Paged,
            IN ULONG Tag)
{
    return ExAllocatePoolWithTag(Paged ? PagedPool : NonPagedPool,
                                 Size,
                                 Tag);
}

VOID
NTAPI
CmpFree(IN PVOID Ptr,
        IN ULONG Quota)
{
    ExFreePool(Ptr);
}

BOOLEAN
NTAPI
CmpFileRead(IN PHHIVE RegistryHive,
            IN ULONG FileType,
            IN PULONG FileOffset,
            OUT PVOID Buffer,
            IN SIZE_T BufferLength)
{
    PCMHIVE CmHive = (PCMHIVE)RegistryHive;
    HANDLE HiveHandle = CmHive->FileHandles[FileType];
    LARGE_INTEGER _FileOffset;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    _FileOffset.QuadPart = *FileOffset;
    Status = ZwReadFile(HiveHandle, 0, 0, 0, &IoStatusBlock,
                       Buffer, BufferLength, &_FileOffset, 0);
    return NT_SUCCESS(Status) ? TRUE : FALSE;
}

BOOLEAN
NTAPI
CmpFileWrite(IN PHHIVE RegistryHive,
             IN ULONG FileType,
             IN PULONG FileOffset,
             IN PVOID Buffer,
             IN SIZE_T BufferLength)
{
    PCMHIVE CmHive = (PCMHIVE)RegistryHive;
    HANDLE HiveHandle = CmHive->FileHandles[FileType];
    LARGE_INTEGER _FileOffset;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    _FileOffset.QuadPart = *FileOffset;
    Status = ZwWriteFile(HiveHandle, 0, 0, 0, &IoStatusBlock,
                       Buffer, BufferLength, &_FileOffset, 0);
    return NT_SUCCESS(Status) ? TRUE : FALSE;
}

BOOLEAN
NTAPI
CmpFileSetSize(IN PHHIVE RegistryHive,
               IN ULONG FileType,
               IN ULONG FileSize,
               IN ULONG OldFileSize)
{
    PCMHIVE CmHive = (PCMHIVE)RegistryHive;
    HANDLE HiveHandle = CmHive->FileHandles[FileType];
    FILE_END_OF_FILE_INFORMATION EndOfFileInfo;
    FILE_ALLOCATION_INFORMATION FileAllocationInfo;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    EndOfFileInfo.EndOfFile.QuadPart = FileSize;
    Status = ZwSetInformationFile(HiveHandle,
                                  &IoStatusBlock,
                                  &EndOfFileInfo,
                                  sizeof(FILE_END_OF_FILE_INFORMATION),
                                  FileEndOfFileInformation);
    if (!NT_SUCCESS(Status)) return FALSE;

    FileAllocationInfo.AllocationSize.QuadPart = FileSize;
    Status = ZwSetInformationFile(HiveHandle,
                                  &IoStatusBlock,
                                  &FileAllocationInfo,
                                  sizeof(FILE_ALLOCATION_INFORMATION),
                                  FileAllocationInformation);
    if (!NT_SUCCESS(Status)) return FALSE;

    return TRUE;
}

BOOLEAN
NTAPI
CmpFileFlush(IN PHHIVE RegistryHive,
             IN ULONG FileType,
             IN OUT PLARGE_INTEGER FileOffset,
             IN ULONG Length)
{
    PCMHIVE CmHive = (PCMHIVE)RegistryHive;
    HANDLE HiveHandle = CmHive->FileHandles[FileType];
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    Status = ZwFlushBuffersFile(HiveHandle, &IoStatusBlock);
    return NT_SUCCESS(Status) ? TRUE : FALSE;
}
