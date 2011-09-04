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

#pragma once

/* Base Addres of Kernel in Physical Memory */
#define KERNEL_BASE_PHYS 0x800000

#if defined(_M_IX86)

/* Bits to shift to convert a Virtual Address into an Offset in the Page Table */
#define PFN_SHIFT 12

/* Bits to shift to convert a Virtual Address into an Offset in the Page Directory */
#define PDE_SHIFT 22
#define PDE_SHIFT_PAE 18

/* Converts a Physical Address Pointer into a Page Frame Number */
#define PaPtrToPfn(p) \
    (((ULONG_PTR)&p) >> PFN_SHIFT)

/* Converts a Physical Address into a Page Frame Number */
#define PaToPfn(p) \
    ((p) >> PFN_SHIFT)

#define STARTUP_BASE                0xC0000000
#define HAL_BASE                    0xFFC00000
#define APIC_BASE                   0xFFFE0000

#define LowMemPageTableIndex        0
#define StartupPageTableIndex       (STARTUP_BASE >> 22)
#define HalPageTableIndex           (HAL_BASE >> 22)

typedef struct _PAGE_DIRECTORY_X86
{
    HARDWARE_PTE Pde[1024];
} PAGE_DIRECTORY_X86, *PPAGE_DIRECTORY_X86;

#endif

///////////////////////////////////////////////////////////////////////////////////////
//
// Odyssey Loading Functions
//
///////////////////////////////////////////////////////////////////////////////////////
VOID LoadAndBootOdyssey(PCSTR OperatingSystemName);

///////////////////////////////////////////////////////////////////////////////////////
//
// Odyssey Setup Loader Functions
//
///////////////////////////////////////////////////////////////////////////////////////
VOID OdysseyRunSetupLoader(VOID);

///////////////////////////////////////////////////////////////////////////////////////
//
// ARC Path Functions
//
///////////////////////////////////////////////////////////////////////////////////////
BOOLEAN
DissectArcPath2(
    IN CHAR* ArcPath,
    OUT ULONG* x,
    OUT ULONG* y,
    OUT ULONG* z,
    OUT ULONG* Partition,
    OUT ULONG *PathSyntax);
BOOLEAN DissectArcPath(CHAR *ArcPath, CHAR *BootPath, UCHAR* BootDrive, ULONG* BootPartition);
VOID ConstructArcPath(PCHAR ArcPath, PCHAR SystemFolder, UCHAR Disk, ULONG Partition);
UCHAR ConvertArcNameToBiosDriveNumber(PCHAR ArcPath);

///////////////////////////////////////////////////////////////////////////////////////
//
// Loader Functions And Definitions
//
///////////////////////////////////////////////////////////////////////////////////////
extern ROS_LOADER_PARAMETER_BLOCK LoaderBlock; /* Multiboot info structure passed to kernel */
extern char					odyssey_kernel_cmdline[255];	// Command line passed to kernel
extern LOADER_MODULE		odyssey_modules[64];		// Array to hold boot module info loaded for the kernel
extern char					odyssey_module_strings[64][256];	// Array to hold module names
typedef struct _odyssey_mem_data {
    unsigned long			memory_map_descriptor_size;
    memory_map_t			memory_map[32];		// Memory map
} odyssey_mem_data_t;
extern odyssey_mem_data_t odyssey_mem_data;
#define odyssey_memory_map_descriptor_size odyssey_mem_data.memory_map_descriptor_size
#define odyssey_memory_map odyssey_mem_data.memory_map

VOID FASTCALL FrLdrSetupPae(ULONG Magic);
VOID FASTCALL FrLdrSetupPageDirectory(VOID);
VOID FASTCALL FrLdrGetPaeMode(VOID);
BOOLEAN NTAPI FrLdrMapKernel(PFILE KernelImage);
ULONG_PTR NTAPI FrLdrCreateModule(LPCSTR ModuleName);
ULONG_PTR NTAPI FrLdrLoadModule(PFILE ModuleImage, LPCSTR ModuleName, PULONG ModuleSize);
BOOLEAN NTAPI FrLdrCloseModule(ULONG_PTR ModuleBase, ULONG dwModuleSize);
VOID NTAPI FrLdrStartup(ULONG Magic);
typedef VOID (NTAPI *ROS_KERNEL_ENTRY_POINT)(IN PROS_LOADER_PARAMETER_BLOCK LoaderBlock);

PVOID
NTAPI
FrLdrMapImage(
    IN PFILE Image,
    IN PCHAR ShortName,
    IN ULONG ImageType
);

PVOID
NTAPI
FrLdrReadAndMapImage(
    IN PFILE Image,
    IN PCHAR ShortName,
    IN ULONG ImageType
);

PVOID
NTAPI
FrLdrLoadImage(
    IN PCHAR szFileName,
    IN INT nPos,
    IN ULONG ImageType
);
