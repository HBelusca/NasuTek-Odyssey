/*
 * Wininet - Url Cache functions
 *
 * Copyright 2001,2002 CodeWeavers
 * Copyright 2003-2008 Robert Shearman
 *
 * Eric Kohl
 * Aric Stewart
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"
#include "wine/port.h"

#define NONAMELESSUNION
#define NONAMELESSSTRUCT

#if defined(__MINGW32__) || defined (_MSC_VER)
#include <ws2tcpip.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#include <time.h>

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "wininet.h"
#include "winineti.h"
#include "winerror.h"
#include "winreg.h"
#include "shlwapi.h"
#include "shlobj.h"
#include "shellapi.h"

#include "internet.h"

#include "wine/unicode.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(wininet);

#define ENTRY_START_OFFSET  0x4000
#define DIR_LENGTH          8
#define BLOCKSIZE           128
#define HASHTABLE_SIZE      448
#define HASHTABLE_BLOCKSIZE 7
#define HASHTABLE_FREE      3
#define ALLOCATION_TABLE_OFFSET 0x250
#define ALLOCATION_TABLE_SIZE   (0x1000 - ALLOCATION_TABLE_OFFSET)
#define HASHTABLE_NUM_ENTRIES   (HASHTABLE_SIZE / HASHTABLE_BLOCKSIZE)
#define NEWFILE_NUM_BLOCKS	0xd80
#define NEWFILE_SIZE		(NEWFILE_NUM_BLOCKS * BLOCKSIZE + ENTRY_START_OFFSET)

#define DWORD_SIG(a,b,c,d)  (a | (b << 8) | (c << 16) | (d << 24))
#define URL_SIGNATURE   DWORD_SIG('U','R','L',' ')
#define REDR_SIGNATURE  DWORD_SIG('R','E','D','R')
#define LEAK_SIGNATURE  DWORD_SIG('L','E','A','K')
#define HASH_SIGNATURE  DWORD_SIG('H','A','S','H')

#define DWORD_ALIGN(x) ( (DWORD)(((DWORD)(x)+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD) )

typedef struct _CACHEFILE_ENTRY
{
/*  union
    {*/
        DWORD dwSignature; /* e.g. "URL " */
/*      CHAR szSignature[4];
    };*/
    DWORD dwBlocksUsed; /* number of 128byte blocks used by this entry */
} CACHEFILE_ENTRY;

typedef struct _URL_CACHEFILE_ENTRY
{
    CACHEFILE_ENTRY CacheFileEntry;
    FILETIME LastModifiedTime;
    FILETIME LastAccessTime;
    WORD wExpiredDate; /* expire date in dos format */
    WORD wExpiredTime; /* expire time in dos format */
    DWORD dwUnknown1; /* usually zero */
    ULARGE_INTEGER size; /* see INTERNET_CACHE_ENTRY_INFO::dwSizeLow/High */
    DWORD dwUnknown2; /* usually zero */
    DWORD dwExemptDelta; /* see INTERNET_CACHE_ENTRY_INFO::dwExemptDelta */
    DWORD dwUnknown3; /* usually 0x60 */
    DWORD dwOffsetUrl; /* offset of start of url from start of entry */
    BYTE CacheDir; /* index of cache directory this url is stored in */
    BYTE Unknown4; /* usually zero */
    WORD wUnknown5; /* usually 0x1010 */
    DWORD dwOffsetLocalName; /* offset of start of local filename from start of entry */
    DWORD CacheEntryType; /* see INTERNET_CACHE_ENTRY_INFO::CacheEntryType */
    DWORD dwOffsetHeaderInfo; /* offset of start of header info from start of entry */
    DWORD dwHeaderInfoSize;
    DWORD dwOffsetFileExtension; /* offset of start of file extension from start of entry */
    WORD wLastSyncDate; /* last sync date in dos format */
    WORD wLastSyncTime; /* last sync time in dos format */
    DWORD dwHitRate; /* see INTERNET_CACHE_ENTRY_INFO::dwHitRate */
    DWORD dwUseCount; /* see INTERNET_CACHE_ENTRY_INFO::dwUseCount */
    WORD wUnknownDate; /* usually same as wLastSyncDate */
    WORD wUnknownTime; /* usually same as wLastSyncTime */
    DWORD dwUnknown7; /* usually zero */
    DWORD dwUnknown8; /* usually zero */
    /* packing to dword align start of next field */
    /* CHAR szSourceUrlName[]; (url) */
    /* packing to dword align start of next field */
    /* CHAR szLocalFileName[]; (local file name excluding path) */
    /* packing to dword align start of next field */
    /* CHAR szHeaderInfo[]; (header info) */
} URL_CACHEFILE_ENTRY;

struct _HASH_ENTRY
{
    DWORD dwHashKey;
    DWORD dwOffsetEntry;
};

typedef struct _HASH_CACHEFILE_ENTRY
{
    CACHEFILE_ENTRY CacheFileEntry;
    DWORD dwAddressNext;
    DWORD dwHashTableNumber;
    struct _HASH_ENTRY HashTable[HASHTABLE_SIZE];
} HASH_CACHEFILE_ENTRY;

typedef struct _DIRECTORY_DATA
{
    DWORD dwNumFiles;
    char filename[DIR_LENGTH];
} DIRECTORY_DATA;

typedef struct _URLCACHE_HEADER
{
    char szSignature[28];
    DWORD dwFileSize;
    DWORD dwOffsetFirstHashTable;
    DWORD dwIndexCapacityInBlocks;
    DWORD dwBlocksInUse;
    DWORD dwUnknown1;
    ULARGE_INTEGER CacheLimit;
    ULARGE_INTEGER CacheUsage;
    ULARGE_INTEGER ExemptUsage;
    DWORD DirectoryCount; /* number of directory_data's */
    DIRECTORY_DATA directory_data[1]; /* first directory entry */
} URLCACHE_HEADER, *LPURLCACHE_HEADER;
typedef const URLCACHE_HEADER *LPCURLCACHE_HEADER;

typedef struct _STREAM_HANDLE
{
    HANDLE hFile;
    CHAR lpszUrl[1];
} STREAM_HANDLE;

typedef struct _URLCACHECONTAINER
{
    struct list entry; /* part of a list */
    LPWSTR cache_prefix; /* string that has to be prefixed for this container to be used */
    LPWSTR path; /* path to url container directory */
    HANDLE hMapping; /* handle of file mapping */
    DWORD file_size; /* size of file when mapping was opened */
    HANDLE hMutex; /* handle of mutex */
} URLCACHECONTAINER;


/* List of all containers available */
static struct list UrlContainers = LIST_INIT(UrlContainers);

static DWORD URLCache_CreateHashTable(LPURLCACHE_HEADER pHeader, HASH_CACHEFILE_ENTRY *pPrevHash, HASH_CACHEFILE_ENTRY **ppHash);

/***********************************************************************
 *           URLCache_PathToObjectName (Internal)
 *
 *  Converts a path to a name suitable for use as a Win32 object name.
 * Replaces '\\' characters in-place with the specified character
 * (usually '_' or '!')
 *
 * RETURNS
 *    nothing
 *
 */
static void URLCache_PathToObjectName(LPWSTR lpszPath, WCHAR replace)
{
    for (; *lpszPath; lpszPath++)
    {
        if (*lpszPath == '\\')
            *lpszPath = replace;
    }
}

/***********************************************************************
 *           URLCacheContainer_OpenIndex (Internal)
 *
 *  Opens the index file and saves mapping handle in hCacheIndexMapping
 *
 * RETURNS
 *    ERROR_SUCCESS if succeeded
 *    Any other Win32 error code if failed
 *
 */
static DWORD URLCacheContainer_OpenIndex(URLCACHECONTAINER * pContainer)
{
    HANDLE hFile;
    WCHAR wszFilePath[MAX_PATH];
    DWORD dwFileSize;

    static const WCHAR wszIndex[] = {'i','n','d','e','x','.','d','a','t',0};
    static const WCHAR wszMappingFormat[] = {'%','s','%','s','_','%','l','u',0};

    WaitForSingleObject(pContainer->hMutex, INFINITE);

    if (pContainer->hMapping) {
        ReleaseMutex(pContainer->hMutex);
        return ERROR_SUCCESS;
    }

    strcpyW(wszFilePath, pContainer->path);
    strcatW(wszFilePath, wszIndex);

    hFile = CreateFileW(wszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
	/* Maybe the directory wasn't there? Try to create it */
	if (CreateDirectoryW(pContainer->path, 0))
            hFile = CreateFileW(wszFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
    }
    if (hFile == INVALID_HANDLE_VALUE)
    {
        TRACE("Could not open or create cache index file \"%s\"\n", debugstr_w(wszFilePath));
        ReleaseMutex(pContainer->hMutex);
        return GetLastError();
    }

    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
	ReleaseMutex(pContainer->hMutex);
        return GetLastError();
    }

    if (dwFileSize == 0)
    {
        static const CHAR szCacheContent[] = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Content";
	HKEY	key;
	char	achZeroes[0x1000];
	DWORD	dwOffset;
	DWORD dwError = ERROR_SUCCESS;

	/* Write zeroes to the entire file so we can safely map it without
	 * fear of getting a SEGV because the disk is full.
	 */
	memset(achZeroes, 0, sizeof(achZeroes));
	for (dwOffset = 0; dwOffset < NEWFILE_SIZE; dwOffset += sizeof(achZeroes))
	{
	    DWORD dwWrite = sizeof(achZeroes);
	    DWORD dwWritten;

	    if (NEWFILE_SIZE - dwOffset < dwWrite)
		dwWrite = NEWFILE_SIZE - dwOffset;
	    if (!WriteFile(hFile, achZeroes, dwWrite, &dwWritten, 0) ||
		dwWritten != dwWrite)
	    {
		/* If we fail to write, we need to return the error that
		 * cause the problem and also make sure the file is no
		 * longer there, if possible.
		 */
		dwError = GetLastError();

		break;
	    }
	}

	if (dwError == ERROR_SUCCESS)
	{
	    HANDLE hMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, NEWFILE_SIZE, NULL);

	    if (hMapping)
	    {
		URLCACHE_HEADER *pHeader = MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, NEWFILE_SIZE);

		if (pHeader)
		{
		    WCHAR *pwchDir;
		    WCHAR wszDirPath[MAX_PATH];
		    FILETIME ft;
		    int i, j;
                    HASH_CACHEFILE_ENTRY *pHashEntry;

		    dwFileSize = NEWFILE_SIZE;
		
		    /* First set some constants and defaults in the header */
		    strcpy(pHeader->szSignature, "WINE URLCache Ver 0.2005001");
		    pHeader->dwFileSize = dwFileSize;
		    pHeader->dwIndexCapacityInBlocks = NEWFILE_NUM_BLOCKS;
		    /* 127MB - taken from default for Windows 2000 */
                    pHeader->CacheLimit.QuadPart = 0x07ff5400;
		    /* Copied from a Windows 2000 cache index */
		    pHeader->DirectoryCount = 4;
		
		    /* If the registry has a cache size set, use the registry value */
		    if (RegOpenKeyA(HKEY_CURRENT_USER, szCacheContent, &key) == ERROR_SUCCESS)
		    {
		        DWORD dw;
		        DWORD len = sizeof(dw);
		        DWORD keytype;
		
		        if (RegQueryValueExA(key, "CacheLimit", NULL, &keytype,
					     (BYTE *) &dw, &len) == ERROR_SUCCESS &&
			    keytype == REG_DWORD)
			{
                            pHeader->CacheLimit.QuadPart = (ULONGLONG)dw * 1024;
			}
			RegCloseKey(key);
		    }
		
		    URLCache_CreateHashTable(pHeader, NULL, &pHashEntry);

		    /* Last step - create the directories */
	
		    strcpyW(wszDirPath, pContainer->path);
		    pwchDir = wszDirPath + strlenW(wszDirPath);
		    pwchDir[8] = 0;
	
		    GetSystemTimeAsFileTime(&ft);
	
		    for (i = 0; !dwError && i < pHeader->DirectoryCount; ++i)
		    {
			pHeader->directory_data[i].dwNumFiles = 0;
			for (j = 0;; ++j)
			{
			    int k;
			    ULONGLONG n = ft.dwHighDateTime;
	
			    /* Generate a file name to attempt to create.
			     * This algorithm will create what will appear
			     * to be random and unrelated directory names
			     * of up to 9 characters in length.
			     */
			    n <<= 32;
			    n += ft.dwLowDateTime;
			    n ^= ((ULONGLONG) i << 56) | ((ULONGLONG) j << 48);
	
			    for (k = 0; k < 8; ++k)
			    {
				int r = (n % 36);
	
				/* Dividing by a prime greater than 36 helps
				 * with the appearance of randomness
				 */
				n /= 37;
	
				if (r < 10)
				    pwchDir[k] = '0' + r;
				else
				    pwchDir[k] = 'A' + (r - 10);
			    }
	
			    if (CreateDirectoryW(wszDirPath, 0))
			    {
				/* The following is OK because we generated an
				 * 8 character directory name made from characters
				 * [A-Z0-9], which are equivalent for all code
				 * pages and for UTF-16
				 */
				for (k = 0; k < 8; ++k)
				    pHeader->directory_data[i].filename[k] = pwchDir[k];
				break;
			    }
			    else if (j >= 255)
			    {
				/* Give up. The most likely cause of this
				 * is a full disk, but whatever the cause
				 * is, it should be more than apparent that
				 * we won't succeed.
				 */
				dwError = GetLastError();
				break;
			    }
			}
		    }
		
		    UnmapViewOfFile(pHeader);
		}
		else
		{
		    dwError = GetLastError();
		}
		CloseHandle(hMapping);
	    }
	    else
	    {
		dwError = GetLastError();
	    }
	}

	if (dwError)
	{
	    CloseHandle(hFile);
	    DeleteFileW(wszFilePath);
	    ReleaseMutex(pContainer->hMutex);
	    return dwError;
	}

    }

    wsprintfW(wszFilePath, wszMappingFormat, pContainer->path, wszIndex, dwFileSize);
    URLCache_PathToObjectName(wszFilePath, '_');
    pContainer->hMapping = OpenFileMappingW(FILE_MAP_WRITE, FALSE, wszFilePath);
    if (!pContainer->hMapping)
        pContainer->hMapping = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 0, 0, wszFilePath);
    CloseHandle(hFile);
    if (!pContainer->hMapping)
    {
        ERR("Couldn't create file mapping (error is %d)\n", GetLastError());
        ReleaseMutex(pContainer->hMutex);
        return GetLastError();
    }

    ReleaseMutex(pContainer->hMutex);

    return ERROR_SUCCESS;
}

/***********************************************************************
 *           URLCacheContainer_CloseIndex (Internal)
 *
 *  Closes the index
 *
 * RETURNS
 *    nothing
 *
 */
static void URLCacheContainer_CloseIndex(URLCACHECONTAINER * pContainer)
{
    CloseHandle(pContainer->hMapping);
    pContainer->hMapping = NULL;
}

static BOOL URLCacheContainers_AddContainer(LPCWSTR cache_prefix, LPCWSTR path, LPWSTR mutex_name)
{
    URLCACHECONTAINER * pContainer = heap_alloc(sizeof(URLCACHECONTAINER));
    int cache_prefix_len = strlenW(cache_prefix);

    if (!pContainer)
    {
        return FALSE;
    }

    pContainer->hMapping = NULL;
    pContainer->file_size = 0;

    pContainer->path = heap_strdupW(path);
    if (!pContainer->path)
    {
        HeapFree(GetProcessHeap(), 0, pContainer);
        return FALSE;
    }

    pContainer->cache_prefix = heap_alloc((cache_prefix_len + 1) * sizeof(WCHAR));
    if (!pContainer->cache_prefix)
    {
        HeapFree(GetProcessHeap(), 0, pContainer->path);
        HeapFree(GetProcessHeap(), 0, pContainer);
        return FALSE;
    }

    memcpy(pContainer->cache_prefix, cache_prefix, (cache_prefix_len + 1) * sizeof(WCHAR));

    CharLowerW(mutex_name);
    URLCache_PathToObjectName(mutex_name, '!');

    if ((pContainer->hMutex = CreateMutexW(NULL, FALSE, mutex_name)) == NULL)
    {
        ERR("couldn't create mutex (error is %d)\n", GetLastError());
        HeapFree(GetProcessHeap(), 0, pContainer->path);
        HeapFree(GetProcessHeap(), 0, pContainer);
        return FALSE;
    }

    list_add_head(&UrlContainers, &pContainer->entry);

    return TRUE;
}

static void URLCacheContainer_DeleteContainer(URLCACHECONTAINER * pContainer)
{
    list_remove(&pContainer->entry);

    URLCacheContainer_CloseIndex(pContainer);
    CloseHandle(pContainer->hMutex);
    HeapFree(GetProcessHeap(), 0, pContainer->path);
    HeapFree(GetProcessHeap(), 0, pContainer->cache_prefix);
    HeapFree(GetProcessHeap(), 0, pContainer);
}

void URLCacheContainers_CreateDefaults(void)
{
    static const WCHAR UrlSuffix[] = {'C','o','n','t','e','n','t','.','I','E','5',0};
    static const WCHAR UrlPrefix[] = {0};
    static const WCHAR HistorySuffix[] = {'H','i','s','t','o','r','y','.','I','E','5',0};
    static const WCHAR HistoryPrefix[] = {'V','i','s','i','t','e','d',':',0};
    static const WCHAR CookieSuffix[] = {0};
    static const WCHAR CookiePrefix[] = {'C','o','o','k','i','e',':',0};
    static const WCHAR UserProfile[] = {'U','S','E','R','P','R','O','F','I','L','E',0};
    static const struct
    {
        int nFolder; /* CSIDL_* constant */
        const WCHAR * shpath_suffix; /* suffix on path returned by SHGetSpecialFolderPath */
        const WCHAR * cache_prefix; /* prefix used to reference the container */
    } DefaultContainerData[] = 
    {
        { CSIDL_INTERNET_CACHE, UrlSuffix, UrlPrefix },
        { CSIDL_HISTORY, HistorySuffix, HistoryPrefix },
        { CSIDL_COOKIES, CookieSuffix, CookiePrefix },
    };
    DWORD i;

    if (GetEnvironmentVariableW(UserProfile, NULL, 0) == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)
    {
        TRACE("Environment variable 'USERPROFILE' does not exist!\n");
        return;
    }

    for (i = 0; i < sizeof(DefaultContainerData) / sizeof(DefaultContainerData[0]); i++)
    {
        WCHAR wszCachePath[MAX_PATH];
        WCHAR wszMutexName[MAX_PATH];
        int path_len, suffix_len;

        if (!SHGetSpecialFolderPathW(NULL, wszCachePath, DefaultContainerData[i].nFolder, TRUE))
        {
            ERR("Couldn't get path for default container %u\n", i);
            continue;
        }
        path_len = strlenW(wszCachePath);
        suffix_len = strlenW(DefaultContainerData[i].shpath_suffix);

        if (path_len + suffix_len + 2 > MAX_PATH)
        {
            ERR("Path too long\n");
            continue;
        }

        wszCachePath[path_len] = '\\';
        wszCachePath[path_len+1] = 0;

        strcpyW(wszMutexName, wszCachePath);
        
        if (suffix_len)
        {
            memcpy(wszCachePath + path_len + 1, DefaultContainerData[i].shpath_suffix, (suffix_len + 1) * sizeof(WCHAR));
            wszCachePath[path_len + suffix_len + 1] = '\\';
            wszCachePath[path_len + suffix_len + 2] = '\0';
        }

        URLCacheContainers_AddContainer(DefaultContainerData[i].cache_prefix, wszCachePath, wszMutexName);
    }
}

void URLCacheContainers_DeleteAll(void)
{
    while(!list_empty(&UrlContainers))
        URLCacheContainer_DeleteContainer(
            LIST_ENTRY(list_head(&UrlContainers), URLCACHECONTAINER, entry)
        );
}

static DWORD URLCacheContainers_FindContainerW(LPCWSTR lpwszUrl, URLCACHECONTAINER ** ppContainer)
{
    URLCACHECONTAINER * pContainer;

    TRACE("searching for prefix for URL: %s\n", debugstr_w(lpwszUrl));

    if(!lpwszUrl)
        return ERROR_INVALID_PARAMETER;

    LIST_FOR_EACH_ENTRY(pContainer, &UrlContainers, URLCACHECONTAINER, entry)
    {
        int prefix_len = strlenW(pContainer->cache_prefix);
        if (!strncmpW(pContainer->cache_prefix, lpwszUrl, prefix_len))
        {
            TRACE("found container with prefix %s for URL %s\n", debugstr_w(pContainer->cache_prefix), debugstr_w(lpwszUrl));
            *ppContainer = pContainer;
            return ERROR_SUCCESS;
        }
    }
    ERR("no container found\n");
    return ERROR_FILE_NOT_FOUND;
}

static DWORD URLCacheContainers_FindContainerA(LPCSTR lpszUrl, URLCACHECONTAINER ** ppContainer)
{
    LPWSTR url = NULL;
    DWORD ret;

    if (lpszUrl && !(url = heap_strdupAtoW(lpszUrl)))
        return ERROR_OUTOFMEMORY;

    ret = URLCacheContainers_FindContainerW(url, ppContainer);
    HeapFree(GetProcessHeap(), 0, url);
    return ret;
}

static BOOL URLCacheContainers_Enum(LPCWSTR lpwszSearchPattern, DWORD dwIndex, URLCACHECONTAINER ** ppContainer)
{
    DWORD i = 0;
    URLCACHECONTAINER * pContainer;

    TRACE("searching for prefix: %s\n", debugstr_w(lpwszSearchPattern));

    /* non-NULL search pattern only returns one container ever */
    if (lpwszSearchPattern && dwIndex > 0)
        return FALSE;

    LIST_FOR_EACH_ENTRY(pContainer, &UrlContainers, URLCACHECONTAINER, entry)
    {
        if (lpwszSearchPattern)
        {
            if (!strcmpW(pContainer->cache_prefix, lpwszSearchPattern))
            {
                TRACE("found container with prefix %s\n", debugstr_w(pContainer->cache_prefix));
                *ppContainer = pContainer;
                return TRUE;
            }
        }
        else
        {
            if (i == dwIndex)
            {
                TRACE("found container with prefix %s\n", debugstr_w(pContainer->cache_prefix));
                *ppContainer = pContainer;
                return TRUE;
            }
        }
        i++;
    }
    return FALSE;
}

/***********************************************************************
 *           URLCacheContainer_LockIndex (Internal)
 *
 * Locks the index for system-wide exclusive access.
 *
 * RETURNS
 *  Cache file header if successful
 *  NULL if failed and calls SetLastError.
 */
static LPURLCACHE_HEADER URLCacheContainer_LockIndex(URLCACHECONTAINER * pContainer)
{
    BYTE index;
    LPVOID pIndexData;
    URLCACHE_HEADER * pHeader;
    DWORD error;

    /* acquire mutex */
    WaitForSingleObject(pContainer->hMutex, INFINITE);

    pIndexData = MapViewOfFile(pContainer->hMapping, FILE_MAP_WRITE, 0, 0, 0);

    if (!pIndexData)
    {
        ReleaseMutex(pContainer->hMutex);
        ERR("Couldn't MapViewOfFile. Error: %d\n", GetLastError());
        return NULL;
    }
    pHeader = (URLCACHE_HEADER *)pIndexData;

    /* file has grown - we need to remap to prevent us getting
     * access violations when we try and access beyond the end
     * of the memory mapped file */
    if (pHeader->dwFileSize != pContainer->file_size)
    {
        UnmapViewOfFile( pHeader );
        URLCacheContainer_CloseIndex(pContainer);
        error = URLCacheContainer_OpenIndex(pContainer);
        if (error != ERROR_SUCCESS)
        {
            ReleaseMutex(pContainer->hMutex);
            SetLastError(error);
            return NULL;
        }
        pIndexData = MapViewOfFile(pContainer->hMapping, FILE_MAP_WRITE, 0, 0, 0);

        if (!pIndexData)
        {
            ReleaseMutex(pContainer->hMutex);
            ERR("Couldn't MapViewOfFile. Error: %d\n", GetLastError());
            return NULL;
        }
        pHeader = (URLCACHE_HEADER *)pIndexData;
    }

    TRACE("Signature: %s, file size: %d bytes\n", pHeader->szSignature, pHeader->dwFileSize);

    for (index = 0; index < pHeader->DirectoryCount; index++)
    {
        TRACE("Directory[%d] = \"%.8s\"\n", index, pHeader->directory_data[index].filename);
    }
    
    return pHeader;
}

/***********************************************************************
 *           URLCacheContainer_UnlockIndex (Internal)
 *
 */
static BOOL URLCacheContainer_UnlockIndex(URLCACHECONTAINER * pContainer, LPURLCACHE_HEADER pHeader)
{
    /* release mutex */
    ReleaseMutex(pContainer->hMutex);
    return UnmapViewOfFile(pHeader);
}


#ifndef CHAR_BIT
#define CHAR_BIT    (8 * sizeof(CHAR))
#endif

/***********************************************************************
 *           URLCache_Allocation_BlockIsFree (Internal)
 *
 *  Is the specified block number free?
 *
 * RETURNS
 *    zero if free
 *    non-zero otherwise
 *
 */
static inline BYTE URLCache_Allocation_BlockIsFree(BYTE * AllocationTable, DWORD dwBlockNumber)
{
    BYTE mask = 1 << (dwBlockNumber % CHAR_BIT);
    return (AllocationTable[dwBlockNumber / CHAR_BIT] & mask) == 0;
}

/***********************************************************************
 *           URLCache_Allocation_BlockFree (Internal)
 *
 *  Marks the specified block as free
 *
 * RETURNS
 *    nothing
 *
 */
static inline void URLCache_Allocation_BlockFree(BYTE * AllocationTable, DWORD dwBlockNumber)
{
    BYTE mask = ~(1 << (dwBlockNumber % CHAR_BIT));
    AllocationTable[dwBlockNumber / CHAR_BIT] &= mask;
}

/***********************************************************************
 *           URLCache_Allocation_BlockAllocate (Internal)
 *
 *  Marks the specified block as allocated
 *
 * RETURNS
 *    nothing
 *
 */
static inline void URLCache_Allocation_BlockAllocate(BYTE * AllocationTable, DWORD dwBlockNumber)
{
    BYTE mask = 1 << (dwBlockNumber % CHAR_BIT);
    AllocationTable[dwBlockNumber / CHAR_BIT] |= mask;
}

/***********************************************************************
 *           URLCache_FindFirstFreeEntry (Internal)
 *
 *  Finds and allocates the first block of free space big enough and
 * sets ppEntry to point to it.
 *
 * RETURNS
 *    TRUE if it had enough space
 *    FALSE if it couldn't find enough space
 *
 */
static BOOL URLCache_FindFirstFreeEntry(URLCACHE_HEADER * pHeader, DWORD dwBlocksNeeded, CACHEFILE_ENTRY ** ppEntry)
{
    LPBYTE AllocationTable = (LPBYTE)pHeader + ALLOCATION_TABLE_OFFSET;
    DWORD dwBlockNumber;
    DWORD dwFreeCounter;
    for (dwBlockNumber = 0; dwBlockNumber < pHeader->dwIndexCapacityInBlocks; dwBlockNumber++)
    {
        for (dwFreeCounter = 0; 
            dwFreeCounter < dwBlocksNeeded &&
              dwFreeCounter + dwBlockNumber < pHeader->dwIndexCapacityInBlocks &&
              URLCache_Allocation_BlockIsFree(AllocationTable, dwBlockNumber + dwFreeCounter);
            dwFreeCounter++)
                TRACE("Found free block at no. %d (0x%x)\n", dwBlockNumber, ENTRY_START_OFFSET + dwBlockNumber * BLOCKSIZE);

        if (dwFreeCounter == dwBlocksNeeded)
        {
            DWORD index;
            TRACE("Found free blocks starting at no. %d (0x%x)\n", dwBlockNumber, ENTRY_START_OFFSET + dwBlockNumber * BLOCKSIZE);
            for (index = 0; index < dwBlocksNeeded; index++)
                URLCache_Allocation_BlockAllocate(AllocationTable, dwBlockNumber + index);
            *ppEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + ENTRY_START_OFFSET + dwBlockNumber * BLOCKSIZE);
            (*ppEntry)->dwBlocksUsed = dwBlocksNeeded;
            return TRUE;
        }
    }
    FIXME("Grow file\n");
    return FALSE;
}

/***********************************************************************
 *           URLCache_DeleteEntry (Internal)
 *
 *  Deletes the specified entry and frees the space allocated to it
 *
 * RETURNS
 *    TRUE if it succeeded
 *    FALSE if it failed
 *
 */
static BOOL URLCache_DeleteEntry(LPURLCACHE_HEADER pHeader, CACHEFILE_ENTRY * pEntry)
{
    DWORD dwStartBlock;
    DWORD dwBlock;
    BYTE * AllocationTable = (LPBYTE)pHeader + ALLOCATION_TABLE_OFFSET;

    /* update allocation table */
    dwStartBlock = ((DWORD)((BYTE *)pEntry - (BYTE *)pHeader)) / BLOCKSIZE;
    for (dwBlock = dwStartBlock; dwBlock < dwStartBlock + pEntry->dwBlocksUsed; dwBlock++)
        URLCache_Allocation_BlockFree(AllocationTable, dwBlock);

    ZeroMemory(pEntry, pEntry->dwBlocksUsed * BLOCKSIZE);
    return TRUE;
}

/***********************************************************************
 *           URLCache_LocalFileNameToPathW (Internal)
 *
 *  Copies the full path to the specified buffer given the local file
 * name and the index of the directory it is in. Always sets value in
 * lpBufferSize to the required buffer size (in bytes).
 *
 * RETURNS
 *    TRUE if the buffer was big enough
 *    FALSE if the buffer was too small
 *
 */
static BOOL URLCache_LocalFileNameToPathW(
    const URLCACHECONTAINER * pContainer,
    LPCURLCACHE_HEADER pHeader,
    LPCSTR szLocalFileName,
    BYTE Directory,
    LPWSTR wszPath,
    LPLONG lpBufferSize)
{
    LONG nRequired;
    int path_len = strlenW(pContainer->path);
    int file_name_len = MultiByteToWideChar(CP_ACP, 0, szLocalFileName, -1, NULL, 0);
    if (Directory >= pHeader->DirectoryCount)
    {
        *lpBufferSize = 0;
        return FALSE;
    }

    nRequired = (path_len + DIR_LENGTH + file_name_len + 1) * sizeof(WCHAR);
    if (nRequired <= *lpBufferSize)
    {
        int dir_len;

        memcpy(wszPath, pContainer->path, path_len * sizeof(WCHAR));
        dir_len = MultiByteToWideChar(CP_ACP, 0, pHeader->directory_data[Directory].filename, DIR_LENGTH, wszPath + path_len, DIR_LENGTH);
        wszPath[dir_len + path_len] = '\\';
        MultiByteToWideChar(CP_ACP, 0, szLocalFileName, -1, wszPath + dir_len + path_len + 1, file_name_len);
        *lpBufferSize = nRequired;
        return TRUE;
    }
    *lpBufferSize = nRequired;
    return FALSE;
}

/***********************************************************************
 *           URLCache_LocalFileNameToPathA (Internal)
 *
 *  Copies the full path to the specified buffer given the local file
 * name and the index of the directory it is in. Always sets value in
 * lpBufferSize to the required buffer size.
 *
 * RETURNS
 *    TRUE if the buffer was big enough
 *    FALSE if the buffer was too small
 *
 */
static BOOL URLCache_LocalFileNameToPathA(
    const URLCACHECONTAINER * pContainer,
    LPCURLCACHE_HEADER pHeader,
    LPCSTR szLocalFileName,
    BYTE Directory,
    LPSTR szPath,
    LPLONG lpBufferSize)
{
    LONG nRequired;
    int path_len, file_name_len, dir_len;

    if (Directory >= pHeader->DirectoryCount)
    {
        *lpBufferSize = 0;
        return FALSE;
    }

    path_len = WideCharToMultiByte(CP_ACP, 0, pContainer->path, -1, NULL, 0, NULL, NULL) - 1;
    file_name_len = strlen(szLocalFileName) + 1 /* for nul-terminator */;
    dir_len = DIR_LENGTH;

    nRequired = (path_len + dir_len + 1 + file_name_len) * sizeof(char);
    if (nRequired < *lpBufferSize)
    {
        WideCharToMultiByte(CP_ACP, 0, pContainer->path, -1, szPath, path_len, NULL, NULL);
        memcpy(szPath+path_len, pHeader->directory_data[Directory].filename, dir_len);
        szPath[path_len + dir_len] = '\\';
        memcpy(szPath + path_len + dir_len + 1, szLocalFileName, file_name_len);
        *lpBufferSize = nRequired;
        return TRUE;
    }
    *lpBufferSize = nRequired;
    return FALSE;
}

/* Just like DosDateTimeToFileTime, except that it also maps the special
 * case of a DOS date/time of (0,0) to a filetime of (0,0).
 */
static void URLCache_DosDateTimeToFileTime(WORD fatdate, WORD fattime,
                                           FILETIME *ft)
{
    if (!fatdate && !fattime)
        ft->dwLowDateTime = ft->dwHighDateTime = 0;
    else
        DosDateTimeToFileTime(fatdate, fattime, ft);
}

/***********************************************************************
 *           URLCache_CopyEntry (Internal)
 *
 *  Copies an entry from the cache index file to the Win32 structure
 *
 * RETURNS
 *    ERROR_SUCCESS if the buffer was big enough
 *    ERROR_INSUFFICIENT_BUFFER if the buffer was too small
 *
 */
static DWORD URLCache_CopyEntry(
    URLCACHECONTAINER * pContainer,
    LPCURLCACHE_HEADER pHeader, 
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo, 
    LPDWORD lpdwBufferSize, 
    const URL_CACHEFILE_ENTRY * pUrlEntry,
    BOOL bUnicode)
{
    int lenUrl;
    DWORD dwRequiredSize = sizeof(*lpCacheEntryInfo);

    if (*lpdwBufferSize >= dwRequiredSize)
    {
        lpCacheEntryInfo->lpHeaderInfo = NULL;
        lpCacheEntryInfo->lpszFileExtension = NULL;
        lpCacheEntryInfo->lpszLocalFileName = NULL;
        lpCacheEntryInfo->lpszSourceUrlName = NULL;
        lpCacheEntryInfo->CacheEntryType = pUrlEntry->CacheEntryType;
        lpCacheEntryInfo->u.dwExemptDelta = pUrlEntry->dwExemptDelta;
        lpCacheEntryInfo->dwHeaderInfoSize = pUrlEntry->dwHeaderInfoSize;
        lpCacheEntryInfo->dwHitRate = pUrlEntry->dwHitRate;
        lpCacheEntryInfo->dwSizeHigh = pUrlEntry->size.u.HighPart;
        lpCacheEntryInfo->dwSizeLow = pUrlEntry->size.u.LowPart;
        lpCacheEntryInfo->dwStructSize = sizeof(*lpCacheEntryInfo);
        lpCacheEntryInfo->dwUseCount = pUrlEntry->dwUseCount;
        URLCache_DosDateTimeToFileTime(pUrlEntry->wExpiredDate, pUrlEntry->wExpiredTime, &lpCacheEntryInfo->ExpireTime);
        lpCacheEntryInfo->LastAccessTime.dwHighDateTime = pUrlEntry->LastAccessTime.dwHighDateTime;
        lpCacheEntryInfo->LastAccessTime.dwLowDateTime = pUrlEntry->LastAccessTime.dwLowDateTime;
        lpCacheEntryInfo->LastModifiedTime.dwHighDateTime = pUrlEntry->LastModifiedTime.dwHighDateTime;
        lpCacheEntryInfo->LastModifiedTime.dwLowDateTime = pUrlEntry->LastModifiedTime.dwLowDateTime;
        URLCache_DosDateTimeToFileTime(pUrlEntry->wLastSyncDate, pUrlEntry->wLastSyncTime, &lpCacheEntryInfo->LastSyncTime);
    }

    if ((dwRequiredSize % 4) && (dwRequiredSize < *lpdwBufferSize))
        ZeroMemory((LPBYTE)lpCacheEntryInfo + dwRequiredSize, 4 - (dwRequiredSize % 4));
    dwRequiredSize = DWORD_ALIGN(dwRequiredSize);
    if (bUnicode)
        lenUrl = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl, -1, NULL, 0);
    else
        lenUrl = strlen((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl);
    dwRequiredSize += (lenUrl + 1) * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

    /* FIXME: is source url optional? */
    if (*lpdwBufferSize >= dwRequiredSize)
    {
        DWORD lenUrlBytes = (lenUrl+1) * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

        lpCacheEntryInfo->lpszSourceUrlName = (LPSTR)lpCacheEntryInfo + dwRequiredSize - lenUrlBytes;
        if (bUnicode)
            MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl, -1, (LPWSTR)lpCacheEntryInfo->lpszSourceUrlName, lenUrl + 1);
        else
            memcpy(lpCacheEntryInfo->lpszSourceUrlName, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl, lenUrlBytes);
    }

    if ((dwRequiredSize % 4) && (dwRequiredSize < *lpdwBufferSize))
        ZeroMemory((LPBYTE)lpCacheEntryInfo + dwRequiredSize, 4 - (dwRequiredSize % 4));
    dwRequiredSize = DWORD_ALIGN(dwRequiredSize);

    if (pUrlEntry->dwOffsetLocalName)
    {
        LONG nLocalFilePathSize;
        LPSTR lpszLocalFileName;
        lpszLocalFileName = (LPSTR)lpCacheEntryInfo + dwRequiredSize;
        nLocalFilePathSize = *lpdwBufferSize - dwRequiredSize;
        if ((bUnicode && URLCache_LocalFileNameToPathW(pContainer, pHeader, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName, pUrlEntry->CacheDir, (LPWSTR)lpszLocalFileName, &nLocalFilePathSize)) ||
            (!bUnicode && URLCache_LocalFileNameToPathA(pContainer, pHeader, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName, pUrlEntry->CacheDir, lpszLocalFileName, &nLocalFilePathSize)))
        {
            lpCacheEntryInfo->lpszLocalFileName = lpszLocalFileName;
        }
        dwRequiredSize += nLocalFilePathSize * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR)) ;

        if ((dwRequiredSize % 4) && (dwRequiredSize < *lpdwBufferSize))
            ZeroMemory((LPBYTE)lpCacheEntryInfo + dwRequiredSize, 4 - (dwRequiredSize % 4));
        dwRequiredSize = DWORD_ALIGN(dwRequiredSize);
    }
    dwRequiredSize += pUrlEntry->dwHeaderInfoSize + 1;

    if (*lpdwBufferSize >= dwRequiredSize)
    {
        lpCacheEntryInfo->lpHeaderInfo = (LPBYTE)lpCacheEntryInfo + dwRequiredSize - pUrlEntry->dwHeaderInfoSize - 1;
        memcpy(lpCacheEntryInfo->lpHeaderInfo, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetHeaderInfo, pUrlEntry->dwHeaderInfoSize);
        ((LPBYTE)lpCacheEntryInfo)[dwRequiredSize - 1] = '\0';
    }
    if ((dwRequiredSize % 4) && (dwRequiredSize < *lpdwBufferSize))
        ZeroMemory((LPBYTE)lpCacheEntryInfo + dwRequiredSize, 4 - (dwRequiredSize % 4));
    dwRequiredSize = DWORD_ALIGN(dwRequiredSize);

    if (pUrlEntry->dwOffsetFileExtension)
    {
        int lenExtension;

        if (bUnicode)
            lenExtension = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetFileExtension, -1, NULL, 0);
        else
            lenExtension = strlen((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetFileExtension) + 1;
        dwRequiredSize += lenExtension * (bUnicode ? sizeof(WCHAR) : sizeof(CHAR));

        if (*lpdwBufferSize >= dwRequiredSize)
        {
            lpCacheEntryInfo->lpszFileExtension = (LPSTR)lpCacheEntryInfo + dwRequiredSize - lenExtension;
            if (bUnicode)
                MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetFileExtension, -1, (LPWSTR)lpCacheEntryInfo->lpszSourceUrlName, lenExtension);
            else
                memcpy(lpCacheEntryInfo->lpszFileExtension, (LPCSTR)pUrlEntry + pUrlEntry->dwOffsetFileExtension, lenExtension * sizeof(CHAR));
        }

        if ((dwRequiredSize % 4) && (dwRequiredSize < *lpdwBufferSize))
            ZeroMemory((LPBYTE)lpCacheEntryInfo + dwRequiredSize, 4 - (dwRequiredSize % 4));
        dwRequiredSize = DWORD_ALIGN(dwRequiredSize);
    }

    if (dwRequiredSize > *lpdwBufferSize)
    {
        *lpdwBufferSize = dwRequiredSize;
        return ERROR_INSUFFICIENT_BUFFER;
    }
    *lpdwBufferSize = dwRequiredSize;
    return ERROR_SUCCESS;
}

/* Just like FileTimeToDosDateTime, except that it also maps the special
 * case of a filetime of (0,0) to a DOS date/time of (0,0).
 */
static void URLCache_FileTimeToDosDateTime(const FILETIME *ft, WORD *fatdate,
                                           WORD *fattime)
{
    if (!ft->dwLowDateTime && !ft->dwHighDateTime)
        *fatdate = *fattime = 0;
    else
        FileTimeToDosDateTime(ft, fatdate, fattime);
}

/***********************************************************************
 *           URLCache_SetEntryInfo (Internal)
 *
 *  Helper for SetUrlCacheEntryInfo{A,W}. Sets fields in URL entry
 * according to the flags set by dwFieldControl.
 *
 * RETURNS
 *    ERROR_SUCCESS if the buffer was big enough
 *    ERROR_INSUFFICIENT_BUFFER if the buffer was too small
 *
 */
static DWORD URLCache_SetEntryInfo(URL_CACHEFILE_ENTRY * pUrlEntry, const INTERNET_CACHE_ENTRY_INFOW * lpCacheEntryInfo, DWORD dwFieldControl)
{
    if (dwFieldControl & CACHE_ENTRY_ACCTIME_FC)
        pUrlEntry->LastAccessTime = lpCacheEntryInfo->LastAccessTime;
    if (dwFieldControl & CACHE_ENTRY_ATTRIBUTE_FC)
        pUrlEntry->CacheEntryType = lpCacheEntryInfo->CacheEntryType;
    if (dwFieldControl & CACHE_ENTRY_EXEMPT_DELTA_FC)
        pUrlEntry->dwExemptDelta = lpCacheEntryInfo->u.dwExemptDelta;
    if (dwFieldControl & CACHE_ENTRY_EXPTIME_FC)
        URLCache_FileTimeToDosDateTime(&lpCacheEntryInfo->ExpireTime, &pUrlEntry->wExpiredDate, &pUrlEntry->wExpiredTime);
    if (dwFieldControl & CACHE_ENTRY_HEADERINFO_FC)
        FIXME("CACHE_ENTRY_HEADERINFO_FC unimplemented\n");
    if (dwFieldControl & CACHE_ENTRY_HITRATE_FC)
        pUrlEntry->dwHitRate = lpCacheEntryInfo->dwHitRate;
    if (dwFieldControl & CACHE_ENTRY_MODTIME_FC)
        pUrlEntry->LastModifiedTime = lpCacheEntryInfo->LastModifiedTime;
    if (dwFieldControl & CACHE_ENTRY_SYNCTIME_FC)
        URLCache_FileTimeToDosDateTime(&lpCacheEntryInfo->LastAccessTime, &pUrlEntry->wLastSyncDate, &pUrlEntry->wLastSyncTime);

    return ERROR_SUCCESS;
}

/***********************************************************************
 *           URLCache_HashKey (Internal)
 *
 *  Returns the hash key for a given string
 *
 * RETURNS
 *    hash key for the string
 *
 */
static DWORD URLCache_HashKey(LPCSTR lpszKey)
{
    /* NOTE: this uses the same lookup table as SHLWAPI.UrlHash{A,W}
     * but the algorithm and result are not the same!
     */
    static const unsigned char lookupTable[256] = 
    {
        0x01, 0x0E, 0x6E, 0x19, 0x61, 0xAE, 0x84, 0x77,
        0x8A, 0xAA, 0x7D, 0x76, 0x1B, 0xE9, 0x8C, 0x33,
        0x57, 0xC5, 0xB1, 0x6B, 0xEA, 0xA9, 0x38, 0x44,
        0x1E, 0x07, 0xAD, 0x49, 0xBC, 0x28, 0x24, 0x41,
        0x31, 0xD5, 0x68, 0xBE, 0x39, 0xD3, 0x94, 0xDF,
        0x30, 0x73, 0x0F, 0x02, 0x43, 0xBA, 0xD2, 0x1C,
        0x0C, 0xB5, 0x67, 0x46, 0x16, 0x3A, 0x4B, 0x4E,
        0xB7, 0xA7, 0xEE, 0x9D, 0x7C, 0x93, 0xAC, 0x90,
        0xB0, 0xA1, 0x8D, 0x56, 0x3C, 0x42, 0x80, 0x53,
        0x9C, 0xF1, 0x4F, 0x2E, 0xA8, 0xC6, 0x29, 0xFE,
        0xB2, 0x55, 0xFD, 0xED, 0xFA, 0x9A, 0x85, 0x58,
        0x23, 0xCE, 0x5F, 0x74, 0xFC, 0xC0, 0x36, 0xDD,
        0x66, 0xDA, 0xFF, 0xF0, 0x52, 0x6A, 0x9E, 0xC9,
        0x3D, 0x03, 0x59, 0x09, 0x2A, 0x9B, 0x9F, 0x5D,
        0xA6, 0x50, 0x32, 0x22, 0xAF, 0xC3, 0x64, 0x63,
        0x1A, 0x96, 0x10, 0x91, 0x04, 0x21, 0x08, 0xBD,
        0x79, 0x40, 0x4D, 0x48, 0xD0, 0xF5, 0x82, 0x7A,
        0x8F, 0x37, 0x69, 0x86, 0x1D, 0xA4, 0xB9, 0xC2,
        0xC1, 0xEF, 0x65, 0xF2, 0x05, 0xAB, 0x7E, 0x0B,
        0x4A, 0x3B, 0x89, 0xE4, 0x6C, 0xBF, 0xE8, 0x8B,
        0x06, 0x18, 0x51, 0x14, 0x7F, 0x11, 0x5B, 0x5C,
        0xFB, 0x97, 0xE1, 0xCF, 0x15, 0x62, 0x71, 0x70,
        0x54, 0xE2, 0x12, 0xD6, 0xC7, 0xBB, 0x0D, 0x20,
        0x5E, 0xDC, 0xE0, 0xD4, 0xF7, 0xCC, 0xC4, 0x2B,
        0xF9, 0xEC, 0x2D, 0xF4, 0x6F, 0xB6, 0x99, 0x88,
        0x81, 0x5A, 0xD9, 0xCA, 0x13, 0xA5, 0xE7, 0x47,
        0xE6, 0x8E, 0x60, 0xE3, 0x3E, 0xB3, 0xF6, 0x72,
        0xA2, 0x35, 0xA0, 0xD7, 0xCD, 0xB4, 0x2F, 0x6D,
        0x2C, 0x26, 0x1F, 0x95, 0x87, 0x00, 0xD8, 0x34,
        0x3F, 0x17, 0x25, 0x45, 0x27, 0x75, 0x92, 0xB8,
        0xA3, 0xC8, 0xDE, 0xEB, 0xF8, 0xF3, 0xDB, 0x0A,
        0x98, 0x83, 0x7B, 0xE5, 0xCB, 0x4C, 0x78, 0xD1
    };
    BYTE key[4];
    DWORD i;

    for (i = 0; i < sizeof(key) / sizeof(key[0]); i++)
        key[i] = lookupTable[i];

    for (lpszKey++; *lpszKey && ((lpszKey[0] != '/') || (lpszKey[1] != 0)); lpszKey++)
    {
        for (i = 0; i < sizeof(key) / sizeof(key[0]); i++)
            key[i] = lookupTable[*lpszKey ^ key[i]];
    }

    return *(DWORD *)key;
}

static inline HASH_CACHEFILE_ENTRY * URLCache_HashEntryFromOffset(LPCURLCACHE_HEADER pHeader, DWORD dwOffset)
{
    return (HASH_CACHEFILE_ENTRY *)((LPBYTE)pHeader + dwOffset);
}

static inline BOOL URLCache_IsHashEntryValid(LPCURLCACHE_HEADER pHeader, const HASH_CACHEFILE_ENTRY *pHashEntry)
{
    /* check pHashEntry located within acceptable bounds in the URL cache mapping */
    return ((DWORD)((const BYTE*)pHashEntry - (const BYTE*)pHeader) >= ENTRY_START_OFFSET) &&
           ((DWORD)((const BYTE*)pHashEntry - (const BYTE*)pHeader) < pHeader->dwFileSize);
}

static BOOL URLCache_FindHash(LPCURLCACHE_HEADER pHeader, LPCSTR lpszUrl, struct _HASH_ENTRY ** ppHashEntry)
{
    /* structure of hash table:
     *  448 entries divided into 64 blocks
     *  each block therefore contains a chain of 7 key/offset pairs
     * how position in table is calculated:
     *  1. the url is hashed in helper function
     *  2. the key % 64 * 8 is the offset
     *  3. the key in the hash table is the hash key aligned to 64
     *
     * note:
     *  there can be multiple hash tables in the file and the offset to
     *  the next one is stored in the header of the hash table
     */
    DWORD key = URLCache_HashKey(lpszUrl);
    DWORD offset = (key % HASHTABLE_NUM_ENTRIES) * sizeof(struct _HASH_ENTRY);
    HASH_CACHEFILE_ENTRY * pHashEntry;
    DWORD dwHashTableNumber = 0;

    key = (key / HASHTABLE_NUM_ENTRIES) * HASHTABLE_NUM_ENTRIES;

    for (pHashEntry = URLCache_HashEntryFromOffset(pHeader, pHeader->dwOffsetFirstHashTable);
         URLCache_IsHashEntryValid(pHeader, pHashEntry);
         pHashEntry = URLCache_HashEntryFromOffset(pHeader, pHashEntry->dwAddressNext))
    {
        int i;
        if (pHashEntry->dwHashTableNumber != dwHashTableNumber++)
        {
            ERR("Error: not right hash table number (%d) expected %d\n", pHashEntry->dwHashTableNumber, dwHashTableNumber);
            continue;
        }
        /* make sure that it is in fact a hash entry */
        if (pHashEntry->CacheFileEntry.dwSignature != HASH_SIGNATURE)
        {
            ERR("Error: not right signature (\"%.4s\") - expected \"HASH\"\n", (LPCSTR)&pHashEntry->CacheFileEntry.dwSignature);
            continue;
        }

        for (i = 0; i < HASHTABLE_BLOCKSIZE; i++)
        {
            struct _HASH_ENTRY * pHashElement = &pHashEntry->HashTable[offset + i];
            if (key == (pHashElement->dwHashKey / HASHTABLE_NUM_ENTRIES) * HASHTABLE_NUM_ENTRIES)
            {
                /* FIXME: we should make sure that this is the right element
                 * before returning and claiming that it is. We can do this
                 * by doing a simple compare between the URL we were given
                 * and the URL stored in the entry. However, this assumes
                 * we know the format of all the entries stored in the
                 * hash table */
                *ppHashEntry = pHashElement;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static BOOL URLCache_FindHashW(LPCURLCACHE_HEADER pHeader, LPCWSTR lpszUrl, struct _HASH_ENTRY ** ppHashEntry)
{
    LPSTR urlA;
    BOOL ret;

    urlA = heap_strdupWtoA(lpszUrl);
    if (!urlA)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    ret = URLCache_FindHash(pHeader, urlA, ppHashEntry);
    HeapFree(GetProcessHeap(), 0, urlA);
    return ret;
}

/***********************************************************************
 *           URLCache_HashEntrySetUse (Internal)
 *
 *  Searches all the hash tables in the index for the given URL and
 * sets the use count (stored or'ed with key)
 *
 * RETURNS
 *    TRUE if the entry was found
 *    FALSE if the entry could not be found
 *
 */
static BOOL URLCache_HashEntrySetUse(struct _HASH_ENTRY * pHashEntry, DWORD dwUseCount)
{
    pHashEntry->dwHashKey = dwUseCount | (pHashEntry->dwHashKey / HASHTABLE_NUM_ENTRIES) * HASHTABLE_NUM_ENTRIES;
    return TRUE;
}

/***********************************************************************
 *           URLCache_DeleteEntryFromHash (Internal)
 *
 *  Searches all the hash tables in the index for the given URL and
 * then if found deletes the entry.
 *
 * RETURNS
 *    TRUE if the entry was found
 *    FALSE if the entry could not be found
 *
 */
static BOOL URLCache_DeleteEntryFromHash(struct _HASH_ENTRY * pHashEntry)
{
    pHashEntry->dwHashKey = HASHTABLE_FREE;
    pHashEntry->dwOffsetEntry = HASHTABLE_FREE;
    return TRUE;
}

/***********************************************************************
 *           URLCache_AddEntryToHash (Internal)
 *
 *  Searches all the hash tables for a free slot based on the offset
 * generated from the hash key. If a free slot is found, the offset and
 * key are entered into the hash table.
 *
 * RETURNS
 *    ERROR_SUCCESS if the entry was added
 *    Any other Win32 error code if the entry could not be added
 *
 */
static DWORD URLCache_AddEntryToHash(LPURLCACHE_HEADER pHeader, LPCSTR lpszUrl, DWORD dwOffsetEntry)
{
    /* see URLCache_FindEntryInHash for structure of hash tables */

    DWORD key = URLCache_HashKey(lpszUrl);
    DWORD offset = (key % HASHTABLE_NUM_ENTRIES) * sizeof(struct _HASH_ENTRY);
    HASH_CACHEFILE_ENTRY * pHashEntry;
    DWORD dwHashTableNumber = 0;
    DWORD error;

    key = (key / HASHTABLE_NUM_ENTRIES) * HASHTABLE_NUM_ENTRIES;

    for (pHashEntry = URLCache_HashEntryFromOffset(pHeader, pHeader->dwOffsetFirstHashTable);
         URLCache_IsHashEntryValid(pHeader, pHashEntry);
         pHashEntry = URLCache_HashEntryFromOffset(pHeader, pHashEntry->dwAddressNext))
    {
        int i;
        if (pHashEntry->dwHashTableNumber != dwHashTableNumber++)
        {
            ERR("not right hash table number (%d) expected %d\n", pHashEntry->dwHashTableNumber, dwHashTableNumber);
            break;
        }
        /* make sure that it is in fact a hash entry */
        if (pHashEntry->CacheFileEntry.dwSignature != HASH_SIGNATURE)
        {
            ERR("not right signature (\"%.4s\") - expected \"HASH\"\n", (LPCSTR)&pHashEntry->CacheFileEntry.dwSignature);
            break;
        }

        for (i = 0; i < HASHTABLE_BLOCKSIZE; i++)
        {
            struct _HASH_ENTRY * pHashElement = &pHashEntry->HashTable[offset + i];
            if (pHashElement->dwHashKey == HASHTABLE_FREE) /* if the slot is free */
            {
                pHashElement->dwHashKey = key;
                pHashElement->dwOffsetEntry = dwOffsetEntry;
                return ERROR_SUCCESS;
            }
        }
    }
    error = URLCache_CreateHashTable(pHeader, pHashEntry, &pHashEntry);
    if (error != ERROR_SUCCESS)
        return error;

    pHashEntry->HashTable[offset].dwHashKey = key;
    pHashEntry->HashTable[offset].dwOffsetEntry = dwOffsetEntry;
    return ERROR_SUCCESS;
}

/***********************************************************************
 *           URLCache_CreateHashTable (Internal)
 *
 *  Creates a new hash table in free space and adds it to the chain of existing
 * hash tables.
 *
 * RETURNS
 *    ERROR_SUCCESS if the hash table was created
 *    ERROR_DISK_FULL if the hash table could not be created
 *
 */
static DWORD URLCache_CreateHashTable(LPURLCACHE_HEADER pHeader, HASH_CACHEFILE_ENTRY *pPrevHash, HASH_CACHEFILE_ENTRY **ppHash)
{
    DWORD dwOffset;
    int i;

    if (!URLCache_FindFirstFreeEntry(pHeader, 0x20, (CACHEFILE_ENTRY **)ppHash))
    {
        FIXME("no free space for hash table\n");
        return ERROR_DISK_FULL;
    }

    dwOffset = (BYTE *)*ppHash - (BYTE *)pHeader;

    if (pPrevHash)
        pPrevHash->dwAddressNext = dwOffset;
    else
        pHeader->dwOffsetFirstHashTable = dwOffset;
    (*ppHash)->CacheFileEntry.dwSignature = HASH_SIGNATURE;
    (*ppHash)->CacheFileEntry.dwBlocksUsed = 0x20;
    (*ppHash)->dwHashTableNumber = pPrevHash ? pPrevHash->dwHashTableNumber + 1 : 0;
    for (i = 0; i < HASHTABLE_SIZE; i++)
    {
        (*ppHash)->HashTable[i].dwOffsetEntry = 0;
        (*ppHash)->HashTable[i].dwHashKey = HASHTABLE_FREE;
    }
    return ERROR_SUCCESS;
}

/***********************************************************************
 *           URLCache_EnumHashTables (Internal)
 *
 *  Enumerates the hash tables in a container.
 *
 * RETURNS
 *    TRUE if an entry was found
 *    FALSE if there are no more tables to enumerate.
 *
 */
static BOOL URLCache_EnumHashTables(LPCURLCACHE_HEADER pHeader, DWORD *pdwHashTableNumber, HASH_CACHEFILE_ENTRY ** ppHashEntry)
{
    for (*ppHashEntry = URLCache_HashEntryFromOffset(pHeader, pHeader->dwOffsetFirstHashTable);
         URLCache_IsHashEntryValid(pHeader, *ppHashEntry);
         *ppHashEntry = URLCache_HashEntryFromOffset(pHeader, (*ppHashEntry)->dwAddressNext))
    {
        TRACE("looking at hash table number %d\n", (*ppHashEntry)->dwHashTableNumber);
        if ((*ppHashEntry)->dwHashTableNumber != *pdwHashTableNumber)
            continue;
        /* make sure that it is in fact a hash entry */
        if ((*ppHashEntry)->CacheFileEntry.dwSignature != HASH_SIGNATURE)
        {
            ERR("Error: not right signature (\"%.4s\") - expected \"HASH\"\n", (LPCSTR)&(*ppHashEntry)->CacheFileEntry.dwSignature);
            (*pdwHashTableNumber)++;
            continue;
        }

        TRACE("hash table number %d found\n", *pdwHashTableNumber);
        return TRUE;
    }
    return FALSE;
}

/***********************************************************************
 *           URLCache_EnumHashTableEntries (Internal)
 *
 *  Enumerates entries in a hash table and returns the next non-free entry.
 *
 * RETURNS
 *    TRUE if an entry was found
 *    FALSE if the hash table is empty or there are no more entries to
 *     enumerate.
 *
 */
static BOOL URLCache_EnumHashTableEntries(LPCURLCACHE_HEADER pHeader, const HASH_CACHEFILE_ENTRY * pHashEntry,
                                          DWORD * index, const struct _HASH_ENTRY ** ppHashEntry)
{
    for (; *index < HASHTABLE_SIZE ; (*index)++)
    {
        if (pHashEntry->HashTable[*index].dwHashKey == HASHTABLE_FREE)
            continue;

        *ppHashEntry = &pHashEntry->HashTable[*index];
        TRACE("entry found %d\n", *index);
        return TRUE;
    }
    TRACE("no more entries (%d)\n", *index);
    return FALSE;
}

/***********************************************************************
 *           URLCache_DeleteCacheDirectory (Internal)
 *
 *  Erase a directory containing an URL cache.
 *
 * RETURNS
 *    TRUE success, FALSE failure/aborted.
 *
 */
static BOOL URLCache_DeleteCacheDirectory(LPCWSTR lpszPath)
{
    DWORD path_len;
    WCHAR path[MAX_PATH + 1];
    SHFILEOPSTRUCTW shfos;
    int ret;

    path_len = strlenW(lpszPath);
    if (path_len >= MAX_PATH)
        return FALSE;
    strcpyW(path, lpszPath);
    path[path_len + 1] = 0;  /* double-NUL-terminate path */

    shfos.hwnd = NULL;
    shfos.wFunc = FO_DELETE;
    shfos.pFrom = path;
    shfos.pTo = NULL;
    shfos.fFlags = 0;
    shfos.fAnyOperationsAborted = FALSE;
    ret = SHFileOperationW(&shfos);
    if (ret)
        ERR("SHFileOperationW on %s returned %i\n", debugstr_w(path), ret);
    return !(ret || shfos.fAnyOperationsAborted);
}

/***********************************************************************
 *           FreeUrlCacheSpaceW (WININET.@)
 *
 * Frees up some cache.
 *
 * PARAMETERS
 *   lpszCachePath [I] Which volume to free up from, or NULL if you don't care.
 *   dwSize        [I] How much space to free up.
 *   dwSizeType    [I] How to interpret dwSize.
 *
 * RETURNS
 *   TRUE success. FALSE failure.
 *
 * IMPLEMENTATION
 *   This implementation just retrieves the path of the cache directory, and
 *   deletes its contents from the filesystem. The correct approach would
 *   probably be to implement and use {FindFirst,FindNext,Delete}UrlCacheGroup().
 */
BOOL WINAPI FreeUrlCacheSpaceW(LPCWSTR lpszCachePath, DWORD dwSize, DWORD dwSizeType)
{
    URLCACHECONTAINER * pContainer;

    if (lpszCachePath != NULL || dwSize != 100 || dwSizeType != FCS_PERCENT_CACHE_SPACE)
    {
        FIXME("(%s, %x, %x): partial stub!\n", debugstr_w(lpszCachePath), dwSize, dwSizeType);
        SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    LIST_FOR_EACH_ENTRY(pContainer, &UrlContainers, URLCACHECONTAINER, entry)
    {
        /* The URL cache has prefix L"" (unlike Cookies and History) */
        if (pContainer->cache_prefix[0] == 0)
        {
            BOOL ret_del;
            DWORD ret_open;
            WaitForSingleObject(pContainer->hMutex, INFINITE);

            /* unlock, delete, recreate and lock cache */
            URLCacheContainer_CloseIndex(pContainer);
            ret_del = URLCache_DeleteCacheDirectory(pContainer->path);
            ret_open = URLCacheContainer_OpenIndex(pContainer);

            ReleaseMutex(pContainer->hMutex);
            return ret_del && (ret_open == ERROR_SUCCESS);
        }
    }
    return FALSE;
}

/***********************************************************************
 *           FreeUrlCacheSpaceA (WININET.@)
 *
 * See FreeUrlCacheSpaceW.
 */
BOOL WINAPI FreeUrlCacheSpaceA(LPCSTR lpszCachePath, DWORD dwSize, DWORD dwSizeType)
{
    BOOL ret = FALSE;
    LPWSTR path = heap_strdupAtoW(lpszCachePath);
    if (lpszCachePath == NULL || path != NULL)
        ret = FreeUrlCacheSpaceW(path, dwSize, dwSizeType);
    heap_free(path);
    return ret;
}

/***********************************************************************
 *           GetUrlCacheEntryInfoExA (WININET.@)
 *
 */
BOOL WINAPI GetUrlCacheEntryInfoExA(
    LPCSTR lpszUrl,
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo,
    LPDWORD lpdwCacheEntryInfoBufSize,
    LPSTR lpszReserved,
    LPDWORD lpdwReserved,
    LPVOID lpReserved,
    DWORD dwFlags)
{
    TRACE("(%s, %p, %p, %p, %p, %p, %x)\n",
        debugstr_a(lpszUrl), 
        lpCacheEntryInfo,
        lpdwCacheEntryInfoBufSize,
        lpszReserved,
        lpdwReserved,
        lpReserved,
        dwFlags);

    if ((lpszReserved != NULL) ||
        (lpdwReserved != NULL) ||
        (lpReserved != NULL))
    {
        ERR("Reserved value was not 0\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (dwFlags != 0)
    {
        FIXME("Undocumented flag(s): %x\n", dwFlags);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }
    return GetUrlCacheEntryInfoA(lpszUrl, lpCacheEntryInfo, lpdwCacheEntryInfoBufSize);
}

/***********************************************************************
 *           GetUrlCacheEntryInfoA (WININET.@)
 *
 */
BOOL WINAPI GetUrlCacheEntryInfoA(
    IN LPCSTR lpszUrlName,
    IN LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize
)
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    const CACHEFILE_ENTRY * pEntry;
    const URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, %p, %p)\n", debugstr_a(lpszUrlName), lpCacheEntryInfo, lpdwCacheEntryInfoBufferSize);

    error = URLCacheContainers_FindContainerA(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHash(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        WARN("entry %s not found!\n", debugstr_a(lpszUrlName));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (const CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPCSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pUrlEntry = (const URL_CACHEFILE_ENTRY *)pEntry;
    TRACE("Found URL: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl));
    if (pUrlEntry->dwOffsetHeaderInfo)
        TRACE("Header info: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetHeaderInfo));

    if (lpdwCacheEntryInfoBufferSize)
    {
        if (!lpCacheEntryInfo)
            *lpdwCacheEntryInfoBufferSize = 0;

        error = URLCache_CopyEntry(
            pContainer,
            pHeader,
            lpCacheEntryInfo,
            lpdwCacheEntryInfoBufferSize,
            pUrlEntry,
            FALSE /* ANSI */);
        if (error != ERROR_SUCCESS)
        {
            URLCacheContainer_UnlockIndex(pContainer, pHeader);
            SetLastError(error);
            return FALSE;
        }
        TRACE("Local File Name: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName));
    }

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           GetUrlCacheEntryInfoW (WININET.@)
 *
 */
BOOL WINAPI GetUrlCacheEntryInfoW(LPCWSTR lpszUrl,
  LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo,
  LPDWORD lpdwCacheEntryInfoBufferSize)
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    const CACHEFILE_ENTRY * pEntry;
    const URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, %p, %p)\n", debugstr_w(lpszUrl), lpCacheEntryInfo, lpdwCacheEntryInfoBufferSize);

    error = URLCacheContainers_FindContainerW(lpszUrl, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHashW(pHeader, lpszUrl, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        WARN("entry %s not found!\n", debugstr_w(lpszUrl));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (const CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPCSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pUrlEntry = (const URL_CACHEFILE_ENTRY *)pEntry;
    TRACE("Found URL: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl));
    TRACE("Header info: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetHeaderInfo));

    if (lpdwCacheEntryInfoBufferSize)
    {
        if (!lpCacheEntryInfo)
            *lpdwCacheEntryInfoBufferSize = 0;

        error = URLCache_CopyEntry(
            pContainer,
            pHeader,
            (LPINTERNET_CACHE_ENTRY_INFOA)lpCacheEntryInfo,
            lpdwCacheEntryInfoBufferSize,
            pUrlEntry,
            TRUE /* UNICODE */);
        if (error != ERROR_SUCCESS)
        {
            URLCacheContainer_UnlockIndex(pContainer, pHeader);
            SetLastError(error);
            return FALSE;
        }
        TRACE("Local File Name: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName));
    }

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           GetUrlCacheEntryInfoExW (WININET.@)
 *
 */
BOOL WINAPI GetUrlCacheEntryInfoExW(
    LPCWSTR lpszUrl,
    LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo,
    LPDWORD lpdwCacheEntryInfoBufSize,
    LPWSTR lpszReserved,
    LPDWORD lpdwReserved,
    LPVOID lpReserved,
    DWORD dwFlags)
{
    TRACE("(%s, %p, %p, %p, %p, %p, %x)\n",
        debugstr_w(lpszUrl), 
        lpCacheEntryInfo,
        lpdwCacheEntryInfoBufSize,
        lpszReserved,
        lpdwReserved,
        lpReserved,
        dwFlags);

    if ((lpszReserved != NULL) ||
        (lpdwReserved != NULL) ||
        (lpReserved != NULL))
    {
        ERR("Reserved value was not 0\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (dwFlags != 0)
    {
        FIXME("Undocumented flag(s): %x\n", dwFlags);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }
    return GetUrlCacheEntryInfoW(lpszUrl, lpCacheEntryInfo, lpdwCacheEntryInfoBufSize);
}

/***********************************************************************
 *           SetUrlCacheEntryInfoA (WININET.@)
 */
BOOL WINAPI SetUrlCacheEntryInfoA(
    LPCSTR lpszUrlName,
    LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo,
    DWORD dwFieldControl)
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, %p, 0x%08x)\n", debugstr_a(lpszUrlName), lpCacheEntryInfo, dwFieldControl);

    error = URLCacheContainers_FindContainerA(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHash(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        WARN("entry %s not found!\n", debugstr_a(lpszUrlName));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    URLCache_SetEntryInfo(
        (URL_CACHEFILE_ENTRY *)pEntry,
        (const INTERNET_CACHE_ENTRY_INFOW *)lpCacheEntryInfo,
        dwFieldControl);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           SetUrlCacheEntryInfoW (WININET.@)
 */
BOOL WINAPI SetUrlCacheEntryInfoW(LPCWSTR lpszUrl, LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo, DWORD dwFieldControl)
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, %p, 0x%08x)\n", debugstr_w(lpszUrl), lpCacheEntryInfo, dwFieldControl);

    error = URLCacheContainers_FindContainerW(lpszUrl, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHashW(pHeader, lpszUrl, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        WARN("entry %s not found!\n", debugstr_w(lpszUrl));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    URLCache_SetEntryInfo(
        (URL_CACHEFILE_ENTRY *)pEntry,
        lpCacheEntryInfo,
        dwFieldControl);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           RetrieveUrlCacheEntryFileA (WININET.@)
 *
 */
BOOL WINAPI RetrieveUrlCacheEntryFileA(
    IN LPCSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo, 
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize,
    IN DWORD dwReserved
    )
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, %p, %p, 0x%08x)\n",
        debugstr_a(lpszUrlName),
        lpCacheEntryInfo,
        lpdwCacheEntryInfoBufferSize,
        dwReserved);

    if (!lpszUrlName || !lpdwCacheEntryInfoBufferSize ||
        (!lpCacheEntryInfo && *lpdwCacheEntryInfoBufferSize))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    error = URLCacheContainers_FindContainerA(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHash(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        TRACE("entry %s not found!\n", lpszUrlName);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pUrlEntry = (URL_CACHEFILE_ENTRY *)pEntry;
    if (!pUrlEntry->dwOffsetLocalName)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    TRACE("Found URL: %s\n", (LPSTR)pUrlEntry + pUrlEntry->dwOffsetUrl);
    TRACE("Header info: %s\n", (LPBYTE)pUrlEntry + pUrlEntry->dwOffsetHeaderInfo);

    error = URLCache_CopyEntry(pContainer, pHeader, lpCacheEntryInfo,
                               lpdwCacheEntryInfoBufferSize, pUrlEntry,
                               FALSE);
    if (error != ERROR_SUCCESS)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        SetLastError(error);
        return FALSE;
    }
    TRACE("Local File Name: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName));

    pUrlEntry->dwHitRate++;
    pUrlEntry->dwUseCount++;
    URLCache_HashEntrySetUse(pHashEntry, pUrlEntry->dwUseCount);
    GetSystemTimeAsFileTime(&pUrlEntry->LastAccessTime);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           RetrieveUrlCacheEntryFileW (WININET.@)
 *
 */
BOOL WINAPI RetrieveUrlCacheEntryFileW(
    IN LPCWSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize,
    IN DWORD dwReserved
    )
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, %p, %p, 0x%08x)\n",
        debugstr_w(lpszUrlName),
        lpCacheEntryInfo,
        lpdwCacheEntryInfoBufferSize,
        dwReserved);

    if (!lpszUrlName || !lpdwCacheEntryInfoBufferSize ||
        (!lpCacheEntryInfo && *lpdwCacheEntryInfoBufferSize))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    error = URLCacheContainers_FindContainerW(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHashW(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        TRACE("entry %s not found!\n", debugstr_w(lpszUrlName));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pUrlEntry = (URL_CACHEFILE_ENTRY *)pEntry;
    if (!pUrlEntry->dwOffsetLocalName)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        SetLastError(ERROR_INVALID_DATA);
        return FALSE;
    }

    TRACE("Found URL: %s\n", (LPSTR)pUrlEntry + pUrlEntry->dwOffsetUrl);
    TRACE("Header info: %s\n", (LPBYTE)pUrlEntry + pUrlEntry->dwOffsetHeaderInfo);

    error = URLCache_CopyEntry(
        pContainer,
        pHeader,
        (LPINTERNET_CACHE_ENTRY_INFOA)lpCacheEntryInfo,
        lpdwCacheEntryInfoBufferSize,
        pUrlEntry,
        TRUE /* UNICODE */);
    if (error != ERROR_SUCCESS)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        SetLastError(error);
        return FALSE;
    }
    TRACE("Local File Name: %s\n", debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName));

    pUrlEntry->dwHitRate++;
    pUrlEntry->dwUseCount++;
    URLCache_HashEntrySetUse(pHashEntry, pUrlEntry->dwUseCount);
    GetSystemTimeAsFileTime(&pUrlEntry->LastAccessTime);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

static BOOL DeleteUrlCacheEntryInternal(LPURLCACHE_HEADER pHeader,
        struct _HASH_ENTRY *pHashEntry)
{
    CACHEFILE_ENTRY * pEntry;
    URL_CACHEFILE_ENTRY * pUrlEntry;

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        FIXME("Trying to delete entry of unknown format %s\n",
              debugstr_an((LPCSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }
    pUrlEntry = (URL_CACHEFILE_ENTRY *)pEntry;
    if (pUrlEntry->CacheDir < pHeader->DirectoryCount)
    {
        if (pHeader->directory_data[pUrlEntry->CacheDir].dwNumFiles)
            pHeader->directory_data[pUrlEntry->CacheDir].dwNumFiles--;
    }
    if (pUrlEntry->CacheEntryType & STICKY_CACHE_ENTRY)
    {
        if (pUrlEntry->size.QuadPart < pHeader->ExemptUsage.QuadPart)
            pHeader->ExemptUsage.QuadPart -= pUrlEntry->size.QuadPart;
        else
            pHeader->ExemptUsage.QuadPart = 0;
    }
    else
    {
        if (pUrlEntry->size.QuadPart < pHeader->CacheUsage.QuadPart)
            pHeader->CacheUsage.QuadPart -= pUrlEntry->size.QuadPart;
        else
            pHeader->CacheUsage.QuadPart = 0;
    }

    URLCache_DeleteEntry(pHeader, pEntry);

    URLCache_DeleteEntryFromHash(pHashEntry);
    return TRUE;
}

/***********************************************************************
 *           UnlockUrlCacheEntryFileA (WININET.@)
 *
 */
BOOL WINAPI UnlockUrlCacheEntryFileA(
    IN LPCSTR lpszUrlName, 
    IN DWORD dwReserved
    )
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, 0x%08x)\n", debugstr_a(lpszUrlName), dwReserved);

    if (dwReserved)
    {
        ERR("dwReserved != 0\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    error = URLCacheContainers_FindContainerA(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
       SetLastError(error);
       return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHash(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        TRACE("entry %s not found!\n", lpszUrlName);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pUrlEntry = (URL_CACHEFILE_ENTRY *)pEntry;

    if (pUrlEntry->dwUseCount == 0)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        return FALSE;
    }
    pUrlEntry->dwUseCount--;
    URLCache_HashEntrySetUse(pHashEntry, pUrlEntry->dwUseCount);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           UnlockUrlCacheEntryFileW (WININET.@)
 *
 */
BOOL WINAPI UnlockUrlCacheEntryFileW( LPCWSTR lpszUrlName, DWORD dwReserved )
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    DWORD error;

    TRACE("(%s, 0x%08x)\n", debugstr_w(lpszUrlName), dwReserved);

    if (dwReserved)
    {
        ERR("dwReserved != 0\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    error = URLCacheContainers_FindContainerW(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHashW(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        TRACE("entry %s not found!\n", debugstr_w(lpszUrlName));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pEntry = (CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPSTR)&pEntry->dwSignature, sizeof(DWORD)));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    pUrlEntry = (URL_CACHEFILE_ENTRY *)pEntry;

    if (pUrlEntry->dwUseCount == 0)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        return FALSE;
    }
    pUrlEntry->dwUseCount--;
    URLCache_HashEntrySetUse(pHashEntry, pUrlEntry->dwUseCount);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return TRUE;
}

/***********************************************************************
 *           CreateUrlCacheEntryA (WININET.@)
 *
 */
BOOL WINAPI CreateUrlCacheEntryA(
    IN LPCSTR lpszUrlName,
    IN DWORD dwExpectedFileSize,
    IN LPCSTR lpszFileExtension,
    OUT LPSTR lpszFileName,
    IN DWORD dwReserved
)
{
    WCHAR *url_name;
    WCHAR *file_extension = NULL;
    WCHAR file_name[MAX_PATH];
    BOOL bSuccess = FALSE;
    DWORD dwError = 0;

    TRACE("(%s %d %s %p %d)\n", debugstr_a(lpszUrlName), dwExpectedFileSize,
            debugstr_a(lpszFileExtension), lpszFileName, dwReserved);

    if (lpszUrlName && (url_name = heap_strdupAtoW(lpszUrlName)))
    {
	if (!lpszFileExtension || (file_extension = heap_strdupAtoW(lpszFileExtension)))
	{
	    if (CreateUrlCacheEntryW(url_name, dwExpectedFileSize, file_extension, file_name, dwReserved))
	    {
		if (WideCharToMultiByte(CP_ACP, 0, file_name, -1, lpszFileName, MAX_PATH, NULL, NULL) < MAX_PATH)
		{
		    bSuccess = TRUE;
		}
		else
		{
		    dwError = GetLastError();
		}
	    }
	    else
	    {
		dwError = GetLastError();
	    }
	    HeapFree(GetProcessHeap(), 0, file_extension);
	}
	else
	{
	    dwError = GetLastError();
	}
        HeapFree(GetProcessHeap(), 0, url_name);
	if (!bSuccess)
	    SetLastError(dwError);
    }
    return bSuccess;
}
/***********************************************************************
 *           CreateUrlCacheEntryW (WININET.@)
 *
 */
BOOL WINAPI CreateUrlCacheEntryW(
    IN LPCWSTR lpszUrlName,
    IN DWORD dwExpectedFileSize,
    IN LPCWSTR lpszFileExtension,
    OUT LPWSTR lpszFileName,
    IN DWORD dwReserved
)
{
    URLCACHECONTAINER * pContainer;
    LPURLCACHE_HEADER pHeader;
    CHAR szFile[MAX_PATH];
    WCHAR szExtension[MAX_PATH];
    LPCWSTR lpszUrlPart;
    LPCWSTR lpszUrlEnd;
    LPCWSTR lpszFileNameExtension;
    LPWSTR lpszFileNameNoPath;
    int i;
    int countnoextension;
    BYTE CacheDir;
    LONG lBufferSize;
    BOOL bFound = FALSE;
    int count;
    DWORD error;
    HANDLE hFile;
    FILETIME ft;

    static const WCHAR szWWW[] = {'w','w','w',0};
    static const WCHAR fmt[] = {'%','0','8','X','%','s',0};

    TRACE("(%s, 0x%08x, %s, %p, 0x%08x)\n",
        debugstr_w(lpszUrlName),
        dwExpectedFileSize,
        debugstr_w(lpszFileExtension),
        lpszFileName,
        dwReserved);

    if (dwReserved)
        FIXME("dwReserved 0x%08x\n", dwReserved);

   lpszUrlEnd = lpszUrlName + strlenW(lpszUrlName);
    
    if (((lpszUrlEnd - lpszUrlName) > 1) && (*(lpszUrlEnd - 1) == '/' || *(lpszUrlEnd - 1) == '\\'))
        lpszUrlEnd--;

    lpszUrlPart = memchrW(lpszUrlName, '?', lpszUrlEnd - lpszUrlName);
    if (!lpszUrlPart)
        lpszUrlPart = memchrW(lpszUrlName, '#', lpszUrlEnd - lpszUrlName);
    if (lpszUrlPart)
        lpszUrlEnd = lpszUrlPart;

    for (lpszUrlPart = lpszUrlEnd; 
        (lpszUrlPart >= lpszUrlName); 
        lpszUrlPart--)
    {
        if ((*lpszUrlPart == '/' || *lpszUrlPart == '\\') && ((lpszUrlEnd - lpszUrlPart) > 1))
        {
            bFound = TRUE;
            lpszUrlPart++;
            break;
        }
    }
    if (!lstrcmpW(lpszUrlPart, szWWW))
    {
        lpszUrlPart += lstrlenW(szWWW);
    }

    count = lpszUrlEnd - lpszUrlPart;

    if (bFound && (count < MAX_PATH))
    {
	int len = WideCharToMultiByte(CP_ACP, 0, lpszUrlPart, count, szFile, sizeof(szFile) - 1, NULL, NULL);
	if (!len)
	    return FALSE;
        szFile[len] = '\0';
        while(len && szFile[--len] == '/') szFile[len] = '\0';

        /* FIXME: get rid of illegal characters like \, / and : */
    }
    else
    {
        FIXME("need to generate a random filename\n");
    }

    TRACE("File name: %s\n", debugstr_a(szFile));

    error = URLCacheContainers_FindContainerW(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    CacheDir = (BYTE)(rand() % pHeader->DirectoryCount);

    lBufferSize = MAX_PATH * sizeof(WCHAR);
    if (!URLCache_LocalFileNameToPathW(pContainer, pHeader, szFile, CacheDir, lpszFileName, &lBufferSize))
    {
        WARN("Failed to get full path for filename %s, needed %u bytes.\n",
                debugstr_a(szFile), lBufferSize);
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        return FALSE;
    }

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    for (lpszFileNameNoPath = lpszFileName + lBufferSize / sizeof(WCHAR) - 2;
        lpszFileNameNoPath >= lpszFileName; 
        --lpszFileNameNoPath)
    {
        if (*lpszFileNameNoPath == '/' || *lpszFileNameNoPath == '\\')
            break;
    }

    countnoextension = lstrlenW(lpszFileNameNoPath);
    lpszFileNameExtension = PathFindExtensionW(lpszFileNameNoPath);
    if (lpszFileNameExtension)
        countnoextension -= lstrlenW(lpszFileNameExtension);
    *szExtension = '\0';

    if (lpszFileExtension)
    {
        szExtension[0] = '.';
        lstrcpyW(szExtension+1, lpszFileExtension);
    }

    for (i = 0; i < 255; i++)
    {
        static const WCHAR szFormat[] = {'[','%','u',']','%','s',0};
        WCHAR *p;

        wsprintfW(lpszFileNameNoPath + countnoextension, szFormat, i, szExtension);
        for (p = lpszFileNameNoPath + 1; *p; p++)
        {
            switch (*p)
            {
            case '<': case '>':
            case ':': case '"':
            case '/': case '\\':
            case '|': case '?':
            case '*':
                *p = '_'; break;
            default: break;
            }
        }
        if (p[-1] == ' ' || p[-1] == '.') p[-1] = '_';

        TRACE("Trying: %s\n", debugstr_w(lpszFileName));
        hFile = CreateFileW(lpszFileName, GENERIC_READ, 0, NULL, CREATE_NEW, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFile);
            return TRUE;
        }
    }

    GetSystemTimeAsFileTime(&ft);
    wsprintfW(lpszFileNameNoPath + countnoextension, fmt, ft.dwLowDateTime, szExtension);

    TRACE("Trying: %s\n", debugstr_w(lpszFileName));
    hFile = CreateFileW(lpszFileName, GENERIC_READ, 0, NULL, CREATE_NEW, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        return TRUE;
    }

    WARN("Could not find a unique filename\n");
    return FALSE;
}


/***********************************************************************
 *           CommitUrlCacheEntryInternal (Compensates for an MS bug)
 *
 *   The bug we are compensating for is that some drongo at Microsoft
 *   used lpHeaderInfo to pass binary data to CommitUrlCacheEntryA.
 *   As a consequence, CommitUrlCacheEntryA has been effectively
 *   redefined as LPBYTE rather than LPCSTR. But CommitUrlCacheEntryW
 *   is still defined as LPCWSTR. The result (other than madness) is
 *   that we always need to store lpHeaderInfo in CP_ACP rather than
 *   in UTF16, and we need to avoid converting lpHeaderInfo in
 *   CommitUrlCacheEntryA to UTF16 and then back to CP_ACP, since the
 *   result will lose data for arbitrary binary data.
 *
 */
static BOOL CommitUrlCacheEntryInternal(
    IN LPCWSTR lpszUrlName,
    IN LPCWSTR lpszLocalFileName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN LPBYTE lpHeaderInfo,
    IN DWORD dwHeaderSize,
    IN LPCWSTR lpszFileExtension,
    IN LPCWSTR lpszOriginalUrl
    )
{
    URLCACHECONTAINER * pContainer;
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    CACHEFILE_ENTRY * pEntry;
    URL_CACHEFILE_ENTRY * pUrlEntry;
    DWORD dwBytesNeeded = DWORD_ALIGN(sizeof(*pUrlEntry));
    DWORD dwOffsetLocalFileName = 0;
    DWORD dwOffsetHeader = 0;
    DWORD dwOffsetFileExtension = 0;
    LARGE_INTEGER file_size;
    BYTE cDirectory = 0;
    char achFile[MAX_PATH];
    LPSTR lpszUrlNameA = NULL;
    LPSTR lpszFileExtensionA = NULL;
    char *pchLocalFileName = 0;
    DWORD error;

    TRACE("(%s, %s, ..., ..., %x, %p, %d, %s, %s)\n",
        debugstr_w(lpszUrlName),
        debugstr_w(lpszLocalFileName),
        CacheEntryType,
        lpHeaderInfo,
        dwHeaderSize,
        debugstr_w(lpszFileExtension),
        debugstr_w(lpszOriginalUrl));

    if (CacheEntryType & STICKY_CACHE_ENTRY && !lpszLocalFileName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    if (lpszOriginalUrl)
        WARN(": lpszOriginalUrl ignored\n");

    file_size.QuadPart = 0;
    if (lpszLocalFileName)
    {
        HANDLE hFile;

        hFile = CreateFileW(lpszLocalFileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            ERR("couldn't open file %s (error is %d)\n", debugstr_w(lpszLocalFileName), GetLastError());
            return FALSE;
        }

        /* Get file size */
        if (!GetFileSizeEx(hFile, &file_size))
        {
            ERR("couldn't get file size (error is %d)\n", GetLastError());
            CloseHandle(hFile);
            return FALSE;
        }

        CloseHandle(hFile);
    }

    error = URLCacheContainers_FindContainerW(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    lpszUrlNameA = heap_strdupWtoA(lpszUrlName);
    if (!lpszUrlNameA)
    {
        error = GetLastError();
        goto cleanup;
    }

    if (lpszFileExtension && !(lpszFileExtensionA = heap_strdupWtoA(lpszFileExtension)))
    {
        error = GetLastError();
        goto cleanup;
    }

    if (URLCache_FindHash(pHeader, lpszUrlNameA, &pHashEntry))
    {
        FIXME("entry already in cache - don't know what to do!\n");
/*
 *        SetLastError(ERROR_FILE_NOT_FOUND);
 *        return FALSE;
 */
        goto cleanup;
    }

    if (lpszLocalFileName)
    {
        BOOL bFound = FALSE;

        if (strncmpW(lpszLocalFileName, pContainer->path, lstrlenW(pContainer->path)))
        {
            ERR("path %s must begin with cache content path %s\n", debugstr_w(lpszLocalFileName), debugstr_w(pContainer->path));
            error = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

        /* skip container path prefix */
        lpszLocalFileName += lstrlenW(pContainer->path);

        WideCharToMultiByte(CP_ACP, 0, lpszLocalFileName, -1, achFile, MAX_PATH, NULL, NULL);
	pchLocalFileName = achFile;

        for (cDirectory = 0; cDirectory < pHeader->DirectoryCount; cDirectory++)
        {
            if (!strncmp(pHeader->directory_data[cDirectory].filename, pchLocalFileName, DIR_LENGTH))
            {
                bFound = TRUE;
                break;
            }
        }

        if (!bFound)
        {
            ERR("cache directory not found in path %s\n", debugstr_w(lpszLocalFileName));
            error = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

        lpszLocalFileName += (DIR_LENGTH + 1); /* "1234WXYZ\" */
    }

    dwBytesNeeded = DWORD_ALIGN(dwBytesNeeded + strlen(lpszUrlNameA) + 1);
    if (lpszLocalFileName)
    {
        dwOffsetLocalFileName = dwBytesNeeded;
        dwBytesNeeded = DWORD_ALIGN(dwBytesNeeded + strlen(pchLocalFileName) + 1);
    }
    if (lpHeaderInfo)
    {
        dwOffsetHeader = dwBytesNeeded;
        dwBytesNeeded = DWORD_ALIGN(dwBytesNeeded + dwHeaderSize);
    }
    if (lpszFileExtensionA)
    {
        dwOffsetFileExtension = dwBytesNeeded;
        dwBytesNeeded = DWORD_ALIGN(dwBytesNeeded + strlen(lpszFileExtensionA) + 1);
    }

    /* round up to next block */
    if (dwBytesNeeded % BLOCKSIZE)
    {
        dwBytesNeeded -= dwBytesNeeded % BLOCKSIZE;
        dwBytesNeeded += BLOCKSIZE;
    }

    if (!URLCache_FindFirstFreeEntry(pHeader, dwBytesNeeded / BLOCKSIZE, &pEntry))
    {
        ERR("no free entries\n");
        error = ERROR_DISK_FULL;
        goto cleanup;
    }

    /* FindFirstFreeEntry fills in blocks used */
    pUrlEntry = (URL_CACHEFILE_ENTRY *)pEntry;
    pUrlEntry->CacheFileEntry.dwSignature = URL_SIGNATURE;
    pUrlEntry->CacheDir = cDirectory;
    pUrlEntry->CacheEntryType = CacheEntryType;
    pUrlEntry->dwHeaderInfoSize = dwHeaderSize;
    if (CacheEntryType & STICKY_CACHE_ENTRY)
    {
        /* Sticky entries have a default exempt time of one day */
        pUrlEntry->dwExemptDelta = 86400;
    }
    else
        pUrlEntry->dwExemptDelta = 0;
    pUrlEntry->dwHitRate = 0;
    pUrlEntry->dwOffsetFileExtension = dwOffsetFileExtension;
    pUrlEntry->dwOffsetHeaderInfo = dwOffsetHeader;
    pUrlEntry->dwOffsetLocalName = dwOffsetLocalFileName;
    pUrlEntry->dwOffsetUrl = DWORD_ALIGN(sizeof(*pUrlEntry));
    pUrlEntry->size.QuadPart = file_size.QuadPart;
    pUrlEntry->dwUseCount = 0;
    GetSystemTimeAsFileTime(&pUrlEntry->LastAccessTime);
    pUrlEntry->LastModifiedTime = LastModifiedTime;
    URLCache_FileTimeToDosDateTime(&pUrlEntry->LastAccessTime, &pUrlEntry->wLastSyncDate, &pUrlEntry->wLastSyncTime);
    URLCache_FileTimeToDosDateTime(&ExpireTime, &pUrlEntry->wExpiredDate, &pUrlEntry->wExpiredTime);
    pUrlEntry->wUnknownDate = pUrlEntry->wLastSyncDate;
    pUrlEntry->wUnknownTime = pUrlEntry->wLastSyncTime;

    /*** Unknowns ***/
    pUrlEntry->dwUnknown1 = 0;
    pUrlEntry->dwUnknown2 = 0;
    pUrlEntry->dwUnknown3 = 0x60;
    pUrlEntry->Unknown4 = 0;
    pUrlEntry->wUnknown5 = 0x1010;
    pUrlEntry->dwUnknown7 = 0;
    pUrlEntry->dwUnknown8 = 0;


    strcpy((LPSTR)pUrlEntry + pUrlEntry->dwOffsetUrl, lpszUrlNameA);
    if (dwOffsetLocalFileName)
        strcpy((LPSTR)((LPBYTE)pUrlEntry + dwOffsetLocalFileName), pchLocalFileName + DIR_LENGTH + 1);
    if (dwOffsetHeader)
        memcpy((LPBYTE)pUrlEntry + dwOffsetHeader, lpHeaderInfo, dwHeaderSize);
    if (dwOffsetFileExtension)
        strcpy((LPSTR)((LPBYTE)pUrlEntry + dwOffsetFileExtension), lpszFileExtensionA);

    error = URLCache_AddEntryToHash(pHeader, lpszUrlNameA,
        (DWORD)((LPBYTE)pUrlEntry - (LPBYTE)pHeader));
    if (error != ERROR_SUCCESS)
        URLCache_DeleteEntry(pHeader, &pUrlEntry->CacheFileEntry);
    else
    {
        if (pUrlEntry->CacheDir < pHeader->DirectoryCount)
            pHeader->directory_data[pUrlEntry->CacheDir].dwNumFiles++;
        if (CacheEntryType & STICKY_CACHE_ENTRY)
            pHeader->ExemptUsage.QuadPart += file_size.QuadPart;
        else
            pHeader->CacheUsage.QuadPart += file_size.QuadPart;
        if (pHeader->CacheUsage.QuadPart + pHeader->ExemptUsage.QuadPart >
            pHeader->CacheLimit.QuadPart)
            FIXME("file of size %s bytes fills cache\n", wine_dbgstr_longlong(file_size.QuadPart));
    }

cleanup:
    URLCacheContainer_UnlockIndex(pContainer, pHeader);
    HeapFree(GetProcessHeap(), 0, lpszUrlNameA);
    HeapFree(GetProcessHeap(), 0, lpszFileExtensionA);

    if (error == ERROR_SUCCESS)
        return TRUE;
    else
    {
        SetLastError(error);
        return FALSE;
    }
}

/***********************************************************************
 *           CommitUrlCacheEntryA (WININET.@)
 *
 */
BOOL WINAPI CommitUrlCacheEntryA(
    IN LPCSTR lpszUrlName,
    IN LPCSTR lpszLocalFileName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN LPBYTE lpHeaderInfo,
    IN DWORD dwHeaderSize,
    IN LPCSTR lpszFileExtension,
    IN LPCSTR lpszOriginalUrl
    )
{
    WCHAR *url_name = NULL;
    WCHAR *local_file_name = NULL;
    WCHAR *original_url = NULL;
    WCHAR *file_extension = NULL;
    BOOL bSuccess = FALSE;

    TRACE("(%s, %s, ..., ..., %x, %p, %d, %s, %s)\n",
        debugstr_a(lpszUrlName),
        debugstr_a(lpszLocalFileName),
        CacheEntryType,
        lpHeaderInfo,
        dwHeaderSize,
        debugstr_a(lpszFileExtension),
        debugstr_a(lpszOriginalUrl));

    url_name = heap_strdupAtoW(lpszUrlName);
    if (!url_name)
        goto cleanup;

    if (lpszLocalFileName)
    {
        local_file_name = heap_strdupAtoW(lpszLocalFileName);
        if (!local_file_name)
            goto cleanup;
    }
    if (lpszFileExtension)
    {
        file_extension = heap_strdupAtoW(lpszFileExtension);
        if (!file_extension)
            goto cleanup;
    }
    if (lpszOriginalUrl)
    {
        original_url = heap_strdupAtoW(lpszOriginalUrl);
        if (!original_url)
            goto cleanup;
    }

    bSuccess = CommitUrlCacheEntryInternal(url_name, local_file_name, ExpireTime, LastModifiedTime,
                                           CacheEntryType, lpHeaderInfo, dwHeaderSize,
                                           file_extension, original_url);

cleanup:
    HeapFree(GetProcessHeap(), 0, original_url);
    HeapFree(GetProcessHeap(), 0, file_extension);
    HeapFree(GetProcessHeap(), 0, local_file_name);
    HeapFree(GetProcessHeap(), 0, url_name);

    return bSuccess;
}

/***********************************************************************
 *           CommitUrlCacheEntryW (WININET.@)
 *
 */
BOOL WINAPI CommitUrlCacheEntryW(
    IN LPCWSTR lpszUrlName,
    IN LPCWSTR lpszLocalFileName,
    IN FILETIME ExpireTime,
    IN FILETIME LastModifiedTime,
    IN DWORD CacheEntryType,
    IN LPWSTR lpHeaderInfo,
    IN DWORD dwHeaderSize,
    IN LPCWSTR lpszFileExtension,
    IN LPCWSTR lpszOriginalUrl
    )
{
    DWORD dwError = 0;
    BOOL bSuccess = FALSE;
    DWORD len = 0;
    CHAR *header_info = NULL;

    TRACE("(%s, %s, ..., ..., %x, %p, %d, %s, %s)\n",
        debugstr_w(lpszUrlName),
        debugstr_w(lpszLocalFileName),
        CacheEntryType,
        lpHeaderInfo,
        dwHeaderSize,
        debugstr_w(lpszFileExtension),
        debugstr_w(lpszOriginalUrl));

    if (!lpHeaderInfo || (header_info = heap_strdupWtoA(lpHeaderInfo)))
    {
	if (CommitUrlCacheEntryInternal(lpszUrlName, lpszLocalFileName, ExpireTime, LastModifiedTime,
				CacheEntryType, (LPBYTE)header_info, len, lpszFileExtension, lpszOriginalUrl))
	{
		bSuccess = TRUE;
	}
	else
	{
		dwError = GetLastError();
	}
	if (header_info)
	{
	    HeapFree(GetProcessHeap(), 0, header_info);
	    if (!bSuccess)
		SetLastError(dwError);
	}
    }
    return bSuccess;
}

/***********************************************************************
 *           ReadUrlCacheEntryStream (WININET.@)
 *
 */
BOOL WINAPI ReadUrlCacheEntryStream(
    IN HANDLE hUrlCacheStream,
    IN  DWORD dwLocation,
    IN OUT LPVOID lpBuffer,
    IN OUT LPDWORD lpdwLen,
    IN DWORD dwReserved
    )
{
    /* Get handle to file from 'stream' */
    STREAM_HANDLE * pStream = (STREAM_HANDLE *)hUrlCacheStream;

    if (dwReserved != 0)
    {
        ERR("dwReserved != 0\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (IsBadReadPtr(pStream, sizeof(*pStream)) || IsBadStringPtrA(pStream->lpszUrl, INTERNET_MAX_URL_LENGTH))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (SetFilePointer(pStream->hFile, dwLocation, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
        return FALSE;
    return ReadFile(pStream->hFile, lpBuffer, *lpdwLen, lpdwLen, NULL);
}

/***********************************************************************
 *           RetrieveUrlCacheEntryStreamA (WININET.@)
 *
 */
HANDLE WINAPI RetrieveUrlCacheEntryStreamA(
    IN LPCSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFOA lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize,
    IN BOOL fRandomRead,
    IN DWORD dwReserved
    )
{
    /* NOTE: this is not the same as the way that the native
     * version allocates 'stream' handles. I did it this way
     * as it is much easier and no applications should depend
     * on this behaviour. (Native version appears to allocate
     * indices into a table)
     */
    STREAM_HANDLE * pStream;
    HANDLE hFile;

    TRACE( "(%s, %p, %p, %x, 0x%08x)\n", debugstr_a(lpszUrlName), lpCacheEntryInfo,
           lpdwCacheEntryInfoBufferSize, fRandomRead, dwReserved );

    if (!RetrieveUrlCacheEntryFileA(lpszUrlName,
        lpCacheEntryInfo,
        lpdwCacheEntryInfoBufferSize,
        dwReserved))
    {
        return NULL;
    }

    hFile = CreateFileA(lpCacheEntryInfo->lpszLocalFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        fRandomRead ? FILE_FLAG_RANDOM_ACCESS : 0,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;
    
    /* allocate handle storage space */
    pStream = heap_alloc(sizeof(STREAM_HANDLE) + strlen(lpszUrlName) * sizeof(CHAR));
    if (!pStream)
    {
        CloseHandle(hFile);
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    pStream->hFile = hFile;
    strcpy(pStream->lpszUrl, lpszUrlName);
    return pStream;
}

/***********************************************************************
 *           RetrieveUrlCacheEntryStreamW (WININET.@)
 *
 */
HANDLE WINAPI RetrieveUrlCacheEntryStreamW(
    IN LPCWSTR lpszUrlName,
    OUT LPINTERNET_CACHE_ENTRY_INFOW lpCacheEntryInfo,
    IN OUT LPDWORD lpdwCacheEntryInfoBufferSize,
    IN BOOL fRandomRead,
    IN DWORD dwReserved
    )
{
    DWORD size;
    int url_len;
    /* NOTE: this is not the same as the way that the native
     * version allocates 'stream' handles. I did it this way
     * as it is much easier and no applications should depend
     * on this behaviour. (Native version appears to allocate
     * indices into a table)
     */
    STREAM_HANDLE * pStream;
    HANDLE hFile;

    TRACE( "(%s, %p, %p, %x, 0x%08x)\n", debugstr_w(lpszUrlName), lpCacheEntryInfo,
           lpdwCacheEntryInfoBufferSize, fRandomRead, dwReserved );

    if (!RetrieveUrlCacheEntryFileW(lpszUrlName,
        lpCacheEntryInfo,
        lpdwCacheEntryInfoBufferSize,
        dwReserved))
    {
        return NULL;
    }

    hFile = CreateFileW(lpCacheEntryInfo->lpszLocalFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        fRandomRead ? FILE_FLAG_RANDOM_ACCESS : 0,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    /* allocate handle storage space */
    size = sizeof(STREAM_HANDLE);
    url_len = WideCharToMultiByte(CP_ACP, 0, lpszUrlName, -1, NULL, 0, NULL, NULL);
    size += url_len;
    pStream = heap_alloc(size);
    if (!pStream)
    {
        CloseHandle(hFile);
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    pStream->hFile = hFile;
    WideCharToMultiByte(CP_ACP, 0, lpszUrlName, -1, pStream->lpszUrl, url_len, NULL, NULL);
    return pStream;
}

/***********************************************************************
 *           UnlockUrlCacheEntryStream (WININET.@)
 *
 */
BOOL WINAPI UnlockUrlCacheEntryStream(
    IN HANDLE hUrlCacheStream,
    IN DWORD dwReserved
)
{
    STREAM_HANDLE * pStream = (STREAM_HANDLE *)hUrlCacheStream;

    if (dwReserved != 0)
    {
        ERR("dwReserved != 0\n");
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (IsBadReadPtr(pStream, sizeof(*pStream)) || IsBadStringPtrA(pStream->lpszUrl, INTERNET_MAX_URL_LENGTH))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    if (!UnlockUrlCacheEntryFileA(pStream->lpszUrl, 0))
        return FALSE;

    /* close file handle */
    CloseHandle(pStream->hFile);

    /* free allocated space */
    HeapFree(GetProcessHeap(), 0, pStream);

    return TRUE;
}


/***********************************************************************
 *           DeleteUrlCacheEntryA (WININET.@)
 *
 */
BOOL WINAPI DeleteUrlCacheEntryA(LPCSTR lpszUrlName)
{
    URLCACHECONTAINER * pContainer;
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    DWORD error;
    BOOL ret;

    TRACE("(%s)\n", debugstr_a(lpszUrlName));

    error = URLCacheContainers_FindContainerA(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
        return FALSE;

    if (!URLCache_FindHash(pHeader, lpszUrlName, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        TRACE("entry %s not found!\n", lpszUrlName);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    ret = DeleteUrlCacheEntryInternal(pHeader, pHashEntry);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return ret;
}

/***********************************************************************
 *           DeleteUrlCacheEntryW (WININET.@)
 *
 */
BOOL WINAPI DeleteUrlCacheEntryW(LPCWSTR lpszUrlName)
{
    URLCACHECONTAINER * pContainer;
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    LPSTR urlA;
    DWORD error;
    BOOL ret;

    TRACE("(%s)\n", debugstr_w(lpszUrlName));

    urlA = heap_strdupWtoA(lpszUrlName);
    if (!urlA)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    error = URLCacheContainers_FindContainerW(lpszUrlName, &pContainer);
    if (error != ERROR_SUCCESS)
    {
        HeapFree(GetProcessHeap(), 0, urlA);
        SetLastError(error);
        return FALSE;
    }

    error = URLCacheContainer_OpenIndex(pContainer);
    if (error != ERROR_SUCCESS)
    {
        HeapFree(GetProcessHeap(), 0, urlA);
        SetLastError(error);
        return FALSE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
    {
        HeapFree(GetProcessHeap(), 0, urlA);
        return FALSE;
    }

    if (!URLCache_FindHash(pHeader, urlA, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        TRACE("entry %s not found!\n", debugstr_a(urlA));
        HeapFree(GetProcessHeap(), 0, urlA);
        SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

    ret = DeleteUrlCacheEntryInternal(pHeader, pHashEntry);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    HeapFree(GetProcessHeap(), 0, urlA);
    return ret;
}

BOOL WINAPI DeleteUrlCacheContainerA(DWORD d1, DWORD d2)
{
    FIXME("(0x%08x, 0x%08x) stub\n", d1, d2);
    return TRUE;
}

BOOL WINAPI DeleteUrlCacheContainerW(DWORD d1, DWORD d2)
{
    FIXME("(0x%08x, 0x%08x) stub\n", d1, d2);
    return TRUE;
}

/***********************************************************************
 *           CreateCacheContainerA (WININET.@)
 */
BOOL WINAPI CreateUrlCacheContainerA(DWORD d1, DWORD d2, DWORD d3, DWORD d4,
                                     DWORD d5, DWORD d6, DWORD d7, DWORD d8)
{
    FIXME("(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x) stub\n",
          d1, d2, d3, d4, d5, d6, d7, d8);
    return TRUE;
}

/***********************************************************************
 *           CreateCacheContainerW (WININET.@)
 */
BOOL WINAPI CreateUrlCacheContainerW(DWORD d1, DWORD d2, DWORD d3, DWORD d4,
                                     DWORD d5, DWORD d6, DWORD d7, DWORD d8)
{
    FIXME("(0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x) stub\n",
          d1, d2, d3, d4, d5, d6, d7, d8);
    return TRUE;
}

/***********************************************************************
 *           FindFirstUrlCacheContainerA (WININET.@)
 */
HANDLE WINAPI FindFirstUrlCacheContainerA( LPVOID p1, LPVOID p2, LPVOID p3, DWORD d1 )
{
    FIXME("(%p, %p, %p, 0x%08x) stub\n", p1, p2, p3, d1 );
    return NULL;
}

/***********************************************************************
 *           FindFirstUrlCacheContainerW (WININET.@)
 */
HANDLE WINAPI FindFirstUrlCacheContainerW( LPVOID p1, LPVOID p2, LPVOID p3, DWORD d1 )
{
    FIXME("(%p, %p, %p, 0x%08x) stub\n", p1, p2, p3, d1 );
    return NULL;
}

/***********************************************************************
 *           FindNextUrlCacheContainerA (WININET.@)
 */
BOOL WINAPI FindNextUrlCacheContainerA( HANDLE handle, LPVOID p1, LPVOID p2 )
{
    FIXME("(%p, %p, %p) stub\n", handle, p1, p2 );
    return FALSE;
}

/***********************************************************************
 *           FindNextUrlCacheContainerW (WININET.@)
 */
BOOL WINAPI FindNextUrlCacheContainerW( HANDLE handle, LPVOID p1, LPVOID p2 )
{
    FIXME("(%p, %p, %p) stub\n", handle, p1, p2 );
    return FALSE;
}

HANDLE WINAPI FindFirstUrlCacheEntryExA(
  LPCSTR lpszUrlSearchPattern,
  DWORD dwFlags,
  DWORD dwFilter,
  GROUPID GroupId,
  LPINTERNET_CACHE_ENTRY_INFOA lpFirstCacheEntryInfo,
  LPDWORD lpdwFirstCacheEntryInfoBufferSize,
  LPVOID lpReserved,
  LPDWORD pcbReserved2,
  LPVOID lpReserved3
)
{
    FIXME("(%s, 0x%08x, 0x%08x, 0x%08x%08x, %p, %p, %p, %p, %p) stub\n", debugstr_a(lpszUrlSearchPattern),
          dwFlags, dwFilter, (ULONG)(GroupId >> 32), (ULONG)GroupId, lpFirstCacheEntryInfo,
          lpdwFirstCacheEntryInfoBufferSize, lpReserved, pcbReserved2,lpReserved3);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return NULL;
}

HANDLE WINAPI FindFirstUrlCacheEntryExW(
  LPCWSTR lpszUrlSearchPattern,
  DWORD dwFlags,
  DWORD dwFilter,
  GROUPID GroupId,
  LPINTERNET_CACHE_ENTRY_INFOW lpFirstCacheEntryInfo,
  LPDWORD lpdwFirstCacheEntryInfoBufferSize,
  LPVOID lpReserved,
  LPDWORD pcbReserved2,
  LPVOID lpReserved3
)
{
    FIXME("(%s, 0x%08x, 0x%08x, 0x%08x%08x, %p, %p, %p, %p, %p) stub\n", debugstr_w(lpszUrlSearchPattern),
          dwFlags, dwFilter, (ULONG)(GroupId >> 32), (ULONG)GroupId, lpFirstCacheEntryInfo,
          lpdwFirstCacheEntryInfoBufferSize, lpReserved, pcbReserved2,lpReserved3);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return NULL;
}

#define URLCACHE_FIND_ENTRY_HANDLE_MAGIC 0xF389ABCD

typedef struct URLCacheFindEntryHandle
{
    DWORD dwMagic;
    LPWSTR lpszUrlSearchPattern;
    DWORD dwContainerIndex;
    DWORD dwHashTableIndex;
    DWORD dwHashEntryIndex;
} URLCacheFindEntryHandle;

/***********************************************************************
 *           FindFirstUrlCacheEntryA (WININET.@)
 *
 */
INTERNETAPI HANDLE WINAPI FindFirstUrlCacheEntryA(LPCSTR lpszUrlSearchPattern,
 LPINTERNET_CACHE_ENTRY_INFOA lpFirstCacheEntryInfo, LPDWORD lpdwFirstCacheEntryInfoBufferSize)
{
    URLCacheFindEntryHandle *pEntryHandle;

    TRACE("(%s, %p, %p)\n", debugstr_a(lpszUrlSearchPattern), lpFirstCacheEntryInfo, lpdwFirstCacheEntryInfoBufferSize);

    pEntryHandle = heap_alloc(sizeof(*pEntryHandle));
    if (!pEntryHandle)
        return NULL;

    pEntryHandle->dwMagic = URLCACHE_FIND_ENTRY_HANDLE_MAGIC;
    if (lpszUrlSearchPattern)
    {
        pEntryHandle->lpszUrlSearchPattern = heap_strdupAtoW(lpszUrlSearchPattern);
        if (!pEntryHandle->lpszUrlSearchPattern)
        {
            HeapFree(GetProcessHeap(), 0, pEntryHandle);
            return NULL;
        }
    }
    else
        pEntryHandle->lpszUrlSearchPattern = NULL;
    pEntryHandle->dwContainerIndex = 0;
    pEntryHandle->dwHashTableIndex = 0;
    pEntryHandle->dwHashEntryIndex = 0;

    if (!FindNextUrlCacheEntryA(pEntryHandle, lpFirstCacheEntryInfo, lpdwFirstCacheEntryInfoBufferSize))
    {
        HeapFree(GetProcessHeap(), 0, pEntryHandle);
        return NULL;
    }
    return pEntryHandle;
}

/***********************************************************************
 *           FindFirstUrlCacheEntryW (WININET.@)
 *
 */
INTERNETAPI HANDLE WINAPI FindFirstUrlCacheEntryW(LPCWSTR lpszUrlSearchPattern,
 LPINTERNET_CACHE_ENTRY_INFOW lpFirstCacheEntryInfo, LPDWORD lpdwFirstCacheEntryInfoBufferSize)
{
    URLCacheFindEntryHandle *pEntryHandle;

    TRACE("(%s, %p, %p)\n", debugstr_w(lpszUrlSearchPattern), lpFirstCacheEntryInfo, lpdwFirstCacheEntryInfoBufferSize);

    pEntryHandle = heap_alloc(sizeof(*pEntryHandle));
    if (!pEntryHandle)
        return NULL;

    pEntryHandle->dwMagic = URLCACHE_FIND_ENTRY_HANDLE_MAGIC;
    if (lpszUrlSearchPattern)
    {
        pEntryHandle->lpszUrlSearchPattern = heap_strdupW(lpszUrlSearchPattern);
        if (!pEntryHandle->lpszUrlSearchPattern)
        {
            HeapFree(GetProcessHeap(), 0, pEntryHandle);
            return NULL;
        }
    }
    else
        pEntryHandle->lpszUrlSearchPattern = NULL;
    pEntryHandle->dwContainerIndex = 0;
    pEntryHandle->dwHashTableIndex = 0;
    pEntryHandle->dwHashEntryIndex = 0;

    if (!FindNextUrlCacheEntryW(pEntryHandle, lpFirstCacheEntryInfo, lpdwFirstCacheEntryInfoBufferSize))
    {
        HeapFree(GetProcessHeap(), 0, pEntryHandle);
        return NULL;
    }
    return pEntryHandle;
}

static BOOL FindNextUrlCacheEntryInternal(
  HANDLE hEnumHandle,
  LPINTERNET_CACHE_ENTRY_INFOA lpNextCacheEntryInfo,
  LPDWORD lpdwNextCacheEntryInfoBufferSize,
  BOOL unicode)
{
    URLCacheFindEntryHandle *pEntryHandle = (URLCacheFindEntryHandle *)hEnumHandle;
    URLCACHECONTAINER * pContainer;

    if (pEntryHandle->dwMagic != URLCACHE_FIND_ENTRY_HANDLE_MAGIC)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    for (; URLCacheContainers_Enum(pEntryHandle->lpszUrlSearchPattern, pEntryHandle->dwContainerIndex, &pContainer);
         pEntryHandle->dwContainerIndex++, pEntryHandle->dwHashTableIndex = 0)
    {
        LPURLCACHE_HEADER pHeader;
        HASH_CACHEFILE_ENTRY *pHashTableEntry;
        DWORD error;

        error = URLCacheContainer_OpenIndex(pContainer);
        if (error != ERROR_SUCCESS)
        {
            SetLastError(error);
            return FALSE;
        }

        if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
            return FALSE;

        for (; URLCache_EnumHashTables(pHeader, &pEntryHandle->dwHashTableIndex, &pHashTableEntry);
             pEntryHandle->dwHashTableIndex++, pEntryHandle->dwHashEntryIndex = 0)
        {
            const struct _HASH_ENTRY *pHashEntry = NULL;
            for (; URLCache_EnumHashTableEntries(pHeader, pHashTableEntry, &pEntryHandle->dwHashEntryIndex, &pHashEntry);
                 pEntryHandle->dwHashEntryIndex++)
            {
                const URL_CACHEFILE_ENTRY *pUrlEntry;
                const CACHEFILE_ENTRY *pEntry = (const CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);

                if (pEntry->dwSignature != URL_SIGNATURE)
                    continue;

                pUrlEntry = (const URL_CACHEFILE_ENTRY *)pEntry;
                TRACE("Found URL: %s\n",
                      debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetUrl));
                TRACE("Header info: %s\n",
                      debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetHeaderInfo));

                error = URLCache_CopyEntry(
                    pContainer,
                    pHeader,
                    lpNextCacheEntryInfo,
                    lpdwNextCacheEntryInfoBufferSize,
                    pUrlEntry,
                    unicode);
                if (error != ERROR_SUCCESS)
                {
                    URLCacheContainer_UnlockIndex(pContainer, pHeader);
                    SetLastError(error);
                    return FALSE;
                }
                TRACE("Local File Name: %s\n",
                      debugstr_a((LPCSTR)pUrlEntry + pUrlEntry->dwOffsetLocalName));

                /* increment the current index so that next time the function
                 * is called the next entry is returned */
                pEntryHandle->dwHashEntryIndex++;
                URLCacheContainer_UnlockIndex(pContainer, pHeader);
                return TRUE;
            }
        }

        URLCacheContainer_UnlockIndex(pContainer, pHeader);
    }

    SetLastError(ERROR_NO_MORE_ITEMS);
    return FALSE;
}

/***********************************************************************
 *           FindNextUrlCacheEntryA (WININET.@)
 */
BOOL WINAPI FindNextUrlCacheEntryA(
  HANDLE hEnumHandle,
  LPINTERNET_CACHE_ENTRY_INFOA lpNextCacheEntryInfo,
  LPDWORD lpdwNextCacheEntryInfoBufferSize)
{
    TRACE("(%p, %p, %p)\n", hEnumHandle, lpNextCacheEntryInfo, lpdwNextCacheEntryInfoBufferSize);

    return FindNextUrlCacheEntryInternal(hEnumHandle, lpNextCacheEntryInfo,
            lpdwNextCacheEntryInfoBufferSize, FALSE /* not UNICODE */);
}

/***********************************************************************
 *           FindNextUrlCacheEntryW (WININET.@)
 */
BOOL WINAPI FindNextUrlCacheEntryW(
  HANDLE hEnumHandle,
  LPINTERNET_CACHE_ENTRY_INFOW lpNextCacheEntryInfo,
  LPDWORD lpdwNextCacheEntryInfoBufferSize
)
{
    TRACE("(%p, %p, %p)\n", hEnumHandle, lpNextCacheEntryInfo, lpdwNextCacheEntryInfoBufferSize);

    return FindNextUrlCacheEntryInternal(hEnumHandle,
            (LPINTERNET_CACHE_ENTRY_INFOA)lpNextCacheEntryInfo,
            lpdwNextCacheEntryInfoBufferSize, TRUE /* UNICODE */);
}

/***********************************************************************
 *           FindCloseUrlCache (WININET.@)
 */
BOOL WINAPI FindCloseUrlCache(HANDLE hEnumHandle)
{
    URLCacheFindEntryHandle *pEntryHandle = (URLCacheFindEntryHandle *)hEnumHandle;

    TRACE("(%p)\n", hEnumHandle);

    if (!pEntryHandle || pEntryHandle->dwMagic != URLCACHE_FIND_ENTRY_HANDLE_MAGIC)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    pEntryHandle->dwMagic = 0;
    HeapFree(GetProcessHeap(), 0, pEntryHandle->lpszUrlSearchPattern);
    HeapFree(GetProcessHeap(), 0, pEntryHandle);

    return TRUE;
}

HANDLE WINAPI FindFirstUrlCacheGroup( DWORD dwFlags, DWORD dwFilter, LPVOID lpSearchCondition,
                                      DWORD dwSearchCondition, GROUPID* lpGroupId, LPVOID lpReserved )
{
    FIXME("(0x%08x, 0x%08x, %p, 0x%08x, %p, %p) stub\n", dwFlags, dwFilter, lpSearchCondition,
          dwSearchCondition, lpGroupId, lpReserved);
    return NULL;
}

BOOL WINAPI FindNextUrlCacheEntryExA(
  HANDLE hEnumHandle,
  LPINTERNET_CACHE_ENTRY_INFOA lpFirstCacheEntryInfo,
  LPDWORD lpdwFirstCacheEntryInfoBufferSize,
  LPVOID lpReserved,
  LPDWORD pcbReserved2,
  LPVOID lpReserved3
)
{
    FIXME("(%p, %p, %p, %p, %p, %p) stub\n", hEnumHandle, lpFirstCacheEntryInfo, lpdwFirstCacheEntryInfoBufferSize,
          lpReserved, pcbReserved2, lpReserved3);
    return FALSE;
}

BOOL WINAPI FindNextUrlCacheEntryExW(
  HANDLE hEnumHandle,
  LPINTERNET_CACHE_ENTRY_INFOW lpFirstCacheEntryInfo,
  LPDWORD lpdwFirstCacheEntryInfoBufferSize,
  LPVOID lpReserved,
  LPDWORD pcbReserved2,
  LPVOID lpReserved3
)
{
    FIXME("(%p, %p, %p, %p, %p, %p) stub\n", hEnumHandle, lpFirstCacheEntryInfo, lpdwFirstCacheEntryInfoBufferSize,
          lpReserved, pcbReserved2, lpReserved3);
    return FALSE;
}

BOOL WINAPI FindNextUrlCacheGroup( HANDLE hFind, GROUPID* lpGroupId, LPVOID lpReserved )
{
    FIXME("(%p, %p, %p) stub\n", hFind, lpGroupId, lpReserved);
    return FALSE;
}

/***********************************************************************
 *           CreateUrlCacheGroup (WININET.@)
 *
 */
INTERNETAPI GROUPID WINAPI CreateUrlCacheGroup(DWORD dwFlags, LPVOID lpReserved)
{
  FIXME("(0x%08x, %p): stub\n", dwFlags, lpReserved);
  return FALSE;
}

/***********************************************************************
 *           DeleteUrlCacheGroup (WININET.@)
 *
 */
BOOL WINAPI DeleteUrlCacheGroup(GROUPID GroupId, DWORD dwFlags, LPVOID lpReserved)
{
    FIXME("(0x%08x%08x, 0x%08x, %p) stub\n",
          (ULONG)(GroupId >> 32), (ULONG)GroupId, dwFlags, lpReserved);
    return FALSE;
}

/***********************************************************************
 *           SetUrlCacheEntryGroupA (WININET.@)
 *
 */
BOOL WINAPI SetUrlCacheEntryGroupA(LPCSTR lpszUrlName, DWORD dwFlags,
  GROUPID GroupId, LPBYTE pbGroupAttributes, DWORD cbGroupAttributes,
  LPVOID lpReserved)
{
    FIXME("(%s, 0x%08x, 0x%08x%08x, %p, 0x%08x, %p) stub\n",
          debugstr_a(lpszUrlName), dwFlags, (ULONG)(GroupId >> 32), (ULONG)GroupId,
          pbGroupAttributes, cbGroupAttributes, lpReserved);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return FALSE;
}

/***********************************************************************
 *           SetUrlCacheEntryGroupW (WININET.@)
 *
 */
BOOL WINAPI SetUrlCacheEntryGroupW(LPCWSTR lpszUrlName, DWORD dwFlags,
  GROUPID GroupId, LPBYTE pbGroupAttributes, DWORD cbGroupAttributes,
  LPVOID lpReserved)
{
    FIXME("(%s, 0x%08x, 0x%08x%08x, %p, 0x%08x, %p) stub\n",
          debugstr_w(lpszUrlName), dwFlags, (ULONG)(GroupId >> 32), (ULONG)GroupId,
          pbGroupAttributes, cbGroupAttributes, lpReserved);
    SetLastError(ERROR_FILE_NOT_FOUND);
    return FALSE;
}

/***********************************************************************
 *           GetUrlCacheConfigInfoW (WININET.@)
 */
BOOL WINAPI GetUrlCacheConfigInfoW(LPINTERNET_CACHE_CONFIG_INFOW CacheInfo, LPDWORD size, DWORD bitmask)
{
    FIXME("(%p, %p, %x)\n", CacheInfo, size, bitmask);
    INTERNET_SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

/***********************************************************************
 *           GetUrlCacheConfigInfoA (WININET.@)
 */
BOOL WINAPI GetUrlCacheConfigInfoA(LPINTERNET_CACHE_CONFIG_INFOA CacheInfo, LPDWORD size, DWORD bitmask)
{
    FIXME("(%p, %p, %x)\n", CacheInfo, size, bitmask);
    INTERNET_SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL WINAPI GetUrlCacheGroupAttributeA( GROUPID gid, DWORD dwFlags, DWORD dwAttributes,
                                        LPINTERNET_CACHE_GROUP_INFOA lpGroupInfo,
                                        LPDWORD lpdwGroupInfo, LPVOID lpReserved )
{
    FIXME("(0x%08x%08x, 0x%08x, 0x%08x, %p, %p, %p) stub\n",
          (ULONG)(gid >> 32), (ULONG)gid, dwFlags, dwAttributes, lpGroupInfo,
          lpdwGroupInfo, lpReserved);
    return FALSE;
}

BOOL WINAPI GetUrlCacheGroupAttributeW( GROUPID gid, DWORD dwFlags, DWORD dwAttributes,
                                        LPINTERNET_CACHE_GROUP_INFOW lpGroupInfo,
                                        LPDWORD lpdwGroupInfo, LPVOID lpReserved )
{
    FIXME("(0x%08x%08x, 0x%08x, 0x%08x, %p, %p, %p) stub\n",
          (ULONG)(gid >> 32), (ULONG)gid, dwFlags, dwAttributes, lpGroupInfo,
          lpdwGroupInfo, lpReserved);
    return FALSE;
}

BOOL WINAPI SetUrlCacheGroupAttributeA( GROUPID gid, DWORD dwFlags, DWORD dwAttributes,
                                        LPINTERNET_CACHE_GROUP_INFOA lpGroupInfo, LPVOID lpReserved )
{
    FIXME("(0x%08x%08x, 0x%08x, 0x%08x, %p, %p) stub\n",
          (ULONG)(gid >> 32), (ULONG)gid, dwFlags, dwAttributes, lpGroupInfo, lpReserved);
    return TRUE;
}

BOOL WINAPI SetUrlCacheGroupAttributeW( GROUPID gid, DWORD dwFlags, DWORD dwAttributes,
                                        LPINTERNET_CACHE_GROUP_INFOW lpGroupInfo, LPVOID lpReserved )
{
    FIXME("(0x%08x%08x, 0x%08x, 0x%08x, %p, %p) stub\n",
          (ULONG)(gid >> 32), (ULONG)gid, dwFlags, dwAttributes, lpGroupInfo, lpReserved);
    return TRUE;
}

BOOL WINAPI SetUrlCacheConfigInfoA( LPINTERNET_CACHE_CONFIG_INFOA lpCacheConfigInfo, DWORD dwFieldControl )
{
    FIXME("(%p, 0x%08x) stub\n", lpCacheConfigInfo, dwFieldControl);
    return TRUE;
}

BOOL WINAPI SetUrlCacheConfigInfoW( LPINTERNET_CACHE_CONFIG_INFOW lpCacheConfigInfo, DWORD dwFieldControl )
{
    FIXME("(%p, 0x%08x) stub\n", lpCacheConfigInfo, dwFieldControl);
    return TRUE;
}

/***********************************************************************
 *           DeleteIE3Cache (WININET.@)
 *
 * Deletes the files used by the IE3 URL caching system.
 *
 * PARAMS
 *   hWnd        [I] A dummy window.
 *   hInst       [I] Instance of process calling the function.
 *   lpszCmdLine [I] Options used by function.
 *   nCmdShow    [I] The nCmdShow value to use when showing windows created, if any.
 */
DWORD WINAPI DeleteIE3Cache(HWND hWnd, HINSTANCE hInst, LPSTR lpszCmdLine, int nCmdShow)
{
    FIXME("(%p, %p, %s, %d)\n", hWnd, hInst, debugstr_a(lpszCmdLine), nCmdShow);
    return 0;
}

static BOOL IsUrlCacheEntryExpiredInternal(const URL_CACHEFILE_ENTRY *pUrlEntry,
        FILETIME *pftLastModified)
{
    BOOL ret;
    FILETIME now, expired;

    *pftLastModified = pUrlEntry->LastModifiedTime;
    GetSystemTimeAsFileTime(&now);
    URLCache_DosDateTimeToFileTime(pUrlEntry->wExpiredDate,
            pUrlEntry->wExpiredTime, &expired);
    /* If the expired time is 0, it's interpreted as not expired */
    if (!expired.dwLowDateTime && !expired.dwHighDateTime)
        ret = FALSE;
    else
        ret = CompareFileTime(&expired, &now) < 0;
    return ret;
}

/***********************************************************************
 *           IsUrlCacheEntryExpiredA (WININET.@)
 *
 * PARAMS
 *   url             [I] Url
 *   dwFlags         [I] Unknown
 *   pftLastModified [O] Last modified time
 */
BOOL WINAPI IsUrlCacheEntryExpiredA( LPCSTR url, DWORD dwFlags, FILETIME* pftLastModified )
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    const CACHEFILE_ENTRY * pEntry;
    const URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    BOOL expired;

    TRACE("(%s, %08x, %p)\n", debugstr_a(url), dwFlags, pftLastModified);

    if (!url || !pftLastModified)
        return TRUE;
    if (dwFlags)
        FIXME("unknown flags 0x%08x\n", dwFlags);

    /* Any error implies that the URL is expired, i.e. not in the cache */
    if (URLCacheContainers_FindContainerA(url, &pContainer))
    {
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        return TRUE;
    }

    if (URLCacheContainer_OpenIndex(pContainer))
    {
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        return TRUE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
    {
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        return TRUE;
    }

    if (!URLCache_FindHash(pHeader, url, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        TRACE("entry %s not found!\n", url);
        return TRUE;
    }

    pEntry = (const CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPCSTR)&pEntry->dwSignature, sizeof(DWORD)));
        return TRUE;
    }

    pUrlEntry = (const URL_CACHEFILE_ENTRY *)pEntry;
    expired = IsUrlCacheEntryExpiredInternal(pUrlEntry, pftLastModified);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return expired;
}

/***********************************************************************
 *           IsUrlCacheEntryExpiredW (WININET.@)
 *
 * PARAMS
 *   url             [I] Url
 *   dwFlags         [I] Unknown
 *   pftLastModified [O] Last modified time
 */
BOOL WINAPI IsUrlCacheEntryExpiredW( LPCWSTR url, DWORD dwFlags, FILETIME* pftLastModified )
{
    LPURLCACHE_HEADER pHeader;
    struct _HASH_ENTRY * pHashEntry;
    const CACHEFILE_ENTRY * pEntry;
    const URL_CACHEFILE_ENTRY * pUrlEntry;
    URLCACHECONTAINER * pContainer;
    BOOL expired;

    TRACE("(%s, %08x, %p)\n", debugstr_w(url), dwFlags, pftLastModified);

    if (!url || !pftLastModified)
        return TRUE;
    if (dwFlags)
        FIXME("unknown flags 0x%08x\n", dwFlags);

    /* Any error implies that the URL is expired, i.e. not in the cache */
    if (URLCacheContainers_FindContainerW(url, &pContainer))
    {
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        return TRUE;
    }

    if (URLCacheContainer_OpenIndex(pContainer))
    {
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        return TRUE;
    }

    if (!(pHeader = URLCacheContainer_LockIndex(pContainer)))
    {
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        return TRUE;
    }

    if (!URLCache_FindHashW(pHeader, url, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        TRACE("entry %s not found!\n", debugstr_w(url));
        return TRUE;
    }

    if (!URLCache_FindHashW(pHeader, url, &pHashEntry))
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        TRACE("entry %s not found!\n", debugstr_w(url));
        return TRUE;
    }

    pEntry = (const CACHEFILE_ENTRY *)((LPBYTE)pHeader + pHashEntry->dwOffsetEntry);
    if (pEntry->dwSignature != URL_SIGNATURE)
    {
        URLCacheContainer_UnlockIndex(pContainer, pHeader);
        memset(pftLastModified, 0, sizeof(*pftLastModified));
        FIXME("Trying to retrieve entry of unknown format %s\n", debugstr_an((LPCSTR)&pEntry->dwSignature, sizeof(DWORD)));
        return TRUE;
    }

    pUrlEntry = (const URL_CACHEFILE_ENTRY *)pEntry;
    expired = IsUrlCacheEntryExpiredInternal(pUrlEntry, pftLastModified);

    URLCacheContainer_UnlockIndex(pContainer, pHeader);

    return expired;
}

/***********************************************************************
 *           GetDiskInfoA (WININET.@)
 */
BOOL WINAPI GetDiskInfoA(PCSTR path, PDWORD cluster_size, PDWORDLONG free, PDWORDLONG total)
{
    BOOL ret;
    ULARGE_INTEGER bytes_free, bytes_total;

    TRACE("(%s, %p, %p, %p)\n", debugstr_a(path), cluster_size, free, total);

    if (!path)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((ret = GetDiskFreeSpaceExA(path, NULL, &bytes_total, &bytes_free)))
    {
        if (cluster_size) *cluster_size = 1;
        if (free) *free = bytes_free.QuadPart;
        if (total) *total = bytes_total.QuadPart;
    }
    return ret;
}

/***********************************************************************
 *           RegisterUrlCacheNotification (WININET.@)
 */
DWORD WINAPI RegisterUrlCacheNotification(LPVOID a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f)
{
    FIXME("(%p %x %x %x %x %x)\n", a, b, c, d, e, f);
    return 0;
}

/***********************************************************************
 *           IncrementUrlCacheHeaderData (WININET.@)
 */
BOOL WINAPI IncrementUrlCacheHeaderData(DWORD index, LPDWORD data)
{
    FIXME("(%u, %p)\n", index, data);
    return FALSE;
}
