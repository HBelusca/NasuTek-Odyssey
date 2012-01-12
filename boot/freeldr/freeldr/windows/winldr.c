/*
 *  FreeLoader
 *
 *  Copyright (C) 1998-2003  Brian Palmer    <brianp@sginet.com>
 *  Copyright (C) 2006       Aleksey Bragin  <aleksey@odyssey.org>
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

#include <ndk/ldrtypes.h>
#include <debug.h>

DBG_DEFAULT_CHANNEL(WINDOWS);

//FIXME: Do a better way to retrieve Arc disk information
extern ULONG odyssey_disk_count;
extern ARC_DISK_SIGNATURE odyssey_arc_disk_info[];
extern char odyssey_arc_strings[32][256];

extern BOOLEAN UseRealHeap;
extern ULONG LoaderPagesSpanned;
extern BOOLEAN AcpiPresent;

extern HEADLESS_LOADER_BLOCK LoaderRedirectionInformation;
extern BOOLEAN WinLdrTerminalConnected;
extern void WinLdrSetupEms(IN PCHAR BootOptions);

// debug stuff
VOID DumpMemoryAllocMap(VOID);

// Init "phase 0"
VOID
AllocateAndInitLPB(PLOADER_PARAMETER_BLOCK *OutLoaderBlock)
{
	PLOADER_PARAMETER_BLOCK LoaderBlock;

	/* Allocate and zero-init the LPB */
	LoaderBlock = MmHeapAlloc(sizeof(LOADER_PARAMETER_BLOCK));
	RtlZeroMemory(LoaderBlock, sizeof(LOADER_PARAMETER_BLOCK));

	/* Init three critical lists, used right away */
	InitializeListHead(&LoaderBlock->LoadOrderListHead);
	InitializeListHead(&LoaderBlock->MemoryDescriptorListHead);
	InitializeListHead(&LoaderBlock->BootDriverListHead);

	/* Alloc space for NLS (it will be converted to VA in WinLdrLoadNLS) */
	LoaderBlock->NlsData = MmHeapAlloc(sizeof(NLS_DATA_BLOCK));
	if (LoaderBlock->NlsData == NULL)
	{
		UiMessageBox("Failed to allocate memory for NLS table data!");
		return;
	}
	RtlZeroMemory(LoaderBlock->NlsData, sizeof(NLS_DATA_BLOCK));

	*OutLoaderBlock = LoaderBlock;
}

// Init "phase 1"
VOID
WinLdrInitializePhase1(PLOADER_PARAMETER_BLOCK LoaderBlock,
                       LPCSTR Options,
                       LPCSTR SystemRoot,
                       LPCSTR BootPath,
                       USHORT VersionToBoot)
{
	/* Examples of correct options and paths */
	//CHAR	Options[] = "/DEBUGPORT=COM1 /BAUDRATE=115200";
	//CHAR	Options[] = "/NODEBUG";
	//CHAR	SystemRoot[] = "\\WINNT\\";
	//CHAR	ArcBoot[] = "multi(0)disk(0)rdisk(0)partition(1)";

	CHAR	HalPath[] = "\\";
	CHAR	ArcBoot[256];
	CHAR	MiscFiles[256];
	ULONG i, PathSeparator;
	PLOADER_PARAMETER_EXTENSION Extension;

	/* Construct SystemRoot and ArcBoot from SystemPath */
	PathSeparator = strstr(BootPath, "\\") - BootPath;
	strncpy(ArcBoot, BootPath, PathSeparator);
	ArcBoot[PathSeparator] = 0;

	TRACE("ArcBoot: %s\n", ArcBoot);
	TRACE("SystemRoot: %s\n", SystemRoot);
	TRACE("Options: %s\n", Options);

	/* Fill Arc BootDevice */
	LoaderBlock->ArcBootDeviceName = MmHeapAlloc(strlen(ArcBoot)+1);
	strcpy(LoaderBlock->ArcBootDeviceName, ArcBoot);
	LoaderBlock->ArcBootDeviceName = PaToVa(LoaderBlock->ArcBootDeviceName);

	/* Fill Arc HalDevice, it matches ArcBoot path */
	LoaderBlock->ArcHalDeviceName = MmHeapAlloc(strlen(ArcBoot)+1);
	strcpy(LoaderBlock->ArcHalDeviceName, ArcBoot);
	LoaderBlock->ArcHalDeviceName = PaToVa(LoaderBlock->ArcHalDeviceName);

	/* Fill SystemRoot */
	LoaderBlock->NtBootPathName = MmHeapAlloc(strlen(SystemRoot)+1);
	strcpy(LoaderBlock->NtBootPathName, SystemRoot);
	LoaderBlock->NtBootPathName = PaToVa(LoaderBlock->NtBootPathName);

	/* Fill NtHalPathName */
	LoaderBlock->NtHalPathName = MmHeapAlloc(strlen(HalPath)+1);
	strcpy(LoaderBlock->NtHalPathName, HalPath);
	LoaderBlock->NtHalPathName = PaToVa(LoaderBlock->NtHalPathName);

	/* Fill load options */
	LoaderBlock->LoadOptions = MmHeapAlloc(strlen(Options)+1);
	strcpy(LoaderBlock->LoadOptions, Options);
	LoaderBlock->LoadOptions = PaToVa(LoaderBlock->LoadOptions);

	/* Arc devices */
	LoaderBlock->ArcDiskInformation = (PARC_DISK_INFORMATION)MmHeapAlloc(sizeof(ARC_DISK_INFORMATION));
	InitializeListHead(&LoaderBlock->ArcDiskInformation->DiskSignatureListHead);

	/* Convert ARC disk information from freeldr to a correct format */
	for (i = 0; i < odyssey_disk_count; i++)
	{
		PARC_DISK_SIGNATURE ArcDiskInfo;

		/* Get the ARC structure */
		ArcDiskInfo = (PARC_DISK_SIGNATURE)MmHeapAlloc(sizeof(ARC_DISK_SIGNATURE));
		RtlZeroMemory(ArcDiskInfo, sizeof(ARC_DISK_SIGNATURE));

		/* Copy the data over */
		ArcDiskInfo->Signature = odyssey_arc_disk_info[i].Signature;
		ArcDiskInfo->CheckSum = odyssey_arc_disk_info[i].CheckSum;

		/* Copy the ARC Name */
		ArcDiskInfo->ArcName = (PCHAR)MmHeapAlloc(sizeof(CHAR)*256);
		strcpy(ArcDiskInfo->ArcName, odyssey_arc_disk_info[i].ArcName);
		ArcDiskInfo->ArcName = (PCHAR)PaToVa(ArcDiskInfo->ArcName);

		/* Mark partition table as valid */
		ArcDiskInfo->ValidPartitionTable = TRUE;

		/* Insert into the list */
		InsertTailList(&LoaderBlock->ArcDiskInformation->DiskSignatureListHead,
			&ArcDiskInfo->ListEntry);
	}

	/* Convert all list's to Virtual address */

	/* Convert the ArcDisks list to virtual address */
	List_PaToVa(&LoaderBlock->ArcDiskInformation->DiskSignatureListHead);
	LoaderBlock->ArcDiskInformation = PaToVa(LoaderBlock->ArcDiskInformation);

	/* Convert configuration entries to VA */
	ConvertConfigToVA(LoaderBlock->ConfigurationRoot);
	LoaderBlock->ConfigurationRoot = PaToVa(LoaderBlock->ConfigurationRoot);

	/* Convert all DTE into virtual addresses */
	List_PaToVa(&LoaderBlock->LoadOrderListHead);

	/* this one will be converted right before switching to
	   virtual paging mode */
	//List_PaToVa(&LoaderBlock->MemoryDescriptorListHead);

	/* Convert list of boot drivers */
	List_PaToVa(&LoaderBlock->BootDriverListHead);

	/* Initialize Extension now */
	Extension = MmHeapAlloc(sizeof(LOADER_PARAMETER_EXTENSION));
	if (Extension == NULL)
	{
		UiMessageBox("Failed to allocate LPB Extension!");
		return;
	}
	RtlZeroMemory(Extension, sizeof(LOADER_PARAMETER_EXTENSION));

	/* Fill LPB extension */
	Extension->Size = sizeof(LOADER_PARAMETER_EXTENSION);
	Extension->MajorVersion = (VersionToBoot & 0xFF00) >> 8;
	Extension->MinorVersion = VersionToBoot & 0xFF;
	Extension->Profile.Status = 2;

	/* Check if ACPI is present */
	if (AcpiPresent)
	{
		/* See KiRosFrldrLpbToNtLpb for details */
		Extension->AcpiTable = (PVOID)1;
	}

#ifdef _M_IX86
    /* Set headless block pointer */
    if (WinLdrTerminalConnected)
    {
        Extension->HeadlessLoaderBlock = MmHeapAlloc(sizeof(HEADLESS_LOADER_BLOCK));
        if (Extension->HeadlessLoaderBlock == NULL)
        {
            UiMessageBox("Failed to allocate HLB Extension!");
            while (TRUE);
            return;
        }
        RtlCopyMemory(
            Extension->HeadlessLoaderBlock,
            &LoaderRedirectionInformation,
            sizeof(HEADLESS_LOADER_BLOCK));
        Extension->HeadlessLoaderBlock = PaToVa(Extension->HeadlessLoaderBlock);
    }
#endif
	/* Load drivers database */
	strcpy(MiscFiles, BootPath);
	strcat(MiscFiles, "AppPatch\\drvmain.sdb");
	Extension->DrvDBImage = PaToVa(WinLdrLoadModule(MiscFiles,
		&Extension->DrvDBSize, LoaderRegistryData));

	/* Convert extension and setup block pointers */
	LoaderBlock->Extension = PaToVa(Extension);

	if (LoaderBlock->SetupLdrBlock)
		LoaderBlock->SetupLdrBlock = PaToVa(LoaderBlock->SetupLdrBlock);

}

BOOLEAN
WinLdrLoadDeviceDriver(PLOADER_PARAMETER_BLOCK LoaderBlock,
                       LPSTR BootPath,
                       PUNICODE_STRING FilePath,
                       ULONG Flags,
                       PLDR_DATA_TABLE_ENTRY *DriverDTE)
{
	CHAR FullPath[1024];
	CHAR DriverPath[1024];
	CHAR DllName[1024];
	PCHAR DriverNamePos;
	BOOLEAN Status;
	PVOID DriverBase;

	// Separate the path to file name and directory path
	_snprintf(DriverPath, sizeof(DriverPath), "%wZ", FilePath);
	DriverNamePos = strrchr(DriverPath, '\\');
	if (DriverNamePos != NULL)
	{
		// Copy the name
		strcpy(DllName, DriverNamePos+1);

		// Cut out the name from the path
		*(DriverNamePos+1) = 0;
	}
	else
	{
		// There is no directory in the path
		strcpy(DllName, DriverPath);
		DriverPath[0] = 0;
	}

	TRACE("DriverPath: %s, DllName: %s, LPB %p\n", DriverPath, DllName, LoaderBlock);


	// Check if driver is already loaded
	Status = WinLdrCheckForLoadedDll(LoaderBlock, DllName, DriverDTE);
	if (Status)
	{
		// We've got the pointer to its DTE, just return success
		return TRUE;
	}

	// It's not loaded, we have to load it
	_snprintf(FullPath, sizeof(FullPath), "%s%wZ", BootPath, FilePath);
	Status = WinLdrLoadImage(FullPath, LoaderBootDriver, &DriverBase);
	if (!Status)
		return FALSE;

	// Allocate a DTE for it
	Status = WinLdrAllocateDataTableEntry(LoaderBlock, DllName, DllName, DriverBase, DriverDTE);
	if (!Status)
	{
		ERR("WinLdrAllocateDataTableEntry() failed\n");
		return FALSE;
	}

	// Modify any flags, if needed
	(*DriverDTE)->Flags |= Flags;

	// Look for any dependencies it may have, and load them too
	sprintf(FullPath,"%s%s", BootPath, DriverPath);
	Status = WinLdrScanImportDescriptorTable(LoaderBlock, FullPath, *DriverDTE);
	if (!Status)
	{
		ERR("WinLdrScanImportDescriptorTable() failed for %s\n", FullPath);
		return FALSE;
	}

	return TRUE;
}

BOOLEAN
WinLdrLoadBootDrivers(PLOADER_PARAMETER_BLOCK LoaderBlock,
                      LPSTR BootPath)
{
	PLIST_ENTRY NextBd;
	PBOOT_DRIVER_LIST_ENTRY BootDriver;
	BOOLEAN Status;

	// Walk through the boot drivers list
	NextBd = LoaderBlock->BootDriverListHead.Flink;

	while (NextBd != &LoaderBlock->BootDriverListHead)
	{
		BootDriver = CONTAINING_RECORD(NextBd, BOOT_DRIVER_LIST_ENTRY, Link);

		TRACE("BootDriver %wZ DTE %08X RegPath: %wZ\n", &BootDriver->FilePath,
			BootDriver->LdrEntry, &BootDriver->RegistryPath);

		// Paths are relative (FIXME: Are they always relative?)

		// Load it
		Status = WinLdrLoadDeviceDriver(LoaderBlock, BootPath, &BootDriver->FilePath,
			0, &BootDriver->LdrEntry);

		// If loading failed - cry loudly
		//FIXME: Maybe remove it from the list and try to continue?
		if (!Status)
		{
			UiMessageBox("Can't load boot driver!");
			return FALSE;
		}

		// Convert the RegistryPath and DTE addresses to VA since we are not going to use it anymore
		BootDriver->RegistryPath.Buffer = PaToVa(BootDriver->RegistryPath.Buffer);
		BootDriver->FilePath.Buffer = PaToVa(BootDriver->FilePath.Buffer);
		BootDriver->LdrEntry = PaToVa(BootDriver->LdrEntry);

		NextBd = BootDriver->Link.Flink;
	}

	return TRUE;
}

PVOID WinLdrLoadModule(PCSTR ModuleName, ULONG *Size,
					   TYPE_OF_MEMORY MemoryType)
{
	ULONG FileId;
	PVOID PhysicalBase;
	FILEINFORMATION FileInfo;
	ULONG FileSize;
	ULONG Status;
	ULONG BytesRead;

	//CHAR ProgressString[256];

	/* Inform user we are loading files */
	//sprintf(ProgressString, "Loading %s...", FileName);
	//UiDrawProgressBarCenter(1, 100, ProgressString);

	TRACE("Loading module %s\n", ModuleName);
	*Size = 0;

	/* Open the image file */
	Status = ArcOpen((PCHAR)ModuleName, OpenReadOnly, &FileId);
	if (Status != ESUCCESS)
	{
		/* In case of errors, we just return, without complaining to the user */
		return NULL;
	}

	/* Get this file's size */
	Status = ArcGetFileInformation(FileId, &FileInfo);
	if (Status != ESUCCESS)
	{
		ArcClose(FileId);
		return NULL;
	}
	FileSize = FileInfo.EndingAddress.LowPart;
	*Size = FileSize;

	/* Allocate memory */
	PhysicalBase = MmAllocateMemoryWithType(FileSize, MemoryType);
	if (PhysicalBase == NULL)
	{
		ArcClose(FileId);
		return NULL;
	}

	/* Load whole file */
	Status = ArcRead(FileId, PhysicalBase, FileSize, &BytesRead);
	ArcClose(FileId);
	if (Status != ESUCCESS)
	{
		return NULL;
	}

	TRACE("Loaded %s at 0x%x with size 0x%x\n", ModuleName, PhysicalBase, FileSize);

	return PhysicalBase;
}


USHORT
WinLdrDetectVersion()
{
	LONG rc;
	FRLDRHKEY hKey;

	rc = RegOpenKey(
		NULL,
		L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server",
		&hKey);
	if (rc != ERROR_SUCCESS)
	{
		// Key doesn't exist; assume NT 4.0
		return _WIN32_WINNT_NT4;
	}

	// We may here want to read the value of ProductVersion
	return _WIN32_WINNT_WS03;
}

PVOID
LoadModule(
    PLOADER_PARAMETER_BLOCK LoaderBlock,
    PCCH Path,
    PCCH File,
    PLDR_DATA_TABLE_ENTRY *Dte,
    ULONG Percentage)
{
	CHAR FullFileName[MAX_PATH];
	CHAR ProgressString[256];
	NTSTATUS Status;
	PVOID BaseAdress;

	UiDrawBackdrop();
	sprintf(ProgressString, "Loading %s...", File);
	UiDrawStatusText(ProgressString);
	UiDrawProgressBarCenter(Percentage, 100, "Starting Odyssey...");

	strcpy(FullFileName, Path);
	strcat(FullFileName, "SYSTEM32\\");
	strcat(FullFileName, File);

	Status = WinLdrLoadImage(FullFileName, LoaderSystemCode, &BaseAdress);
	TRACE("%s loaded with status %d at %p\n",
	        File, Status, BaseAdress);

	strcpy(FullFileName, "WINDOWS\\SYSTEM32\\");
	strcat(FullFileName, File);
	WinLdrAllocateDataTableEntry(LoaderBlock, File,
		FullFileName, BaseAdress, Dte);

	return BaseAdress;
}

VOID
LoadAndBootWindows(PCSTR OperatingSystemName,
                   PSTR SettingsValue,
                   USHORT OperatingSystemVersion)
{
	BOOLEAN HasSection;
	char  BootPath[MAX_PATH];
	CHAR  FileName[MAX_PATH];
	CHAR  BootOptions[256];
	PCHAR File;
	BOOLEAN Status;
	ULONG_PTR SectionId;
	PLOADER_PARAMETER_BLOCK LoaderBlock;

	// Open the operating system section
	// specified in the .ini file
	HasSection = TRUE;

	UiDrawBackdrop();
	UiDrawProgressBarCenter(1, 100, "Starting Odyssey...");

	/* Read the system path is set in the .ini file */
	if (!HasSection ||
	    !OcdReadSetting(OperatingSystemName, L"SystemPath", &BootPath, sizeof(BootPath)))
	{
		strcpy(BootPath, OperatingSystemName);
	}

	/* Special case for LiveCD */
	if (!_strnicmp(BootPath, "LiveCD", strlen("LiveCD")))
	{
		strcpy(FileName, BootPath + strlen("LiveCD"));
		MachDiskGetBootPath(BootPath, sizeof(BootPath));
		strcat(BootPath, FileName);
	}

	/* append a backslash */
	if ((strlen(BootPath)==0) || BootPath[strlen(BootPath)] != '\\')
		strcat(BootPath, "\\");

	/* Read booting options */
	if (!HasSection || !OcdReadSetting(OperatingSystemName, L"Options", &BootOptions, sizeof(BootOptions)))
	{
		/* Get options after the title */
		const CHAR*p = SettingsValue;
		while (*p == ' ' || *p == '"')
			p++;
		while (*p != '\0' && *p != '"')
			p++;
		strcpy(BootOptions, p);
		TRACE("BootOptions: '%s'\n", BootOptions);
	}

	/* Append boot-time options */
	AppendBootTimeOptions(BootOptions);

	/* Check if a ramdisk file was given */
	File = strstr(BootOptions, "/RDPATH=");
	if (File)
	{
		/* Copy the file name and everything else after it */
		strcpy(FileName, File + 8);

		/* Null-terminate */
		*strstr(FileName, " ") = ANSI_NULL;

		/* Load the ramdisk */
		RamDiskLoadVirtualFile(FileName);
	}

	/* Let user know we started loading */
	//UiDrawStatusText("Loading...");

	TRACE("BootPath: '%s'\n", BootPath);

	/* Allocate and minimalistic-initialize LPB */
	AllocateAndInitLPB(&LoaderBlock);

#ifdef _M_IX86
   	/* Setup redirection support */
	WinLdrSetupEms(BootOptions);
#endif

	/* Load Hive */
	UiDrawBackdrop();
	UiDrawProgressBarCenter(15, 100, "Starting Odyssey...");
	Status = WinLdrInitSystemHive(LoaderBlock, BootPath);
	TRACE("SYSTEM hive loaded with status %d\n", Status);

	/* Load NLS data, OEM font, and prepare boot drivers list */
	Status = WinLdrScanSystemHive(LoaderBlock, BootPath);
	TRACE("SYSTEM hive scanned with status %d\n", Status);


	LoadAndBootWindowsCommon(OperatingSystemVersion,
	                         LoaderBlock,
	                         BootOptions,
	                         BootPath,
	                         0);
}

VOID
LoadAndBootWindowsCommon(
	USHORT OperatingSystemVersion,
	PLOADER_PARAMETER_BLOCK LoaderBlock,
	LPCSTR BootOptions,
	LPCSTR BootPath,
	BOOLEAN Setup)
{
	PLOADER_PARAMETER_BLOCK LoaderBlockVA;
	BOOLEAN Status;
	CHAR FileName[MAX_PATH];
	PVOID NtosBase = NULL, HalBase = NULL, KdComBase = NULL;
	PLDR_DATA_TABLE_ENTRY KernelDTE, HalDTE, KdComDTE = NULL;
	KERNEL_ENTRY_POINT KiSystemStartup;
	LPCSTR SystemRoot;

	/* Convert BootPath to SystemRoot */
	SystemRoot = strstr(BootPath, "\\");

	/* Detect hardware */
	UseRealHeap = TRUE;
	LoaderBlock->ConfigurationRoot = MachHwDetect();

	if (OperatingSystemVersion == 0)
		OperatingSystemVersion = WinLdrDetectVersion();

	/* Load kernel */
	NtosBase = LoadModule(LoaderBlock, BootPath, "NTOSKRNL.EXE", &KernelDTE, 30);

	/* Load HAL */
	HalBase = LoadModule(LoaderBlock, BootPath, "HAL.DLL", &HalDTE, 45);

	/* Load kernel-debugger support dll */
	if (OperatingSystemVersion > _WIN32_WINNT_WIN2K)
	{
		KdComBase = LoadModule(LoaderBlock, BootPath, "KDCOM.DLL", &KdComDTE, 60);
	}

	/* Load all referenced DLLs for kernel, HAL and kdcom.dll */
	strcpy(FileName, BootPath);
	strcat(FileName, "system32\\");
	Status = WinLdrScanImportDescriptorTable(LoaderBlock, FileName, KernelDTE);
	Status &= WinLdrScanImportDescriptorTable(LoaderBlock, FileName, HalDTE);
	if (KdComDTE)
		Status &= WinLdrScanImportDescriptorTable(LoaderBlock, FileName, KdComDTE);

	if (!Status)
	{
		UiMessageBox("Error loading imported dll.");
		return;
	}

	/* Load boot drivers */
	UiDrawBackdrop();
	UiDrawProgressBarCenter(100, 100, "Starting Odyssey...");
	Status = WinLdrLoadBootDrivers(LoaderBlock, (PCHAR)BootPath);
	TRACE("Boot drivers loaded with status %d\n", Status);

	/* Initialize Phase 1 - no drivers loading anymore */
	WinLdrInitializePhase1(LoaderBlock, BootOptions, SystemRoot, BootPath, OperatingSystemVersion);

	/* Save entry-point pointer and Loader block VAs */
	KiSystemStartup = (KERNEL_ENTRY_POINT)KernelDTE->EntryPoint;
	LoaderBlockVA = PaToVa(LoaderBlock);

	/* "Stop all motors", change videomode */
	MachPrepareForOdyssey(Setup);

	/* Debugging... */
	//DumpMemoryAllocMap();

	/* Do the machine specific initialization */
	WinLdrSetupMachineDependent(LoaderBlock);

	/* Map pages and create memory descriptors */
	WinLdrSetupMemoryLayout(LoaderBlock);

	/* Save final value of LoaderPagesSpanned */
	LoaderBlock->Extension->LoaderPagesSpanned = LoaderPagesSpanned;

	/* Set processor context */
	WinLdrSetProcessorContext();

	TRACE("Hello from paged mode, KiSystemStartup %p, LoaderBlockVA %p!\n",
		KiSystemStartup, LoaderBlockVA);

	// Zero KI_USER_SHARED_DATA page
	memset((PVOID)KI_USER_SHARED_DATA, 0, MM_PAGE_SIZE);

	WinLdrpDumpMemoryDescriptors(LoaderBlockVA);
	WinLdrpDumpBootDriver(LoaderBlockVA);
	WinLdrpDumpArcDisks(LoaderBlockVA);

	//FIXME: If I substitute this debugging checkpoint, GCC will "optimize away" the code below
	//while (1) {};
	/*asm(".intel_syntax noprefix\n");
		asm("test1:\n");
		asm("jmp test1\n");
	asm(".att_syntax\n");*/

	/* Pass control */
	(*KiSystemStartup)(LoaderBlockVA);
}

VOID
WinLdrpDumpMemoryDescriptors(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
	PLIST_ENTRY NextMd;
	PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

	NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

	while (NextMd != &LoaderBlock->MemoryDescriptorListHead)
	{
		MemoryDescriptor = CONTAINING_RECORD(NextMd, MEMORY_ALLOCATION_DESCRIPTOR, ListEntry);

		TRACE("BP %08X PC %04X MT %d\n", MemoryDescriptor->BasePage,
			MemoryDescriptor->PageCount, MemoryDescriptor->MemoryType);

		NextMd = MemoryDescriptor->ListEntry.Flink;
	}
}

VOID
WinLdrpDumpBootDriver(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
	PLIST_ENTRY NextBd;
	PBOOT_DRIVER_LIST_ENTRY BootDriver;

	NextBd = LoaderBlock->BootDriverListHead.Flink;

	while (NextBd != &LoaderBlock->BootDriverListHead)
	{
		BootDriver = CONTAINING_RECORD(NextBd, BOOT_DRIVER_LIST_ENTRY, Link);

		TRACE("BootDriver %wZ DTE %08X RegPath: %wZ\n", &BootDriver->FilePath,
			BootDriver->LdrEntry, &BootDriver->RegistryPath);

		NextBd = BootDriver->Link.Flink;
	}
}

VOID
WinLdrpDumpArcDisks(PLOADER_PARAMETER_BLOCK LoaderBlock)
{
	PLIST_ENTRY NextBd;
	PARC_DISK_SIGNATURE ArcDisk;

	NextBd = LoaderBlock->ArcDiskInformation->DiskSignatureListHead.Flink;

	while (NextBd != &LoaderBlock->ArcDiskInformation->DiskSignatureListHead)
	{
		ArcDisk = CONTAINING_RECORD(NextBd, ARC_DISK_SIGNATURE, ListEntry);

		TRACE("ArcDisk %s checksum: 0x%X, signature: 0x%X\n",
			ArcDisk->ArcName, ArcDisk->CheckSum, ArcDisk->Signature);

		NextBd = ArcDisk->ListEntry.Flink;
	}
}


