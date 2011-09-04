/*
 * PROJECT:         Odyssey Boot Loader
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            boot/freeldr/include/arch/arm/hardware.h
 * PURPOSE:         Header for ARC definitions (to be cleaned up)
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

#pragma once

#ifndef __REGISTRY_H
#include "../../odyssey/registry.h"
#endif

#include "../../../../../armllb/inc/osloader.h"
#include "../../../../../armllb/inc/machtype.h"

//
// ARC Component Configuration Routines
//
VOID
NTAPI
FldrCreateSystemKey(
    OUT PCONFIGURATION_COMPONENT_DATA *SystemKey
);

#define HW_MAX_ARC_HEAP_SIZE 256

extern PARM_BOARD_CONFIGURATION_BLOCK ArmBoardBlock;
extern ULONG FirstLevelDcacheSize;
extern ULONG FirstLevelDcacheFillSize;
extern ULONG FirstLevelIcacheSize;
extern ULONG FirstLevelIcacheFillSize;
extern ULONG SecondLevelDcacheSize;
extern ULONG SecondLevelDcacheFillSize;
extern ULONG SecondLevelIcacheSize;
extern ULONG SecondLevelIcacheFillSize;

extern ULONG gDiskReadBuffer, gFileSysBuffer;
#define DISKREADBUFFER gDiskReadBuffer
#define FILESYSBUFFER  gFileSysBuffer
