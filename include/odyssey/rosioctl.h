/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey PSDK extensions
 * FILE:            include/odyssey/rosioctl.h
 * PURPOSE:         Additional partition types
 *                  (partition types not covered by winioctl.h of PSDK)
 *
 * PROGRAMMERS:     Matthias Kupfer (mkupfer@odyssey.com)
 */

#ifndef __ROSIOCTL_H
#define __ROSIOCTL_H

#define PARTITION_OS2BOOTMGR          0x0A // OS/2 Boot Manager/OPUS/Coherent swap
#define PARTITION_LINUX_SWAP          0x82 // Linux Swap Partition
#define PARTITION_LINUX               0x83 // Linux Partition Ext2/Ext3/Ext4
#define PARTITION_EXT2                PARTITION_LINUX // some apps use this identifier
#define PARTITION_LINUX_LVM           0x8E

#endif /* __ROSIOCTL_H */

/* EOF */

