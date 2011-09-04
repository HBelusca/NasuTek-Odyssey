/*
 *  Odyssey W32 Subsystem
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 ReactOS Team; (C) 2011 NasuTek Enterprises
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
 *
 *  COPYRIGHT:        See COPYING in the top level directory
 *  PROJECT:          Odyssey kernel
 *  PURPOSE:          Window stations
 *  FILE:             subsys/win32k/ntuser/winsta.c
 *  PROGRAMER:        Casper S. Hornstrup (chorns@users.sourceforge.net)
 *  REVISION HISTORY:
 *       06-06-2001  CSH  Created
 *  NOTES:            Exported functions set the Win32 last error value
 *                    on errors. The value can be retrieved with the Win32
 *                    function GetLastError().
 *  TODO:             The process window station is created on
 *                    the first USER32/GDI32 call not related
 *                    to window station/desktop handling
 */

/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserWinsta);

/* GLOBALS *******************************************************************/

/* Currently active window station */
PWINSTATION_OBJECT InputWindowStation = NULL;

/* Winlogon sas window*/
HWND hwndSAS = NULL;

/* INITALIZATION FUNCTIONS ****************************************************/

static GENERIC_MAPPING IntWindowStationMapping =
   {
      STANDARD_RIGHTS_READ     | WINSTA_ENUMDESKTOPS      | WINSTA_ENUMERATE         | WINSTA_READATTRIBUTES | WINSTA_READSCREEN,
      STANDARD_RIGHTS_WRITE    | WINSTA_ACCESSCLIPBOARD   | WINSTA_CREATEDESKTOP     | WINSTA_WRITEATTRIBUTES,
      STANDARD_RIGHTS_EXECUTE  | WINSTA_ACCESSGLOBALATOMS | WINSTA_EXITWINDOWS,
      STANDARD_RIGHTS_REQUIRED | WINSTA_ACCESSCLIPBOARD   | WINSTA_ACCESSGLOBALATOMS | WINSTA_CREATEDESKTOP  |
      WINSTA_ENUMDESKTOPS      | WINSTA_ENUMERATE         | WINSTA_EXITWINDOWS    |
      WINSTA_READATTRIBUTES    | WINSTA_READSCREEN        | WINSTA_WRITEATTRIBUTES
   };


INIT_FUNCTION
NTSTATUS
NTAPI
InitWindowStationImpl(VOID)
{
   OBJECT_ATTRIBUTES ObjectAttributes;
   HANDLE WindowStationsDirectory;
   UNICODE_STRING UnicodeString;
   NTSTATUS Status;

   /*
    * Create the '\Windows\WindowStations' directory
    */

   RtlInitUnicodeString(&UnicodeString, WINSTA_ROOT_NAME);
   InitializeObjectAttributes(&ObjectAttributes, &UnicodeString,
                              0, NULL, NULL);
   Status = ZwCreateDirectoryObject(&WindowStationsDirectory, 0,
                                    &ObjectAttributes);
   if (!NT_SUCCESS(Status))
   {
      TRACE("Could not create \\Windows\\WindowStations directory "
             "(Status 0x%X)\n", Status);
      return Status;
   }

   /* Set Winsta Object Attributes */
   ExWindowStationObjectType->TypeInfo.DefaultNonPagedPoolCharge = sizeof(WINSTATION_OBJECT);
   ExWindowStationObjectType->TypeInfo.GenericMapping = IntWindowStationMapping;

   return STATUS_SUCCESS;
}

NTSTATUS FASTCALL
CleanupWindowStationImpl(VOID)
{
   return STATUS_SUCCESS;
}

BOOL FASTCALL
IntSetupClipboard(PWINSTATION_OBJECT WinStaObj)
{
    WinStaObj->Clipboard = ExAllocatePoolWithTag(PagedPool, sizeof(CLIPBOARDSYSTEM), TAG_WINSTA);
    if (WinStaObj->Clipboard)
    {
        RtlZeroMemory(WinStaObj->Clipboard, sizeof(CLIPBOARDSYSTEM));
        return TRUE;
    }
    return FALSE;
}

/* OBJECT CALLBACKS  **********************************************************/

VOID APIENTRY
IntWinStaObjectDelete(PWIN32_DELETEMETHOD_PARAMETERS Parameters)
{
   PWINSTATION_OBJECT WinSta = (PWINSTATION_OBJECT)Parameters->Object;

   TRACE("Deleting window station (0x%X)\n", WinSta);

   RtlDestroyAtomTable(WinSta->AtomTable);

   RtlFreeUnicodeString(&WinSta->Name);
}

NTSTATUS
APIENTRY
IntWinStaObjectParse(PWIN32_PARSEMETHOD_PARAMETERS Parameters)
{
    PUNICODE_STRING RemainingName = Parameters->RemainingName;

    /* Assume we don't find anything */
    *Parameters->Object = NULL;

    /* Check for an empty name */
    if (!RemainingName->Length)
    {
        /* Make sure this is a window station, can't parse a desktop now */
        if (Parameters->ObjectType != ExWindowStationObjectType)
        {
            /* Fail */
            return STATUS_OBJECT_TYPE_MISMATCH;
        }

        /* Reference the window station and return */
        ObReferenceObject(Parameters->ParseObject);
        *Parameters->Object = Parameters->ParseObject;
        return STATUS_SUCCESS;
    }

    /* Check for leading slash */
    if (RemainingName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR)
    {
        /* Skip it */
        RemainingName->Buffer++;
        RemainingName->Length -= sizeof(WCHAR);
        RemainingName->MaximumLength -= sizeof(WCHAR);
    }

    /* Check if there is still a slash */
    if (wcschr(RemainingName->Buffer, OBJ_NAME_PATH_SEPARATOR))
    {
        /* In this case, fail */
        return STATUS_OBJECT_PATH_INVALID;
    }

    /*
     * Check if we are parsing a desktop.
     */
    if (Parameters->ObjectType == ExDesktopObjectType)
    {
        /* Then call the desktop parse routine */
        return IntDesktopObjectParse(Parameters->ParseObject,
                                     Parameters->ObjectType,
                                     Parameters->AccessState,
                                     Parameters->AccessMode,
                                     Parameters->Attributes,
                                     Parameters->CompleteName,
                                     RemainingName,
                                     Parameters->Context,
                                     Parameters->SecurityQos,
                                     Parameters->Object);
    }

    /* Should hopefully never get here */
    return STATUS_OBJECT_TYPE_MISMATCH;
}

NTSTATUS
NTAPI
IntWinstaOkToClose(PWIN32_OKAYTOCLOSEMETHOD_PARAMETERS Parameters)
{
    PPROCESSINFO ppi;

    ppi = PsGetCurrentProcessWin32Process();

    if(ppi && (Parameters->Handle == ppi->hwinsta))
    {
        return STATUS_ACCESS_DENIED;
    }

    return STATUS_SUCCESS;
}

/* PRIVATE FUNCTIONS **********************************************************/

/*
 * IntGetFullWindowStationName
 *
 * Get a full window station object name from a name specified in
 * NtUserCreateWindowStation, NtUserOpenWindowStation, NtUserCreateDesktop
 * or NtUserOpenDesktop.
 *
 * Return Value
 *    TRUE on success, FALSE on failure.
 */

BOOL FASTCALL
IntGetFullWindowStationName(
   OUT PUNICODE_STRING FullName,
   IN PUNICODE_STRING WinStaName,
   IN OPTIONAL PUNICODE_STRING DesktopName)
{
   PWCHAR Buffer;

   FullName->Length = WINSTA_ROOT_NAME_LENGTH * sizeof(WCHAR);
   if (WinStaName != NULL)
      FullName->Length += WinStaName->Length + sizeof(WCHAR);
   if (DesktopName != NULL)
      FullName->Length += DesktopName->Length + sizeof(WCHAR);
   FullName->MaximumLength = FullName->Length;
   FullName->Buffer = ExAllocatePoolWithTag(PagedPool, FullName->Length, TAG_STRING);
   if (FullName->Buffer == NULL)
   {
      return FALSE;
   }

   Buffer = FullName->Buffer;
   memcpy(Buffer, WINSTA_ROOT_NAME, WINSTA_ROOT_NAME_LENGTH * sizeof(WCHAR));
   Buffer += WINSTA_ROOT_NAME_LENGTH;
   if (WinStaName != NULL)
   {
      memcpy(Buffer, L"\\", sizeof(WCHAR));
      Buffer ++;
      memcpy(Buffer, WinStaName->Buffer, WinStaName->Length);

      if (DesktopName != NULL)
      {
         Buffer += WinStaName->Length / sizeof(WCHAR);
         memcpy(Buffer, L"\\", sizeof(WCHAR));
         Buffer ++;
         memcpy(Buffer, DesktopName->Buffer, DesktopName->Length);
      }
   }

   return TRUE;
}

/*
 * IntValidateWindowStationHandle
 *
 * Validates the window station handle.
 *
 * Remarks
 *    If the function succeeds, the handle remains referenced. If the
 *    fucntion fails, last error is set.
 */

NTSTATUS FASTCALL
IntValidateWindowStationHandle(
   HWINSTA WindowStation,
   KPROCESSOR_MODE AccessMode,
   ACCESS_MASK DesiredAccess,
   PWINSTATION_OBJECT *Object)
{
   NTSTATUS Status;

   if (WindowStation == NULL)
   {
      //      ERR("Invalid window station handle\n");
      EngSetLastError(ERROR_INVALID_HANDLE);
      return STATUS_INVALID_HANDLE;
   }

   Status = ObReferenceObjectByHandle(
               WindowStation,
               DesiredAccess,
               ExWindowStationObjectType,
               AccessMode,
               (PVOID*)Object,
               NULL);

   if (!NT_SUCCESS(Status))
      SetLastNtError(Status);

   return Status;
}

BOOL FASTCALL
IntGetWindowStationObject(PWINSTATION_OBJECT Object)
{
   NTSTATUS Status;

   Status = ObReferenceObjectByPointer(
               Object,
               KernelMode,
               ExWindowStationObjectType,
               0);

   return NT_SUCCESS(Status);
}


BOOL FASTCALL
co_IntInitializeDesktopGraphics(VOID)
{
   TEXTMETRICW tmw;
   UNICODE_STRING DriverName = RTL_CONSTANT_STRING(L"DISPLAY");
   if (! IntCreatePrimarySurface())
   {
      return FALSE;
   }
   ScreenDeviceContext = IntGdiCreateDC(&DriverName, NULL, NULL, NULL, FALSE);
   if (NULL == ScreenDeviceContext)
   {
      IntDestroyPrimarySurface();
      return FALSE;
   }
   GreSetDCOwner(ScreenDeviceContext, GDI_OBJ_HMGR_PUBLIC);

   /* Setup the cursor */
   co_IntLoadDefaultCursors();

   hSystemBM = NtGdiCreateCompatibleDC(ScreenDeviceContext);

   NtGdiSelectFont(hSystemBM, NtGdiGetStockObject(SYSTEM_FONT));
   GreSetDCOwner(hSystemBM, GDI_OBJ_HMGR_PUBLIC);

   // FIXME! Move these to a update routine.
   gpsi->Planes        = NtGdiGetDeviceCaps(ScreenDeviceContext, PLANES);
   gpsi->BitsPixel     = NtGdiGetDeviceCaps(ScreenDeviceContext, BITSPIXEL);
   gpsi->BitCount      = gpsi->Planes * gpsi->BitsPixel;
   gpsi->dmLogPixels   = NtGdiGetDeviceCaps(ScreenDeviceContext, LOGPIXELSY);
   // Font is realized and this dc was previously set to internal DC_ATTR.
   gpsi->cxSysFontChar = IntGetCharDimensions(hSystemBM, &tmw, (DWORD*)&gpsi->cySysFontChar);
   gpsi->tmSysFont     = tmw;

   return TRUE;
}

VOID FASTCALL
IntEndDesktopGraphics(VOID)
{
   if (NULL != ScreenDeviceContext)
   {  // No need to allocate a new dcattr.
      GreSetDCOwner(ScreenDeviceContext, GDI_OBJ_HMGR_POWNED);
      GreDeleteObject(ScreenDeviceContext);
      ScreenDeviceContext = NULL;
   }
   IntHideDesktop(IntGetActiveDesktop());
   IntDestroyPrimarySurface();
}

HDC FASTCALL
IntGetScreenDC(VOID)
{
   return ScreenDeviceContext;
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * NtUserCreateWindowStation
 *
 * Creates a new window station.
 *
 * Parameters
 *    lpszWindowStationName
 *       Pointer to a null-terminated string specifying the name of the
 *       window station to be created. Window station names are
 *       case-insensitive and cannot contain backslash characters (\).
 *       Only members of the Administrators group are allowed to specify a
 *       name.
 *
 *    dwDesiredAccess
 *       Requested type of access
 *
 *    lpSecurity
 *       Security descriptor
 *
 *    Unknown3, Unknown4, Unknown5
 *       Unused
 *
 * Return Value
 *    If the function succeeds, the return value is a handle to the newly
 *    created window station. If the specified window station already
 *    exists, the function succeeds and returns a handle to the existing
 *    window station. If the function fails, the return value is NULL.
 *
 * Todo
 *    Correct the prototype to match the Windows one (with 7 parameters
 *    on Windows XP).
 *
 * Status
 *    @implemented
 */

HWINSTA APIENTRY
NtUserCreateWindowStation(
   POBJECT_ATTRIBUTES ObjectAttributes,
   ACCESS_MASK dwDesiredAccess,
   DWORD Unknown2,
   DWORD Unknown3,
   DWORD Unknown4,
   DWORD Unknown5,
   DWORD Unknown6)
{
   UNICODE_STRING WindowStationName;
   PWINSTATION_OBJECT WindowStationObject;
   HWINSTA WindowStation;
   NTSTATUS Status;

   Status = ObOpenObjectByName(
               ObjectAttributes,
               ExWindowStationObjectType,
               UserMode,
               NULL,
               dwDesiredAccess,
               NULL,
               (PVOID*)&WindowStation);

   if (NT_SUCCESS(Status))
   {
      return (HWINSTA)WindowStation;
   }


   /*
    * No existing window station found, try to create new one
    */

   /* Capture window station name */
   _SEH2_TRY
   {
      ProbeForRead( ObjectAttributes, sizeof(OBJECT_ATTRIBUTES), 1);
      Status = IntSafeCopyUnicodeString(&WindowStationName, ObjectAttributes->ObjectName);
   }
   _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
   {
      Status =_SEH2_GetExceptionCode();
   }
   _SEH2_END

   if (! NT_SUCCESS(Status))
   {
      ERR("Failed reading capturing window station name\n");
      SetLastNtError(Status);
      return NULL;
   }

   /* Create the window station object */
   Status = ObCreateObject(
               UserMode,
               ExWindowStationObjectType,
               ObjectAttributes,
               UserMode,
               NULL,
               sizeof(WINSTATION_OBJECT),
               0,
               0,
               (PVOID*)&WindowStationObject);

   if (!NT_SUCCESS(Status))
   {
      ExFreePoolWithTag(WindowStationName.Buffer, TAG_STRING);
      SetLastNtError(STATUS_INSUFFICIENT_RESOURCES);
      return 0;
   }

   Status = ObInsertObject(
               (PVOID)WindowStationObject,
               NULL,
               STANDARD_RIGHTS_REQUIRED,
               0,
               NULL,
               (PVOID*)&WindowStation);

   if (!NT_SUCCESS(Status))
   {
      ExFreePoolWithTag(WindowStationName.Buffer, TAG_STRING);
      SetLastNtError(STATUS_INSUFFICIENT_RESOURCES);
      ObDereferenceObject(WindowStationObject);
      return 0;
   }

   /* Initialize the window station */
   RtlZeroMemory(WindowStationObject, sizeof(WINSTATION_OBJECT));

   KeInitializeSpinLock(&WindowStationObject->Lock);
   InitializeListHead(&WindowStationObject->DesktopListHead);
   Status = RtlCreateAtomTable(37, &WindowStationObject->AtomTable);
   WindowStationObject->SystemMenuTemplate = (HANDLE)0;
   WindowStationObject->Name = WindowStationName;
   WindowStationObject->ScreenSaverRunning = FALSE;
   WindowStationObject->FlatMenu = FALSE;

   if (!IntSetupClipboard(WindowStationObject))
   {
       ERR("WindowStation: Error Setting up the clipboard!!!\n");
   }

   if (InputWindowStation == NULL)
   {
      InputWindowStation = WindowStationObject;

      InitCursorImpl();
   }

   return WindowStation;
}

/*
 * NtUserOpenWindowStation
 *
 * Opens an existing window station.
 *
 * Parameters
 *    lpszWindowStationName
 *       Name of the existing window station.
 *
 *    dwDesiredAccess
 *       Requested type of access.
 *
 * Return Value
 *    If the function succeeds, the return value is the handle to the
 *    specified window station. If the function fails, the return value
 *    is NULL.
 *
 * Remarks
 *    The returned handle can be closed with NtUserCloseWindowStation.
 *
 * Status
 *    @implemented
 */

HWINSTA APIENTRY
NtUserOpenWindowStation(
   POBJECT_ATTRIBUTES ObjectAttributes,
   ACCESS_MASK dwDesiredAccess)
{
   HWINSTA WindowStation;
   NTSTATUS Status;

   Status = ObOpenObjectByName(
               ObjectAttributes,
               ExWindowStationObjectType,
               UserMode,
               NULL,
               dwDesiredAccess,
               NULL,
               (PVOID*)&WindowStation);

   if (!NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      return 0;
   }

   return WindowStation;
}

/*
 * NtUserCloseWindowStation
 *
 * Closes a window station handle.
 *
 * Parameters
 *    hWinSta
 *       Handle to the window station.
 *
 * Return Value
 *    Status
 *
 * Remarks
 *    The window station handle can be created with NtUserCreateWindowStation
 *    or NtUserOpenWindowStation. Attemps to close a handle to the window
 *    station assigned to the calling process will fail.
 *
 * Status
 *    @implemented
 */

BOOL
APIENTRY
NtUserCloseWindowStation(
   HWINSTA hWinSta)
{
   PWINSTATION_OBJECT Object;
   NTSTATUS Status;

   TRACE("About to close window station handle (0x%X)\n", hWinSta);

	if (hWinSta == UserGetProcessWindowStation())
	{
		return FALSE;
	}

   Status = IntValidateWindowStationHandle(
               hWinSta,
               KernelMode,
               0,
               &Object);

   if (!NT_SUCCESS(Status))
   {
      TRACE("Validation of window station handle (0x%X) failed\n", hWinSta);
      return FALSE;
   }

   ObDereferenceObject(Object);

   TRACE("Closing window station handle (0x%X)\n", hWinSta);

   Status = ObCloseHandle(hWinSta, UserMode);
   if (!NT_SUCCESS(Status))
   {
      SetLastNtError(Status);
      return FALSE;
   }

   return TRUE;
}

/*
 * NtUserGetObjectInformation
 *
 * The NtUserGetObjectInformation function retrieves information about a
 * window station or desktop object.
 *
 * Parameters
 *    hObj
 *       Handle to the window station or desktop object for which to
 *       return information. This can be a handle of type HDESK or HWINSTA
 *       (for example, a handle returned by NtUserCreateWindowStation,
 *       NtUserOpenWindowStation, NtUserCreateDesktop, or NtUserOpenDesktop).
 *
 *    nIndex
 *       Specifies the object information to be retrieved.
 *
 *    pvInfo
 *       Pointer to a buffer to receive the object information.
 *
 *    nLength
 *       Specifies the size, in bytes, of the buffer pointed to by the
 *       pvInfo parameter.
 *
 *    lpnLengthNeeded
 *       Pointer to a variable receiving the number of bytes required to
 *       store the requested information. If this variable's value is
 *       greater than the value of the nLength parameter when the function
 *       returns, the function returns FALSE, and none of the information
 *       is copied to the pvInfo buffer. If the value of the variable pointed
 *       to by lpnLengthNeeded is less than or equal to the value of nLength,
 *       the entire information block is copied.
 *
 * Return Value
 *    If the function succeeds, the return value is nonzero. If the function
 *    fails, the return value is zero.
 *
 * Status
 *    @unimplemented
 */

BOOL APIENTRY
NtUserGetObjectInformation(
   HANDLE hObject,
   DWORD nIndex,
   PVOID pvInformation,
   DWORD nLength,
   PDWORD nLengthNeeded)
{
   PWINSTATION_OBJECT WinStaObject = NULL;
   PDESKTOP DesktopObject = NULL;
   NTSTATUS Status;
   PVOID pvData = NULL;
   DWORD nDataSize = 0;

   /* try windowstation */
   TRACE("Trying to open window station 0x%x\n", hObject);
   Status = IntValidateWindowStationHandle(
               hObject,
               UserMode,/*ExGetPreviousMode(),*/
               GENERIC_READ, /* FIXME: is this ok? */
               &WinStaObject);


   if (!NT_SUCCESS(Status) && Status != STATUS_OBJECT_TYPE_MISMATCH)
   {
      TRACE("Failed: 0x%x\n", Status);
      SetLastNtError(Status);
      return FALSE;
   }

   if (Status == STATUS_OBJECT_TYPE_MISMATCH)
   {
      /* try desktop */
      TRACE("Trying to open desktop 0x%x\n", hObject);
      Status = IntValidateDesktopHandle(
                  hObject,
                  UserMode,/*ExGetPreviousMode(),*/
                  GENERIC_READ, /* FIXME: is this ok? */
                  &DesktopObject);
      if (!NT_SUCCESS(Status))
      {
         TRACE("Failed: 0x%x\n", Status);
         SetLastNtError(Status);
         return FALSE;
      }
   }
   TRACE("WinSta or Desktop opened!!\n");

   /* get data */
   switch (nIndex)
   {
      case UOI_FLAGS:
         Status = STATUS_NOT_IMPLEMENTED;
         ERR("UOI_FLAGS unimplemented!\n");
         break;

      case UOI_NAME:
         if (WinStaObject != NULL)
         {
            pvData = ((PUNICODE_STRING)GET_DESKTOP_NAME(WinStaObject))->Buffer;
            nDataSize = ((PUNICODE_STRING)GET_DESKTOP_NAME(WinStaObject))->Length + 2;
            Status = STATUS_SUCCESS;
         }
         else if (DesktopObject != NULL)
         {
            pvData = ((PUNICODE_STRING)GET_DESKTOP_NAME(DesktopObject))->Buffer;
            nDataSize = ((PUNICODE_STRING)GET_DESKTOP_NAME(DesktopObject))->Length + 2;
            Status = STATUS_SUCCESS;
         }
         else
            Status = STATUS_INVALID_PARAMETER;
         break;

      case UOI_TYPE:
         if (WinStaObject != NULL)
         {
            pvData = L"WindowStation";
            nDataSize = (wcslen(pvData) + 1) * sizeof(WCHAR);
            Status = STATUS_SUCCESS;
         }
         else if (DesktopObject != NULL)
         {
            pvData = L"Desktop";
            nDataSize = (wcslen(pvData) + 1) * sizeof(WCHAR);
            Status = STATUS_SUCCESS;
         }
         else
            Status = STATUS_INVALID_PARAMETER;
         break;

      case UOI_USER_SID:
         Status = STATUS_NOT_IMPLEMENTED;
         ERR("UOI_USER_SID unimplemented!\n");
         break;

      default:
         Status = STATUS_INVALID_PARAMETER;
         break;
   }

   /* try to copy data to caller */
   if (Status == STATUS_SUCCESS)
   {
      TRACE("Trying to copy data to caller (len = %d, len needed = %d)\n", nLength, nDataSize);
      *nLengthNeeded = nDataSize;
      if (nLength >= nDataSize)
         Status = MmCopyToCaller(pvInformation, pvData, nDataSize);
      else
         Status = STATUS_BUFFER_TOO_SMALL;
   }

   /* release objects */
   if (WinStaObject != NULL)
      ObDereferenceObject(WinStaObject);
   if (DesktopObject != NULL)
      ObDereferenceObject(DesktopObject);

   SetLastNtError(Status);
   return NT_SUCCESS(Status);
}

/*
 * NtUserSetObjectInformation
 *
 * The NtUserSetObjectInformation function sets information about a
 * window station or desktop object.
 *
 * Parameters
 *    hObj
 *       Handle to the window station or desktop object for which to set
 *       object information. This value can be a handle of type HDESK or
 *       HWINSTA.
 *
 *    nIndex
 *       Specifies the object information to be set.
 *
 *    pvInfo
 *       Pointer to a buffer containing the object information.
 *
 *    nLength
 *       Specifies the size, in bytes, of the information contained in the
 *       buffer pointed to by pvInfo.
 *
 * Return Value
 *    If the function succeeds, the return value is nonzero. If the function
 *    fails the return value is zero.
 *
 * Status
 *    @unimplemented
 */

BOOL
APIENTRY
NtUserSetObjectInformation(
   HANDLE hObject,
   DWORD nIndex,
   PVOID pvInformation,
   DWORD nLength)
{
   /* FIXME: ZwQueryObject */
   /* FIXME: ZwSetInformationObject */
   SetLastNtError(STATUS_UNSUCCESSFUL);
   return FALSE;
}




HWINSTA FASTCALL
UserGetProcessWindowStation(VOID)
{
   NTSTATUS Status;
   PTHREADINFO pti;
   HWINSTA WinSta;

   if(PsGetCurrentProcess() != CsrProcess)
   {
      return PsGetCurrentProcess()->Win32WindowStation;
   }
   else
   {
      ERR("Should use ObFindHandleForObject\n");
      pti = PsGetCurrentThreadWin32Thread();
      Status = ObOpenObjectByPointer(pti->rpdesk->rpwinstaParent,
                                     0,
                                     NULL,
                                     WINSTA_ALL_ACCESS,
                                     ExWindowStationObjectType,
                                     UserMode,
                                     (PHANDLE) &WinSta);
      if (! NT_SUCCESS(Status))
      {
         SetLastNtError(Status);
         ERR("Unable to open handle for CSRSSs winsta, status 0x%08x\n",
                 Status);
         return NULL;
      }
      return WinSta;
   }
}


/*
 * NtUserGetProcessWindowStation
 *
 * Returns a handle to the current process window station.
 *
 * Return Value
 *    If the function succeeds, the return value is handle to the window
 *    station assigned to the current process. If the function fails, the
 *    return value is NULL.
 *
 * Status
 *    @implemented
 */

HWINSTA APIENTRY
NtUserGetProcessWindowStation(VOID)
{
   return UserGetProcessWindowStation();
}

PWINSTATION_OBJECT FASTCALL
IntGetWinStaObj(VOID)
{
   PWINSTATION_OBJECT WinStaObj;
   PTHREADINFO Win32Thread;
   PEPROCESS CurrentProcess;

   /*
    * just a temporary hack, this will be gone soon
    */

   Win32Thread = PsGetCurrentThreadWin32Thread();
   if(Win32Thread != NULL && Win32Thread->rpdesk != NULL)
   {
      WinStaObj = Win32Thread->rpdesk->rpwinstaParent;
      ObReferenceObjectByPointer(WinStaObj, KernelMode, ExWindowStationObjectType, 0);
   }
   else if((CurrentProcess = PsGetCurrentProcess()) != CsrProcess)
   {
      NTSTATUS Status = IntValidateWindowStationHandle(CurrentProcess->Win32WindowStation,
                        KernelMode,
                        0,
                        &WinStaObj);
      if(!NT_SUCCESS(Status))
      {
         SetLastNtError(Status);
         return NULL;
      }
   }
   else
   {
      WinStaObj = NULL;
   }

   return WinStaObj;
}

BOOL FASTCALL
UserSetProcessWindowStation(HWINSTA hWindowStation)
{
    PPROCESSINFO ppi;
    NTSTATUS Status;
    HWINSTA hwinstaOld;
    PWINSTATION_OBJECT NewWinSta = NULL, OldWinSta;

    ppi = PsGetCurrentProcessWin32Process();

    /* Reference the new window station */
    if(hWindowStation !=NULL)
    {
        Status = IntValidateWindowStationHandle( hWindowStation,
                                                 KernelMode,
                                                 0,
                                                 &NewWinSta);
       if (!NT_SUCCESS(Status))
       {
          TRACE("Validation of window station handle (0x%X) failed\n",
                 hWindowStation);
          SetLastNtError(Status);
          return FALSE;
       }
    }

   OldWinSta = ppi->prpwinsta;
   hwinstaOld = PsGetProcessWin32WindowStation(ppi->peProcess);

   /* Dereference the previous window station */
   if(OldWinSta != NULL)
   {
       ObDereferenceObject(OldWinSta);
   }

   /* Check if we have a stale handle (it should happen for console apps) */
   if(hwinstaOld != ppi->hwinsta)
   {
       ObCloseHandle(hwinstaOld, UserMode);
   }

   /*
    * FIXME - don't allow changing the window station if there are threads that are attached to desktops and own gui objects
    */

   PsSetProcessWindowStation(ppi->peProcess, hWindowStation);

   ppi->prpwinsta = NewWinSta;
   ppi->hwinsta = hWindowStation;

   return TRUE;
}

/*
 * NtUserSetProcessWindowStation
 *
 * Assigns a window station to the current process.
 *
 * Parameters
 *    hWinSta
 *       Handle to the window station.
 *
 * Return Value
 *    Status
 *
 * Status
 *    @implemented
 */

BOOL APIENTRY
NtUserSetProcessWindowStation(HWINSTA hWindowStation)
{
    BOOL ret;

    UserEnterExclusive();

    ret = UserSetProcessWindowStation(hWindowStation);

    UserLeave();

    return ret;
}

/*
 * NtUserLockWindowStation
 *
 * Locks switching desktops. Only the logon application is allowed to call this function.
 *
 * Status
 *    @implemented
 */

BOOL APIENTRY
NtUserLockWindowStation(HWINSTA hWindowStation)
{
   PWINSTATION_OBJECT Object;
   NTSTATUS Status;

   TRACE("About to set process window station with handle (0x%X)\n",
          hWindowStation);

   if(PsGetCurrentProcessWin32Process() != LogonProcess)
   {
      ERR("Unauthorized process attempted to lock the window station!\n");
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   Status = IntValidateWindowStationHandle(
               hWindowStation,
               KernelMode,
               0,
               &Object);
   if (!NT_SUCCESS(Status))
   {
      TRACE("Validation of window station handle (0x%X) failed\n",
             hWindowStation);
      SetLastNtError(Status);
      return FALSE;
   }

   Object->Flags |= WSS_LOCKED;

   ObDereferenceObject(Object);
   return TRUE;
}

/*
 * NtUserUnlockWindowStation
 *
 * Unlocks switching desktops. Only the logon application is allowed to call this function.
 *
 * Status
 *    @implemented
 */

BOOL APIENTRY
NtUserUnlockWindowStation(HWINSTA hWindowStation)
{
   PWINSTATION_OBJECT Object;
   NTSTATUS Status;
   BOOL Ret;

   TRACE("About to set process window station with handle (0x%X)\n",
          hWindowStation);

   if(PsGetCurrentProcessWin32Process() != LogonProcess)
   {
      ERR("Unauthorized process attempted to unlock the window station!\n");
      EngSetLastError(ERROR_ACCESS_DENIED);
      return FALSE;
   }

   Status = IntValidateWindowStationHandle(
               hWindowStation,
               KernelMode,
               0,
               &Object);
   if (!NT_SUCCESS(Status))
   {
      TRACE("Validation of window station handle (0x%X) failed\n",
             hWindowStation);
      SetLastNtError(Status);
      return FALSE;
   }

   Ret = (Object->Flags & WSS_LOCKED) == WSS_LOCKED;
   Object->Flags &= ~WSS_LOCKED;

   ObDereferenceObject(Object);
   return Ret;
}

static NTSTATUS FASTCALL
BuildWindowStationNameList(
   ULONG dwSize,
   PVOID lpBuffer,
   PULONG pRequiredSize)
{
   OBJECT_ATTRIBUTES ObjectAttributes;
   NTSTATUS Status;
   HANDLE DirectoryHandle;
   UNICODE_STRING DirectoryName = RTL_CONSTANT_STRING(WINSTA_ROOT_NAME);
   char InitialBuffer[256], *Buffer;
   ULONG Context, ReturnLength, BufferSize;
   DWORD EntryCount;
   POBJECT_DIRECTORY_INFORMATION DirEntry;
   WCHAR NullWchar;

   /*
    * Try to open the directory.
    */
   InitializeObjectAttributes(
      &ObjectAttributes,
      &DirectoryName,
      OBJ_CASE_INSENSITIVE,
      NULL,
      NULL);

   Status = ZwOpenDirectoryObject(
               &DirectoryHandle,
               DIRECTORY_QUERY,
               &ObjectAttributes);

   if (!NT_SUCCESS(Status))
   {
      return Status;
   }

   /* First try to query the directory using a fixed-size buffer */
   Context = 0;
   Buffer = NULL;
   Status = ZwQueryDirectoryObject(DirectoryHandle, InitialBuffer, sizeof(InitialBuffer),
                                   FALSE, TRUE, &Context, &ReturnLength);
   if (NT_SUCCESS(Status))
   {
      if (STATUS_NO_MORE_ENTRIES == ZwQueryDirectoryObject(DirectoryHandle, NULL, 0, FALSE,
            FALSE, &Context, NULL))
      {
         /* Our fixed-size buffer is large enough */
         Buffer = InitialBuffer;
      }
   }

   if (NULL == Buffer)
   {
      /* Need a larger buffer, check how large exactly */
      Status = ZwQueryDirectoryObject(DirectoryHandle, NULL, 0, FALSE, TRUE, &Context,
                                      &ReturnLength);
      if (STATUS_BUFFER_TOO_SMALL == Status)
      {
         BufferSize = ReturnLength;
         Buffer = ExAllocatePoolWithTag(PagedPool, BufferSize, TAG_WINSTA);
         if (NULL == Buffer)
         {
            ObDereferenceObject(DirectoryHandle);
            return STATUS_NO_MEMORY;
         }

         /* We should have a sufficiently large buffer now */
         Context = 0;
         Status = ZwQueryDirectoryObject(DirectoryHandle, Buffer, BufferSize,
                                         FALSE, TRUE, &Context, &ReturnLength);
         if (! NT_SUCCESS(Status) ||
               STATUS_NO_MORE_ENTRIES != ZwQueryDirectoryObject(DirectoryHandle, NULL, 0, FALSE,
                     FALSE, &Context, NULL))
         {
            /* Something went wrong, maybe someone added a directory entry? Just give up. */
            ExFreePoolWithTag(Buffer, TAG_WINSTA);
            ObDereferenceObject(DirectoryHandle);
            return NT_SUCCESS(Status) ? STATUS_INTERNAL_ERROR : Status;
         }
      }
   }

   ZwClose(DirectoryHandle);

   /*
    * Count the required size of buffer.
    */
   ReturnLength = sizeof(DWORD);
   EntryCount = 0;
   for (DirEntry = (POBJECT_DIRECTORY_INFORMATION) Buffer; 0 != DirEntry->Name.Length;
         DirEntry++)
   {
      ReturnLength += DirEntry->Name.Length + sizeof(WCHAR);
      EntryCount++;
   }
   TRACE("Required size: %d Entry count: %d\n", ReturnLength, EntryCount);
   if (NULL != pRequiredSize)
   {
      Status = MmCopyToCaller(pRequiredSize, &ReturnLength, sizeof(ULONG));
      if (! NT_SUCCESS(Status))
      {
         if (Buffer != InitialBuffer)
         {
            ExFreePoolWithTag(Buffer, TAG_WINSTA);
         }
         return STATUS_BUFFER_TOO_SMALL;
      }
   }

   /*
    * Check if the supplied buffer is large enough.
    */
   if (dwSize < ReturnLength)
   {
      if (Buffer != InitialBuffer)
      {
         ExFreePoolWithTag(Buffer, TAG_WINSTA);
      }
      return STATUS_BUFFER_TOO_SMALL;
   }

   /*
    * Generate the resulting buffer contents.
    */
   Status = MmCopyToCaller(lpBuffer, &EntryCount, sizeof(DWORD));
   if (! NT_SUCCESS(Status))
   {
      if (Buffer != InitialBuffer)
      {
         ExFreePool(Buffer);
      }
      return Status;
   }
   lpBuffer = (PVOID) ((PCHAR) lpBuffer + sizeof(DWORD));

   NullWchar = L'\0';
   for (DirEntry = (POBJECT_DIRECTORY_INFORMATION) Buffer; 0 != DirEntry->Name.Length;
         DirEntry++)
   {
      Status = MmCopyToCaller(lpBuffer, DirEntry->Name.Buffer, DirEntry->Name.Length);
      if (! NT_SUCCESS(Status))
      {
         if (Buffer != InitialBuffer)
         {
            ExFreePool(Buffer);
         }
         return Status;
      }
      lpBuffer = (PVOID) ((PCHAR) lpBuffer + DirEntry->Name.Length);
      Status = MmCopyToCaller(lpBuffer, &NullWchar, sizeof(WCHAR));
      if (! NT_SUCCESS(Status))
      {
         if (Buffer != InitialBuffer)
         {
            ExFreePool(Buffer);
         }
         return Status;
      }
      lpBuffer = (PVOID) ((PCHAR) lpBuffer + sizeof(WCHAR));
   }

   /*
    * Clean up
    */
   if (NULL != Buffer && Buffer != InitialBuffer)
   {
      ExFreePool(Buffer);
   }

   return STATUS_SUCCESS;
}

static NTSTATUS FASTCALL
BuildDesktopNameList(
   HWINSTA hWindowStation,
   ULONG dwSize,
   PVOID lpBuffer,
   PULONG pRequiredSize)
{
   NTSTATUS Status;
   PWINSTATION_OBJECT WindowStation;
   KIRQL OldLevel;
   PLIST_ENTRY DesktopEntry;
   PDESKTOP DesktopObject;
   DWORD EntryCount;
   ULONG ReturnLength;
   WCHAR NullWchar;

   Status = IntValidateWindowStationHandle(hWindowStation,
                                           KernelMode,
                                           0,
                                           &WindowStation);
   if (! NT_SUCCESS(Status))
   {
      return Status;
   }

   KeAcquireSpinLock(&WindowStation->Lock, &OldLevel);

   /*
    * Count the required size of buffer.
    */
   ReturnLength = sizeof(DWORD);
   EntryCount = 0;
   for (DesktopEntry = WindowStation->DesktopListHead.Flink;
         DesktopEntry != &WindowStation->DesktopListHead;
         DesktopEntry = DesktopEntry->Flink)
   {
      DesktopObject = CONTAINING_RECORD(DesktopEntry, DESKTOP, ListEntry);
      ReturnLength += ((PUNICODE_STRING)GET_DESKTOP_NAME(DesktopObject))->Length + sizeof(WCHAR);
      EntryCount++;
   }
   TRACE("Required size: %d Entry count: %d\n", ReturnLength, EntryCount);
   if (NULL != pRequiredSize)
   {
      Status = MmCopyToCaller(pRequiredSize, &ReturnLength, sizeof(ULONG));
      if (! NT_SUCCESS(Status))
      {
         KeReleaseSpinLock(&WindowStation->Lock, OldLevel);
         ObDereferenceObject(WindowStation);
         return STATUS_BUFFER_TOO_SMALL;
      }
   }

   /*
    * Check if the supplied buffer is large enough.
    */
   if (dwSize < ReturnLength)
   {
      KeReleaseSpinLock(&WindowStation->Lock, OldLevel);
      ObDereferenceObject(WindowStation);
      return STATUS_BUFFER_TOO_SMALL;
   }

   /*
    * Generate the resulting buffer contents.
    */
   Status = MmCopyToCaller(lpBuffer, &EntryCount, sizeof(DWORD));
   if (! NT_SUCCESS(Status))
   {
      KeReleaseSpinLock(&WindowStation->Lock, OldLevel);
      ObDereferenceObject(WindowStation);
      return Status;
   }
   lpBuffer = (PVOID) ((PCHAR) lpBuffer + sizeof(DWORD));

   NullWchar = L'\0';
   for (DesktopEntry = WindowStation->DesktopListHead.Flink;
         DesktopEntry != &WindowStation->DesktopListHead;
         DesktopEntry = DesktopEntry->Flink)
   {
      DesktopObject = CONTAINING_RECORD(DesktopEntry, DESKTOP, ListEntry);
      Status = MmCopyToCaller(lpBuffer, ((PUNICODE_STRING)GET_DESKTOP_NAME(DesktopObject))->Buffer, ((PUNICODE_STRING)GET_DESKTOP_NAME(DesktopObject))->Length);
      if (! NT_SUCCESS(Status))
      {
         KeReleaseSpinLock(&WindowStation->Lock, OldLevel);
         ObDereferenceObject(WindowStation);
         return Status;
      }
      lpBuffer = (PVOID) ((PCHAR) lpBuffer + ((PUNICODE_STRING)GET_DESKTOP_NAME(DesktopObject))->Length);
      Status = MmCopyToCaller(lpBuffer, &NullWchar, sizeof(WCHAR));
      if (! NT_SUCCESS(Status))
      {
         KeReleaseSpinLock(&WindowStation->Lock, OldLevel);
         ObDereferenceObject(WindowStation);
         return Status;
      }
      lpBuffer = (PVOID) ((PCHAR) lpBuffer + sizeof(WCHAR));
   }

   /*
    * Clean up
    */
   KeReleaseSpinLock(&WindowStation->Lock, OldLevel);
   ObDereferenceObject(WindowStation);

   return STATUS_SUCCESS;
}

/*
 * NtUserBuildNameList
 *
 * Function used for enumeration of desktops or window stations.
 *
 * Parameters
 *    hWinSta
 *       For enumeration of window stations this parameter must be set to
 *       zero. Otherwise it's handle for window station.
 *
 *    dwSize
 *       Size of buffer passed by caller.
 *
 *    lpBuffer
 *       Buffer passed by caller. If the function succedes, the buffer is
 *       filled with window station/desktop count (in first DWORD) and
 *       NULL-terminated window station/desktop names.
 *
 *    pRequiredSize
 *       If the function suceedes, this is the number of bytes copied.
 *       Otherwise it's size of buffer needed for function to succeed.
 *
 * Status
 *    @implemented
 */

NTSTATUS APIENTRY
NtUserBuildNameList(
   HWINSTA hWindowStation,
   ULONG dwSize,
   PVOID lpBuffer,
   PULONG pRequiredSize)
{
   /* The WindowStation name list and desktop name list are build in completely
      different ways. Call the appropriate function */
   return NULL == hWindowStation ? BuildWindowStationNameList(dwSize, lpBuffer, pRequiredSize) :
          BuildDesktopNameList(hWindowStation, dwSize, lpBuffer, pRequiredSize);
}

/*
 * @implemented
 */
BOOL APIENTRY
NtUserSetLogonNotifyWindow(HWND hWnd)
{
    if(LogonProcess != PsGetCurrentProcessWin32Process())
    {
        return FALSE;
    }

    if(!IntIsWindow(hWnd))
    {
        return FALSE;
    }

    hwndSAS = hWnd;

    return TRUE;
}

/* EOF */
