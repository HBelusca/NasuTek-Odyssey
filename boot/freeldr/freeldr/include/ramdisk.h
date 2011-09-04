/*
 * PROJECT:         Odyssey Boot Loader
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            boot/freeldr/include/ramdisk.h
 * PURPOSE:         Header file for ramdisk support
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

#pragma once

//
// Ramdisk Routines
//
VOID
NTAPI
RamDiskLoadVirtualFile(
    IN PCHAR FileName
);

VOID
NTAPI
RamDiskInitialize(
    VOID
);

extern PVOID gRamDiskBase;
extern ULONG gRamDiskSize;
