#pragma once

#include <internal/arch/mm.h>

/* TYPES *********************************************************************/

#define MM_WAIT_ENTRY            0x7ffff800
#define PFN_FROM_SSE(E)          ((E) >> PAGE_SHIFT)
#define IS_SWAP_FROM_SSE(E)      ((E) & 0x00000001)
#define MM_IS_WAIT_PTE(E)        \
	(IS_SWAP_FROM_SSE(E) && SWAPENTRY_FROM_SSE(E) == MM_WAIT_ENTRY)
#define MAKE_PFN_SSE(P)          ((P) << PAGE_SHIFT)
#define SWAPENTRY_FROM_SSE(E)    ((E) >> 1)
#define MAKE_SWAP_SSE(S)         (((S) << 1) | 0x1)
#define DIRTY_SSE(E)             ((E) | 2)
#define CLEAN_SSE(E)             ((E) & ~2)
#define IS_DIRTY_SSE(E)          ((E) & 2)

#define MEMORY_AREA_CACHE   (2)
#define MM_SEGMENT_FINALIZE (0x40000000)

#define RMAP_SEGMENT_MASK ~0xff
#define RMAP_IS_SEGMENT(x) (((ULONG_PTR)(x) & RMAP_SEGMENT_MASK) == RMAP_SEGMENT_MASK)

#define MIN(x,y) (((x)<(y))?(x):(y))
#define MAX(x,y) (((x)>(y))?(x):(y))

/* Determine what's needed to make paged pool fit in this category.
 * it seems that something more is required to satisfy arm3. */
#define BALANCER_CAN_EVICT(Consumer) \
	(((Consumer) == MC_USER) || \
	 ((Consumer) == MC_CACHE)) 

#define SEC_CACHE                           (0x40000000)

#define MiWaitForPageEvent(Process,Address) do {						\
	DPRINT("MiWaitForPageEvent %x:%x #\n", Process, Address); \
    KeWaitForSingleObject(&MmWaitPageEvent, 0, KernelMode, FALSE, NULL); \
} while(0)

#define MiSetPageEvent(Process,Address) do {						\
	DPRINT("MiSetPageEvent %x:%x #\n",Process, Address);	\
	KeSetEvent(&MmWaitPageEvent, IO_NO_INCREMENT, FALSE); \
} while(0)

/* We store 8 bits of location with a page association */
#define ENTRIES_PER_ELEMENT 256

extern KEVENT MmWaitPageEvent;
								   
typedef struct _MM_CACHE_SECTION_SEGMENT
{
    FAST_MUTEX Lock;		/* lock which protects the page directory */
	PFILE_OBJECT FileObject;
    ULARGE_INTEGER RawLength;		/* length of the segment which is part of the mapped file */
    ULARGE_INTEGER Length;			/* absolute length of the segment */
    ULONG ReferenceCount;
    ULONG Protection;
    ULONG Flags;
    BOOLEAN WriteCopy;

	struct 
	{
		LONG FileOffset;		/* start offset into the file for image sections */
		ULONG_PTR VirtualAddress;	/* dtart offset into the address range for image sections */
		ULONG Characteristics;
	} Image;

	RTL_GENERIC_TABLE PageTable;
} MM_CACHE_SECTION_SEGMENT, *PMM_CACHE_SECTION_SEGMENT;

typedef struct _CACHE_SECTION_PAGE_TABLE 
{
    LARGE_INTEGER FileOffset;
	PMM_CACHE_SECTION_SEGMENT Segment;
    ULONG Refcount;
    ULONG PageEntries[ENTRIES_PER_ELEMENT];
} CACHE_SECTION_PAGE_TABLE, *PCACHE_SECTION_PAGE_TABLE;

struct _MM_REQUIRED_RESOURCES;

typedef NTSTATUS (NTAPI * AcquireResource)
	(PMMSUPPORT AddressSpace,
	 struct _MEMORY_AREA *MemoryArea,
	 struct _MM_REQUIRED_RESOURCES *Required);
typedef NTSTATUS (NTAPI * NotPresentFaultHandler)
	(PMMSUPPORT AddressSpace, 
	 struct _MEMORY_AREA *MemoryArea, 
	 PVOID Address,
	 BOOLEAN Locked,
	 struct _MM_REQUIRED_RESOURCES *Required);
typedef NTSTATUS (NTAPI * FaultHandler)
	(PMMSUPPORT AddressSpace, 
	 struct _MEMORY_AREA *MemoryArea, 
	 PVOID Address,
	 struct _MM_REQUIRED_RESOURCES *Required);

typedef struct _MM_REQUIRED_RESOURCES
{
	ULONG Consumer;
	ULONG Amount;
	ULONG Offset;
	ULONG State;
	PVOID Context;
	LARGE_INTEGER FileOffset;
	AcquireResource DoAcquisition;
	PFN_NUMBER Page[2];
	PVOID Buffer[2];
	SWAPENTRY SwapEntry;
	const char *File;
	int Line;
} MM_REQUIRED_RESOURCES, *PMM_REQUIRED_RESOURCES;

PFN_NUMBER
NTAPI
MmWithdrawSectionPage
(PMM_CACHE_SECTION_SEGMENT Segment, PLARGE_INTEGER FileOffset, BOOLEAN *Dirty);

NTSTATUS
NTAPI
MmFinalizeSectionPageOut
(PMM_CACHE_SECTION_SEGMENT Segment, PLARGE_INTEGER FileOffset, PFN_NUMBER Page,
 BOOLEAN Dirty);

/* sptab.c *******************************************************************/

VOID
NTAPI
MiInitializeSectionPageTable(PMM_CACHE_SECTION_SEGMENT Segment);

NTSTATUS
NTAPI
_MiSetPageEntryCacheSectionSegment
(PMM_CACHE_SECTION_SEGMENT Segment,
 PLARGE_INTEGER Offset,
 ULONG Entry, const char *file, int line);

ULONG
NTAPI
_MiGetPageEntryCacheSectionSegment
(PMM_CACHE_SECTION_SEGMENT Segment,
 PLARGE_INTEGER Offset, const char *file, int line);

#define MiSetPageEntryCacheSectionSegment(S,O,E) _MiSetPageEntryCacheSectionSegment(S,O,E,__FILE__,__LINE__)
#define MiGetPageEntryCacheSectionSegment(S,O) _MiGetPageEntryCacheSectionSegment(S,O,__FILE__,__LINE__)

typedef VOID (NTAPI *FREE_SECTION_PAGE_FUN)
	(PMM_CACHE_SECTION_SEGMENT Segment,
	 PLARGE_INTEGER Offset);

VOID
NTAPI
MiFreePageTablesSectionSegment(PMM_CACHE_SECTION_SEGMENT Segment, FREE_SECTION_PAGE_FUN FreePage);

/* Yields a lock */
PMM_CACHE_SECTION_SEGMENT
NTAPI
MmGetSectionAssociation(PFN_NUMBER Page, PLARGE_INTEGER Offset);

NTSTATUS
NTAPI
MmSetSectionAssociation(PFN_NUMBER Page, PMM_CACHE_SECTION_SEGMENT Segment, PLARGE_INTEGER Offset);

VOID
NTAPI
MmDeleteSectionAssociation(PFN_NUMBER Page);

NTSTATUS
NTAPI
MmpPageOutPhysicalAddress(PFN_NUMBER Page);

/* io.c **********************************************************************/

NTSTATUS
MmspWaitForFileLock(PFILE_OBJECT File);

NTSTATUS
NTAPI
MiSimpleRead
(PFILE_OBJECT FileObject, 
 PLARGE_INTEGER FileOffset,
 PVOID Buffer, 
 ULONG Length,
#ifdef __ROS_CMAKE__
 BOOLEAN Paging,
#endif
 PIO_STATUS_BLOCK ReadStatus);

NTSTATUS
NTAPI
_MiSimpleWrite
(PFILE_OBJECT FileObject, 
 PLARGE_INTEGER FileOffset,
 PVOID Buffer, 
 ULONG Length,
 PIO_STATUS_BLOCK ReadStatus,
 const char *file,
 int line);

#define MiSimpleWrite(F,O,B,L,R) _MiSimpleWrite(F,O,B,L,R,__FILE__,__LINE__)

NTSTATUS
NTAPI
_MiWriteBackPage
(PFILE_OBJECT FileObject,
 PLARGE_INTEGER Offset,
 ULONG Length,
 PFN_NUMBER Page,
 const char *File,
 int Line);

#define MiWriteBackPage(F,O,L,P) _MiWriteBackPage(F,O,L,P,__FILE__,__LINE__)

/* section.c *****************************************************************/

NTSTATUS
NTAPI
MmAccessFaultCacheSection
(KPROCESSOR_MODE Mode,
 ULONG_PTR Address,
 BOOLEAN FromMdl);

NTSTATUS
NTAPI
MiReadFilePage
(PMMSUPPORT AddressSpace, 
 PMEMORY_AREA MemoryArea, 
 PMM_REQUIRED_RESOURCES RequiredResources);

ULONG
NTAPI
MiChecksumPage(PFN_NUMBER Page, BOOLEAN Lock);

NTSTATUS
NTAPI
MiGetOnePage
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PMM_REQUIRED_RESOURCES RequiredResources);

NTSTATUS
NTAPI
MiSwapInPage
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PMM_REQUIRED_RESOURCES RequiredResources);

NTSTATUS
NTAPI
MiWriteSwapPage
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PMM_REQUIRED_RESOURCES Resources);

NTSTATUS
NTAPI
MiWriteFilePage
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PMM_REQUIRED_RESOURCES Resources);

VOID
NTAPI
MiFreeSegmentPage
(PMM_CACHE_SECTION_SEGMENT Segment,
 PLARGE_INTEGER FileOffset);

NTSTATUS
NTAPI
MiCowCacheSectionPage
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PVOID Address,
 BOOLEAN Locked,
 PMM_REQUIRED_RESOURCES Required);

NTSTATUS
NTAPI
MiZeroFillSection(PVOID Address, PLARGE_INTEGER FileOffsetPtr, ULONG Length);

VOID
MmPageOutDeleteMapping(PVOID Context, PEPROCESS Process, PVOID Address);

VOID
NTAPI
_MmLockCacheSectionSegment(PMM_CACHE_SECTION_SEGMENT Segment, const char *file, int line);

#define MmLockCacheSectionSegment(x) _MmLockCacheSectionSegment(x,__FILE__,__LINE__)

VOID
NTAPI
_MmUnlockCacheSectionSegment(PMM_CACHE_SECTION_SEGMENT Segment, const char *file, int line);

#define MmUnlockCacheSectionSegment(x) _MmUnlockCacheSectionSegment(x,__FILE__,__LINE__)

VOID
MmFreeCacheSectionPage
(PVOID Context, MEMORY_AREA* MemoryArea, PVOID Address,
 PFN_NUMBER Page, SWAPENTRY SwapEntry, BOOLEAN Dirty);

NTSTATUS
NTAPI
_MiFlushMappedSection(PVOID BaseAddress, PLARGE_INTEGER BaseOffset, PLARGE_INTEGER FileSize, BOOLEAN Dirty, const char *File, int Line);

#define MiFlushMappedSection(A,O,S,D) _MiFlushMappedSection(A,O,S,D,__FILE__,__LINE__)

VOID
NTAPI
MmFinalizeSegment(PMM_CACHE_SECTION_SEGMENT Segment);

VOID
NTAPI
MmFreeSectionSegments(PFILE_OBJECT FileObject);

NTSTATUS NTAPI
MmMapCacheViewInSystemSpaceAtOffset
(IN PMM_CACHE_SECTION_SEGMENT Segment,
 OUT PVOID * MappedBase,
 IN PLARGE_INTEGER ViewOffset,
 IN OUT PULONG ViewSize);

NTSTATUS
NTAPI
_MiMapViewOfSegment
(PMMSUPPORT AddressSpace,
 PMM_CACHE_SECTION_SEGMENT Segment,
 PVOID* BaseAddress,
 SIZE_T ViewSize,
 ULONG Protect,
 PLARGE_INTEGER ViewOffset,
 ULONG AllocationType,
 const char *file,
 int line);

#define MiMapViewOfSegment(AddressSpace,Segment,BaseAddress,ViewSize,Protect,ViewOffset,AllocationType) _MiMapViewOfSegment(AddressSpace,Segment,BaseAddress,ViewSize,Protect,ViewOffset,AllocationType,__FILE__,__LINE__)

NTSTATUS
NTAPI
MmUnmapViewOfCacheSegment
(PMMSUPPORT AddressSpace,
 PVOID BaseAddress);

NTSTATUS
NTAPI
MmUnmapCacheViewInSystemSpace(PVOID Address);

NTSTATUS
NTAPI
MmNotPresentFaultCachePage
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PVOID Address,
 BOOLEAN Locked,
 PMM_REQUIRED_RESOURCES Required);

NTSTATUS
NTAPI
MmPageOutPageFileView
(PMMSUPPORT AddressSpace,
 PMEMORY_AREA MemoryArea,
 PVOID Address,
 PMM_REQUIRED_RESOURCES Required);

FORCEINLINE
BOOLEAN
_MmTryToLockAddressSpace(IN PMMSUPPORT AddressSpace, const char *file, int line)
{
	BOOLEAN Result = KeTryToAcquireGuardedMutex(&CONTAINING_RECORD(AddressSpace, EPROCESS, Vm)->AddressCreationLock);
	//DbgPrint("(%s:%d) Try Lock Address Space %x -> %s\n", file, line, AddressSpace, Result ? "true" : "false");
	return Result;
}

#define MmTryToLockAddressSpace(x) _MmTryToLockAddressSpace(x,__FILE__,__LINE__)

NTSTATUS
NTAPI
MiWidenSegment
(PMMSUPPORT AddressSpace, 
 PMEMORY_AREA MemoryArea, 
 PMM_REQUIRED_RESOURCES RequiredResources);

NTSTATUS
NTAPI
MiSwapInSectionPage
(PMMSUPPORT AddressSpace, 
 PMEMORY_AREA MemoryArea, 
 PMM_REQUIRED_RESOURCES RequiredResources);

NTSTATUS
NTAPI
MmExtendCacheSection(PMM_CACHE_SECTION_SEGMENT Section, PLARGE_INTEGER NewSize, BOOLEAN ExtendFile);

NTSTATUS
NTAPI
_MiFlushMappedSection(PVOID BaseAddress, PLARGE_INTEGER BaseOffset, PLARGE_INTEGER FileSize, BOOLEAN Dirty, const char *File, int Line);

#define MiFlushMappedSection(A,O,S,D) _MiFlushMappedSection(A,O,S,D,__FILE__,__LINE__)

NTSTATUS
NTAPI
MmCreateCacheSection
(PMM_CACHE_SECTION_SEGMENT *SegmentObject,
 ACCESS_MASK DesiredAccess,
 POBJECT_ATTRIBUTES ObjectAttributes,
 PLARGE_INTEGER UMaximumSize,
 ULONG SectionPageProtection,
 ULONG AllocationAttributes,
 PFILE_OBJECT FileObject);

PVOID
NTAPI
MmGetSegmentRmap(PFN_NUMBER Page, PULONG RawOffset);

NTSTATUS
NTAPI
MmNotPresentFaultCacheSection
(KPROCESSOR_MODE Mode,
 ULONG_PTR Address,
 BOOLEAN FromMdl);

ULONG
NTAPI
MiCacheEvictPages(PVOID BaseAddress, ULONG Target);
