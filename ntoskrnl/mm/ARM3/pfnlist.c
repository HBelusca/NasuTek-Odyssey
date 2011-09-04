/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/mm/ARM3/pfnlist.c
 * PURPOSE:         ARM Memory Manager PFN List Manipulation
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

#define MODULE_INVOLVED_IN_ARM3
#include "../ARM3/miarm.h"

#if DBG
#define ASSERT_LIST_INVARIANT(x) \
do { \
	ASSERT(((x)->Total == 0 && \
            (x)->Flink == LIST_HEAD && \
			(x)->Blink == LIST_HEAD) || \
		   ((x)->Total != 0 && \
			(x)->Flink != LIST_HEAD && \
			(x)->Blink != LIST_HEAD)); \
} while (0)
#else
#define ASSERT_LIST_INVARIANT(x)
#endif

/* GLOBALS ********************************************************************/

BOOLEAN MmDynamicPfn;
BOOLEAN MmMirroring;
ULONG MmSystemPageColor;

MMPFNLIST MmZeroedPageListHead = {0, ZeroedPageList, LIST_HEAD, LIST_HEAD};
MMPFNLIST MmFreePageListHead = {0, FreePageList, LIST_HEAD, LIST_HEAD};
MMPFNLIST MmStandbyPageListHead = {0, StandbyPageList, LIST_HEAD, LIST_HEAD};
MMPFNLIST MmModifiedPageListHead = {0, ModifiedPageList, LIST_HEAD, LIST_HEAD};
MMPFNLIST MmModifiedNoWritePageListHead = {0, ModifiedNoWritePageList, LIST_HEAD, LIST_HEAD};
MMPFNLIST MmBadPageListHead = {0, BadPageList, LIST_HEAD, LIST_HEAD};
MMPFNLIST MmRomPageListHead = {0, StandbyPageList, LIST_HEAD, LIST_HEAD};

PMMPFNLIST MmPageLocationList[] =
{
    &MmZeroedPageListHead,
    &MmFreePageListHead,
    &MmStandbyPageListHead,
    &MmModifiedPageListHead,
    &MmModifiedNoWritePageListHead,
    &MmBadPageListHead,
    NULL,
    NULL
};

ULONG MI_PFN_CURRENT_USAGE;
CHAR MI_PFN_CURRENT_PROCESS_NAME[16] = "None yet";

/* FUNCTIONS ******************************************************************/

VOID
NTAPI
MiZeroPhysicalPage(IN PFN_NUMBER PageFrameIndex)
{
    KIRQL OldIrql;
    PVOID VirtualAddress;
    PEPROCESS Process = PsGetCurrentProcess();

    /* Map in hyperspace, then wipe it using XMMI or MEMSET */
    VirtualAddress = MiMapPageInHyperSpace(Process, PageFrameIndex, &OldIrql);
    ASSERT(VirtualAddress);
    KeZeroPages(VirtualAddress, PAGE_SIZE);
    MiUnmapPageInHyperSpace(Process, VirtualAddress, OldIrql);
}

VOID
NTAPI
MiUnlinkFreeOrZeroedPage(IN PMMPFN Entry)
{
    PFN_NUMBER OldFlink, OldBlink;
    PMMPFNLIST ListHead;
    MMLISTS ListName;
    ULONG Color;
    PMMCOLOR_TABLES ColorTable;
    PMMPFN Pfn1;

    /* Make sure the PFN lock is held */
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    /* Make sure the PFN entry isn't in-use */
    ASSERT(Entry->u3.e1.WriteInProgress == 0);
    ASSERT(Entry->u3.e1.ReadInProgress == 0);

    /* Find the list for this entry, make sure it's the free or zero list */
    ListHead = MmPageLocationList[Entry->u3.e1.PageLocation];
    ListName = ListHead->ListName;
    ASSERT(ListHead != NULL);
    ASSERT(ListName <= FreePageList);
    ASSERT_LIST_INVARIANT(ListHead);

    /* Remove one count */
    ASSERT(ListHead->Total != 0);
    ListHead->Total--;

    /* Get the forward and back pointers */
    OldFlink = Entry->u1.Flink;
    OldBlink = Entry->u2.Blink;

    /* Check if the next entry is the list head */
    if (OldFlink != LIST_HEAD)
    {
        /* It is not, so set the backlink of the actual entry, to our backlink */
        MI_PFN_ELEMENT(OldFlink)->u2.Blink = OldBlink;
    }
    else
    {
        /* Set the list head's backlink instead */
        ListHead->Blink = OldBlink;
    }

    /* Check if the back entry is the list head */
    if (OldBlink != LIST_HEAD)
    {
        /* It is not, so set the backlink of the actual entry, to our backlink */
        MI_PFN_ELEMENT(OldBlink)->u1.Flink = OldFlink;
    }
    else
    {
        /* Set the list head's backlink instead */
        ListHead->Flink = OldFlink;
    }

    /* Get the page color */
    OldBlink = MiGetPfnEntryIndex(Entry);
    Color = OldBlink & MmSecondaryColorMask;

    /* Get the first page on the color list */
    ColorTable = &MmFreePagesByColor[ListName][Color];

    /* Check if this was was actually the head */
    OldFlink = ColorTable->Flink;
    if (OldFlink == OldBlink)
    {
        /* Make the table point to the next page this page was linking to */
        ColorTable->Flink = Entry->OriginalPte.u.Long;
        if (ColorTable->Flink != LIST_HEAD)
        {
            /* And make the previous link point to the head now */
            MI_PFN_ELEMENT(ColorTable->Flink)->u4.PteFrame = COLORED_LIST_HEAD;
        }
        else
        {
            /* And if that page was the head, loop the list back around */
            ColorTable->Blink = (PVOID)LIST_HEAD;
        }
    }
    else
    {
        /* This page shouldn't be pointing back to the head */
        ASSERT(Entry->u4.PteFrame != COLORED_LIST_HEAD);

        /* Make the back link point to whoever the next page is */
        Pfn1 = MI_PFN_ELEMENT(Entry->u4.PteFrame);
        Pfn1->OriginalPte.u.Long = Entry->OriginalPte.u.Long;

        /* Check if this page was pointing to the head */
        if (Entry->OriginalPte.u.Long != LIST_HEAD)
        {
            /* Make the back link point to the head */
            Pfn1 = MI_PFN_ELEMENT(Entry->OriginalPte.u.Long);
            Pfn1->u4.PteFrame = Entry->u4.PteFrame;
        }
        else
        {
            /* Then the table is directly back pointing to this page now */
            ColorTable->Blink = Pfn1;
        }
    }

    /* One less colored page */
    ASSERT(ColorTable->Count >= 1);
    ColorTable->Count--;

    /* Odyssey Hack */
    Entry->OriginalPte.u.Long = 0;

    /* We are not on a list anymore */
    Entry->u1.Flink = Entry->u2.Blink = 0;
    ASSERT_LIST_INVARIANT(ListHead);

    /* See if we hit any thresholds */
    if (MmAvailablePages == MmHighMemoryThreshold)
    {
        /* Clear the high memory event */
        KeClearEvent(MiHighMemoryEvent);
    }
    else if (MmAvailablePages == MmLowMemoryThreshold)
    {
        /* Signal the low memory event */
        KeSetEvent(MiLowMemoryEvent, 0, FALSE);
    }

    /* One less page */
    if (--MmAvailablePages < MmMinimumFreePages)
    {
        /* FIXME: Should wake up the MPW and working set manager, if we had one */
    }

#if MI_TRACE_PFNS
    ASSERT(MI_PFN_CURRENT_USAGE != MI_USAGE_NOT_SET);
    Entry->PfnUsage = MI_PFN_CURRENT_USAGE;
    memcpy(Entry->ProcessName, MI_PFN_CURRENT_PROCESS_NAME, 16);
//    MI_PFN_CURRENT_USAGE = MI_USAGE_NOT_SET;
//    memcpy(MI_PFN_CURRENT_PROCESS_NAME, "Not Set", 16);
#endif
}

PFN_NUMBER
NTAPI
MiRemovePageByColor(IN PFN_NUMBER PageIndex,
                    IN ULONG Color)
{
    PMMPFN Pfn1;
    PMMPFNLIST ListHead;
    MMLISTS ListName;
    PFN_NUMBER OldFlink, OldBlink;
    ULONG OldColor, OldCache;
    PMMCOLOR_TABLES ColorTable;

    /* Make sure PFN lock is held */
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(Color < MmSecondaryColors);

    /* Get the PFN entry */
    Pfn1 = MI_PFN_ELEMENT(PageIndex);
    ASSERT(Pfn1->u3.e1.RemovalRequested == 0);
    ASSERT(Pfn1->u3.e1.Rom == 0);

    /* Capture data for later */
    OldColor = Pfn1->u3.e1.PageColor;
    OldCache = Pfn1->u3.e1.CacheAttribute;

    /* Could be either on free or zero list */
    ListHead = MmPageLocationList[Pfn1->u3.e1.PageLocation];
    ASSERT_LIST_INVARIANT(ListHead);
    ListName = ListHead->ListName;
    ASSERT(ListName <= FreePageList);

    /* Remove a page */
    ListHead->Total--;

    /* Get the forward and back pointers */
    OldFlink = Pfn1->u1.Flink;
    OldBlink = Pfn1->u2.Blink;

    /* Check if the next entry is the list head */
    if (OldFlink != LIST_HEAD)
    {
        /* It is not, so set the backlink of the actual entry, to our backlink */
        MI_PFN_ELEMENT(OldFlink)->u2.Blink = OldBlink;
    }
    else
    {
        /* Set the list head's backlink instead */
        ListHead->Blink = OldBlink;
    }

    /* Check if the back entry is the list head */
    if (OldBlink != LIST_HEAD)
    {
        /* It is not, so set the backlink of the actual entry, to our backlink */
        MI_PFN_ELEMENT(OldBlink)->u1.Flink = OldFlink;
    }
    else
    {
        /* Set the list head's backlink instead */
        ListHead->Flink = OldFlink;
    }

    /* We are not on a list anymore */
	ASSERT_LIST_INVARIANT(ListHead);
    Pfn1->u1.Flink = Pfn1->u2.Blink = 0;

    /* Zero flags but restore color and cache */
    Pfn1->u3.e2.ShortFlags = 0;
    Pfn1->u3.e1.PageColor = OldColor;
    Pfn1->u3.e1.CacheAttribute = OldCache;

    /* Get the first page on the color list */
    ASSERT(Color < MmSecondaryColors);
    ColorTable = &MmFreePagesByColor[ListName][Color];
    ASSERT(ColorTable->Count >= 1);

    /* Set the forward link to whoever we were pointing to */
    ColorTable->Flink = Pfn1->OriginalPte.u.Long;

    /* Get the first page on the color list */
    if (ColorTable->Flink == LIST_HEAD)
    {
        /* This is the beginning of the list, so set the sentinel value */
        ColorTable->Blink = (PVOID)LIST_HEAD;
    }
    else
    {
        /* The list is empty, so we are the first page */
        MI_PFN_ELEMENT(ColorTable->Flink)->u4.PteFrame = COLORED_LIST_HEAD;
    }

    /* One less page */
    ColorTable->Count--;

    /* Odyssey Hack */
    Pfn1->OriginalPte.u.Long = 0;

    /* See if we hit any thresholds */
    if (MmAvailablePages == MmHighMemoryThreshold)
    {
        /* Clear the high memory event */
        KeClearEvent(MiHighMemoryEvent);
    }
    else if (MmAvailablePages == MmLowMemoryThreshold)
    {
        /* Signal the low memory event */
        KeSetEvent(MiLowMemoryEvent, 0, FALSE);
    }

    /* One less page */
    if (--MmAvailablePages < MmMinimumFreePages)
    {
        /* FIXME: Should wake up the MPW and working set manager, if we had one */
    }

#if MI_TRACE_PFNS
    //ASSERT(MI_PFN_CURRENT_USAGE != MI_USAGE_NOT_SET);
    Pfn1->PfnUsage = MI_PFN_CURRENT_USAGE;
    memcpy(Pfn1->ProcessName, MI_PFN_CURRENT_PROCESS_NAME, 16);
    //MI_PFN_CURRENT_USAGE = MI_USAGE_NOT_SET;
    //memcpy(MI_PFN_CURRENT_PROCESS_NAME, "Not Set", 16);
#endif

    /* Return the page */
    return PageIndex;
}

PFN_NUMBER
NTAPI
MiRemoveAnyPage(IN ULONG Color)
{
    PFN_NUMBER PageIndex;
    PMMPFN Pfn1;

    /* Make sure PFN lock is held and we have pages */
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(MmAvailablePages != 0);
    ASSERT(Color < MmSecondaryColors);

    /* Check the colored free list */
    PageIndex = MmFreePagesByColor[FreePageList][Color].Flink;
    if (PageIndex == LIST_HEAD)
    {
        /* Check the colored zero list */
        PageIndex = MmFreePagesByColor[ZeroedPageList][Color].Flink;
        if (PageIndex == LIST_HEAD)
        {
            /* Check the free list */
            ASSERT_LIST_INVARIANT(&MmFreePageListHead);
            PageIndex = MmFreePageListHead.Flink;
            Color = PageIndex & MmSecondaryColorMask;
            if (PageIndex == LIST_HEAD)
            {
                /* Check the zero list */
                ASSERT_LIST_INVARIANT(&MmZeroedPageListHead);
                PageIndex = MmZeroedPageListHead.Flink;
                Color = PageIndex & MmSecondaryColorMask;
                ASSERT(PageIndex != LIST_HEAD);
                if (PageIndex == LIST_HEAD)
                {
                    /* FIXME: Should check the standby list */
                    ASSERT(MmZeroedPageListHead.Total == 0);
                }
            }
        }
    }

    /* Remove the page from its list */
    PageIndex = MiRemovePageByColor(PageIndex, Color);

    /* Sanity checks */
    Pfn1 = MI_PFN_ELEMENT(PageIndex);
    ASSERT((Pfn1->u3.e1.PageLocation == FreePageList) ||
           (Pfn1->u3.e1.PageLocation == ZeroedPageList));
    ASSERT(Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT(Pfn1->u2.ShareCount == 0);
    ASSERT_LIST_INVARIANT(&MmFreePageListHead);
    ASSERT_LIST_INVARIANT(&MmZeroedPageListHead);

    /* Return the page */
    return PageIndex;
}

PFN_NUMBER
NTAPI
MiRemoveZeroPage(IN ULONG Color)
{
    PFN_NUMBER PageIndex;
    PMMPFN Pfn1;
    BOOLEAN Zero = FALSE;

    /* Make sure PFN lock is held and we have pages */
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(MmAvailablePages != 0);
    ASSERT(Color < MmSecondaryColors);

    /* Check the colored zero list */
    PageIndex = MmFreePagesByColor[ZeroedPageList][Color].Flink;
    if (PageIndex == LIST_HEAD)
    {
        /* Check the zero list */
        ASSERT_LIST_INVARIANT(&MmZeroedPageListHead);
        PageIndex = MmZeroedPageListHead.Flink;
        if (PageIndex == LIST_HEAD)
        {
            /* This means there's no zero pages, we have to look for free ones */
            ASSERT(MmZeroedPageListHead.Total == 0);
            Zero = TRUE;

            /* Check the colored free list */
            PageIndex = MmFreePagesByColor[FreePageList][Color].Flink;
            if (PageIndex == LIST_HEAD)
            {
                /* Check the free list */
                ASSERT_LIST_INVARIANT(&MmFreePageListHead);
                PageIndex = MmFreePageListHead.Flink;
                Color = PageIndex & MmSecondaryColorMask;
                ASSERT(PageIndex != LIST_HEAD);
                if (PageIndex == LIST_HEAD)
                {
                    /* FIXME: Should check the standby list */
                    ASSERT(MmZeroedPageListHead.Total == 0);
                }
            }
        }
        else
        {
            Color = PageIndex & MmSecondaryColorMask;
        }
    }

    /* Sanity checks */
    Pfn1 = MI_PFN_ELEMENT(PageIndex);
    ASSERT((Pfn1->u3.e1.PageLocation == FreePageList) ||
           (Pfn1->u3.e1.PageLocation == ZeroedPageList));

    /* Remove the page from its list */
    PageIndex = MiRemovePageByColor(PageIndex, Color);
    ASSERT(Pfn1 == MI_PFN_ELEMENT(PageIndex));

    /* Zero it, if needed */
    if (Zero) MiZeroPhysicalPage(PageIndex);

    /* Sanity checks */
    ASSERT(Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT(Pfn1->u2.ShareCount == 0);
    ASSERT_LIST_INVARIANT(&MmFreePageListHead);
    ASSERT_LIST_INVARIANT(&MmZeroedPageListHead);

    /* Return the page */
    return PageIndex;
}

VOID
NTAPI
MiInsertPageInFreeList(IN PFN_NUMBER PageFrameIndex)
{
    PMMPFNLIST ListHead;
    PFN_NUMBER LastPage;
    PMMPFN Pfn1;
    ULONG Color;
    PMMPFN Blink;
    PMMCOLOR_TABLES ColorTable;

    /* Make sure the page index is valid */
    ASSERT(KeGetCurrentIrql() >= DISPATCH_LEVEL);
    ASSERT((PageFrameIndex != 0) &&
           (PageFrameIndex <= MmHighestPhysicalPage) &&
           (PageFrameIndex >= MmLowestPhysicalPage));

    /* Get the PFN entry */
    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

    /* Sanity checks that a right kind of page is being inserted here */
    ASSERT(Pfn1->u4.MustBeCached == 0);
    ASSERT(Pfn1->u3.e1.Rom != 1);
    ASSERT(Pfn1->u3.e1.RemovalRequested == 0);
    ASSERT(Pfn1->u4.VerifierAllocation == 0);
    ASSERT(Pfn1->u3.e2.ReferenceCount == 0);

    /* Get the free page list and increment its count */
    ListHead = &MmFreePageListHead;
    ASSERT_LIST_INVARIANT(ListHead);
    ListHead->Total++;

    /* Get the last page on the list */
    LastPage = ListHead->Blink;
    if (LastPage != LIST_HEAD)
    {
        /* Link us with the previous page, so we're at the end now */
        MI_PFN_ELEMENT(LastPage)->u1.Flink = PageFrameIndex;
    }
    else
    {
        /* The list is empty, so we are the first page */
        ListHead->Flink = PageFrameIndex;
    }

    /* Now make the list head point back to us (since we go at the end) */
    ListHead->Blink = PageFrameIndex;
    ASSERT_LIST_INVARIANT(ListHead);

    /* And initialize our own list pointers */
    Pfn1->u1.Flink = LIST_HEAD;
    Pfn1->u2.Blink = LastPage;

    /* Set the list name and default priority */
    Pfn1->u3.e1.PageLocation = FreePageList;
    Pfn1->u4.Priority = 3;

    /* Clear some status fields */
    Pfn1->u4.InPageError = 0;
    Pfn1->u4.AweAllocation = 0;

    /* Increase available pages */
    MmAvailablePages++;

    /* Check if we've reached the configured low memory threshold */
    if (MmAvailablePages == MmLowMemoryThreshold)
    {
        /* Clear the event, because now we're ABOVE the threshold */
        KeClearEvent(MiLowMemoryEvent);
    }
    else if (MmAvailablePages == MmHighMemoryThreshold)
    {
        /* Otherwise check if we reached the high threshold and signal the event */
        KeSetEvent(MiHighMemoryEvent, 0, FALSE);
    }

    /* Get the page color */
    Color = PageFrameIndex & MmSecondaryColorMask;

    /* Get the first page on the color list */
    ColorTable = &MmFreePagesByColor[FreePageList][Color];
    if (ColorTable->Flink == LIST_HEAD)
    {
        /* The list is empty, so we are the first page */
        Pfn1->u4.PteFrame = COLORED_LIST_HEAD;
        ColorTable->Flink = PageFrameIndex;
    }
    else
    {
        /* Get the previous page */
        Blink = (PMMPFN)ColorTable->Blink;

        /* Make it link to us, and link back to it */
        Blink->OriginalPte.u.Long = PageFrameIndex;
        Pfn1->u4.PteFrame = MiGetPfnEntryIndex(Blink);
    }

    /* Now initialize our own list pointers */
    ColorTable->Blink = Pfn1;

    /* This page is now the last */
    Pfn1->OriginalPte.u.Long = LIST_HEAD;

    /* And increase the count in the colored list */
    ColorTable->Count++;

    /* Notify zero page thread if enough pages are on the free list now */
    if ((ListHead->Total >= 8) && !(MmZeroingPageThreadActive))
    {
        /* Set the event */
        MmZeroingPageThreadActive = TRUE;
        KeSetEvent(&MmZeroingPageEvent, IO_NO_INCREMENT, FALSE);
    }

#if MI_TRACE_PFNS
    Pfn1->PfnUsage = MI_USAGE_FREE_PAGE;
    RtlZeroMemory(Pfn1->ProcessName, 16);
#endif
}

/* Note: This function is hardcoded only for the zeroed page list, for now */
VOID
NTAPI
MiInsertPageInList(IN PMMPFNLIST ListHead,
                   IN PFN_NUMBER PageFrameIndex)
{
    PFN_NUMBER Flink;
    PMMPFN Pfn1, Pfn2;
    MMLISTS ListName;
    PMMCOLOR_TABLES ColorHead;
    ULONG Color;

    /* For free pages, use MiInsertPageInFreeList */
    ASSERT(ListHead != &MmFreePageListHead);

    /* Make sure the lock is held */
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    /* Make sure the PFN is valid */
    ASSERT((PageFrameIndex) &&
           (PageFrameIndex <= MmHighestPhysicalPage) &&
           (PageFrameIndex >= MmLowestPhysicalPage));

    /* Page should be unused */
    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
    ASSERT(Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT(Pfn1->u3.e1.Rom != 1);

    /* Only used for zero pages in Odyssey */
    ListName = ListHead->ListName;
    ASSERT(ListName == ZeroedPageList);
    ListHead->Total++;

    /* Don't handle bad pages yet yet */
    ASSERT(Pfn1->u3.e1.RemovalRequested == 0);

    /* Make the head of the list point to this page now */
    Flink = ListHead->Flink;
    ListHead->Flink = PageFrameIndex;

    /* Make the page point to the previous head, and back to the list */
    Pfn1->u1.Flink = Flink;
    Pfn1->u2.Blink = LIST_HEAD;

    /* Was the list empty? */
    if (Flink != LIST_HEAD)
    {
        /* It wasn't, so update the backlink of the previous head page */
        Pfn2 = MI_PFN_ELEMENT(Flink);
        Pfn2->u2.Blink = PageFrameIndex;
    }
    else
    {
        /* It was empty, so have it loop back around to this new page */
        ListHead->Blink = PageFrameIndex;
    }

    /* Move the page onto its new location */
    Pfn1->u3.e1.PageLocation = ListName;

    /* One more page on the system */
    MmAvailablePages++;

    /* Check if we've reached the configured low memory threshold */
    if (MmAvailablePages == MmLowMemoryThreshold)
    {
        /* Clear the event, because now we're ABOVE the threshold */
        KeClearEvent(MiLowMemoryEvent);
    }
    else if (MmAvailablePages == MmHighMemoryThreshold)
    {
        /* Otherwise check if we reached the high threshold and signal the event */
        KeSetEvent(MiHighMemoryEvent, 0, FALSE);
    }

    /* Sanity checks */
    ASSERT(ListName == ZeroedPageList);
    ASSERT(Pfn1->u4.InPageError == 0);

    /* Get the page color */
    Color = PageFrameIndex & MmSecondaryColorMask;

    /* Get the list for this color */
    ColorHead = &MmFreePagesByColor[ZeroedPageList][Color];

    /* Get the old head */
    Flink = ColorHead->Flink;

    /* Make this page point back to the list, and point forwards to the old head */
    Pfn1->OriginalPte.u.Long = Flink;
    Pfn1->u4.PteFrame = COLORED_LIST_HEAD;

    /* Set the new head */
    ColorHead->Flink = PageFrameIndex;

    /* Was the head empty? */
    if (Flink != LIST_HEAD)
    {
        /* No, so make the old head point to this page */
        Pfn2 = MI_PFN_ELEMENT(Flink);
        Pfn2->u4.PteFrame = PageFrameIndex;
    }
    else
    {
        /* Yes, make it loop back to this page */
        ColorHead->Blink = (PVOID)Pfn1;
    }

    /* One more paged on the colored list */
    ColorHead->Count++;

#if MI_TRACE_PFNS
    //ASSERT(MI_PFN_CURRENT_USAGE == MI_USAGE_NOT_SET);
    Pfn1->PfnUsage = MI_USAGE_FREE_PAGE;
    MI_PFN_CURRENT_USAGE = MI_USAGE_NOT_SET;
    RtlZeroMemory(Pfn1->ProcessName, 16);
#endif
}

VOID
NTAPI
MiInitializePfn(IN PFN_NUMBER PageFrameIndex,
                IN PMMPTE PointerPte,
                IN BOOLEAN Modified)
{
    PMMPFN Pfn1;
    NTSTATUS Status;
    PMMPTE PointerPtePte;
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    /* Setup the PTE */
    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

    /* Check if this PFN is part of a valid address space */
    if (PointerPte->u.Hard.Valid == 1)
    {
        /* Only valid from MmCreateProcessAddressSpace path */
        ASSERT(PsGetCurrentProcess()->Vm.WorkingSetSize == 0);

        /* Make this a demand zero PTE */
        MI_MAKE_SOFTWARE_PTE(&Pfn1->OriginalPte, MM_READWRITE);
    }
    else
    {
        /* Copy the PTE data */
        Pfn1->OriginalPte = *PointerPte;
        ASSERT(!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                 (Pfn1->OriginalPte.u.Soft.Transition == 1)));
    }

    /* Otherwise this is a fresh page -- set it up */
    ASSERT(Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount = 1;
    Pfn1->u2.ShareCount = 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;
    ASSERT(Pfn1->u3.e1.Rom == 0);
    Pfn1->u3.e1.Modified = Modified;

    /* Get the page table for the PTE */
    PointerPtePte = MiAddressToPte(PointerPte);
    if (PointerPtePte->u.Hard.Valid == 0)
    {
        /* Make sure the PDE gets paged in properly */
        Status = MiCheckPdeForPagedPool(PointerPte);
        if (!NT_SUCCESS(Status))
        {
            /* Crash */
            KeBugCheckEx(MEMORY_MANAGEMENT,
                         0x61940,
                         (ULONG_PTR)PointerPte,
                         (ULONG_PTR)PointerPtePte->u.Long,
                         (ULONG_PTR)MiPteToAddress(PointerPte));
        }
    }

    /* Get the PFN for the page table */
    PageFrameIndex = PFN_FROM_PTE(PointerPtePte);
    ASSERT(PageFrameIndex != 0);
    Pfn1->u4.PteFrame = PageFrameIndex;

    /* Increase its share count so we don't get rid of it */
    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
    Pfn1->u2.ShareCount++;
}

PFN_NUMBER
NTAPI
MiAllocatePfn(IN PMMPTE PointerPte,
              IN ULONG Protection)
{
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    MMPTE TempPte;

    /* Sanity check that we aren't passed a valid PTE */
    ASSERT(PointerPte->u.Hard.Valid == 0);

    /* Make an empty software PTE */
    MI_MAKE_SOFTWARE_PTE(&TempPte, MM_READWRITE);

    /* Lock the PFN database */
    OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock);

    /* Check if we're running low on pages */
    if (MmAvailablePages < 128)
    {
        DPRINT1("Warning, running low on memory: %d pages left\n", MmAvailablePages);
        //MiEnsureAvailablePageOrWait(NULL, OldIrql);
    }

    /* Grab a page */
    ASSERT_LIST_INVARIANT(&MmFreePageListHead);
    ASSERT_LIST_INVARIANT(&MmZeroedPageListHead);
    PageFrameIndex = MiRemoveAnyPage(MI_GET_NEXT_COLOR());

    /* Write the software PTE */
    MI_WRITE_INVALID_PTE(PointerPte, TempPte);
    PointerPte->u.Soft.Protection |= Protection;

    /* Initialize its PFN entry */
    MiInitializePfn(PageFrameIndex, PointerPte, TRUE);

    /* Release the PFN lock and return the page */
    ASSERT_LIST_INVARIANT(&MmFreePageListHead);
    ASSERT_LIST_INVARIANT(&MmZeroedPageListHead);
    KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql);
    return PageFrameIndex;
}

VOID
NTAPI
MiDecrementShareCount(IN PMMPFN Pfn1,
                      IN PFN_NUMBER PageFrameIndex)
{
    ASSERT(PageFrameIndex > 0);
    ASSERT(MI_PFN_ELEMENT(PageFrameIndex) != NULL);
    ASSERT(Pfn1 == MI_PFN_ELEMENT(PageFrameIndex));
    ASSERT(MI_IS_ROS_PFN(Pfn1) == FALSE);

    /* Page must be in-use */
    if ((Pfn1->u3.e1.PageLocation != ActiveAndValid) &&
        (Pfn1->u3.e1.PageLocation != StandbyPageList))
    {
        /* Otherwise we have PFN corruption */
        KeBugCheckEx(PFN_LIST_CORRUPT,
                     0x99,
                     PageFrameIndex,
                     Pfn1->u3.e1.PageLocation,
                     0);
    }

    /* Check if the share count is now 0 */
    ASSERT(Pfn1->u2.ShareCount < 0xF000000);
    if (!--Pfn1->u2.ShareCount)
    {
        /* Odyssey does not handle these */
        ASSERT(Pfn1->u3.e1.PrototypePte == 0);

        /* Put the page in transition */
        Pfn1->u3.e1.PageLocation = TransitionPage;

        /* PFN lock must be held */
        ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

        /* Page should at least have one reference */
        ASSERT(Pfn1->u3.e2.ReferenceCount != 0);
        if (Pfn1->u3.e2.ReferenceCount == 1)
        {
            /* In Odyssey, this path should always be hit with a deleted PFN */
            ASSERT(MI_IS_PFN_DELETED(Pfn1) == TRUE);

            /* Clear the last reference */
            Pfn1->u3.e2.ReferenceCount = 0;
            ASSERT(Pfn1->OriginalPte.u.Soft.Prototype == 0);

            /* Mark the page temporarily as valid, we're going to make it free soon */
            Pfn1->u3.e1.PageLocation = ActiveAndValid;

            /* Bring it back into the free list */
            MiInsertPageInFreeList(PageFrameIndex);
        }
        else
        {
            /* Otherwise, just drop the reference count */
            InterlockedDecrement16((PSHORT)&Pfn1->u3.e2.ReferenceCount);
        }
    }
}

VOID
NTAPI
MiDecrementReferenceCount(IN PMMPFN Pfn1,
                          IN PFN_NUMBER PageFrameIndex)
{
    /* PFN lock must be held */
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    /* Sanity checks on the page */
    ASSERT(PageFrameIndex < MmHighestPhysicalPage);
    ASSERT(Pfn1 == MI_PFN_ELEMENT(PageFrameIndex));
    ASSERT(Pfn1->u3.e2.ReferenceCount != 0);

    /* Dereference the page, bail out if it's still alive */
    InterlockedDecrement16((PSHORT)&Pfn1->u3.e2.ReferenceCount);
    if (Pfn1->u3.e2.ReferenceCount) return;

    /* Nobody should still have reference to this page */
    if (Pfn1->u2.ShareCount != 0)
    {
        /* Otherwise something's really wrong */
        KeBugCheckEx(PFN_LIST_CORRUPT, 7, PageFrameIndex, Pfn1->u2.ShareCount, 0);
    }

    /* And it should be lying on some page list */
    ASSERT(Pfn1->u3.e1.PageLocation != ActiveAndValid);

    /* Did someone set the delete flag? */
    if (MI_IS_PFN_DELETED(Pfn1))
    {
        /* Insert it into the free list, there's nothing left to do */
        MiInsertPageInFreeList(PageFrameIndex);
        return;
    }

    /* We don't have a modified list yet */
    ASSERT(Pfn1->u3.e1.Modified == 0);
    ASSERT(Pfn1->u3.e1.RemovalRequested == 0);

    /* FIXME: Normally it would go on the standby list, but we're pushing it on the free list */
    MiInsertPageInFreeList(PageFrameIndex);
}

VOID
NTAPI
MiInitializePfnForOtherProcess(IN PFN_NUMBER PageFrameIndex,
                               IN PMMPTE PointerPte,
                               IN PFN_NUMBER PteFrame)
{
    PMMPFN Pfn1;

    /* Setup the PTE */
    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

    /* Make this a software PTE */
    MI_MAKE_SOFTWARE_PTE(&Pfn1->OriginalPte, MM_READWRITE);

    /* Setup the page */
    ASSERT(Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount = 1;
    Pfn1->u2.ShareCount = 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;
    Pfn1->u3.e1.Modified = TRUE;
    Pfn1->u4.InPageError = FALSE;

    /* Did we get a PFN for the page table */
    if (PteFrame)
    {
        /* Store it */
        Pfn1->u4.PteFrame = PteFrame;

        /* Increase its share count so we don't get rid of it */
        Pfn1 = MI_PFN_ELEMENT(PteFrame);
        Pfn1->u2.ShareCount++;
    }
}

/* EOF */
