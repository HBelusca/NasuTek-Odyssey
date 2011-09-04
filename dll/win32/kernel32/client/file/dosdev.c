/* $Id: dosdev.c 52819 2011-07-23 18:54:29Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey system libraries
 * FILE:            lib/kernel32/file/dosdev.c
 * PURPOSE:         Dos device functions
 * PROGRAMMER:      Ariadne ( ariadne@xs4all.nl)
 * UPDATE HISTORY:
 *                  Created 01/11/98
 */

/* INCLUDES ******************************************************************/

#include <k32.h>
#define NDEBUG
#include <debug.h>
#include <dbt.h>
DEBUG_CHANNEL(kernel32file);

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
BOOL
WINAPI
DefineDosDeviceA(
    DWORD dwFlags,
    LPCSTR lpDeviceName,
    LPCSTR lpTargetPath
    )
{
    UNICODE_STRING DeviceNameU = {0};
    UNICODE_STRING TargetPathU = {0};
    BOOL Result;

    if (lpDeviceName &&
        ! RtlCreateUnicodeStringFromAsciiz(&DeviceNameU,
        (LPSTR)lpDeviceName))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    if (lpTargetPath &&
        ! RtlCreateUnicodeStringFromAsciiz(&TargetPathU,
        (LPSTR)lpTargetPath))
    {
        if (DeviceNameU.Buffer)
        {
            RtlFreeHeap(RtlGetProcessHeap (),
                        0,
                        DeviceNameU.Buffer);
        }
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return 0;
    }

    Result = DefineDosDeviceW(dwFlags,
                              DeviceNameU.Buffer,
                              TargetPathU.Buffer);

    if (TargetPathU.Buffer)
    {
        RtlFreeHeap(RtlGetProcessHeap (),
                    0,
                    TargetPathU.Buffer);
    }

    if (DeviceNameU.Buffer)
    {
        RtlFreeHeap(RtlGetProcessHeap (),
                    0,
                    DeviceNameU.Buffer);
    }
    return Result;
}


/*
 * @implemented
 */
BOOL
WINAPI
DefineDosDeviceW(
    DWORD dwFlags,
    LPCWSTR lpDeviceName,
    LPCWSTR lpTargetPath
    )
{
    ULONG ArgumentCount;
    ULONG BufferSize;
    PCSR_CAPTURE_BUFFER CaptureBuffer;
    CSR_API_MESSAGE Request;
    NTSTATUS Status;
    UNICODE_STRING NtTargetPathU;
    UNICODE_STRING DeviceNameU;
    UNICODE_STRING DeviceUpcaseNameU;
    HANDLE hUser32;
    DEV_BROADCAST_VOLUME dbcv;
    BOOL Result = TRUE;
    DWORD dwRecipients;
    typedef long (WINAPI *BSM_type)(DWORD,LPDWORD,UINT,WPARAM,LPARAM);
    BSM_type BSM_ptr;

    if ( (dwFlags & 0xFFFFFFF0) ||
        ((dwFlags & DDD_EXACT_MATCH_ON_REMOVE) &&
        ! (dwFlags & DDD_REMOVE_DEFINITION)) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    ArgumentCount = 1;
    BufferSize = 0;
    if (! lpTargetPath)
    {
        RtlInitUnicodeString(&NtTargetPathU,
                             NULL);
    }
    else
    {
        if (dwFlags & DDD_RAW_TARGET_PATH)
        {
            RtlInitUnicodeString(&NtTargetPathU,
                                 lpTargetPath);
        }
        else
        {
            if (! RtlDosPathNameToNtPathName_U(lpTargetPath,
                                               &NtTargetPathU,
                                               0,
                                               0))
            {
                WARN("RtlDosPathNameToNtPathName_U() failed\n");
                BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
                return FALSE;
            }
        }
        ArgumentCount = 2;
        BufferSize += NtTargetPathU.Length;
    }

    RtlInitUnicodeString(&DeviceNameU,
                         lpDeviceName);
    RtlUpcaseUnicodeString(&DeviceUpcaseNameU,
                           &DeviceNameU,
                           TRUE);
    BufferSize += DeviceUpcaseNameU.Length;

    CaptureBuffer = CsrAllocateCaptureBuffer(ArgumentCount,
                                             BufferSize);
    if (! CaptureBuffer)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        Result = FALSE;
    }
    else
    {
        Request.Data.DefineDosDeviceRequest.dwFlags = dwFlags;

        CsrCaptureMessageBuffer(CaptureBuffer,
                                (PVOID)DeviceUpcaseNameU.Buffer,
                                DeviceUpcaseNameU.Length,
                                (PVOID*)&Request.Data.DefineDosDeviceRequest.DeviceName.Buffer);

        Request.Data.DefineDosDeviceRequest.DeviceName.Length =
            DeviceUpcaseNameU.Length;
        Request.Data.DefineDosDeviceRequest.DeviceName.MaximumLength =
            DeviceUpcaseNameU.Length;

        if (NtTargetPathU.Buffer)
        {
            CsrCaptureMessageBuffer(CaptureBuffer,
                                    (PVOID)NtTargetPathU.Buffer,
                                    NtTargetPathU.Length,
                                    (PVOID*)&Request.Data.DefineDosDeviceRequest.TargetName.Buffer);
        }
        Request.Data.DefineDosDeviceRequest.TargetName.Length =
            NtTargetPathU.Length;
        Request.Data.DefineDosDeviceRequest.TargetName.MaximumLength =
            NtTargetPathU.Length;

        Status = CsrClientCallServer(&Request,
                                     CaptureBuffer,
                                     MAKE_CSR_API(DEFINE_DOS_DEVICE, CSR_CONSOLE),
                                     sizeof(CSR_API_MESSAGE));
        CsrFreeCaptureBuffer(CaptureBuffer);

        if (! NT_SUCCESS(Status) ||
            ! NT_SUCCESS(Status = Request.Status))
        {
            WARN("CsrClientCallServer() failed (Status %lx)\n",
                Status);
            BaseSetLastNTError(Status);
            Result = FALSE;
        }
        else
        {
            if (! (dwFlags & DDD_NO_BROADCAST_SYSTEM) &&
                DeviceUpcaseNameU.Length == 2 * sizeof(WCHAR) &&
                DeviceUpcaseNameU.Buffer[1] == L':' &&
                ( (DeviceUpcaseNameU.Buffer[0] - L'A') < 26 ))
            {
                hUser32 = LoadLibraryA("user32.dll");
                if (hUser32)
                {
                    BSM_ptr = (BSM_type)
                        GetProcAddress(hUser32, "BroadcastSystemMessageW");
                    if (BSM_ptr)
                    {
                        dwRecipients = BSM_APPLICATIONS;
                        dbcv.dbcv_size = sizeof(DEV_BROADCAST_VOLUME);
                        dbcv.dbcv_devicetype = DBT_DEVTYP_VOLUME;
                        dbcv.dbcv_reserved = 0;
                        dbcv.dbcv_unitmask |= 
                            (1 << (DeviceUpcaseNameU.Buffer[0] - L'A'));
                        dbcv.dbcv_flags = DBTF_NET;
                        (void) BSM_ptr(BSF_SENDNOTIFYMESSAGE | BSF_FLUSHDISK,
                                       &dwRecipients,
                                       WM_DEVICECHANGE,
                                       (WPARAM)DBT_DEVICEARRIVAL,
                                       (LPARAM)&dbcv);
                    }
                    FreeLibrary(hUser32);
                }
            }
        }
    }

    if (NtTargetPathU.Buffer)
    {
        RtlFreeHeap(RtlGetProcessHeap(),
                    0,
                    NtTargetPathU.Buffer);
    }
    RtlFreeUnicodeString(&DeviceUpcaseNameU);
    return Result;
}


/*
 * @implemented
 */
DWORD
WINAPI
QueryDosDeviceA(
    LPCSTR lpDeviceName,
    LPSTR lpTargetPath,
    DWORD ucchMax
    )
{
  UNICODE_STRING DeviceNameU;
  UNICODE_STRING TargetPathU;
  ANSI_STRING TargetPathA;
  DWORD Length;
  DWORD CurrentLength;
  PWCHAR Buffer;

  if (lpDeviceName)
  {
    if (!RtlCreateUnicodeStringFromAsciiz (&DeviceNameU,
					   (LPSTR)lpDeviceName))
    {
      SetLastError (ERROR_NOT_ENOUGH_MEMORY);
      return 0;
    }
  }
  Buffer = RtlAllocateHeap (RtlGetProcessHeap (),
			    0,
			    ucchMax * sizeof(WCHAR));
  if (Buffer == NULL)
  {
    if (lpDeviceName)
    {
      RtlFreeHeap (RtlGetProcessHeap (),
	           0,
	           DeviceNameU.Buffer);
    }
    SetLastError (ERROR_NOT_ENOUGH_MEMORY);
    return 0;
  }

  Length = QueryDosDeviceW (lpDeviceName ? DeviceNameU.Buffer : NULL,
			    Buffer,
			    ucchMax);
  if (Length != 0)
  {
    TargetPathA.Buffer = lpTargetPath;
    TargetPathU.Buffer = Buffer;
    ucchMax = Length;

    while (ucchMax)
    {
      CurrentLength = min (ucchMax, MAXUSHORT / 2);
      TargetPathU.MaximumLength = TargetPathU.Length = (USHORT)CurrentLength * sizeof(WCHAR);
     
      TargetPathA.Length = 0;
      TargetPathA.MaximumLength = (USHORT)CurrentLength;

      RtlUnicodeStringToAnsiString (&TargetPathA,
				    &TargetPathU,
				    FALSE);
      ucchMax -= CurrentLength;
      TargetPathA.Buffer += TargetPathA.Length;
      TargetPathU.Buffer += TargetPathU.Length / sizeof(WCHAR);
    }
  }

  RtlFreeHeap (RtlGetProcessHeap (),
	       0,
	       Buffer);
  if (lpDeviceName)
  {
    RtlFreeHeap (RtlGetProcessHeap (),
	         0,
	         DeviceNameU.Buffer);
  }
  return Length;
}


/*
 * @implemented
 */
DWORD
WINAPI
QueryDosDeviceW(
    LPCWSTR lpDeviceName,
    LPWSTR lpTargetPath,
    DWORD ucchMax
    )
{
  POBJECT_DIRECTORY_INFORMATION DirInfo;
  OBJECT_ATTRIBUTES ObjectAttributes;
  UNICODE_STRING UnicodeString;
  HANDLE DirectoryHandle;
  HANDLE DeviceHandle;
  ULONG ReturnLength;
  ULONG NameLength;
  ULONG Length;
  ULONG Context;
  BOOLEAN RestartScan;
  NTSTATUS Status;
  UCHAR Buffer[512];
  PWSTR Ptr;

  /* Open the '\??' directory */
  RtlInitUnicodeString (&UnicodeString,
			L"\\??");
  InitializeObjectAttributes (&ObjectAttributes,
			      &UnicodeString,
			      OBJ_CASE_INSENSITIVE,
			      NULL,
			      NULL);
  Status = NtOpenDirectoryObject (&DirectoryHandle,
				  DIRECTORY_QUERY,
				  &ObjectAttributes);
  if (!NT_SUCCESS (Status))
  {
    WARN ("NtOpenDirectoryObject() failed (Status %lx)\n", Status);
    BaseSetLastNTError (Status);
    return 0;
  }

  Length = 0;

  if (lpDeviceName != NULL)
  {
    /* Open the lpDeviceName link object */
    RtlInitUnicodeString (&UnicodeString,
			  (PWSTR)lpDeviceName);
    InitializeObjectAttributes (&ObjectAttributes,
				&UnicodeString,
				OBJ_CASE_INSENSITIVE,
				DirectoryHandle,
				NULL);
    Status = NtOpenSymbolicLinkObject (&DeviceHandle,
				       SYMBOLIC_LINK_QUERY,
				       &ObjectAttributes);
    if (!NT_SUCCESS (Status))
    {
      WARN ("NtOpenSymbolicLinkObject() failed (Status %lx)\n", Status);
      NtClose (DirectoryHandle);
      BaseSetLastNTError (Status);
      return 0;
    }

    /* Query link target */
    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = (USHORT)ucchMax * sizeof(WCHAR);
    UnicodeString.Buffer = lpTargetPath;

    ReturnLength = 0;
    Status = NtQuerySymbolicLinkObject (DeviceHandle,
					&UnicodeString,
					&ReturnLength);
    NtClose (DeviceHandle);
    NtClose (DirectoryHandle);
    if (!NT_SUCCESS (Status))
    {
      WARN ("NtQuerySymbolicLinkObject() failed (Status %lx)\n", Status);
      BaseSetLastNTError (Status);
      return 0;
    }

    TRACE ("ReturnLength: %lu\n", ReturnLength);
    TRACE ("TargetLength: %hu\n", UnicodeString.Length);
    TRACE ("Target: '%wZ'\n", &UnicodeString);

    Length = UnicodeString.Length / sizeof(WCHAR);
    if (Length < ucchMax)
    {
      /* Append null-charcter */
      lpTargetPath[Length] = UNICODE_NULL;
      Length++;
    }
    else
    {
      TRACE ("Buffer is too small\n");
      BaseSetLastNTError (STATUS_BUFFER_TOO_SMALL);
      return 0;
    }
  }
  else
  {
    RestartScan = TRUE;
    Context = 0;
    Ptr = lpTargetPath;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)Buffer;

    while (TRUE)
    {
      Status = NtQueryDirectoryObject (DirectoryHandle,
				       Buffer,
				       sizeof (Buffer),
				       TRUE,
				       RestartScan,
				       &Context,
				       &ReturnLength);
      if (!NT_SUCCESS(Status))
      {
	if (Status == STATUS_NO_MORE_ENTRIES)
	{
	  /* Terminate the buffer */
	  *Ptr = UNICODE_NULL;
	  Length++;

	  Status = STATUS_SUCCESS;
	}
	else
	{
	  Length = 0;
	}
	BaseSetLastNTError (Status);
	break;
      }

      if (!wcscmp (DirInfo->TypeName.Buffer, L"SymbolicLink"))
      {
	TRACE ("Name: '%wZ'\n", &DirInfo->Name);

	NameLength = DirInfo->Name.Length / sizeof(WCHAR);
	if (Length + NameLength + 1 >= ucchMax)
	{
	  Length = 0;
	  BaseSetLastNTError (STATUS_BUFFER_TOO_SMALL);
	  break;
	}

	memcpy (Ptr,
		DirInfo->Name.Buffer,
		DirInfo->Name.Length);
	Ptr += NameLength;
	Length += NameLength;
	*Ptr = UNICODE_NULL;
	Ptr++;
	Length++;
      }

      RestartScan = FALSE;
    }

    NtClose (DirectoryHandle);
  }

  return Length;
}

/* EOF */
