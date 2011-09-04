/*
 * Copyright (C) 1998-2005 ReactOS Team; (C) 2011 NasuTek Enterprises (and the authors from the programmers section)
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
 * FILE:            ntoskrnl/mm/section/fault.c
 * PURPOSE:         Consolidate fault handlers for sections
 *
 * PROGRAMMERS:     Arty
 *                  Rex Jolliff
 *                  David Welch
 *                  Eric Kohl
 *                  Emanuele Aliberti
 *                  Eugene Ingerman
 *                  Casper Hornstrup
 *                  KJK::Hyperion
 *                  Guido de Jong
 *                  Ge van Geldorp
 *                  Royce Mitchell III
 *                  Filip Navara
 *                  Aleksey Bragin
 *                  Jason Filby
 *                  Thomas Weidenmueller
 *                  Gunnar Andre' Dalsnes
 *                  Mike Nordell
 *                  Alex Ionescu
 *                  Gregor Anich
 *                  Steven Edwards
 *                  Herve Poussineau
 */

/* INCLUDES *****************************************************************/

#include <ntoskrnl.h>
#include "newmm.h"
#define NDEBUG
#include <debug.h>

#define DPRINTC DPRINT

extern KEVENT MmWaitPageEvent;
extern FAST_MUTEX RmapListLock;

FAST_MUTEX GlobalPageOperation;

PFN_NUMBER
NTAPI
MmWithdrawSectionPage
(PMM_CACHE_SECTION_SEGMENT Segment, PLARGE_INTEGER FileOffset, BOOLEAN *Dirty)
{
	ULONG Entry;

	DPRINT("MmWithdrawSectionPage(%x,%08x%08x,%x)\n", Segment, FileOffset->HighPart, FileOffset->LowPart, Dirty);

	MmLockCacheSectionSegment(Segment);
	Entry = MiGetPageEntryCacheSectionSegment(Segment, FileOffset);

	*Dirty = !!IS_DIRTY_SSE(Entry);

	DPRINT("Withdraw %x (%x) of %wZ\n", FileOffset->LowPart, Entry, Segment->FileObject ? &Segment->FileObject->FileName : NULL);

	if (!Entry) 
	{
		DPRINT("Stoeled!\n");
		MmUnlockCacheSectionSegment(Segment);
		return 0;
	}
	else if (MM_IS_WAIT_PTE(Entry)) 
	{
		DPRINT("WAIT\n");
		MmUnlockCacheSectionSegment(Segment);
		return MM_WAIT_ENTRY;
	}
	else if (Entry && !IS_SWAP_FROM_SSE(Entry))
	{
		DPRINT("Page %x\n", PFN_FROM_SSE(Entry));
		*Dirty |= (Entry & 2);
		MiSetPageEntryCacheSectionSegment(Segment, FileOffset, MAKE_SWAP_SSE(MM_WAIT_ENTRY));
		MmUnlockCacheSectionSegment(Segment);
		return PFN_FROM_SSE(Entry);
	}
	else
	{
		DPRINT1("SWAP ENTRY?! (%x:%08x%08x)\n", Segment, FileOffset->HighPart, FileOffset->LowPart);
		ASSERT(FALSE);
		MmUnlockCacheSectionSegment(Segment);
		return 0;
	}
}

NTSTATUS
NTAPI
MmFinalizeSectionPageOut
(PMM_CACHE_SECTION_SEGMENT Segment, PLARGE_INTEGER FileOffset, PFN_NUMBER Page,
 BOOLEAN Dirty)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BOOLEAN WriteZero = FALSE, WritePage = FALSE;
	SWAPENTRY Swap = MmGetSavedSwapEntryPage(Page);

	MmLockCacheSectionSegment(Segment);
	(void)InterlockedIncrementUL(&Segment->ReferenceCount);

	if (Dirty)
	{
		DPRINT("Finalize (dirty) Segment %x Page %x\n", Segment, Page);
		DPRINT("Segment->FileObject %x\n", Segment->FileObject);
		DPRINT("Segment->Flags %x\n", Segment->Flags);

		WriteZero = TRUE;
		WritePage = TRUE;
	}
	else
	{
		WriteZero = TRUE;
	}

	DPRINT("Status %x\n", Status);

	MmUnlockCacheSectionSegment(Segment);

	if (WritePage)
	{
		DPRINT("MiWriteBackPage(Segment %x FileObject %x Offset %x)\n", Segment, Segment->FileObject, FileOffset->LowPart);
		Status = MiWriteBackPage(Segment->FileObject, FileOffset, PAGE_SIZE, Page);
	}

	MmLockCacheSectionSegment(Segment);

	if (WriteZero && NT_SUCCESS(Status))
	{
		DPRINT("Setting page entry in segment %x:%x to swap %x\n", Segment, FileOffset->LowPart, Swap);
		MiSetPageEntryCacheSectionSegment(Segment, FileOffset, Swap ? MAKE_SWAP_SSE(Swap) : 0);
	}
	else
	{
		DPRINT("Setting page entry in segment %x:%x to page %x\n", Segment, FileOffset->LowPart, Page);
		MiSetPageEntryCacheSectionSegment
			(Segment, FileOffset, Page ? (Dirty ? DIRTY_SSE(MAKE_PFN_SSE(Page)) : MAKE_PFN_SSE(Page)) : 0);
	}

	if (NT_SUCCESS(Status))
	{
		DPRINT("Removing page %x for real\n", Page);
		MmSetSavedSwapEntryPage(Page, 0);
		// Note: the other one is held by MmTrimUserMemory
		if (MmGetReferenceCountPage(Page) != 2) {
			DPRINT1("ALERT: Page %x about to be evicted with ref count %d\n", Page, MmGetReferenceCountPage(Page));
		}
		MmDereferencePage(Page);
	}

	MmUnlockCacheSectionSegment(Segment);

	if (InterlockedDecrementUL(&Segment->ReferenceCount) == 0)
	{
		MmFinalizeSegment(Segment);
	}

	/* Note: Writing may evict the segment...  Nothing is guaranteed from here down */
	MiSetPageEvent(Segment, FileOffset->LowPart);

	DPRINT("Status %x\n", Status);
	return Status;
}

NTSTATUS
NTAPI
MmPageOutCacheSection
(PMMSUPPORT AddressSpace,
 MEMORY_AREA* MemoryArea,
 PVOID Address,
 PMM_REQUIRED_RESOURCES Required)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG Entry;
	BOOLEAN Dirty = FALSE;
	PEPROCESS Process = MmGetAddressSpaceOwner(AddressSpace);
	LARGE_INTEGER TotalOffset;
	PMM_CACHE_SECTION_SEGMENT Segment;
	PVOID PAddress = MM_ROUND_DOWN(Address, PAGE_SIZE);

	TotalOffset.QuadPart = (ULONG_PTR)PAddress - (ULONG_PTR)MemoryArea->StartingAddress + 
		MemoryArea->Data.CacheData.ViewOffset.QuadPart;

	Segment = MemoryArea->Data.CacheData.Segment;

	MmLockCacheSectionSegment(Segment);
	ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

	Dirty = MmIsDirtyPageRmap(Required->Page[0]);
	Entry = MiGetPageEntryCacheSectionSegment(Segment, &TotalOffset);

	if (Dirty)
	{
		PFN_NUMBER OurPage;
		MiSetPageEntryCacheSectionSegment(Segment, &TotalOffset, DIRTY_SSE(Entry));
		MmDeleteRmap(Required->Page[0], Process, Address);
		MmDeleteVirtualMapping(Process, Address, FALSE, &Dirty, &OurPage);
		ASSERT(OurPage == Required->Page[0]);
	} else {
		/* Just unmap if the page wasn't dirty */
		PFN_NUMBER OurPage;
		MmDeleteRmap(Required->Page[0], Process, Address);
		MmDeleteVirtualMapping(Process, Address, FALSE, &Dirty, &OurPage);
		DPRINT("OurPage %x ThePage %x\n", OurPage, Required->Page[0]);
		ASSERT(OurPage == Required->Page[0]);
	}

	if (NT_SUCCESS(Status)) 
	{
		MmDereferencePage(Required->Page[0]);
	} 

	MmUnlockCacheSectionSegment(Segment);
	MiSetPageEvent(Process, Address);
	return Status;
}

NTSTATUS
NTAPI
MmpPageOutPhysicalAddress(PFN_NUMBER Page)
{
   BOOLEAN ProcRef = FALSE;
   PFN_NUMBER SectionPage = 0;
   PMM_RMAP_ENTRY entry;
   PMM_CACHE_SECTION_SEGMENT Segment = NULL;
   LARGE_INTEGER FileOffset;
   PMEMORY_AREA MemoryArea;
   PMMSUPPORT AddressSpace = MmGetKernelAddressSpace();
   BOOLEAN Dirty = FALSE;
   PVOID Address = NULL;
   PEPROCESS Process = NULL;
   NTSTATUS Status = STATUS_SUCCESS;
   MM_REQUIRED_RESOURCES Resources = { 0 };

   DPRINTC("Page out %x (ref ct %x)\n", Page, MmGetReferenceCountPage(Page));

   ExAcquireFastMutex(&GlobalPageOperation);
   if ((Segment = MmGetSectionAssociation(Page, &FileOffset)))
   {
	   DPRINT1("Withdrawing page (%x) %x:%x\n", Page, Segment, FileOffset.LowPart);
	   SectionPage = MmWithdrawSectionPage(Segment, &FileOffset, &Dirty);
	   DPRINTC("SectionPage %x\n", SectionPage);

	   if (SectionPage == MM_WAIT_ENTRY || SectionPage == 0)
	   {
		   DPRINT1("In progress page out %x\n", SectionPage);
		   ExReleaseFastMutex(&GlobalPageOperation);
		   return STATUS_UNSUCCESSFUL;
	   }
	   else
	   {
		   ASSERT(SectionPage == Page);
	   }
	   Resources.State = Dirty ? 1 : 0;
   }
   else
   {
	   DPRINT("No segment association for %x\n", Page);
   }


   Dirty = MmIsDirtyPageRmap(Page);

   DPRINTC("Trying to unmap all instances of %x\n", Page);
   ExAcquireFastMutex(&RmapListLock);
   entry = MmGetRmapListHeadPage(Page);

   // Entry and Segment might be null here in the case that the page
   // is new and is in the process of being swapped in
   if (!entry && !Segment)
   {
	   Status = STATUS_UNSUCCESSFUL;
	   DPRINT1("Page %x is in transit\n", Page);
	   ExReleaseFastMutex(&RmapListLock);
	   goto bail;
   }

   while (entry != NULL && NT_SUCCESS(Status))
   {
	   Process = entry->Process;
	   Address = entry->Address;

	   DPRINTC("Process %x Address %x Page %x\n", Process, Address, Page);

	   if (RMAP_IS_SEGMENT(Address)) {
		   entry = entry->Next;
		   continue;
	   }

	   if (Process && Address < MmSystemRangeStart)
	   {
		   // Make sure we don't try to page out part of an exiting process
		   if (PspIsProcessExiting(Process))
		   {
			   DPRINT("bail\n");
			   ExReleaseFastMutex(&RmapListLock);
			   goto bail;
		   }
		   Status = ObReferenceObject(Process);
		   if (!NT_SUCCESS(Status))
		   {
			   DPRINT("bail\n");
			   ExReleaseFastMutex(&RmapListLock);
			   goto bail;
		   }
		   ProcRef = TRUE;
		   AddressSpace = &Process->Vm;
	   }
	   else
	   {
		   AddressSpace = MmGetKernelAddressSpace();
	   }
	   ExReleaseFastMutex(&RmapListLock);

	   RtlZeroMemory(&Resources, sizeof(Resources));

	   if ((((ULONG_PTR)Address) & 0xFFF) != 0)
	   {
		   KeBugCheck(MEMORY_MANAGEMENT);
	   }

	   if (!MmTryToLockAddressSpace(AddressSpace))
	   {
		   DPRINT1("Could not lock address space for process %x\n", MmGetAddressSpaceOwner(AddressSpace));
		   Status = STATUS_UNSUCCESSFUL;
		   goto bail;
	   }

	   do 
	   {
		   MemoryArea = MmLocateMemoryAreaByAddress(AddressSpace, Address);
		   if (MemoryArea == NULL || 
			   MemoryArea->DeleteInProgress)
		   {
			   Status = STATUS_UNSUCCESSFUL;
			   MmUnlockAddressSpace(AddressSpace);
			   DPRINTC("bail\n");
			   goto bail;
		   }
		   
		   DPRINTC
			   ("Type %x (%x -> %x)\n", 
				MemoryArea->Type, 
				MemoryArea->StartingAddress, 
				MemoryArea->EndingAddress);
		   
		   Resources.DoAcquisition = NULL;
		   
		   Resources.Page[0] = Page;
		   
		   ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

		   DPRINT("%x:%x, page %x %x\n", Process, Address, Page, Resources.Page[0]);
		   Status = MmPageOutCacheSection
			   (AddressSpace, MemoryArea, Address, &Resources);
		   DPRINT("%x\n", Status);
		   
		   ASSERT(KeGetCurrentIrql() <= APC_LEVEL);
		   
		   MmUnlockAddressSpace(AddressSpace);
		   
		   if (Status == STATUS_SUCCESS + 1)
		   {
			   // Wait page ... the other guy has it, so we'll just fail for now
			   DPRINTC("Wait entry ... can't continue\n");
			   Status = STATUS_UNSUCCESSFUL;
			   goto bail;
		   }
		   else if (Status == STATUS_MORE_PROCESSING_REQUIRED)
		   {
			   DPRINTC("DoAcquisition %x\n", Resources.DoAcquisition);
			   Status = Resources.DoAcquisition(AddressSpace, MemoryArea, &Resources);
			   DPRINTC("Status %x\n", Status);
			   if (!NT_SUCCESS(Status)) 
			   {
				   DPRINT1("bail\n");
				   goto bail;
			   }
			   else Status = STATUS_MM_RESTART_OPERATION;
		   }
		   
		   MmLockAddressSpace(AddressSpace);
	   } 
	   while (Status == STATUS_MM_RESTART_OPERATION);
	   Dirty |= Resources.State & 1; // Accumulate dirty

	   MmUnlockAddressSpace(AddressSpace);

	   if (ProcRef)
	   {
		   ObDereferenceObject(Process);
		   ProcRef = FALSE;
	   }
	   
	   ExAcquireFastMutex(&RmapListLock);
	   ASSERT(!MM_IS_WAIT_PTE(MmGetPfnForProcess(Process, Address)));
	   entry = MmGetRmapListHeadPage(Page);

	   DPRINTC("Entry %x\n", entry);
   }

   ExReleaseFastMutex(&RmapListLock);

bail:
   DPRINTC("BAIL %x\n", Status);

   if (Segment)
   {
	   DPRINTC("About to finalize section page %x (%x:%x) Status %x %s\n", Page, Segment, FileOffset.LowPart, Status, Dirty ? "dirty" : "clean");

	   if (!NT_SUCCESS(Status) ||
		   !NT_SUCCESS
		   (Status = MmFinalizeSectionPageOut
			(Segment, &FileOffset, Page, Dirty)))
	   {
		   DPRINTC
			   ("Failed to page out %x, replacing %x at %x in segment %x\n",
				SectionPage, FileOffset.LowPart, Segment);
		   MmLockCacheSectionSegment(Segment);
		   MiSetPageEntryCacheSectionSegment(Segment, &FileOffset, Dirty ? MAKE_PFN_SSE(Page) : DIRTY_SSE(MAKE_PFN_SSE(Page)));
		   MmUnlockCacheSectionSegment(Segment);
	   }

	   // Alas, we had the last reference
	   ULONG RefCount;
	   if ((RefCount = InterlockedDecrementUL(&Segment->ReferenceCount)) == 0)
		   MmFinalizeSegment(Segment);
   }

   if (ProcRef)
   {
	   DPRINTC("Dereferencing process...\n");
	   ObDereferenceObject(Process);
   }

   ExReleaseFastMutex(&GlobalPageOperation);
   DPRINTC("%s %x %x\n", NT_SUCCESS(Status) ? "Evicted" : "Spared", Page, Status);
   return NT_SUCCESS(Status) ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
}

ULONG
NTAPI
MiCacheEvictPages(PVOID BaseAddress, ULONG Target)
{
	ULONG i, Entry, Result = 0;
	NTSTATUS Status;
	PFN_NUMBER Page;
	PMEMORY_AREA MemoryArea;
	LARGE_INTEGER Offset;
	PMM_CACHE_SECTION_SEGMENT Segment;

	MmLockAddressSpace(MmGetKernelAddressSpace());
	MemoryArea = MmLocateMemoryAreaByAddress
		(MmGetKernelAddressSpace(), 
		 BaseAddress);

	ASSERT(MemoryArea);
	ASSERT(MemoryArea->Type == MEMORY_AREA_CACHE);

	Segment = MemoryArea->Data.CacheData.Segment;

	ASSERT(Segment);

	MmLockCacheSectionSegment(Segment);

	for (i = 0; 
		 i < ((ULONG_PTR)MemoryArea->EndingAddress) - 
			 ((ULONG_PTR)MemoryArea->StartingAddress) && 
			 Result < Target; 
		 i += PAGE_SIZE) {
		Offset.QuadPart = MemoryArea->Data.CacheData.ViewOffset.QuadPart + i;
		Entry = MiGetPageEntryCacheSectionSegment(Segment, &Offset);
		if (Entry && !IS_SWAP_FROM_SSE(Entry)) {
			Page = PFN_FROM_SSE(Entry);
			MmReferencePage(Page);
			MmUnlockCacheSectionSegment(Segment);
			MmUnlockAddressSpace(MmGetKernelAddressSpace());
			Status = MmpPageOutPhysicalAddress(Page);
			if (NT_SUCCESS(Status))
				Result++;
			MmLockCacheSectionSegment(Segment);
			MmLockAddressSpace(MmGetKernelAddressSpace());
			MmReleasePageMemoryConsumer(MC_CACHE, Page);
		}
	}

	MmUnlockCacheSectionSegment(Segment);
	MmUnlockAddressSpace(MmGetKernelAddressSpace());

	return Result;
}

