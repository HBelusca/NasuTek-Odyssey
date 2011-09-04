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

// Returns a pointer to a CACHE_BLOCK structure
// Adds the block to the cache manager block list
// in cache memory if it isn't already there
PCACHE_BLOCK CacheInternalGetBlockPointer(PCACHE_DRIVE CacheDrive, ULONG BlockNumber)
{
	PCACHE_BLOCK	CacheBlock = NULL;

	TRACE("CacheInternalGetBlockPointer() BlockNumber = %d\n", BlockNumber);

	CacheBlock = CacheInternalFindBlock(CacheDrive, BlockNumber);

	if (CacheBlock != NULL)
	{
		TRACE("Cache hit! BlockNumber: %d CacheBlock->BlockNumber: %d\n", BlockNumber, CacheBlock->BlockNumber);

		return CacheBlock;
	}

	TRACE("Cache miss! BlockNumber: %d\n", BlockNumber);

	CacheBlock = CacheInternalAddBlockToCache(CacheDrive, BlockNumber);

	// Optimize the block list so it has a LRU structure
	CacheInternalOptimizeBlockList(CacheDrive, CacheBlock);

	return CacheBlock;
}

PCACHE_BLOCK CacheInternalFindBlock(PCACHE_DRIVE CacheDrive, ULONG BlockNumber)
{
	PCACHE_BLOCK	CacheBlock = NULL;

	TRACE("CacheInternalFindBlock() BlockNumber = %d\n", BlockNumber);

	//
	// Make sure the block list has entries before I start searching it.
	//
	if (!IsListEmpty(&CacheDrive->CacheBlockHead))
	{
		//
		// Search the list and find the BIOS drive number
		//
		CacheBlock = CONTAINING_RECORD(CacheDrive->CacheBlockHead.Flink, CACHE_BLOCK, ListEntry);

		while (&CacheBlock->ListEntry != &CacheDrive->CacheBlockHead)
		{
			//
			// We found the block, so return it
			//
			if (CacheBlock->BlockNumber == BlockNumber)
			{
				//
				// Increment the blocks access count
				//
				CacheBlock->AccessCount++;

				return CacheBlock;
			}

			CacheBlock = CONTAINING_RECORD(CacheBlock->ListEntry.Flink, CACHE_BLOCK, ListEntry);
		}
	}

	return NULL;
}

PCACHE_BLOCK CacheInternalAddBlockToCache(PCACHE_DRIVE CacheDrive, ULONG BlockNumber)
{
	PCACHE_BLOCK	CacheBlock = NULL;

	TRACE("CacheInternalAddBlockToCache() BlockNumber = %d\n", BlockNumber);

	// Check the size of the cache so we don't exceed our limits
	CacheInternalCheckCacheSizeLimits(CacheDrive);

	// We will need to add the block to the
	// drive's list of cached blocks. So allocate
	// the block memory.
	CacheBlock = MmHeapAlloc(sizeof(CACHE_BLOCK));
	if (CacheBlock == NULL)
	{
		return NULL;
	}

	// Now initialize the structure and
	// allocate room for the block data
	RtlZeroMemory(CacheBlock, sizeof(CACHE_BLOCK));
	CacheBlock->BlockNumber = BlockNumber;
	CacheBlock->BlockData = MmHeapAlloc(CacheDrive->BlockSize * CacheDrive->BytesPerSector);
	if (CacheBlock->BlockData ==NULL)
	{
		MmHeapFree(CacheBlock);
		return NULL;
	}

	// Now try to read in the block
	if (!MachDiskReadLogicalSectors(CacheDrive->DriveNumber, (BlockNumber * CacheDrive->BlockSize), CacheDrive->BlockSize, (PVOID)DISKREADBUFFER))
	{
		MmHeapFree(CacheBlock->BlockData);
		MmHeapFree(CacheBlock);
		return NULL;
	}
	RtlCopyMemory(CacheBlock->BlockData, (PVOID)DISKREADBUFFER, CacheDrive->BlockSize * CacheDrive->BytesPerSector);

	// Add it to our list of blocks managed by the cache
	InsertTailList(&CacheDrive->CacheBlockHead, &CacheBlock->ListEntry);

	// Update the cache data
	CacheBlockCount++;
	CacheSizeCurrent = CacheBlockCount * (CacheDrive->BlockSize * CacheDrive->BytesPerSector);

	CacheInternalDumpBlockList(CacheDrive);

	return CacheBlock;
}

BOOLEAN CacheInternalFreeBlock(PCACHE_DRIVE CacheDrive)
{
	PCACHE_BLOCK	CacheBlockToFree;

	TRACE("CacheInternalFreeBlock()\n");

	// Get a pointer to the last item in the block list
	// that isn't forced to be in the cache and remove
	// it from the list
	CacheBlockToFree = CONTAINING_RECORD(CacheDrive->CacheBlockHead.Blink, CACHE_BLOCK, ListEntry);
	while (&CacheBlockToFree->ListEntry != &CacheDrive->CacheBlockHead && CacheBlockToFree->LockedInCache == TRUE)
	{
		CacheBlockToFree = CONTAINING_RECORD(CacheBlockToFree->ListEntry.Blink, CACHE_BLOCK, ListEntry);
	}

	// No blocks left in cache that can be freed
	// so just return
	if (IsListEmpty(&CacheDrive->CacheBlockHead))
	{
		return FALSE;
	}

	RemoveEntryList(&CacheBlockToFree->ListEntry);

	// Free the block memory and the block structure
	MmHeapFree(CacheBlockToFree->BlockData);
	MmHeapFree(CacheBlockToFree);

	// Update the cache data
	CacheBlockCount--;
	CacheSizeCurrent = CacheBlockCount * (CacheDrive->BlockSize * CacheDrive->BytesPerSector);

	return TRUE;
}

VOID CacheInternalCheckCacheSizeLimits(PCACHE_DRIVE CacheDrive)
{
	ULONG		NewCacheSize;

	TRACE("CacheInternalCheckCacheSizeLimits()\n");

	// Calculate the size of the cache if we added a block
	NewCacheSize = (CacheBlockCount + 1) * (CacheDrive->BlockSize * CacheDrive->BytesPerSector);

	// Check the new size against the cache size limit
	if (NewCacheSize > CacheSizeLimit)
	{
		CacheInternalFreeBlock(CacheDrive);
		CacheInternalDumpBlockList(CacheDrive);
	}
}

VOID CacheInternalDumpBlockList(PCACHE_DRIVE CacheDrive)
{
	PCACHE_BLOCK	CacheBlock;

	TRACE("Dumping block list for BIOS drive 0x%x.\n", CacheDrive->DriveNumber);
	TRACE("BytesPerSector: %d.\n", CacheDrive->BytesPerSector);
	TRACE("BlockSize: %d.\n", CacheDrive->BlockSize);
	TRACE("CacheSizeLimit: %d.\n", CacheSizeLimit);
	TRACE("CacheSizeCurrent: %d.\n", CacheSizeCurrent);
	TRACE("CacheBlockCount: %d.\n", CacheBlockCount);

	CacheBlock = CONTAINING_RECORD(CacheDrive->CacheBlockHead.Flink, CACHE_BLOCK, ListEntry);
	while (&CacheBlock->ListEntry != &CacheDrive->CacheBlockHead)
	{
		TRACE("Cache Block: CacheBlock: 0x%x\n", CacheBlock);
		TRACE("Cache Block: Block Number: %d\n", CacheBlock->BlockNumber);
		TRACE("Cache Block: Access Count: %d\n", CacheBlock->AccessCount);
		TRACE("Cache Block: Block Data: 0x%x\n", CacheBlock->BlockData);
		TRACE("Cache Block: Locked In Cache: %d\n", CacheBlock->LockedInCache);

		if (CacheBlock->BlockData == NULL)
		{
			BugCheck("CacheBlock->BlockData == NULL\n");
		}

		CacheBlock = CONTAINING_RECORD(CacheBlock->ListEntry.Flink, CACHE_BLOCK, ListEntry);
	}
}

VOID CacheInternalOptimizeBlockList(PCACHE_DRIVE CacheDrive, PCACHE_BLOCK CacheBlock)
{

	TRACE("CacheInternalOptimizeBlockList()\n");

	// Don't do this if this block is already at the head of the list
	if (&CacheBlock->ListEntry != CacheDrive->CacheBlockHead.Flink)
	{
		// Remove this item from the block list
		RemoveEntryList(&CacheBlock->ListEntry);

		// Re-insert it at the head of the list
		InsertHeadList(&CacheDrive->CacheBlockHead, &CacheBlock->ListEntry);
	}
}
