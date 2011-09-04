/*
 *  FreeLoader
 *  Copyright (C) 2006-2008     Aleksey Bragin  <aleksey@odyssey.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <freeldr.h>
#include <debug.h>

#if DBG
VOID		DumpMemoryAllocMap(VOID);
VOID		MemAllocTest(VOID);
#endif // DBG

DBG_DEFAULT_CHANNEL(MEMORY);

ULONG LoaderPagesSpanned = 0;

PVOID MmAllocateMemoryWithType(ULONG MemorySize, TYPE_OF_MEMORY MemoryType)
{
	ULONG	PagesNeeded;
	ULONG	FirstFreePageFromEnd;
	PVOID	MemPointer;

	if (MemorySize == 0)
	{
		WARN("MmAllocateMemory() called for 0 bytes. Returning NULL.\n");
		UiMessageBoxCritical("Memory allocation failed: MmAllocateMemory() called for 0 bytes.");
		return NULL;
	}

	MemorySize = ROUND_UP(MemorySize, 4);

	// Find out how many blocks it will take to
	// satisfy this allocation
	PagesNeeded = ROUND_UP(MemorySize, MM_PAGE_SIZE) / MM_PAGE_SIZE;

	// If we don't have enough available mem
	// then return NULL
	if (FreePagesInLookupTable < PagesNeeded)
	{
		ERR("Memory allocation failed in MmAllocateMemory(). Not enough free memory to allocate %d bytes.\n", MemorySize);
		UiMessageBoxCritical("Memory allocation failed: out of memory.");
		return NULL;
	}

	FirstFreePageFromEnd = MmFindAvailablePages(PageLookupTableAddress, TotalPagesInLookupTable, PagesNeeded, FALSE);

	if (FirstFreePageFromEnd == 0)
	{
		ERR("Memory allocation failed in MmAllocateMemory(). Not enough free memory to allocate %d bytes.\n", MemorySize);
		UiMessageBoxCritical("Memory allocation failed: out of memory.");
		return NULL;
	}

	MmAllocatePagesInLookupTable(PageLookupTableAddress, FirstFreePageFromEnd, PagesNeeded, MemoryType);

	FreePagesInLookupTable -= PagesNeeded;
	MemPointer = (PVOID)((ULONG_PTR)FirstFreePageFromEnd * MM_PAGE_SIZE);

	TRACE("Allocated %d bytes (%d pages) of memory starting at page %d.\n", MemorySize, PagesNeeded, FirstFreePageFromEnd);
	TRACE("Memory allocation pointer: 0x%x\n", MemPointer);

	// Update LoaderPagesSpanned count
	if ((((ULONG_PTR)MemPointer + MemorySize + PAGE_SIZE - 1) >> PAGE_SHIFT) > LoaderPagesSpanned)
		LoaderPagesSpanned = (((ULONG_PTR)MemPointer + MemorySize + PAGE_SIZE - 1) >> PAGE_SHIFT);

	// Now return the pointer
	return MemPointer;
}

PVOID MmHeapAlloc(ULONG MemorySize)
{
	PVOID Result;

	if (MemorySize > MM_PAGE_SIZE)
	{
		WARN("Consider using other functions to allocate %d bytes of memory!\n", MemorySize);
	}

	// Get the buffer from BGET pool
	Result = bget(MemorySize);

	if (Result == NULL)
	{
		ERR("Heap allocation for %d bytes failed\n", MemorySize);
	}
#if MM_DBG
    {
    	LONG CurAlloc, TotalFree, MaxFree, NumberOfGets, NumberOfRels;

	    // Gather some stats
	    bstats(&CurAlloc, &TotalFree, &MaxFree, &NumberOfGets, &NumberOfRels);

	    TRACE("Current alloced %d bytes, free %d bytes, allocs %d, frees %d\n",
		    CurAlloc, TotalFree, NumberOfGets, NumberOfRels);
	}
#endif
	return Result;
}

VOID MmHeapFree(PVOID MemoryPointer)
{
	// Release the buffer to the pool
	brel(MemoryPointer);
}

PVOID MmAllocateMemory(ULONG MemorySize)
{
	// Temporary forwarder...
	return MmAllocateMemoryWithType(MemorySize, LoaderOsloaderHeap);
}

PVOID MmAllocateMemoryAtAddress(ULONG MemorySize, PVOID DesiredAddress, TYPE_OF_MEMORY MemoryType)
{
	ULONG		PagesNeeded;
	ULONG		StartPageNumber;
	PVOID	MemPointer;

	if (MemorySize == 0)
	{
		WARN("MmAllocateMemoryAtAddress() called for 0 bytes. Returning NULL.\n");
		UiMessageBoxCritical("Memory allocation failed: MmAllocateMemoryAtAddress() called for 0 bytes.");
		return NULL;
	}

	// Find out how many blocks it will take to
	// satisfy this allocation
	PagesNeeded = ROUND_UP(MemorySize, MM_PAGE_SIZE) / MM_PAGE_SIZE;

	// Get the starting page number
	StartPageNumber = MmGetPageNumberFromAddress(DesiredAddress);

	// If we don't have enough available mem
	// then return NULL
	if (FreePagesInLookupTable < PagesNeeded)
	{
		ERR("Memory allocation failed in MmAllocateMemoryAtAddress(). "
			"Not enough free memory to allocate %d bytes (requesting %d pages but have only %d). "
			"\n", MemorySize, PagesNeeded, FreePagesInLookupTable);
		UiMessageBoxCritical("Memory allocation failed: out of memory.");
		return NULL;
	}

	if (MmAreMemoryPagesAvailable(PageLookupTableAddress, TotalPagesInLookupTable, DesiredAddress, PagesNeeded) == FALSE)
	{
		WARN("Memory allocation failed in MmAllocateMemoryAtAddress(). "
			 "Not enough free memory to allocate %d bytes at address %p.\n",
             MemorySize, DesiredAddress);

		// Don't tell this to user since caller should try to alloc this memory
		// at a different address
		//UiMessageBoxCritical("Memory allocation failed: out of memory.");
		return NULL;
	}

	MmAllocatePagesInLookupTable(PageLookupTableAddress, StartPageNumber, PagesNeeded, MemoryType);

	FreePagesInLookupTable -= PagesNeeded;
	MemPointer = (PVOID)((ULONG_PTR)StartPageNumber * MM_PAGE_SIZE);

	TRACE("Allocated %d bytes (%d pages) of memory starting at page %d.\n", MemorySize, PagesNeeded, StartPageNumber);
	TRACE("Memory allocation pointer: 0x%x\n", MemPointer);

	// Update LoaderPagesSpanned count
	if ((((ULONG_PTR)MemPointer + MemorySize + PAGE_SIZE - 1) >> PAGE_SHIFT) > LoaderPagesSpanned)
		LoaderPagesSpanned = (((ULONG_PTR)MemPointer + MemorySize + PAGE_SIZE - 1) >> PAGE_SHIFT);

	// Now return the pointer
	return MemPointer;
}

VOID MmSetMemoryType(PVOID MemoryAddress, ULONG MemorySize, TYPE_OF_MEMORY NewType)
{
	ULONG		PagesNeeded;
	ULONG		StartPageNumber;

	// Find out how many blocks it will take to
	// satisfy this allocation
	PagesNeeded = ROUND_UP(MemorySize, MM_PAGE_SIZE) / MM_PAGE_SIZE;

	// Get the starting page number
	StartPageNumber = MmGetPageNumberFromAddress(MemoryAddress);

	// Set new type for these pages
	MmAllocatePagesInLookupTable(PageLookupTableAddress, StartPageNumber, PagesNeeded, NewType);
}

PVOID MmAllocateHighestMemoryBelowAddress(ULONG MemorySize, PVOID DesiredAddress, TYPE_OF_MEMORY MemoryType)
{
	ULONG		PagesNeeded;
	ULONG		FirstFreePageFromEnd;
	ULONG		DesiredAddressPageNumber;
	PVOID	MemPointer;

	if (MemorySize == 0)
	{
		WARN("MmAllocateHighestMemoryBelowAddress() called for 0 bytes. Returning NULL.\n");
		UiMessageBoxCritical("Memory allocation failed: MmAllocateHighestMemoryBelowAddress() called for 0 bytes.");
		return NULL;
	}

	// Find out how many blocks it will take to
	// satisfy this allocation
	PagesNeeded = ROUND_UP(MemorySize, MM_PAGE_SIZE) / MM_PAGE_SIZE;

	// Get the page number for their desired address
	DesiredAddressPageNumber = (ULONG_PTR)DesiredAddress / MM_PAGE_SIZE;

	// If we don't have enough available mem
	// then return NULL
	if (FreePagesInLookupTable < PagesNeeded)
	{
		ERR("Memory allocation failed in MmAllocateHighestMemoryBelowAddress(). Not enough free memory to allocate %d bytes.\n", MemorySize);
		UiMessageBoxCritical("Memory allocation failed: out of memory.");
		return NULL;
	}

	FirstFreePageFromEnd = MmFindAvailablePagesBeforePage(PageLookupTableAddress, TotalPagesInLookupTable, PagesNeeded, DesiredAddressPageNumber);

	if (FirstFreePageFromEnd == 0)
	{
		ERR("Memory allocation failed in MmAllocateHighestMemoryBelowAddress(). Not enough free memory to allocate %d bytes.\n", MemorySize);
		UiMessageBoxCritical("Memory allocation failed: out of memory.");
		return NULL;
	}

	MmAllocatePagesInLookupTable(PageLookupTableAddress, FirstFreePageFromEnd, PagesNeeded, MemoryType);

	FreePagesInLookupTable -= PagesNeeded;
	MemPointer = (PVOID)((ULONG_PTR)FirstFreePageFromEnd * MM_PAGE_SIZE);

	TRACE("Allocated %d bytes (%d pages) of memory starting at page %d.\n", MemorySize, PagesNeeded, FirstFreePageFromEnd);
	TRACE("Memory allocation pointer: 0x%x\n", MemPointer);

	// Update LoaderPagesSpanned count
	if ((((ULONG_PTR)MemPointer + MemorySize) >> PAGE_SHIFT) > LoaderPagesSpanned)
		LoaderPagesSpanned = (((ULONG_PTR)MemPointer + MemorySize) >> PAGE_SHIFT);

	// Now return the pointer
	return MemPointer;
}

VOID MmFreeMemory(PVOID MemoryPointer)
{
}

#if DBG

VOID DumpMemoryAllocMap(VOID)
{
	ULONG							Idx;
	PPAGE_LOOKUP_TABLE_ITEM		RealPageLookupTable = (PPAGE_LOOKUP_TABLE_ITEM)PageLookupTableAddress;

	DbgPrint("----------- Memory Allocation Bitmap -----------\n");

	for (Idx=0; Idx<TotalPagesInLookupTable; Idx++)
	{
		if ((Idx % 32) == 0)
		{
			DbgPrint("\n");
			DbgPrint("%08x:\t", (Idx * MM_PAGE_SIZE));
		}
		else if ((Idx % 4) == 0)
		{
			DbgPrint(" ");
		}

		switch (RealPageLookupTable[Idx].PageAllocated)
		{
		case LoaderFree:
			DbgPrint("*");
			break;
		case LoaderBad:
			DbgPrint( "-");
			break;
		case LoaderLoadedProgram:
			DbgPrint("O");
			break;
		case LoaderFirmwareTemporary:
			DbgPrint("T");
			break;
		case LoaderFirmwarePermanent:
			DbgPrint( "P");
			break;
		case LoaderOsloaderHeap:
			DbgPrint("H");
			break;
		case LoaderOsloaderStack:
			DbgPrint("S");
			break;
		case LoaderSystemCode:
			DbgPrint("K");
			break;
		case LoaderHalCode:
			DbgPrint("L");
			break;
		case LoaderBootDriver:
			DbgPrint("B");
			break;
		case LoaderStartupPcrPage:
			DbgPrint("G");
			break;
		case LoaderRegistryData:
			DbgPrint("R");
			break;
		case LoaderMemoryData:
			DbgPrint("M");
			break;
		case LoaderNlsData:
			DbgPrint("N");
			break;
		case LoaderSpecialMemory:
			DbgPrint("C");
			break;
		default:
			DbgPrint("?");
			break;
		}
	}

	DbgPrint("\n");
}
#endif // DBG

PPAGE_LOOKUP_TABLE_ITEM MmGetMemoryMap(ULONG *NoEntries)
{
	PPAGE_LOOKUP_TABLE_ITEM		RealPageLookupTable = (PPAGE_LOOKUP_TABLE_ITEM)PageLookupTableAddress;

	*NoEntries = TotalPagesInLookupTable;

	return RealPageLookupTable;
}

#undef ExAllocatePoolWithTag
NTKERNELAPI
PVOID
NTAPI
ExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag)
{
    return MmHeapAlloc(NumberOfBytes);
}

#undef ExFreePool
NTKERNELAPI
VOID
NTAPI
ExFreePool(
    IN PVOID P)
{
    MmHeapFree(P);
}

#undef ExFreePoolWithTag
NTKERNELAPI
VOID
NTAPI
ExFreePoolWithTag(
  IN PVOID P,
  IN ULONG Tag)
{
    ExFreePool(P);
}

PVOID
NTAPI
RtlAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size)
{
    PVOID ptr;

    ptr = MmHeapAlloc(Size);
    if (ptr && (Flags & HEAP_ZERO_MEMORY))
    {
        RtlZeroMemory(ptr, Size);
    }

    return ptr;
}

BOOLEAN
NTAPI
RtlFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID HeapBase)
{
    MmHeapFree(HeapBase);
    return TRUE;
}
