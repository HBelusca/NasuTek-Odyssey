#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../pecoff.h"

static
void
Usage(void)
{
    printf("Converts a coff object file into a raw binary file.\n"
           "Syntax: obj2bin <source file> <dest file> <base address>\n");
}

static
void
RelocateSection(
    char *pData,
    IMAGE_SECTION_HEADER *pSectionHeader,
    PIMAGE_SYMBOL pSymbols,
    unsigned int iOffset)
{
    unsigned int i, nOffset;
    PIMAGE_RELOCATION pReloc;
    char *pSection;
    WORD *p16;
    DWORD *p32;

    pSection = pData + pSectionHeader->PointerToRawData;

    /* Calculate pointer to relocation table */
    pReloc = (PIMAGE_RELOCATION)(pData + pSectionHeader->PointerToRelocations);

    /* Loop all relocations */
    for (i = 0; i < pSectionHeader->NumberOfRelocations; i++)
    {
        nOffset = pReloc->VirtualAddress - pSectionHeader->VirtualAddress;

        if (nOffset > pSectionHeader->SizeOfRawData) continue;

        switch (pReloc->Type)
        {
            case IMAGE_REL_I386_ABSOLUTE:
            case 16:
                p16 = (void*)(pSection + nOffset);
                *p16 += (WORD)(pSymbols[pReloc->SymbolTableIndex].Value + iOffset);
                break;

            case IMAGE_REL_I386_DIR32:
                p32 = (void*)(pSection + nOffset);
                *p32 += (DWORD)(pSymbols[pReloc->SymbolTableIndex].Value + iOffset);
                break;

            default:
                printf("Unknown relocatation type %ld address %ld\n",
                       pReloc->Type, pReloc->VirtualAddress);
        }

        pReloc++;
    }
}

int main(int argc, char *argv[])
{
    char *pszSourceFile;
    char *pszDestFile;
    unsigned long nFileSize, nBaseAddress;
    FILE *pSourceFile, *pDestFile;
    IMAGE_FILE_HEADER *pFileHeader;
    IMAGE_SECTION_HEADER *pSectionHeader;
    unsigned int i;
    char *pData;
    PIMAGE_SYMBOL pSymbols;

    if ((argc != 4) || (strcmp(argv[1], "--help") == 0))
    {
        Usage();
        return -1;
    }

    pszSourceFile = argv[1];
    pszDestFile = argv[2];
    nBaseAddress = strtol(argv[3], 0, 16);

    pSourceFile = fopen(pszSourceFile, "rb");
    if (!pSourceFile)
    {
        fprintf(stderr, "Couldn't open source file '%s'\n", pszSourceFile);
        return -2;
    }

    /* Get file size */
    fseek(pSourceFile, 0, SEEK_END);
    nFileSize = ftell(pSourceFile);
    rewind(pSourceFile);

    /* Allocate memory for the file */
    pData = malloc(nFileSize);
    if (!pData)
    {
        fprintf(stderr, "Failed to allocate %ld bytes\n", nFileSize);
        return -3;
    }

    /* Read the whole source file */
    if (!fread(pData, nFileSize, 1, pSourceFile))
    {
        fprintf(stderr, "Failed to read source file: %ld\n", nFileSize);
        return -4;
    }

    /* Close source file */
    fclose(pSourceFile);

    /* Open the destination file */
    pDestFile = fopen(pszDestFile, "wb");
    if (!pszDestFile)
    {
        fprintf(stderr, "Couldn't open dest file '%s'\n", pszDestFile);
        return -5;
    }

    /* Calculate table pointers */
    pFileHeader = (IMAGE_FILE_HEADER*)pData;
    pSymbols = (void*)(pData + pFileHeader->PointerToSymbolTable);
    pSectionHeader = (void*)(((char*)(pFileHeader + 1)) + pFileHeader->SizeOfOptionalHeader);

    /* Loop all sections */
    for (i = 0; i < pFileHeader->NumberOfSections; i++)
    {
        /* Check if this is '.text' section */
        if ((strcmp(pSectionHeader->Name, ".text") == 0) &&
            (pSectionHeader->SizeOfRawData != 0))
        {
            RelocateSection(pData,
                            pSectionHeader,
                            pSymbols,
                            nBaseAddress);

            /* Write the section to the destination file */
            if (!fwrite(pData + pSectionHeader->PointerToRawData,
                        pSectionHeader->SizeOfRawData, 1, pDestFile))
            {
                fprintf(stderr, "Failed to write data %ld\n",
                        pSectionHeader->SizeOfRawData);
                return -6;
            }

            nBaseAddress += pSectionHeader->SizeOfRawData;
        }

        pSectionHeader++;
    }

    fclose(pDestFile);

    return 0;
}

