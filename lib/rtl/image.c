/* COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/rtl/image.c
 * PURPOSE:         Image handling functions
 *                  Relocate functions were previously located in
 *                  ntoskrnl/ldr/loader.c and
 *                  dll/ntdll/ldr/utils.c files
 * PROGRAMMER:      Eric Kohl + original authors from loader.c and utils.c file
 *                  Aleksey Bragin
 */

/* INCLUDES *****************************************************************/

#include <rtl.h>

#define NDEBUG
#include <debug.h>

#define RVA(m, b) ((PVOID)((ULONG_PTR)(b) + (ULONG_PTR)(m)))

/* FUNCTIONS *****************************************************************/

USHORT
FORCEINLINE
ChkSum(ULONG Sum, PUSHORT Src, ULONG Len)
{
    ULONG i;

    for (i=0; i<Len; i++)
    {
        /* Sum up the current word */
        Sum += Src[i];

        /* Sum up everything above the low word as a carry */
        Sum = (Sum & 0xFFFF) + (Sum >> 16);
    }

    /* Apply carry one more time and clamp to the USHORT */
    return (Sum + (Sum >> 16)) & 0xFFFF;
}

BOOLEAN
NTAPI
LdrVerifyMappedImageMatchesChecksum(
    IN PVOID BaseAddress,
    IN ULONG ImageSize,
    IN ULONG FileLength)
{
#if 0
    PIMAGE_NT_HEADERS Header;
    PUSHORT Ptr;
    ULONG Sum;
    ULONG CalcSum;
    ULONG HeaderSum;
    ULONG i;

    // HACK: Ignore calls with ImageSize=0. Should be fixed by new MM.
    if (ImageSize == 0) return TRUE;

    /* Get NT header to check if it's an image at all */
    Header = RtlImageNtHeader(BaseAddress);
    if (!Header) return FALSE;

    /* Get checksum to match */
    HeaderSum = Header->OptionalHeader.CheckSum;

    /* Zero checksum seems to be accepted */
    if (HeaderSum == 0) return TRUE;

    /* Calculate the checksum */
    Sum = 0;
    Ptr = (PUSHORT) BaseAddress;
    for (i = 0; i < ImageSize / sizeof (USHORT); i++)
    {
        Sum += (ULONG)*Ptr;
        if (HIWORD(Sum) != 0)
        {
            Sum = LOWORD(Sum) + HIWORD(Sum);
        }
        Ptr++;
    }

    if (ImageSize & 1)
    {
        Sum += (ULONG)*((PUCHAR)Ptr);
        if (HIWORD(Sum) != 0)
        {
            Sum = LOWORD(Sum) + HIWORD(Sum);
        }
    }

    CalcSum = (USHORT)(LOWORD(Sum) + HIWORD(Sum));

    /* Subtract image checksum from calculated checksum. */
    /* fix low word of checksum */
    if (LOWORD(CalcSum) >= LOWORD(HeaderSum))
    {
        CalcSum -= LOWORD(HeaderSum);
    }
    else
    {
        CalcSum = ((LOWORD(CalcSum) - LOWORD(HeaderSum)) & 0xFFFF) - 1;
    }

    /* Fix high word of checksum */
    if (LOWORD(CalcSum) >= HIWORD(HeaderSum))
    {
        CalcSum -= HIWORD(HeaderSum);
    }
    else
    {
        CalcSum = ((LOWORD(CalcSum) - HIWORD(HeaderSum)) & 0xFFFF) - 1;
    }

    /* Add file length */
    CalcSum += ImageSize;

    if (CalcSum != HeaderSum)
        DPRINT1("Image %p checksum mismatches! 0x%x != 0x%x, ImageSize %x, FileLen %x\n", BaseAddress, CalcSum, HeaderSum, ImageSize, FileLength);

    return (BOOLEAN)(CalcSum == HeaderSum);
#else
    /*
     * FIXME: Warning, this violates the PE standard and makes Odyssey drivers
     * and other system code when normally on Windows they would not, since
     * we do not write the checksum in them.
     * Our compilers should be made to write out the checksum and this function
     * should be enabled as to reject badly checksummed code.
     */
    return TRUE;
#endif
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlImageNtHeaderEx(IN ULONG Flags,
                   IN PVOID Base,
                   IN ULONG64 Size,
                   OUT PIMAGE_NT_HEADERS *OutHeaders)
{
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_DOS_HEADER DosHeader;
    BOOLEAN WantsRangeCheck;

    /* You must want NT Headers, no? */
    if (!OutHeaders) return STATUS_INVALID_PARAMETER;

    /* Assume failure */
    *OutHeaders = NULL;

    /* Validate Flags */
    if (Flags &~ RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK)
    {
        DPRINT1("Invalid flag combination... check for new API flags?\n");
        return STATUS_INVALID_PARAMETER;
    }

    /* Validate base */
    if (!(Base) || (Base == (PVOID)-1)) return STATUS_INVALID_PARAMETER;

    /* Check if the caller wants validation */
    WantsRangeCheck = !(Flags & RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK);
    if (WantsRangeCheck)
    {
        /* Make sure the image size is at least big enough for the DOS header */
        if (Size < sizeof(IMAGE_DOS_HEADER))
        {
            DPRINT1("Size too small\n");
            return STATUS_INVALID_IMAGE_FORMAT;
        }
    }

    /* Check if the DOS Signature matches */
    DosHeader = Base;
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        /* Not a valid COFF */
        DPRINT1("Not an MZ file\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    /* Check if the caller wants validation */
    if (WantsRangeCheck)
    {
        /* The offset should fit in the passsed-in size */
        if (DosHeader->e_lfanew >= Size)
        {
            /* Fail */
            DPRINT1("e_lfanew is larger than PE file\n");
            return STATUS_INVALID_IMAGE_FORMAT;
        }
        
        /* It shouldn't be past 4GB either */
        if (DosHeader->e_lfanew >=
            (MAXULONG - sizeof(IMAGE_DOS_SIGNATURE) - sizeof(IMAGE_FILE_HEADER)))
        {
            /* Fail */
            DPRINT1("e_lfanew is larger than 4GB\n");
            return STATUS_INVALID_IMAGE_FORMAT;
        }
        
        /* And the whole file shouldn't overflow past 4GB */
        if ((DosHeader->e_lfanew +
            sizeof(IMAGE_DOS_SIGNATURE) - sizeof(IMAGE_FILE_HEADER)) >= Size)
        {
            /* Fail */
            DPRINT1("PE is larger than 4GB\n");
            return STATUS_INVALID_IMAGE_FORMAT;
        }
    }
    
    /* The offset also can't be larger than 256MB, as a hard-coded check */
    if (DosHeader->e_lfanew >= (256 * 1024 * 1024))
    {
        /* Fail */
        DPRINT1("PE offset is larger than 256MB\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    /* Now it's safe to get the NT Headers */
    NtHeaders = (PIMAGE_NT_HEADERS)((ULONG_PTR)Base + DosHeader->e_lfanew);

    /* Verify the PE Signature */
    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        /* Fail */
        DPRINT1("PE signature missing\n");
        return STATUS_INVALID_IMAGE_FORMAT;
    }

    /* Now return success and the NT header */
    *OutHeaders = NtHeaders;
    return STATUS_SUCCESS;
}
    
/*
 * @implemented
 */
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(IN PVOID Base)
{
    PIMAGE_NT_HEADERS NtHeader;

    /* Call the new API */
    RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK,
                       Base,
                       0,
                       &NtHeader);
    return NtHeader;
}

/*
 * @implemented
 */
PVOID
NTAPI
RtlImageDirectoryEntryToData(
    PVOID BaseAddress,
    BOOLEAN MappedAsImage,
    USHORT Directory,
    PULONG Size)
{
    PIMAGE_NT_HEADERS NtHeader;
    ULONG Va;

    /* Magic flag for non-mapped images. */
    if ((ULONG_PTR)BaseAddress & 1)
    {
        BaseAddress = (PVOID)((ULONG_PTR)BaseAddress & ~1);
        MappedAsImage = FALSE;
    }


    NtHeader = RtlImageNtHeader (BaseAddress);
    if (NtHeader == NULL)
        return NULL;

    if (Directory >= SWAPD(NtHeader->OptionalHeader.NumberOfRvaAndSizes))
        return NULL;

    Va = SWAPD(NtHeader->OptionalHeader.DataDirectory[Directory].VirtualAddress);
    if (Va == 0)
        return NULL;

    *Size = SWAPD(NtHeader->OptionalHeader.DataDirectory[Directory].Size);

    if (MappedAsImage || Va < SWAPD(NtHeader->OptionalHeader.SizeOfHeaders))
        return (PVOID)((ULONG_PTR)BaseAddress + Va);

    /* image mapped as ordinary file, we must find raw pointer */
    return RtlImageRvaToVa (NtHeader, BaseAddress, Va, NULL);
}


/*
 * @implemented
 */
PIMAGE_SECTION_HEADER
NTAPI
RtlImageRvaToSection(
    PIMAGE_NT_HEADERS NtHeader,
    PVOID BaseAddress,
    ULONG Rva)
{
    PIMAGE_SECTION_HEADER Section;
    ULONG Va;
    ULONG Count;

    Count = SWAPW(NtHeader->FileHeader.NumberOfSections);
    Section = IMAGE_FIRST_SECTION(NtHeader);

    while (Count--)
    {
        Va = SWAPD(Section->VirtualAddress);
        if ((Va <= Rva) &&
                (Rva < Va + SWAPD(Section->Misc.VirtualSize)))
            return Section;
        Section++;
    }
    return NULL;
}


/*
 * @implemented
 */
PVOID
NTAPI
RtlImageRvaToVa(
    PIMAGE_NT_HEADERS NtHeader,
    PVOID BaseAddress,
    ULONG Rva,
    PIMAGE_SECTION_HEADER *SectionHeader)
{
    PIMAGE_SECTION_HEADER Section = NULL;

    if (SectionHeader)
        Section = *SectionHeader;

    if (Section == NULL ||
            Rva < SWAPD(Section->VirtualAddress) ||
            Rva >= SWAPD(Section->VirtualAddress) + SWAPD(Section->Misc.VirtualSize))
    {
        Section = RtlImageRvaToSection (NtHeader, BaseAddress, Rva);
        if (Section == NULL)
            return 0;

        if (SectionHeader)
            *SectionHeader = Section;
    }

    return (PVOID)((ULONG_PTR)BaseAddress +
                   Rva +
                   SWAPD(Section->PointerToRawData) -
                   (ULONG_PTR)SWAPD(Section->VirtualAddress));
}

PIMAGE_BASE_RELOCATION
NTAPI
LdrProcessRelocationBlockLongLong(
    IN ULONG_PTR Address,
    IN ULONG Count,
    IN PUSHORT TypeOffset,
    IN LONGLONG Delta)
{
    SHORT Offset;
    USHORT Type;
    USHORT i;
    PUSHORT ShortPtr;
    PULONG LongPtr;
    PULONGLONG LongLongPtr;

    for (i = 0; i < Count; i++)
    {
        Offset = SWAPW(*TypeOffset) & 0xFFF;
        Type = SWAPW(*TypeOffset) >> 12;
        ShortPtr = (PUSHORT)(RVA(Address, Offset));

        /*
        * Don't relocate within the relocation section itself.
        * GCC/LD generates sometimes relocation records for the relocation section.
        * This is a bug in GCC/LD.
        * Fix for it disabled, since it was only in ntoskrnl and not in ntdll
        */
        /*
        if ((ULONG_PTR)ShortPtr < (ULONG_PTR)RelocationDir ||
        (ULONG_PTR)ShortPtr >= (ULONG_PTR)RelocationEnd)
        {*/
        switch (Type)
        {
            /* case IMAGE_REL_BASED_SECTION : */
            /* case IMAGE_REL_BASED_REL32 : */
        case IMAGE_REL_BASED_ABSOLUTE:
            break;

        case IMAGE_REL_BASED_HIGH:
            *ShortPtr = HIWORD(MAKELONG(0, *ShortPtr) + (LONG)Delta);
            break;

        case IMAGE_REL_BASED_LOW:
            *ShortPtr = SWAPW(*ShortPtr) + LOWORD(Delta);
            break;

        case IMAGE_REL_BASED_HIGHLOW:
            LongPtr = (PULONG)RVA(Address, Offset);
            *LongPtr = SWAPD(*LongPtr) + (ULONG)Delta;
            break;

        case IMAGE_REL_BASED_DIR64:
            LongLongPtr = (PUINT64)RVA(Address, Offset);
            *LongLongPtr = SWAPQ(*LongLongPtr) + Delta;
            break;

        case IMAGE_REL_BASED_HIGHADJ:
        case IMAGE_REL_BASED_MIPS_JMPADDR:
        default:
            DPRINT1("Unknown/unsupported fixup type %hu.\n", Type);
            DPRINT1("Address %x, Current %d, Count %d, *TypeOffset %x\n", Address, i, Count, SWAPW(*TypeOffset));
            return (PIMAGE_BASE_RELOCATION)NULL;
        }

        TypeOffset++;
    }

    return (PIMAGE_BASE_RELOCATION)TypeOffset;
}

ULONG
NTAPI
LdrRelocateImageWithBias(
    IN PVOID BaseAddress,
    IN LONGLONG AdditionalBias,
    IN PCCH  LoaderName,
    IN ULONG Success,
    IN ULONG Conflict,
    IN ULONG Invalid)
{
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_DATA_DIRECTORY RelocationDDir;
    PIMAGE_BASE_RELOCATION RelocationDir, RelocationEnd;
    ULONG Count;
    ULONG_PTR Address;
    PUSHORT TypeOffset;
    LONGLONG Delta;

    NtHeaders = RtlImageNtHeader(BaseAddress);

    if (NtHeaders == NULL)
        return Invalid;

    if (SWAPW(NtHeaders->FileHeader.Characteristics) & IMAGE_FILE_RELOCS_STRIPPED)
    {
        return Conflict;
    }

    RelocationDDir = &NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

    if (SWAPD(RelocationDDir->VirtualAddress) == 0 || SWAPD(RelocationDDir->Size) == 0)
    {
        return Success;
    }

    Delta = (ULONG_PTR)BaseAddress - SWAPD(NtHeaders->OptionalHeader.ImageBase) + AdditionalBias;
    RelocationDir = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)BaseAddress + SWAPD(RelocationDDir->VirtualAddress));
    RelocationEnd = (PIMAGE_BASE_RELOCATION)((ULONG_PTR)RelocationDir + SWAPD(RelocationDDir->Size));

    while (RelocationDir < RelocationEnd &&
            SWAPW(RelocationDir->SizeOfBlock) > 0)
    {
        Count = (SWAPW(RelocationDir->SizeOfBlock) - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);
        Address = (ULONG_PTR)RVA(BaseAddress, SWAPD(RelocationDir->VirtualAddress));
        TypeOffset = (PUSHORT)(RelocationDir + 1);

        RelocationDir = LdrProcessRelocationBlockLongLong(Address,
                        Count,
                        TypeOffset,
                        Delta);

        if (RelocationDir == NULL)
        {
            DPRINT1("Error during call to LdrProcessRelocationBlockLongLong()!\n");
            return Invalid;
        }
    }

    return Success;
}

/* EOF */
