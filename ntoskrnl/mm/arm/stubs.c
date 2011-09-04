/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/mm/arm/stubs.c
 * PURPOSE:         ARM Memory Manager
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

ULONG MmGlobalKernelPageDirectory[1024];
MMPTE MiArmTemplatePte;
MMPDE_HARDWARE MiArmTemplatePde;

/* PRIVATE FUNCTIONS **********************************************************/

BOOLEAN
NTAPI
MiUnmapPageTable(IN PMMPTE PointerPde)
{
    //
    // Check if this address belongs to the kernel
    //
    if (((ULONG_PTR)PointerPde > PDE_BASE) ||
        ((ULONG_PTR)PointerPde < (PDE_BASE + 1024*1024)))
    {
        //
        // Nothing to do
        //
        return TRUE;
    }
    
    //
    // FIXME-USER: Shouldn't get here yet
    //
    ASSERT(FALSE);
    return FALSE;
}

VOID
NTAPI
MiFlushTlb(IN PMMPTE PointerPte,
           IN PVOID Address)
{
    //
    // Make sure the PTE is valid, and unmap the pagetable if user-mode
    //
    if (((PointerPte) && (MiUnmapPageTable(PointerPte))) ||
        (Address >= MmSystemRangeStart))
    {
        //
        // Invalidate this page
        //
        KeArmInvalidateTlbEntry(Address);
    }
}

PMMPTE
NTAPI
MiGetPageTableForProcess(IN PEPROCESS Process,
                         IN PVOID Address,
                         IN BOOLEAN Create)
{
    //ULONG PdeOffset;
    PMMPTE PointerPte;
    PMMPDE_HARDWARE PointerPde;
    MMPDE_HARDWARE TempPde;
    MMPTE TempPte;
    NTSTATUS Status;
    PFN_NUMBER Pfn;
    
    //
    // Check if this is a user-mode, non-kernel or non-current address
    //
    if ((Address < MmSystemRangeStart) &&
        (Process) &&
        (Process != PsGetCurrentProcess()))
    {
        //
        // FIXME-USER: No user-mode memory support
        //
        ASSERT(FALSE);
    }
    
    //
    // Get our templates
    //
    TempPde = MiArmTemplatePde;
    TempPte = MiArmTemplatePte;
    
    //
    // Get the PDE
    //
    PointerPde = MiGetPdeAddress(Address);
    if (PointerPde->u.Hard.Coarse.Valid)
    {
        //
        // Invalid PDE, is this a kernel address?
        //
        if (Address >= MmSystemRangeStart)
        {            
            //
            // Does it exist in the kernel page directory?
            //
            //PdeOffset = MiGetPdeOffset(Address);
            //if (MmGlobalKernelPageDirectory[PdeOffset] == 0)
            {
                //
                // It doesn't. Is this a create operation? If not, fail
                //
                if (Create == FALSE) return NULL;
            kernelHack:
                DPRINT1("Must create a page for: %p PDE: %p\n", // Offset: %lx!\n",
                        Address, PointerPde);//, PdeOffset);
                
                //
                // Allocate a non paged pool page for the PDE
                //
                Status = MmRequestPageMemoryConsumer(MC_NPPOOL, FALSE, &Pfn);
                if (!NT_SUCCESS(Status)) return NULL;
                
                //
                // Setup the PFN
                //
                TempPde.u.Hard.Coarse.PageFrameNumber = (Pfn << PAGE_SHIFT) >> CPT_SHIFT;
                
                //
                // Write the PDE
                //
                ASSERT(PointerPde->u.Hard.Coarse.Valid == 0);
                ASSERT(TempPde.u.Hard.Coarse.Valid == 1);
                *PointerPde = TempPde;
                
                //
                // Save it
                //
                //MmGlobalKernelPageDirectory[PdeOffset] = TempPde.u.Hard.AsUlong;
                //DPRINT1("KPD: %p PDEADDR: %p\n", &MmGlobalKernelPageDirectory[PdeOffset], MiGetPdeAddress(Address));
                
                //
                // FIXFIX: Double check with Felix tomorrow
                //
/////
                //
                // Get the PTE for this 1MB region
                //
                PointerPte = MiGetPteAddress(MiGetPteAddress(Address));
                DPRINT1("PointerPte: %p\n", PointerPte);
                
                //
                // Write the PFN of the PDE
                //
                TempPte.u.Hard.PageFrameNumber = Pfn;
                
                //
                // Write the PTE
                //
                ASSERT(PointerPte->u.Hard.Valid == 0);
                ASSERT(TempPte.u.Hard.Valid == 1);
                *PointerPte = TempPte;
/////
            }
            
            //
            // Now set the actual PDE
            //
            //PointerPde = (PMMPTE)&MmGlobalKernelPageDirectory[PdeOffset];
        }
        else
        {
            //
            // Is this a create operation? If not, fail
            //
            if (Create == FALSE) return NULL;
            
            //
            // THIS WHOLE PATH IS TODO
            //
            goto kernelHack;
            ASSERT(FALSE);
         
            //
            // Allocate a non paged pool page for the PDE
            //
            Status = MmRequestPageMemoryConsumer(MC_NPPOOL, FALSE, &Pfn);
            if (!NT_SUCCESS(Status)) return NULL;
            
            //
            // Make the entry valid
            //
            TempPde.u.Hard.AsUlong = 0xDEADBEEF;
            
            //
            // Set it
            //
            *PointerPde = TempPde;
        }
    }
    
    //
    // Return the PTE
    //
    return MiGetPteAddress(Address);
}

MMPTE
NTAPI
MiGetPageEntryForProcess(IN PEPROCESS Process,
                         IN PVOID Address)
{
    PMMPTE PointerPte;
    MMPTE Pte;
    Pte.u.Hard.AsUlong = 0;
    
    //
    // Get the PTE
    //
    PointerPte = MiGetPageTableForProcess(Process, Address, FALSE);
    if (PointerPte)
    {
        //
        // Capture the PTE value and unmap the page table
        //
        Pte = *PointerPte;
        MiUnmapPageTable(PointerPte);
    }

    //
    // Return the PTE value
    //
    return Pte;
}

VOID
NTAPI
MmDeletePageTable(IN PEPROCESS Process,
                  IN PVOID Address)
{
    PMMPDE_HARDWARE PointerPde;
    
    //
    // Not valid for kernel addresses
    //
    DPRINT("MmDeletePageTable(%p, %p)\n", Process, Address);
    ASSERT(Address < MmSystemRangeStart);
    
    //
    // Check if this is for a different process
    //
    if ((Process) && (Process != PsGetCurrentProcess()))
    {
        //
        // FIXME-USER: Need to attach to the process
        //
        ASSERT(FALSE);
    }
    
    //
    // Get the PDE
    //
    PointerPde = MiGetPdeAddress(Address);
    
    //
    // On ARM, we use a section mapping for the original low-memory mapping
    //
    if ((Address) || (PointerPde->u.Hard.Section.Valid == 0))
    {
        //
        // Make sure it's valid
        //
        ASSERT(PointerPde->u.Hard.Coarse.Valid == 1);
    }
    
    //
    // Clear the PDE
    //
    PointerPde->u.Hard.AsUlong = 0;
    ASSERT(PointerPde->u.Hard.Coarse.Valid == 0);
    
    //
    // Invalidate the TLB entry
    //
    MiFlushTlb((PMMPTE)PointerPde, MiGetPteAddress(Address));
}

BOOLEAN
NTAPI
MmCreateProcessAddressSpace(IN ULONG MinWs,
                            IN PEPROCESS Process,
                            IN PULONG DirectoryTableBase)
{
    NTSTATUS Status;
    ULONG i;
    PFN_NUMBER Pfn[2];
    PMMPDE_HARDWARE PageDirectory, PointerPde;
    MMPDE_HARDWARE TempPde;
    ASSERT(FALSE);
    
    //
    // Loop two tables (Hyperspace and TTB). Each one is 16KB
    //
    //
    for (i = 0; i < 8; i++)
    {
        //
        // Allocate a page
        //
        Status = MmRequestPageMemoryConsumer(MC_NPPOOL, FALSE, &Pfn[i]);
        if (!NT_SUCCESS(Status)) ASSERT(FALSE);
    }
    
    //
    // Map the base
    //
    PageDirectory = MmCreateHyperspaceMapping(Pfn[0]);
    
    //
    // Copy the PDEs for kernel-mode
    //
    RtlCopyMemory(PageDirectory + MiGetPdeOffset(MmSystemRangeStart),
                  MmGlobalKernelPageDirectory + MiGetPdeOffset(MmSystemRangeStart),
                  (1024 - MiGetPdeOffset(MmSystemRangeStart)) * sizeof(ULONG));
    
    
    //
    // Setup the PDE for the table base
    //
    TempPde = MiArmTemplatePde;
    TempPde.u.Hard.Coarse.PageFrameNumber = (Pfn[0] << PAGE_SHIFT) >> CPT_SHIFT;
    PointerPde = &PageDirectory[MiGetPdeOffset(PTE_BASE)];
    
    //
    // Write the PDE
    //
    ASSERT(PointerPde->u.Hard.Coarse.Valid == 0);
    ASSERT(TempPde.u.Hard.Coarse.Valid == 1);
    *PointerPde = TempPde;
    
    //
    // Setup the PDE for the hyperspace
    //
    TempPde.u.Hard.Coarse.PageFrameNumber = (Pfn[1] << PAGE_SHIFT) >> CPT_SHIFT;
    PointerPde = &PageDirectory[MiGetPdeOffset(HYPER_SPACE)];
    
    //
    // Write the PDE
    //
    ASSERT(PointerPde->u.Hard.Coarse.Valid == 0);
    ASSERT(TempPde.u.Hard.Coarse.Valid == 1);
    *PointerPde = TempPde;

    //
    // Unmap the page directory
    //
    MmDeleteHyperspaceMapping(PageDirectory);
    
    //
    // Return the page table base
    //
    DirectoryTableBase[0] = Pfn[0] << PAGE_SHIFT;
    return TRUE;
}

NTSTATUS
NTAPI
Mmi386ReleaseMmInfo(IN PEPROCESS Process)
{
    //
    // FIXME-USER: Need to delete address space
    //
    UNIMPLEMENTED;
    while (TRUE);
    return 0;
}

PULONG
NTAPI
MmGetPageDirectory(VOID)
{
    //
    // Return the TTB
    //
    return (PULONG)KeArmTranslationTableRegisterGet().AsUlong;
}

VOID
NTAPI
MmDisableVirtualMapping(IN PEPROCESS Process,
                        IN PVOID Address,
                        OUT PBOOLEAN WasDirty,
                        OUT PPFN_NUMBER Page)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
}

VOID
NTAPI
MmEnableVirtualMapping(IN PEPROCESS Process,
                       IN PVOID Address)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
}

NTSTATUS
NTAPI
MmCreateVirtualMappingInternal(IN PEPROCESS Process,
                               IN PVOID Address,
                               IN ULONG Protection,
                               IN PPFN_NUMBER Pages,
                               IN ULONG PageCount,
                               IN BOOLEAN MarkAsMapped)
{
    PMMPTE PointerPte = NULL;
    MMPTE TempPte;
    PVOID Addr;
    ULONG OldPdeOffset, PdeOffset, i;
    DPRINT("[KMAP]: %p %d\n", Address, PageCount);
    //ASSERT(Address >= MmSystemRangeStart);
    
    //
    // Get our template PTE
    //
    TempPte = MiArmTemplatePte;
    
    //
    // Loop every page
    //
    Addr = Address;
    OldPdeOffset = MiGetPdeOffset(Addr) + 1;
    for (i = 0; i < PageCount; i++)
    {
        //
        // Get the next PDE offset and check if it's a new one
        //
        PdeOffset = MiGetPdeOffset(Addr);
        if (OldPdeOffset != PdeOffset)
        {
            //
            // Get rid of the old L2 Table, if this was the last PTE on it
            //
            MiUnmapPageTable(PointerPte);
            
            //
            // Get the PTE for this address, and create the PDE for it
            //
            PointerPte = MiGetPageTableForProcess(NULL, Addr, TRUE);
            ASSERT(PointerPte);
        }
        else
        {
            //
            // Go to the next PTE on this PDE
            //
            ASSERT(PointerPte);
            PointerPte++;
        }
        
        //
        // Save the current PDE
        //
        OldPdeOffset = PdeOffset;
        
        //
        // Set the PFN
        //
        TempPte.u.Hard.PageFrameNumber = *Pages++;
        
        //
        // Write the PTE
        //
        ASSERT(PointerPte->u.Hard.Valid == 0);
        ASSERT(TempPte.u.Hard.Valid == 1);
        *PointerPte = TempPte;
        
        //
        // Move to the next page
        //
        Addr = (PVOID)((ULONG_PTR)Addr + PAGE_SIZE);
    }
    
    //
    // All done
    //
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
MmCreateVirtualMappingForKernel(IN PVOID Address,
                                IN ULONG Protection,
                                IN PPFN_NUMBER Pages,
                                IN ULONG PageCount)
{
    //
    // Call the internal version
    //
    return MmCreateVirtualMappingInternal(NULL,
                                          Address,
                                          Protection,
                                          Pages,
                                          PageCount,
                                          FALSE);
}

NTSTATUS
NTAPI
MmCreateVirtualMappingUnsafe(IN PEPROCESS Process,
                             IN PVOID Address,
                             IN ULONG Protection,
                             IN PPFN_NUMBER Pages,
                             IN ULONG PageCount)
{
    //
    // Are we only handling the kernel?
    //
    if (!(Process) || (Process == PsGetCurrentProcess()))
    {
        //
        // Call the internal version
        //
        return MmCreateVirtualMappingInternal(Process,
                                              Address,
                                              Protection,
                                              Pages,
                                              PageCount,
                                              TRUE);
    }
    
    //
    // FIXME-USER: Support user-mode mappings
    //
    ASSERT(FALSE);
    return 0;
}

NTSTATUS
NTAPI
MmCreateVirtualMapping(IN PEPROCESS Process,
                       IN PVOID Address,
                       IN ULONG Protection,
                       IN PPFN_NUMBER Pages,
                       IN ULONG PageCount)
{
    ULONG i;
    
    //
    // Loop each page
    //
    for (i = 0; i < PageCount; i++)
    {
        //
        // Make sure the page is marked as in use
        //
        ASSERT(MmIsPageInUse(Pages[i]));
    }
    
    //
    // Call the unsafe version
    //
    return MmCreateVirtualMappingUnsafe(Process,
                                        Address,
                                        Protection,
                                        Pages,
                                        PageCount);
}

VOID
NTAPI
MmRawDeleteVirtualMapping(IN PVOID Address)
{
    PMMPTE PointerPte;
    
    //
    // Get the PTE
    //
    PointerPte = MiGetPageTableForProcess(NULL, Address, FALSE);
    if ((PointerPte) && (PointerPte->u.Hard.Valid))
    {
        //
        // Destroy it
        //
        PointerPte->u.Hard.AsUlong = 0;
    
        //
        // Flush the TLB
        //
        MiFlushTlb(PointerPte, Address);
    }
}

VOID
NTAPI
MmDeleteVirtualMapping(IN PEPROCESS Process,
                       IN PVOID Address,
                       IN BOOLEAN FreePage,
                       OUT PBOOLEAN WasDirty,
                       OUT PPFN_NUMBER Page)
{
    PMMPTE PointerPte;
    MMPTE Pte;
    PFN_NUMBER Pfn = 0;
    
    //
    // Get the PTE
    //
    PointerPte = MiGetPageTableForProcess(NULL, Address, FALSE);
    if (PointerPte)
    {       
        //
        // Save and destroy the PTE
        //
        Pte = *PointerPte;
        PointerPte->u.Hard.AsUlong = 0;
        
        //
        // Flush the TLB
        //
        MiFlushTlb(PointerPte, Address);
        
        //
        // Unmap the PFN
        //
        Pfn = Pte.u.Hard.PageFrameNumber;
        
        //
        // Release the PFN if it was ours
        //
        if ((FreePage) && (Pfn)) MmReleasePageMemoryConsumer(MC_NPPOOL, Pfn);
    }
    
    //
    // Return if the page was dirty
    //
    if (WasDirty) *WasDirty = FALSE; // LIE!!!
    if (Page) *Page = Pfn;
}

VOID
NTAPI
MmDeletePageFileMapping(IN PEPROCESS Process,
                        IN PVOID Address,
                        IN SWAPENTRY *SwapEntry)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
}

NTSTATUS
NTAPI
MmCreatePageFileMapping(IN PEPROCESS Process,
                        IN PVOID Address,
                        IN SWAPENTRY SwapEntry)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
    return 0;
}

PFN_NUMBER
NTAPI
MmGetPfnForProcess(IN PEPROCESS Process,
                   IN PVOID Address)
{
    MMPTE Pte;
    
    //
    // Get the PTE
    //
    Pte = MiGetPageEntryForProcess(Process, Address);
    if (Pte.u.Hard.Valid == 0) return 0;
    
    //
    // Return PFN
    //
    return Pte.u.Hard.PageFrameNumber;
}

BOOLEAN
NTAPI
MmIsDirtyPage(IN PEPROCESS Process,
              IN PVOID Address)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
    return 0;
}

VOID
NTAPI
MmSetCleanPage(IN PEPROCESS Process,
               IN PVOID Address)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
}

VOID
NTAPI
MmSetDirtyPage(IN PEPROCESS Process,
               IN PVOID Address)
{
    //
    // TODO
    //
    UNIMPLEMENTED;
    while (TRUE);
}

BOOLEAN
NTAPI
MmIsPagePresent(IN PEPROCESS Process,
                IN PVOID Address)
{
    //
    // Fault PTEs are 0, which is FALSE (non-present)
    //
    return MiGetPageEntryForProcess(Process, Address).u.Hard.Valid;
}

BOOLEAN
NTAPI
MmIsPageSwapEntry(IN PEPROCESS Process,
                  IN PVOID Address)
{
    MMPTE Pte;
    
    //
    // Get the PTE
    //
    Pte = MiGetPageEntryForProcess(Process, Address);
    
    //
    // Make sure it exists, but is faulting
    //
    return (Pte.u.Hard.Valid == 0) && (Pte.u.Hard.AsUlong);
}

ULONG
NTAPI
MmGetPageProtect(IN PEPROCESS Process,
                 IN PVOID Address)
{
    //
    // We don't enforce any protection on the pages -- they are all RWX
    //
    return PAGE_READWRITE;
}

VOID
NTAPI
MmSetPageProtect(IN PEPROCESS Process,
                 IN PVOID Address,
                 IN ULONG Protection)
{
    //
    // We don't enforce any protection on the pages -- they are all RWX
    //
    return;
}

VOID
NTAPI
MmInitGlobalKernelPageDirectory(VOID)
{
    ULONG i;
    PULONG CurrentPageDirectory = (PULONG)PDE_BASE;
    
    //
    // Good place to setup template PTE/PDEs.
    // We are lazy and pick a known-good PTE
    //
    MiArmTemplatePte = *MiGetPteAddress(0x80000000);
    MiArmTemplatePde = *MiGetPdeAddress(0x80000000);
    
    //
    // Loop the 2GB of address space which belong to the kernel
    //
    for (i = MiGetPdeOffset(MmSystemRangeStart); i < 1024; i++)
    {
        //
        // Check if we have an entry for this already
        //
        if ((i != MiGetPdeOffset(PTE_BASE)) &&
            (i != MiGetPdeOffset(HYPER_SPACE)) &&
            (!MmGlobalKernelPageDirectory[i]) &&
            (CurrentPageDirectory[i]))
        {
            //
            // We don't, link it in our global page directory
            //
            MmGlobalKernelPageDirectory[i] = CurrentPageDirectory[i];
        }
    }
}

VOID
NTAPI
MiInitPageDirectoryMap(VOID)
{
    MEMORY_AREA* MemoryArea = NULL;
    PHYSICAL_ADDRESS BoundaryAddressMultiple;
    PVOID BaseAddress;
    NTSTATUS Status;
    
    //
    // Create memory area for the PTE area
    //
    BoundaryAddressMultiple.QuadPart = 0;
    BaseAddress = (PVOID)PTE_BASE;
    Status = MmCreateMemoryArea(MmGetKernelAddressSpace(),
                                MEMORY_AREA_OWNED_BY_ARM3,
                                &BaseAddress,
                                0x1000000,
                                PAGE_READWRITE,
                                &MemoryArea,
                                TRUE,
                                0,
                                BoundaryAddressMultiple);
    ASSERT(NT_SUCCESS(Status));
    
    //
    // Create memory area for the PDE area
    //
    BaseAddress = (PVOID)PDE_BASE;
    Status = MmCreateMemoryArea(MmGetKernelAddressSpace(),
                                MEMORY_AREA_OWNED_BY_ARM3,
                                &BaseAddress,
                                0x100000,
                                PAGE_READWRITE,
                                &MemoryArea,
                                TRUE,
                                0,
                                BoundaryAddressMultiple);
    ASSERT(NT_SUCCESS(Status));
    
    //
    // And finally, hyperspace
    //
    BaseAddress = (PVOID)HYPER_SPACE;
    Status = MmCreateMemoryArea(MmGetKernelAddressSpace(),
                                MEMORY_AREA_OWNED_BY_ARM3,
                                &BaseAddress,
                                PAGE_SIZE,
                                PAGE_READWRITE,
                                &MemoryArea,
                                TRUE,
                                0,
                                BoundaryAddressMultiple);
    ASSERT(NT_SUCCESS(Status));
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * @implemented
 */
PHYSICAL_ADDRESS
NTAPI
MmGetPhysicalAddress(IN PVOID Address)
{
    PHYSICAL_ADDRESS PhysicalAddress;
    MMPTE Pte;

    //
    // Early boot PCR check
    //
    if (Address == PCR)
    {
        //
        // ARM Hack while we still use a section PTE
        //
        PMMPDE_HARDWARE PointerPde;
        PointerPde = MiGetPdeAddress(PCR);
        ASSERT(PointerPde->u.Hard.Section.Valid == 1);
        PhysicalAddress.QuadPart = PointerPde->u.Hard.Section.PageFrameNumber;
        PhysicalAddress.QuadPart <<= CPT_SHIFT;
        PhysicalAddress.LowPart += BYTE_OFFSET(Address);
        return PhysicalAddress;
    }
    
    //
    // Get the PTE
    //
    Pte = MiGetPageEntryForProcess(NULL, Address);
    if (Pte.u.Hard.Valid)
    {
        //
        // Return the information
        //
        PhysicalAddress.QuadPart = Pte.u.Hard.PageFrameNumber;
        PhysicalAddress.QuadPart <<= PAGE_SHIFT;
        PhysicalAddress.LowPart += BYTE_OFFSET(Address);
    }
    else
    {
        //
        // Invalid or unmapped
        //
        PhysicalAddress.QuadPart = 0;
    }
    
    //
    // Return the physical address
    //
    return PhysicalAddress;
}
