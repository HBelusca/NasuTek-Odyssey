/*
 * PROJECT:   .inf file parser
 * LICENSE:   GPL - See COPYING in the top level directory
 * COPYRIGHT: Copyright 2005 Ge van Geldorp <gvg@odyssey.org>
 */

#ifdef INFLIB_HOST

/* Definitions native to the host on which we're building */

#include <typedefs.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define FREE(Area) free(Area)
#define MALLOC(Size) malloc((size_t)(Size))
#define ZEROMEMORY(Area, Size) memset((Area), '\0', (size_t)(Size))
#define MEMCPY(Dest, Src, Size) memcpy((Dest), (Src), (size_t)(Size))

#define STATUS_SUCCESS               0
#define INF_STATUS_SUCCESS           0
#define INF_STATUS_NO_MEMORY         ENOMEM
#define INF_STATUS_INVALID_PARAMETER EINVAL
#define INF_STATUS_NOT_FOUND         ENOENT
#define INF_STATUS_BUFFER_OVERFLOW   E2BIG
#define INF_SUCCESS(x) (0 == (x))

#define STRFMT "%s"

NTSTATUS NTAPI RtlMultiByteToUnicodeN(IN PWCHAR UnicodeString,
    IN ULONG UnicodeSize, IN PULONG ResultSize, IN PCSTR MbString, IN ULONG MbSize);

BOOLEAN NTAPI RtlIsTextUnicode( PVOID buf, INT len, INT *pf );


#define IS_TEXT_UNICODE_ASCII16 1
#define IS_TEXT_UNICODE_REVERSE_ASCII16 16
#define IS_TEXT_UNICODE_STATISTICS 2
#define IS_TEXT_UNICODE_REVERSE_STATISTICS 32
#define IS_TEXT_UNICODE_CONTROLS 4
#define IS_TEXT_UNICODE_REVERSE_CONTROLS 64
#define IS_TEXT_UNICODE_SIGNATURE 8
#define IS_TEXT_UNICODE_REVERSE_SIGNATURE 128
#define IS_TEXT_UNICODE_ILLEGAL_CHARS 256
#define IS_TEXT_UNICODE_ODD_LENGTH 512
#define IS_TEXT_UNICODE_NULL_BYTES 4096
#define IS_TEXT_UNICODE_UNICODE_MASK 15
#define IS_TEXT_UNICODE_REVERSE_MASK 240
#define IS_TEXT_UNICODE_NOT_UNICODE_MASK 3840
#define IS_TEXT_UNICODE_NOT_ASCII_MASK 61440

#else /* ! defined(INFLIB_HOST) */

/* Odyssey definitions */

#define UNICODE
#define _UNICODE
#define WIN32_NO_STATUS
#include <windows.h>
#define NTOS_MODE_USER
#include <ndk/iofuncs.h>
#include <ndk/obfuncs.h>
#include <ndk/rtlfuncs.h>

extern PVOID InfpHeap;

#define FREE(Area) RtlFreeHeap(InfpHeap, 0, (Area))
#define MALLOC(Size) RtlAllocateHeap(InfpHeap, 0, (Size))
#define ZEROMEMORY(Area, Size) RtlZeroMemory((Area), (Size))
#define MEMCPY(Dest, Src, Size) RtlCopyMemory((Dest), (Src), (Size))

#define INF_STATUS_SUCCESS           STATUS_SUCCESS
#define INF_STATUS_NO_MEMORY         STATUS_NO_MEMORY
#define INF_STATUS_INVALID_PARAMETER STATUS_INVALID_PARAMETER
#define INF_STATUS_NOT_FOUND         STATUS_NOT_FOUND
#define INF_STATUS_BUFFER_OVERFLOW   STATUS_BUFFER_OVERFLOW
#define INF_SUCCESS(x) (0 <= (x))

#define STRFMT "%S"

#endif /* INFLIB_HOST */

#include <wine/unicode.h>

/* EOF */
