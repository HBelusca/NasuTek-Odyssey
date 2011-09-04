/*
 * PROJECT:         Odyssey Boot Loader
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            drivers/video/miniport/vmx_svga/precomp.h
 * PURPOSE:         VMWARE SVGA-II Driver Header
 * PROGRAMMERS:     Odyssey Portable Systems Group
 */

#include <ntdef.h>
#include <dderror.h>
#include <miniport.h>
#include <ntddvdeo.h>
#include <video.h>
#include "vmx_regs.h"

typedef struct _HW_DEVICE_EXTENSION
{
    USHORT Version;
    PHYSICAL_ADDRESS FrameBuffer;
    LARGE_INTEGER VramSize;
    PHYSICAL_ADDRESS VramBase;
    ULONG MemSize;
    PULONG IndexPort;
    PULONG ValuePort;
    PVOID FrameBufferBase;
    PVOID Fifo;
    ULONG InterruptPort;
    ULONG InterruptState;
    PENG_EVENT SyncEvent;
    VIDEO_MODE_INFORMATION CurrentMode;
    ULONG VideoModeCount;
    ULONG Capabilities;
    USHORT Flags;
    USHORT DisplayIndex;
    ULONG YOrigin;
    ULONG XOrigin;
} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;
