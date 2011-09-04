/*
 *  FreeLoader
 *  Copyright (C) 1998-2003  Brian Palmer  <brianp@sginet.com>
 *  Copyright (C) 2009       Herv� Poussineau  <hpoussin@odyssey.org>
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

#ifndef _M_ARM
#include <freeldr.h>
#include <debug.h>

#define SECTORSIZE 2048

DBG_DEFAULT_CHANNEL(FILESYSTEM);

static BOOLEAN IsoSearchDirectoryBufferForFile(PVOID DirectoryBuffer, ULONG DirectoryLength, PCHAR FileName, PISO_FILE_INFO IsoFileInfoPointer)
{
	PDIR_RECORD	Record;
	ULONG		Offset;
	ULONG i;
	CHAR Name[32];

	TRACE("IsoSearchDirectoryBufferForFile() DirectoryBuffer = 0x%x DirectoryLength = %d FileName = %s\n", DirectoryBuffer, DirectoryLength, FileName);

	RtlZeroMemory(Name, 32 * sizeof(UCHAR));

	Offset = 0;
	Record = (PDIR_RECORD)DirectoryBuffer;
	while (TRUE)
	{
		Offset = Offset + Record->RecordLength;
		Record = (PDIR_RECORD)((ULONG_PTR)DirectoryBuffer + Offset);

		if (Record->RecordLength == 0)
		{
			Offset = ROUND_UP(Offset, SECTORSIZE);
			Record = (PDIR_RECORD)((ULONG_PTR)DirectoryBuffer + Offset);
		}

		if (Offset >= DirectoryLength)
			return FALSE;

		if (Record->FileIdLength == 1 && Record->FileId[0] == 0)
		{
			TRACE("Name '.'\n");
		}
		else if (Record->FileIdLength == 1 && Record->FileId[0] == 1)
		{
			TRACE("Name '..'\n");
		}
		else
		{
			for (i = 0; i < Record->FileIdLength && Record->FileId[i] != ';'; i++)
				Name[i] = Record->FileId[i];
			Name[i] = 0;
			TRACE("Name '%s'\n", Name);

			if (strlen(FileName) == strlen(Name) && _stricmp(FileName, Name) == 0)
			{
				IsoFileInfoPointer->FileStart = Record->ExtentLocationL;
				IsoFileInfoPointer->FileSize = Record->DataLengthL;
				IsoFileInfoPointer->FilePointer = 0;
				IsoFileInfoPointer->Directory = (Record->FileFlags & 0x02)?TRUE:FALSE;

				return TRUE;
			}

		}

		RtlZeroMemory(Name, 32 * sizeof(UCHAR));
	}

	return FALSE;
}


/*
 * IsoBufferDirectory()
 * This function allocates a buffer, reads the specified directory
 * and returns a pointer to that buffer into pDirectoryBuffer. The
 * function returns an ARC error code. The directory is specified
 * by its starting sector and length.
 */
static LONG IsoBufferDirectory(ULONG DeviceId, ULONG DirectoryStartSector, ULONG DirectoryLength,
    PVOID* pDirectoryBuffer)
{
	PVOID DirectoryBuffer;
	ULONG SectorCount;
	LARGE_INTEGER Position;
	ULONG Count;
	ULONG ret;

	TRACE("IsoBufferDirectory() DirectoryStartSector = %d DirectoryLength = %d\n", DirectoryStartSector, DirectoryLength);

	SectorCount = ROUND_UP(DirectoryLength, SECTORSIZE) / SECTORSIZE;
	TRACE("Trying to read (DirectoryCount) %d sectors.\n", SectorCount);

	//
	// Attempt to allocate memory for directory buffer
	//
	TRACE("Trying to allocate (DirectoryLength) %d bytes.\n", DirectoryLength);
	DirectoryBuffer = MmHeapAlloc(DirectoryLength);
	if (!DirectoryBuffer)
		return ENOMEM;

	//
	// Now read directory contents into DirectoryBuffer
	//
	Position.HighPart = 0;
	Position.LowPart = DirectoryStartSector * SECTORSIZE;
	ret = ArcSeek(DeviceId, &Position, SeekAbsolute);
	if (ret != ESUCCESS)
	{
		MmHeapFree(DirectoryBuffer);
		return ret;
	}
	ret = ArcRead(DeviceId, DirectoryBuffer, SectorCount * SECTORSIZE, &Count);
	if (ret != ESUCCESS || Count != SectorCount * SECTORSIZE)
	{
		MmHeapFree(DirectoryBuffer);
		return EIO;
	}

	*pDirectoryBuffer = DirectoryBuffer;
	return ESUCCESS;
}


/*
 * IsoLookupFile()
 * This function searches the file system for the
 * specified filename and fills in an ISO_FILE_INFO structure
 * with info describing the file, etc. returns ARC error code
 */
static LONG IsoLookupFile(PCSTR FileName, ULONG DeviceId, PISO_FILE_INFO IsoFileInfoPointer)
{
	UCHAR Buffer[SECTORSIZE];
	PPVD Pvd = (PPVD)Buffer;
	UINT32		i;
	ULONG			NumberOfPathParts;
	CHAR		PathPart[261];
	PVOID		DirectoryBuffer;
	ULONG		DirectorySector;
	ULONG		DirectoryLength;
	ISO_FILE_INFO	IsoFileInfo;
	LARGE_INTEGER Position;
	ULONG Count;
	LONG ret;

	TRACE("IsoLookupFile() FileName = %s\n", FileName);

	RtlZeroMemory(IsoFileInfoPointer, sizeof(ISO_FILE_INFO));
	RtlZeroMemory(&IsoFileInfo, sizeof(ISO_FILE_INFO));

	//
	// Read The Primary Volume Descriptor
	//
	Position.HighPart = 0;
	Position.LowPart = 16 * SECTORSIZE;
	ret = ArcSeek(DeviceId, &Position, SeekAbsolute);
	if (ret != ESUCCESS)
		return ret;
	ret = ArcRead(DeviceId, Pvd, SECTORSIZE, &Count);
	if (ret != ESUCCESS || Count < sizeof(PVD))
		return EIO;

	DirectorySector = Pvd->RootDirRecord.ExtentLocationL;
	DirectoryLength = Pvd->RootDirRecord.DataLengthL;

	//
	// Figure out how many sub-directories we are nested in
	//
	NumberOfPathParts = FsGetNumPathParts(FileName);

	//
	// Loop once for each part
	//
	for (i=0; i<NumberOfPathParts; i++)
	{
		//
		// Get first path part
		//
		FsGetFirstNameFromPath(PathPart, FileName);

		//
		// Advance to the next part of the path
		//
		for (; (*FileName != '\\') && (*FileName != '/') && (*FileName != '\0'); FileName++)
		{
		}
		FileName++;

		//
		// Buffer the directory contents
		//
		ret = IsoBufferDirectory(DeviceId, DirectorySector, DirectoryLength, &DirectoryBuffer);
		if (ret != ESUCCESS)
			return ret;

		//
		// Search for file name in directory
		//
		if (!IsoSearchDirectoryBufferForFile(DirectoryBuffer, DirectoryLength, PathPart, &IsoFileInfo))
		{
			MmHeapFree(DirectoryBuffer);
			return ENOENT;
		}

		MmHeapFree(DirectoryBuffer);

		//
		// If we have another sub-directory to go then
		// grab the start sector and file size
		//
		if ((i+1) < NumberOfPathParts)
		{
			DirectorySector = IsoFileInfo.FileStart;
			DirectoryLength = IsoFileInfo.FileSize;
		}

	}

	RtlCopyMemory(IsoFileInfoPointer, &IsoFileInfo, sizeof(ISO_FILE_INFO));

	return ESUCCESS;
}

LONG IsoClose(ULONG FileId)
{
	PISO_FILE_INFO FileHandle = FsGetDeviceSpecific(FileId);

	MmHeapFree(FileHandle);

	return ESUCCESS;
}

LONG IsoGetFileInformation(ULONG FileId, FILEINFORMATION* Information)
{
	PISO_FILE_INFO FileHandle = FsGetDeviceSpecific(FileId);

	TRACE("IsoGetFileInformation() FileSize = %d\n", FileHandle->FileSize);
	TRACE("IsoGetFileInformation() FilePointer = %d\n", FileHandle->FilePointer);

	RtlZeroMemory(Information, sizeof(FILEINFORMATION));
	Information->EndingAddress.LowPart = FileHandle->FileSize;
	Information->CurrentAddress.LowPart = FileHandle->FilePointer;

	return ESUCCESS;
}

LONG IsoOpen(CHAR* Path, OPENMODE OpenMode, ULONG* FileId)
{
	ISO_FILE_INFO TempFileInfo;
	PISO_FILE_INFO FileHandle;
	ULONG DeviceId;
	LONG ret;

	if (OpenMode != OpenReadOnly)
		return EACCES;

	DeviceId = FsGetDeviceId(*FileId);

	TRACE("IsoOpen() FileName = %s\n", Path);

	RtlZeroMemory(&TempFileInfo, sizeof(TempFileInfo));
	ret = IsoLookupFile(Path, DeviceId, &TempFileInfo);
	if (ret != ESUCCESS)
		return ENOENT;

	FileHandle = MmHeapAlloc(sizeof(ISO_FILE_INFO));
	if (!FileHandle)
		return ENOMEM;

	RtlCopyMemory(FileHandle, &TempFileInfo, sizeof(ISO_FILE_INFO));

	FsSetDeviceSpecific(*FileId, FileHandle);
	return ESUCCESS;
}

LONG IsoRead(ULONG FileId, VOID* Buffer, ULONG N, ULONG* Count)
{
	PISO_FILE_INFO FileHandle = FsGetDeviceSpecific(FileId);
	UCHAR SectorBuffer[SECTORSIZE];
	LARGE_INTEGER Position;
	ULONG DeviceId;
	ULONG FilePointer;
	ULONG		SectorNumber;
	ULONG		OffsetInSector;
	ULONG		LengthInSector;
	ULONG		NumberOfSectors;
	ULONG BytesRead;
	LONG ret;

	TRACE("IsoRead() Buffer = %p, N = %lu\n", Buffer, N);

	DeviceId = FsGetDeviceId(FileId);
	*Count = 0;

	//
	// If they are trying to read past the
	// end of the file then return success
	// with Count == 0
	//
	FilePointer = FileHandle->FilePointer;
	if (FilePointer >= FileHandle->FileSize)
	{
		return ESUCCESS;
	}

	//
	// If they are trying to read more than there is to read
	// then adjust the amount to read
	//
	if (FilePointer + N > FileHandle->FileSize)
	{
		N = FileHandle->FileSize - FilePointer;
	}

	//
	// Ok, now we have to perform at most 3 calculations
	// I'll draw you a picture (using nifty ASCII art):
	//
	// CurrentFilePointer -+
	//                     |
	//    +----------------+
	//    |
	// +-----------+-----------+-----------+-----------+
	// | Sector  1 | Sector  2 | Sector  3 | Sector  4 |
	// +-----------+-----------+-----------+-----------+
	//    |                                    |
	//    +---------------+--------------------+
	//                    |
	// N -----------------+
	//
	// 1 - The first calculation (and read) will align
	//     the file pointer with the next sector
	//     boundary (if we are supposed to read that much)
	// 2 - The next calculation (and read) will read
	//     in all the full sectors that the requested
	//     amount of data would cover (in this case
	//     sectors 2 & 3).
	// 3 - The last calculation (and read) would read
	//     in the remainder of the data requested out of
	//     the last sector.
	//


	//
	// Only do the first read if we
	// aren't aligned on a cluster boundary
	//
	if (FilePointer % SECTORSIZE)
	{
		//
		// Do the math for our first read
		//
		SectorNumber = FileHandle->FileStart + (FilePointer / SECTORSIZE);
		OffsetInSector = FilePointer % SECTORSIZE;
		LengthInSector = (N > (SECTORSIZE - OffsetInSector)) ? (SECTORSIZE - OffsetInSector) : N;

		//
		// Now do the read and update Count, N, FilePointer, & Buffer
		//
		Position.HighPart = 0;
		Position.LowPart = SectorNumber * SECTORSIZE;
		ret = ArcSeek(DeviceId, &Position, SeekAbsolute);
		if (ret != ESUCCESS)
		{
			return ret;
		}
		ret = ArcRead(DeviceId, SectorBuffer, SECTORSIZE, &BytesRead);
		if (ret != ESUCCESS || BytesRead != SECTORSIZE)
		{
			return EIO;
		}
		RtlCopyMemory(Buffer, SectorBuffer + OffsetInSector, LengthInSector);
		*Count += LengthInSector;
		N -= LengthInSector;
		FilePointer += LengthInSector;
		Buffer = (PVOID)((ULONG_PTR)Buffer + LengthInSector);
	}

	//
	// Do the math for our second read (if any data left)
	//
	if (N > 0)
	{
		//
		// Determine how many full clusters we need to read
		//
		NumberOfSectors = (N / SECTORSIZE);

		SectorNumber = FileHandle->FileStart + (FilePointer / SECTORSIZE);

		//
		// Now do the read and update Count, N, FilePointer, & Buffer
		//
		Position.HighPart = 0;
		Position.LowPart = SectorNumber * SECTORSIZE;
		ret = ArcSeek(DeviceId, &Position, SeekAbsolute);
		if (ret != ESUCCESS)
		{
			return ret;
		}
		ret = ArcRead(DeviceId, Buffer, NumberOfSectors * SECTORSIZE, &BytesRead);
		if (ret != ESUCCESS || BytesRead != NumberOfSectors * SECTORSIZE)
		{
			return EIO;
		}

		*Count += NumberOfSectors * SECTORSIZE;
		N -= NumberOfSectors * SECTORSIZE;
		FilePointer += NumberOfSectors * SECTORSIZE;
		Buffer = (PVOID)((ULONG_PTR)Buffer + NumberOfSectors * SECTORSIZE);
	}

	//
	// Do the math for our third read (if any data left)
	//
	if (N > 0)
	{
		SectorNumber = FileHandle->FileStart + (FilePointer / SECTORSIZE);

		//
		// Now do the read and update Count, N, FilePointer, & Buffer
		//
		Position.HighPart = 0;
		Position.LowPart = SectorNumber * SECTORSIZE;
		ret = ArcSeek(DeviceId, &Position, SeekAbsolute);
		if (ret != ESUCCESS)
		{
			return ret;
		}
		ret = ArcRead(DeviceId, SectorBuffer, SECTORSIZE, &BytesRead);
		if (ret != ESUCCESS || BytesRead != SECTORSIZE)
		{
			return EIO;
		}
		RtlCopyMemory(Buffer, SectorBuffer, N);
		*Count += N;
		FilePointer += N;
	}

	TRACE("IsoRead() done\n");

	return ESUCCESS;
}

LONG IsoSeek(ULONG FileId, LARGE_INTEGER* Position, SEEKMODE SeekMode)
{
	PISO_FILE_INFO FileHandle = FsGetDeviceSpecific(FileId);

	TRACE("IsoSeek() NewFilePointer = %lu\n", Position->LowPart);

	if (SeekMode != SeekAbsolute)
		return EINVAL;
	if (Position->HighPart != 0)
		return EINVAL;
	if (Position->LowPart >= FileHandle->FileSize)
		return EINVAL;

	FileHandle->FilePointer = Position->LowPart;
	return ESUCCESS;
}

const DEVVTBL Iso9660FuncTable =
{
	IsoClose,
	IsoGetFileInformation,
	IsoOpen,
	IsoRead,
	IsoSeek,
	L"cdfs",
};

const DEVVTBL* IsoMount(ULONG DeviceId)
{
	UCHAR Buffer[SECTORSIZE];
	PPVD Pvd = (PPVD)Buffer;
	LARGE_INTEGER Position;
	ULONG Count;
	LONG ret;

	//
	// Read The Primary Volume Descriptor
	//
	Position.HighPart = 0;
	Position.LowPart = 16 * SECTORSIZE;
	ret = ArcSeek(DeviceId, &Position, SeekAbsolute);
	if (ret != ESUCCESS)
		return NULL;
	ret = ArcRead(DeviceId, Pvd, SECTORSIZE, &Count);
	if (ret != ESUCCESS || Count < sizeof(PVD))
		return NULL;

	//
	// Check if PVD is valid. If yes, return ISO9660 function table
	//
	if (Pvd->VdType == 1 && RtlEqualMemory(Pvd->StandardId, "CD001", 5))
		return &Iso9660FuncTable;
	else
		return NULL;
}

#endif

