
/*
 * PROJECT:         Odyssey Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            subsystems/win32/win32k/ntuser/kbdlayout.c
 * PURPOSE:         Keyboard layout management
 * COPYRIGHT:       Copyright 2007 Saveliy Tretiakov
 *                  Copyright 2008 Colin Finck
 *
 */


/* INCLUDES ******************************************************************/

#include <win32k.h>

DBG_DEFAULT_CHANNEL(UserKbdLayout);

PKBL KBLList = NULL; // Keyboard layout list.

typedef PVOID (*KbdLayerDescriptor)(VOID);
NTSTATUS APIENTRY LdrGetProcedureAddress(PVOID module,
                                        PANSI_STRING import_name,
                                        DWORD flags,
                                        PVOID *func_addr);



/* PRIVATE FUNCTIONS ******************************************************/


/*
 * Utility function to read a value from the registry more easily.
 *
 * IN  PUNICODE_STRING KeyName       -> Name of key to open
 * IN  PUNICODE_STRING ValueName     -> Name of value to open
 * OUT PUNICODE_STRING ReturnedValue -> String contained in registry
 *
 * Returns NTSTATUS
 */

static NTSTATUS APIENTRY ReadRegistryValue( PUNICODE_STRING KeyName,
      PUNICODE_STRING ValueName,
      PUNICODE_STRING ReturnedValue )
{
   NTSTATUS Status;
   HANDLE KeyHandle;
   OBJECT_ATTRIBUTES KeyAttributes;
   PKEY_VALUE_PARTIAL_INFORMATION KeyValuePartialInfo;
   ULONG Length = 0;
   ULONG ResLength = 0;
   PWCHAR ReturnBuffer;

   InitializeObjectAttributes(&KeyAttributes, KeyName, OBJ_CASE_INSENSITIVE,
                              NULL, NULL);
   Status = ZwOpenKey(&KeyHandle, KEY_READ, &KeyAttributes);
   if( !NT_SUCCESS(Status) )
   {
      return Status;
   }

   Status = ZwQueryValueKey(KeyHandle, ValueName, KeyValuePartialInformation,
                            0,
                            0,
                            &ResLength);

   if( Status != STATUS_BUFFER_TOO_SMALL )
   {
      NtClose(KeyHandle);
      return Status;
   }

   ResLength += sizeof( *KeyValuePartialInfo );
   KeyValuePartialInfo =
      ExAllocatePoolWithTag(PagedPool, ResLength, TAG_STRING);
   Length = ResLength;

   if( !KeyValuePartialInfo )
   {
      NtClose(KeyHandle);
      return STATUS_NO_MEMORY;
   }

   Status = ZwQueryValueKey(KeyHandle,
      ValueName,
      KeyValuePartialInformation,
      (PVOID)KeyValuePartialInfo,
      Length,
      &ResLength);

   if( !NT_SUCCESS(Status) )
   {
      NtClose(KeyHandle);
      ExFreePoolWithTag(KeyValuePartialInfo, TAG_STRING);
      return Status;
   }

   /* At this point, KeyValuePartialInfo->Data contains the key data */
   ReturnBuffer = ExAllocatePoolWithTag(PagedPool,
      KeyValuePartialInfo->DataLength,
      TAG_STRING);

   if(!ReturnBuffer)
   {
      NtClose(KeyHandle);
      ExFreePoolWithTag(KeyValuePartialInfo, TAG_STRING);
      return STATUS_NO_MEMORY;
   }

   RtlCopyMemory(ReturnBuffer,
      KeyValuePartialInfo->Data,
      KeyValuePartialInfo->DataLength);
   RtlInitUnicodeString(ReturnedValue, ReturnBuffer);

   ExFreePoolWithTag(KeyValuePartialInfo, TAG_STRING);
   NtClose(KeyHandle);

   return Status;
}

static BOOL UserLoadKbdDll(WCHAR *wsKLID,
   HANDLE *phModule,
   PKBDTABLES *pKbdTables)
{
   NTSTATUS Status;
   KbdLayerDescriptor layerDescGetFn;
   ANSI_STRING kbdProcedureName;
   UNICODE_STRING LayoutKeyName;
   UNICODE_STRING LayoutValueName;
   UNICODE_STRING LayoutFile;
   UNICODE_STRING FullLayoutPath;
   UNICODE_STRING klid;
   WCHAR LayoutPathBuffer[MAX_PATH] = L"\\SystemRoot\\System32\\";
   WCHAR KeyNameBuffer[MAX_PATH] = L"\\REGISTRY\\Machine\\SYSTEM\\"
                   L"CurrentControlSet\\Control\\Keyboard Layouts\\";

   RtlInitUnicodeString(&klid, wsKLID);
   RtlInitUnicodeString(&LayoutValueName,L"Layout File");
   RtlInitUnicodeString(&LayoutKeyName, KeyNameBuffer);
   LayoutKeyName.MaximumLength = sizeof(KeyNameBuffer);

   RtlAppendUnicodeStringToString(&LayoutKeyName, &klid);
   Status = ReadRegistryValue(&LayoutKeyName, &LayoutValueName, &LayoutFile);

   if(!NT_SUCCESS(Status))
   {
      TRACE("Can't get layout filename for %wZ. (%08lx)\n", klid, Status);
      return FALSE;
   }

   TRACE("Read registry and got %wZ\n", &LayoutFile);
   RtlInitUnicodeString(&FullLayoutPath, LayoutPathBuffer);
   FullLayoutPath.MaximumLength = sizeof(LayoutPathBuffer);
   RtlAppendUnicodeStringToString(&FullLayoutPath, &LayoutFile);
   TRACE("Loading Keyboard DLL %wZ\n", &FullLayoutPath);
   ExFreePoolWithTag(LayoutFile.Buffer, TAG_STRING);

   *phModule = EngLoadImage(FullLayoutPath.Buffer);

   if(*phModule)
   {
      TRACE("Loaded %wZ\n", &FullLayoutPath);

      RtlInitAnsiString( &kbdProcedureName, "KbdLayerDescriptor" );
      layerDescGetFn = EngFindImageProcAddress(*phModule, "KbdLayerDescriptor");

      if(layerDescGetFn)
      {
         *pKbdTables = layerDescGetFn();
      }
      else
      {
         ERR("Error: %wZ has no KbdLayerDescriptor()\n", &FullLayoutPath);
      }

      if(!layerDescGetFn || !*pKbdTables)
      {
         ERR("Failed to load the keyboard layout.\n");
         EngUnloadImage(*phModule);
         return FALSE;
      }
   }
   else
   {
      ERR("Failed to load dll %wZ\n", &FullLayoutPath);
      return FALSE;
   }

   return TRUE;
}

static PKBL UserLoadDllAndCreateKbl(DWORD LocaleId)
{
   PKBL NewKbl;
   ULONG hKl;
   LANGID langid;

   NewKbl = ExAllocatePoolWithTag(PagedPool, sizeof(KBL), USERTAG_KBDLAYOUT);

   if(!NewKbl)
   {
      ERR("%s: Can't allocate memory!\n", __FUNCTION__);
      return NULL;
   }

   swprintf(NewKbl->Name, L"%08lx", LocaleId);

   if(!UserLoadKbdDll(NewKbl->Name, &NewKbl->hModule, &NewKbl->KBTables))
   {
      TRACE("%s: failed to load %x dll!\n", __FUNCTION__, LocaleId);
      ExFreePoolWithTag(NewKbl, USERTAG_KBDLAYOUT);
      return NULL;
   }

   /* Microsoft Office expects this value to be something specific
    * for Japanese and Korean Windows with an IME the value is 0xe001
    * We should probably check to see if an IME exists and if so then
    * set this word properly.
    */
   langid = PRIMARYLANGID(LANGIDFROMLCID(LocaleId));
   hKl = LocaleId;

   if (langid == LANG_CHINESE || langid == LANG_JAPANESE || langid == LANG_KOREAN)
      hKl |= 0xe001 << 16; /* FIXME */
   else hKl |= hKl << 16;

   NewKbl->hkl = (HKL)(ULONG_PTR) hKl;
   NewKbl->klid = LocaleId;
   NewKbl->Flags = 0;
   NewKbl->RefCount = 0;

   return NewKbl;
}

BOOL UserInitDefaultKeyboardLayout()
{
   LCID LocaleId;
   NTSTATUS Status;

   Status = ZwQueryDefaultLocale(FALSE, &LocaleId);
   if (!NT_SUCCESS(Status))
   {
      ERR("Could not get default locale (%08lx).\n", Status);
   }
   else
   {
      TRACE("DefaultLocale = %08lx\n", LocaleId);
   }

   if(!NT_SUCCESS(Status) || !(KBLList = UserLoadDllAndCreateKbl(LocaleId)))
   {
      ERR("Trying to load US Keyboard Layout.\n");
      LocaleId = 0x409;

      if(!(KBLList = UserLoadDllAndCreateKbl(LocaleId)))
      {
         ERR("Failed to load any Keyboard Layout\n");
         return FALSE;
      }
   }

   KBLList->Flags |= KBL_PRELOAD;

   InitializeListHead(&KBLList->List);
   return TRUE;
}

PKBL W32kGetDefaultKeyLayout(VOID)
{
   const WCHAR szKeyboardLayoutPath[] = L"\\Keyboard Layout\\Preload";
   const WCHAR szDefaultUserPath[] = L"\\REGISTRY\\USER\\.DEFAULT";

   HANDLE KeyHandle;
   LCID LayoutLocaleId = 0;
   NTSTATUS Status;
   OBJECT_ATTRIBUTES KeyAttributes;
   PKBL pKbl;
   UNICODE_STRING CurrentUserPath;
   UNICODE_STRING FullKeyboardLayoutPath;
   UNICODE_STRING LayoutValueName;
   UNICODE_STRING LayoutLocaleIdString;
   WCHAR wszBuffer[MAX_PATH];

   // Get the path to HKEY_CURRENT_USER
   Status = RtlFormatCurrentUserKeyPath(&CurrentUserPath);

   if( NT_SUCCESS(Status) )
   {
      // FIXME: Is this 100% correct?
      // We're called very early, so HKEY_CURRENT_USER might not be available yet. Check this first.
      InitializeObjectAttributes(&KeyAttributes, &CurrentUserPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
      Status = ZwOpenKey(&KeyHandle, KEY_READ, &KeyAttributes);

      if(Status == STATUS_OBJECT_NAME_NOT_FOUND)
      {
         // It is not available, so read it from HKEY_USERS\.DEFAULT
         RtlCopyMemory(wszBuffer, szDefaultUserPath, sizeof(szDefaultUserPath));
      }
      else
      {
         // The path is available
         ZwClose(KeyHandle);
         RtlCopyMemory(wszBuffer, CurrentUserPath.Buffer, CurrentUserPath.MaximumLength);
      }

      // Build the full path
      RtlInitUnicodeString(&FullKeyboardLayoutPath, wszBuffer);
      FullKeyboardLayoutPath.MaximumLength = MAX_PATH;

      Status = RtlAppendUnicodeToString(&FullKeyboardLayoutPath, szKeyboardLayoutPath);

      if( NT_SUCCESS(Status) )
      {
         // Return the first keyboard layout listed there
         RtlInitUnicodeString(&LayoutValueName, L"1");

         Status = ReadRegistryValue(&FullKeyboardLayoutPath, &LayoutValueName, &LayoutLocaleIdString);

         if( NT_SUCCESS(Status) )
         {
            RtlUnicodeStringToInteger(&LayoutLocaleIdString, 16, &LayoutLocaleId);
            ExFreePoolWithTag(LayoutLocaleIdString.Buffer, TAG_STRING);
         }
         else
            ERR("ReadRegistryValue failed! (%08lx).\n", Status);
      }
      else
         ERR("RtlAppendUnicodeToString failed! (%08lx)\n", Status);

      RtlFreeUnicodeString(&CurrentUserPath);
   }
   else
      ERR("RtlFormatCurrentUserKeyPath failed! (%08lx)\n", Status);

   if(!LayoutLocaleId)
   {
      ERR("Assuming default locale for the keyboard layout (0x409 - US)\n");
      LayoutLocaleId = 0x409;
   }

   pKbl = KBLList;
   do
   {
      if(pKbl->klid == LayoutLocaleId)
      {
         return pKbl;
      }

      pKbl = (PKBL) pKbl->List.Flink;
   } while(pKbl != KBLList);

   TRACE("Loading new default keyboard layout.\n");
   pKbl = UserLoadDllAndCreateKbl(LayoutLocaleId);

   if(!pKbl)
   {
      TRACE("Failed to load %x!!! Returning any availableKL.\n", LayoutLocaleId);
      return KBLList;
   }

   InsertTailList(&KBLList->List, &pKbl->List);
   return pKbl;
}

PKBL UserHklToKbl(HKL hKl)
{
   PKBL pKbl = KBLList;
   do
   {
      if(pKbl->hkl == hKl) return pKbl;
      pKbl = (PKBL) pKbl->List.Flink;
   } while(pKbl != KBLList);

   return NULL;
}

BOOL UserUnloadKbl(PKBL pKbl)
{
   /* According to msdn, UnloadKeyboardLayout can fail
      if the keyboard layout identifier was preloaded. */

   if(pKbl->Flags & KBL_PRELOAD)
   {
      ERR("Attempted to unload preloaded keyboard layout.\n");
      return FALSE;
   }

   if(pKbl->RefCount > 0)
   {
      /* Layout is used by other threads.
         Mark it as unloaded and don't do anything else. */
      pKbl->Flags |= KBL_UNLOAD;
   }
   else
   {
      //Unload the layout
      EngUnloadImage(pKbl->hModule);
      RemoveEntryList(&pKbl->List);
      ExFreePoolWithTag(pKbl, USERTAG_KBDLAYOUT);
   }

   return TRUE;
}

static PKBL co_UserActivateKbl(PTHREADINFO w32Thread, PKBL pKbl, UINT Flags)
{
   PKBL Prev;

   Prev = w32Thread->KeyboardLayout;
   Prev->RefCount--;
   w32Thread->KeyboardLayout = pKbl;
   pKbl->RefCount++;

   if(Flags & KLF_SETFORPROCESS)
   {
      //FIXME

   }

   if(Prev->Flags & KBL_UNLOAD && Prev->RefCount == 0)
   {
      UserUnloadKbl(Prev);
   }

   // Send WM_INPUTLANGCHANGE to thread's focus window
   co_IntSendMessage(w32Thread->MessageQueue->FocusWindow,
      WM_INPUTLANGCHANGE,
      0, // FIXME: put charset here (what is this?)
      (LPARAM)pKbl->hkl); //klid

   return Prev;
}

/* EXPORTS *******************************************************************/

HKL FASTCALL
UserGetKeyboardLayout(
   DWORD dwThreadId)
{
   NTSTATUS Status;
   PETHREAD Thread;
   PTHREADINFO W32Thread;
   HKL Ret;

   if(!dwThreadId)
   {
      W32Thread = PsGetCurrentThreadWin32Thread();
      return W32Thread->KeyboardLayout->hkl;
   }

   Status = PsLookupThreadByThreadId((HANDLE)(DWORD_PTR)dwThreadId, &Thread);
   if(!NT_SUCCESS(Status))
   {
      EngSetLastError(ERROR_INVALID_PARAMETER);
      return NULL;
   }

   W32Thread = PsGetThreadWin32Thread(Thread);
   Ret = W32Thread->KeyboardLayout->hkl;
   ObDereferenceObject(Thread);
   return Ret;
}

UINT
APIENTRY
NtUserGetKeyboardLayoutList(
   INT nItems,
   HKL* pHklBuff)
{
   UINT Ret = 0;
   PKBL pKbl;

   UserEnterShared();
   pKbl = KBLList;

   if(nItems == 0)
   {
      do
      {
         Ret++;
         pKbl = (PKBL) pKbl->List.Flink;
      } while(pKbl != KBLList);
   }
   else
   {
      _SEH2_TRY
      {
         ProbeForWrite(pHklBuff, nItems*sizeof(HKL), 4);

         while(Ret < nItems)
         {
            if(!(pKbl->Flags & KBL_UNLOAD))
            {
               pHklBuff[Ret] = pKbl->hkl;
               Ret++;
               pKbl = (PKBL) pKbl->List.Flink;
               if(pKbl == KBLList) break;
            }
         }

      }
      _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
      {
         SetLastNtError(_SEH2_GetExceptionCode());
         Ret = 0;
      }
      _SEH2_END;
   }

   UserLeave();
   return Ret;
}

BOOL
APIENTRY
NtUserGetKeyboardLayoutName(
   LPWSTR lpszName)
{
   BOOL ret = FALSE;
   PKBL pKbl;
   PTHREADINFO pti;

   UserEnterShared();

   _SEH2_TRY
   {
      ProbeForWrite(lpszName, KL_NAMELENGTH*sizeof(WCHAR), 1);
      pti = PsGetCurrentThreadWin32Thread();
      pKbl = pti->KeyboardLayout;
      RtlCopyMemory(lpszName,  pKbl->Name, KL_NAMELENGTH*sizeof(WCHAR));
      ret = TRUE;
   }
   _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
   {
      SetLastNtError(_SEH2_GetExceptionCode());
      ret = FALSE;
   }
   _SEH2_END;

   UserLeave();
   return ret;
}


HKL
APIENTRY
NtUserLoadKeyboardLayoutEx(
   IN HANDLE Handle,
   IN DWORD offTable,
   IN PUNICODE_STRING puszKeyboardName,
   IN HKL hKL,
   IN PUNICODE_STRING puszKLID,
   IN DWORD dwKLID,
   IN UINT Flags)
{
   HKL Ret = NULL;
   PKBL pKbl = NULL, Cur;

   UserEnterExclusive();

   //Let's see if layout was already loaded.
   Cur = KBLList;
   do
   {
      if(Cur->klid == dwKLID)
      {
         pKbl = Cur;
         pKbl->Flags &= ~KBL_UNLOAD;
         break;
      }

      Cur = (PKBL) Cur->List.Flink;
   } while(Cur != KBLList);

   //It wasn't, so load it.
   if(!pKbl)
   {
      pKbl = UserLoadDllAndCreateKbl(dwKLID);

      if(!pKbl)
      {
         goto the_end;
      }

      InsertTailList(&KBLList->List, &pKbl->List);
   }

   if(Flags & KLF_REORDER) KBLList = pKbl;

   if(Flags & KLF_ACTIVATE)
   {
      co_UserActivateKbl(PsGetCurrentThreadWin32Thread(), pKbl, Flags);
   }

   Ret = pKbl->hkl;

   //FIXME: KLF_NOTELLSHELL
   //       KLF_REPLACELANG
   //       KLF_SUBSTITUTE_OK

the_end:
   UserLeave();
   return Ret;
}

HKL
APIENTRY
NtUserActivateKeyboardLayout(
   HKL hKl,
   ULONG Flags)
{
   PKBL pKbl;
   HKL Ret = NULL;
   PTHREADINFO pWThread;

   UserEnterExclusive();

   pWThread = PsGetCurrentThreadWin32Thread();

   if(pWThread->KeyboardLayout->hkl == hKl)
   {
      Ret = hKl;
      goto the_end;
   }

   if(hKl == (HKL)HKL_NEXT)
   {
      pKbl = (PKBL)pWThread->KeyboardLayout->List.Flink;
   }
   else if(hKl == (HKL)HKL_PREV)
   {
      pKbl = (PKBL)pWThread->KeyboardLayout->List.Blink;
   }
   else pKbl = UserHklToKbl(hKl);

   //FIXME:  KLF_RESET, KLF_SHIFTLOCK

   if(pKbl)
   {
      if(Flags & KLF_REORDER)
         KBLList = pKbl;

      if(pKbl == pWThread->KeyboardLayout)
      {
         Ret = pKbl->hkl;
      }
      else
      {
         pKbl = co_UserActivateKbl(pWThread, pKbl, Flags);
         Ret = pKbl->hkl;
      }
   }
   else
   {
      ERR("%s: Invalid HKL %x!\n", __FUNCTION__, hKl);
   }

the_end:
   UserLeave();
   return Ret;
}

BOOL
APIENTRY
NtUserUnloadKeyboardLayout(
   HKL hKl)
{
   PKBL pKbl;
   BOOL Ret = FALSE;

   UserEnterExclusive();

   if((pKbl = UserHklToKbl(hKl)))
   {
      Ret = UserUnloadKbl(pKbl);
   }
   else
   {
      ERR("%s: Invalid HKL %x!\n", __FUNCTION__, hKl);
   }

   UserLeave();
   return Ret;
}

/* EOF */
