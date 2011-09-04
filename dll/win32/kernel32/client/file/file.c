/* $Id: file.c 52819 2011-07-23 18:54:29Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/kernel32/file/file.c
 * PURPOSE:         Directory functions
 * PROGRAMMER:      Ariadne ( ariadne@xs4all.nl)
 *                  Pierre Schweitzer (pierre.schweitzer@odyssey.org)
 * UPDATE HISTORY:
 *                  Created 01/11/98
 */

/* INCLUDES *****************************************************************/

#include <k32.h>
#define NDEBUG
#include <debug.h>
DEBUG_CHANNEL(kernel32file);

/* GLOBALS ******************************************************************/

BOOL bIsFileApiAnsi = TRUE; // set the file api to ansi or oem

/* FUNCTIONS ****************************************************************/



PWCHAR
FilenameA2W(LPCSTR NameA, BOOL alloc)
{
   ANSI_STRING str;
   UNICODE_STRING strW;
   PUNICODE_STRING pstrW;
   NTSTATUS Status;

   //ASSERT(NtCurrentTeb()->StaticUnicodeString.Buffer == NtCurrentTeb()->StaticUnicodeBuffer);
   ASSERT(NtCurrentTeb()->StaticUnicodeString.MaximumLength == sizeof(NtCurrentTeb()->StaticUnicodeBuffer));

   RtlInitAnsiString(&str, NameA);
   pstrW = alloc ? &strW : &NtCurrentTeb()->StaticUnicodeString;

   if (bIsFileApiAnsi)
        Status= RtlAnsiStringToUnicodeString( pstrW, &str, (BOOLEAN)alloc );
   else
        Status= RtlOemStringToUnicodeString( pstrW, &str, (BOOLEAN)alloc );

    if (NT_SUCCESS(Status))
       return pstrW->Buffer;

    if (Status== STATUS_BUFFER_OVERFLOW)
        SetLastError( ERROR_FILENAME_EXCED_RANGE );
    else
        BaseSetLastNTError(Status);

    return NULL;
}


/*
No copy/conversion is done if the dest. buffer is too small.

Returns:
   Success: number of TCHARS copied into dest. buffer NOT including nullterm
   Fail: size of buffer in TCHARS required to hold the converted filename, including nullterm
*/
DWORD
FilenameU2A_FitOrFail(
   LPSTR  DestA,
   INT destLen, /* buffer size in TCHARS incl. nullchar */
   PUNICODE_STRING SourceU
   )
{
   DWORD ret;

   /* destLen should never exceed MAX_PATH */
   if (destLen > MAX_PATH) destLen = MAX_PATH;

   ret = bIsFileApiAnsi? RtlUnicodeStringToAnsiSize(SourceU) : RtlUnicodeStringToOemSize(SourceU);
   /* ret incl. nullchar */

   if (DestA && (INT)ret <= destLen)
   {
      ANSI_STRING str;

      str.Buffer = DestA;
      str.MaximumLength = (USHORT)destLen;


      if (bIsFileApiAnsi)
         RtlUnicodeStringToAnsiString(&str, SourceU, FALSE );
      else
         RtlUnicodeStringToOemString(&str, SourceU, FALSE );

      ret = str.Length;  /* SUCCESS: length without terminating 0 */
   }

   return ret;
}


/*
No copy/conversion is done if the dest. buffer is too small.

Returns:
   Success: number of TCHARS copied into dest. buffer NOT including nullterm
   Fail: size of buffer in TCHARS required to hold the converted filename, including nullterm
*/
DWORD
FilenameW2A_FitOrFail(
   LPSTR  DestA,
   INT destLen, /* buffer size in TCHARS incl. nullchar */
   LPCWSTR SourceW,
   INT sourceLen /* buffer size in TCHARS incl. nullchar */
   )
{
   UNICODE_STRING strW;

   if (sourceLen < 0) sourceLen = wcslen(SourceW) + 1;

   strW.Buffer = (PWCHAR)SourceW;
   strW.MaximumLength = sourceLen * sizeof(WCHAR);
   strW.Length = strW.MaximumLength - sizeof(WCHAR);

   return FilenameU2A_FitOrFail(DestA, destLen, &strW);
}


/*
Return: num. TCHARS copied into dest including nullterm
*/
DWORD
FilenameA2W_N(
   LPWSTR dest,
   INT destlen, /* buffer size in TCHARS incl. nullchar */
   LPCSTR src,
   INT srclen /* buffer size in TCHARS incl. nullchar */
   )
{
    DWORD ret;

    if (srclen < 0) srclen = strlen( src ) + 1;

    if (bIsFileApiAnsi)
        RtlMultiByteToUnicodeN( dest, destlen* sizeof(WCHAR), &ret, (LPSTR)src, srclen  );
    else
        RtlOemToUnicodeN( dest, destlen* sizeof(WCHAR), &ret, (LPSTR)src, srclen );

    if (ret) dest[(ret/sizeof(WCHAR))-1]=0;

    return ret/sizeof(WCHAR);
}

/*
Return: num. TCHARS copied into dest including nullterm
*/
DWORD
FilenameW2A_N(
   LPSTR dest,
   INT destlen, /* buffer size in TCHARS incl. nullchar */
   LPCWSTR src,
   INT srclen /* buffer size in TCHARS incl. nullchar */
   )
{
    DWORD ret;

    if (srclen < 0) srclen = wcslen( src ) + 1;

    if (bIsFileApiAnsi)
        RtlUnicodeToMultiByteN( dest, destlen, &ret, (LPWSTR) src, srclen * sizeof(WCHAR));
    else
        RtlUnicodeToOemN( dest, destlen, &ret, (LPWSTR) src, srclen * sizeof(WCHAR) );

    if (ret) dest[ret-1]=0;

    return ret;
}


/*
 * @implemented
 */
VOID
WINAPI
SetFileApisToOEM(VOID)
{
    /* Set the correct Base Api */
    Basep8BitStringToUnicodeString = (PRTL_CONVERT_STRING)RtlOemStringToUnicodeString;

    /* FIXME: Old, deprecated way */
    bIsFileApiAnsi = FALSE;
}


/*
 * @implemented
 */
VOID
WINAPI
SetFileApisToANSI(VOID)
{
    /* Set the correct Base Api */
    Basep8BitStringToUnicodeString = RtlAnsiStringToUnicodeString;

    /* FIXME: Old, deprecated way */
    bIsFileApiAnsi = TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
AreFileApisANSI(VOID)
{
   return bIsFileApiAnsi;
}


/*
 * @implemented
 */
HFILE WINAPI
OpenFile(LPCSTR lpFileName,
	 LPOFSTRUCT lpReOpenBuff,
	 UINT uStyle)
{
	OBJECT_ATTRIBUTES ObjectAttributes;
	IO_STATUS_BLOCK IoStatusBlock;
	UNICODE_STRING FileNameString;
	UNICODE_STRING FileNameU;
	ANSI_STRING FileName;
	WCHAR PathNameW[MAX_PATH];
	HANDLE FileHandle = NULL;
	NTSTATUS errCode;
	PWCHAR FilePart;
	ULONG Len;

	TRACE("OpenFile('%s', lpReOpenBuff %x, uStyle %x)\n", lpFileName, lpReOpenBuff, uStyle);

	if (lpReOpenBuff == NULL)
	{
		return HFILE_ERROR;
	}

    lpReOpenBuff->nErrCode = 0;

	if (uStyle & OF_REOPEN) lpFileName = lpReOpenBuff->szPathName;

	if (!lpFileName)
	{
		return HFILE_ERROR;
	}

	if (!GetFullPathNameA(lpFileName,
						  sizeof(lpReOpenBuff->szPathName),
						  lpReOpenBuff->szPathName,
						  NULL))
	{
	    lpReOpenBuff->nErrCode = GetLastError();
		return HFILE_ERROR;
	}

    if (uStyle & OF_PARSE)
    {
        lpReOpenBuff->fFixedDisk = (GetDriveTypeA(lpReOpenBuff->szPathName) != DRIVE_REMOVABLE);
        TRACE("(%s): OF_PARSE, res = '%s'\n", lpFileName, lpReOpenBuff->szPathName);
        return 0;
    }

    if ((uStyle & OF_EXIST) && !(uStyle & OF_CREATE))
    {
        DWORD dwAttributes = GetFileAttributesA(lpReOpenBuff->szPathName);

        switch (dwAttributes)
        {
            case 0xFFFFFFFF: /* File does not exist */
                SetLastError(ERROR_FILE_NOT_FOUND);
                lpReOpenBuff->nErrCode = (WORD) ERROR_FILE_NOT_FOUND;
                return -1;

            case FILE_ATTRIBUTE_DIRECTORY:
                SetLastError(ERROR_ACCESS_DENIED);
                lpReOpenBuff->nErrCode = (WORD) ERROR_ACCESS_DENIED;
                return -1;

            default:
                lpReOpenBuff->cBytes = sizeof(OFSTRUCT);
                return 1;
        }
    }
    lpReOpenBuff->cBytes = sizeof(OFSTRUCT);
	if ((uStyle & OF_CREATE) == OF_CREATE)
	{
		DWORD Sharing;
		switch (uStyle & 0x70)
		{
			case OF_SHARE_EXCLUSIVE: Sharing = 0; break;
			case OF_SHARE_DENY_WRITE: Sharing = FILE_SHARE_READ; break;
			case OF_SHARE_DENY_READ: Sharing = FILE_SHARE_WRITE; break;
			case OF_SHARE_DENY_NONE:
			case OF_SHARE_COMPAT:
			default:
				Sharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
		}
		return (HFILE) CreateFileA (lpFileName,
		                            GENERIC_READ | GENERIC_WRITE,
		                            Sharing,
		                            NULL,
		                            CREATE_ALWAYS,
		                            FILE_ATTRIBUTE_NORMAL,
		                            0);
	}

	RtlInitAnsiString (&FileName, (LPSTR)lpFileName);

	/* convert ansi (or oem) string to unicode */
	if (bIsFileApiAnsi)
		RtlAnsiStringToUnicodeString (&FileNameU, &FileName, TRUE);
	else
		RtlOemStringToUnicodeString (&FileNameU, &FileName, TRUE);

	Len = SearchPathW (NULL,
	                   FileNameU.Buffer,
        	           NULL,
	                   OFS_MAXPATHNAME,
	                   PathNameW,
        	           &FilePart);

	RtlFreeUnicodeString(&FileNameU);

	if (Len == 0 || Len > OFS_MAXPATHNAME)
	{
		lpReOpenBuff->nErrCode = GetLastError();
		return (HFILE)INVALID_HANDLE_VALUE;
	}

    if (uStyle & OF_DELETE)
    {
        if (!DeleteFileW(PathNameW))
		{
			lpReOpenBuff->nErrCode = GetLastError();
			return HFILE_ERROR;
		}
        TRACE("(%s): OF_DELETE return = OK\n", lpFileName);
        return TRUE;
    }

	FileName.Buffer = lpReOpenBuff->szPathName;
	FileName.Length = 0;
	FileName.MaximumLength = OFS_MAXPATHNAME;

	RtlInitUnicodeString(&FileNameU, PathNameW);

	/* convert unicode string to ansi (or oem) */
	if (bIsFileApiAnsi)
		RtlUnicodeStringToAnsiString (&FileName, &FileNameU, FALSE);
	else
		RtlUnicodeStringToOemString (&FileName, &FileNameU, FALSE);

	if (!RtlDosPathNameToNtPathName_U (PathNameW,
					   &FileNameString,
					   NULL,
					   NULL))
	{
		return (HFILE)INVALID_HANDLE_VALUE;
	}

	// FILE_SHARE_READ
	// FILE_NO_INTERMEDIATE_BUFFERING

	ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
	ObjectAttributes.RootDirectory = NULL;
	ObjectAttributes.ObjectName = &FileNameString;
	ObjectAttributes.Attributes = OBJ_CASE_INSENSITIVE| OBJ_INHERIT;
	ObjectAttributes.SecurityDescriptor = NULL;
	ObjectAttributes.SecurityQualityOfService = NULL;

	errCode = NtOpenFile (&FileHandle,
	                      GENERIC_READ|SYNCHRONIZE,
	                      &ObjectAttributes,
	                      &IoStatusBlock,
	                      FILE_SHARE_READ,
	                      FILE_NON_DIRECTORY_FILE|FILE_SYNCHRONOUS_IO_NONALERT);

	RtlFreeHeap(RtlGetProcessHeap(), 0, FileNameString.Buffer);

	lpReOpenBuff->nErrCode = RtlNtStatusToDosError(errCode);

	if (!NT_SUCCESS(errCode))
	{
		BaseSetLastNTError (errCode);
		return (HFILE)INVALID_HANDLE_VALUE;
	}

	if (uStyle & OF_EXIST)
	{
		NtClose(FileHandle);
		return (HFILE)1;
	}

	return (HFILE)FileHandle;
}


/*
 * @implemented
 */
BOOL WINAPI
FlushFileBuffers(IN HANDLE hFile)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    hFile = TranslateStdHandle(hFile);

    if (IsConsoleHandle(hFile))
    {
        return FlushConsoleInputBuffer(hFile);
    }

    Status = NtFlushBuffersFile(hFile,
                                &IoStatusBlock);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}


/*
 * @implemented
 */
DWORD WINAPI
SetFilePointer(HANDLE hFile,
           LONG lDistanceToMove,
           PLONG lpDistanceToMoveHigh,
           DWORD dwMoveMethod)
{
   FILE_POSITION_INFORMATION FilePosition;
   FILE_STANDARD_INFORMATION FileStandard;
   NTSTATUS errCode;
   IO_STATUS_BLOCK IoStatusBlock;
   LARGE_INTEGER Distance;

   TRACE("SetFilePointer(hFile %x, lDistanceToMove %d, dwMoveMethod %d)\n",
      hFile,lDistanceToMove,dwMoveMethod);

   if(IsConsoleHandle(hFile))
   {
     SetLastError(ERROR_INVALID_HANDLE);
     return INVALID_SET_FILE_POINTER;
   }

   if (lpDistanceToMoveHigh)
   {
      Distance.u.HighPart = *lpDistanceToMoveHigh;
      Distance.u.LowPart = lDistanceToMove;
   }
   else
   {
      Distance.QuadPart = lDistanceToMove;
   }

   switch(dwMoveMethod)
   {
     case FILE_CURRENT:
    errCode = NtQueryInformationFile(hFile,
                   &IoStatusBlock,
                   &FilePosition,
                   sizeof(FILE_POSITION_INFORMATION),
                   FilePositionInformation);
    FilePosition.CurrentByteOffset.QuadPart += Distance.QuadPart;
    if (!NT_SUCCESS(errCode))
    {
      if (lpDistanceToMoveHigh != NULL)
          *lpDistanceToMoveHigh = -1;
      BaseSetLastNTError(errCode);
      return INVALID_SET_FILE_POINTER;
    }
    break;
     case FILE_END:
    errCode = NtQueryInformationFile(hFile,
                               &IoStatusBlock,
                               &FileStandard,
                               sizeof(FILE_STANDARD_INFORMATION),
                               FileStandardInformation);
    FilePosition.CurrentByteOffset.QuadPart =
                  FileStandard.EndOfFile.QuadPart + Distance.QuadPart;
    if (!NT_SUCCESS(errCode))
    {
      if (lpDistanceToMoveHigh != NULL)
          *lpDistanceToMoveHigh = -1;
      BaseSetLastNTError(errCode);
      return INVALID_SET_FILE_POINTER;
    }
    break;
     case FILE_BEGIN:
        FilePosition.CurrentByteOffset.QuadPart = Distance.QuadPart;
    break;
     default:
        SetLastError(ERROR_INVALID_PARAMETER);
    return INVALID_SET_FILE_POINTER;
   }

   if(FilePosition.CurrentByteOffset.QuadPart < 0)
   {
     SetLastError(ERROR_NEGATIVE_SEEK);
     return INVALID_SET_FILE_POINTER;
   }

   if (lpDistanceToMoveHigh == NULL && FilePosition.CurrentByteOffset.HighPart != 0)
   {
     /* If we're moving the pointer outside of the 32 bit boundaries but
        the application only passed a 32 bit value we need to bail out! */
     SetLastError(ERROR_INVALID_PARAMETER);
     return INVALID_SET_FILE_POINTER;
   }

   errCode = NtSetInformationFile(hFile,
                  &IoStatusBlock,
                  &FilePosition,
                  sizeof(FILE_POSITION_INFORMATION),
                  FilePositionInformation);
   if (!NT_SUCCESS(errCode))
     {
       if (lpDistanceToMoveHigh != NULL)
           *lpDistanceToMoveHigh = -1;

       BaseSetLastNTError(errCode);
       return INVALID_SET_FILE_POINTER;
     }

   if (lpDistanceToMoveHigh != NULL)
     {
        *lpDistanceToMoveHigh = FilePosition.CurrentByteOffset.u.HighPart;
     }

   if (FilePosition.CurrentByteOffset.u.LowPart == MAXDWORD)
     {
       /* The value of -1 is valid here, especially when the new
          file position is greater than 4 GB. Since NtSetInformationFile
          succeeded we never set an error code and we explicitly need
          to clear a previously set error code in this case, which
          an application will check if INVALID_SET_FILE_POINTER is returned! */
       SetLastError(ERROR_SUCCESS);
     }

   return FilePosition.CurrentByteOffset.u.LowPart;
}


/*
 * @implemented
 */
BOOL
WINAPI
SetFilePointerEx(HANDLE hFile,
		 LARGE_INTEGER liDistanceToMove,
		 PLARGE_INTEGER lpNewFilePointer,
		 DWORD dwMoveMethod)
{
   FILE_POSITION_INFORMATION FilePosition;
   FILE_STANDARD_INFORMATION FileStandard;
   NTSTATUS errCode;
   IO_STATUS_BLOCK IoStatusBlock;

   if(IsConsoleHandle(hFile))
   {
     SetLastError(ERROR_INVALID_HANDLE);
     return FALSE;
   }

   switch(dwMoveMethod)
   {
     case FILE_CURRENT:
	NtQueryInformationFile(hFile,
			       &IoStatusBlock,
			       &FilePosition,
			       sizeof(FILE_POSITION_INFORMATION),
			       FilePositionInformation);
	FilePosition.CurrentByteOffset.QuadPart += liDistanceToMove.QuadPart;
	break;
     case FILE_END:
	NtQueryInformationFile(hFile,
                               &IoStatusBlock,
                               &FileStandard,
                               sizeof(FILE_STANDARD_INFORMATION),
                               FileStandardInformation);
        FilePosition.CurrentByteOffset.QuadPart =
                  FileStandard.EndOfFile.QuadPart + liDistanceToMove.QuadPart;
	break;
     case FILE_BEGIN:
        FilePosition.CurrentByteOffset.QuadPart = liDistanceToMove.QuadPart;
	break;
     default:
        SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
   }

   if(FilePosition.CurrentByteOffset.QuadPart < 0)
   {
     SetLastError(ERROR_NEGATIVE_SEEK);
     return FALSE;
   }

   errCode = NtSetInformationFile(hFile,
				  &IoStatusBlock,
				  &FilePosition,
				  sizeof(FILE_POSITION_INFORMATION),
				  FilePositionInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return FALSE;
     }

   if (lpNewFilePointer)
     {
       *lpNewFilePointer = FilePosition.CurrentByteOffset;
     }
   return TRUE;
}


/*
 * @implemented
 */
DWORD WINAPI
GetFileType(HANDLE hFile)
{
  FILE_FS_DEVICE_INFORMATION DeviceInfo;
  IO_STATUS_BLOCK StatusBlock;
  NTSTATUS Status;

  /* Get real handle */
  hFile = TranslateStdHandle(hFile);

  /* Check for console handle */
  if (IsConsoleHandle(hFile))
    {
      if (VerifyConsoleIoHandle(hFile))
	return FILE_TYPE_CHAR;
    }

  Status = NtQueryVolumeInformationFile(hFile,
					&StatusBlock,
					&DeviceInfo,
					sizeof(FILE_FS_DEVICE_INFORMATION),
					FileFsDeviceInformation);
  if (!NT_SUCCESS(Status))
    {
      BaseSetLastNTError(Status);
      return FILE_TYPE_UNKNOWN;
    }

  switch (DeviceInfo.DeviceType)
    {
      case FILE_DEVICE_CD_ROM:
      case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
      case FILE_DEVICE_CONTROLLER:
      case FILE_DEVICE_DATALINK:
      case FILE_DEVICE_DFS:
      case FILE_DEVICE_DISK:
      case FILE_DEVICE_DISK_FILE_SYSTEM:
      case FILE_DEVICE_VIRTUAL_DISK:
	return FILE_TYPE_DISK;

      case FILE_DEVICE_KEYBOARD:
      case FILE_DEVICE_MOUSE:
      case FILE_DEVICE_NULL:
      case FILE_DEVICE_PARALLEL_PORT:
      case FILE_DEVICE_PRINTER:
      case FILE_DEVICE_SERIAL_PORT:
      case FILE_DEVICE_SCREEN:
      case FILE_DEVICE_SOUND:
      case FILE_DEVICE_MODEM:
	return FILE_TYPE_CHAR;

      case FILE_DEVICE_NAMED_PIPE:
	return FILE_TYPE_PIPE;
    }

  return FILE_TYPE_UNKNOWN;
}


/*
 * @implemented
 */
DWORD WINAPI
GetFileSize(HANDLE hFile,
	    LPDWORD lpFileSizeHigh)
{
   NTSTATUS errCode;
   FILE_STANDARD_INFORMATION FileStandard;
   IO_STATUS_BLOCK IoStatusBlock;

   errCode = NtQueryInformationFile(hFile,
				    &IoStatusBlock,
				    &FileStandard,
				    sizeof(FILE_STANDARD_INFORMATION),
				    FileStandardInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	if ( lpFileSizeHigh == NULL )
	  {
	     return -1;
	  }
	else
	  {
	     return 0;
	  }
     }
   if ( lpFileSizeHigh != NULL )
     *lpFileSizeHigh = FileStandard.EndOfFile.u.HighPart;

   return FileStandard.EndOfFile.u.LowPart;
}


/*
 * @implemented
 */
BOOL
WINAPI
GetFileSizeEx(
    HANDLE hFile,
    PLARGE_INTEGER lpFileSize
    )
{
   NTSTATUS errCode;
   FILE_STANDARD_INFORMATION FileStandard;
   IO_STATUS_BLOCK IoStatusBlock;

   errCode = NtQueryInformationFile(hFile,
				    &IoStatusBlock,
				    &FileStandard,
				    sizeof(FILE_STANDARD_INFORMATION),
				    FileStandardInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return FALSE;
     }
   if (lpFileSize)
     *lpFileSize = FileStandard.EndOfFile;

   return TRUE;
}


/*
 * @implemented
 */
DWORD WINAPI
GetCompressedFileSizeA(LPCSTR lpFileName,
		       LPDWORD lpFileSizeHigh)
{
   PWCHAR FileNameW;

   if (!(FileNameW = FilenameA2W(lpFileName, FALSE)))
      return INVALID_FILE_SIZE;

   return GetCompressedFileSizeW(FileNameW, lpFileSizeHigh);
}


/*
 * @implemented
 */
DWORD WINAPI
GetCompressedFileSizeW(LPCWSTR lpFileName,
		       LPDWORD lpFileSizeHigh)
{
   FILE_COMPRESSION_INFORMATION FileCompression;
   NTSTATUS errCode;
   IO_STATUS_BLOCK IoStatusBlock;
   HANDLE hFile;

   hFile = CreateFileW(lpFileName,
		       GENERIC_READ,
		       FILE_SHARE_READ,
		       NULL,
		       OPEN_EXISTING,
		       FILE_ATTRIBUTE_NORMAL,
		       NULL);

   if (hFile == INVALID_HANDLE_VALUE)
      return INVALID_FILE_SIZE;

   errCode = NtQueryInformationFile(hFile,
				    &IoStatusBlock,
				    &FileCompression,
				    sizeof(FILE_COMPRESSION_INFORMATION),
				    FileCompressionInformation);

   CloseHandle(hFile);

   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return INVALID_FILE_SIZE;
     }

   if(lpFileSizeHigh)
    *lpFileSizeHigh = FileCompression.CompressedFileSize.u.HighPart;

   SetLastError(NO_ERROR);
   return FileCompression.CompressedFileSize.u.LowPart;
}


/*
 * @implemented
 */
BOOL WINAPI
GetFileInformationByHandle(HANDLE hFile,
			   LPBY_HANDLE_FILE_INFORMATION lpFileInformation)
{
   struct
   {
        FILE_FS_VOLUME_INFORMATION FileFsVolume;
        WCHAR Name[255];
   }
   FileFsVolume;

   FILE_BASIC_INFORMATION FileBasic;
   FILE_INTERNAL_INFORMATION FileInternal;
   FILE_STANDARD_INFORMATION FileStandard;
   NTSTATUS errCode;
   IO_STATUS_BLOCK IoStatusBlock;

   if(IsConsoleHandle(hFile))
   {
     SetLastError(ERROR_INVALID_HANDLE);
     return FALSE;
   }

   errCode = NtQueryInformationFile(hFile,
				    &IoStatusBlock,
				    &FileBasic,
				    sizeof(FILE_BASIC_INFORMATION),
				    FileBasicInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return FALSE;
     }

   lpFileInformation->dwFileAttributes = (DWORD)FileBasic.FileAttributes;

   lpFileInformation->ftCreationTime.dwHighDateTime = FileBasic.CreationTime.u.HighPart;
   lpFileInformation->ftCreationTime.dwLowDateTime = FileBasic.CreationTime.u.LowPart;

   lpFileInformation->ftLastAccessTime.dwHighDateTime = FileBasic.LastAccessTime.u.HighPart;
   lpFileInformation->ftLastAccessTime.dwLowDateTime = FileBasic.LastAccessTime.u.LowPart;

   lpFileInformation->ftLastWriteTime.dwHighDateTime = FileBasic.LastWriteTime.u.HighPart;
   lpFileInformation->ftLastWriteTime.dwLowDateTime = FileBasic.LastWriteTime.u.LowPart;

   errCode = NtQueryInformationFile(hFile,
				    &IoStatusBlock,
				    &FileInternal,
				    sizeof(FILE_INTERNAL_INFORMATION),
				    FileInternalInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return FALSE;
     }

   lpFileInformation->nFileIndexHigh = FileInternal.IndexNumber.u.HighPart;
   lpFileInformation->nFileIndexLow = FileInternal.IndexNumber.u.LowPart;

   errCode = NtQueryVolumeInformationFile(hFile,
					  &IoStatusBlock,
					  &FileFsVolume,
					  sizeof(FileFsVolume),
					  FileFsVolumeInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return FALSE;
     }

   lpFileInformation->dwVolumeSerialNumber = FileFsVolume.FileFsVolume.VolumeSerialNumber;

   errCode = NtQueryInformationFile(hFile,
				    &IoStatusBlock,
				    &FileStandard,
				    sizeof(FILE_STANDARD_INFORMATION),
				    FileStandardInformation);
   if (!NT_SUCCESS(errCode))
     {
	BaseSetLastNTError(errCode);
	return FALSE;
     }

   lpFileInformation->nNumberOfLinks = FileStandard.NumberOfLinks;
   lpFileInformation->nFileSizeHigh = FileStandard.EndOfFile.u.HighPart;
   lpFileInformation->nFileSizeLow = FileStandard.EndOfFile.u.LowPart;

   return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
GetFileAttributesExW(LPCWSTR lpFileName,
		     GET_FILEEX_INFO_LEVELS fInfoLevelId,
		     LPVOID lpFileInformation)
{
  FILE_NETWORK_OPEN_INFORMATION FileInformation;
  OBJECT_ATTRIBUTES ObjectAttributes;
  UNICODE_STRING FileName;
  NTSTATUS Status;
  WIN32_FILE_ATTRIBUTE_DATA* FileAttributeData;

  TRACE("GetFileAttributesExW(%S) called\n", lpFileName);


  if (fInfoLevelId != GetFileExInfoStandard || lpFileInformation == NULL)
  {
     SetLastError(ERROR_INVALID_PARAMETER);
     return FALSE;
  }

  /* Validate and translate the filename */
  if (!RtlDosPathNameToNtPathName_U (lpFileName,
				     &FileName,
				     NULL,
				     NULL))
    {
      WARN ("Invalid path '%S'\n", lpFileName);
      SetLastError (ERROR_BAD_PATHNAME);
      return FALSE;
    }

  /* build the object attributes */
  InitializeObjectAttributes (&ObjectAttributes,
			      &FileName,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);

  /* Get file attributes */
  Status = NtQueryFullAttributesFile(&ObjectAttributes,
                                     &FileInformation);

  RtlFreeUnicodeString (&FileName);
  if (!NT_SUCCESS (Status))
    {
      WARN ("NtQueryFullAttributesFile() failed (Status %lx)\n", Status);
      BaseSetLastNTError (Status);
      return FALSE;
    }

  FileAttributeData = (WIN32_FILE_ATTRIBUTE_DATA*)lpFileInformation;
  FileAttributeData->dwFileAttributes = FileInformation.FileAttributes;
  FileAttributeData->ftCreationTime.dwLowDateTime = FileInformation.CreationTime.u.LowPart;
  FileAttributeData->ftCreationTime.dwHighDateTime = FileInformation.CreationTime.u.HighPart;
  FileAttributeData->ftLastAccessTime.dwLowDateTime = FileInformation.LastAccessTime.u.LowPart;
  FileAttributeData->ftLastAccessTime.dwHighDateTime = FileInformation.LastAccessTime.u.HighPart;
  FileAttributeData->ftLastWriteTime.dwLowDateTime = FileInformation.LastWriteTime.u.LowPart;
  FileAttributeData->ftLastWriteTime.dwHighDateTime = FileInformation.LastWriteTime.u.HighPart;
  FileAttributeData->nFileSizeLow = FileInformation.EndOfFile.u.LowPart;
  FileAttributeData->nFileSizeHigh = FileInformation.EndOfFile.u.HighPart;

  return TRUE;
}

/*
 * @implemented
 */
BOOL WINAPI
GetFileAttributesExA(LPCSTR lpFileName,
		     GET_FILEEX_INFO_LEVELS fInfoLevelId,
		     LPVOID lpFileInformation)
{
   PWCHAR FileNameW;

   if (!(FileNameW = FilenameA2W(lpFileName, FALSE)))
      return FALSE;

   return GetFileAttributesExW(FileNameW, fInfoLevelId, lpFileInformation);
}


/*
 * @implemented
 */
DWORD WINAPI
GetFileAttributesA(LPCSTR lpFileName)
{
   WIN32_FILE_ATTRIBUTE_DATA FileAttributeData;
   PWSTR FileNameW;
   BOOL ret;

   if (!lpFileName || !(FileNameW = FilenameA2W(lpFileName, FALSE)))
      return INVALID_FILE_ATTRIBUTES;

   ret = GetFileAttributesExW(FileNameW, GetFileExInfoStandard, &FileAttributeData);

   return ret ? FileAttributeData.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
}


/*
 * @implemented
 */
DWORD WINAPI
GetFileAttributesW(LPCWSTR lpFileName)
{
  WIN32_FILE_ATTRIBUTE_DATA FileAttributeData;
  BOOL Result;

  TRACE ("GetFileAttributeW(%S) called\n", lpFileName);

  Result = GetFileAttributesExW(lpFileName, GetFileExInfoStandard, &FileAttributeData);

  return Result ? FileAttributeData.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
}


/*
 * @implemented
 */
BOOL WINAPI
GetFileAttributesByHandle(IN HANDLE hFile,
                          OUT LPDWORD dwFileAttributes,
                          IN DWORD dwFlags)
{
    FILE_BASIC_INFORMATION FileBasic;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(dwFlags);

    if (IsConsoleHandle(hFile))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Status = NtQueryInformationFile(hFile,
                                    &IoStatusBlock,
                                    &FileBasic,
                                    sizeof(FileBasic),
                                    FileBasicInformation);
    if (NT_SUCCESS(Status))
    {
        *dwFileAttributes = FileBasic.FileAttributes;
        return TRUE;
    }

    BaseSetLastNTError(Status);
    return FALSE;
}


/*
 * @implemented
 */
BOOL WINAPI
SetFileAttributesByHandle(IN HANDLE hFile,
                          IN DWORD dwFileAttributes,
                          IN DWORD dwFlags)
{
    FILE_BASIC_INFORMATION FileBasic;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(dwFlags);

    if (IsConsoleHandle(hFile))
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Status = NtQueryInformationFile(hFile,
                                    &IoStatusBlock,
                                    &FileBasic,
                                    sizeof(FileBasic),
                                    FileBasicInformation);
    if (NT_SUCCESS(Status))
    {
        FileBasic.FileAttributes = dwFileAttributes;

        Status = NtSetInformationFile(hFile,
                                      &IoStatusBlock,
                                      &FileBasic,
                                      sizeof(FileBasic),
                                      FileBasicInformation);
    }

    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
SetFileAttributesA(
   LPCSTR lpFileName,
	DWORD dwFileAttributes)
{
   PWCHAR FileNameW;

   if (!(FileNameW = FilenameA2W(lpFileName, FALSE)))
      return FALSE;

   return SetFileAttributesW(FileNameW, dwFileAttributes);
}


/*
 * @implemented
 */
BOOL WINAPI
SetFileAttributesW(LPCWSTR lpFileName,
		   DWORD dwFileAttributes)
{
  FILE_BASIC_INFORMATION FileInformation;
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatusBlock;
  UNICODE_STRING FileName;
  HANDLE FileHandle;
  NTSTATUS Status;

  TRACE ("SetFileAttributeW(%S, 0x%lx) called\n", lpFileName, dwFileAttributes);

  /* Validate and translate the filename */
  if (!RtlDosPathNameToNtPathName_U (lpFileName,
				     &FileName,
				     NULL,
				     NULL))
    {
      WARN ("Invalid path\n");
      SetLastError (ERROR_BAD_PATHNAME);
      return FALSE;
    }
  TRACE ("FileName: \'%wZ\'\n", &FileName);

  /* build the object attributes */
  InitializeObjectAttributes (&ObjectAttributes,
			      &FileName,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);

  /* Open the file */
  Status = NtOpenFile (&FileHandle,
		       SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
		       &ObjectAttributes,
		       &IoStatusBlock,
		       FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		       FILE_SYNCHRONOUS_IO_NONALERT);
  RtlFreeUnicodeString (&FileName);
  if (!NT_SUCCESS (Status))
    {
      WARN ("NtOpenFile() failed (Status %lx)\n", Status);
      BaseSetLastNTError (Status);
      return FALSE;
    }

  Status = NtQueryInformationFile(FileHandle,
				  &IoStatusBlock,
				  &FileInformation,
				  sizeof(FILE_BASIC_INFORMATION),
				  FileBasicInformation);
  if (!NT_SUCCESS(Status))
    {
      WARN ("SetFileAttributes NtQueryInformationFile failed with status 0x%08x\n", Status);
      NtClose (FileHandle);
      BaseSetLastNTError (Status);
      return FALSE;
    }

  FileInformation.FileAttributes = dwFileAttributes;
  Status = NtSetInformationFile(FileHandle,
				&IoStatusBlock,
				&FileInformation,
				sizeof(FILE_BASIC_INFORMATION),
				FileBasicInformation);
  NtClose (FileHandle);
  if (!NT_SUCCESS(Status))
    {
      WARN ("SetFileAttributes NtSetInformationFile failed with status 0x%08x\n", Status);
      BaseSetLastNTError (Status);
      return FALSE;
    }

  return TRUE;
}




/***********************************************************************
 *           GetTempFileNameA   (KERNEL32.@)
 */
UINT WINAPI
GetTempFileNameA(IN LPCSTR lpPathName,
                 IN LPCSTR lpPrefixString,
                 IN UINT uUnique,
                 OUT LPSTR lpTempFileName)
{
    UINT ID;
    NTSTATUS Status;
    LPWSTR lpTempFileNameW;
    PUNICODE_STRING lpPathNameW;
    ANSI_STRING TempFileNameStringA;
    UNICODE_STRING lpPrefixStringW, TempFileNameStringW;

    /* Convert strings */
    lpPathNameW = Basep8BitStringToStaticUnicodeString(lpPathName);
    if (!lpPathNameW)
    {
        return 0;
    }

    if (!Basep8BitStringToDynamicUnicodeString(&lpPrefixStringW, lpPrefixString))
    {
        return 0;
    }

    lpTempFileNameW = RtlAllocateHeap(RtlGetProcessHeap(), 0, MAX_PATH * sizeof(WCHAR));
    if (!lpTempFileNameW)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RtlFreeUnicodeString(&lpPrefixStringW);
        return 0;
    }

    /* Call Unicode */
    ID = GetTempFileNameW(lpPathNameW->Buffer, lpPrefixStringW.Buffer, uUnique, lpTempFileNameW);
    if (ID)
    {
        RtlInitUnicodeString(&TempFileNameStringW, lpTempFileNameW);
        TempFileNameStringA.Buffer = lpTempFileName;
        TempFileNameStringA.MaximumLength = MAX_PATH;
 
        Status = BasepUnicodeStringTo8BitString(&TempFileNameStringA, &TempFileNameStringW, FALSE);
        if (!NT_SUCCESS(Status))
        {
            BaseSetLastNTError(Status);
            ID = 0;
        }
    }

    /* Cleanup */
    RtlFreeUnicodeString(&lpPrefixStringW);
    RtlFreeHeap(RtlGetProcessHeap(), 0, lpTempFileNameW);
    return ID;
 }
 
 /***********************************************************************
  *           GetTempFileNameW   (KERNEL32.@)
  */
UINT WINAPI
GetTempFileNameW(IN LPCWSTR lpPathName,
                 IN LPCWSTR lpPrefixString,
                 IN UINT uUnique,
                 OUT LPWSTR lpTempFileName)
{
    CHAR * Let;
    HANDLE TempFile;
    UINT ID, Num = 0;
    CHAR IDString[5];
    WCHAR * TempFileName;
    CSR_API_MESSAGE ApiMessage;
    DWORD FileAttributes, LastError;
    UNICODE_STRING PathNameString, PrefixString;
    static const WCHAR Ext[] = { L'.', 't', 'm', 'p', UNICODE_NULL };

    RtlInitUnicodeString(&PathNameString, lpPathName);
    if (PathNameString.Length == 0 || PathNameString.Buffer[(PathNameString.Length / sizeof(WCHAR)) - 1] != L'\\')
    {
        PathNameString.Length += sizeof(WCHAR);
    }

    /* lpTempFileName must be able to contain: PathName, Prefix (3), number(4), .tmp(4) & \0(1)
     * See: http://msdn.microsoft.com/en-us/library/aa364991%28v=vs.85%29.aspx
     */
    if (PathNameString.Length > (MAX_PATH - 3 - 4 - 4 - 1) * sizeof(WCHAR))
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return 0;
    }
 
    /* If PathName and TempFileName aren't the same buffer, move PathName to TempFileName */
    if (lpPathName != lpTempFileName)
    {
        memmove(lpTempFileName, PathNameString.Buffer, PathNameString.Length);
    }

    /* PathName MUST BE a path. Check it */
    lpTempFileName[(PathNameString.Length / sizeof(WCHAR)) - 1] = UNICODE_NULL;
    FileAttributes = GetFileAttributesW(lpTempFileName);
    if (FileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        /* Append a '\' if necessary */
        lpTempFileName[(PathNameString.Length / sizeof(WCHAR)) - 1] = L'\\';
        lpTempFileName[PathNameString.Length / sizeof(WCHAR)] = UNICODE_NULL;
        FileAttributes = GetFileAttributesW(lpTempFileName);
        if (FileAttributes == INVALID_FILE_ATTRIBUTES)
        {
            SetLastError(ERROR_DIRECTORY);
            return 0;
        }
    }
    if (!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        SetLastError(ERROR_DIRECTORY);
        return 0;
    }
 
    /* Make sure not to mix path & prefix */
    lpTempFileName[(PathNameString.Length / sizeof(WCHAR)) - 1] = L'\\';
    RtlInitUnicodeString(&PrefixString, lpPrefixString);
    if (PrefixString.Length > 3 * sizeof(WCHAR))
    {
        PrefixString.Length = 3 * sizeof(WCHAR);
    }
 
    /* Append prefix to path */
    TempFileName = lpTempFileName + PathNameString.Length / sizeof(WCHAR);
    memmove(TempFileName, PrefixString.Buffer, PrefixString.Length);
    TempFileName += PrefixString.Length / sizeof(WCHAR);
 
    /* Then, generate filename */
    do
    {
        /* If user didn't gave any ID, ask Csrss to give one */
        if (!uUnique)
        {
            CsrClientCallServer(&ApiMessage, NULL, MAKE_CSR_API(GET_TEMP_FILE, CSR_NATIVE), sizeof(CSR_API_MESSAGE));
            if (ApiMessage.Data.GetTempFile.UniqueID == 0)
            {
                Num++;
                continue;
            }
 
            ID = ApiMessage.Data.GetTempFile.UniqueID;
        }
        else
        {
            ID = uUnique;
        }
 
        /* Convert that ID to wchar */
        RtlIntegerToChar(ID, 0x10, sizeof(IDString), IDString);
        Let = IDString;
        do
        {
            *(TempFileName++) = RtlAnsiCharToUnicodeChar(&Let);
        } while (*Let != 0);
 
        /* Append extension & UNICODE_NULL */
        memmove(TempFileName, Ext, sizeof(Ext) + sizeof(WCHAR));

        /* If user provided its ID, just return */
        if (uUnique)
        {
            return uUnique;
        }

        /* Then, try to create file */
        if (!RtlIsDosDeviceName_U(lpTempFileName))
        {
            TempFile = CreateFileW(lpTempFileName,
                                   GENERIC_READ,
                                   0,
                                   NULL,
                                   CREATE_NEW,
                                   FILE_ATTRIBUTE_NORMAL,
                                   0);
            if (TempFile != INVALID_HANDLE_VALUE)
            {
                NtClose(TempFile);
                DPRINT("Temp file: %S\n", lpTempFileName);
                return ID;
            }

            LastError = GetLastError();
            /* There is no need to recover from those errors, they would hit next step */
            if (LastError == ERROR_INVALID_PARAMETER || LastError == ERROR_CANNOT_MAKE ||
                LastError == ERROR_WRITE_PROTECT || LastError == ERROR_NETWORK_ACCESS_DENIED ||
                LastError == ERROR_DISK_FULL || LastError == ERROR_INVALID_NAME ||
                LastError == ERROR_BAD_PATHNAME || LastError == ERROR_NO_INHERITANCE ||
                LastError == ERROR_DISK_CORRUPT ||
                (LastError == ERROR_ACCESS_DENIED && NtCurrentTeb()->LastStatusValue != STATUS_FILE_IS_A_DIRECTORY))
            {
                break;
            }
        }
        Num++;
    } while (Num & 0xFFFF);
 
    return 0;
}





/*
 * @implemented
 */
BOOL WINAPI
GetFileTime(IN HANDLE hFile,
            OUT LPFILETIME lpCreationTime OPTIONAL,
            OUT LPFILETIME lpLastAccessTime OPTIONAL,
            OUT LPFILETIME lpLastWriteTime OPTIONAL)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION FileBasic;

    if(IsConsoleHandle(hFile))
    {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
    }

    Status = NtQueryInformationFile(hFile,
                                    &IoStatusBlock,
                                    &FileBasic,
                                    sizeof(FILE_BASIC_INFORMATION),
                                    FileBasicInformation);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    if (lpCreationTime)
    {
        lpCreationTime->dwLowDateTime = FileBasic.CreationTime.LowPart;
        lpCreationTime->dwHighDateTime = FileBasic.CreationTime.HighPart;
    }

    if (lpLastAccessTime)
    {
        lpLastAccessTime->dwLowDateTime = FileBasic.LastAccessTime.LowPart;
        lpLastAccessTime->dwHighDateTime = FileBasic.LastAccessTime.HighPart;
    }

    if (lpLastWriteTime)
    {
        lpLastWriteTime->dwLowDateTime = FileBasic.LastWriteTime.LowPart;
        lpLastWriteTime->dwHighDateTime = FileBasic.LastWriteTime.HighPart;
    }

    return TRUE;
}


/*
 * @implemented
 */
BOOL WINAPI
SetFileTime(IN HANDLE hFile,
            CONST FILETIME *lpCreationTime OPTIONAL,
            CONST FILETIME *lpLastAccessTime OPTIONAL,
            CONST FILETIME *lpLastWriteTime OPTIONAL)
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_BASIC_INFORMATION FileBasic;

    if(IsConsoleHandle(hFile))
    {
        BaseSetLastNTError(STATUS_INVALID_HANDLE);
        return FALSE;
    }

    memset(&FileBasic, 0, sizeof(FILE_BASIC_INFORMATION));

    if (lpCreationTime)
    {
        FileBasic.CreationTime.LowPart = lpCreationTime->dwLowDateTime;
        FileBasic.CreationTime.HighPart = lpCreationTime->dwHighDateTime;
    }

    if (lpLastAccessTime)
    {
        FileBasic.LastAccessTime.LowPart = lpLastAccessTime->dwLowDateTime;
        FileBasic.LastAccessTime.HighPart = lpLastAccessTime->dwHighDateTime;
    }

    if (lpLastWriteTime)
    {
        FileBasic.LastWriteTime.LowPart = lpLastWriteTime->dwLowDateTime;
        FileBasic.LastWriteTime.HighPart = lpLastWriteTime->dwHighDateTime;
    }

    Status = NtSetInformationFile(hFile,
                                  &IoStatusBlock,
                                  &FileBasic,
                                  sizeof(FILE_BASIC_INFORMATION),
                                  FileBasicInformation);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}


/*
 * The caller must have opened the file with the DesiredAccess FILE_WRITE_DATA flag set.
 *
 * @implemented
 */
BOOL WINAPI
SetEndOfFile(HANDLE hFile)
{
	IO_STATUS_BLOCK  IoStatusBlock;
	FILE_END_OF_FILE_INFORMATION	EndOfFileInfo;
	FILE_ALLOCATION_INFORMATION		FileAllocationInfo;
	FILE_POSITION_INFORMATION		 FilePosInfo;
	NTSTATUS Status;

	if(IsConsoleHandle(hFile))
	{
		SetLastError(ERROR_INVALID_HANDLE);
		return FALSE;
	}

	//get current position
	Status = NtQueryInformationFile(
					hFile,
					&IoStatusBlock,
					&FilePosInfo,
					sizeof(FILE_POSITION_INFORMATION),
					FilePositionInformation
					);

	if (!NT_SUCCESS(Status)){
		BaseSetLastNTError(Status);
		return FALSE;
	}

	EndOfFileInfo.EndOfFile.QuadPart = FilePosInfo.CurrentByteOffset.QuadPart;

	/*
	NOTE:
	This call is not supposed to free up any space after the eof marker
	if the file gets truncated. We have to deallocate the space explicitly afterwards.
	But...most file systems dispatch both FileEndOfFileInformation
	and FileAllocationInformation as they were the same	command.

	*/
	Status = NtSetInformationFile(
						hFile,
						&IoStatusBlock,	 //out
						&EndOfFileInfo,
						sizeof(FILE_END_OF_FILE_INFORMATION),
						FileEndOfFileInformation
						);

	if (!NT_SUCCESS(Status)){
		BaseSetLastNTError(Status);
		return FALSE;
	}

	FileAllocationInfo.AllocationSize.QuadPart = FilePosInfo.CurrentByteOffset.QuadPart;


	Status = NtSetInformationFile(
						hFile,
						&IoStatusBlock,	 //out
						&FileAllocationInfo,
						sizeof(FILE_ALLOCATION_INFORMATION),
						FileAllocationInformation
						);

	if (!NT_SUCCESS(Status)){
		BaseSetLastNTError(Status);
		return FALSE;
	}

	return TRUE;

}


/*
 * @implemented
 */
BOOL
WINAPI
SetFileValidData(
    HANDLE hFile,
    LONGLONG ValidDataLength
    )
{
	IO_STATUS_BLOCK IoStatusBlock;
	FILE_VALID_DATA_LENGTH_INFORMATION ValidDataLengthInformation;
	NTSTATUS Status;

	ValidDataLengthInformation.ValidDataLength.QuadPart = ValidDataLength;

	Status = NtSetInformationFile(
						hFile,
						&IoStatusBlock,	 //out
						&ValidDataLengthInformation,
						sizeof(FILE_VALID_DATA_LENGTH_INFORMATION),
						FileValidDataLengthInformation
						);

	if (!NT_SUCCESS(Status)){
		BaseSetLastNTError(Status);
		return FALSE;
	}

	return TRUE;
}



/*
 * @implemented
 */
BOOL
WINAPI
SetFileShortNameW(
  HANDLE hFile,
  LPCWSTR lpShortName)
{
  NTSTATUS Status;
  ULONG NeededSize;
  UNICODE_STRING ShortName;
  IO_STATUS_BLOCK IoStatusBlock;
  PFILE_NAME_INFORMATION FileNameInfo;

  if(IsConsoleHandle(hFile))
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }

  if(!lpShortName)
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  RtlInitUnicodeString(&ShortName, lpShortName);

  NeededSize = sizeof(FILE_NAME_INFORMATION) + ShortName.Length + sizeof(WCHAR);
  if(!(FileNameInfo = RtlAllocateHeap(RtlGetProcessHeap(), HEAP_ZERO_MEMORY, NeededSize)))
  {
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    return FALSE;
  }

  FileNameInfo->FileNameLength = ShortName.Length;
  RtlCopyMemory(FileNameInfo->FileName, ShortName.Buffer, ShortName.Length);

  Status = NtSetInformationFile(hFile,
                                &IoStatusBlock,	 //out
                                FileNameInfo,
                                NeededSize,
                                FileShortNameInformation);

  RtlFreeHeap(RtlGetProcessHeap(), 0, FileNameInfo);
  if(!NT_SUCCESS(Status))
  {
    BaseSetLastNTError(Status);
    return FALSE;
  }

  return TRUE;
}


/*
 * @implemented
 */
BOOL
WINAPI
SetFileShortNameA(
    HANDLE hFile,
    LPCSTR lpShortName
    )
{
  PWCHAR ShortNameW;

  if(IsConsoleHandle(hFile))
  {
    SetLastError(ERROR_INVALID_HANDLE);
    return FALSE;
  }

  if(!lpShortName)
  {
    SetLastError(ERROR_INVALID_PARAMETER);
    return FALSE;
  }

  if (!(ShortNameW = FilenameA2W(lpShortName, FALSE)))
     return FALSE;

  return SetFileShortNameW(hFile, ShortNameW);
}


/*
 * @implemented
 */
BOOL
WINAPI
CheckNameLegalDOS8Dot3W(
    LPCWSTR lpName,
    LPSTR lpOemName OPTIONAL,
    DWORD OemNameSize OPTIONAL,
    PBOOL pbNameContainsSpaces OPTIONAL,
    PBOOL pbNameLegal
    )
{
    UNICODE_STRING Name;
    ANSI_STRING AnsiName;

    if(lpName == NULL ||
       (lpOemName == NULL && OemNameSize != 0) ||
       pbNameLegal == NULL)
    {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    if(lpOemName != NULL)
    {
      AnsiName.Buffer = lpOemName;
      AnsiName.MaximumLength = (USHORT)OemNameSize * sizeof(CHAR);
      AnsiName.Length = 0;
    }

    RtlInitUnicodeString(&Name, lpName);

    *pbNameLegal = RtlIsNameLegalDOS8Dot3(&Name,
                                          (lpOemName ? &AnsiName : NULL),
                                          (BOOLEAN*)pbNameContainsSpaces);

    return TRUE;
}


/*
 * @implemented
 */
BOOL
WINAPI
CheckNameLegalDOS8Dot3A(
    LPCSTR lpName,
    LPSTR lpOemName OPTIONAL,
    DWORD OemNameSize OPTIONAL,
    PBOOL pbNameContainsSpaces OPTIONAL,
    PBOOL pbNameLegal
    )
{
    UNICODE_STRING Name;
    ANSI_STRING AnsiName, AnsiInputName;
    NTSTATUS Status;

    if(lpName == NULL ||
       (lpOemName == NULL && OemNameSize != 0) ||
       pbNameLegal == NULL)
    {
      SetLastError(ERROR_INVALID_PARAMETER);
      return FALSE;
    }

    if(lpOemName != NULL)
    {
      AnsiName.Buffer = lpOemName;
      AnsiName.MaximumLength = (USHORT)OemNameSize * sizeof(CHAR);
      AnsiName.Length = 0;
    }

    RtlInitAnsiString(&AnsiInputName, (LPSTR)lpName);
    if(bIsFileApiAnsi)
      Status = RtlAnsiStringToUnicodeString(&Name, &AnsiInputName, TRUE);
    else
      Status = RtlOemStringToUnicodeString(&Name, &AnsiInputName, TRUE);

    if(!NT_SUCCESS(Status))
    {
      BaseSetLastNTError(Status);
      return FALSE;
    }

    *pbNameLegal = RtlIsNameLegalDOS8Dot3(&Name,
                                          (lpOemName ? &AnsiName : NULL),
                                          (BOOLEAN*)pbNameContainsSpaces);

    RtlFreeUnicodeString(&Name);

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
ReplaceFileA(
    LPCSTR  lpReplacedFileName,
    LPCSTR  lpReplacementFileName,
    LPCSTR  lpBackupFileName,
    DWORD   dwReplaceFlags,
    LPVOID  lpExclude,
    LPVOID  lpReserved
    )
{
    WCHAR *replacedW, *replacementW, *backupW = NULL;
    BOOL ret;

    /* This function only makes sense when the first two parameters are defined */
    if (!lpReplacedFileName || !(replacedW = FilenameA2W(lpReplacedFileName, TRUE)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!lpReplacementFileName || !(replacementW = FilenameA2W(lpReplacementFileName, TRUE)))
    {
        HeapFree(GetProcessHeap(), 0, replacedW);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /* The backup parameter, however, is optional */
    if (lpBackupFileName)
    {
        if (!(backupW = FilenameA2W(lpBackupFileName, TRUE)))
        {
            HeapFree(GetProcessHeap(), 0, replacedW);
            HeapFree(GetProcessHeap(), 0, replacementW);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    ret = ReplaceFileW(replacedW, replacementW, backupW, dwReplaceFlags, lpExclude, lpReserved);
    HeapFree(GetProcessHeap(), 0, replacedW);
    HeapFree(GetProcessHeap(), 0, replacementW);
    HeapFree(GetProcessHeap(), 0, backupW);

    return ret;
}

/*
 * @unimplemented
 */
BOOL
WINAPI
ReplaceFileW(
    LPCWSTR lpReplacedFileName,
    LPCWSTR lpReplacementFileName,
    LPCWSTR lpBackupFileName,
    DWORD   dwReplaceFlags,
    LPVOID  lpExclude,
    LPVOID  lpReserved
    )
{
    HANDLE hReplaced = NULL, hReplacement = NULL;
    UNICODE_STRING NtReplacedName = { 0, 0, NULL };
    UNICODE_STRING NtReplacementName = { 0, 0, NULL };
    DWORD Error = ERROR_SUCCESS;
    NTSTATUS Status;
    BOOL Ret = FALSE;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PVOID Buffer = NULL ;

    if (dwReplaceFlags)
        FIXME("Ignoring flags %x\n", dwReplaceFlags);

    /* First two arguments are mandatory */
    if (!lpReplacedFileName || !lpReplacementFileName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    /* Back it up */
    if(lpBackupFileName)
    {
        if(!CopyFileW(lpReplacedFileName, lpBackupFileName, FALSE))
        {
            Error = GetLastError();
            goto Cleanup ;
        }
    }

    /* Open the "replaced" file for reading and writing */
    if (!(RtlDosPathNameToNtPathName_U(lpReplacedFileName, &NtReplacedName, NULL, NULL)))
    {
        Error = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &NtReplacedName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenFile(&hReplaced,
                        GENERIC_READ | GENERIC_WRITE | DELETE | SYNCHRONIZE | WRITE_DAC,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

    if (!NT_SUCCESS(Status))
    {
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
            Error = ERROR_FILE_NOT_FOUND;
        else
            Error = ERROR_UNABLE_TO_REMOVE_REPLACED;
        goto Cleanup;
    }

    /* Blank it */
    SetEndOfFile(hReplaced) ;

    /*
     * Open the replacement file for reading, writing, and deleting
     * (deleting is needed when finished)
     */
    if (!(RtlDosPathNameToNtPathName_U(lpReplacementFileName, &NtReplacementName, NULL, NULL)))
    {
        Error = ERROR_PATH_NOT_FOUND;
        goto Cleanup;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &NtReplacementName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenFile(&hReplacement,
                        GENERIC_READ | DELETE | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        0,
                        FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_DELETE_ON_CLOSE);

    if (!NT_SUCCESS(Status))
    {
        Error = RtlNtStatusToDosError(Status);
        goto Cleanup;
    }

    Buffer = RtlAllocateHeap(RtlGetProcessHeap(), HEAP_ZERO_MEMORY, 0x10000) ;
    if (!Buffer)
    {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup ;
    }
    while (Status != STATUS_END_OF_FILE)
    {
        Status = NtReadFile(hReplacement, NULL, NULL, NULL, &IoStatusBlock, Buffer, 0x10000, NULL, NULL) ;
        if (NT_SUCCESS(Status))
        {
            Status = NtWriteFile(hReplaced, NULL, NULL, NULL, &IoStatusBlock, Buffer,
                    IoStatusBlock.Information, NULL, NULL) ;
            if (!NT_SUCCESS(Status))
            {
                Error = RtlNtStatusToDosError(Status);
                goto Cleanup;
            }
        }
        else if (Status != STATUS_END_OF_FILE)
        {
            Error = RtlNtStatusToDosError(Status);
            goto Cleanup;
        }
    }

    Ret = TRUE;

    /* Perform resource cleanup */
Cleanup:
    if (hReplaced) NtClose(hReplaced);
    if (hReplacement) NtClose(hReplacement);
    if (Buffer) RtlFreeHeap(RtlGetProcessHeap(), 0, Buffer);

    if (NtReplacementName.Buffer)
        RtlFreeHeap(GetProcessHeap(), 0, NtReplacementName.Buffer);
    if (NtReplacedName.Buffer)
        RtlFreeHeap(GetProcessHeap(), 0, NtReplacedName.Buffer);

    /* If there was an error, set the error code */
    if(!Ret)
    {
        TRACE("ReplaceFileW failed (error=%d)\n", Error);
        SetLastError(Error);
    }
    return Ret;
}

/*
 * @implemented
 */
BOOL
WINAPI
ReadFileScatter(HANDLE hFile,
                FILE_SEGMENT_ELEMENT aSegmentArray[],
                DWORD nNumberOfBytesToRead,
                LPDWORD lpReserved,
                LPOVERLAPPED lpOverlapped)
{
    PIO_STATUS_BLOCK pIOStatus;
    LARGE_INTEGER Offset;
    NTSTATUS Status;

    DPRINT("(%p %p %u %p)\n", hFile, aSegmentArray, nNumberOfBytesToRead, lpOverlapped);

    Offset.LowPart  = lpOverlapped->Offset;
    Offset.HighPart = lpOverlapped->OffsetHigh;
    pIOStatus = (PIO_STATUS_BLOCK) lpOverlapped;
    pIOStatus->Status = STATUS_PENDING;
    pIOStatus->Information = 0;

    Status = NtReadFileScatter(hFile,
                               NULL,
                               NULL,
                               NULL,
                               pIOStatus,
                               aSegmentArray,
                               nNumberOfBytesToRead,
                               &Offset,
                               NULL);

    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}

/*
 * @implemented
 */
BOOL
WINAPI
WriteFileGather(HANDLE hFile,
                FILE_SEGMENT_ELEMENT aSegmentArray[],
                DWORD nNumberOfBytesToWrite,
                LPDWORD lpReserved,
                LPOVERLAPPED lpOverlapped)
{
    PIO_STATUS_BLOCK IOStatus;
    LARGE_INTEGER Offset;
    NTSTATUS Status;

    DPRINT("%p %p %u %p\n", hFile, aSegmentArray, nNumberOfBytesToWrite, lpOverlapped);

    Offset.LowPart = lpOverlapped->Offset;
    Offset.HighPart = lpOverlapped->OffsetHigh;
    IOStatus = (PIO_STATUS_BLOCK) lpOverlapped;
    IOStatus->Status = STATUS_PENDING;
    IOStatus->Information = 0;

    Status = NtWriteFileGather(hFile,
                               NULL,
                               NULL,
                               NULL,
                               IOStatus,
                               aSegmentArray,
                               nNumberOfBytesToWrite,
                               &Offset,
                               NULL);

    if (!NT_SUCCESS(Status))
    {
        SetLastError(RtlNtStatusToDosError(Status));
        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
OpenDataFile(HANDLE hFile, DWORD dwUnused)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
PrivMoveFileIdentityW(DWORD Unknown1, DWORD Unknown2, DWORD Unknown3)
{
    STUB;
    return FALSE;
}

/*
 * @unimplemented
 */
HANDLE
WINAPI
ReOpenFile(IN HANDLE hOriginalFile,
           IN DWORD dwDesiredAccess,
           IN DWORD dwShareMode,
           IN DWORD dwFlags)
{
   STUB;
   return INVALID_HANDLE_VALUE;
}

BOOLEAN
WINAPI
Wow64EnableWow64FsRedirection (BOOLEAN Wow64EnableWow64FsRedirection)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
Wow64DisableWow64FsRedirection (VOID ** pv)
{
    STUB;
    return FALSE;
}

BOOL
WINAPI
Wow64RevertWow64FsRedirection (VOID * pv)
{
    STUB;
    return FALSE;
}


/* EOF */
