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

#define INF_STATUS_SUCCESS           0
#define INF_STATUS_NO_MEMORY         ENOMEM
#define INF_STATUS_INVALID_PARAMETER EINVAL
#define INF_STATUS_NOT_FOUND         ENOENT
#define INF_STATUS_BUFFER_OVERFLOW   E2BIG
#define INF_SUCCESS(x) (0 == (x))

typedef char TCHAR, *PTCHAR, *PTSTR;
typedef const TCHAR *PCTSTR;

#define _T(x) x
#define _tcsicmp strcasecmp
#define _tcslen strlen
#define _tcscpy strcpy
#define _tcstoul strtoul
#define _tcstol strtol
#define STRFMT "%s"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

#else /* ! defined(INFLIB_HOST) */

/* Odyssey definitions */

#define UNICODE
#define _UNICODE
#define WIN32_NO_STATUS
#include <windows.h>
#define NTOS_MODE_USER
#include <ndk/ntndk.h>
#include <tchar.h>

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

/* EOF */
