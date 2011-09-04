/*
 * Copyright (C) 2002-2005 ReactOS Team; (C) 2011 NasuTek Enterprises (and the authors from the programmers section)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/mm/anonmem.c
 * PURPOSE:         Implementing anonymous memory.
 *
 * PROGRAMMERS:     David Welch
 *                  Casper Hornstrup
 *                  KJK::Hyperion
 *                  Ge van Geldorp
 *                  Eric Kohl
 *                  Royce Mitchell III
 *                  Aleksey Bragin
 *                  Jason Filby
 *                  Art Yerkes
 *                  Gunnar Andre' Dalsnes
 *                  Filip Navara
 *                  Thomas Weidenmueller
 *                  Alex Ionescu
 *                  Trevor McCort
 *                  Steven Edwards
 */

/* INCLUDE *****************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

#define MODULE_INVOLVED_IN_ARM3
#include "ARM3/miarm.h"

/* FUNCTIONS *****************************************************************/

NTSTATUS
NTAPI
MmPageOutVirtualMemory(PMMSUPPORT AddressSpace,
                       PMEMORY_AREA MemoryArea,
                       PVOID Address,
                       PMM_PAGEOP PageOp)
{
   PFN_NUMBER Page;
   BOOLEAN WasDirty;
   SWAPENTRY SwapEntry;
   NTSTATUS Status;
   PEPROCESS Process = MmGetAddressSpaceOwner(AddressSpace);
    
   DPRINT("MmPageOutVirtualMemory(Address 0x%.8X) PID %d\n",
          Address, Process->UniqueProcessId);

   /*
    * Check for paging out from a deleted virtual memory area.
    */
   if (MemoryArea->DeleteInProgress)
   {
      PageOp->Status = STATUS_UNSUCCESSFUL;
      KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
      MmReleasePageOp(PageOp);
      return(STATUS_UNSUCCESSFUL);
   }

   /*
    * Disable the virtual mapping.
    */
   MmDisableVirtualMapping(Process, Address,
                           &WasDirty, &Page);

   if (Page == 0)
   {
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   /*
    * Paging out non-dirty data is easy.
    */
   if (!WasDirty)
   {
      MmLockAddressSpace(AddressSpace);
      MmDeleteVirtualMapping(Process, Address, FALSE, NULL, NULL);
      MmDeleteAllRmaps(Page, NULL, NULL);
      if ((SwapEntry = MmGetSavedSwapEntryPage(Page)) != 0)
      {
         MmCreatePageFileMapping(Process, Address, SwapEntry);
         MmSetSavedSwapEntryPage(Page, 0);
      }
      MmUnlockAddressSpace(AddressSpace);
      MmReleasePageMemoryConsumer(MC_USER, Page);
      PageOp->Status = STATUS_SUCCESS;
      KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
      MmReleasePageOp(PageOp);
      return(STATUS_SUCCESS);
   }

   /*
    * If necessary, allocate an entry in the paging file for this page
    */
   SwapEntry = MmGetSavedSwapEntryPage(Page);
   if (SwapEntry == 0)
   {
      SwapEntry = MmAllocSwapPage();
      if (SwapEntry == 0)
      {
         MmShowOutOfSpaceMessagePagingFile();
         MmEnableVirtualMapping(Process, Address);
         PageOp->Status = STATUS_UNSUCCESSFUL;
         KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
         MmReleasePageOp(PageOp);
         return(STATUS_PAGEFILE_QUOTA);
      }
   }

   /*
    * Write the page to the pagefile
    */
   Status = MmWriteToSwapPage(SwapEntry, Page);
   if (!NT_SUCCESS(Status))
   {
      DPRINT1("MM: Failed to write to swap page (Status was 0x%.8X)\n",
              Status);
      MmEnableVirtualMapping(Process, Address);
      PageOp->Status = STATUS_UNSUCCESSFUL;
      KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
      MmReleasePageOp(PageOp);
      return(STATUS_UNSUCCESSFUL);
   }

   /*
    * Otherwise we have succeeded, free the page
    */
   DPRINT("MM: Swapped out virtual memory page 0x%.8X!\n", Page << PAGE_SHIFT);
   MmLockAddressSpace(AddressSpace);
   MmDeleteVirtualMapping(Process, Address, FALSE, NULL, NULL);
   MmCreatePageFileMapping(Process, Address, SwapEntry);
   MmUnlockAddressSpace(AddressSpace);
   MmDeleteAllRmaps(Page, NULL, NULL);
   MmSetSavedSwapEntryPage(Page, 0);
   MmReleasePageMemoryConsumer(MC_USER, Page);
   PageOp->Status = STATUS_SUCCESS;
   KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
   MmReleasePageOp(PageOp);
   return(STATUS_SUCCESS);
}

NTSTATUS
NTAPI
MmNotPresentFaultVirtualMemory(PMMSUPPORT AddressSpace,
                               MEMORY_AREA* MemoryArea,
                               PVOID Address,
                               BOOLEAN Locked)
/*
 * FUNCTION: Move data into memory to satisfy a page not present fault
 * ARGUMENTS:
 *      AddressSpace = Address space within which the fault occurred
 *      MemoryArea = The memory area within which the fault occurred
 *      Address = The absolute address of fault
 * RETURNS: Status
 * NOTES: This function is called with the address space lock held.
 */
{
   PFN_NUMBER Page;
   NTSTATUS Status;
   PMM_REGION Region;
   PMM_PAGEOP PageOp;
   PEPROCESS Process = MmGetAddressSpaceOwner(AddressSpace);
    
   /*
    * There is a window between taking the page fault and locking the
    * address space when another thread could load the page so we check
    * that.
    */
   if (MmIsPagePresent(NULL, Address))
   {
      return(STATUS_SUCCESS);
   }

   /*
    * Check for the virtual memory area being deleted.
    */
   if (MemoryArea->DeleteInProgress)
   {
      return(STATUS_UNSUCCESSFUL);
   }

   /*
    * Get the segment corresponding to the virtual address
    */
   Region = MmFindRegion(MemoryArea->StartingAddress,
                         &MemoryArea->Data.VirtualMemoryData.RegionListHead,
                         Address, NULL);

   if (Region->Type == MEM_RESERVE || Region->Protect == PAGE_NOACCESS)
   {
      return(STATUS_ACCESS_VIOLATION);
   }

   /*
    * FIXME
    */
    if (Region->Protect & PAGE_GUARD)
    {
        return(STATUS_GUARD_PAGE_VIOLATION);
    }

   /*
    * Get or create a page operation
    */
   PageOp = MmGetPageOp(MemoryArea, Process->UniqueProcessId,
                        (PVOID)PAGE_ROUND_DOWN(Address), NULL, 0,
                        MM_PAGEOP_PAGEIN, FALSE);
   if (PageOp == NULL)
   {
      DPRINT1("MmGetPageOp failed");
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   /*
    * Check if someone else is already handling this fault, if so wait
    * for them
    */
   if (PageOp->Thread != PsGetCurrentThread())
   {
      MmUnlockAddressSpace(AddressSpace);
      Status = KeWaitForSingleObject(&PageOp->CompletionEvent,
                                     0,
                                     KernelMode,
                                     FALSE,
                                     NULL);
      /*
      * Check for various strange conditions
      */
      if (Status != STATUS_SUCCESS)
      {
         DPRINT1("Failed to wait for page op\n");
         KeBugCheck(MEMORY_MANAGEMENT);
      }
      if (PageOp->Status == STATUS_PENDING)
      {
         DPRINT1("Woke for page op before completion\n");
         KeBugCheck(MEMORY_MANAGEMENT);
      }
      /*
      * If this wasn't a pagein then we need to restart the handling
      */
      if (PageOp->OpType != MM_PAGEOP_PAGEIN)
      {
         MmLockAddressSpace(AddressSpace);
         KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
         MmReleasePageOp(PageOp);
         return(STATUS_MM_RESTART_OPERATION);
      }
      /*
      * If the thread handling this fault has failed then we don't retry
      */
      if (!NT_SUCCESS(PageOp->Status))
      {
         MmLockAddressSpace(AddressSpace);
         KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
         Status = PageOp->Status;
         MmReleasePageOp(PageOp);
         return(Status);
      }
      MmLockAddressSpace(AddressSpace);
      KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
      MmReleasePageOp(PageOp);
      return(STATUS_SUCCESS);
   }

   /*
    * Try to allocate a page
    */
   MI_SET_USAGE(MI_USAGE_VAD);
   MI_SET_PROCESS2(Process->ImageFileName);
   Status = MmRequestPageMemoryConsumer(MC_USER, FALSE, &Page);
   if (Status == STATUS_NO_MEMORY)
   {
      MmUnlockAddressSpace(AddressSpace);
      Status = MmRequestPageMemoryConsumer(MC_USER, TRUE, &Page);
      MmLockAddressSpace(AddressSpace);
   }
   if (!NT_SUCCESS(Status))
   {
      DPRINT1("MmRequestPageMemoryConsumer failed, status = %x\n", Status);
      KeBugCheck(MEMORY_MANAGEMENT);
   }

   /*
    * Handle swapped out pages.
    */
   if (MmIsPageSwapEntry(NULL, Address))
   {
      SWAPENTRY SwapEntry;

      MmDeletePageFileMapping(Process, Address, &SwapEntry);
      Status = MmReadFromSwapPage(SwapEntry, Page);
      if (!NT_SUCCESS(Status))
      {
         KeBugCheck(MEMORY_MANAGEMENT);
      }
      MmSetSavedSwapEntryPage(Page, SwapEntry);
   }

   /*
    * Set the page. If we fail because we are out of memory then
    * try again
    */
   Status = MmCreateVirtualMapping(Process,
                                   (PVOID)PAGE_ROUND_DOWN(Address),
                                   Region->Protect,
                                   &Page,
                                   1);
   while (Status == STATUS_NO_MEMORY)
   {
      MmUnlockAddressSpace(AddressSpace);
      Status = MmCreateVirtualMapping(Process,
                                      Address,
                                      Region->Protect,
                                      &Page,
                                      1);
      MmLockAddressSpace(AddressSpace);
   }
   if (!NT_SUCCESS(Status))
   {
      DPRINT1("MmCreateVirtualMapping failed, not out of memory\n");
      KeBugCheck(MEMORY_MANAGEMENT);
      return(Status);
   }

   /*
    * Add the page to the process's working set
    */
   MmInsertRmap(Page, Process, (PVOID)PAGE_ROUND_DOWN(Address));

   /*
    * Finish the operation
    */
   PageOp->Status = STATUS_SUCCESS;
   KeSetEvent(&PageOp->CompletionEvent, IO_NO_INCREMENT, FALSE);
   MmReleasePageOp(PageOp);
   return(STATUS_SUCCESS);
}

static VOID
MmModifyAttributes(PMMSUPPORT AddressSpace,
                   PVOID BaseAddress,
                   ULONG RegionSize,
                   ULONG OldType,
                   ULONG OldProtect,
                   ULONG NewType,
                   ULONG NewProtect)
/*
 * FUNCTION: Modify the attributes of a memory region
 */
{
   PEPROCESS Process = MmGetAddressSpaceOwner(AddressSpace);
    
   /*
    * If we are switching a previously committed region to reserved then
    * free any allocated pages within the region
    */
   if (NewType == MEM_RESERVE && OldType == MEM_COMMIT)
   {
      ULONG i;

      for (i=0; i < PAGE_ROUND_UP(RegionSize)/PAGE_SIZE; i++)
      {
         PFN_NUMBER Page;

         if (MmIsPageSwapEntry(Process,
                               (char*)BaseAddress + (i * PAGE_SIZE)))
         {
            SWAPENTRY SwapEntry;

            MmDeletePageFileMapping(Process,
                                    (char*)BaseAddress + (i * PAGE_SIZE),
                                    &SwapEntry);
            MmFreeSwapPage(SwapEntry);
         }
         else
         {
            MmDeleteVirtualMapping(Process,
                                   (char*)BaseAddress + (i*PAGE_SIZE),
                                   FALSE, NULL, &Page);
            if (Page != 0)
            {
               SWAPENTRY SavedSwapEntry;
               SavedSwapEntry = MmGetSavedSwapEntryPage(Page);
               if (SavedSwapEntry != 0)
               {
                  MmFreeSwapPage(SavedSwapEntry);
                  MmSetSavedSwapEntryPage(Page, 0);
               }
               MmDeleteRmap(Page, Process,
                            (char*)BaseAddress + (i * PAGE_SIZE));
               MmReleasePageMemoryConsumer(MC_USER, Page);
            }
         }
      }
   }

   /*
    * If we are changing the protection attributes of a committed region then
    * alter the attributes for any allocated pages within the region
    */
   if (NewType == MEM_COMMIT && OldType == MEM_COMMIT &&
       OldProtect != NewProtect)
   {
      ULONG i;

      for (i=0; i < PAGE_ROUND_UP(RegionSize)/PAGE_SIZE; i++)
      {
         if (MmIsPagePresent(Process,
                             (char*)BaseAddress + (i*PAGE_SIZE)))
         {
            MmSetPageProtect(Process,
                             (char*)BaseAddress + (i*PAGE_SIZE),
                             NewProtect);
         }
      }
   }
}

NTSTATUS NTAPI
MiProtectVirtualMemory(IN PEPROCESS Process,
                       IN OUT PVOID *BaseAddress,
                       IN OUT PSIZE_T NumberOfBytesToProtect,
                       IN ULONG NewAccessProtection,
                       OUT PULONG OldAccessProtection  OPTIONAL)
{
    PMEMORY_AREA MemoryArea;
    PMMSUPPORT AddressSpace;
    ULONG OldAccessProtection_;
    NTSTATUS Status;

    *NumberOfBytesToProtect =
    PAGE_ROUND_UP((ULONG_PTR)(*BaseAddress) + (*NumberOfBytesToProtect)) -
    PAGE_ROUND_DOWN(*BaseAddress);
    *BaseAddress = (PVOID)PAGE_ROUND_DOWN(*BaseAddress);

    AddressSpace = &Process->Vm;

    MmLockAddressSpace(AddressSpace);
    MemoryArea = MmLocateMemoryAreaByAddress(AddressSpace, *BaseAddress);
    if (MemoryArea == NULL)
    {
        MmUnlockAddressSpace(AddressSpace);
        return STATUS_UNSUCCESSFUL;
    }

    if (OldAccessProtection == NULL)
        OldAccessProtection = &OldAccessProtection_;

    if (MemoryArea->Type == MEMORY_AREA_VIRTUAL_MEMORY)
    {
        Status = MmProtectAnonMem(AddressSpace, MemoryArea, *BaseAddress,
                                  *NumberOfBytesToProtect, NewAccessProtection,
                                  OldAccessProtection);
    }
    else if (MemoryArea->Type == MEMORY_AREA_SECTION_VIEW)
    {
        Status = MmProtectSectionView(AddressSpace, MemoryArea, *BaseAddress,
                                      *NumberOfBytesToProtect,
                                      NewAccessProtection,
                                      OldAccessProtection);
    }
    else
    {
        /* FIXME: Should we return failure or success in this case? */
        Status = STATUS_CONFLICTING_ADDRESSES;
    }

    MmUnlockAddressSpace(AddressSpace);

    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
NtAllocateVirtualMemory(IN HANDLE ProcessHandle,
                        IN OUT PVOID* UBaseAddress,
                        IN ULONG_PTR ZeroBits,
                        IN OUT PSIZE_T URegionSize,
                        IN ULONG AllocationType,
                        IN ULONG Protect)
{
   PEPROCESS Process;
   MEMORY_AREA* MemoryArea;
   ULONG_PTR MemoryAreaLength;
   ULONG Type;
   NTSTATUS Status;
   PMMSUPPORT AddressSpace;
   PVOID BaseAddress;
   ULONG RegionSize;
   PVOID PBaseAddress;
   ULONG PRegionSize;
   PHYSICAL_ADDRESS BoundaryAddressMultiple;
    PEPROCESS CurrentProcess = PsGetCurrentProcess();
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    KAPC_STATE ApcState;
    ULONG ProtectionMask;
    BOOLEAN Attached = FALSE;
    BoundaryAddressMultiple.QuadPart = 0;
    PAGED_CODE();

    /* Check for valid Zero bits */
    if (ZeroBits > 21)
    {
        DPRINT1("Too many zero bits\n");
        return STATUS_INVALID_PARAMETER_3;
    }

    /* Check for valid Allocation Types */
    if ((AllocationType & ~(MEM_COMMIT | MEM_RESERVE | MEM_RESET | MEM_PHYSICAL |
                            MEM_TOP_DOWN | MEM_WRITE_WATCH)))
    {
        DPRINT1("Invalid Allocation Type\n");
        return STATUS_INVALID_PARAMETER_5;
    }

    /* Check for at least one of these Allocation Types to be set */
    if (!(AllocationType & (MEM_COMMIT | MEM_RESERVE | MEM_RESET)))
    {
        DPRINT1("No memory allocation base type\n");
        return STATUS_INVALID_PARAMETER_5;
    }

    /* MEM_RESET is an exclusive flag, make sure that is valid too */
    if ((AllocationType & MEM_RESET) && (AllocationType != MEM_RESET))
    {
        DPRINT1("Invalid use of MEM_RESET\n");
        return STATUS_INVALID_PARAMETER_5;
    }

    /* Check if large pages are being used */
    if (AllocationType & MEM_LARGE_PAGES)
    {
        /* Large page allocations MUST be committed */
        if (!(AllocationType & MEM_COMMIT))
        {
            DPRINT1("Must supply MEM_COMMIT with MEM_LARGE_PAGES\n");
            return STATUS_INVALID_PARAMETER_5;
        }
        
        /* These flags are not allowed with large page allocations */
        if (AllocationType & (MEM_PHYSICAL | MEM_RESET | MEM_WRITE_WATCH))
        {
            DPRINT1("Using illegal flags with MEM_LARGE_PAGES\n");
            return STATUS_INVALID_PARAMETER_5;
        }
    }

    /* MEM_WRITE_WATCH can only be used if MEM_RESERVE is also used */
    if ((AllocationType & MEM_WRITE_WATCH) && !(AllocationType & MEM_RESERVE))
    {
        DPRINT1("MEM_WRITE_WATCH used without MEM_RESERVE\n");
        return STATUS_INVALID_PARAMETER_5;
    }

    /* MEM_PHYSICAL can only be used if MEM_RESERVE is also used */
    if ((AllocationType & MEM_PHYSICAL) && !(AllocationType & MEM_RESERVE))
    {
        DPRINT1("MEM_WRITE_WATCH used without MEM_RESERVE\n");
        return STATUS_INVALID_PARAMETER_5;
    }

    /* Check for valid MEM_PHYSICAL usage */
    if (AllocationType & MEM_PHYSICAL)
    {
        /* Only these flags are allowed with MEM_PHYSIAL */
        if (AllocationType & ~(MEM_RESERVE | MEM_TOP_DOWN | MEM_PHYSICAL))
        {
            DPRINT1("Using illegal flags with MEM_PHYSICAL\n");
            return STATUS_INVALID_PARAMETER_5;
        }

        /* Then make sure PAGE_READWRITE is used */
        if (Protect != PAGE_READWRITE)
        {
            DPRINT1("MEM_PHYSICAL used without PAGE_READWRITE\n");
            return STATUS_INVALID_PARAMETER_6;
        }
    }

    /* Calculate the protection mask and make sure it's valid */
    ProtectionMask = MiMakeProtectionMask(Protect);
    if (ProtectionMask == MM_INVALID_PROTECTION)
    {
        DPRINT1("Invalid protection mask\n");
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    /* Enter SEH */
    _SEH2_TRY
    {
        /* Check for user-mode parameters */
        if (PreviousMode != KernelMode)
        {
            /* Make sure they are writable */
            ProbeForWritePointer(UBaseAddress);
            ProbeForWriteUlong(URegionSize);
        }
        
        /* Capture their values */
        PBaseAddress = *UBaseAddress;
        PRegionSize = *URegionSize;
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Return the exception code */
        _SEH2_YIELD(return _SEH2_GetExceptionCode());
    }
    _SEH2_END;
    
    /* Make sure the allocation isn't past the VAD area */
    if (PBaseAddress >= MM_HIGHEST_VAD_ADDRESS)
    {
        DPRINT1("Virtual allocation base above User Space\n");
        return STATUS_INVALID_PARAMETER_2;
    }
    
    /* Make sure the allocation wouldn't overflow past the VAD area */
    if ((((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) - (ULONG_PTR)PBaseAddress) < PRegionSize)
    {
        DPRINT1("Region size would overflow into kernel-memory\n");
        return STATUS_INVALID_PARAMETER_4;
    }
    
    /* Make sure there's a size specified */
    if (!PRegionSize)
    {
        DPRINT1("Region size is invalid (zero)\n");
        return STATUS_INVALID_PARAMETER_4;
    }

    /* Check if this is for the current process */
    if (ProcessHandle == NtCurrentProcess())
    {
        /* We already have the current process, no need to go through Ob */
        Process = CurrentProcess;
    }
    else
    {
        /* Reference the handle for correct permissions */
        Status = ObReferenceObjectByHandle(ProcessHandle,
                                           PROCESS_VM_OPERATION,
                                           PsProcessType,
                                           PreviousMode,
                                           (PVOID*)&Process,
                                           NULL);
        if (!NT_SUCCESS(Status)) return Status;
        
        /* Check if not running in the current process */
        if (CurrentProcess != Process)
        {
            /* Attach to it */
            KeStackAttachProcess(&Process->Pcb, &ApcState);
            Attached = TRUE;
        }
    }
    
    /* Check for large page allocations */
    if (AllocationType & MEM_LARGE_PAGES)
    {
        /* The lock memory privilege is required */
        if (!SeSinglePrivilegeCheck(SeLockMemoryPrivilege, PreviousMode))
        {
            /* Fail without it */
            DPRINT1("Privilege not held for MEM_LARGE_PAGES\n");
            if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

   BaseAddress = (PVOID)PAGE_ROUND_DOWN(PBaseAddress);
   RegionSize = PAGE_ROUND_UP((ULONG_PTR)PBaseAddress + PRegionSize) -
                PAGE_ROUND_DOWN(PBaseAddress);


   /*
    * Copy on Write is reserved for system use. This case is a certain failure
    * but there may be other cases...needs more testing
    */
   if ((!BaseAddress || (AllocationType & MEM_RESERVE)) &&
       (Protect & (PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY)))
   {
      DPRINT1("Copy on write is not supported by VirtualAlloc\n");
      return STATUS_INVALID_PAGE_PROTECTION;
   }

   Type = (AllocationType & MEM_COMMIT) ? MEM_COMMIT : MEM_RESERVE;
   DPRINT("Type %x\n", Type);

   AddressSpace = &Process->Vm;
   MmLockAddressSpace(AddressSpace);

   if (PBaseAddress != 0)
   {
      MemoryArea = MmLocateMemoryAreaByAddress(AddressSpace, BaseAddress);

      if (MemoryArea != NULL)
      {
         MemoryAreaLength = (ULONG_PTR)MemoryArea->EndingAddress -
                            (ULONG_PTR)MemoryArea->StartingAddress;

         if (((ULONG_PTR)BaseAddress + RegionSize) > (ULONG_PTR)MemoryArea->EndingAddress)
         {
            DPRINT("BaseAddress + RegionSize %x is larger than MemoryArea's EndingAddress %x\n",
                  (ULONG_PTR)BaseAddress + RegionSize, MemoryArea->EndingAddress);

            MmUnlockAddressSpace(AddressSpace);
            if (Attached) KeUnstackDetachProcess(&ApcState);
            if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);
            return STATUS_MEMORY_NOT_ALLOCATED;
         }

         if (AllocationType == MEM_RESET)
         {
            if (MmIsPagePresent(Process, BaseAddress))
            {
               /* FIXME: mark pages as not modified */
            }
            else
            {
               /* FIXME: if pages are in paging file discard them and bring in pages of zeros */
            }

            MmUnlockAddressSpace(AddressSpace);
            if (Attached) KeUnstackDetachProcess(&ApcState);
            if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);

            /* MEM_RESET does not modify any attributes of region */
            return STATUS_SUCCESS;
         }

         if (MemoryArea->Type == MEMORY_AREA_VIRTUAL_MEMORY &&
             MemoryAreaLength >= RegionSize)
         {
            Status =
               MmAlterRegion(AddressSpace,
                             MemoryArea->StartingAddress,
                             &MemoryArea->Data.VirtualMemoryData.RegionListHead,
                             BaseAddress, RegionSize,
                             Type, Protect, MmModifyAttributes);
            MmUnlockAddressSpace(AddressSpace);
            if (Attached) KeUnstackDetachProcess(&ApcState);
            if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);
            DPRINT("NtAllocateVirtualMemory() = %x\n",Status);

            /* Give the caller rounded BaseAddress and area length */
            if (NT_SUCCESS(Status))
            {
                *UBaseAddress = BaseAddress;
                *URegionSize = RegionSize;
                DPRINT("*UBaseAddress %x  *URegionSize %x\n", BaseAddress, RegionSize);
            }

            return(Status);
         }
         else if (MemoryAreaLength >= RegionSize)
         {
            /* Region list initialized? */
            if (MemoryArea->Data.SectionData.RegionListHead.Flink)
            {
               Status =
                  MmAlterRegion(AddressSpace,
                                MemoryArea->StartingAddress,
                                &MemoryArea->Data.SectionData.RegionListHead,
                                BaseAddress, RegionSize,
                                Type, Protect, MmModifyAttributes);
            }
            else
            {
               Status = STATUS_ACCESS_VIOLATION;
            }

            MmUnlockAddressSpace(AddressSpace);
            if (Attached) KeUnstackDetachProcess(&ApcState);
            if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);
            DPRINT("NtAllocateVirtualMemory() = %x\n",Status);

            /* Give the caller rounded BaseAddress and area length */
            if (NT_SUCCESS(Status))
            {
                *UBaseAddress = BaseAddress;
                *URegionSize = RegionSize;
                DPRINT("*UBaseAddress %x  *URegionSize %x\n", BaseAddress, RegionSize);
            }

            return(Status);
         }
         else
         {
            MmUnlockAddressSpace(AddressSpace);
            if (Attached) KeUnstackDetachProcess(&ApcState);
            if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);
            return(STATUS_UNSUCCESSFUL);
         }
      }
   }

   Status = MmCreateMemoryArea(AddressSpace,
                               MEMORY_AREA_VIRTUAL_MEMORY,
                               &BaseAddress,
                               RegionSize,
                               Protect,
                               &MemoryArea,
                               PBaseAddress != 0,
                               AllocationType & MEM_TOP_DOWN,
                               BoundaryAddressMultiple);
   if (!NT_SUCCESS(Status))
   {
      MmUnlockAddressSpace(AddressSpace);
      if (Attached) KeUnstackDetachProcess(&ApcState);
      if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);
      DPRINT("NtAllocateVirtualMemory() = %x\n",Status);
      return(Status);
   }

   MemoryAreaLength = (ULONG_PTR)MemoryArea->EndingAddress -
                      (ULONG_PTR)MemoryArea->StartingAddress;

   MmInitializeRegion(&MemoryArea->Data.VirtualMemoryData.RegionListHead,
                      MemoryAreaLength, Type, Protect);

   if ((AllocationType & MEM_COMMIT) &&
       (Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE)))
   {
      const ULONG nPages = PAGE_ROUND_UP(MemoryAreaLength) >> PAGE_SHIFT;
      MmReserveSwapPages(nPages);
   }

   MmUnlockAddressSpace(AddressSpace);
   if (Attached) KeUnstackDetachProcess(&ApcState);
   if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);

   *UBaseAddress = BaseAddress;
   *URegionSize = MemoryAreaLength;
   DPRINT("*UBaseAddress %x  *URegionSize %x\n", BaseAddress, RegionSize);

   return(STATUS_SUCCESS);
}

static VOID
MmFreeVirtualMemoryPage(PVOID Context,
                        MEMORY_AREA* MemoryArea,
                        PVOID Address,
                        PFN_NUMBER Page,
                        SWAPENTRY SwapEntry,
                        BOOLEAN Dirty)
{
   PEPROCESS Process = (PEPROCESS)Context;

   if (Page != 0)
   {
      SWAPENTRY SavedSwapEntry;
      SavedSwapEntry = MmGetSavedSwapEntryPage(Page);
      if (SavedSwapEntry != 0)
      {
         MmFreeSwapPage(SavedSwapEntry);
         MmSetSavedSwapEntryPage(Page, 0);
      }
      MmDeleteRmap(Page, Process, Address);
      MmReleasePageMemoryConsumer(MC_USER, Page);
   }
   else if (SwapEntry != 0)
   {
      MmFreeSwapPage(SwapEntry);
   }
}

VOID
NTAPI
MmFreeVirtualMemory(PEPROCESS Process,
                    PMEMORY_AREA MemoryArea)
{
   PLIST_ENTRY current_entry;
   PMM_REGION current;
   ULONG i;

   DPRINT("MmFreeVirtualMemory(Process %p  MemoryArea %p)\n", Process,
          MemoryArea);

   /* Mark this memory area as about to be deleted. */
   MemoryArea->DeleteInProgress = TRUE;

   /*
    * Wait for any ongoing paging operations. Notice that since we have
    * flagged this memory area as deleted no more page ops will be added.
    */
   if (MemoryArea->PageOpCount > 0)
   {
      ULONG_PTR MemoryAreaLength = (ULONG_PTR)MemoryArea->EndingAddress -
                                   (ULONG_PTR)MemoryArea->StartingAddress;
      const ULONG nPages = PAGE_ROUND_UP(MemoryAreaLength) >> PAGE_SHIFT;

      for (i = 0; i < nPages && MemoryArea->PageOpCount != 0; ++i)
      {
         PMM_PAGEOP PageOp;
         PageOp = MmCheckForPageOp(MemoryArea, Process->UniqueProcessId,
                                   (PVOID)((ULONG_PTR)MemoryArea->StartingAddress + (i * PAGE_SIZE)),
                                   NULL, 0);
         if (PageOp != NULL)
         {
            NTSTATUS Status;
            MmUnlockAddressSpace(&Process->Vm);
            Status = KeWaitForSingleObject(&PageOp->CompletionEvent,
                                           0,
                                           KernelMode,
                                           FALSE,
                                           NULL);
            if (Status != STATUS_SUCCESS)
            {
               DPRINT1("Failed to wait for page op\n");
               KeBugCheck(MEMORY_MANAGEMENT);
            }
            MmLockAddressSpace(&Process->Vm);
            MmReleasePageOp(PageOp);
         }
      }
   }

   /* Free all the individual segments. */
   current_entry = MemoryArea->Data.VirtualMemoryData.RegionListHead.Flink;
   while (current_entry != &MemoryArea->Data.VirtualMemoryData.RegionListHead)
   {
      current = CONTAINING_RECORD(current_entry, MM_REGION, RegionListEntry);
      current_entry = current_entry->Flink;
      ExFreePool(current);
   }

   /* Actually free the memory area. */
   MmFreeMemoryArea(&Process->Vm,
                    MemoryArea,
                    MmFreeVirtualMemoryPage,
                    (PVOID)Process);
}

/*
 * @implemented
 */
NTSTATUS NTAPI
NtFreeVirtualMemory(IN HANDLE ProcessHandle,
                    IN PVOID*  UBaseAddress,
                    IN PSIZE_T URegionSize,
                    IN ULONG FreeType)
/*
 * FUNCTION: Frees a range of virtual memory
 * ARGUMENTS:
 *        ProcessHandle = Points to the process that allocated the virtual
 *                        memory
 *        BaseAddress = Points to the memory address, rounded down to a
 *                      multiple of the pagesize
 *        RegionSize = Limits the range to free, rounded up to a multiple of
 *                     the paging size
 *        FreeType = Can be one of the values:  MEM_DECOMMIT, or MEM_RELEASE
 * RETURNS: Status
 */
{
   MEMORY_AREA* MemoryArea;
   NTSTATUS Status;
   PEPROCESS Process;
   PMMSUPPORT AddressSpace;
   PVOID BaseAddress, PBaseAddress;
   ULONG RegionSize, PRegionSize;
    PEPROCESS CurrentProcess = PsGetCurrentProcess();
    KPROCESSOR_MODE PreviousMode = KeGetPreviousMode();
    KAPC_STATE ApcState;
    BOOLEAN Attached = FALSE;
    PAGED_CODE();

    /* Only two flags are supported */
    if (!(FreeType & (MEM_RELEASE | MEM_DECOMMIT)))
    {
        DPRINT1("Invalid FreeType\n");
        return STATUS_INVALID_PARAMETER_4;
    }
   
    /* Check if no flag was used, or if both flags were used */
    if (!((FreeType & (MEM_DECOMMIT | MEM_RELEASE))) ||
         ((FreeType & (MEM_DECOMMIT | MEM_RELEASE)) == (MEM_DECOMMIT | MEM_RELEASE)))
    {
        DPRINT1("Invalid FreeType combination\n");
        return STATUS_INVALID_PARAMETER_4;
    }
   
    /* Enter SEH */
    _SEH2_TRY
    {
        /* Check for user-mode parameters */
        if (PreviousMode != KernelMode)
        {
            /* Make sure they are writable */
            ProbeForWritePointer(UBaseAddress);
            ProbeForWriteUlong(URegionSize);
        }
       
        /* Capture their values */
        PBaseAddress = *UBaseAddress;
        PRegionSize = *URegionSize;
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        /* Return the exception code */
        _SEH2_YIELD(return _SEH2_GetExceptionCode());
    }
    _SEH2_END;
   
    /* Make sure the allocation isn't past the user area */
    if (PBaseAddress >= MM_HIGHEST_USER_ADDRESS)
    {
        DPRINT1("Virtual free base above User Space\n");
        return STATUS_INVALID_PARAMETER_2;
    }
   
   /* Make sure the allocation wouldn't overflow past the user area */
   if (((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)PBaseAddress) < PRegionSize)
   {
       DPRINT1("Region size would overflow into kernel-memory\n");
       return STATUS_INVALID_PARAMETER_3;
   }

   /* Check if this is for the current process */
   if (ProcessHandle == NtCurrentProcess())
   {
       /* We already have the current process, no need to go through Ob */
       Process = CurrentProcess;
   }
   else
   {
       /* Reference the handle for correct permissions */
       Status = ObReferenceObjectByHandle(ProcessHandle,
                                          PROCESS_VM_OPERATION,
                                          PsProcessType,
                                          PreviousMode,
                                          (PVOID*)&Process,
                                          NULL);
       if (!NT_SUCCESS(Status)) return Status;
       
       /* Check if not running in the current process */
       if (CurrentProcess != Process)
       {
           /* Attach to it */
           KeStackAttachProcess(&Process->Pcb, &ApcState);
           Attached = TRUE;
       }
   }

   BaseAddress = (PVOID)PAGE_ROUND_DOWN((PBaseAddress));
   RegionSize = PAGE_ROUND_UP((ULONG_PTR)(PBaseAddress) + (PRegionSize)) -
                PAGE_ROUND_DOWN((PBaseAddress));

   AddressSpace = &Process->Vm;

   MmLockAddressSpace(AddressSpace);
   MemoryArea = MmLocateMemoryAreaByAddress(AddressSpace, BaseAddress);
   if (MemoryArea == NULL)
   {
      Status = STATUS_UNSUCCESSFUL;
      goto unlock_deref_and_return;
   }

   switch (FreeType)
   {
      case MEM_RELEASE:
         /* We can only free a memory area in one step. */
         if (MemoryArea->StartingAddress != BaseAddress ||
             MemoryArea->Type != MEMORY_AREA_VIRTUAL_MEMORY)
         {
            Status = STATUS_UNSUCCESSFUL;
            goto unlock_deref_and_return;
         }

         MmFreeVirtualMemory(Process, MemoryArea);
         Status = STATUS_SUCCESS;
         goto unlock_deref_and_return;

      case MEM_DECOMMIT:
         Status =
            MmAlterRegion(AddressSpace,
                          MemoryArea->StartingAddress,
                          (MemoryArea->Type == MEMORY_AREA_SECTION_VIEW) ?
                             &MemoryArea->Data.SectionData.RegionListHead :
                             &MemoryArea->Data.VirtualMemoryData.RegionListHead,
                          BaseAddress,
                          RegionSize,
                          MEM_RESERVE,
                          PAGE_NOACCESS,
                          MmModifyAttributes);
         goto unlock_deref_and_return;
   }

   Status = STATUS_NOT_IMPLEMENTED;

unlock_deref_and_return:

   MmUnlockAddressSpace(AddressSpace);
   if (Attached) KeUnstackDetachProcess(&ApcState);
   if (ProcessHandle != NtCurrentProcess()) ObDereferenceObject(Process);

   return(Status);
}

NTSTATUS
NTAPI
MmProtectAnonMem(PMMSUPPORT AddressSpace,
                 PMEMORY_AREA MemoryArea,
                 PVOID BaseAddress,
                 ULONG Length,
                 ULONG Protect,
                 PULONG OldProtect)
{
   PMM_REGION Region;
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG LengthCount = 0;

   /* Search all Regions in MemoryArea up to Length */
   /* Every Region up to Length must be committed for success */
   for (;;)
   {
      Region = MmFindRegion(MemoryArea->StartingAddress,
                            &MemoryArea->Data.VirtualMemoryData.RegionListHead,
                            (PVOID)((ULONG_PTR)BaseAddress + (ULONG_PTR)LengthCount), NULL);

      /* If a Region was found and it is committed */
      if ((Region) && (Region->Type == MEM_COMMIT))
      {
         LengthCount += Region->Length;
         if (Length <= LengthCount) break;
         continue;
      }
      /* If Region was found and it is not commited */
      else if (Region)
      {
         Status = STATUS_NOT_COMMITTED;
         break;
      }
      /* If no Region was found at all */
      else if (LengthCount == 0)
      {
         Status = STATUS_INVALID_ADDRESS;
         break;
      }
   }

   if (NT_SUCCESS(Status))
   {
       *OldProtect = Region->Protect;
       Status = MmAlterRegion(AddressSpace, MemoryArea->StartingAddress,
                              &MemoryArea->Data.VirtualMemoryData.RegionListHead,
                              BaseAddress, Length, Region->Type, Protect,
                              MmModifyAttributes);
   }

   return (Status);
}

NTSTATUS NTAPI
MmQueryAnonMem(PMEMORY_AREA MemoryArea,
               PVOID Address,
               PMEMORY_BASIC_INFORMATION Info,
               PSIZE_T ResultLength)
{
   PMM_REGION Region;
   PVOID RegionBase = NULL;

   Info->BaseAddress = (PVOID)PAGE_ROUND_DOWN(Address);

   Region = MmFindRegion(MemoryArea->StartingAddress,
                         &MemoryArea->Data.VirtualMemoryData.RegionListHead,
                         Address, &RegionBase);
   Info->BaseAddress = RegionBase;
   Info->AllocationBase = MemoryArea->StartingAddress;
   Info->AllocationProtect = MemoryArea->Protect;
   Info->RegionSize = Region->Length;
   Info->State = Region->Type;
   Info->Protect = Region->Protect;
   Info->Type = MEM_PRIVATE;

   *ResultLength = sizeof(MEMORY_BASIC_INFORMATION);
   return(STATUS_SUCCESS);
}

/* EOF */
