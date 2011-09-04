/* $Id: nls.c 52854 2011-07-24 23:42:09Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            dll/win32/kernel32/misc/nls.c
 * PURPOSE:         National Language Support
 * PROGRAMMER:      Filip Navara
 *                  Hartmut Birr
 *                  Gunnar Andre Dalsnes
 *                  Thomas Weidenmueller
 * UPDATE HISTORY:
 *                  Created 24/08/2004
 */

/* INCLUDES *******************************************************************/

#include <k32.h>
#define NDEBUG
#include <debug.h>

/* GLOBAL VARIABLES ***********************************************************/

/* Sequence length based on the first character. */
static const char UTF8Length[128] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x80 - 0x8F */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0x90 - 0x9F */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xA0 - 0xAF */
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 0xB0 - 0xBF */
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0xC0 - 0xCF */
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /* 0xD0 - 0xDF */
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, /* 0xE0 - 0xEF */
   3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 0, 0  /* 0xF0 - 0xFF */
};

/* First byte mask depending on UTF-8 sequence length. */
static const unsigned char UTF8Mask[6] = {0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01};

/* FIXME: Change to HASH table or linear array. */
static LIST_ENTRY CodePageListHead;
static CODEPAGE_ENTRY AnsiCodePage;
static CODEPAGE_ENTRY OemCodePage;
static RTL_CRITICAL_SECTION CodePageListLock;

/* FORWARD DECLARATIONS *******************************************************/

BOOL WINAPI
GetNlsSectionName(UINT CodePage, UINT Base, ULONG Unknown,
                  LPSTR BaseName, LPSTR Result, ULONG ResultSize);

BOOL WINAPI
GetCPFileNameFromRegistry(UINT CodePage, LPWSTR FileName, ULONG FileNameSize);

/* PRIVATE FUNCTIONS **********************************************************/

/**
 * @name NlsInit
 *
 * Internal NLS related stuff initialization.
 */

BOOL
FASTCALL
NlsInit(VOID)
{
    UNICODE_STRING DirName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;

    InitializeListHead(&CodePageListHead);
    RtlInitializeCriticalSection(&CodePageListLock);

    /*
     * FIXME: Eventually this should be done only for the NLS Server
     * process, but since we don't have anything like that (yet?) we
     * always try to create the "\Nls" directory here.
     */
    RtlInitUnicodeString(&DirName, L"\\Nls");

    InitializeObjectAttributes(&ObjectAttributes,
                               &DirName,
                               OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                               NULL,
                               NULL);

    if (NT_SUCCESS(NtCreateDirectoryObject(&Handle, DIRECTORY_ALL_ACCESS, &ObjectAttributes)))
    {
        NtClose(Handle);
    }

    /* Setup ANSI code page. */
    AnsiCodePage.CodePage = CP_ACP;
    AnsiCodePage.SectionHandle = NULL;
    AnsiCodePage.SectionMapping = NtCurrentTeb()->ProcessEnvironmentBlock->AnsiCodePageData;

    RtlInitCodePageTable((PUSHORT)AnsiCodePage.SectionMapping,
                         &AnsiCodePage.CodePageTable);
    InsertTailList(&CodePageListHead, &AnsiCodePage.Entry);

    /* Setup OEM code page. */
    OemCodePage.CodePage = CP_OEMCP;
    OemCodePage.SectionHandle = NULL;
    OemCodePage.SectionMapping = NtCurrentTeb()->ProcessEnvironmentBlock->OemCodePageData;

    RtlInitCodePageTable((PUSHORT)OemCodePage.SectionMapping,
                         &OemCodePage.CodePageTable);
    InsertTailList(&CodePageListHead, &OemCodePage.Entry);

    return TRUE;
}

/**
 * @name NlsUninit
 *
 * Internal NLS related stuff uninitialization.
 */

VOID
FASTCALL
NlsUninit(VOID)
{
    PCODEPAGE_ENTRY Current;

    /* Delete the code page list. */
    while (!IsListEmpty(&CodePageListHead))
    {
        Current = CONTAINING_RECORD(CodePageListHead.Flink, CODEPAGE_ENTRY, Entry);
        if (Current->SectionHandle != NULL)
        {
            UnmapViewOfFile(Current->SectionMapping);
            NtClose(Current->SectionHandle);
        }
        RemoveHeadList(&CodePageListHead);
    }
    RtlDeleteCriticalSection(&CodePageListLock);
}

/**
 * @name IntGetLoadedCodePageEntry
 *
 * Internal function to get structure containing a code page information
 * of code page that is already loaded.
 *
 * @param CodePage
 *        Number of the code page. Special values like CP_OEMCP, CP_ACP
 *        or CP_UTF8 aren't allowed.
 *
 * @return Code page entry or NULL if the specified code page hasn't
 *         been loaded yet.
 */

PCODEPAGE_ENTRY
FASTCALL
IntGetLoadedCodePageEntry(UINT CodePage)
{
    LIST_ENTRY *CurrentEntry;
    PCODEPAGE_ENTRY Current;

    RtlEnterCriticalSection(&CodePageListLock);
    for (CurrentEntry = CodePageListHead.Flink;
         CurrentEntry != &CodePageListHead;
         CurrentEntry = CurrentEntry->Flink)
    {
        Current = CONTAINING_RECORD(CurrentEntry, CODEPAGE_ENTRY, Entry);
        if (Current->CodePage == CodePage)
        {
            RtlLeaveCriticalSection(&CodePageListLock);
            return Current;
        }
    }
    RtlLeaveCriticalSection(&CodePageListLock);

    return NULL;
}

/**
 * @name IntGetCodePageEntry
 *
 * Internal function to get structure containing a code page information.
 *
 * @param CodePage
 *        Number of the code page. Special values like CP_OEMCP, CP_ACP
 *        or CP_THREAD_ACP are allowed, but CP_UTF[7/8] isn't.
 *
 * @return Code page entry.
 */

PCODEPAGE_ENTRY
FASTCALL
IntGetCodePageEntry(UINT CodePage)
{
    CHAR SectionName[40];
    NTSTATUS Status;
    HANDLE SectionHandle = INVALID_HANDLE_VALUE, FileHandle;
    PBYTE SectionMapping;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ANSI_STRING AnsiName;
    UNICODE_STRING UnicodeName;
    WCHAR FileName[MAX_PATH + 1];
    UINT FileNamePos;
    PCODEPAGE_ENTRY CodePageEntry;

    if (CodePage == CP_THREAD_ACP)
    {
        if (!GetLocaleInfoW(GetThreadLocale(),
                            LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,
                            (WCHAR *)&CodePage,
                            sizeof(CodePage) / sizeof(WCHAR)))
        {
            /* Last error is set by GetLocaleInfoW. */
            return NULL;
        }
    }
    else if (CodePage == CP_MACCP)
    {
        if (!GetLocaleInfoW(LOCALE_SYSTEM_DEFAULT,
                            LOCALE_IDEFAULTMACCODEPAGE | LOCALE_RETURN_NUMBER,
                            (WCHAR *)&CodePage,
                            sizeof(CodePage) / sizeof(WCHAR)))
        {
            /* Last error is set by GetLocaleInfoW. */
            return NULL;
        }
    }

    /* Try searching for loaded page first. */
    CodePageEntry = IntGetLoadedCodePageEntry(CodePage);
    if (CodePageEntry != NULL)
    {
        return CodePageEntry;
    }

    /*
     * Yes, we really want to lock here. Otherwise it can happen that
     * two parallel requests will try to get the entry for the same
     * code page and we would load it twice.
     */
    RtlEnterCriticalSection(&CodePageListLock);

    /* Generate the section name. */
    if (!GetNlsSectionName(CodePage,
                           10,
                           0,
                           "\\Nls\\NlsSectionCP",
                           SectionName,
                           sizeof(SectionName)))
    {
        RtlLeaveCriticalSection(&CodePageListLock);
        return NULL;
    }

    RtlInitAnsiString(&AnsiName, SectionName);
    RtlAnsiStringToUnicodeString(&UnicodeName, &AnsiName, TRUE);

    InitializeObjectAttributes(&ObjectAttributes, &UnicodeName, 0, NULL, NULL);

    /* Try to open the section first */
    Status = NtOpenSection(&SectionHandle, SECTION_MAP_READ, &ObjectAttributes);

    /* If the section doesn't exist, try to create it. */
    if (Status == STATUS_UNSUCCESSFUL ||
        Status == STATUS_OBJECT_NAME_NOT_FOUND ||
        Status == STATUS_OBJECT_PATH_NOT_FOUND)
    {
        FileNamePos = GetSystemDirectoryW(FileName, MAX_PATH);
        if (GetCPFileNameFromRegistry(CodePage,
                                      FileName + FileNamePos + 1,
                                      MAX_PATH - FileNamePos - 1))
        {
            FileName[FileNamePos] = L'\\';
            FileName[MAX_PATH] = 0;
            FileHandle = CreateFileW(FileName,
                                     FILE_GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     0,
                                     NULL);

            Status = NtCreateSection(&SectionHandle,
                                     SECTION_MAP_READ,
                                     &ObjectAttributes,
                                     NULL,
                                     PAGE_READONLY,
                                     SEC_COMMIT,
                                     FileHandle);

            /* HACK: Check if another process was faster
             * and already created this section. See bug 3626 for details */
            if (Status == STATUS_OBJECT_NAME_COLLISION)
            {
                /* Close the file then */
                NtClose(FileHandle);

                /* And open the section */
                Status = NtOpenSection(&SectionHandle,
                                       SECTION_MAP_READ,
                                       &ObjectAttributes);
            }
        }
    }
    RtlFreeUnicodeString(&UnicodeName);

    if (!NT_SUCCESS(Status))
    {
        RtlLeaveCriticalSection(&CodePageListLock);
        return NULL;
    }

    SectionMapping = MapViewOfFile(SectionHandle, FILE_MAP_READ, 0, 0, 0);
    if (SectionMapping == NULL)
    {
        NtClose(SectionHandle);
        RtlLeaveCriticalSection(&CodePageListLock);
        return NULL;
    }

    CodePageEntry = HeapAlloc(GetProcessHeap(), 0, sizeof(CODEPAGE_ENTRY));
    if (CodePageEntry == NULL)
    {
        NtClose(SectionHandle);
        RtlLeaveCriticalSection(&CodePageListLock);
        return NULL;
    }

    CodePageEntry->CodePage = CodePage;
    CodePageEntry->SectionHandle = SectionHandle;
    CodePageEntry->SectionMapping = SectionMapping;

    RtlInitCodePageTable((PUSHORT)SectionMapping, &CodePageEntry->CodePageTable);

    /* Insert the new entry to list and unlock. Uff. */
    InsertTailList(&CodePageListHead, &CodePageEntry->Entry);
    RtlLeaveCriticalSection(&CodePageListLock);

    return CodePageEntry;
}

/**
 * @name IntMultiByteToWideCharUTF8
 *
 * Internal version of MultiByteToWideChar for UTF8.
 *
 * @see MultiByteToWideChar
 * @todo Add UTF8 validity checks.
 */

static
INT
WINAPI
IntMultiByteToWideCharUTF8(DWORD Flags,
                           LPCSTR MultiByteString,
                           INT MultiByteCount,
                           LPWSTR WideCharString,
                           INT WideCharCount)
{
    LPCSTR MbsEnd;
    UCHAR Char, Length;
    WCHAR WideChar;
    LONG Count;

    if (Flags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return 0;
    }

    /* Does caller query for output buffer size? */
    if (WideCharCount == 0)
    {
        MbsEnd = MultiByteString + MultiByteCount;
        for (; MultiByteString < MbsEnd; WideCharCount++)
        {
            Char = *MultiByteString++;
            if (Char < 0xC0)
                continue;
            MultiByteString += UTF8Length[Char - 0x80];
        }
        return WideCharCount;
    }

    MbsEnd = MultiByteString + MultiByteCount;
    for (Count = 0; Count < WideCharCount && MultiByteString < MbsEnd; Count++)
    {
        Char = *MultiByteString++;
        if (Char < 0x80)
        {
            *WideCharString++ = Char;
            continue;
        }
        Length = UTF8Length[Char - 0x80];
        WideChar = Char & UTF8Mask[Length];
        while (Length && MultiByteString < MbsEnd)
        {
            WideChar = (WideChar << 6) | (*MultiByteString++ & 0x7f);
            Length--;
        }
        *WideCharString++ = WideChar;
    }

    if (MultiByteString < MbsEnd)
        SetLastError(ERROR_INSUFFICIENT_BUFFER);

    return Count;
}

/**
 * @name IntMultiByteToWideCharCP
 *
 * Internal version of MultiByteToWideChar for code page tables.
 *
 * @see MultiByteToWideChar
 * @todo Handle MB_PRECOMPOSED, MB_COMPOSITE, MB_USEGLYPHCHARS and
 *       DBCS codepages.
 */

static
INT
WINAPI
IntMultiByteToWideCharCP(UINT CodePage,
                         DWORD Flags,
                         LPCSTR MultiByteString,
                         INT MultiByteCount,
                         LPWSTR WideCharString,
                         INT WideCharCount)
{
    PCODEPAGE_ENTRY CodePageEntry;
    PCPTABLEINFO CodePageTable;
    LPCSTR TempString;
    INT TempLength;

    /* Get code page table. */
    CodePageEntry = IntGetCodePageEntry(CodePage);
    if (CodePageEntry == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    CodePageTable = &CodePageEntry->CodePageTable;

    /* Different handling for DBCS code pages. */
    if (CodePageTable->MaximumCharacterSize > 1)
    {
        /* FIXME */

        UCHAR Char;
        USHORT DBCSOffset;
        LPCSTR MbsEnd = MultiByteString + MultiByteCount;
        INT Count;

        /* Does caller query for output buffer size? */
        if (WideCharCount == 0)
        {
            for (; MultiByteString < MbsEnd; WideCharCount++)
            {
                Char = *MultiByteString++;

                if (Char < 0x80)
                    continue;

                DBCSOffset = CodePageTable->DBCSOffsets[Char];

                if (!DBCSOffset)
                    continue;

                if (MultiByteString < MbsEnd)
                    MultiByteString++;
            }

            return WideCharCount;
        }

        for (Count = 0; Count < WideCharCount && MultiByteString < MbsEnd; Count++)
        {
            Char = *MultiByteString++;

            if (Char < 0x80)
            {
                *WideCharString++ = Char;
                continue;
            }

            DBCSOffset = CodePageTable->DBCSOffsets[Char];

            if (!DBCSOffset)
            {
                *WideCharString++ = CodePageTable->MultiByteTable[Char];
                continue;
            }

            if (MultiByteString < MbsEnd)
                *WideCharString++ = CodePageTable->DBCSOffsets[DBCSOffset + *(PUCHAR)MultiByteString++];
        }

        if (MultiByteString < MbsEnd)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return 0;
        }

        return Count;
    }
    else /* Not DBCS code page */
    {
        /* Check for invalid characters. */
        if (Flags & MB_ERR_INVALID_CHARS)
        {
            for (TempString = MultiByteString, TempLength = MultiByteCount;
                 TempLength > 0;
                 TempString++, TempLength--)
            {
                if (CodePageTable->MultiByteTable[(UCHAR)*TempString] ==
                    CodePageTable->UniDefaultChar &&
                    *TempString != CodePageEntry->CodePageTable.DefaultChar)
                {
                    SetLastError(ERROR_NO_UNICODE_TRANSLATION);
                    return 0;
                }
            }
        }

        /* Does caller query for output buffer size? */
        if (WideCharCount == 0)
            return MultiByteCount;

        /* Fill the WideCharString buffer with what will fit: Verified on WinXP */
        for (TempLength = (WideCharCount < MultiByteCount) ? WideCharCount : MultiByteCount;
            TempLength > 0;
            MultiByteString++, TempLength--)
        {
            *WideCharString++ = CodePageTable->MultiByteTable[(UCHAR)*MultiByteString];
        }

        /* Adjust buffer size. Wine trick ;-) */
        if (WideCharCount < MultiByteCount)
        {
            MultiByteCount = WideCharCount;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return 0;
        }
	    return MultiByteCount;
    }
}

/**
 * @name IntMultiByteToWideCharSYMBOL
 *
 * Internal version of MultiByteToWideChar for SYMBOL.
 *
 * @see MultiByteToWideChar
 */

static
INT
WINAPI 
IntMultiByteToWideCharSYMBOL(DWORD Flags, 
                             LPCSTR MultiByteString,
                             INT MultiByteCount, 
                             LPWSTR WideCharString,
                             INT WideCharCount)
{
    LONG Count;
    UCHAR Char;
    INT WideCharMaxLen;


    if (Flags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return 0;
    }

    if (WideCharCount == 0) 
    {
        return MultiByteCount;
    }

    WideCharMaxLen = WideCharCount > MultiByteCount ? MultiByteCount : WideCharCount;

    for (Count = 0; Count < WideCharMaxLen; Count++)
    {
        Char = MultiByteString[Count];
        if ( Char < 0x20 )
        {
            WideCharString[Count] = Char;
        }
        else
        {
            WideCharString[Count] = Char + 0xf000;
        }
    }
    if (MultiByteCount > WideCharMaxLen) 
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

    return WideCharMaxLen;
}

/**
 * @name IntWideCharToMultiByteSYMBOL
 *
 * Internal version of WideCharToMultiByte for SYMBOL.
 *
 * @see WideCharToMultiByte
 */

static INT
WINAPI
IntWideCharToMultiByteSYMBOL(DWORD Flags,
                             LPCWSTR WideCharString,
                             INT WideCharCount,
                             LPSTR MultiByteString,
                             INT MultiByteCount)
{
    LONG Count;
    INT MaxLen;
    WCHAR Char;

    if (Flags!=0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }


    if (MultiByteCount == 0) 
    {
        return WideCharCount;
    }

    MaxLen = MultiByteCount > WideCharCount ? WideCharCount : MultiByteCount;
    for (Count = 0; Count < MaxLen; Count++)
    {
        Char = WideCharString[Count];
        if (Char < 0x20)
        {
            MultiByteString[Count] = Char;
        }
        else 
        {
            if ((Char>=0xf020)&&(Char<0xf100))
            {
                MultiByteString[Count] = Char - 0xf000;
            }
            else
            {
                SetLastError(ERROR_NO_UNICODE_TRANSLATION);
                return 0;
            }
        }
    }
    if (WideCharCount > MaxLen) 
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }
    return MaxLen;
}

/**
 * @name IntWideCharToMultiByteUTF8
 *
 * Internal version of WideCharToMultiByte for UTF8.
 *
 * @see WideCharToMultiByte
 */

static INT
WINAPI
IntWideCharToMultiByteUTF8(UINT CodePage,
                           DWORD Flags,
                           LPCWSTR WideCharString,
                           INT WideCharCount,
                           LPSTR MultiByteString,
                           INT MultiByteCount,
                           LPCSTR DefaultChar,
                           LPBOOL UsedDefaultChar)
{
    INT TempLength;
    WCHAR Char;

    /* Does caller query for output buffer size? */
    if (MultiByteCount == 0)
    {
        for (TempLength = 0; WideCharCount;
            WideCharCount--, WideCharString++)
        {
            TempLength++;
            if (*WideCharString >= 0x80)
            {
                TempLength++;
                if (*WideCharString >= 0x800)
                    TempLength++;
            }
        }
        return TempLength;
    }

    for (TempLength = MultiByteCount; WideCharCount; WideCharCount--, WideCharString++)
    {
        Char = *WideCharString;
        if (Char < 0x80)
        {
            if (!TempLength)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                break;
            }
            TempLength--;
            *MultiByteString++ = (CHAR)Char;
            continue;
        }

        if (Char < 0x800)  /* 0x80-0x7ff: 2 bytes */
        {
            if (TempLength < 2)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                break;
            }
            MultiByteString[1] = 0x80 | (Char & 0x3f); Char >>= 6;
            MultiByteString[0] = 0xc0 | Char;
            MultiByteString += 2;
            TempLength -= 2;
            continue;
        }

        /* 0x800-0xffff: 3 bytes */
        if (TempLength < 3)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            break;
        }
        MultiByteString[2] = 0x80 | (Char & 0x3f); Char >>= 6;
        MultiByteString[1] = 0x80 | (Char & 0x3f); Char >>= 6;
        MultiByteString[0] = 0xe0 | Char;
        MultiByteString += 3;
        TempLength -= 3;
    }

    return MultiByteCount - TempLength;
}

/**
 * @name IsValidSBCSMapping
 *
 * Checks if ch (single-byte character) is a valid mapping for wch
 *
 * @see IntWideCharToMultiByteCP
 */
static
inline
BOOL
IntIsValidSBCSMapping(PCPTABLEINFO CodePageTable, DWORD Flags, WCHAR wch, UCHAR ch)
{
    /* If the WC_NO_BEST_FIT_CHARS flag has been specified, the characters need to match exactly. */
    if (Flags & WC_NO_BEST_FIT_CHARS)
        return (CodePageTable->MultiByteTable[ch] == wch);

    /* By default, all characters except TransDefaultChar apply as a valid mapping
       for ch (so also "nearest" characters) */
    if (ch != CodePageTable->TransDefaultChar)
        return TRUE;

    /* The only possible left valid mapping is the default character itself */
    return (wch == CodePageTable->TransUniDefaultChar);
}

/**
 * @name IsValidDBCSMapping
 *
 * Checks if ch (double-byte character) is a valid mapping for wch
 *
 * @see IntWideCharToMultiByteCP
 */
static inline BOOL
IntIsValidDBCSMapping(PCPTABLEINFO CodePageTable, DWORD Flags, WCHAR wch, USHORT ch)
{
    /* If ch is the default character, but the wch is not, it can't be a valid mapping */
    if (ch == CodePageTable->TransDefaultChar && wch != CodePageTable->TransUniDefaultChar)
        return FALSE;

    /* If the WC_NO_BEST_FIT_CHARS flag has been specified, the characters need to match exactly. */
    if (Flags & WC_NO_BEST_FIT_CHARS)
    {
        if(ch & 0xff00)
        {
            USHORT uOffset = CodePageTable->DBCSOffsets[ch >> 8];
            /* if (!uOffset) return (CodePageTable->MultiByteTable[ch] == wch); */
            return (CodePageTable->DBCSOffsets[uOffset + (ch & 0xff)] == wch);
        }

        return (CodePageTable->MultiByteTable[ch] == wch);
    }

    /* If we're still here, we have a valid mapping */
    return TRUE;
}

/**
 * @name IntWideCharToMultiByteCP
 *
 * Internal version of WideCharToMultiByte for code page tables.
 *
 * @see WideCharToMultiByte
 * @todo Handle WC_COMPOSITECHECK
 */
static
INT
WINAPI
IntWideCharToMultiByteCP(UINT CodePage,
                         DWORD Flags,
                         LPCWSTR WideCharString,
                         INT WideCharCount,
                         LPSTR MultiByteString,
                         INT MultiByteCount,
                         LPCSTR DefaultChar,
                         LPBOOL UsedDefaultChar)
{
    PCODEPAGE_ENTRY CodePageEntry;
    PCPTABLEINFO CodePageTable;
    INT TempLength;

    /* Get code page table. */
    CodePageEntry = IntGetCodePageEntry(CodePage);
    if (CodePageEntry == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }
    CodePageTable = &CodePageEntry->CodePageTable;


    /* Different handling for DBCS code pages. */
    if (CodePageTable->MaximumCharacterSize > 1)
    {
        /* If Flags, DefaultChar or UsedDefaultChar were given, we have to do some more work */
        if(Flags || DefaultChar || UsedDefaultChar)
        {
            BOOL TempUsedDefaultChar;
            USHORT DefChar;

            /* If UsedDefaultChar is not set, set it to a temporary value, so we don't have
               to check on every character */
            if(!UsedDefaultChar)
                UsedDefaultChar = &TempUsedDefaultChar;

            *UsedDefaultChar = FALSE;

            /* Use the CodePage's TransDefaultChar if none was given. Don't modify the DefaultChar pointer here. */
            if(DefaultChar)
                DefChar = DefaultChar[1] ? ((DefaultChar[0] << 8) | DefaultChar[1]) : DefaultChar[0];
            else
                DefChar = CodePageTable->TransDefaultChar;

            /* Does caller query for output buffer size? */
            if(!MultiByteCount)
            {
                for(TempLength = 0; WideCharCount; WideCharCount--, WideCharString++, TempLength++)
                {
                    USHORT uChar;

                    if ((Flags & WC_COMPOSITECHECK) && WideCharCount > 1)
                    {
                        /* FIXME: Handle WC_COMPOSITECHECK */
                    }

                    uChar = ((PUSHORT) CodePageTable->WideCharTable)[*WideCharString];

                    /* Verify if the mapping is valid for handling DefaultChar and UsedDefaultChar */
                    if (!IntIsValidDBCSMapping(CodePageTable, Flags, *WideCharString, uChar))
                    {
                        uChar = DefChar;
                        *UsedDefaultChar = TRUE;
                    }

                    /* Increment TempLength again if this is a double-byte character */
                    if (uChar & 0xff00)
                        TempLength++;
                }

                return TempLength;
            }

            /* Convert the WideCharString to the MultiByteString and verify if the mapping is valid */
            for(TempLength = MultiByteCount;
                WideCharCount && TempLength;
                TempLength--, WideCharString++, WideCharCount--)
            {
                USHORT uChar;

                if ((Flags & WC_COMPOSITECHECK) && WideCharCount > 1)
                {
                    /* FIXME: Handle WC_COMPOSITECHECK */
                }

                uChar = ((PUSHORT)CodePageTable->WideCharTable)[*WideCharString];

                /* Verify if the mapping is valid for handling DefaultChar and UsedDefaultChar */
                if (!IntIsValidDBCSMapping(CodePageTable, Flags, *WideCharString, uChar))
                {
                    uChar = DefChar;
                    *UsedDefaultChar = TRUE;
                }

                /* Handle double-byte characters */
                if (uChar & 0xff00)
                {
                    /* Don't output a partial character */
                    if (TempLength == 1)
                        break;

                    TempLength--;
                    *MultiByteString++ = uChar >> 8;
                }

                *MultiByteString++ = (char)uChar;
            }

            /* WideCharCount should be 0 if all characters were converted */
            if (WideCharCount)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return 0;
            }

            return MultiByteCount - TempLength;
        }

        /* Does caller query for output buffer size? */
        if (!MultiByteCount)
        {
            for (TempLength = 0; WideCharCount; WideCharCount--, WideCharString++, TempLength++)
            {
                /* Increment TempLength again if this is a double-byte character */
                if (((PWCHAR)CodePageTable->WideCharTable)[*WideCharString] & 0xff00)
                    TempLength++;
            }

            return TempLength;
        }

        /* Convert the WideCharString to the MultiByteString */
        for (TempLength = MultiByteCount;
             WideCharCount && TempLength;
             TempLength--, WideCharString++, WideCharCount--)
        {
            USHORT uChar = ((PUSHORT) CodePageTable->WideCharTable)[*WideCharString];

            /* Is this a double-byte character? */
            if (uChar & 0xff00)
            {
                /* Don't output a partial character */
                if (TempLength == 1)
                    break;

                TempLength--;
                *MultiByteString++ = uChar >> 8;
            }

            *MultiByteString++ = (char)uChar;
        }

        /* WideCharCount should be 0 if all characters were converted */
        if (WideCharCount)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return 0;
        }

        return MultiByteCount - TempLength;
    }
    else /* Not DBCS code page */
    {
        INT nReturn;

        /* If Flags, DefaultChar or UsedDefaultChar were given, we have to do some more work */
        if (Flags || DefaultChar || UsedDefaultChar)
        {
            BOOL TempUsedDefaultChar;
            CHAR DefChar;

            /* If UsedDefaultChar is not set, set it to a temporary value, so we don't have
               to check on every character */
            if (!UsedDefaultChar)
                UsedDefaultChar = &TempUsedDefaultChar;

            *UsedDefaultChar = FALSE;

            /* Does caller query for output buffer size? */
            if (!MultiByteCount)
            {
                /* Loop through the whole WideCharString and check if we can get a valid mapping for each character */
                for (TempLength = 0; WideCharCount; TempLength++, WideCharString++, WideCharCount--)
                {
                    if ((Flags & WC_COMPOSITECHECK) && WideCharCount > 1)
                    {
                        /* FIXME: Handle WC_COMPOSITECHECK */
                    }

                    if (!*UsedDefaultChar)
                        *UsedDefaultChar = !IntIsValidSBCSMapping(CodePageTable,
                                                                  Flags,
                                                                  *WideCharString,
                                                                  ((PCHAR)CodePageTable->WideCharTable)[*WideCharString]);
                }

                return TempLength;
            }

            /* Use the CodePage's TransDefaultChar if none was given. Don't modify the DefaultChar pointer here. */
            if (DefaultChar)
                DefChar = *DefaultChar;
            else
                DefChar = CodePageTable->TransDefaultChar;

            /* Convert the WideCharString to the MultiByteString and verify if the mapping is valid */
            for (TempLength = MultiByteCount;
                 WideCharCount && TempLength;
                 MultiByteString++, TempLength--, WideCharString++, WideCharCount--)
            {
                if ((Flags & WC_COMPOSITECHECK) && WideCharCount > 1)
                {
                    /* FIXME: Handle WC_COMPOSITECHECK */
                }

                *MultiByteString = ((PCHAR)CodePageTable->WideCharTable)[*WideCharString];

                if (!IntIsValidSBCSMapping(CodePageTable, Flags, *WideCharString, *MultiByteString))
                {
                    *MultiByteString = DefChar;
                    *UsedDefaultChar = TRUE;
                }
            }

            /* WideCharCount should be 0 if all characters were converted */
            if (WideCharCount)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return 0;
            }

            return MultiByteCount - TempLength;
        }

        /* Does caller query for output buffer size? */
        if (!MultiByteCount)
            return WideCharCount;

        /* Is the buffer large enough? */
        if (MultiByteCount < WideCharCount)
        {
            /* Convert the string up to MultiByteCount and return 0 */
            WideCharCount = MultiByteCount;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            nReturn = 0;
        }
        else
        {
            /* Otherwise WideCharCount will be the number of converted characters */
            nReturn = WideCharCount;
        }

        /* Convert the WideCharString to the MultiByteString */
        for (TempLength = WideCharCount; --TempLength >= 0; WideCharString++, MultiByteString++)
        {
            *MultiByteString = ((PCHAR)CodePageTable->WideCharTable)[*WideCharString];
        }

        return nReturn;
    }
}

/**
 * @name IntIsLeadByte
 *
 * Internal function to detect if byte is lead byte in specific character
 * table.
 */

static BOOL
WINAPI
IntIsLeadByte(PCPTABLEINFO TableInfo, BYTE Byte)
{
    UINT i;

    if (TableInfo->MaximumCharacterSize == 2)
    {
        for (i = 0; i < MAXIMUM_LEADBYTES && TableInfo->LeadByte[i]; i += 2)
        {
            if (Byte >= TableInfo->LeadByte[i] && Byte <= TableInfo->LeadByte[i+1])
                return TRUE;
        }
    }

    return FALSE;
}

/* PUBLIC FUNCTIONS ***********************************************************/

/**
 * @name GetNlsSectionName
 *
 * Construct a name of NLS section.
 *
 * @param CodePage
 *        Code page number.
 * @param Base
 *        Integer base used for converting to string. Usually set to 10.
 * @param Unknown
 *        As the name suggests the meaning of this parameter is unknown.
 *        The native version of Kernel32 passes it as the third parameter
 *        to NlsConvertIntegerToString function, which is used for the
 *        actual conversion of the code page number.
 * @param BaseName
 *        Base name of the section. (ex. "\\Nls\\NlsSectionCP")
 * @param Result
 *        Buffer that will hold the constructed name.
 * @param ResultSize
 *        Size of the buffer for the result.
 *
 * @return TRUE if the buffer was large enough and was filled with
 *         the requested information, FALSE otherwise.
 *
 * @implemented
 */

BOOL
WINAPI
GetNlsSectionName(UINT CodePage,
                  UINT Base,
                  ULONG Unknown,
                  LPSTR BaseName,
                  LPSTR Result,
                  ULONG ResultSize)
{
    CHAR Integer[11];

    if (!NT_SUCCESS(RtlIntegerToChar(CodePage, Base, sizeof(Integer), Integer)))
        return FALSE;

    /*
     * If the name including the terminating NULL character doesn't
     * fit in the output buffer then fail.
     */
    if (strlen(Integer) + strlen(BaseName) >= ResultSize)
        return FALSE;

    lstrcpyA(Result, BaseName);
    lstrcatA(Result, Integer);

    return TRUE;
}

/**
 * @name GetCPFileNameFromRegistry
 *
 * Get file name of code page definition file.
 *
 * @param CodePage
 *        Code page number to get file name of.
 * @param FileName
 *        Buffer that is filled with file name of successful return. Can
 *        be set to NULL.
 * @param FileNameSize
 *        Size of the buffer to hold file name in WCHARs.
 *
 * @return TRUE if the file name was retrieved, FALSE otherwise.
 *
 * @implemented
 */

BOOL
WINAPI
GetCPFileNameFromRegistry(UINT CodePage, LPWSTR FileName, ULONG FileNameSize)
{
    WCHAR ValueNameBuffer[11];
    UNICODE_STRING KeyName, ValueName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION Kvpi;
    DWORD KvpiSize;
    BOOL bRetValue;

    bRetValue = FALSE;

    /* Convert the codepage number to string. */
    ValueName.Buffer = ValueNameBuffer;
    ValueName.MaximumLength = sizeof(ValueNameBuffer);

    if (!NT_SUCCESS(RtlIntegerToUnicodeString(CodePage, 10, &ValueName)))
        return bRetValue;

    /* Open the registry key containing file name mappings. */
    RtlInitUnicodeString(&KeyName, L"\\Registry\\Machine\\System\\"
                         L"CurrentControlSet\\Control\\Nls\\CodePage");
    InitializeObjectAttributes(&ObjectAttributes, &KeyName, OBJ_CASE_INSENSITIVE,
                               NULL, NULL);
    Status = NtOpenKey(&KeyHandle, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        return bRetValue;
    }

    /* Allocate buffer that will be used to query the value data. */
    KvpiSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + (MAX_PATH * sizeof(WCHAR));
    Kvpi = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, KvpiSize);
    if (Kvpi == NULL)
    {
        NtClose(KeyHandle);
        return bRetValue;
    }

    /* Query the file name for our code page. */
    Status = NtQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation,
                             Kvpi, KvpiSize, &KvpiSize);

    NtClose(KeyHandle);

    /* Check if we succeded and the value is non-empty string. */
    if (NT_SUCCESS(Status) && Kvpi->Type == REG_SZ &&
        Kvpi->DataLength > sizeof(WCHAR))
    {
        bRetValue = TRUE;
        if (FileName != NULL)
        {
            lstrcpynW(FileName, (WCHAR*)Kvpi->Data,
                      min(Kvpi->DataLength / sizeof(WCHAR), FileNameSize));
        }
    }

    /* free temporary buffer */
    HeapFree(GetProcessHeap(),0,Kvpi);
    return bRetValue;
}

/**
 * @name IsValidCodePage
 *
 * Detect if specified code page is valid and present in the system.
 *
 * @param CodePage
 *        Code page number to query.
 *
 * @return TRUE if code page is present.
 */

BOOL
WINAPI
IsValidCodePage(UINT CodePage)
{
    if (CodePage == 0) return FALSE;
    if (CodePage == CP_UTF8 || CodePage == CP_UTF7)
        return TRUE;
    if (IntGetLoadedCodePageEntry(CodePage))
        return TRUE;
    return GetCPFileNameFromRegistry(CodePage, NULL, 0);
}

static const signed char 
base64inv[] = 
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, 
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

static VOID Utf7Base64Decode(BYTE *pbDest, LPCSTR pszSrc, INT cchSrc)
{
    INT i, j, n;
    BYTE b;

    for(i = 0; i < cchSrc / 4 * 4; i += 4)
    {
        for(j = n = 0; j < 4; )
        {
            b = (BYTE) base64inv[(BYTE) *pszSrc++];
            n |= (((INT) b) << ((3 - j) * 6));
            j++;
        }
        for(j = 0; j < 3; j++)
            *pbDest++ = (BYTE) ((n >> (8 * (2 - j))) & 0xFF);
    }
    for(j = n = 0; j < cchSrc % 4; )
    {
        b = (BYTE) base64inv[(BYTE) *pszSrc++];
        n |= (((INT) b) << ((3 - j) * 6));
        j++;
    }
    for(j = 0; j < ((cchSrc % 4) * 6 / 8); j++)
        *pbDest++ = (BYTE) ((n >> (8 * (2 - j))) & 0xFF);
}

static VOID myswab(LPVOID pv, INT cw)
{
    LPBYTE pb = (LPBYTE) pv;
    BYTE b;
    while(cw > 0)
    {
        b = *pb;
        *pb = pb[1];
        pb[1] = b;
        pb += 2;
        cw--;
    }
}

static INT Utf7ToWideCharSize(LPCSTR pszUtf7, INT cchUtf7)
{
    INT n, c, cch;
    CHAR ch;
    LPCSTR pch;

    c = 0;
    while(cchUtf7 > 0)
    {
        ch = *pszUtf7++;
        if (ch == '+')
        {
            ch = *pszUtf7;
            if (ch == '-')
            {
                c++;
                pszUtf7++;
                cchUtf7 -= 2;
                continue;
            }
            cchUtf7--;
            pch = pszUtf7;
            while(cchUtf7 > 0 && (BYTE) *pszUtf7 < 0x80 && 
                  base64inv[*pszUtf7] >= 0)
            {
                cchUtf7--;
                pszUtf7++;
            }
            cch = pszUtf7 - pch;
            n = (cch * 3) / 8;
            c += n;
            if (cchUtf7 > 0 && *pszUtf7 == '-')
            {
                pszUtf7++;
                cchUtf7--;
            }
        }
        else
        {
            c++;
            cchUtf7--;
        }
    }

    return c;
}

static INT Utf7ToWideChar(LPCSTR pszUtf7, INT cchUtf7, LPWSTR pszWide, INT cchWide)
{
    INT n, c, cch;
    CHAR ch;
    LPCSTR pch;
    WORD *pwsz;

    c = Utf7ToWideCharSize(pszUtf7, cchUtf7);
    if (cchWide == 0)
        return c;

    if (cchWide < c)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

    while(cchUtf7 > 0)
    {
        ch = *pszUtf7++;
        if (ch == '+')
        {
            if (*pszUtf7 == '-')
            {
                *pszWide++ = L'+';
                pszUtf7++;
                cchUtf7 -= 2;
                continue;
            }
            cchUtf7--;
            pch = pszUtf7;
            while(cchUtf7 > 0 && (BYTE) *pszUtf7 < 0x80 && 
                  base64inv[*pszUtf7] >= 0)
            {
                cchUtf7--;
                pszUtf7++;
            }
            cch = pszUtf7 - pch;
            n = (cch * 3) / 8;
            pwsz = (WORD *) HeapAlloc(GetProcessHeap(), 0, (n + 1) * sizeof(WORD));
            if (pwsz == NULL)
                return 0;
            ZeroMemory(pwsz, n * sizeof(WORD));
            Utf7Base64Decode((BYTE *) pwsz, pch, cch);
            myswab(pwsz, n);
            CopyMemory(pszWide, pwsz, n * sizeof(WORD));
            HeapFree(GetProcessHeap(), 0, pwsz);
            pszWide += n;
            if (cchUtf7 > 0 && *pszUtf7 == '-')
            {
                pszUtf7++;
                cchUtf7--;
            }
        }
        else
        {
            *pszWide++ = (WCHAR) ch;
            cchUtf7--;
        }
    }

    return c;
}

/**
 * @name MultiByteToWideChar
 *
 * Convert a multi-byte string to wide-charater equivalent.
 *
 * @param CodePage
 *        Code page to be used to perform the conversion. It can be also
 *        one of the special values (CP_ACP for ANSI code page, CP_MACCP
 *        for Macintosh code page, CP_OEMCP for OEM code page, CP_THREAD_ACP
 *        for thread active code page, CP_UTF7 or CP_UTF8).
 * @param Flags
 *        Additional conversion flags (MB_PRECOMPOSED, MB_COMPOSITE,
 *        MB_ERR_INVALID_CHARS, MB_USEGLYPHCHARS).
 * @param MultiByteString
 *        Input buffer.
 * @param MultiByteCount
 *        Size of MultiByteString, or -1 if MultiByteString is NULL
 *        terminated.
 * @param WideCharString
 *        Output buffer.
 * @param WideCharCount
 *        Size in WCHARs of WideCharString, or 0 if the caller just wants
 *        to know how large WideCharString should be for a successful
 *        conversion.
 *
 * @return Zero on error, otherwise the number of WCHARs written
 *         in the WideCharString buffer.
 *
 * @implemented
 */

INT
WINAPI
MultiByteToWideChar(UINT CodePage,
                    DWORD Flags,
                    LPCSTR MultiByteString,
                    INT MultiByteCount,
                    LPWSTR WideCharString,
                    INT WideCharCount)
{
    /* Check the parameters. */
    if (MultiByteString == NULL ||
        (WideCharString == NULL && WideCharCount > 0) ||
        (PVOID)MultiByteString == (PVOID)WideCharString)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    /* Determine the input string length. */
    if (MultiByteCount < 0)
    {
        MultiByteCount = lstrlenA(MultiByteString) + 1;
    }

    switch (CodePage)
    {
        case CP_UTF8:
            return IntMultiByteToWideCharUTF8(Flags,
                                              MultiByteString,
                                              MultiByteCount,
                                              WideCharString,
                                              WideCharCount);

        case CP_UTF7:
            if (Flags)
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return 0;
            }
            return Utf7ToWideChar(MultiByteString, MultiByteCount,
                                  WideCharString, WideCharCount);

        case CP_SYMBOL:
            return IntMultiByteToWideCharSYMBOL(Flags,
                                                MultiByteString,
                                                MultiByteCount,
                                                WideCharString,
                                                WideCharCount);
        default:
            return IntMultiByteToWideCharCP(CodePage,
                                            Flags,
                                            MultiByteString,
                                            MultiByteCount,
                                            WideCharString,
                                            WideCharCount);
    }
}

static const char mustshift[] =
{
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1
};

static const char base64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static INT WideCharToUtf7Size(LPCWSTR pszWide, INT cchWide)
{
    WCHAR wch;
    INT c = 0;
    BOOL fShift = FALSE;

    while(cchWide > 0)
    {
        wch = *pszWide;
        if (wch < 0x80 && !mustshift[wch])
        {
            c++;
            cchWide--;
            pszWide++;
        }
        else
        {
            if (wch == L'+')
            {
                c++;
                c++;
                cchWide--;
                pszWide++;
                continue;
            }
            if (!fShift)
            {
                c++;
                fShift = TRUE;
            }
            pszWide++;
            cchWide--;
            c += 3;
            if (cchWide > 0 && (*pszWide >= 0x80 || mustshift[*pszWide]))
            {
                pszWide++;
                cchWide--;
                c += 3;
                if (cchWide > 0 && (*pszWide >= 0x80 || mustshift[*pszWide]))
                {
                    pszWide++;
                    cchWide--;
                    c += 2;
                }
            }
            if (cchWide > 0 && *pszWide < 0x80 && !mustshift[*pszWide])
            {
                c++;
                fShift = FALSE;
            }
        }
    }
    if (fShift)
        c++;

    return c;
}

static INT WideCharToUtf7(LPCWSTR pszWide, INT cchWide, LPSTR pszUtf7, INT cchUtf7)
{
    WCHAR wch;
    INT c, n;
    WCHAR wsz[3];
    BOOL fShift = FALSE;

    c = WideCharToUtf7Size(pszWide, cchWide);
    if (cchUtf7 == 0)
        return c;

    if (cchUtf7 < c)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return 0;
    }

    while(cchWide > 0)
    {
        wch = *pszWide;
        if (wch < 0x80 && !mustshift[wch])
        {
            *pszUtf7++ = (CHAR) wch;
            cchWide--;
            pszWide++;
        }
        else
        {
            if (wch == L'+')
            {
                *pszUtf7++ = '+';
                *pszUtf7++ = '-';
                cchWide--;
                pszWide++;
                continue;
            }
            if (!fShift)
            {
                *pszUtf7++ = '+';
                fShift = TRUE;
            }
            wsz[0] = *pszWide++;
            cchWide--;
            n = 1;
            if (cchWide > 0 && (*pszWide >= 0x80 || mustshift[*pszWide]))
            {
                wsz[1] = *pszWide++;
                cchWide--;
                n++;
                if (cchWide > 0 && (*pszWide >= 0x80 || mustshift[*pszWide]))
                {
                    wsz[2] = *pszWide++;
                    cchWide--;
                    n++;
                }
            }
            *pszUtf7++ = base64[wsz[0] >> 10];
            *pszUtf7++ = base64[(wsz[0] >> 4) & 0x3F];
            *pszUtf7++ = base64[(wsz[0] << 2 | wsz[1] >> 14) & 0x3F];
            if (n >= 2)
            {
                *pszUtf7++ = base64[(wsz[1] >> 8) & 0x3F];
                *pszUtf7++ = base64[(wsz[1] >> 2) & 0x3F];
                *pszUtf7++ = base64[(wsz[1] << 4 | wsz[2] >> 12) & 0x3F];
                if (n >= 3)
                {
                    *pszUtf7++ = base64[(wsz[2] >> 6) & 0x3F];
                    *pszUtf7++ = base64[wsz[2] & 0x3F];
                }
            }
            if (cchWide > 0 && *pszWide < 0x80 && !mustshift[*pszWide])
            {
                *pszUtf7++ = '-';
                fShift = FALSE;
            }
        }
    }
    if (fShift)
        *pszUtf7 = '-';

    return c;
}

static BOOL
GetLocalisedText(DWORD dwResId, WCHAR *lpszDest)
{
    HRSRC hrsrc;
    LCID lcid;
    LANGID langId;
    DWORD dwId;

    if (dwResId == 37)
        dwId = dwResId * 100;
    else
        dwId = dwResId;

    lcid = GetUserDefaultLCID();
    lcid = ConvertDefaultLocale(lcid);

    langId = LANGIDFROMLCID(lcid);

    if (PRIMARYLANGID(langId) == LANG_NEUTRAL)
        langId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    hrsrc = FindResourceExW(hCurrentModule,
                            (LPWSTR)RT_STRING,
                            MAKEINTRESOURCEW((dwId >> 4) + 1),
                            langId);
    if (hrsrc)
    {
        HGLOBAL hmem = LoadResource(hCurrentModule, hrsrc);

        if (hmem)
        {
            const WCHAR *p;
            unsigned int i;

            p = LockResource(hmem);
            for (i = 0; i < (dwId & 0x0f); i++) p += *p + 1;

            memcpy(lpszDest, p + 1, *p * sizeof(WCHAR));
            lpszDest[*p] = '\0';

            return TRUE;
        }
    }

    DPRINT1("Could not get codepage name. dwResId = %ld\n", dwResId);
    return FALSE;
}

/*
 * @implemented
 */
BOOL
WINAPI
GetCPInfo(UINT CodePage,
          LPCPINFO CodePageInfo)
{
    PCODEPAGE_ENTRY CodePageEntry;

    if (!CodePageInfo)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CodePageEntry = IntGetCodePageEntry(CodePage);
    if (CodePageEntry == NULL)
    {
        switch(CodePage)
        {
            case CP_UTF7:
            case CP_UTF8:
                CodePageInfo->DefaultChar[0] = 0x3f;
                CodePageInfo->DefaultChar[1] = 0;
                CodePageInfo->LeadByte[0] = CodePageInfo->LeadByte[1] = 0;
                CodePageInfo->MaxCharSize = (CodePage == CP_UTF7) ? 5 : 4;
                return TRUE;
        }

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (CodePageEntry->CodePageTable.DefaultChar & 0xff00)
    {
        CodePageInfo->DefaultChar[0] = (CodePageEntry->CodePageTable.DefaultChar & 0xff00) >> 8;
        CodePageInfo->DefaultChar[1] = CodePageEntry->CodePageTable.DefaultChar & 0x00ff;
    }
    else
    {
        CodePageInfo->DefaultChar[0] = CodePageEntry->CodePageTable.DefaultChar & 0xff;
        CodePageInfo->DefaultChar[1] = 0;
    }

    if ((CodePageInfo->MaxCharSize = CodePageEntry->CodePageTable.MaximumCharacterSize) == 2)
        memcpy(CodePageInfo->LeadByte, CodePageEntry->CodePageTable.LeadByte, sizeof(CodePageInfo->LeadByte));
    else
        CodePageInfo->LeadByte[0] = CodePageInfo->LeadByte[1] = 0;

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
GetCPInfoExW(UINT CodePage,
             DWORD dwFlags,
             LPCPINFOEXW lpCPInfoEx)
{
    if (!GetCPInfo(CodePage, (LPCPINFO) lpCPInfoEx))
        return FALSE;

    switch(CodePage)
    {
        case CP_UTF7:
        {
            lpCPInfoEx->CodePage = CP_UTF7;
            lpCPInfoEx->UnicodeDefaultChar = 0x3f;
            return GetLocalisedText((DWORD)CodePage, lpCPInfoEx->CodePageName);
        }
        break;

        case CP_UTF8:
        {
            lpCPInfoEx->CodePage = CP_UTF8;
            lpCPInfoEx->UnicodeDefaultChar = 0x3f;
            return GetLocalisedText((DWORD)CodePage, lpCPInfoEx->CodePageName);
        }

        default:
        {
            PCODEPAGE_ENTRY CodePageEntry;

            CodePageEntry = IntGetCodePageEntry(CodePage);
            if (CodePageEntry == NULL)
            {
                DPRINT1("Could not get CodePage Entry! CodePageEntry = 0\n");
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            lpCPInfoEx->CodePage = CodePageEntry->CodePageTable.CodePage;
            lpCPInfoEx->UnicodeDefaultChar = CodePageEntry->CodePageTable.UniDefaultChar;
            return GetLocalisedText((DWORD)CodePage, lpCPInfoEx->CodePageName);
        }
        break;
    }
}


/*
 * @implemented
 */
BOOL
WINAPI
GetCPInfoExA(UINT CodePage,
             DWORD dwFlags,
             LPCPINFOEXA lpCPInfoEx)
{
    CPINFOEXW CPInfo;

    if (!GetCPInfoExW(CodePage, dwFlags, &CPInfo))
        return FALSE;

    /* the layout is the same except for CodePageName */
    memcpy(lpCPInfoEx, &CPInfo, sizeof(CPINFOEXA));

    WideCharToMultiByte(CP_ACP,
                        0,
                        CPInfo.CodePageName,
                        -1,
                        lpCPInfoEx->CodePageName,
                        sizeof(lpCPInfoEx->CodePageName),
                        NULL,
                        NULL);
    return TRUE;
}

/**
 * @name WideCharToMultiByte
 *
 * Convert a wide-charater string to closest multi-byte equivalent.
 *
 * @param CodePage
 *        Code page to be used to perform the conversion. It can be also
 *        one of the special values (CP_ACP for ANSI code page, CP_MACCP
 *        for Macintosh code page, CP_OEMCP for OEM code page, CP_THREAD_ACP
 *        for thread active code page, CP_UTF7 or CP_UTF8).
 * @param Flags
 *        Additional conversion flags (WC_NO_BEST_FIT_CHARS, WC_COMPOSITECHECK,
 *        WC_DISCARDNS, WC_SEPCHARS, WC_DEFAULTCHAR).
 * @param WideCharString
 *        Points to the wide-character string to be converted.
 * @param WideCharCount
 *        Size in WCHARs of WideCharStr, or 0 if the caller just wants to
 *        know how large WideCharString should be for a successful conversion.
 * @param MultiByteString
 *        Points to the buffer to receive the translated string.
 * @param MultiByteCount
 *        Specifies the size in bytes of the buffer pointed to by the
 *        MultiByteString parameter. If this value is zero, the function
 *        returns the number of bytes required for the buffer.
 * @param DefaultChar
 *        Points to the character used if a wide character cannot be
 *        represented in the specified code page. If this parameter is
 *        NULL, a system default value is used.
 * @param UsedDefaultChar
 *        Points to a flag that indicates whether a default character was
 *        used. This parameter can be NULL.
 *
 * @return Zero on error, otherwise the number of bytes written in the
 *         MultiByteString buffer. Or the number of bytes needed for
 *         the MultiByteString buffer if MultiByteCount is zero.
 *
 * @implemented
 */

INT
WINAPI
WideCharToMultiByte(UINT CodePage,
                    DWORD Flags,
                    LPCWSTR WideCharString,
                    INT WideCharCount,
                    LPSTR MultiByteString,
                    INT MultiByteCount,
                    LPCSTR DefaultChar,
                    LPBOOL UsedDefaultChar)
{
    /* Check the parameters. */
    if (WideCharString == NULL ||
        (MultiByteString == NULL && MultiByteCount > 0) ||
        (PVOID)WideCharString == (PVOID)MultiByteString ||
        MultiByteCount < 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    /* Determine the input string length. */
    if (WideCharCount < 0)
    {
        WideCharCount = lstrlenW(WideCharString) + 1;
    }

    switch (CodePage)
    {
        case CP_UTF8:
            return IntWideCharToMultiByteUTF8(CodePage,
                                              Flags,
                                              WideCharString,
                                              WideCharCount,
                                              MultiByteString,
                                              MultiByteCount,
                                              DefaultChar,
                                              UsedDefaultChar);

        case CP_UTF7:
            if (DefaultChar != NULL || UsedDefaultChar != NULL)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return 0;
            }
            if (Flags)
            {
                SetLastError(ERROR_INVALID_FLAGS);
                return 0;
            }
            return WideCharToUtf7(WideCharString, WideCharCount,
                                  MultiByteString, MultiByteCount);

        case CP_SYMBOL:
            if ((DefaultChar!=NULL) || (UsedDefaultChar!=NULL))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return 0;
            }
            return IntWideCharToMultiByteSYMBOL(Flags,
                                                WideCharString,
                                                WideCharCount,
                                                MultiByteString,
                                                MultiByteCount);

        default:
            return IntWideCharToMultiByteCP(CodePage,
                                            Flags,
                                            WideCharString,
                                            WideCharCount,
                                            MultiByteString,
                                            MultiByteCount,
                                            DefaultChar,
                                            UsedDefaultChar);
   }
}

/**
 * @name GetACP
 *
 * Get active ANSI code page number.
 *
 * @implemented
 */

UINT
WINAPI
GetACP(VOID)
{
    return AnsiCodePage.CodePageTable.CodePage;
}

/**
 * @name GetOEMCP
 *
 * Get active OEM code page number.
 *
 * @implemented
 */

UINT
WINAPI
GetOEMCP(VOID)
{
    return OemCodePage.CodePageTable.CodePage;
}

/**
 * @name IsDBCSLeadByteEx
 *
 * Determine if passed byte is lead byte in specified code page.
 *
 * @implemented
 */

BOOL
WINAPI
IsDBCSLeadByteEx(UINT CodePage, BYTE TestByte)
{
    PCODEPAGE_ENTRY CodePageEntry;

    CodePageEntry = IntGetCodePageEntry(CodePage);
    if (CodePageEntry != NULL)
        return IntIsLeadByte(&CodePageEntry->CodePageTable, TestByte);

    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

/**
 * @name IsDBCSLeadByteEx
 *
 * Determine if passed byte is lead byte in current ANSI code page.
 *
 * @implemented
 */

BOOL
WINAPI
IsDBCSLeadByte(BYTE TestByte)
{
    return IntIsLeadByte(&AnsiCodePage.CodePageTable, TestByte);
}

/*
 * @unimplemented
 */
NTSTATUS WINAPI CreateNlsSecurityDescriptor(PSECURITY_DESCRIPTOR SecurityDescriptor,ULONG Size,ULONG AccessMask)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL WINAPI IsValidUILanguage(LANGID langid)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
VOID WINAPI NlsConvertIntegerToString(ULONG Value,ULONG Base,ULONG strsize, LPWSTR str, ULONG strsize2)
{
    STUB;
}

/*
 * @unimplemented
 */
UINT WINAPI SetCPGlobal(UINT CodePage)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
ValidateLCType(int a1, unsigned int a2, int a3, int a4)
{
    STUB;
    return FALSE;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
NlsResetProcessLocale(VOID)
{
    STUB;
    return TRUE;
}

/*
 * @unimplemented
 */
VOID
WINAPI
GetDefaultSortkeySize(LPVOID lpUnknown)
{
    STUB;
    lpUnknown = NULL;
}

/*
 * @unimplemented
 */
VOID
WINAPI
GetLinguistLangSize(LPVOID lpUnknown)
{
    STUB;
    lpUnknown = NULL;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
ValidateLocale(IN ULONG LocaleId)
{
    STUB;
    return TRUE;
}

/*
 * @unimplemented
 */
ULONG
WINAPI
NlsGetCacheUpdateCount(VOID)
{
    STUB;
    return 0;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
IsNLSDefinedString(IN NLS_FUNCTION Function,
                   IN DWORD dwFlags,
                   IN LPNLSVERSIONINFO lpVersionInformation,
                   IN LPCWSTR lpString,
                   IN INT cchStr)
{
    STUB;
    return TRUE;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
GetNLSVersion(IN NLS_FUNCTION Function,
              IN LCID Locale,
              IN OUT LPNLSVERSIONINFO lpVersionInformation)
{
    STUB;
    return TRUE;
}
    
/* EOF */
