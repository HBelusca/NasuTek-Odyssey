/* $Id: cnotify.c 52819 2011-07-23 18:54:29Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/kernel32/file/find.c
 * PURPOSE:         Find functions
 * PROGRAMMER:      Ariadne ( ariadne@xs4all.nl)
 * UPDATE HISTORY:
 *                  Created 01/11/98
 */

/* INCLUDES *****************************************************************/

#include <k32.h>
#define NDEBUG
#include <debug.h>

/* FUNCTIONS ****************************************************************/

/*
 * @implemented
 */
BOOL WINAPI
FindCloseChangeNotification (HANDLE hChangeHandle)
{
   NTSTATUS Status = NtClose(hChangeHandle);
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
HANDLE
WINAPI
FindFirstChangeNotificationA(IN LPCSTR lpPathName,
                             IN BOOL bWatchSubtree,
                             IN DWORD dwNotifyFilter)
{
    /* Call the W(ide) function */
    ConvertWin32AnsiChangeApiToUnicodeApi(FindFirstChangeNotification,
                                          lpPathName,
                                          bWatchSubtree,
                                          dwNotifyFilter);
}


/*
 * @implemented
 */
HANDLE
WINAPI
FindFirstChangeNotificationW (
	LPCWSTR	lpPathName,
	BOOL	bWatchSubtree,
	DWORD	dwNotifyFilter
	)
{
   NTSTATUS Status;
   UNICODE_STRING NtPathU;
   IO_STATUS_BLOCK IoStatus;
   OBJECT_ATTRIBUTES ObjectAttributes;
   HANDLE hDir;

   if (!RtlDosPathNameToNtPathName_U (lpPathName,
                                          &NtPathU,
                                          NULL,
                                          NULL))
   {
      BaseSetLastNTError(STATUS_OBJECT_PATH_SYNTAX_BAD);
      return INVALID_HANDLE_VALUE;
   }



   InitializeObjectAttributes (&ObjectAttributes,
                               &NtPathU,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

   Status = NtOpenFile (&hDir,
                        SYNCHRONIZE|FILE_LIST_DIRECTORY,
                        &ObjectAttributes,
                        &IoStatus,
                        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                        FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT);

   RtlFreeHeap(RtlGetProcessHeap(),
               0,
               NtPathU.Buffer);



   if (!NT_SUCCESS(Status))
   {
      BaseSetLastNTError(Status);
      return INVALID_HANDLE_VALUE;
   }

   Status = NtNotifyChangeDirectoryFile(hDir,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        NULL,//Buffer,
                                        0,//BufferLength,
                                        dwNotifyFilter,
                                        (BOOLEAN)bWatchSubtree);
   if (!NT_SUCCESS(Status))
   {
      NtClose(hDir);
      BaseSetLastNTError(Status);
      return INVALID_HANDLE_VALUE;
   }

   return hDir;
}


/*
 * @implemented
 */
BOOL
WINAPI
FindNextChangeNotification (
	HANDLE	hChangeHandle
	)
{
   IO_STATUS_BLOCK IoStatus;
   NTSTATUS Status;

   Status = NtNotifyChangeDirectoryFile(hChangeHandle,
      NULL,
      NULL,
      NULL,
      &IoStatus,
      NULL,//Buffer,
      0,//BufferLength,
      FILE_NOTIFY_CHANGE_SECURITY,//meaningless/ignored for subsequent calls, but must contain a valid flag
      0 //meaningless/ignored for subsequent calls
      );

   if (!NT_SUCCESS(Status))
   {
      BaseSetLastNTError(Status);
      return FALSE;
   }

   return TRUE;
}


extern VOID
(WINAPI ApcRoutine)(PVOID ApcContext,
      struct _IO_STATUS_BLOCK* IoStatusBlock,
      ULONG Reserved);


/*
 * @implemented
 */
BOOL
WINAPI
ReadDirectoryChangesW(
    HANDLE hDirectory,
    LPVOID lpBuffer OPTIONAL,
    DWORD nBufferLength,
    BOOL bWatchSubtree,
    DWORD dwNotifyFilter,
    LPDWORD lpBytesReturned, /* undefined for asych. operations */
    LPOVERLAPPED lpOverlapped OPTIONAL,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine /* OPTIONAL???????? */
    )
{
   PVOID CompletionRoutine;
   NTSTATUS Status;
   IO_STATUS_BLOCK IoStatus;
   HANDLE EventHandle;
   PIO_APC_ROUTINE IoApcRoutine;

   if (lpOverlapped )
   {
      if (lpCompletionRoutine)
      {
          CompletionRoutine = (PVOID) lpCompletionRoutine;
          EventHandle = NULL;
          IoApcRoutine = ApcRoutine;
      }
      else
      {
          if (((ULONG_PTR) lpOverlapped->hEvent & 1) == 0)
              CompletionRoutine = (PVOID) lpOverlapped;
          else
              CompletionRoutine = NULL;

          EventHandle = lpOverlapped->hEvent;
          IoApcRoutine = NULL;
      }

      lpOverlapped->Internal = STATUS_PENDING;
   }
   else
   {
       EventHandle = NULL;
       IoApcRoutine = NULL;
       CompletionRoutine = NULL;
   }

   Status = NtNotifyChangeDirectoryFile(
      hDirectory,
      EventHandle,
      IoApcRoutine,
      CompletionRoutine, /* ApcContext */
      lpOverlapped ? (PIO_STATUS_BLOCK) lpOverlapped : &IoStatus,
      lpBuffer,
      nBufferLength,
      dwNotifyFilter,
      (BOOLEAN)bWatchSubtree
      );

   if ((Status == STATUS_PENDING) && (!lpOverlapped))
   {
       Status = NtWaitForSingleObject(hDirectory, FALSE, NULL);

       if (NT_SUCCESS(Status))
       {
           Status = IoStatus.Status;
       }
   }

   if (!NT_SUCCESS(Status))
   {
      BaseSetLastNTError(Status);
      return FALSE;
   }


   /* NOTE: lpBytesReturned is undefined for asynch. operations */
   *lpBytesReturned = IoStatus.Information;

   return TRUE;
}





/* EOF */
