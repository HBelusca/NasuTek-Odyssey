#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rsym.h"
#include "rsym64.h"
#include "dwarf2.h"

char DoPrint = 0;
ULONG g_ehframep;

#define DPRINT if(DoPrint) printf

struct {char *name; char regnt;} regs[] =
{ {"rax", REG_RAX}, {"rdx", REG_RDX}, {"rcx", REG_RCX}, {"rbx", REG_RBX},
  {"rsi", REG_RSI}, {"rdi", REG_RDI}, {"rbp", REG_RBP}, {"rsp", REG_RSP},
  {"r8",  REG_R8},  {"r9",  REG_R9},  {"r10", REG_R10}, {"r11", REG_R11},
  {"r12", REG_R12}, {"r13", REG_R13}, {"r14", REG_R14}, {"r15", REG_R15},
  {"xmm0", REG_XMM0}, {"xmm1", REG_XMM1}, {"xmm2", REG_XMM2}, {"xmm3", REG_XMM3},
  {"xmm4", REG_XMM4}, {"xmm5", REG_XMM5}, {"xmm6", REG_XMM6}, {"xmm7", REG_XMM7},
  {"xmm8", REG_XMM8}, {"xmm9", REG_XMM9}, {"xmm10",REG_XMM10},{"xmm11",REG_XMM11},
  {"xmm12",REG_XMM12},{"xmm13",REG_XMM13},{"xmm14",REG_XMM14},{"xmm15",REG_XMM15},
//      "st0", "st1", "st2", "st3",
//      "st4", "st5", "st6", "st7",
//      "mm0", "mm1", "mm2", "mm3",
//      "mm4", "mm5", "mm6", "mm7"
};

/** Functions for DWARF2 ******************************************************/

unsigned long
DwDecodeUleb128(unsigned long *pResult, char *pc)
{
	unsigned long ulResult = 0;
	unsigned long ulShift = 0;
	unsigned char current;
	unsigned long ulSize = 0;

	do
	{
		current = pc[ulSize];
		ulSize++;
		ulResult |= (current & 0x7f) << ulShift;
		ulShift += 7;
	}
	while (current & 0x80);

    *pResult = ulResult;
	return ulSize;
}

unsigned long
DwDecodeSleb128(long *pResult, char *pc)
{
	long lResult = 0;
	unsigned long ulShift = 0;
	unsigned char current;
	unsigned long ulSize = 0;

	do
	{
		current = pc[ulSize];
		ulSize++;
		lResult |= (current & 0x7f) << ulShift;
		ulShift += 7;
	}
	while (current & 0x80);

	if (current & 0x40)
		lResult |= - (1 << (ulShift));

    *pResult = lResult;

	return ulSize;
}

unsigned long
DwDecodeCie(PDW2CIE Cie, char *pc)
{
    Cie->Length = *(ULONG*)pc;
    Cie->Next = pc + 4 + Cie->Length;
    Cie->CieId = *(ULONG*)(pc + 4);
    Cie->Version = pc[8];
    Cie->AugString = pc + 9;
    Cie->AugStringLength = strlen(Cie->AugString);
    pc = Cie->AugString + Cie->AugStringLength + 1;
    pc += DwDecodeUleb128(&Cie->CodeAlign, pc);
    pc += DwDecodeSleb128(&Cie->DataAlign, pc);
    pc += DwDecodeUleb128(&Cie->ReturnAddressRegister, pc);
    pc += DwDecodeUleb128(&Cie->AugLength, pc);
    Cie->AugData = pc;
    pc += Cie->AugLength;
    Cie->Instructions = pc;

    return Cie->Length + 4;
}

unsigned long
DwDecodeFde(PDW2FDE Fde, char *pc)
{
    Fde->Length = *(ULONG*)pc;
    Fde->Next = pc + 4 + Fde->Length;
    Fde->CiePointer = pc + 4 - *(ULONG*)(pc + 4);
    Fde->PcBegin = *(ULONG*)(pc + 8);
    Fde->PcRange = *(ULONG*)(pc + 12);
    pc += 16;
    pc += DwDecodeUleb128(&Fde->AugLength, pc);
    Fde->AugData = pc;
    Fde->Instructions = Fde->AugData + Fde->AugLength;

    return Fde->Length + 4;
}

unsigned long
DwExecIntruction(PDW2CFSTATE State, char *pc)
{
    unsigned char Code;
    unsigned long Length;
    unsigned long PrevFramePtr = State->FramePtr;

    State->Scope = 0;
    State->IsUwop = 0;
    State->Code = Code = *pc;
    Length = 1;
    if ((Code & 0xc0) == DW_CFA_advance_loc)
    {
        State->Code = DW_CFA_advance_loc;
        State->Location += Code & 0x3f;
    }
    else if ((Code & 0xc0) == DW_CFA_offset)
    {
        State->Code = DW_CFA_offset;
        State->Reg = Code & 0x3f;
        Length += DwDecodeUleb128((unsigned long*)&State->Offset, pc + 1);
        State->Offset *= 8; // fixme data alignment
        State->IsUwop = 1;
    }
    else if ((Code & 0xc0) == DW_CFA_restore)
    {
        State->Code = DW_CFA_restore;
        State->Reg = Code & 0x3f;
    }
    else switch (Code)
    {
        case DW_CFA_nop:
            break;
        case DW_CFA_set_loc:
            Length = 9; // address
            State->Location = *(DWORD*)(pc + 1);
            break;
        case DW_CFA_advance_loc1:
            Length = 2;
            State->Location += pc[1];
            break;
        case DW_CFA_advance_loc2:
            Length = 3;
//            printf("Found a DW_CFA_advance_loc2 : 0x%lx ->", *(WORD*)(pc + 1));
            State->Location += *(WORD*)(pc + 1);
//            printf(" 0x%lx\n", State->Location);
            break;
        case DW_CFA_advance_loc4:
            Length = 5;
//            printf("Found a DW_CFA_advance_loc4 : 0x%lx ->", *(DWORD*)(pc + 1));
            State->Location += *(DWORD*)(pc + 1);
//            printf(" 0x%lx\n", State->Location);
            break;
        case DW_CFA_offset_extended:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            Length += DwDecodeUleb128((unsigned long*)&State->Offset, pc + Length);
            State->IsUwop = 1;
            break;
        case DW_CFA_offset_extended_sf:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            Length += DwDecodeSleb128(&State->Offset, pc + Length);
            State->IsUwop = 1;
            break;
        case DW_CFA_restore_extended:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            break;
        case DW_CFA_undefined:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            break;
        case DW_CFA_same_value:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            break;
        case DW_CFA_register:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            Length += DwDecodeUleb128(&State->Reg2, pc + Length);
            break;
        case DW_CFA_remember_state:
            break;
        case DW_CFA_restore_state:
            break;
        case DW_CFA_def_cfa:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            Length += DwDecodeUleb128((unsigned long*)&State->FramePtr, pc + Length);
            State->IsUwop = 1;
            break;
        case DW_CFA_def_cfa_register:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            break;
        case DW_CFA_def_cfa_offset:
            Length += DwDecodeUleb128((unsigned long*)&State->FramePtr, pc + Length);
            State->IsUwop = 1;
            break;
        case DW_CFA_def_cfa_sf:
            Length += DwDecodeUleb128(&State->Reg, pc + Length);
            Length += DwDecodeSleb128(&State->FramePtr, pc + Length);
            State->FramePtr *= 8; // data alignment
            State->IsUwop = 1;
            break;
        case DW_CFA_GNU_args_size:
        {
            unsigned long argsize;
            printf("Warning, DW_CFA_GNU_args_size is unimplemented\n");
            Length += DwDecodeUleb128(&argsize, pc + Length);
            break;
        }
        /* PSEH */
        case 0x21:
        {
            unsigned long SehType;

//            printf("found 0x21 at %lx\n", State->Location);
            Length += DwDecodeUleb128(&SehType, pc + Length);
            switch (SehType)
            {
                case 1: /* Begin Try */
                    State->TryLevel++;
                    if (State->TryLevel >= 20)
                    {
                        printf("WTF? Trylevel of 20 exceeded...\n");
                        exit(1);
                    }
                    State->SehBlock[State->TryLevel-1].BeginTry = State->Location;
//                    printf("Found begintry at 0x%lx\n", State->Location);
                    State->Scope = 1;
                    break;

                case 2: /* End Try */
                    State->SehBlock[State->TryLevel-1].EndTry = State->Location;
                    State->Scope = 2;
                    break;

                case 3: /* Jump target */
                    State->SehBlock[State->TryLevel-1].Target = State->Location;
                    State->Scope = 3;
                    break;

                case 4: /* SEH End */
                    if (State->TryLevel == 20)
                    {
                        printf("Ooops, end of SEH with trylevel at 0!\n");
                        exit(1);
                    }
                    State->SehBlock[State->TryLevel-1].End = State->Location;
                    State->TryLevel--;
                    State->cScopes++;
                    State->Scope = 0;
                    break;

                case 5: /* Constant filter */
                {
                    unsigned long value;
                    Length += DwDecodeUleb128(&value, pc + Length);
                    State->SehBlock[State->TryLevel-1].Handler = value;
//                     printf("Found a constant filter at 0x%lx\n", State->Location);
                    break;
                }

               /* These work differently. We are in a new function.
                 * We have to parse a lea opcode to find the adress of
                 * the jump target. This is the reference to find the 
                 * appropriate C_SCOPE_TABLE. */
                case 6: /* Filter func */
//                    printf("Found a filter func at 0x%lx\n", State->Location);
                    break;

                case 7: /* Finally func */
                {
//                     printf("Found a finally func at 0x%lx\n", State->Location);
                    break;
                }

                default:
                    printf("Found unknow PSEH code 0x%lx\n", SehType);
                    exit(1);
            }
            break;
        }
        default:
            fprintf(stderr, "unknown instruction 0x%x at 0x%p\n", Code, pc);
            exit(1);
    }
    
    State->FramePtrDiff = State->FramePtr - PrevFramePtr;
    DPRINT("@%p: code=%x, Loc=%lx, offset=%lx, reg=0x%lx:%s\n", 
        (void*)((ULONG)pc - g_ehframep), Code, State->Location, State->Offset, State->Reg, regs[State->Reg].name);
    return Length;
}

/** Windows unwind data functions *********************************************/

ULONG
StoreUnwindCodes(PUNWIND_INFO Info, PDW2CFSTATE State, ULONG FunctionStart)
{
    ULONG cCodes = 0;
    ULONG AllocSize;
    UNWIND_CODE Code[3];
    int i;

    Code[0].CodeOffset = State->Location - FunctionStart;

    switch (State->Code)
    {
        case DW_CFA_offset:
        case DW_CFA_offset_extended:
            // save register at offset
            Code[0].OpInfo = regs[State->Reg].regnt;
            if (State->Offset <= 0x7FFF8)
            {
                Code[0].UnwindOp = UWOP_SAVE_NONVOL;
                Code[1].FrameOffset = State->Offset / 8;
                cCodes = 2;
            }
            else
            {
                Code[0].UnwindOp = UWOP_SAVE_NONVOL_FAR;
                Code[1].FrameOffset = (State->Offset / 8);
                Code[2].FrameOffset = (State->Offset / 8) >> 16;
                cCodes = 3;
            }
            break;

        case DW_CFA_def_cfa:
        //case DW_CFA_def_cfa_register:
        case DW_CFA_def_cfa_offset:
        case DW_CFA_def_cfa_sf:
            AllocSize = State->FramePtrDiff;
            if (AllocSize <= 128)
            {
                Code[0].UnwindOp = UWOP_ALLOC_SMALL;
                Code[0].OpInfo = (AllocSize / 8) - 1;
                cCodes = 1;
            }
            else if (AllocSize <= 0x7FFF8)
            {
                Code[0].UnwindOp = UWOP_ALLOC_LARGE;
                Code[0].OpInfo = 0;
                Code[1].FrameOffset = AllocSize / 8;
                cCodes = 2;
            }
            else // if (AllocSize > 0x7FFF8)
            {
                Code[0].UnwindOp = UWOP_ALLOC_LARGE;
                Code[0].OpInfo = 1;
                Code[1].FrameOffset = (USHORT)AllocSize;
                Code[2].FrameOffset = (USHORT)(AllocSize >> 16);
                cCodes = 3;
            }
            break;
    }

    if (Info)
    {
        /* Move old codes */
        for (i = Info->CountOfCodes - 1; i >= 0; i--)
        {
            Info->UnwindCode[i + cCodes] = Info->UnwindCode[i];
        }

        /* Copy new codes */
        for (i = 0; i < cCodes; i++)
        {
            Info->UnwindCode[i] = Code[i];
        }

        Info->CountOfCodes += cCodes;
    }

    return cCodes;
}

#define GetxdataSize(cFuncs, cUWOP, cScopes) \
    ( cFuncs * (sizeof(UNWIND_INFO) + 2 + 4 + 4) \
    + cUWOP * sizeof(UNWIND_CODE) \
    + cScopes * sizeof(C_SCOPE_TABLE_ENTRY) )

ULONG
StoreUnwindInfo(PUNWIND_INFO Info, PDW2FDE pFde, ULONG FunctionStart)
{
    ULONG cbSize;
    DW2CFSTATE State;
    char *pInst;
    ULONG c;
    DW2CIE Cie;

    cbSize = 4; // sizeof(UNWIND_INFO);
    Info->Version = 1;
    Info->Flags = 0;
    Info->SizeOfProlog = 0;
    Info->CountOfCodes = 0;
    Info->FrameRegister = 0;
    Info->FrameOffset = 0;

    /* Decode the CIE */
    DwDecodeCie(&Cie, pFde->CiePointer);

    /* Initialize state */
    State.Location = FunctionStart;
    State.FramePtr = 0;
    State.TryLevel = 0;
    State.cScopes = 0;

    /* Parse the CIE's initial instructions */
    pInst = Cie.Instructions;
    while (pInst < Cie.Next)
    {
        pInst += DwExecIntruction(&State, pInst);
    }

    /* Parse the FDE instructions */
    pInst = pFde->Instructions;
    while (pInst < pFde->Next)
    {
        pInst += DwExecIntruction(&State, pInst);

        if (State.IsUwop)
        {
            c = StoreUnwindCodes(Info, &State, FunctionStart);
            cbSize += c * sizeof(UNWIND_CODE);
            Info->SizeOfProlog = State.Location - FunctionStart;
        }
    }
    cbSize = ROUND_UP(cbSize, 4);

    /* Do we have scope table to write? */
    if (State.cScopes > 0)
    {
        unsigned long i;
        ULONG *pExceptionHandler;
        PC_SCOPE_TABLE pScopeTable;

        /* Set flag for exception handler */ 
        Info->Flags |= UNW_FLAG_EHANDLER;

        /* Store address of handler and number of scope tables */
        pExceptionHandler = (ULONG*)((char*)Info + cbSize);
        // HACK for testing purpose
        *pExceptionHandler = FunctionStart; // _C_specific_handler

        pScopeTable = (PC_SCOPE_TABLE)(pExceptionHandler + 1);
        pScopeTable->NumEntries = State.cScopes;

        /* Store the scope table entries */
        for (i = 0; i < State.cScopes; i++)
        {
            pScopeTable->Entry[i].Begin = State.SehBlock[i].BeginTry;
            pScopeTable->Entry[i].End = State.SehBlock[i].EndTry;
            pScopeTable->Entry[i].Handler = 1;//State.SehBlock[i].Handler;
            pScopeTable->Entry[i].Target = State.SehBlock[i].Target;
        }
        
        /* Update size */
        cbSize += 8 + State.cScopes * sizeof(C_SCOPE_TABLE_ENTRY);
    }

    return cbSize;
}

void
CountUnwindData(PFILE_INFO File)
{
    DW2CIEFDE *p;
    DW2FDE Fde;
    char *pInst, *pmax;
    DW2CFSTATE State;

    File->cFuncs = 0;
    File->cScopes = 0;
    File->cUWOP = 0;
    State.FramePtr = 0;
    State.TryLevel = 0;

    p = File->eh_frame.p;
    pmax = (char*)p + File->eh_frame.psh->Misc.VirtualSize;
    for (; p->Length && (char*)p < pmax; p = NextCIE(p))
    {
        /* Is this an FDE? */
        if (p->CiePointer != 0)
        {
            File->cFuncs++;
            DwDecodeFde(&Fde, (char*)p);

            pInst = Fde.Instructions;
            while (pInst < Fde.Next)
            {
                pInst += DwExecIntruction(&State, pInst);
                File->cUWOP += StoreUnwindCodes(NULL, &State, 0);
                File->cScopes += State.Scope ? 1 : 0;
            }
        }
    }

    return;
}

int CompFunc(const void *p1, const void *p2)
{
    PRUNTIME_FUNCTION prf1 = (void*)p1, prf2 = (void*)p2;
    return (prf1->FunctionStart > prf2->FunctionStart ? 1 : -1);
}

void
GeneratePData(PFILE_INFO File)
{
    DW2CIEFDE *p;
    DW2FDE Fde;
    PIMAGE_DATA_DIRECTORY Dir;
    ULONG i, Offset;
    void * eh_frame;
    PRUNTIME_FUNCTION pdata;
    ULONG xdata_va;
    char *xdata_p;
    ULONG cbSize;
    PIMAGE_SECTION_HEADER pshp, pshx;
    ULONG FileAlignment;
    char *pmax;

    FileAlignment = File->OptionalHeader->FileAlignment;

    /* Get pointer to eh_frame section */
    eh_frame = File->eh_frame.p;
    g_ehframep = (ULONG)eh_frame;

    /* Get sizes */
    CountUnwindData(File);
//    printf("cFuncs = %ld, cUWOPS = %ld, cScopes = %ld\n", 
//        File->cFuncs, File->cUWOP, File->cScopes);

    /* Initialize section header for .pdata */
    i = File->pdata.idx = File->UsedSections;
    pshp = File->pdata.psh = &File->NewSectionHeaders[i];
    memcpy(pshp->Name, ".pdata", 7);
    pshp->Misc.VirtualSize = (File->cFuncs + 1) * sizeof(RUNTIME_FUNCTION);
    pshp->VirtualAddress = File->NewSectionHeaders[i - 1].VirtualAddress +
                           File->NewSectionHeaders[i - 1].SizeOfRawData;
    pshp->SizeOfRawData = ROUND_UP(pshp->Misc.VirtualSize, FileAlignment);
    pshp->PointerToRawData = File->NewSectionHeaders[i - 1].PointerToRawData +
                           File->NewSectionHeaders[i - 1].SizeOfRawData;
    pshp->PointerToRelocations = 0;
    pshp->PointerToLinenumbers = 0;
    pshp->NumberOfRelocations = 0;
    pshp->NumberOfLinenumbers = 0;
    pshp->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_NOT_PAGED |
                            IMAGE_SCN_CNT_INITIALIZED_DATA;

    /* Allocate .pdata buffer */
    pdata = File->pdata.p = malloc(pshp->SizeOfRawData);
    memset(File->pdata.p, pshp->SizeOfRawData, 0);

    /* Init exception data dir */
    Dir = &File->OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
    Dir->VirtualAddress = pshp->VirtualAddress;
    Dir->Size = pshp->Misc.VirtualSize;

    /* Initialize section header for .xdata */
    File->xdata.idx = File->pdata.idx + 1;
    pshx = File->xdata.psh = &File->NewSectionHeaders[File->xdata.idx];
    memcpy(pshx->Name, ".xdata", 7);
    pshx->Misc.VirtualSize = GetxdataSize(File->cFuncs, File->cUWOP, File->cScopes);
    pshx->VirtualAddress = pshp->VirtualAddress + pshp->SizeOfRawData;
    pshx->SizeOfRawData = ROUND_UP(pshx->Misc.VirtualSize, FileAlignment);
    pshx->PointerToRawData = pshp->PointerToRawData + pshp->SizeOfRawData;
    pshx->PointerToRelocations = 0;
    pshx->PointerToLinenumbers = 0;
    pshx->NumberOfRelocations = 0;
    pshx->NumberOfLinenumbers = 0;
    pshx->Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_NOT_PAGED |
                            IMAGE_SCN_CNT_INITIALIZED_DATA;

    /* Allocate .xdata buffer */
    File->xdata.p = malloc(pshx->SizeOfRawData);
    memset(File->xdata.p, pshx->SizeOfRawData, 0);

    i = 0;
    Offset = File->eh_frame.psh->VirtualAddress;
    xdata_va = pshx->VirtualAddress;
    xdata_p = File->xdata.p;
    pmax = (char*)eh_frame + File->eh_frame.psh->Misc.VirtualSize - 100;

    for (p = eh_frame; p->Length && (char*)p < pmax; p = NextCIE(p))
    {
        /* Is this an FDE? */
        if (p->CiePointer != 0)
        {
            DwDecodeFde(&Fde, (char*)p);
            pdata[i].FunctionStart = Offset + 8 + Fde.PcBegin;
            pdata[i].FunctionEnd = pdata[i].FunctionStart + Fde.PcRange;
            pdata[i].UnwindInfo = xdata_va;

//            printf("%ld: RUNTIME_FUNCTION: {0x%lx, 0x%lx, 0x%lx}\n", i, pdata[i].FunctionStart, pdata[i].FunctionEnd, pdata[i].UnwindInfo);

            cbSize = StoreUnwindInfo((void*)xdata_p, &Fde, pdata[i].FunctionStart);
            xdata_va += cbSize;
            xdata_p += cbSize;
            i++;
        }
        Offset += 4 + p->Length;
    }

    /* Sort the RUNTIME_FUNCTIONS */
    qsort(pdata, i, sizeof(RUNTIME_FUNCTION), CompFunc);

}

/** Functions for COFF ********************************************************/


WORD
CalculateChecksum(DWORD Start, void *pFile, ULONG cbSize)
{
    WORD *Ptr = pFile;
    DWORD i;
    DWORD checksum = Start;

    for (i = 0; i < (cbSize + 1) / sizeof(WORD); i++)
    {
        checksum += Ptr[i];
        checksum = (checksum + (checksum >> 16)) & 0xffff;
    }

    return checksum ;
}

void
WriteOutFile(FILE *handle, PFILE_INFO File)
{
    int ret, Size, Pos = 0;
    DWORD CheckSum;
    ULONG i, Alignment;

    Alignment = File->OptionalHeader->FileAlignment;

    /* Update section count */
    File->FileHeader->NumberOfSections = File->UsedSections + 2; // FIXME!!!

    /* Update SizeOfImage */
    Size = File->xdata.psh->VirtualAddress
           + File->xdata.psh->SizeOfRawData;
    File->OptionalHeader->SizeOfImage = Size;

    /* Recalculate checksum */
    CheckSum = CalculateChecksum(0, File->FilePtr, File->HeaderSize);
    for (i = 0; i < File->AllSections; i++)
    {
        if (File->UseSection[i])
        {
            Size = File->SectionHeaders[i].SizeOfRawData;
            if (Size)
            {
                void *p;
                p = File->FilePtr + File->SectionHeaders[i].PointerToRawData;
                CheckSum = CalculateChecksum(CheckSum, p, Size);
            }
        }
    }
    Size = File->pdata.psh->Misc.VirtualSize;
    CheckSum = CalculateChecksum(CheckSum, File->pdata.p, Size);
    Size = File->xdata.psh->Misc.VirtualSize;
    CheckSum = CalculateChecksum(CheckSum, File->xdata.p, Size);
    CheckSum += File->HeaderSize;
    CheckSum += File->pdata.psh->Misc.VirtualSize;
    CheckSum += File->xdata.psh->Misc.VirtualSize;
    File->OptionalHeader->CheckSum = CheckSum;

    /* Write file header */
    Size = File->HeaderSize;
    ret = fwrite(File->DosHeader, 1, Size, handle);
    Pos = Size;

    /* Write Section headers */
    Size = File->NewSectionHeaderSize;
    ret = fwrite(File->NewSectionHeaders, 1, Size, handle);
    Pos += Size;

    /* Fill up to next alignement */
    Size = ROUND_UP(Pos, Alignment) - Pos;
    ret = fwrite(File->AlignBuf, 1, Size, handle);
    Pos += Size;

    /* Write sections */
    for (i = 0; i < File->AllSections; i++)
    {
        if (File->UseSection[i])
        {
            void *p;
            Size = File->SectionHeaders[i].SizeOfRawData;
            if (Size)
            {
                p = File->FilePtr + File->SectionHeaders[i].PointerToRawData;
                ret = fwrite(p, 1, Size, handle);
                Pos += Size;
            }
        }
    }

    /* Write .pdata section */
    Size = File->pdata.psh->SizeOfRawData;
    ret = fwrite(File->pdata.p, 1, Size, handle);
    Pos += Size;

    /* Write .xdata section */
    Size = File->xdata.psh->SizeOfRawData;
    ret = fwrite(File->xdata.p, 1, Size, handle);
    Pos += Size;

}


int
ParsePEHeaders(PFILE_INFO File)
{
    DWORD OldChecksum, Checksum;
    ULONG Alignment, CurrentPos;
    int i, j;

    /* Check if MZ header exists  */
    File->DosHeader = (PIMAGE_DOS_HEADER)File->FilePtr;
    if ((File->DosHeader->e_magic != IMAGE_DOS_MAGIC) || 
        (File->DosHeader->e_lfanew == 0L))
    {
        perror("Input file is not a PE image.\n");
        return -1;
    }

    /* Locate PE file header  */
    File->FileHeader = (PIMAGE_FILE_HEADER)(File->FilePtr + 
                               File->DosHeader->e_lfanew + sizeof(ULONG));

    /* Check for x64 image */
    if (File->FileHeader->Machine != IMAGE_FILE_MACHINE_AMD64)
    {
        perror("Input file is not an x64 image.\n");
        return -1;
    }

    /* Locate optional header */
    File->OptionalHeader = (PIMAGE_OPTIONAL_HEADER64)(File->FileHeader + 1);

    /* Check if checksum is correct */
    OldChecksum = File->OptionalHeader->CheckSum;
    File->OptionalHeader->CheckSum = 0;
    Checksum = CalculateChecksum(0, File->FilePtr, File->cbInFileSize);
    Checksum += File->cbInFileSize;
    if ((Checksum & 0xffff) != (OldChecksum & 0xffff))
    {
        fprintf(stderr, "Input file has incorrect PE checksum: 0x%lx (calculated: 0x%lx)\n",
            OldChecksum, Checksum);
//        return 0;
    }

    /* Locate PE section headers  */
    File->SectionHeaders = (PIMAGE_SECTION_HEADER)((char*)File->OptionalHeader
                           + File->FileHeader->SizeOfOptionalHeader);

    File->HeaderSize = File->DosHeader->e_lfanew
                       + sizeof(ULONG)
                       + sizeof(IMAGE_FILE_HEADER)
                       + File->FileHeader->SizeOfOptionalHeader;

    if (!File->FileHeader->PointerToSymbolTable)
    {
        fprintf(stderr, "No symbol table.\n");
        return -1;
    }

    /* Create some shortcuts */
    File->ImageBase = File->OptionalHeader->ImageBase;
    File->Symbols = File->FilePtr + File->FileHeader->PointerToSymbolTable;
    File->Strings = (char*)File->Symbols + File->FileHeader->NumberOfSymbols * 18;

    /* Check section names */
    File->AllSections = File->FileHeader->NumberOfSections;
    Alignment = File->OptionalHeader->FileAlignment;
    File->NewSectionHeaders = malloc((File->AllSections+2) * sizeof(IMAGE_SECTION_HEADER));
    File->UsedSections = 0;
    File->eh_frame.idx = -1;

    /* Allocate array of chars, specifiying wheter to copy the section */
    File->UseSection = malloc(File->AllSections);

    for (i = 0; i < File->AllSections; i++)
    {
        char *pName = (char*)File->SectionHeaders[i].Name;
        File->UseSection[i] = 1;

        /* Check for long name */
        if (pName[0] == '/')
        {
            unsigned long index = strtoul(pName+1, 0, 10);
            pName = File->Strings + index;
            
            // Hack, simply remove all sections with long names
            File->UseSection[i] = 0;
        }

        /* Chek if we have the eh_frame section */
        if (strcmp(pName, ".eh_frame") == 0)
        {
            File->eh_frame.psh = &File->SectionHeaders[i];
            File->eh_frame.idx = i;
            File->eh_frame.p = File->FilePtr + File->eh_frame.psh->PointerToRawData;
        }
        
        /* Increase number of used sections */
        if (File->UseSection[i])
            File->UsedSections = i+1;

    }

    /* This is the actual size of the new section headers */
    File->NewSectionHeaderSize = 
        (File->UsedSections+2) * sizeof(IMAGE_SECTION_HEADER);

    /* Calculate the position to start writing the sections to */
    CurrentPos = File->HeaderSize + File->NewSectionHeaderSize;
    CurrentPos = ROUND_UP(CurrentPos, Alignment);

    /* Create new section headers */
    for (i = 0, j = 0; i < File->UsedSections; i++)
    {
        /* Copy section header */
        File->NewSectionHeaders[j] = File->SectionHeaders[i];

        /* Shall we strip the section? */
        if (File->UseSection[i] == 0)
        {
            /* Make it a bss section */
            File->NewSectionHeaders[j].PointerToRawData = 0;
            File->NewSectionHeaders[j].SizeOfRawData = 0;
            File->NewSectionHeaders[j].Characteristics = 0xC0500080;
        }

        /* Fix Offset into File */
        File->NewSectionHeaders[j].PointerToRawData =
              File->NewSectionHeaders[j].PointerToRawData ? CurrentPos : 0;
        CurrentPos += File->NewSectionHeaders[j].SizeOfRawData;
        j++;
    }

    if (File->eh_frame.idx == -1)
    {
        //fprintf(stderr, "No .eh_frame section found\n");
        return 0;
    }

    return 1;
}

int main(int argc, char* argv[])
{
    char* pszInFile;
    char* pszOutFile;
    FILE_INFO File;
    FILE* outfile;
    int ret;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: rsym <exefile> <symfile>\n");
        exit(1);
    }

    pszInFile = convert_path(argv[1]);
    pszOutFile = convert_path(argv[2]);

    File.FilePtr = load_file(pszInFile, &File.cbInFileSize);
    if (!File.FilePtr)
    {
        fprintf(stderr, "An error occured loading '%s'\n", pszInFile);
        exit(1);
    }

    ret = ParsePEHeaders(&File);
    if (ret != 1)
    {
        free(File.FilePtr);
        exit(ret == -1 ? 1 : 0);
    }

    File.AlignBuf = malloc(File.OptionalHeader->FileAlignment);
    memset(File.AlignBuf, File.OptionalHeader->FileAlignment, 0);

    GeneratePData(&File);

    outfile = fopen(pszOutFile, "wb");
    if (outfile == NULL)
    {
        perror("Cannot open output file");
        free(File.FilePtr);
        exit(1);
    }

    WriteOutFile(outfile, &File);

    fclose(outfile);

    return 0;
}
