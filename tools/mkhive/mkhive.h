/*
 *  Odyssey kernel
 *  Copyright (C) 2003, 2006 ReactOS Team; (C) 2011 NasuTek Enterprises
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
/* COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey hive maker
 * FILE:            tools/mkhive/mkhive.h
 * PURPOSE:         Hive maker
 * PROGRAMMER:      Eric Kohl
 *                  Herv� Poussineau
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <typedefs.h>

// Definitions copied from <ntstatus.h>
// We only want to include host headers, so we define them manually
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000)
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001)
#define STATUS_NOT_IMPLEMENTED           ((NTSTATUS)0xC0000002)
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000D)
#define STATUS_NO_MEMORY                 ((NTSTATUS)0xC0000017)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009A)
#define STATUS_OBJECT_NAME_NOT_FOUND     ((NTSTATUS)0xC0000034)
#define STATUS_INVALID_PARAMETER_2       ((NTSTATUS)0xC00000F0)
#define STATUS_BUFFER_OVERFLOW           ((NTSTATUS)0x80000005)

unsigned char BitScanForward(ULONG * Index, unsigned long Mask);
unsigned char BitScanReverse(ULONG * const Index, unsigned long Mask);
#define RtlFillMemoryUlong(dst, len, val) memset(dst, val, len)

NTSTATUS NTAPI
RtlAnsiStringToUnicodeString(
    IN OUT PUNICODE_STRING UniDest,
    IN PANSI_STRING AnsiSource,
    IN BOOLEAN AllocateDestinationString);
VOID NTAPI
RtlInitAnsiString(
    IN OUT PANSI_STRING DestinationString,
    IN PCSTR SourceString);
VOID NTAPI
RtlInitUnicodeString(
    IN OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString);
WCHAR NTAPI
RtlUpcaseUnicodeChar(
    IN WCHAR Source);

LONG WINAPI
RegQueryValueExW(
    IN HKEY hKey,
    IN LPCWSTR lpValueName,
    IN PULONG lpReserved,
    OUT PULONG lpType,
    OUT PUCHAR lpData,
    OUT PSIZE_T lpcbData);

LONG WINAPI
RegSetValueExW(
    IN HKEY hKey,
    IN LPCWSTR lpValueName OPTIONAL,
    IN ULONG Reserved,
    IN ULONG dwType,
    IN const UCHAR* lpData,
    IN USHORT cbData);

LONG WINAPI
RegDeleteKeyW(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey);

LONG WINAPI
RegDeleteValueW(
    IN HKEY hKey,
    IN LPCWSTR lpValueName OPTIONAL);

LONG WINAPI
RegCreateKeyW(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult);

LONG WINAPI
RegOpenKeyW(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey,
    OUT PHKEY phkResult);

#define CMLIB_HOST
#include <cmlib.h>
#include <infhost.h>
#include "reginf.h"
#include "cmi.h"
#include "registry.h"
#include "binhive.h"

#define HIVE_NO_FILE 2
#define VERIFY_REGISTRY_HIVE(hive)
extern LIST_ENTRY CmiHiveListHead;
#define ABS_VALUE(V) (((V) < 0) ? -(V) : (V))

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#include <string.h>
#endif//_WIN32


/* EOF */
