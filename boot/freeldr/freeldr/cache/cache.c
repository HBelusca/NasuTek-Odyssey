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

#include <freeldr.h>
#include <debug.h>

DBG_DEFAULT_CHANNEL(CACHE);

///////////////////////////////////////////////////////////////////////////////////////
//
// Internal data
//
///////////////////////////////////////////////////////////////////////////////////////
CACHE_DRIVE		CacheManagerDrive;
BOOLEAN			CacheManagerInitialized = FALSE;
BOOLEAN			CacheManagerDataInvalid = FALSE;
ULONG			CacheBlockCount = 0;
ULONG			CacheSizeLimit = 0;
ULONG			CacheSizeCurrent = 0;

BOOLEAN CacheInitializeDrive(UCHAR DriveNumber)
{
	PCACHE_BLOCK	NextCacheBlock;
	GEOMETRY	DriveGeometry;

	// If we already have a cache for this drive then
	// by all means lets keep it, unless it is a removable
	// drive, in which case we'll invalidate the cache
	if ((CacheManagerInitialized == TRUE) &&
		(DriveNumber == CacheManagerDrive.DriveNumber) &&
		(DriveNumber >= 0x80) &&
		(CacheManagerDataInvalid != TRUE))
	{
		return TRUE;
	}

	CacheManagerDataInvalid = FALSE;

	//
	// If we have already been initialized then free
	// the old data
	//
	if (CacheManagerInitialized)
	{
		CacheManagerInitialized = FALSE;

		TRACE("CacheBlockCount: %d\n", CacheBlockCount);
		TRACE("CacheSizeLimit: %d\n", CacheSizeLimit);
		TRACE("CacheSizeCurrent: %d\n", CacheSizeCurrent);
		//
		// Loop through and free the cache blocks
		//
		while (!IsListEmpty(&CacheManagerDrive.CacheBlockHead))
		{
			NextCacheBlock = CONTAINING_RECORD(RemoveHeadList(&CacheManagerDrive.CacheBlockHead),
			                                   CACHE_BLOCK,
			                                   ListEntry);

			MmHeapFree(NextCacheBlock->BlockData);
			MmHeapFree(NextCacheBlock);
		}
	}

	// Initialize the structure
	RtlZeroMemory(&CacheManagerDrive, sizeof(CACHE_DRIVE));
	InitializeListHead(&CacheManagerDrive.CacheBlockHead);
	CacheManagerDrive.DriveNumber = DriveNumber;
	if (!MachDiskGetDriveGeometry(DriveNumber, &DriveGeometry))
	{
		return FALSE;
	}
	CacheManagerDrive.BytesPerSector = DriveGeometry.BytesPerSector;

	// Get the number of sectors in each cache block
	CacheManagerDrive.BlockSize = MachDiskGetCacheableBlockCount(DriveNumber);

	CacheBlockCount = 0;
	CacheSizeLimit = TotalPagesInLookupTable / 8 * MM_PAGE_SIZE;
	CacheSizeCurrent = 0;
	if (CacheSizeLimit < (64 * 1024))
	{
		CacheSizeLimit = (64 * 1024);
	}

	CacheManagerInitialized = TRUE;

	TRACE("Initializing BIOS drive 0x%x.\n", DriveNumber);
	TRACE("BytesPerSector: %d.\n", CacheManagerDrive.BytesPerSector);
	TRACE("BlockSize: %d.\n", CacheManagerDrive.BlockSize);
	TRACE("CacheSizeLimit: %d.\n", CacheSizeLimit);

	return TRUE;
}

VOID CacheInvalidateCacheData(VOID)
{
	CacheManagerDataInvalid = TRUE;
}

BOOLEAN CacheReadDiskSectors(UCHAR DiskNumber, ULONGLONG StartSector, ULONG SectorCount, PVOID Buffer)
{
	PCACHE_BLOCK	CacheBlock;
	ULONG				StartBlock;
	ULONG				SectorOffsetInStartBlock;
	ULONG				CopyLengthInStartBlock;
	ULONG				EndBlock;
	ULONG				SectorOffsetInEndBlock;
	ULONG				BlockCount;
	ULONG				Idx;

	TRACE("CacheReadDiskSectors() DiskNumber: 0x%x StartSector: %I64d SectorCount: %d Buffer: 0x%x\n", DiskNumber, StartSector, SectorCount, Buffer);

	// If we aren't initialized yet then they can't do this
	if (CacheManagerInitialized == FALSE)
	{
		return FALSE;
	}

	//
	// Caculate which blocks we must cache
	//
	StartBlock = (ULONG)(StartSector / CacheManagerDrive.BlockSize);
	SectorOffsetInStartBlock = (ULONG)(StartSector % CacheManagerDrive.BlockSize);
	CopyLengthInStartBlock = (ULONG)((SectorCount > (CacheManagerDrive.BlockSize - SectorOffsetInStartBlock)) ? (CacheManagerDrive.BlockSize - SectorOffsetInStartBlock) : SectorCount);
	EndBlock = (ULONG)((StartSector + (SectorCount - 1)) / CacheManagerDrive.BlockSize);
	SectorOffsetInEndBlock = (ULONG)(1 + (StartSector + (SectorCount - 1)) % CacheManagerDrive.BlockSize);
	BlockCount = (EndBlock - StartBlock) + 1;
	TRACE("StartBlock: %d SectorOffsetInStartBlock: %d CopyLengthInStartBlock: %d EndBlock: %d SectorOffsetInEndBlock: %d BlockCount: %d\n", StartBlock, SectorOffsetInStartBlock, CopyLengthInStartBlock, EndBlock, SectorOffsetInEndBlock, BlockCount);

	//
	// Read the first block into the buffer
	//
	if (BlockCount > 0)
	{
		//
		// Get cache block pointer (this forces the disk sectors into the cache memory)
		//
		CacheBlock = CacheInternalGetBlockPointer(&CacheManagerDrive, StartBlock);
		if (CacheBlock == NULL)
		{
			return FALSE;
		}

		//
		// Copy the portion requested into the buffer
		//
		RtlCopyMemory(Buffer,
			(PVOID)((ULONG_PTR)CacheBlock->BlockData + (SectorOffsetInStartBlock * CacheManagerDrive.BytesPerSector)),
			(CopyLengthInStartBlock * CacheManagerDrive.BytesPerSector));
		TRACE("1 - RtlCopyMemory(0x%x, 0x%x, %d)\n", Buffer, ((ULONG_PTR)CacheBlock->BlockData + (SectorOffsetInStartBlock * CacheManagerDrive.BytesPerSector)), (CopyLengthInStartBlock * CacheManagerDrive.BytesPerSector));

		//
		// Update the buffer address
		//
		Buffer = (PVOID)((ULONG_PTR)Buffer + (CopyLengthInStartBlock * CacheManagerDrive.BytesPerSector));

		//
		// Update the block count
		//
		BlockCount--;
	}

	//
	// Loop through the middle blocks and read them into the buffer
	//
	for (Idx=StartBlock+1; BlockCount>1; Idx++)
	{
		//
		// Get cache block pointer (this forces the disk sectors into the cache memory)
		//
		CacheBlock = CacheInternalGetBlockPointer(&CacheManagerDrive, Idx);
		if (CacheBlock == NULL)
		{
			return FALSE;
		}

		//
		// Copy the portion requested into the buffer
		//
		RtlCopyMemory(Buffer,
			CacheBlock->BlockData,
			CacheManagerDrive.BlockSize * CacheManagerDrive.BytesPerSector);
		TRACE("2 - RtlCopyMemory(0x%x, 0x%x, %d)\n", Buffer, CacheBlock->BlockData, CacheManagerDrive.BlockSize * CacheManagerDrive.BytesPerSector);

		//
		// Update the buffer address
		//
		Buffer = (PVOID)((ULONG_PTR)Buffer + (CacheManagerDrive.BlockSize * CacheManagerDrive.BytesPerSector));

		//
		// Update the block count
		//
		BlockCount--;
	}

	//
	// Read the last block into the buffer
	//
	if (BlockCount > 0)
	{
		//
		// Get cache block pointer (this forces the disk sectors into the cache memory)
		//
		CacheBlock = CacheInternalGetBlockPointer(&CacheManagerDrive, EndBlock);
		if (CacheBlock == NULL)
		{
			return FALSE;
		}

		//
		// Copy the portion requested into the buffer
		//
		RtlCopyMemory(Buffer,
			CacheBlock->BlockData,
			SectorOffsetInEndBlock * CacheManagerDrive.BytesPerSector);
		TRACE("3 - RtlCopyMemory(0x%x, 0x%x, %d)\n", Buffer, CacheBlock->BlockData, SectorOffsetInEndBlock * CacheManagerDrive.BytesPerSector);

		//
		// Update the buffer address
		//
		Buffer = (PVOID)((ULONG_PTR)Buffer + (SectorOffsetInEndBlock * CacheManagerDrive.BytesPerSector));

		//
		// Update the block count
		//
		BlockCount--;
	}

	return TRUE;
}

#if 0
BOOLEAN CacheForceDiskSectorsIntoCache(UCHAR DiskNumber, ULONGLONG StartSector, ULONG SectorCount)
{
	PCACHE_BLOCK	CacheBlock;
	ULONG				StartBlock;
	ULONG				EndBlock;
	ULONG				BlockCount;
	ULONG				Idx;

	TRACE("CacheForceDiskSectorsIntoCache() DiskNumber: 0x%x StartSector: %d SectorCount: %d\n", DiskNumber, StartSector, SectorCount);

	// If we aren't initialized yet then they can't do this
	if (CacheManagerInitialized == FALSE)
	{
		return FALSE;
	}

	//
	// Caculate which blocks we must cache
	//
	StartBlock = StartSector / CacheManagerDrive.BlockSize;
	EndBlock = (StartSector + SectorCount) / CacheManagerDrive.BlockSize;
	BlockCount = (EndBlock - StartBlock) + 1;

	//
	// Loop through and cache them
	//
	for (Idx=StartBlock; Idx<(StartBlock+BlockCount); Idx++)
	{
		//
		// Get cache block pointer (this forces the disk sectors into the cache memory)
		//
		CacheBlock = CacheInternalGetBlockPointer(&CacheManagerDrive, Idx);
		if (CacheBlock == NULL)
		{
			return FALSE;
		}

		//
		// Lock the sectors into the cache
		//
		CacheBlock->LockedInCache = TRUE;
	}

	return TRUE;
}
#endif

BOOLEAN CacheReleaseMemory(ULONG MinimumAmountToRelease)
{
	ULONG				AmountReleased;

	TRACE("CacheReleaseMemory() MinimumAmountToRelease = %d\n", MinimumAmountToRelease);

	// If we aren't initialized yet then they can't do this
	if (CacheManagerInitialized == FALSE)
	{
		return FALSE;
	}

	// Loop through and try to free the requested amount of memory
	for (AmountReleased=0; AmountReleased<MinimumAmountToRelease; )
	{
		// Try to free a block
		// If this fails then break out of the loop
		if (!CacheInternalFreeBlock(&CacheManagerDrive))
		{
			break;
		}

		// It succeeded so increment the amount of memory we have freed
		AmountReleased += CacheManagerDrive.BlockSize * CacheManagerDrive.BytesPerSector;
	}

	// Return status
	return (AmountReleased >= MinimumAmountToRelease);
}
