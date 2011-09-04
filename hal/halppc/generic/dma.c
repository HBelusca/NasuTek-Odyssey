/* $Id: dma.c 24759 2006-11-14 20:59:48Z ion $
 *
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            ntoskrnl/hal/x86/dma.c
 * PURPOSE:         DMA functions
 * PROGRAMMERS:     David Welch (welch@mcmail.com)
 *                  Filip Navara (navaraf@odyssey.com)
 * UPDATE HISTORY:
 *                  Created 22/05/98
 */

/**
 * @page DMA Implementation Notes
 *
 * Concepts:
 *
 * - Map register
 *
 *   Abstract encapsulation of physically contiguous buffer that resides
 *   in memory accessible by both the DMA device / controller and the system.
 *   The map registers are allocated and distributed on demand and are
 *   scarce resource.
 *
 *   The actual use of map registers is to allow transfers from/to buffer
 *   located in physical memory at address inaccessible by the DMA device /
 *   controller directly. For such transfers the map register buffers
 *   are used as intermediate data storage.
 *
 * - Master adapter
 *
 *   A container for map registers (typically corresponding to one physical
 *   bus connection type). There can be master adapters for 24-bit address
 *   ranges, 32-bit address ranges, etc. Every time a new DMA adapter is
 *   created it's associated with a corresponding master adapter that
 *   is used for any map register allocation requests.
 *
 * - Bus-master / Slave DMA
 *
 *   Slave DMA is term used for DMA transfers done by the system (E)ISA
 *   controller as opposed to transfers mastered by the device itself
 *   (hence the name).
 *
 *   For slave DMA special care is taken to actually access the system
 *   controller and handle the transfers. The relevant code is in
 *   HalpDmaInitializeEisaAdapter, HalReadDmaCounter, IoFlushAdapterBuffers
 *   and IoMapTransfer.
 *
 * Implementation:
 *
 * - Allocation of map registers
 *
 *   Initial set of map registers is allocated on the system start to
 *   ensure that low memory won't get filled up later. Additional map
 *   registers are allocated as needed by HalpGrowMapBuffers. This
 *   routine is called on two places:
 *
 *   - HalGetAdapter, since we're at PASSIVE_LEVEL and it's known that
 *     more map registers will probably be needed.
 *   - IoAllocateAdapterChannel (indirectly using HalpGrowMapBufferWorker
 *     since we're at DISPATCH_LEVEL and call HalpGrowMapBuffers directly)
 *     when no more map registers are free.
 *
 *   Note that even if no more map registers can be allocated it's not
 *   the end of the world. The adapters waiting for free map registers
 *   are queued in the master adapter's queue and once one driver hands
 *   back it's map registers (using IoFreeMapRegisters or indirectly using
 *   the execution routine callback in IoAllocateAdapterChannel) the
 *   queue gets processed and the map registers are reassigned.
 */

/* INCLUDES *****************************************************************/

#include <hal.h>
#define NDEBUG
#include <debug.h>

static KEVENT HalpDmaLock;
static LIST_ENTRY HalpDmaAdapterList;
static PADAPTER_OBJECT HalpEisaAdapter[8];
static BOOLEAN HalpEisaDma;
static PADAPTER_OBJECT HalpMasterAdapter;

static const ULONG_PTR HalpEisaPortPage[8] = {
   FIELD_OFFSET(DMA_PAGE, Channel0),
   FIELD_OFFSET(DMA_PAGE, Channel1),
   FIELD_OFFSET(DMA_PAGE, Channel2),
   FIELD_OFFSET(DMA_PAGE, Channel3),
   0,
   FIELD_OFFSET(DMA_PAGE, Channel5),
   FIELD_OFFSET(DMA_PAGE, Channel6),
   FIELD_OFFSET(DMA_PAGE, Channel7)
};

static DMA_OPERATIONS HalpDmaOperations = {
   sizeof(DMA_OPERATIONS),
   (PPUT_DMA_ADAPTER)HalPutDmaAdapter,
   (PALLOCATE_COMMON_BUFFER)HalAllocateCommonBuffer,
   (PFREE_COMMON_BUFFER)HalFreeCommonBuffer,
   NULL, /* Initialized in HalpInitDma() */
   NULL, /* Initialized in HalpInitDma() */
   NULL, /* Initialized in HalpInitDma() */
   NULL, /* Initialized in HalpInitDma() */
   NULL, /* Initialized in HalpInitDma() */
   (PGET_DMA_ALIGNMENT)HalpDmaGetDmaAlignment,
   (PREAD_DMA_COUNTER)HalReadDmaCounter,
   /* FIXME: Implement the S/G funtions. */
   NULL /*(PGET_SCATTER_GATHER_LIST)HalGetScatterGatherList*/,
   NULL /*(PPUT_SCATTER_GATHER_LIST)HalPutScatterGatherList*/,
   NULL /*(PCALCULATE_SCATTER_GATHER_LIST_SIZE)HalCalculateScatterGatherListSize*/,
   NULL /*(PBUILD_SCATTER_GATHER_LIST)HalBuildScatterGatherList*/,
   NULL /*(PBUILD_MDL_FROM_SCATTER_GATHER_LIST)HalBuildMdlFromScatterGatherList*/
};

#define MAX_MAP_REGISTERS 64

#define TAG_DMA ' AMD'

/* FUNCTIONS *****************************************************************/

VOID
HalpInitDma(VOID)
{
   /*
    * Initialize the DMA Operation table
    */
   HalpDmaOperations.AllocateAdapterChannel = (PALLOCATE_ADAPTER_CHANNEL)IoAllocateAdapterChannel;
   HalpDmaOperations.FlushAdapterBuffers = (PFLUSH_ADAPTER_BUFFERS)IoFlushAdapterBuffers;
   HalpDmaOperations.FreeAdapterChannel = (PFREE_ADAPTER_CHANNEL)IoFreeAdapterChannel;
   HalpDmaOperations.FreeMapRegisters = (PFREE_MAP_REGISTERS)IoFreeMapRegisters;
   HalpDmaOperations.MapTransfer = (PMAP_TRANSFER)IoMapTransfer;

   /*
    * Check if Extended DMA is available. We're just going to do a random
    * read and write.
    */

   WRITE_PORT_UCHAR((PUCHAR)FIELD_OFFSET(EISA_CONTROL, DmaController2Pages.Channel2), 0x2A);
   if (READ_PORT_UCHAR((PUCHAR)FIELD_OFFSET(EISA_CONTROL, DmaController2Pages.Channel2)) == 0x2A)
      HalpEisaDma = TRUE;

   /*
    * Intialize all the global variables and allocate master adapter with
    * first map buffers.
    */

   InitializeListHead(&HalpDmaAdapterList);
   KeInitializeEvent(&HalpDmaLock, NotificationEvent, TRUE);

   HalpMasterAdapter = HalpDmaAllocateMasterAdapter();

   /*
    * Setup the HalDispatchTable callback for creating PnP DMA adapters. It's
    * used by IoGetDmaAdapter in the kernel.
    */

   HalGetDmaAdapter = HalpGetDmaAdapter;
}

/**
 * @name HalpGetAdapterMaximumPhysicalAddress
 *
 * Get the maximum physical address acceptable by the device represented
 * by the passed DMA adapter.
 */

PHYSICAL_ADDRESS NTAPI
HalpGetAdapterMaximumPhysicalAddress(
   IN PADAPTER_OBJECT AdapterObject)
{
   PHYSICAL_ADDRESS HighestAddress;

   if (AdapterObject->MasterDevice)
   {
      if (AdapterObject->Dma64BitAddresses)
      {
         HighestAddress.QuadPart = 0xFFFFFFFFFFFFFFFFULL;
         return HighestAddress;
      }
      else if (AdapterObject->Dma32BitAddresses)
      {
         HighestAddress.QuadPart = 0xFFFFFFFF;
         return HighestAddress;
      }
   }

   HighestAddress.QuadPart = 0xFFFFFF;
   return HighestAddress;
}

/**
 * @name HalpGrowMapBuffers
 *
 * Allocate initial, or additional, map buffers for DMA master adapter.
 *
 * @param MasterAdapter
 *        DMA master adapter to allocate buffers for.
 * @param SizeOfMapBuffers
 *        Size of the map buffers to allocate (not including the size
 *        already allocated).
 */

BOOLEAN NTAPI
HalpGrowMapBuffers(
   IN PADAPTER_OBJECT AdapterObject,
   IN ULONG SizeOfMapBuffers)
{
   PVOID VirtualAddress;
   PHYSICAL_ADDRESS PhysicalAddress;
   PHYSICAL_ADDRESS HighestAcceptableAddress;
   PHYSICAL_ADDRESS LowestAcceptableAddress;
   PHYSICAL_ADDRESS BoundryAddressMultiple;
   KIRQL OldIrql;
   ULONG MapRegisterCount;

   /* FIXME: Check if enough map register slots are available. */

   MapRegisterCount = BYTES_TO_PAGES(SizeOfMapBuffers);

   /*
    * Allocate memory for the new map registers. For 32-bit adapters we use
    * two passes in order not to waste scare resource (low memory).
    */

   HighestAcceptableAddress =
      HalpGetAdapterMaximumPhysicalAddress(AdapterObject);
   LowestAcceptableAddress.HighPart = 0;
   LowestAcceptableAddress.LowPart =
      HighestAcceptableAddress.LowPart == 0xFFFFFFFF ? 0x1000000 : 0;
   BoundryAddressMultiple.QuadPart = 0;

   VirtualAddress = MmAllocateContiguousMemorySpecifyCache(
      MapRegisterCount << PAGE_SHIFT, LowestAcceptableAddress,
      HighestAcceptableAddress, BoundryAddressMultiple, MmNonCached);

   if (VirtualAddress == NULL && LowestAcceptableAddress.LowPart != 0)
   {
      LowestAcceptableAddress.LowPart = 0;
      VirtualAddress = MmAllocateContiguousMemorySpecifyCache(
         MapRegisterCount << PAGE_SHIFT, LowestAcceptableAddress,
         HighestAcceptableAddress, BoundryAddressMultiple, MmNonCached);
   }

   if (VirtualAddress == NULL)
      return FALSE;

   PhysicalAddress = MmGetPhysicalAddress(VirtualAddress);

   /*
    * All the following must be done with the master adapter lock held
    * to prevent corruption.
    */

   OldIrql = KfAcquireSpinLock(&AdapterObject->SpinLock);

   /*
    * Setup map register entries for the buffer allocated. Each entry has
    * a virtual and physical address and corresponds to PAGE_SIZE large
    * buffer.
    */

   if (MapRegisterCount > 0)
   {
      PROS_MAP_REGISTER_ENTRY CurrentEntry, PreviousEntry;

      CurrentEntry = AdapterObject->MapRegisterBase +
                     AdapterObject->NumberOfMapRegisters;
      do
      {
         /*
          * Leave one entry free for every non-contiguous memory region
          * in the map register bitmap. This ensures that we can search
          * using RtlFindClearBits for contiguous map register regions.
          *
          * Also for non-EISA DMA leave one free entry for every 64Kb
          * break, because the DMA controller can handle only coniguous
          * 64Kb regions.
          */

         if (CurrentEntry != AdapterObject->MapRegisterBase)
         {
            PreviousEntry = CurrentEntry - 1;
            if (PreviousEntry->PhysicalAddress.LowPart + PAGE_SIZE ==
                PhysicalAddress.LowPart)
            {
               if (!HalpEisaDma)
               {
                  if ((PreviousEntry->PhysicalAddress.LowPart ^
                       PhysicalAddress.LowPart) & 0xFFFF0000)
                  {
                     CurrentEntry++;
                     AdapterObject->NumberOfMapRegisters++;
                  }
               }
            }
            else
            {
               CurrentEntry++;
               AdapterObject->NumberOfMapRegisters++;
            }
         }

         RtlClearBit(AdapterObject->MapRegisters,
                     CurrentEntry - AdapterObject->MapRegisterBase);
         CurrentEntry->VirtualAddress = VirtualAddress;
         CurrentEntry->PhysicalAddress = PhysicalAddress;

         PhysicalAddress.LowPart += PAGE_SIZE;
         VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress + PAGE_SIZE);

         CurrentEntry++;
         AdapterObject->NumberOfMapRegisters++;
         MapRegisterCount--;
      }
      while (MapRegisterCount != 0);
   }

   KfReleaseSpinLock(&AdapterObject->SpinLock, OldIrql);

   return TRUE;
}

/**
 * @name HalpDmaAllocateMasterAdapter
 *
 * Helper routine to allocate and initialize master adapter object and it's
 * associated map register buffers.
 *
 * @see HalpInitDma
 */

PADAPTER_OBJECT NTAPI
HalpDmaAllocateMasterAdapter(VOID)
{
   PADAPTER_OBJECT MasterAdapter;
   ULONG Size, SizeOfBitmap;

   SizeOfBitmap = MAX_MAP_REGISTERS;
   Size = sizeof(ADAPTER_OBJECT);
   Size += sizeof(RTL_BITMAP);
   Size += (SizeOfBitmap + 7) >> 3;

   MasterAdapter = ExAllocatePoolWithTag(NonPagedPool, Size, TAG_DMA);
   if (MasterAdapter == NULL)
      return NULL;

   RtlZeroMemory(MasterAdapter, Size);

   KeInitializeSpinLock(&MasterAdapter->SpinLock);
   InitializeListHead(&MasterAdapter->AdapterQueue);

   MasterAdapter->MapRegisters = (PVOID)(MasterAdapter + 1);
   RtlInitializeBitMap(
      MasterAdapter->MapRegisters,
      (PULONG)(MasterAdapter->MapRegisters + 1),
      SizeOfBitmap);
   RtlSetAllBits(MasterAdapter->MapRegisters);
   MasterAdapter->NumberOfMapRegisters = 0;
   MasterAdapter->CommittedMapRegisters = 0;

   MasterAdapter->MapRegisterBase = ExAllocatePoolWithTag(
      NonPagedPool,
      SizeOfBitmap * sizeof(ROS_MAP_REGISTER_ENTRY),
      TAG_DMA);
   if (MasterAdapter->MapRegisterBase == NULL)
   {
      ExFreePool(MasterAdapter);
      return NULL;
   }

   RtlZeroMemory(MasterAdapter->MapRegisterBase,
                 SizeOfBitmap * sizeof(ROS_MAP_REGISTER_ENTRY));
   if (!HalpGrowMapBuffers(MasterAdapter, 0x10000))
   {
      ExFreePool(MasterAdapter);
      return NULL;
   }

   return MasterAdapter;
}

/**
 * @name HalpDmaAllocateChildAdapter
 *
 * Helper routine of HalGetAdapter. Allocate child adapter object and
 * fill out some basic fields.
 *
 * @see HalGetAdapter
 */

PADAPTER_OBJECT NTAPI
HalpDmaAllocateChildAdapter(
   ULONG NumberOfMapRegisters,
   PDEVICE_DESCRIPTION DeviceDescription)
{
   PADAPTER_OBJECT AdapterObject;
   OBJECT_ATTRIBUTES ObjectAttributes;
   NTSTATUS Status;
   HANDLE Handle;

   InitializeObjectAttributes(
      &ObjectAttributes,
      NULL,
      OBJ_KERNEL_HANDLE | OBJ_PERMANENT,
      NULL,
      NULL);

   Status = ObCreateObject(
      KernelMode,
      IoAdapterObjectType,
      &ObjectAttributes,
      KernelMode,
      NULL,
      sizeof(ADAPTER_OBJECT),
      0,
      0,
      (PVOID)&AdapterObject);
   if (!NT_SUCCESS(Status))
      return NULL;

   Status = ObReferenceObjectByPointer(
      AdapterObject,
      FILE_READ_DATA | FILE_WRITE_DATA,
      IoAdapterObjectType,
      KernelMode);
   if (!NT_SUCCESS(Status))
      return NULL;

   RtlZeroMemory(AdapterObject, sizeof(ADAPTER_OBJECT));

   Status = ObInsertObject(
      AdapterObject,
      NULL,
      FILE_READ_DATA | FILE_WRITE_DATA,
      0,
      NULL,
      &Handle);
   if (!NT_SUCCESS(Status))
      return NULL;

   ZwClose(Handle);

   AdapterObject->DmaHeader.Version = (USHORT)DeviceDescription->Version;
   AdapterObject->DmaHeader.Size = sizeof(ADAPTER_OBJECT);
   AdapterObject->DmaHeader.DmaOperations = &HalpDmaOperations;
   AdapterObject->MapRegistersPerChannel = 1;
   AdapterObject->Dma32BitAddresses = DeviceDescription->Dma32BitAddresses;
   AdapterObject->ChannelNumber = 0xFF;
   AdapterObject->MasterAdapter = HalpMasterAdapter;
   KeInitializeDeviceQueue(&AdapterObject->ChannelWaitQueue);

   return AdapterObject;
}

/**
 * @name HalpDmaInitializeEisaAdapter
 *
 * Setup DMA modes and extended modes for (E)ISA DMA adapter object.
 */

BOOLEAN NTAPI
HalpDmaInitializeEisaAdapter(
   PADAPTER_OBJECT AdapterObject,
   PDEVICE_DESCRIPTION DeviceDescription)
{
   UCHAR Controller;
   DMA_MODE DmaMode = {{0 }};
   DMA_EXTENDED_MODE ExtendedMode = {{ 0 }};
   PVOID AdapterBaseVa;

   Controller = (DeviceDescription->DmaChannel & 4) ? 2 : 1;

   if (Controller == 1)
      AdapterBaseVa = (PVOID)FIELD_OFFSET(EISA_CONTROL, DmaController1);
   else
      AdapterBaseVa = (PVOID)FIELD_OFFSET(EISA_CONTROL, DmaController2);

   AdapterObject->AdapterNumber = Controller;
   AdapterObject->ChannelNumber = (UCHAR)(DeviceDescription->DmaChannel & 3);
   AdapterObject->PagePort = (PUCHAR)HalpEisaPortPage[DeviceDescription->DmaChannel];
   AdapterObject->Width16Bits = FALSE;
   AdapterObject->AdapterBaseVa = AdapterBaseVa;

   if (HalpEisaDma)
   {
      ExtendedMode.ChannelNumber = AdapterObject->ChannelNumber;

      switch (DeviceDescription->DmaSpeed)
      {
         case Compatible: ExtendedMode.TimingMode = COMPATIBLE_TIMING; break;
         case TypeA: ExtendedMode.TimingMode = TYPE_A_TIMING; break;
         case TypeB: ExtendedMode.TimingMode = TYPE_B_TIMING; break;
         case TypeC: ExtendedMode.TimingMode = BURST_TIMING; break;
         default:
            return FALSE;
      }

      switch (DeviceDescription->DmaWidth)
      {
         case Width8Bits: ExtendedMode.TransferSize = B_8BITS; break;
         case Width16Bits: ExtendedMode.TransferSize = B_16BITS; break;
         case Width32Bits: ExtendedMode.TransferSize = B_32BITS; break;
         default:
            return FALSE;
      }

      if (Controller == 1)
         WRITE_PORT_UCHAR((PUCHAR)FIELD_OFFSET(EISA_CONTROL, DmaExtendedMode1),
                          ExtendedMode.Byte);
      else
         WRITE_PORT_UCHAR((PUCHAR)FIELD_OFFSET(EISA_CONTROL, DmaExtendedMode2),
                          ExtendedMode.Byte);
   }
   else
   {
      /*
       * Validate setup for non-busmaster DMA adapter. Secondary controller
       * supports only 16-bit transfers and main controller supports only
       * 8-bit transfers. Anything else is invalid.
       */

      if (!DeviceDescription->Master)
      {
         if (Controller == 2 && DeviceDescription->DmaWidth == Width16Bits)
            AdapterObject->Width16Bits = TRUE;
         else if (Controller != 1 || DeviceDescription->DmaWidth != Width8Bits)
            return FALSE;
      }
   }

   DmaMode.Channel = AdapterObject->ChannelNumber;
   DmaMode.AutoInitialize = DeviceDescription->AutoInitialize;

   /*
    * Set the DMA request mode.
    *
    * For (E)ISA bus master devices just unmask (enable) the DMA channel
    * and set it to cascade mode. Otherwise just select the right one
    * bases on the passed device description.
    */

   if (DeviceDescription->Master)
   {
      DmaMode.RequestMode = CASCADE_REQUEST_MODE;
      if (Controller == 1)
      {
         /* Set the Request Data */
         WRITE_PORT_UCHAR(&((PDMA1_CONTROL)AdapterBaseVa)->Mode,
                          DmaMode.Byte);
         /* Unmask DMA Channel */
         WRITE_PORT_UCHAR(&((PDMA1_CONTROL)AdapterBaseVa)->SingleMask,
                          AdapterObject->ChannelNumber | DMA_CLEARMASK);
      } else {
         /* Set the Request Data */
         WRITE_PORT_UCHAR(&((PDMA2_CONTROL)AdapterBaseVa)->Mode,
                          DmaMode.Byte);
         /* Unmask DMA Channel */
         WRITE_PORT_UCHAR(&((PDMA2_CONTROL)AdapterBaseVa)->SingleMask,
                          AdapterObject->ChannelNumber | DMA_CLEARMASK);
      }
   }
   else
   {
      if (DeviceDescription->DemandMode)
         DmaMode.RequestMode = DEMAND_REQUEST_MODE;
      else
         DmaMode.RequestMode = SINGLE_REQUEST_MODE;
   }

   AdapterObject->AdapterMode = DmaMode;

   return TRUE;
}

/**
 * @name HalGetAdapter
 *
 * Allocate an adapter object for DMA device.
 *
 * @param DeviceDescription
 *        Structure describing the attributes of the device.
 * @param NumberOfMapRegisters
 *        On return filled with the maximum number of map registers the
 *        device driver can allocate for DMA transfer operations.
 *
 * @return The DMA adapter on success, NULL otherwise.
 *
 * @implemented
 */

PADAPTER_OBJECT NTAPI
HalGetAdapter(
   PDEVICE_DESCRIPTION DeviceDescription,
   PULONG NumberOfMapRegisters)
{
   PADAPTER_OBJECT AdapterObject = NULL;
   PADAPTER_OBJECT MasterAdapter;
   BOOLEAN EisaAdapter;
   ULONG MapRegisters;
   ULONG MaximumLength;

   /* Validate parameters in device description */
   if (DeviceDescription->Version > DEVICE_DESCRIPTION_VERSION2)
      return NULL;

   /*
    * See if we're going to use ISA/EISA DMA adapter. These adapters are
    * special since they're reused.
    *
    * Also note that we check for channel number since there are only 8 DMA
    * channels on ISA, so any request above this requires new adapter.
    */

   if (DeviceDescription->InterfaceType == Isa || !DeviceDescription->Master)
   {
      if (DeviceDescription->InterfaceType == Isa &&
          DeviceDescription->DmaChannel >= 8)
         EisaAdapter = FALSE;
      else
         EisaAdapter = TRUE;
   }
   else
   {
      EisaAdapter = FALSE;
   }

   /*
    * Disallow creating adapter for ISA/EISA DMA channel 4 since it's used
    * for cascading the controllers and it's not available for software use.
    */

   if (EisaAdapter && DeviceDescription->DmaChannel == 4)
      return NULL;

   /*
    * Calculate the number of map registers.
    *
    * - For EISA and PCI scatter/gather no map registers are needed.
    * - For ISA slave scatter/gather one map register is needed.
    * - For all other cases the number of map registers depends on
    *   DeviceDescription->MaximumLength.
    */

   MaximumLength = DeviceDescription->MaximumLength & MAXLONG;
   if (DeviceDescription->ScatterGather &&
       (DeviceDescription->InterfaceType == Eisa ||
        DeviceDescription->InterfaceType == PCIBus))
   {
      MapRegisters = 0;
   }
   else if (DeviceDescription->ScatterGather &&
            !DeviceDescription->Master)
   {
      MapRegisters = 1;
   }
   else
   {
      /*
       * In the equation below the additional map register added by
       * the "+1" accounts for the case when a transfer does not start
       * at a page-aligned address.
       */
      MapRegisters = BYTES_TO_PAGES(MaximumLength) + 1;
      if (MapRegisters > 16)
         MapRegisters = 16;
   }

   /*
    * Acquire the DMA lock that is used to protect adapter lists and
    * EISA adapter array.
    */

   KeWaitForSingleObject(&HalpDmaLock, Executive, KernelMode,
                         FALSE, NULL);

   /*
    * Now we must get ahold of the adapter object. For first eight ISA/EISA
    * channels there are static adapter objects that are reused and updated
    * on succesive HalGetAdapter calls. In other cases a new adapter object
    * is always created and it's to the DMA adapter list (HalpDmaAdapterList).
    */

   if (EisaAdapter)
   {
      AdapterObject = HalpEisaAdapter[DeviceDescription->DmaChannel];
      if (AdapterObject != NULL)
      {
         if (AdapterObject->NeedsMapRegisters &&
             MapRegisters > AdapterObject->MapRegistersPerChannel)
            AdapterObject->MapRegistersPerChannel = MapRegisters;
      }
   }

   if (AdapterObject == NULL)
   {
      AdapterObject = HalpDmaAllocateChildAdapter(
         MapRegisters, DeviceDescription);
      if (AdapterObject == NULL)
      {
         KeSetEvent(&HalpDmaLock, 0, 0);
         return NULL;
      }

      if (EisaAdapter)
      {
         HalpEisaAdapter[DeviceDescription->DmaChannel] = AdapterObject;
      }

      if (MapRegisters > 0)
      {
         AdapterObject->NeedsMapRegisters = TRUE;
         MasterAdapter = HalpMasterAdapter;
         AdapterObject->MapRegistersPerChannel = MapRegisters;

         /*
          * FIXME: Verify that the following makes sense. Actually
          * MasterAdapter->NumberOfMapRegisters contains even the number
          * of gaps, so this will not work correctly all the time. It
          * doesn't matter much since it's only optimization to avoid
          * queuing work items in HalAllocateAdapterChannel.
          */

         MasterAdapter->CommittedMapRegisters += MapRegisters;
         if (MasterAdapter->CommittedMapRegisters > MasterAdapter->NumberOfMapRegisters)
            HalpGrowMapBuffers(MasterAdapter, 0x10000);
      }
      else
      {
         AdapterObject->NeedsMapRegisters = FALSE;
         if (DeviceDescription->Master)
            AdapterObject->MapRegistersPerChannel = BYTES_TO_PAGES(MaximumLength) + 1;
         else
            AdapterObject->MapRegistersPerChannel = 1;
      }
   }

   if (!EisaAdapter)
      InsertTailList(&HalpDmaAdapterList, &AdapterObject->AdapterList);

   /*
    * Release the DMA lock. HalpDmaAdapterList and HalpEisaAdapter will
    * no longer be touched, so we don't need it.
    */

   KeSetEvent(&HalpDmaLock, 0, 0);

   /*
    * Setup the values in the adapter object that are common for all
    * types of buses.
    */

   if (DeviceDescription->Version >= DEVICE_DESCRIPTION_VERSION1)
      AdapterObject->IgnoreCount = DeviceDescription->IgnoreCount;
   else
      AdapterObject->IgnoreCount = 0;

   AdapterObject->Dma32BitAddresses = DeviceDescription->Dma32BitAddresses;
   AdapterObject->Dma64BitAddresses = DeviceDescription->Dma64BitAddresses;
   AdapterObject->ScatterGather = DeviceDescription->ScatterGather;
   AdapterObject->MasterDevice = DeviceDescription->Master;
   *NumberOfMapRegisters = AdapterObject->MapRegistersPerChannel;

   /*
    * For non-(E)ISA adapters we have already done all the work. On the
    * other hand for (E)ISA adapters we must still setup the DMA modes
    * and prepare the controller.
    */

   if (EisaAdapter)
   {
      if (!HalpDmaInitializeEisaAdapter(AdapterObject, DeviceDescription))
      {
         ObDereferenceObject(AdapterObject);
         return NULL;
      }
   }

   return AdapterObject;
}

/**
 * @name HalpGetDmaAdapter
 *
 * Internal routine to allocate PnP DMA adapter object. It's exported through
 * HalDispatchTable and used by IoGetDmaAdapter.
 *
 * @see HalGetAdapter
 */

PDMA_ADAPTER NTAPI
HalpGetDmaAdapter(
   IN PVOID Context,
   IN PDEVICE_DESCRIPTION DeviceDescription,
   OUT PULONG NumberOfMapRegisters)
{
   return &HalGetAdapter(DeviceDescription, NumberOfMapRegisters)->DmaHeader;
}

/**
 * @name HalPutDmaAdapter
 *
 * Internal routine to free DMA adapter and resources for reuse. It's exported
 * using the DMA_OPERATIONS interface by HalGetAdapter.
 *
 * @see HalGetAdapter
 */

VOID NTAPI
HalPutDmaAdapter(
   PADAPTER_OBJECT AdapterObject)
{
   if (AdapterObject->ChannelNumber == 0xFF)
   {
      KeWaitForSingleObject(&HalpDmaLock, Executive, KernelMode,
                            FALSE, NULL);
      RemoveEntryList(&AdapterObject->AdapterList);
      KeSetEvent(&HalpDmaLock, 0, 0);
   }

   ObDereferenceObject(AdapterObject);
}

/**
 * @name HalAllocateCommonBuffer
 *
 * Allocates memory that is visible to both the processor(s) and the DMA
 * device.
 *
 * @param AdapterObject
 *        Adapter object representing the bus master or system dma controller.
 * @param Length
 *        Number of bytes to allocate.
 * @param LogicalAddress
 *        Logical address the driver can use to access the buffer.
 * @param CacheEnabled
 *        Specifies if the memory can be cached.
 *
 * @return The base virtual address of the memory allocated or NULL on failure.
 *
 * @remarks
 *    On real NT x86 systems the CacheEnabled parameter is ignored, we honour
 *    it. If it proves to cause problems change it.
 *
 * @see HalFreeCommonBuffer
 *
 * @implemented
 */

PVOID NTAPI
HalAllocateCommonBuffer(
   PADAPTER_OBJECT AdapterObject,
   ULONG Length,
   PPHYSICAL_ADDRESS LogicalAddress,
   BOOLEAN CacheEnabled)
{
   PHYSICAL_ADDRESS LowestAcceptableAddress;
   PHYSICAL_ADDRESS HighestAcceptableAddress;
   PHYSICAL_ADDRESS BoundryAddressMultiple;
   PVOID VirtualAddress;

   LowestAcceptableAddress.QuadPart = 0;
   HighestAcceptableAddress =
      HalpGetAdapterMaximumPhysicalAddress(AdapterObject);
   BoundryAddressMultiple.QuadPart = 0;

   /*
    * For bus-master DMA devices the buffer mustn't cross 4Gb boundary. For
    * slave DMA devices the 64Kb boundary mustn't be crossed since the
    * controller wouldn't be able to handle it.
    */

   if (AdapterObject->MasterDevice)
      BoundryAddressMultiple.HighPart = 1;
   else
      BoundryAddressMultiple.LowPart = 0x10000;

   VirtualAddress = MmAllocateContiguousMemorySpecifyCache(
      Length, LowestAcceptableAddress, HighestAcceptableAddress,
      BoundryAddressMultiple, CacheEnabled ? MmCached : MmNonCached);
   if (VirtualAddress == NULL)
      return NULL;

   *LogicalAddress = MmGetPhysicalAddress(VirtualAddress);

   return VirtualAddress;
}

/**
 * @name HalFreeCommonBuffer
 *
 * Free common buffer allocated with HalAllocateCommonBuffer.
 *
 * @see HalAllocateCommonBuffer
 *
 * @implemented
 */

VOID NTAPI
HalFreeCommonBuffer(
   PADAPTER_OBJECT AdapterObject,
   ULONG Length,
   PHYSICAL_ADDRESS LogicalAddress,
   PVOID VirtualAddress,
   BOOLEAN CacheEnabled)
{
   MmFreeContiguousMemory(VirtualAddress);
}

/**
 * @name HalpDmaGetDmaAlignment
 *
 * Internal routine to return the DMA alignment requirement. It's exported
 * using the DMA_OPERATIONS interface by HalGetAdapter.
 *
 * @see HalGetAdapter
 */

ULONG NTAPI
HalpDmaGetDmaAlignment(
   PADAPTER_OBJECT AdapterObject)
{
   return 1;
}

/*
 * @name HalReadDmaCounter
 *
 * Read DMA operation progress counter.
 *
 * @implemented
 */

ULONG NTAPI
HalReadDmaCounter(
   PADAPTER_OBJECT AdapterObject)
{
   KIRQL OldIrql;
   ULONG Count, OldCount;

   ASSERT(!AdapterObject->MasterDevice);

   /*
    * Acquire the master adapter lock since we're going to mess with the
    * system DMA controller registers and we really don't want anyone
    * to do the same at the same time.
    */

   KeAcquireSpinLock(&AdapterObject->MasterAdapter->SpinLock, &OldIrql);

   /* Send the request to the specific controller. */
   if (AdapterObject->AdapterNumber == 1)
   {
      PDMA1_CONTROL DmaControl1 = AdapterObject->AdapterBaseVa;

      Count = 0xffff00;
      do
      {
         OldCount = Count;
         /* Send Reset */
         WRITE_PORT_UCHAR(&DmaControl1->ClearBytePointer, 0);
         /* Read Count */
         Count = READ_PORT_UCHAR(&DmaControl1->DmaAddressCount
                                 [AdapterObject->ChannelNumber].DmaBaseCount);
         Count |= READ_PORT_UCHAR(&DmaControl1->DmaAddressCount
                                  [AdapterObject->ChannelNumber].DmaBaseCount) << 8;
      }
      while (0xffff00 & (OldCount ^ Count));
   }
   else
   {
      PDMA2_CONTROL DmaControl2 = AdapterObject->AdapterBaseVa;

      Count = 0xffff00;
      do
      {
         OldCount = Count;
         /* Send Reset */
         WRITE_PORT_UCHAR(&DmaControl2->ClearBytePointer, 0);
         /* Read Count */
         Count = READ_PORT_UCHAR(&DmaControl2->DmaAddressCount
                                 [AdapterObject->ChannelNumber].DmaBaseCount);
         Count |= READ_PORT_UCHAR(&DmaControl2->DmaAddressCount
                                  [AdapterObject->ChannelNumber].DmaBaseCount) << 8;
      }
      while (0xffff00 & (OldCount ^ Count));
   }

   KeReleaseSpinLock(&AdapterObject->MasterAdapter->SpinLock, OldIrql);

   Count++;
   Count &= 0xffff;
   if (AdapterObject->Width16Bits)
      Count *= 2;

   return Count;
}

/**
 * @name HalpGrowMapBufferWorker
 *
 * Helper routine of HalAllocateAdapterChannel for allocating map registers
 * at PASSIVE_LEVEL in work item.
 */

VOID NTAPI
HalpGrowMapBufferWorker(PVOID DeferredContext)
{
   PGROW_WORK_ITEM WorkItem = (PGROW_WORK_ITEM)DeferredContext;
   KIRQL OldIrql;
   BOOLEAN Succeeded;

   /*
    * Try to allocate new map registers for the adapter.
    *
    * NOTE: The NT implementation actually tries to allocate more map
    * registers than needed as an optimization.
    */

   KeWaitForSingleObject(&HalpDmaLock, Executive, KernelMode,
                         FALSE, NULL);
   Succeeded = HalpGrowMapBuffers(WorkItem->AdapterObject->MasterAdapter,
                                  WorkItem->NumberOfMapRegisters);
   KeSetEvent(&HalpDmaLock, 0, 0);

   if (Succeeded)
   {
      /*
       * Flush the adapter queue now that new map registers are ready. The
       * easiest way to do that is to call IoFreeMapRegisters to not free
       * any registers. Note that we use the magic (PVOID)2 map register
       * base to bypass the parameter checking.
       */

      KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
      IoFreeMapRegisters(WorkItem->AdapterObject, (PVOID)2, 0);
      KeLowerIrql(OldIrql);
   }

   ExFreePool(WorkItem);
}

/**
 * @name HalAllocateAdapterChannel
 *
 * Setup map registers for an adapter object.
 *
 * @param AdapterObject
 *        Pointer to an ADAPTER_OBJECT to set up.
 * @param WaitContextBlock
 *        Context block to be used with ExecutionRoutine.
 * @param NumberOfMapRegisters
 *        Number of map registers requested.
 * @param ExecutionRoutine
 *        Callback to call when map registers are allocated.
 *
 * @return
 *    If not enough map registers can be allocated then
 *    STATUS_INSUFFICIENT_RESOURCES is returned. If the function
 *    succeeds or the callback is queued for later delivering then
 *    STATUS_SUCCESS is returned.
 *
 * @see IoFreeAdapterChannel
 *
 * @implemented
 */

NTSTATUS NTAPI
HalAllocateAdapterChannel(
   PADAPTER_OBJECT AdapterObject,
   PWAIT_CONTEXT_BLOCK WaitContextBlock,
   ULONG NumberOfMapRegisters,
   PDRIVER_CONTROL ExecutionRoutine)
{
   PADAPTER_OBJECT MasterAdapter;
   PGROW_WORK_ITEM WorkItem;
   ULONG Index = MAXULONG;
   ULONG Result;
   KIRQL OldIrql;

   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   /* Set up the wait context block in case we can't run right away. */
   WaitContextBlock->DeviceRoutine = ExecutionRoutine;
   WaitContextBlock->NumberOfMapRegisters = NumberOfMapRegisters;

   /* Returns true if queued, else returns false and sets the queue to busy */
   if (KeInsertDeviceQueue(&AdapterObject->ChannelWaitQueue, &WaitContextBlock->WaitQueueEntry))
      return STATUS_SUCCESS;

   MasterAdapter = AdapterObject->MasterAdapter;

   AdapterObject->NumberOfMapRegisters = NumberOfMapRegisters;
   AdapterObject->CurrentWcb = WaitContextBlock;

   if (NumberOfMapRegisters && AdapterObject->NeedsMapRegisters)
   {
      if (NumberOfMapRegisters > AdapterObject->MapRegistersPerChannel)
      {
         AdapterObject->NumberOfMapRegisters = 0;
         IoFreeAdapterChannel(AdapterObject);
         return STATUS_INSUFFICIENT_RESOURCES;
      }

      /*
       * Get the map registers. This is partly complicated by the fact
       * that new map registers can only be allocated at PASSIVE_LEVEL
       * and we're currently at DISPATCH_LEVEL. The following code has
       * two code paths:
       *
       * - If there is no adapter queued for map register allocation,
       *   try to see if enough contiguous map registers are present.
       *   In case they're we can just get them and proceed further.
       *
       * - If some adapter is already present in the queue we must
       *   respect the order of adapters asking for map registers and
       *   so the fast case described above can't take place.
       *   This case is also entered if not enough coniguous map
       *   registers are present.
       *
       *   A work queue item is allocated and queued, the adapter is
       *   also queued into the master adapter queue. The worker
       *   routine does the job of allocating the map registers at
       *   PASSIVE_LEVEL and calling the ExecutionRoutine.
       */

      OldIrql = KfAcquireSpinLock(&MasterAdapter->SpinLock);

      if (IsListEmpty(&MasterAdapter->AdapterQueue))
      {
         Index = RtlFindClearBitsAndSet(
            MasterAdapter->MapRegisters, NumberOfMapRegisters, 0);
         if (Index != MAXULONG)
         {
            AdapterObject->MapRegisterBase =
               MasterAdapter->MapRegisterBase + Index;
            if (!AdapterObject->ScatterGather)
            {
               AdapterObject->MapRegisterBase =
                  (PROS_MAP_REGISTER_ENTRY)(
                     (ULONG_PTR)AdapterObject->MapRegisterBase |
                     MAP_BASE_SW_SG);
            }
         }
      }

      if (Index == MAXULONG)
      {
         WorkItem = ExAllocatePoolWithTag(
            NonPagedPool, sizeof(GROW_WORK_ITEM), TAG_DMA);
         if (WorkItem == NULL)
         {
            KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);
            AdapterObject->NumberOfMapRegisters = 0;
            IoFreeAdapterChannel(AdapterObject);
            return STATUS_INSUFFICIENT_RESOURCES;
         }

         InsertTailList(&MasterAdapter->AdapterQueue, &AdapterObject->AdapterQueue);

         ExInitializeWorkItem(
            &WorkItem->WorkQueueItem, HalpGrowMapBufferWorker, WorkItem);
         WorkItem->AdapterObject = AdapterObject;
         WorkItem->NumberOfMapRegisters = NumberOfMapRegisters;

         ExQueueWorkItem(&WorkItem->WorkQueueItem, DelayedWorkQueue);

         KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);

         return STATUS_SUCCESS;
      }

      KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);
   }
   else
   {
      AdapterObject->MapRegisterBase = NULL;
      AdapterObject->NumberOfMapRegisters = 0;
   }

   AdapterObject->CurrentWcb = WaitContextBlock;

   Result = ExecutionRoutine(
      WaitContextBlock->DeviceObject, WaitContextBlock->CurrentIrp,
      AdapterObject->MapRegisterBase, WaitContextBlock->DeviceContext);

   /*
    * Possible return values:
    *
    * - KeepObject
    *   Don't free any resources, the ADAPTER_OBJECT is still in use and
    *   the caller will call IoFreeAdapterChannel later.
    *
    * - DeallocateObject
    *   Deallocate the map registers and release the ADAPTER_OBJECT, so
    *   someone else can use it.
    *
    * - DeallocateObjectKeepRegisters
    *   Release the ADAPTER_OBJECT, but hang on to the map registers. The
    *   client will later call IoFreeMapRegisters.
    *
    * NOTE:
    * IoFreeAdapterChannel runs the queue, so it must be called unless
    * the adapter object is not to be freed.
    */

   if (Result == DeallocateObject)
   {
      IoFreeAdapterChannel(AdapterObject);
   }
   else if (Result == DeallocateObjectKeepRegisters)
   {
      AdapterObject->NumberOfMapRegisters = 0;
      IoFreeAdapterChannel(AdapterObject);
   }

   return STATUS_SUCCESS;
}

/**
 * @name IoFreeAdapterChannel
 *
 * Free DMA resources allocated by IoAllocateAdapterChannel.
 *
 * @param AdapterObject
 *        Adapter object with resources to free.
 *
 * @remarks
 *    This function releases map registers registers assigned to the DMA
 *    adapter. After releasing the adapter, it checks the adapter's queue
 *    and runs each queued device object in series until the queue is
 *    empty. This is the only way the device queue is emptied.
 *
 * @see IoAllocateAdapterChannel
 *
 * @implemented
 */

VOID NTAPI
IoFreeAdapterChannel(
   PADAPTER_OBJECT AdapterObject)
{
   PADAPTER_OBJECT MasterAdapter;
   PKDEVICE_QUEUE_ENTRY DeviceQueueEntry;
   PWAIT_CONTEXT_BLOCK WaitContextBlock;
   ULONG Index = MAXULONG;
   ULONG Result;
   KIRQL OldIrql;

   MasterAdapter = AdapterObject->MasterAdapter;

   for (;;)
   {
      /*
       * To keep map registers, call here with AdapterObject->
       * NumberOfMapRegisters set to zero. This trick is used in
       * HalAllocateAdapterChannel for example.
       */
      if (AdapterObject->NumberOfMapRegisters)
      {
         IoFreeMapRegisters(
            AdapterObject,
            AdapterObject->MapRegisterBase,
            AdapterObject->NumberOfMapRegisters);
      }

      DeviceQueueEntry = KeRemoveDeviceQueue(&AdapterObject->ChannelWaitQueue);
      if (DeviceQueueEntry == NULL)
      {
         break;
      }

      WaitContextBlock = CONTAINING_RECORD(
         DeviceQueueEntry,
         WAIT_CONTEXT_BLOCK,
         WaitQueueEntry);

      AdapterObject->CurrentWcb = WaitContextBlock;
      AdapterObject->NumberOfMapRegisters = WaitContextBlock->NumberOfMapRegisters;

      if (WaitContextBlock->NumberOfMapRegisters &&
          AdapterObject->MasterAdapter)
      {
         OldIrql = KfAcquireSpinLock(&MasterAdapter->SpinLock);

         if (IsListEmpty(&MasterAdapter->AdapterQueue))
         {
            Index = RtlFindClearBitsAndSet(
               MasterAdapter->MapRegisters,
               WaitContextBlock->NumberOfMapRegisters, 0);
            if (Index != MAXULONG)
            {
               AdapterObject->MapRegisterBase =
                  MasterAdapter->MapRegisterBase + Index;
               if (!AdapterObject->ScatterGather)
               {
                  AdapterObject->MapRegisterBase =
                     (PROS_MAP_REGISTER_ENTRY)(
                        (ULONG_PTR)AdapterObject->MapRegisterBase |
                        MAP_BASE_SW_SG);
               }
            }
         }

         if (Index == MAXULONG)
         {
            InsertTailList(&MasterAdapter->AdapterQueue, &AdapterObject->AdapterQueue);
            KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);
            break;
         }

         KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);
      }
      else
      {
         AdapterObject->MapRegisterBase = NULL;
         AdapterObject->NumberOfMapRegisters = 0;
      }

      /* Call the adapter control routine. */
      Result = ((PDRIVER_CONTROL)WaitContextBlock->DeviceRoutine)(
          WaitContextBlock->DeviceObject, WaitContextBlock->CurrentIrp,
          AdapterObject->MapRegisterBase, WaitContextBlock->DeviceContext);

      switch (Result)
      {
         case KeepObject:
            /*
             * We're done until the caller manually calls IoFreeAdapterChannel
             * or IoFreeMapRegisters.
             */
            return;

         case DeallocateObjectKeepRegisters:
            /*
             * Hide the map registers so they aren't deallocated next time
             * around.
             */
            AdapterObject->NumberOfMapRegisters = 0;
            break;

         default:
            break;
      }
   }
}

/**
 * @name IoFreeMapRegisters
 *
 * Free map registers reserved by the system for a DMA.
 *
 * @param AdapterObject
 *        DMA adapter to free map registers on.
 * @param MapRegisterBase
 *        Handle to map registers to free.
 * @param NumberOfRegisters
 *        Number of map registers to be freed.
 *
 * @implemented
 */

VOID NTAPI
IoFreeMapRegisters(
   IN PADAPTER_OBJECT AdapterObject,
   IN PVOID MapRegisterBase,
   IN ULONG NumberOfMapRegisters)
{
   PADAPTER_OBJECT MasterAdapter = AdapterObject->MasterAdapter;
   PLIST_ENTRY ListEntry;
   KIRQL OldIrql;
   ULONG Index;
   ULONG Result;

   ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

   if (MasterAdapter == NULL || MapRegisterBase == NULL)
      return;

   OldIrql = KfAcquireSpinLock(&MasterAdapter->SpinLock);

   if (NumberOfMapRegisters != 0)
   {
      PROS_MAP_REGISTER_ENTRY RealMapRegisterBase;

      RealMapRegisterBase =
         (PROS_MAP_REGISTER_ENTRY)((ULONG_PTR)MapRegisterBase & ~MAP_BASE_SW_SG);
      RtlClearBits(MasterAdapter->MapRegisters,
                   RealMapRegisterBase - MasterAdapter->MapRegisterBase,
                   NumberOfMapRegisters);
   }

   /*
    * Now that we freed few map registers it's time to look at the master
    * adapter queue and see if there is someone waiting for map registers.
    */

   while (!IsListEmpty(&MasterAdapter->AdapterQueue))
   {
      ListEntry = RemoveHeadList(&MasterAdapter->AdapterQueue);
      AdapterObject = CONTAINING_RECORD(
         ListEntry, struct _ADAPTER_OBJECT, AdapterQueue);

      Index = RtlFindClearBitsAndSet(
         MasterAdapter->MapRegisters,
         AdapterObject->NumberOfMapRegisters,
         MasterAdapter->NumberOfMapRegisters);
      if (Index == MAXULONG)
      {
         InsertHeadList(&MasterAdapter->AdapterQueue, ListEntry);
         break;
      }

      KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);

      AdapterObject->MapRegisterBase =
         MasterAdapter->MapRegisterBase + Index;
      if (!AdapterObject->ScatterGather)
      {
         AdapterObject->MapRegisterBase =
            (PROS_MAP_REGISTER_ENTRY)(
               (ULONG_PTR)AdapterObject->MapRegisterBase |
               MAP_BASE_SW_SG);
      }

      Result = ((PDRIVER_CONTROL)AdapterObject->CurrentWcb->DeviceRoutine)(
         AdapterObject->CurrentWcb->DeviceObject,
         AdapterObject->CurrentWcb->CurrentIrp,
         AdapterObject->MapRegisterBase,
         AdapterObject->CurrentWcb->DeviceContext);

      switch (Result)
      {
         case DeallocateObjectKeepRegisters:
            AdapterObject->NumberOfMapRegisters = 0;
            /* fall through */

         case DeallocateObject:
            if (AdapterObject->NumberOfMapRegisters)
            {
               OldIrql = KfAcquireSpinLock(&MasterAdapter->SpinLock);
               RtlClearBits(MasterAdapter->MapRegisters,
                            AdapterObject->MapRegisterBase -
                            MasterAdapter->MapRegisterBase,
                            AdapterObject->NumberOfMapRegisters);
               KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);
            }
            IoFreeAdapterChannel(AdapterObject);
            break;

         default:
            break;
      }

      OldIrql = KfAcquireSpinLock(&MasterAdapter->SpinLock);
   }

   KfReleaseSpinLock(&MasterAdapter->SpinLock, OldIrql);
}

/**
 * @name HalpCopyBufferMap
 *
 * Helper function for copying data from/to map register buffers.
 *
 * @see IoFlushAdapterBuffers, IoMapTransfer
 */

VOID NTAPI
HalpCopyBufferMap(
   PMDL Mdl,
   PROS_MAP_REGISTER_ENTRY MapRegisterBase,
   PVOID CurrentVa,
   ULONG Length,
   BOOLEAN WriteToDevice)
{
   ULONG CurrentLength;
   ULONG_PTR CurrentAddress;
   ULONG ByteOffset;
   PVOID VirtualAddress;

   VirtualAddress = MmGetSystemAddressForMdlSafe(Mdl, HighPagePriority);
   if (VirtualAddress == NULL)
   {
      /*
       * NOTE: On real NT a mechanism with reserved pages is implemented
       * to handle this case in a slow, but graceful non-fatal way.
       */
      KeBugCheckEx(HAL_MEMORY_ALLOCATION, PAGE_SIZE, 0, (ULONG_PTR)__FILE__, 0);
   }

   CurrentAddress = (ULONG_PTR)VirtualAddress +
                    (ULONG_PTR)CurrentVa -
                    (ULONG_PTR)MmGetMdlVirtualAddress(Mdl);

   while (Length > 0)
   {
      ByteOffset = BYTE_OFFSET(CurrentAddress);
      CurrentLength = PAGE_SIZE - ByteOffset;
      if (CurrentLength > Length)
         CurrentLength = Length;

      if (WriteToDevice)
      {
         RtlCopyMemory(
            (PVOID)((ULONG_PTR)MapRegisterBase->VirtualAddress + ByteOffset),
            (PVOID)CurrentAddress,
            CurrentLength);
      }
      else
      {
         RtlCopyMemory(
            (PVOID)CurrentAddress,
            (PVOID)((ULONG_PTR)MapRegisterBase->VirtualAddress + ByteOffset),
            CurrentLength);
      }

      Length -= CurrentLength;
      CurrentAddress += CurrentLength;
      MapRegisterBase++;
   }
}

/**
 * @name IoFlushAdapterBuffers
 *
 * Flush any data remaining in the DMA controller's memory into the host
 * memory.
 *
 * @param AdapterObject
 *        The adapter object to flush.
 * @param Mdl
 *        Original MDL to flush data into.
 * @param MapRegisterBase
 *        Map register base that was just used by IoMapTransfer, etc.
 * @param CurrentVa
 *        Offset into Mdl to be flushed into, same as was passed to
 *        IoMapTransfer.
 * @param Length
 *        Length of the buffer to be flushed into.
 * @param WriteToDevice
 *        TRUE if it's a write, FALSE if it's a read.
 *
 * @return TRUE in all cases.
 *
 * @remarks
 *    This copies data from the map register-backed buffer to the user's
 *    target buffer. Data are not in the user buffer until this function
 *    is called.
 *    For slave DMA transfers the controller channel is masked effectively
 *    stopping the current transfer.
 *
 * @unimplemented.
 */

BOOLEAN NTAPI
IoFlushAdapterBuffers(
   PADAPTER_OBJECT AdapterObject,
   PMDL Mdl,
   PVOID MapRegisterBase,
   PVOID CurrentVa,
   ULONG Length,
   BOOLEAN WriteToDevice)
{
   BOOLEAN SlaveDma = FALSE;
   PROS_MAP_REGISTER_ENTRY RealMapRegisterBase;
   PHYSICAL_ADDRESS HighestAcceptableAddress;
   PHYSICAL_ADDRESS PhysicalAddress;
   PPFN_NUMBER MdlPagesPtr;

   ASSERT_IRQL_LESS_OR_EQUAL(DISPATCH_LEVEL);

   if (AdapterObject != NULL && !AdapterObject->MasterDevice)
   {
      /* Mask out (disable) the DMA channel. */
      if (AdapterObject->AdapterNumber == 1)
      {
         PDMA1_CONTROL DmaControl1 = AdapterObject->AdapterBaseVa;
         WRITE_PORT_UCHAR(&DmaControl1->SingleMask,
                          AdapterObject->ChannelNumber | DMA_SETMASK);
      }
      else
      {
         PDMA2_CONTROL DmaControl2 = AdapterObject->AdapterBaseVa;
         WRITE_PORT_UCHAR(&DmaControl2->SingleMask,
                          AdapterObject->ChannelNumber | DMA_SETMASK);
      }
      SlaveDma = TRUE;
   }

   /* This can happen if the device supports hardware scatter/gather. */
   if (MapRegisterBase == NULL)
      return TRUE;

   RealMapRegisterBase =
      (PROS_MAP_REGISTER_ENTRY)((ULONG_PTR)MapRegisterBase & ~MAP_BASE_SW_SG);

   if (!WriteToDevice)
   {
      if ((ULONG_PTR)MapRegisterBase & MAP_BASE_SW_SG)
      {
         if (RealMapRegisterBase->Counter != MAXULONG)
         {
            if (SlaveDma && !AdapterObject->IgnoreCount)
               Length -= HalReadDmaCounter(AdapterObject);
         HalpCopyBufferMap(Mdl, RealMapRegisterBase, CurrentVa, Length, FALSE);
      }
      }
      else
      {
         MdlPagesPtr = MmGetMdlPfnArray(Mdl);
         MdlPagesPtr += ((ULONG_PTR)CurrentVa - (ULONG_PTR)Mdl->StartVa) >> PAGE_SHIFT;

         PhysicalAddress.QuadPart = *MdlPagesPtr << PAGE_SHIFT;
         PhysicalAddress.QuadPart += BYTE_OFFSET(CurrentVa);

         HighestAcceptableAddress = HalpGetAdapterMaximumPhysicalAddress(AdapterObject);
         if (PhysicalAddress.QuadPart + Length >
             HighestAcceptableAddress.QuadPart)
         {
            HalpCopyBufferMap(Mdl, RealMapRegisterBase, CurrentVa, Length, FALSE);
      }
   }
   }

   RealMapRegisterBase->Counter = 0;

   return TRUE;
}

/**
 * @name IoMapTransfer
 *
 * Map a DMA for transfer and do the DMA if it's a slave.
 *
 * @param AdapterObject
 *        Adapter object to do the DMA on. Bus-master may pass NULL.
 * @param Mdl
 *        Locked-down user buffer to DMA in to or out of.
 * @param MapRegisterBase
 *        Handle to map registers to use for this dma.
 * @param CurrentVa
 *        Index into Mdl to transfer into/out of.
 * @param Length
 *        Length of transfer. Number of bytes actually transferred on
 *        output.
 * @param WriteToDevice
 *        TRUE if it's an output DMA, FALSE otherwise.
 *
 * @return
 *    A logical address that can be used to program a DMA controller, it's
 *    not meaningful for slave DMA device.
 *
 * @remarks
 *    This function does a copyover to contiguous memory <16MB represented
 *    by the map registers if needed. If the buffer described by MDL can be
 *    used as is no copyover is done.
 *    If it's a slave transfer, this function actually performs it.
 *
 * @implemented
 */

PHYSICAL_ADDRESS NTAPI
IoMapTransfer(
   IN PADAPTER_OBJECT AdapterObject,
   IN PMDL Mdl,
   IN PVOID MapRegisterBase,
   IN PVOID CurrentVa,
   IN OUT PULONG Length,
   IN BOOLEAN WriteToDevice)
{
   PPFN_NUMBER MdlPagesPtr;
   PFN_NUMBER MdlPage1, MdlPage2;
   ULONG ByteOffset;
   ULONG TransferOffset;
   ULONG TransferLength;
   BOOLEAN UseMapRegisters;
   PROS_MAP_REGISTER_ENTRY RealMapRegisterBase;
   PHYSICAL_ADDRESS PhysicalAddress;
   PHYSICAL_ADDRESS HighestAcceptableAddress;
   ULONG Counter;
   DMA_MODE AdapterMode;
   KIRQL OldIrql;

   /*
    * Precalculate some values that are used in all cases.
    *
    * ByteOffset is offset inside the page at which the transfer starts.
    * MdlPagesPtr is pointer inside the MDL page chain at the page where the
    *             transfer start.
    * PhysicalAddress is physical address corresponding to the transfer
    *                 start page and offset.
    * TransferLength is the inital length of the transfer, which is reminder
    *                of the first page. The actual value is calculated below.
    *
    * Note that all the variables can change during the processing which
    * takes place below. These are just initial values.
    */

   ByteOffset = BYTE_OFFSET(CurrentVa);

   MdlPagesPtr = MmGetMdlPfnArray(Mdl);
   MdlPagesPtr += ((ULONG_PTR)CurrentVa - (ULONG_PTR)Mdl->StartVa) >> PAGE_SHIFT;

   PhysicalAddress.QuadPart = *MdlPagesPtr << PAGE_SHIFT;
   PhysicalAddress.QuadPart += ByteOffset;

   TransferLength = PAGE_SIZE - ByteOffset;

   /*
    * Special case for bus master adapters with S/G support. We can directly
    * use the buffer specified by the MDL, so not much work has to be done.
    *
    * Just return the passed VA's corresponding physical address and update
    * length to the number of physically contiguous bytes found. Also
    * pages crossing the 4Gb boundary aren't considered physically contiguous.
    */

   if (MapRegisterBase == NULL)
   {
      while (TransferLength < *Length)
      {
         MdlPage1 = *MdlPagesPtr;
         MdlPage2 = *(MdlPagesPtr + 1);
         if (MdlPage1 + 1 != MdlPage2)
            break;
         if ((MdlPage1 ^ MdlPage2) & ~0xFFFFF)
            break;
         TransferLength += PAGE_SIZE;
         MdlPagesPtr++;
      }

      if (TransferLength < *Length)
         *Length = TransferLength;

      return PhysicalAddress;
   }

   /*
    * The code below applies to slave DMA adapters and bus master adapters
    * without hardward S/G support.
    */

   RealMapRegisterBase =
      (PROS_MAP_REGISTER_ENTRY)((ULONG_PTR)MapRegisterBase & ~MAP_BASE_SW_SG);

   /*
    * Try to calculate the size of the transfer. We can only transfer
    * pages that are physically contiguous and that don't cross the
    * 64Kb boundary (this limitation applies only for ISA controllers).
    */

   while (TransferLength < *Length)
   {
      MdlPage1 = *MdlPagesPtr;
      MdlPage2 = *(MdlPagesPtr + 1);
      if (MdlPage1 + 1 != MdlPage2)
         break;
      if (!HalpEisaDma && ((MdlPage1 ^ MdlPage2) & ~0xF))
         break;
      TransferLength += PAGE_SIZE;
      MdlPagesPtr++;
   }

   if (TransferLength > *Length)
      TransferLength = *Length;

   /*
    * If we're about to simulate software S/G and not all the pages are
    * physically contiguous then we must use the map registers to store
    * the data and allow the whole transfer to proceed at once.
    */

   if ((ULONG_PTR)MapRegisterBase & MAP_BASE_SW_SG &&
       TransferLength < *Length)
   {
      UseMapRegisters = TRUE;
      PhysicalAddress = RealMapRegisterBase->PhysicalAddress;
      PhysicalAddress.QuadPart += ByteOffset;
      TransferLength = *Length;
      RealMapRegisterBase->Counter = MAXULONG;
      Counter = 0;
   }
   else
   {
      /*
       * This is ordinary DMA transfer, so just update the progress
       * counters. These are used by IoFlushAdapterBuffers to track
       * the transfer progress.
       */

      UseMapRegisters = FALSE;
      Counter = RealMapRegisterBase->Counter;
      RealMapRegisterBase->Counter += BYTES_TO_PAGES(ByteOffset + TransferLength);

      /*
       * Check if the buffer doesn't exceed the highest physical address
       * limit of the device. In that case we must use the map registers to
       * store the data.
       */

      HighestAcceptableAddress = HalpGetAdapterMaximumPhysicalAddress(AdapterObject);
      if (PhysicalAddress.QuadPart + TransferLength >
          HighestAcceptableAddress.QuadPart)
      {
         UseMapRegisters = TRUE;
         PhysicalAddress = RealMapRegisterBase[Counter].PhysicalAddress;
         PhysicalAddress.QuadPart += ByteOffset;
         if ((ULONG_PTR)MapRegisterBase & MAP_BASE_SW_SG)
         {
            RealMapRegisterBase->Counter = MAXULONG;
            Counter = 0;
         }
      }
   }

   /*
    * If we decided to use the map registers (see above) and we're about
    * to transfer data to the device then copy the buffers into the map
    * register memory.
    */

   if (UseMapRegisters && WriteToDevice)
   {
      HalpCopyBufferMap(Mdl, RealMapRegisterBase + Counter,
                        CurrentVa, TransferLength, WriteToDevice);
   }

   /*
    * Return the length of transfer that actually takes place.
    */

   *Length = TransferLength;

   /*
    * If we're doing slave (system) DMA then program the (E)ISA controller
    * to actually start the transfer.
    */

   if (AdapterObject != NULL && !AdapterObject->MasterDevice)
   {
      AdapterMode = AdapterObject->AdapterMode;

      if (WriteToDevice)
      {
         AdapterMode.TransferType = WRITE_TRANSFER;
      }
      else
      {
         AdapterMode.TransferType = READ_TRANSFER;
         if (AdapterObject->IgnoreCount)
         {
            RtlZeroMemory((PUCHAR)RealMapRegisterBase[Counter].VirtualAddress +
                          ByteOffset, TransferLength);
         }
      }

      TransferOffset = PhysicalAddress.LowPart & 0xFFFF;
      if (AdapterObject->Width16Bits)
      {
         TransferLength >>= 1;
         TransferOffset >>= 1;
      }

      OldIrql = KfAcquireSpinLock(&AdapterObject->MasterAdapter->SpinLock);

      if (AdapterObject->AdapterNumber == 1)
      {
         PDMA1_CONTROL DmaControl1 = AdapterObject->AdapterBaseVa;

         /* Reset Register */
         WRITE_PORT_UCHAR(&DmaControl1->ClearBytePointer, 0);
         /* Set the Mode */
         WRITE_PORT_UCHAR(&DmaControl1->Mode, AdapterMode.Byte);
         /* Set the Offset Register */
         WRITE_PORT_UCHAR(&DmaControl1->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseAddress,
                          (UCHAR)(TransferOffset));
         WRITE_PORT_UCHAR(&DmaControl1->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseAddress,
                          (UCHAR)(TransferOffset >> 8));
         /* Set the Page Register */
         WRITE_PORT_UCHAR(AdapterObject->PagePort +
                          FIELD_OFFSET(EISA_CONTROL, DmaController1Pages),
                          (UCHAR)(PhysicalAddress.LowPart >> 16));
         if (HalpEisaDma)
         {
            WRITE_PORT_UCHAR(AdapterObject->PagePort +
                             FIELD_OFFSET(EISA_CONTROL, DmaController2Pages),
                             0);
         }
         /* Set the Length */
         WRITE_PORT_UCHAR(&DmaControl1->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseCount,
                          (UCHAR)(TransferLength - 1));
         WRITE_PORT_UCHAR(&DmaControl1->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseCount,
                          (UCHAR)((TransferLength - 1) >> 8));
         /* Unmask the Channel */
         WRITE_PORT_UCHAR(&DmaControl1->SingleMask,
                          AdapterObject->ChannelNumber | DMA_CLEARMASK);
      }
      else
      {
         PDMA2_CONTROL DmaControl2 = AdapterObject->AdapterBaseVa;

         /* Reset Register */
         WRITE_PORT_UCHAR(&DmaControl2->ClearBytePointer, 0);
         /* Set the Mode */
         WRITE_PORT_UCHAR(&DmaControl2->Mode, AdapterMode.Byte);
         /* Set the Offset Register */
         WRITE_PORT_UCHAR(&DmaControl2->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseAddress,
                          (UCHAR)(TransferOffset));
         WRITE_PORT_UCHAR(&DmaControl2->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseAddress,
                          (UCHAR)(TransferOffset >> 8));
         /* Set the Page Register */
         WRITE_PORT_UCHAR(AdapterObject->PagePort +
                          FIELD_OFFSET(EISA_CONTROL, DmaController1Pages),
                          (UCHAR)(PhysicalAddress.u.LowPart >> 16));
         if (HalpEisaDma)
         {
            WRITE_PORT_UCHAR(AdapterObject->PagePort +
                             FIELD_OFFSET(EISA_CONTROL, DmaController2Pages),
                             0);
         }
         /* Set the Length */
         WRITE_PORT_UCHAR(&DmaControl2->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseCount,
                          (UCHAR)(TransferLength - 1));
         WRITE_PORT_UCHAR(&DmaControl2->DmaAddressCount[AdapterObject->ChannelNumber].DmaBaseCount,
                          (UCHAR)((TransferLength - 1) >> 8));
         /* Unmask the Channel */
         WRITE_PORT_UCHAR(&DmaControl2->SingleMask,
                          AdapterObject->ChannelNumber | DMA_CLEARMASK);
      }

      KfReleaseSpinLock(&AdapterObject->MasterAdapter->SpinLock, OldIrql);
   }

   /*
    * Return physical address of the buffer with data that is used for the
    * transfer. It can either point inside the Mdl that was passed by the
    * caller or into the map registers if the Mdl buffer can't be used
    * directly.
    */

   return PhysicalAddress;
}

/**
 * @name HalFlushCommonBuffer
 *
 * @implemented
 */
BOOLEAN
NTAPI
HalFlushCommonBuffer(IN PADAPTER_OBJECT AdapterObject,
                     IN ULONG Length,
                     IN PHYSICAL_ADDRESS LogicalAddress,
                     IN PVOID VirtualAddress)
{
    /* Function always returns true */
    return TRUE;
}

/*
 * @implemented
 */
PVOID
NTAPI
HalAllocateCrashDumpRegisters(IN PADAPTER_OBJECT AdapterObject,
                              IN OUT PULONG NumberOfMapRegisters)
{
    PADAPTER_OBJECT MasterAdapter = AdapterObject->MasterAdapter;
    ULONG MapRegisterNumber;

    /* Check if it needs map registers */
    if (AdapterObject->NeedsMapRegisters)
    {
        /* Check if we have enough */
        if (*NumberOfMapRegisters > AdapterObject->MapRegistersPerChannel)
        {
            /* We don't, fail */
            AdapterObject->NumberOfMapRegisters = 0;
            return NULL;
        }

        /* Try to find free map registers */
        MapRegisterNumber = MAXULONG;
        MapRegisterNumber = RtlFindClearBitsAndSet(MasterAdapter->MapRegisters,
                                                   *NumberOfMapRegisters,
                                                   0);

        /* Check if nothing was found */
        if (MapRegisterNumber == MAXULONG)
        {
            /* No free registers found, so use the base registers */
            RtlSetBits(MasterAdapter->MapRegisters,
                       0,
                       *NumberOfMapRegisters);
            MapRegisterNumber = 0;
        }

        /* Calculate the new base */
        AdapterObject->MapRegisterBase =
            (PROS_MAP_REGISTER_ENTRY)(MasterAdapter->MapRegisterBase +
                                      MapRegisterNumber);

        /* Check if scatter gather isn't supported */
        if (!AdapterObject->ScatterGather)
        {
            /* Set the flag */
            AdapterObject->MapRegisterBase =
                (PROS_MAP_REGISTER_ENTRY)
                ((ULONG_PTR)AdapterObject->MapRegisterBase | MAP_BASE_SW_SG);
        }
    }
    else
    {
        AdapterObject->MapRegisterBase = NULL;
        AdapterObject->NumberOfMapRegisters = 0;
    }

    /* Return the base */
    return AdapterObject->MapRegisterBase;
}

/* EOF */
