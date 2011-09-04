/* $Id: copy.c 52819 2011-07-23 18:54:29Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/kernel32/file/copy.c
 * PURPOSE:         Copying files
 * PROGRAMMER:      Ariadne (ariadne@xs4all.nl)
 * UPDATE HISTORY:
 *                  01/11/98 Created
 *                  07/02/99 Moved to seperate file
 */

/* INCLUDES ****************************************************************/

#include <k32.h>
#define NDEBUG
#include <debug.h>

#if DBG
DEBUG_CHANNEL(kernel32file);
#endif

/* FUNCTIONS ****************************************************************/


static NTSTATUS
CopyLoop (
	HANDLE			FileHandleSource,
	HANDLE			FileHandleDest,
	LARGE_INTEGER		SourceFileSize,
	LPPROGRESS_ROUTINE	lpProgressRoutine,
	LPVOID			lpData,
	BOOL			*pbCancel,
	BOOL                 *KeepDest
	)
{
   NTSTATUS errCode;
   IO_STATUS_BLOCK IoStatusBlock;
   UCHAR *lpBuffer = NULL;
   SIZE_T RegionSize = 0x10000;
   LARGE_INTEGER BytesCopied;
   DWORD CallbackReason;
   DWORD ProgressResult;
   BOOL EndOfFileFound;

   *KeepDest = FALSE;
   errCode = NtAllocateVirtualMemory(NtCurrentProcess(),
				     (PVOID *)&lpBuffer,
				     2,
				     &RegionSize,
				     MEM_RESERVE | MEM_COMMIT,
				     PAGE_READWRITE);

   if (NT_SUCCESS(errCode))
     {
	BytesCopied.QuadPart = 0;
	EndOfFileFound = FALSE;
	CallbackReason = CALLBACK_STREAM_SWITCH;
	while (! EndOfFileFound &&
	       NT_SUCCESS(errCode) &&
	       (NULL == pbCancel || ! *pbCancel))
	  {
	     if (NULL != lpProgressRoutine)
	       {
		   ProgressResult = (*lpProgressRoutine)(SourceFileSize,
							 BytesCopied,
							 SourceFileSize,
							 BytesCopied,
							 0,
							 CallbackReason,
							 FileHandleSource,
							 FileHandleDest,
							 lpData);
		   switch (ProgressResult)
		     {
		     case PROGRESS_CANCEL:
			TRACE("Progress callback requested cancel\n");
			errCode = STATUS_REQUEST_ABORTED;
			break;
		     case PROGRESS_STOP:
			TRACE("Progress callback requested stop\n");
			errCode = STATUS_REQUEST_ABORTED;
			*KeepDest = TRUE;
			break;
		     case PROGRESS_QUIET:
			lpProgressRoutine = NULL;
			break;
		     case PROGRESS_CONTINUE:
		     default:
			break;
		     }
		   CallbackReason = CALLBACK_CHUNK_FINISHED;
	       }
	     if (NT_SUCCESS(errCode))
	       {
		  errCode = NtReadFile(FileHandleSource,
				       NULL,
				       NULL,
				       NULL,
				       (PIO_STATUS_BLOCK)&IoStatusBlock,
				       lpBuffer,
				       RegionSize,
				       NULL,
				       NULL);
		  if (NT_SUCCESS(errCode) && (NULL == pbCancel || ! *pbCancel))
		    {
		       errCode = NtWriteFile(FileHandleDest,
					     NULL,
					     NULL,
					     NULL,
					     (PIO_STATUS_BLOCK)&IoStatusBlock,
					     lpBuffer,
					     IoStatusBlock.Information,
					     NULL,
					     NULL);
		       if (NT_SUCCESS(errCode))
			 {
			    BytesCopied.QuadPart += IoStatusBlock.Information;
			 }
		       else
			 {
			    WARN("Error 0x%08x reading writing to dest\n", errCode);
			 }
		    }
		  else if (!NT_SUCCESS(errCode))
		    {
		       if (STATUS_END_OF_FILE == errCode)
			 {
			    EndOfFileFound = TRUE;
			    errCode = STATUS_SUCCESS;
			 }
		       else
			 {
			    WARN("Error 0x%08x reading from source\n", errCode);
			 }
		    }
	       }
	  }

	if (! EndOfFileFound && (NULL != pbCancel && *pbCancel))
	  {
	  TRACE("User requested cancel\n");
	  errCode = STATUS_REQUEST_ABORTED;
	  }

	NtFreeVirtualMemory(NtCurrentProcess(),
			    (PVOID *)&lpBuffer,
			    &RegionSize,
			    MEM_RELEASE);
     }
   else
     {
	TRACE("Error 0x%08x allocating buffer of %d bytes\n", errCode, RegionSize);
     }

   return errCode;
}

static NTSTATUS
SetLastWriteTime(
	HANDLE FileHandle,
	LARGE_INTEGER LastWriteTime
	)
{
   NTSTATUS errCode = STATUS_SUCCESS;
   IO_STATUS_BLOCK IoStatusBlock;
   FILE_BASIC_INFORMATION FileBasic;

   errCode = NtQueryInformationFile (FileHandle,
				     &IoStatusBlock,
				     &FileBasic,
				     sizeof(FILE_BASIC_INFORMATION),
				     FileBasicInformation);
   if (!NT_SUCCESS(errCode))
     {
	WARN("Error 0x%08x obtaining FileBasicInformation\n", errCode);
     }
   else
     {
	FileBasic.LastWriteTime.QuadPart = LastWriteTime.QuadPart;
	errCode = NtSetInformationFile (FileHandle,
					&IoStatusBlock,
					&FileBasic,
					sizeof(FILE_BASIC_INFORMATION),
					FileBasicInformation);
	if (!NT_SUCCESS(errCode))
	  {
	     WARN("Error 0x%0x setting LastWriteTime\n", errCode);
	  }
     }

   return errCode;
}


/*
 * @implemented
 */
BOOL
WINAPI
CopyFileExW (
	LPCWSTR			lpExistingFileName,
	LPCWSTR			lpNewFileName,
	LPPROGRESS_ROUTINE	lpProgressRoutine,
	LPVOID			lpData,
	BOOL			*pbCancel,
	DWORD			dwCopyFlags
	)
{
   NTSTATUS errCode;
   HANDLE FileHandleSource, FileHandleDest;
   IO_STATUS_BLOCK IoStatusBlock;
   FILE_STANDARD_INFORMATION FileStandard;
   FILE_BASIC_INFORMATION FileBasic;
   BOOL RC = FALSE;
   BOOL KeepDestOnError = FALSE;
   DWORD SystemError;

   FileHandleSource = CreateFileW(lpExistingFileName,
				  GENERIC_READ,
				  FILE_SHARE_READ | FILE_SHARE_WRITE,
				  NULL,
				  OPEN_EXISTING,
				  FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
				  NULL);
   if (INVALID_HANDLE_VALUE != FileHandleSource)
     {
	errCode = NtQueryInformationFile(FileHandleSource,
					 &IoStatusBlock,
					 &FileStandard,
					 sizeof(FILE_STANDARD_INFORMATION),
					 FileStandardInformation);
	if (!NT_SUCCESS(errCode))
	  {
	     TRACE("Status 0x%08x obtaining FileStandardInformation for source\n", errCode);
	     BaseSetLastNTError(errCode);
	  }
	else
	  {
	     errCode = NtQueryInformationFile(FileHandleSource,
	  				      &IoStatusBlock,&FileBasic,
					      sizeof(FILE_BASIC_INFORMATION),
					      FileBasicInformation);
	     if (!NT_SUCCESS(errCode))
	       {
		  TRACE("Status 0x%08x obtaining FileBasicInformation for source\n", errCode);
		  BaseSetLastNTError(errCode);
	       }
	     else
	       {
		  FileHandleDest = CreateFileW(lpNewFileName,
					       GENERIC_WRITE,
					       FILE_SHARE_WRITE,
					       NULL,
					       dwCopyFlags ? CREATE_NEW : CREATE_ALWAYS,
                                               FileBasic.FileAttributes,
					       NULL);
		  if (INVALID_HANDLE_VALUE != FileHandleDest)
		    {
		       errCode = CopyLoop(FileHandleSource,
					  FileHandleDest,
					  FileStandard.EndOfFile,
					  lpProgressRoutine,
					  lpData,
					  pbCancel,
					  &KeepDestOnError);
		       if (!NT_SUCCESS(errCode))
			 {
			    BaseSetLastNTError(errCode);
			 }
		       else
			 {
                LARGE_INTEGER t;

			    t.QuadPart = FileBasic.LastWriteTime.QuadPart;
			    errCode = SetLastWriteTime(FileHandleDest, t);
			    if (!NT_SUCCESS(errCode))
			      {
				 BaseSetLastNTError(errCode);
			      }
			    else
			      {
				 RC = TRUE;
			      }
			 }
		       NtClose(FileHandleDest);
		       if (! RC && ! KeepDestOnError)
			 {
			    SystemError = GetLastError();
			    SetFileAttributesW(lpNewFileName, FILE_ATTRIBUTE_NORMAL);
			    DeleteFileW(lpNewFileName);
			    SetLastError(SystemError);
			 }
		    }
		  else
		    {
		    WARN("Error %d during opening of dest file\n", GetLastError());
		    }
	       }
	  }
	NtClose(FileHandleSource);
     }
   else
     {
     WARN("Error %d during opening of source file\n", GetLastError());
     }

   return RC;
}


/*
 * @implemented
 */
BOOL
WINAPI
CopyFileExA(IN LPCSTR lpExistingFileName,
            IN LPCSTR lpNewFileName,
            IN LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
            IN LPVOID lpData OPTIONAL,
            IN LPBOOL pbCancel OPTIONAL,
            IN DWORD dwCopyFlags)
{
    BOOL Result = FALSE;
    UNICODE_STRING lpNewFileNameW;
    PUNICODE_STRING lpExistingFileNameW;

    lpExistingFileNameW = Basep8BitStringToStaticUnicodeString(lpExistingFileName);
    if (!lpExistingFileName)
    {
        return FALSE;
    }

    if (Basep8BitStringToDynamicUnicodeString(&lpNewFileNameW, lpNewFileName))
    {
        Result = CopyFileExW(lpExistingFileNameW->Buffer,
                             lpNewFileNameW.Buffer,
                             lpProgressRoutine,
                             lpData,
                             pbCancel,
                             dwCopyFlags);

        RtlFreeUnicodeString(&lpNewFileNameW);
    }

    return Result;
}


/*
 * @implemented
 */
BOOL
WINAPI
CopyFileA (
	LPCSTR	lpExistingFileName,
	LPCSTR	lpNewFileName,
	BOOL	bFailIfExists
	)
{
	return CopyFileExA (lpExistingFileName,
	                    lpNewFileName,
	                    NULL,
	                    NULL,
	                    NULL,
	                    bFailIfExists);
}


/*
 * @implemented
 */
BOOL
WINAPI
CopyFileW (
	LPCWSTR	lpExistingFileName,
	LPCWSTR	lpNewFileName,
	BOOL	bFailIfExists
	)
{
	return CopyFileExW (lpExistingFileName,
	                    lpNewFileName,
	                    NULL,
	                    NULL,
	                    NULL,
	                    bFailIfExists);
}


/*
 * @implemented
 */
BOOL
WINAPI
PrivCopyFileExW (
	LPCWSTR			lpExistingFileName,
	LPCWSTR			lpNewFileName,
	LPPROGRESS_ROUTINE	lpProgressRoutine,
	LPVOID			lpData,
	BOOL			*pbCancel,
	DWORD			dwCopyFlags
	)
{
    UNIMPLEMENTED;
    return FALSE;
}

/* EOF */
