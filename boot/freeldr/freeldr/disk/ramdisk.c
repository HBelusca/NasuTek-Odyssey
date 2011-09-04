/*
 * PROJECT:         Odyssey Boot Loader
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            boot/freeldr/freeldr/disk/ramdisk.c
 * PURPOSE:         Implements routines to support booting from a RAM Disk
 * PROGRAMMERS:     Odyssey Portable Systems Group
 *                  Herv� Poussineau
 */

/* INCLUDES *******************************************************************/

#include <freeldr.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

PVOID gRamDiskBase;
ULONG gRamDiskSize;
ULONG gRamDiskOffset;

/* FUNCTIONS ******************************************************************/

static LONG RamDiskClose(ULONG FileId)
{
    //
    // Nothing to do
    //
    return ESUCCESS;
}

static LONG RamDiskGetFileInformation(ULONG FileId, FILEINFORMATION* Information)
{
    //
    // Give current seek offset and ram disk size to caller
    //
    RtlZeroMemory(Information, sizeof(FILEINFORMATION));
    Information->EndingAddress.LowPart = gRamDiskSize;
    Information->CurrentAddress.LowPart = gRamDiskOffset;

    return ESUCCESS;
}

static LONG RamDiskOpen(CHAR* Path, OPENMODE OpenMode, ULONG* FileId)
{
    //
    // Always return success, as contents are already in memory
    //
    return ESUCCESS;
}

static LONG RamDiskRead(ULONG FileId, VOID* Buffer, ULONG N, ULONG* Count)
{
    PVOID StartAddress;

    //
    // Get actual pointer
    //
    StartAddress = (PVOID)((ULONG_PTR)gRamDiskBase + gRamDiskOffset);

    //
    // Don't allow reads past our image
    //
    if (gRamDiskOffset + N > gRamDiskSize)
    {
        *Count = 0;
        return EIO;
    }

    //
    // Do the read
    //
    RtlCopyMemory(Buffer, StartAddress, N);
    *Count = N;

    return ESUCCESS;
}

static LONG RamDiskSeek(ULONG FileId, LARGE_INTEGER* Position, SEEKMODE SeekMode)
{
    //
    // Only accept absolute mode now
    //
    if (SeekMode != SeekAbsolute)
        return EINVAL;

    //
    // Check if we're in the ramdisk
    //
    if (Position->HighPart != 0)
        return EINVAL;
    if (Position->LowPart >= gRamDiskSize)
        return EINVAL;

    //
    // OK, remember seek position
    //
    gRamDiskOffset = Position->LowPart;

    return ESUCCESS;
}

static const DEVVTBL RamDiskVtbl = {
    RamDiskClose,
    RamDiskGetFileInformation,
    RamDiskOpen,
    RamDiskRead,
    RamDiskSeek,
};

VOID
NTAPI
RamDiskInitialize(VOID)
{
    /* Setup the RAMDISK device */
    FsRegisterDevice("ramdisk(0)", &RamDiskVtbl);
}

VOID
NTAPI
RamDiskLoadVirtualFile(IN PCHAR FileName)
{
    ULONG RamFile;
    ULONG TotalRead, ChunkSize, Count;
    PCHAR MsgBuffer = "Loading ramdisk...";
    ULONG PercentPerChunk, Percent;
    FILEINFORMATION Information;
    LARGE_INTEGER Position;
    LONG ret;

    //
    // Display progress
    //
    UiDrawProgressBarCenter(1, 100, MsgBuffer);

    //
    // Try opening the ramdisk file
    //
    ret = ArcOpen(FileName, OpenReadOnly, &RamFile);
    if (ret == ESUCCESS)
    {
        //
        // Get the file size
        //
        ret = ArcGetFileInformation(RamFile, &Information);
        if (ret != ESUCCESS)
        {
            ArcClose(RamFile);
            return;
        }

        //
        // For now, limit RAM disks to 4GB
        //
        if (Information.EndingAddress.HighPart != 0)
        {
            UiMessageBox("RAM disk too big\n");
            ArcClose(RamFile);
            return;
        }
        gRamDiskSize = Information.EndingAddress.LowPart;

        //
        // Allocate memory for it
        //
        ChunkSize = 8 * 1024 * 1024;
        if (gRamDiskSize < ChunkSize)
            Percent = PercentPerChunk = 0;
        else
            Percent = PercentPerChunk = 100 / (gRamDiskSize / ChunkSize);
        gRamDiskBase = MmAllocateMemoryWithType(gRamDiskSize, LoaderXIPRom);
        if (!gRamDiskBase)
        {
            UiMessageBox("Failed to allocate memory for RAM disk\n");
            ArcClose(RamFile);
            return;
        }

        //
        // Read it in chunks
        //
        for (TotalRead = 0; TotalRead < gRamDiskSize; TotalRead += ChunkSize)
        {
            //
            // Check if we're at the last chunk
            //
            if ((gRamDiskSize - TotalRead) < ChunkSize)
            {
                //
                // Only need the actual data required
                //
                ChunkSize = gRamDiskSize - TotalRead;
            }

            //
            // Draw progress
            //
            UiDrawProgressBarCenter(Percent, 100, MsgBuffer);
            Percent += PercentPerChunk;

            //
            // Copy the contents
            //
            Position.HighPart = 0;
            Position.LowPart = TotalRead;
            ret = ArcSeek(RamFile, &Position, SeekAbsolute);
            if (ret == ESUCCESS)
            {
                ret = ArcRead(RamFile,
                              (PVOID)((ULONG_PTR)gRamDiskBase + TotalRead),
                              ChunkSize,
                              &Count);
            }

            //
            // Check for success
            //
            if (ret != ESUCCESS || Count != ChunkSize)
            {
                MmFreeMemory(gRamDiskBase);
                gRamDiskBase = NULL;
                gRamDiskSize = 0;
                ArcClose(RamFile);
                UiMessageBox("Failed to read ramdisk\n");
                return;
            }
        }

        ArcClose(RamFile);

        // Register a new device for the ramdisk
        FsRegisterDevice("ramdisk(0)", &RamDiskVtbl);
    }
}
