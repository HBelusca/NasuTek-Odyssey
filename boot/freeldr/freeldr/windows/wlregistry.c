/*
 * PROJECT:         EFI Windows Loader
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            freeldr/winldr/wlregistry.c
 * PURPOSE:         Registry support functions
 * PROGRAMMERS:     Aleksey Bragin (aleksey@odyssey.org)
 */

/* INCLUDES ***************************************************************/

#include <freeldr.h>
#include <debug.h>

DBG_DEFAULT_CHANNEL(WINDOWS);

// The only global var here, used to mark mem pages as NLS in WinLdrSetupMemoryLayout()
ULONG TotalNLSSize = 0;

BOOLEAN WinLdrGetNLSNames(LPSTR AnsiName,
                          LPSTR OemName,
                          LPSTR LangName);

BOOLEAN
WinLdrLoadNLSData(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                  IN LPCSTR DirectoryPath,
                  IN LPCSTR AnsiFileName,
                  IN LPCSTR OemFileName,
                  IN LPCSTR LanguageFileName);

VOID
WinLdrScanRegistry(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                   IN LPCSTR DirectoryPath);

BOOLEAN
WinLdrAddDriverToList(LIST_ENTRY *BootDriverListHead,
                      LPWSTR RegistryPath,
                      LPWSTR ImagePath,
                      LPWSTR ServiceName);

/* FUNCTIONS **************************************************************/

BOOLEAN
WinLdrLoadSystemHive(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                     IN LPCSTR DirectoryPath,
                     IN LPCSTR HiveName)
{
	ULONG FileId;
	CHAR FullHiveName[256];
	LONG Status;
	FILEINFORMATION FileInfo;
	ULONG HiveFileSize;
	ULONG_PTR HiveDataPhysical;
	PVOID HiveDataVirtual;
	ULONG BytesRead;
	LPCWSTR FsService;

	/* Concatenate path and filename to get the full name */
	strcpy(FullHiveName, DirectoryPath);
	strcat(FullHiveName, HiveName);
	//Print(L"Loading %s...\n", FullHiveName);
	Status = ArcOpen(FullHiveName, OpenReadOnly, &FileId);

	if (Status != ESUCCESS)
	{
		UiMessageBox("Opening hive file failed!");
		return FALSE;
	}

	/* Get the file length */
	Status = ArcGetFileInformation(FileId, &FileInfo);

	if (Status != ESUCCESS)
	{
		ArcClose(FileId);
		UiMessageBox("Hive file has 0 size!");
		return FALSE;
	}
	HiveFileSize = FileInfo.EndingAddress.LowPart;

	/* Round up the size to page boundary and alloc memory */
	HiveDataPhysical = (ULONG_PTR)MmAllocateMemoryWithType(
		MM_SIZE_TO_PAGES(HiveFileSize + MM_PAGE_SIZE - 1) << MM_PAGE_SHIFT,
		LoaderRegistryData);

	if (HiveDataPhysical == 0)
	{
		ArcClose(FileId);
		UiMessageBox("Unable to alloc memory for a hive!");
		return FALSE;
	}

	/* Convert address to virtual */
    HiveDataVirtual = PaToVa((PVOID)HiveDataPhysical);

	/* Fill LoaderBlock's entries */
	LoaderBlock->RegistryLength = HiveFileSize;
	LoaderBlock->RegistryBase = HiveDataVirtual;

	/* Finally read from file to the memory */
	Status = ArcRead(FileId, (PVOID)HiveDataPhysical, HiveFileSize, &BytesRead);
	if (Status != ESUCCESS)
	{
		ArcClose(FileId);
		UiMessageBox("Unable to read from hive file!");
		return FALSE;
	}

	// Add boot filesystem driver to the list
	FsService = FsGetServiceName(FileId);
	if (FsService)
	{
		TRACE("  Adding filesystem service %S\n", FsService);
		Status = WinLdrAddDriverToList(&LoaderBlock->BootDriverListHead,
			L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\",
			NULL,
			(LPWSTR)FsService);
		if (!Status)
			TRACE(" Failed to add filesystem service\n");
	}
	else
	{
		TRACE("  No required filesystem service\n");
	}

	ArcClose(FileId);
	return TRUE;
}

BOOLEAN WinLdrInitSystemHive(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                             IN LPCSTR DirectoryPath)
{
	CHAR SearchPath[1024];
	BOOLEAN Status;

	// There is a simple logic here: try to load usual hive (system), if it
	// fails, then give system.alt a try, and finally try a system.sav

	// FIXME: For now we only try system
	strcpy(SearchPath, DirectoryPath);
	strcat(SearchPath, "SYSTEM32\\CONFIG\\");
	Status = WinLdrLoadSystemHive(LoaderBlock, SearchPath, "SYSTEM");

	// Fail if failed...
	if (!Status)
		return FALSE;

	// Initialize in-memory registry
	RegInitializeRegistry();

	// Import what was loaded
	Status = RegImportBinaryHive((PCHAR)VaToPa(LoaderBlock->RegistryBase), LoaderBlock->RegistryLength);
	if (!Status)
	{
		UiMessageBox("Importing binary hive failed!");
		return FALSE;
	}

	// Initialize the 'CurrentControlSet' link
	if (RegInitCurrentControlSet(FALSE) != ERROR_SUCCESS)
	{
		UiMessageBox("Initializing CurrentControlSet link failed!");
		return FALSE;
	}

	return TRUE;
}

BOOLEAN WinLdrScanSystemHive(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                             IN LPCSTR DirectoryPath)
{
	CHAR SearchPath[1024];
	CHAR AnsiName[256], OemName[256], LangName[256];
	BOOLEAN Status;

	// Scan registry and prepare boot drivers list
	WinLdrScanRegistry(LoaderBlock, DirectoryPath);

	// Get names of NLS files
	Status = WinLdrGetNLSNames(AnsiName, OemName, LangName);
	if (!Status)
	{
		UiMessageBox("Getting NLS names from registry failed!");
		return FALSE;
	}

	TRACE("NLS data %s %s %s\n", AnsiName, OemName, LangName);

	// Load NLS data
	strcpy(SearchPath, DirectoryPath);
	strcat(SearchPath, "SYSTEM32\\");
	Status = WinLdrLoadNLSData(LoaderBlock, SearchPath, AnsiName, OemName, LangName);
	TRACE("NLS data loaded with status %d\n", Status);

	/* TODO: Load OEM HAL font */


	return TRUE;
}


/* PRIVATE FUNCTIONS ******************************************************/

// Queries registry for those three file names
BOOLEAN WinLdrGetNLSNames(LPSTR AnsiName,
                          LPSTR OemName,
                          LPSTR LangName)
{
	LONG rc = ERROR_SUCCESS;
	FRLDRHKEY hKey;
	WCHAR szIdBuffer[80];
	WCHAR NameBuffer[80];
	ULONG BufferSize;

	/* open the codepage key */
	rc = RegOpenKey(NULL,
		L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\NLS\\CodePage",
		&hKey);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "Couldn't open CodePage registry key");
		return FALSE;
	}

	/* get ANSI codepage */
	BufferSize = sizeof(szIdBuffer);
	rc = RegQueryValue(hKey, L"ACP", NULL, (PUCHAR)szIdBuffer, &BufferSize);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "Couldn't get ACP NLS setting");
		return FALSE;
	}

	BufferSize = sizeof(NameBuffer);
	rc = RegQueryValue(hKey, szIdBuffer, NULL, (PUCHAR)NameBuffer, &BufferSize);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "ACP NLS Setting exists, but isn't readable");
		return FALSE;
	}
	sprintf(AnsiName, "%S", NameBuffer);

	/* get OEM codepage */
	BufferSize = sizeof(szIdBuffer);
	rc = RegQueryValue(hKey, L"OEMCP", NULL, (PUCHAR)szIdBuffer, &BufferSize);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "Couldn't get OEMCP NLS setting");
		return FALSE;
	}

	BufferSize = sizeof(NameBuffer);
	rc = RegQueryValue(hKey, szIdBuffer, NULL, (PUCHAR)NameBuffer, &BufferSize);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "OEMCP NLS setting exists, but isn't readable");
		return FALSE;
	}
	sprintf(OemName, "%S", NameBuffer);

	/* open the language key */
	rc = RegOpenKey(NULL,
		L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\NLS\\Language",
		&hKey);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "Couldn't open Language registry key");
		return FALSE;
	}

	/* get the Unicode case table */
	BufferSize = sizeof(szIdBuffer);
	rc = RegQueryValue(hKey, L"Default", NULL, (PUCHAR)szIdBuffer, &BufferSize);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "Couldn't get Language Default setting");
		return FALSE;
	}

	BufferSize = sizeof(NameBuffer);
	rc = RegQueryValue(hKey, szIdBuffer, NULL, (PUCHAR)NameBuffer, &BufferSize);
	if (rc != ERROR_SUCCESS)
	{
		//strcpy(szErrorOut, "Language Default setting exists, but isn't readable");
		return FALSE;
	}
	sprintf(LangName, "%S", NameBuffer);

	return TRUE;
}


BOOLEAN
WinLdrLoadNLSData(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                  IN LPCSTR DirectoryPath,
                  IN LPCSTR AnsiFileName,
                  IN LPCSTR OemFileName,
                  IN LPCSTR LanguageFileName)
{
	CHAR FileName[255];
	ULONG AnsiFileId;
	ULONG OemFileId;
	ULONG LanguageFileId;
	ULONG AnsiFileSize, OemFileSize, LanguageFileSize;
	ULONG TotalSize;
	ULONG_PTR NlsDataBase;
	PVOID NlsVirtual;
	BOOLEAN AnsiEqualsOem = FALSE;
	FILEINFORMATION FileInfo;
	ULONG BytesRead, Status;

	/* There may be a case, when OEM and ANSI page coincide */
	if (!strcmp(AnsiFileName, OemFileName))
		AnsiEqualsOem = TRUE;

	/* Open file with ANSI and store its size */
	//Print(L"Loading %s...\n", Filename);
	strcpy(FileName, DirectoryPath);
	strcat(FileName, AnsiFileName);
	Status = ArcOpen(FileName, OpenReadOnly, &AnsiFileId);

	if (Status != ESUCCESS)
		goto Failure;

	Status = ArcGetFileInformation(AnsiFileId, &FileInfo);
	if (Status != ESUCCESS)
		goto Failure;
	AnsiFileSize = FileInfo.EndingAddress.LowPart;
	TRACE("AnsiFileSize: %d\n", AnsiFileSize);
	ArcClose(AnsiFileId);

	/* Open OEM file and store its length */
	if (AnsiEqualsOem)
	{
		OemFileSize = 0;
	}
	else
	{
		//Print(L"Loading %s...\n", Filename);
		strcpy(FileName, DirectoryPath);
		strcat(FileName, OemFileName);
		Status = ArcOpen(FileName, OpenReadOnly, &OemFileId);

		if (Status != ESUCCESS)
			goto Failure;

		Status = ArcGetFileInformation(OemFileId, &FileInfo);
		if (Status != ESUCCESS)
			goto Failure;
		OemFileSize = FileInfo.EndingAddress.LowPart;
		ArcClose(OemFileId);
	}
	TRACE("OemFileSize: %d\n", OemFileSize);

	/* And finally open the language codepage file and store its length */
	//Print(L"Loading %s...\n", Filename);
	strcpy(FileName, DirectoryPath);
	strcat(FileName, LanguageFileName);
	Status = ArcOpen(FileName, OpenReadOnly, &LanguageFileId);

	if (Status != ESUCCESS)
		goto Failure;

	Status = ArcGetFileInformation(LanguageFileId, &FileInfo);
	if (Status != ESUCCESS)
		goto Failure;
	LanguageFileSize = FileInfo.EndingAddress.LowPart;
	ArcClose(LanguageFileId);
	TRACE("LanguageFileSize: %d\n", LanguageFileSize);

	/* Sum up all three length, having in mind that every one of them
	   must start at a page boundary => thus round up each file to a page */
	TotalSize = MM_SIZE_TO_PAGES(AnsiFileSize) +
		MM_SIZE_TO_PAGES(OemFileSize)  +
		MM_SIZE_TO_PAGES(LanguageFileSize);

	/* Store it for later marking the pages as NlsData type */
	TotalNLSSize = TotalSize;

	NlsDataBase = (ULONG_PTR)MmAllocateMemoryWithType(TotalSize*MM_PAGE_SIZE, LoaderNlsData);

	if (NlsDataBase == 0)
		goto Failure;

	NlsVirtual = PaToVa((PVOID)NlsDataBase);
	LoaderBlock->NlsData->AnsiCodePageData = NlsVirtual;
	LoaderBlock->NlsData->OemCodePageData = (PVOID)((PUCHAR)NlsVirtual +
		(MM_SIZE_TO_PAGES(AnsiFileSize) << MM_PAGE_SHIFT));
	LoaderBlock->NlsData->UnicodeCodePageData = (PVOID)((PUCHAR)NlsVirtual +
		(MM_SIZE_TO_PAGES(AnsiFileSize) << MM_PAGE_SHIFT) +
		(MM_SIZE_TO_PAGES(OemFileSize) << MM_PAGE_SHIFT));

	/* Ansi and OEM data are the same - just set pointers to the same area */
	if (AnsiEqualsOem)
		LoaderBlock->NlsData->OemCodePageData = LoaderBlock->NlsData->AnsiCodePageData;


	/* Now actually read the data into memory, starting with Ansi file */
	strcpy(FileName, DirectoryPath);
	strcat(FileName, AnsiFileName);
	Status = ArcOpen(FileName, OpenReadOnly, &AnsiFileId);

	if (Status != ESUCCESS)
		goto Failure;

	Status = ArcRead(AnsiFileId, VaToPa(LoaderBlock->NlsData->AnsiCodePageData), AnsiFileSize, &BytesRead);

	if (Status != ESUCCESS)
		goto Failure;

	ArcClose(AnsiFileId);

	/* OEM now, if it doesn't equal Ansi of course */
	if (!AnsiEqualsOem)
	{
		strcpy(FileName, DirectoryPath);
		strcat(FileName, OemFileName);
		Status = ArcOpen(FileName, OpenReadOnly, &OemFileId);

		if (Status != ESUCCESS)
			goto Failure;

		Status = ArcRead(OemFileId, VaToPa(LoaderBlock->NlsData->OemCodePageData), OemFileSize, &BytesRead);

		if (Status != ESUCCESS)
			goto Failure;

		ArcClose(OemFileId);
	}

	/* finally the language file */
	strcpy(FileName, DirectoryPath);
	strcat(FileName, LanguageFileName);
	Status = ArcOpen(FileName, OpenReadOnly, &LanguageFileId);

	if (Status != ESUCCESS)
		goto Failure;

	Status = ArcRead(LanguageFileId, VaToPa(LoaderBlock->NlsData->UnicodeCodePageData), LanguageFileSize, &BytesRead);

	if (Status != ESUCCESS)
		goto Failure;

	ArcClose(LanguageFileId);

	//
	// THIS IS HAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACK
	// Should go to WinLdrLoadOemHalFont(), when it will be implemented
	//
	LoaderBlock->OemFontFile = VaToPa(LoaderBlock->NlsData->UnicodeCodePageData);

	/* Convert NlsTables address to VA */
	LoaderBlock->NlsData = PaToVa(LoaderBlock->NlsData);

	return TRUE;

Failure:
	//UiMessageBox("Error reading NLS file %s\n", Filename);
	UiMessageBox("Error reading NLS file!");
	return FALSE;
}

VOID
WinLdrScanRegistry(IN OUT PLOADER_PARAMETER_BLOCK LoaderBlock,
                   IN LPCSTR DirectoryPath)
{
	LONG rc = 0;
	FRLDRHKEY hGroupKey, hOrderKey, hServiceKey, hDriverKey;
	LPWSTR GroupNameBuffer;
	WCHAR ServiceName[256];
	ULONG OrderList[128];
	ULONG BufferSize;
	ULONG Index;
	ULONG TagIndex;
	LPWSTR GroupName;

	ULONG ValueSize;
	ULONG ValueType;
	ULONG StartValue;
	ULONG TagValue;
	WCHAR DriverGroup[256];
	ULONG DriverGroupSize;

	CHAR ImagePath[256];
	WCHAR TempImagePath[256];

	BOOLEAN Status;

	/* get 'service group order' key */
	rc = RegOpenKey(NULL,
		L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\ServiceGroupOrder",
		&hGroupKey);
	if (rc != ERROR_SUCCESS) {

		TRACE_CH(ODYSSEY, "Failed to open the 'ServiceGroupOrder' key (rc %d)\n", (int)rc);
		return;
	}

	/* get 'group order list' key */
	rc = RegOpenKey(NULL,
		L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\GroupOrderList",
		&hOrderKey);
	if (rc != ERROR_SUCCESS) {

		TRACE_CH(ODYSSEY, "Failed to open the 'GroupOrderList' key (rc %d)\n", (int)rc);
		return;
	}

	/* enumerate drivers */
	rc = RegOpenKey(NULL,
		L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services",
		&hServiceKey);
	if (rc != ERROR_SUCCESS)  {

		TRACE_CH(ODYSSEY, "Failed to open the 'Services' key (rc %d)\n", (int)rc);
		return;
	}

	/* Get the Name Group */
	BufferSize = 4096;
	GroupNameBuffer = MmHeapAlloc(BufferSize);
	rc = RegQueryValue(hGroupKey, L"List", NULL, (PUCHAR)GroupNameBuffer, &BufferSize);
	TRACE_CH(ODYSSEY, "RegQueryValue(): rc %d\n", (int)rc);
	if (rc != ERROR_SUCCESS)
		return;
	TRACE_CH(ODYSSEY, "BufferSize: %d \n", (int)BufferSize);
	TRACE_CH(ODYSSEY, "GroupNameBuffer: '%S' \n", GroupNameBuffer);

	/* Loop through each group */
	GroupName = GroupNameBuffer;
	while (*GroupName)
	{
		TRACE("Driver group: '%S'\n", GroupName);

		/* Query the Order */
		BufferSize = sizeof(OrderList);
		rc = RegQueryValue(hOrderKey, GroupName, NULL, (PUCHAR)OrderList, &BufferSize);
		if (rc != ERROR_SUCCESS) OrderList[0] = 0;

		/* enumerate all drivers */
		for (TagIndex = 1; TagIndex <= OrderList[0]; TagIndex++)
		{
			Index = 0;

			while (TRUE)
			{
				/* Get the Driver's Name */
				ValueSize = sizeof(ServiceName);
				rc = RegEnumKey(hServiceKey, Index, ServiceName, &ValueSize);
				//TRACE_CH(ODYSSEY, "RegEnumKey(): rc %d\n", (int)rc);

				/* Makre sure it's valid, and check if we're done */
				if (rc == ERROR_NO_MORE_ITEMS)
					break;
				if (rc != ERROR_SUCCESS)
				{
					MmHeapFree(GroupNameBuffer);
					return;
				}
				//TRACE_CH(ODYSSEY, "Service %d: '%S'\n", (int)Index, ServiceName);

				/* open driver Key */
				rc = RegOpenKey(hServiceKey, ServiceName, &hDriverKey);
				if (rc == ERROR_SUCCESS)
				{
					/* Read the Start Value */
					ValueSize = sizeof(ULONG);
					rc = RegQueryValue(hDriverKey, L"Start", &ValueType, (PUCHAR)&StartValue, &ValueSize);
					if (rc != ERROR_SUCCESS) StartValue = (ULONG)-1;
					//TRACE_CH(ODYSSEY, "  Start: %x  \n", (int)StartValue);

					/* Read the Tag */
					ValueSize = sizeof(ULONG);
					rc = RegQueryValue(hDriverKey, L"Tag", &ValueType, (PUCHAR)&TagValue, &ValueSize);
					if (rc != ERROR_SUCCESS) TagValue = (ULONG)-1;
					//TRACE_CH(ODYSSEY, "  Tag:   %x  \n", (int)TagValue);

					/* Read the driver's group */
					DriverGroupSize = sizeof(DriverGroup);
					rc = RegQueryValue(hDriverKey, L"Group", NULL, (PUCHAR)DriverGroup, &DriverGroupSize);
					//TRACE_CH(ODYSSEY, "  Group: '%S'  \n", DriverGroup);

					/* Make sure it should be started */
					if ((StartValue == 0) &&
						(TagValue == OrderList[TagIndex]) &&
						(_wcsicmp(DriverGroup, GroupName) == 0)) {

							/* Get the Driver's Location */
							ValueSize = sizeof(TempImagePath);
							rc = RegQueryValue(hDriverKey, L"ImagePath", NULL, (PUCHAR)TempImagePath, &ValueSize);

							/* Write the whole path if it suceeded, else prepare to fail */
							if (rc != ERROR_SUCCESS) {
								TRACE_CH(ODYSSEY, "ImagePath: not found\n");
								TempImagePath[0] = 0;
								sprintf(ImagePath, "%s\\system32\\drivers\\%S.sys", DirectoryPath, ServiceName);
							} else if (TempImagePath[0] != L'\\') {
								sprintf(ImagePath, "%s%S", DirectoryPath, TempImagePath);
							} else {
								sprintf(ImagePath, "%S", TempImagePath);
								TRACE_CH(ODYSSEY, "ImagePath: '%s'\n", ImagePath);
							}

							TRACE("Adding boot driver: '%s'\n", ImagePath);

							Status = WinLdrAddDriverToList(&LoaderBlock->BootDriverListHead,
								L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\",
								TempImagePath,
								ServiceName);

							if (!Status)
								ERR("Failed to add boot driver\n");
					} else
					{
						//TRACE("  Skipping driver '%S' with Start %d, Tag %d and Group '%S' (Current Tag %d, current group '%S')\n",
						//	ServiceName, StartValue, TagValue, DriverGroup, OrderList[TagIndex], GroupName);
					}
				}

				Index++;
			}
		}

		Index = 0;
		while (TRUE)
		{
			/* Get the Driver's Name */
			ValueSize = sizeof(ServiceName);
			rc = RegEnumKey(hServiceKey, Index, ServiceName, &ValueSize);

			//TRACE_CH(ODYSSEY, "RegEnumKey(): rc %d\n", (int)rc);
			if (rc == ERROR_NO_MORE_ITEMS)
				break;
			if (rc != ERROR_SUCCESS)
			{
				MmHeapFree(GroupNameBuffer);
				return;
			}
			//TRACE_CH(ODYSSEY, "Service %d: '%S'\n", (int)Index, ServiceName);

			/* open driver Key */
			rc = RegOpenKey(hServiceKey, ServiceName, &hDriverKey);
			if (rc == ERROR_SUCCESS)
			{
				/* Read the Start Value */
				ValueSize = sizeof(ULONG);
				rc = RegQueryValue(hDriverKey, L"Start", &ValueType, (PUCHAR)&StartValue, &ValueSize);
				if (rc != ERROR_SUCCESS) StartValue = (ULONG)-1;
				//TRACE_CH(ODYSSEY, "  Start: %x  \n", (int)StartValue);

				/* Read the Tag */
				ValueSize = sizeof(ULONG);
				rc = RegQueryValue(hDriverKey, L"Tag", &ValueType, (PUCHAR)&TagValue, &ValueSize);
				if (rc != ERROR_SUCCESS) TagValue = (ULONG)-1;
				//TRACE_CH(ODYSSEY, "  Tag:   %x  \n", (int)TagValue);

				/* Read the driver's group */
				DriverGroupSize = sizeof(DriverGroup);
				rc = RegQueryValue(hDriverKey, L"Group", NULL, (PUCHAR)DriverGroup, &DriverGroupSize);
				//TRACE_CH(ODYSSEY, "  Group: '%S'  \n", DriverGroup);

				for (TagIndex = 1; TagIndex <= OrderList[0]; TagIndex++) {
					if (TagValue == OrderList[TagIndex]) break;
				}

				if ((StartValue == 0) &&
					(TagIndex > OrderList[0]) &&
					(_wcsicmp(DriverGroup, GroupName) == 0)) {

						ValueSize = sizeof(TempImagePath);
						rc = RegQueryValue(hDriverKey, L"ImagePath", NULL, (PUCHAR)TempImagePath, &ValueSize);
						if (rc != ERROR_SUCCESS) {
							TRACE_CH(ODYSSEY, "ImagePath: not found\n");
							TempImagePath[0] = 0;
							sprintf(ImagePath, "%ssystem32\\drivers\\%S.sys", DirectoryPath, ServiceName);
						} else if (TempImagePath[0] != L'\\') {
							sprintf(ImagePath, "%s%S", DirectoryPath, TempImagePath);
						} else {
							sprintf(ImagePath, "%S", TempImagePath);
							TRACE_CH(ODYSSEY, "ImagePath: '%s'\n", ImagePath);
						}
						TRACE("  Adding boot driver: '%s'\n", ImagePath);

						Status = WinLdrAddDriverToList(&LoaderBlock->BootDriverListHead,
							L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\",
							TempImagePath,
							ServiceName);

						if (!Status)
							ERR(" Failed to add boot driver\n");
				} else
				{
					//TRACE("  Skipping driver '%S' with Start %d, Tag %d and Group '%S' (Current group '%S')\n",
					//	ServiceName, StartValue, TagValue, DriverGroup, GroupName);
				}
			}

			Index++;
		}

		/* Move to the next group name */
		GroupName = GroupName + wcslen(GroupName) + 1;
	}

	/* Free allocated memory */
	MmHeapFree(GroupNameBuffer);
}

BOOLEAN
WinLdrAddDriverToList(LIST_ENTRY *BootDriverListHead,
                      LPWSTR RegistryPath,
                      LPWSTR ImagePath,
                      LPWSTR ServiceName)
{
	PBOOT_DRIVER_LIST_ENTRY BootDriverEntry;
	NTSTATUS Status;
	USHORT PathLength;

	BootDriverEntry = MmHeapAlloc(sizeof(BOOT_DRIVER_LIST_ENTRY));

	if (!BootDriverEntry)
		return FALSE;

	// DTE will be filled during actual load of the driver
	BootDriverEntry->LdrEntry = NULL;

	// Check - if we have a valid ImagePath, if not - we need to build it
	// like "System32\\Drivers\\blah.sys"
	if (ImagePath && (wcslen(ImagePath) > 0))
	{
		// Just copy ImagePath to the corresponding field in the structure
		PathLength = wcslen(ImagePath) * sizeof(WCHAR) + sizeof(UNICODE_NULL);

		BootDriverEntry->FilePath.Length = 0;
		BootDriverEntry->FilePath.MaximumLength = PathLength;
		BootDriverEntry->FilePath.Buffer = MmHeapAlloc(PathLength);

		if (!BootDriverEntry->FilePath.Buffer)
		{
			MmHeapFree(BootDriverEntry);
			return FALSE;
		}

		Status = RtlAppendUnicodeToString(&BootDriverEntry->FilePath, ImagePath);
		if (!NT_SUCCESS(Status))
		{
			MmHeapFree(BootDriverEntry->FilePath.Buffer);
			MmHeapFree(BootDriverEntry);
			return FALSE;
		}
	}
	else
	{
		// we have to construct ImagePath ourselves
		PathLength = wcslen(ServiceName)*sizeof(WCHAR) + sizeof(L"system32\\drivers\\.sys");
		BootDriverEntry->FilePath.Length = 0;
		BootDriverEntry->FilePath.MaximumLength = PathLength;
		BootDriverEntry->FilePath.Buffer = MmHeapAlloc(PathLength);

		if (!BootDriverEntry->FilePath.Buffer)
		{
			MmHeapFree(BootDriverEntry);
			return FALSE;
		}

		Status = RtlAppendUnicodeToString(&BootDriverEntry->FilePath, L"system32\\drivers\\");
		if (!NT_SUCCESS(Status))
		{
			MmHeapFree(BootDriverEntry->FilePath.Buffer);
			MmHeapFree(BootDriverEntry);
			return FALSE;
		}

		Status = RtlAppendUnicodeToString(&BootDriverEntry->FilePath, ServiceName);
		if (!NT_SUCCESS(Status))
		{
			MmHeapFree(BootDriverEntry->FilePath.Buffer);
			MmHeapFree(BootDriverEntry);
			return FALSE;
		}

		Status = RtlAppendUnicodeToString(&BootDriverEntry->FilePath, L".sys");
		if (!NT_SUCCESS(Status))
		{
			MmHeapFree(BootDriverEntry->FilePath.Buffer);
			MmHeapFree(BootDriverEntry);
			return FALSE;
		}
	}

	// Add registry path
	PathLength = (wcslen(RegistryPath) + wcslen(ServiceName))*sizeof(WCHAR) + sizeof(UNICODE_NULL);
	BootDriverEntry->RegistryPath.Length = 0;
	BootDriverEntry->RegistryPath.MaximumLength = PathLength;
	BootDriverEntry->RegistryPath.Buffer = MmHeapAlloc(PathLength);
	if (!BootDriverEntry->RegistryPath.Buffer)
		return FALSE;

	Status = RtlAppendUnicodeToString(&BootDriverEntry->RegistryPath, RegistryPath);
	if (!NT_SUCCESS(Status))
		return FALSE;

	Status = RtlAppendUnicodeToString(&BootDriverEntry->RegistryPath, ServiceName);
	if (!NT_SUCCESS(Status))
		return FALSE;

	// Insert entry at top of the list
	InsertTailList(BootDriverListHead, &BootDriverEntry->Link);

	return TRUE;
}
