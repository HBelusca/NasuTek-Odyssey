/*
 *  FreeLoader
 *  Copyright (C) 1998-2003  Brian Palmer  <brianp@sginet.com>
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

#pragma once

typedef struct tagDEVVTBL
{
  ARC_CLOSE Close;
  ARC_GET_FILE_INFORMATION GetFileInformation;
  ARC_OPEN Open;
  ARC_READ Read;
  ARC_SEEK Seek;
  LPCWSTR ServiceName;
} DEVVTBL;

#define	FS_FAT			1
#define	FS_NTFS			2
#define	FS_EXT2			3
#define FS_ISO9660		5

#define PFILE			ULONG

VOID FsRegisterDevice(CHAR* Prefix, const DEVVTBL* FuncTable);
LPCWSTR FsGetServiceName(ULONG FileId);
VOID FsSetDeviceSpecific(ULONG FileId, VOID* Specific);
VOID* FsGetDeviceSpecific(ULONG FileId);
ULONG FsGetDeviceId(ULONG FileId);
VOID FsInit(VOID);

LONG ArcClose(ULONG FileId);
LONG ArcGetFileInformation(ULONG FileId, FILEINFORMATION* Information);
LONG ArcOpen(CHAR* Path, OPENMODE OpenMode, ULONG* FileId);
LONG ArcRead(ULONG FileId, VOID* Buffer, ULONG N, ULONG* Count);
LONG ArcSeek(ULONG FileId, LARGE_INTEGER* Position, SEEKMODE SeekMode);

VOID	FileSystemError(PCSTR ErrorString);
PFILE	FsOpenFile(PCSTR FileName);
VOID	FsCloseFile(PFILE FileHandle);
BOOLEAN	FsReadFile(PFILE FileHandle, ULONG BytesToRead, ULONG* BytesRead, PVOID Buffer);
ULONG		FsGetFileSize(PFILE FileHandle);
VOID	FsSetFilePointer(PFILE FileHandle, ULONG NewFilePointer);
ULONG		FsGetNumPathParts(PCSTR Path);
VOID	FsGetFirstNameFromPath(PCHAR Buffer, PCSTR Path);

#define MAX_FDS 60
