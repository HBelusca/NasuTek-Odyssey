/*++

Copyright (c) 2008-2011 Alexandr A. Telyatnikov (Alter)

Module Name:
    id_probe.cpp

Abstract:
    This module handles SATA-related staff

Author:
    Alexander A. Telyatnikov (Alter)

Environment:
    kernel mode only

Notes:

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Revision History:

--*/

#include "stdafx.h"

UCHAR
NTAPI
UniataSataConnect(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,          // logical channel
    IN ULONG pm_port /* for port multipliers */
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    //ULONG Channel = deviceExtension->Channel + lChannel;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    SATA_SSTATUS_REG SStatus;
    ULONG i;
/*
    UCHAR                signatureLow,
                         signatureHigh;
*/
    UCHAR                Status;

    KdPrint2((PRINT_PREFIX "UniataSataConnect:\n"));

    if(!UniataIsSATARangeAvailable(deviceExtension, lChannel)) {
        KdPrint2((PRINT_PREFIX "  no I/O range\n"));
        return IDE_STATUS_IDLE;
    }

    /* clear SATA error register, some controllers need this */
    UniataSataWritePort4(chan, IDX_SATA_SError,
        UniataSataReadPort4(chan, IDX_SATA_SError, pm_port), pm_port);
    /* wait up to 1 second for "connect well" */
    for(i=0; i<100; i++) {
        SStatus.Reg = UniataSataReadPort4(chan, IDX_SATA_SStatus, pm_port);
        if(SStatus.SPD == SStatus_SPD_Gen1 ||
           SStatus.SPD == SStatus_SPD_Gen2 ||
           SStatus.SPD == SStatus_SPD_Gen3) {
            chan->lun[0]->TransferMode = ATA_SA150 + (UCHAR)(SStatus.SPD - 1);
            KdPrint2((PRINT_PREFIX "SATA TransferMode %#x\n", chan->lun[0]->TransferMode));
            break;
        }
        AtapiStallExecution(10000);
    }
    if(i >= 100) {
        KdPrint2((PRINT_PREFIX "UniataSataConnect: SStatus %8.8x\n", SStatus.Reg));
        return 0xff;
    }
    /* clear SATA error register */
    UniataSataWritePort4(chan, IDX_SATA_SError,
        UniataSataReadPort4(chan, IDX_SATA_SError, pm_port), pm_port);

    Status = WaitOnBaseBusyLong(chan);
    if(Status & IDE_STATUS_BUSY) {
        return Status;
    }
/*
    signatureLow = AtapiReadPort1(chan, &deviceExtension->BaseIoAddress1[lChannel].i.CylinderLow);
    signatureHigh = AtapiReadPort1(chan, &deviceExtension->baseIoAddress1[lChannel].i.CylinderHigh);

    if (signatureLow == ATAPI_MAGIC_LSB && signatureHigh == ATAPI_MAGIC_MSB) {
    }
*/
    KdPrint2((PRINT_PREFIX "UniataSataConnect: OK, ATA status %#x\n", Status));
    return IDE_STATUS_IDLE;
} // end UniataSataConnect()

UCHAR
NTAPI
UniataSataPhyEnable(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,          // logical channel
    IN ULONG pm_port, /* for port multipliers */
    IN BOOLEAN doReset
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    SATA_SCONTROL_REG SControl;
    int loop, retry;

    KdPrint2((PRINT_PREFIX "UniataSataPhyEnable:\n"));

    if(!UniataIsSATARangeAvailable(deviceExtension, lChannel)) {
        KdPrint2((PRINT_PREFIX "  no I/O range\n"));
        return IDE_STATUS_IDLE;
    }

    SControl.Reg = UniataSataReadPort4(chan, IDX_SATA_SControl, pm_port);
    KdPrint2((PRINT_PREFIX "SControl %#x\n", SControl.Reg));
    if(SControl.DET == SControl_DET_Idle) {
        if(!doReset) {
            return UniataSataConnect(HwDeviceExtension, lChannel, pm_port);
        }
    }

    for (retry = 0; retry < 10; retry++) {
        KdPrint2((PRINT_PREFIX "UniataSataPhyEnable: retry init %d\n", retry));
	for (loop = 0; loop < 10; loop++) {
	    SControl.Reg = 0;
	    SControl.DET = SControl_DET_Init;
            UniataSataWritePort4(chan, IDX_SATA_SControl, SControl.Reg, pm_port);
            AtapiStallExecution(100);
            SControl.Reg = UniataSataReadPort4(chan, IDX_SATA_SControl, pm_port);
            KdPrint2((PRINT_PREFIX "  SControl %8.8x\n", SControl.Reg));
            if(SControl.DET == SControl_DET_Init) {
		break;
            }
	}
        AtapiStallExecution(5000);
        KdPrint2((PRINT_PREFIX "UniataSataPhyEnable: retry idle %d\n", retry));
	for (loop = 0; loop < 10; loop++) {
	    SControl.Reg = 0;
	    SControl.DET = SControl_DET_DoNothing;
	    SControl.IPM = SControl_IPM_NoPartialSlumber;
            UniataSataWritePort4(chan, IDX_SATA_SControl, SControl.Reg, pm_port);
            AtapiStallExecution(100);
            SControl.Reg = UniataSataReadPort4(chan, IDX_SATA_SControl, pm_port);
            KdPrint2((PRINT_PREFIX "  SControl %8.8x\n", SControl.Reg));
            if(SControl.DET == SControl_DET_Idle) {
                return UniataSataConnect(HwDeviceExtension, lChannel, pm_port);
	    }
	}
    }

    KdPrint2((PRINT_PREFIX "UniataSataPhyEnable: failed\n"));
    return 0xff;
} // end UniataSataPhyEnable()

BOOLEAN
NTAPI
UniataSataClearErr(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,          // logical channel
    IN BOOLEAN do_connect,
    IN ULONG pm_port /* for port multipliers */
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG ChipFlags = deviceExtension->HwFlags & CHIPFLAG_MASK;
    SATA_SSTATUS_REG SStatus;
    SATA_SERROR_REG  SError;

    if(UniataIsSATARangeAvailable(deviceExtension, lChannel)) {
    //if(ChipFlags & UNIATA_SATA) {

        SStatus.Reg = UniataSataReadPort4(chan, IDX_SATA_SStatus, pm_port);
        SError.Reg  = UniataSataReadPort4(chan, IDX_SATA_SError, pm_port); 

        if(SStatus.Reg) {
            KdPrint2((PRINT_PREFIX "  SStatus %#x\n", SStatus.Reg));
        }
        if(SError.Reg) {
            KdPrint2((PRINT_PREFIX "  SError %#x\n", SError.Reg));
            /* clear error bits/interrupt */
            UniataSataWritePort4(chan, IDX_SATA_SError, SError.Reg, pm_port);

            if(do_connect) {
                /* if we have a connection event deal with it */
                if(SError.DIAG.N) {
                    KdPrint2((PRINT_PREFIX "  catch SATA connect/disconnect\n"));
                    if(SStatus.SPD >= SStatus_SPD_Gen1) {
                        UniataSataEvent(deviceExtension, lChannel, UNIATA_SATA_EVENT_ATTACH, pm_port);
                    } else {
                        UniataSataEvent(deviceExtension, lChannel, UNIATA_SATA_EVENT_DETACH, pm_port);
                    }
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
} // end UniataSataClearErr()

BOOLEAN
NTAPI
UniataSataEvent(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,          // logical channel
    IN ULONG Action,
    IN ULONG pm_port /* for port multipliers */
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    UCHAR Status;
    ULONG DeviceNumber = (pm_port ? 1 : 0);

    if(!UniataIsSATARangeAvailable(deviceExtension, lChannel)) {
        return FALSE;
    }

    switch(Action) {
    case UNIATA_SATA_EVENT_ATTACH:
        KdPrint2((PRINT_PREFIX "  CONNECTED\n"));
        Status = UniataSataConnect(HwDeviceExtension, lChannel, pm_port);
        KdPrint2((PRINT_PREFIX "  Status %#x\n", Status));
        if(Status != IDE_STATUS_IDLE) {
            return FALSE;
        }
        CheckDevice(HwDeviceExtension, lChannel, DeviceNumber /*dev*/, FALSE);
        return TRUE;
        break;
    case UNIATA_SATA_EVENT_DETACH:
        KdPrint2((PRINT_PREFIX "  DISCONNECTED\n"));
        UniataForgetDevice(deviceExtension->chan[lChannel].lun[DeviceNumber]);
        return TRUE;
        break;
    }
    return FALSE;
} // end UniataSataEvent()

ULONG
NTAPI
UniataSataReadPort4(
    IN PHW_CHANNEL chan,
    IN ULONG io_port_ndx,
    IN ULONG pm_port /* for port multipliers */
    )
{
    if(chan && (io_port_ndx < IDX_MAX_REG) &&
       chan->RegTranslation[io_port_ndx].Proc) {

        KdPrint3((PRINT_PREFIX "  UniataSataReadPort4 %#x[%d]\n", io_port_ndx, pm_port));

        PHW_DEVICE_EXTENSION deviceExtension = chan->DeviceExtension;
        PVOID HwDeviceExtension = (PVOID)deviceExtension;
        ULONG slotNumber = deviceExtension->slotNumber;
        ULONG SystemIoBusNumber = deviceExtension->SystemIoBusNumber;
        ULONG VendorID =  deviceExtension->DevID        & 0xffff;
        ULONG offs;
        ULONG p;

        switch(VendorID) {
        case ATA_INTEL_ID: {
            p = pm_port ? 1 : 0;
            if(deviceExtension->HwFlags & ICH5) {
                offs = 0x50+chan->lun[p]->SATA_lun_map*0x10;
                KdPrint3((PRINT_PREFIX "  ICH5 way, offs %#x\n", offs));
                switch(io_port_ndx) {
                case IDX_SATA_SStatus:
                    offs += 0;
                    break;
                case IDX_SATA_SError:
                    offs += 1*4;
                    break;
                case IDX_SATA_SControl:
                    offs += 2*4;
                    break;
                default:
                    return -1;
                }
                SetPciConfig4(0xa0, offs);
                GetPciConfig4(0xa4, offs);
                return offs;
            } else {
                offs = ((deviceExtension->Channel+chan->lChannel)*2+p) * 0x100;
                KdPrint3((PRINT_PREFIX "  def way, offs %#x\n", offs));
                switch(io_port_ndx) {
                case IDX_SATA_SStatus:
                    offs += 0;
                    break;
                case IDX_SATA_SControl:
                    offs += 1;
                    break;
                case IDX_SATA_SError:
                    offs += 2;
                    break;
                default:
                    return -1;
                }
                AtapiWritePort4(chan, IDX_INDEXED_ADDR, offs);
                return AtapiReadPort4(chan, IDX_INDEXED_DATA);
            }
        } // ATA_INTEL_ID
        } // end switch(VendorID)
        return -1;
    }
    return AtapiReadPort4(chan, io_port_ndx);
} // end UniataSataReadPort4()

VOID
NTAPI
UniataSataWritePort4(
    IN PHW_CHANNEL chan,
    IN ULONG io_port_ndx,
    IN ULONG data,
    IN ULONG pm_port /* for port multipliers */
    )
{
    if(chan && (io_port_ndx < IDX_MAX_REG) &&
       chan->RegTranslation[io_port_ndx].Proc) {

        KdPrint3((PRINT_PREFIX "  UniataSataWritePort4 %#x[%d]\n", io_port_ndx, pm_port));

        PHW_DEVICE_EXTENSION deviceExtension = chan->DeviceExtension;
        PVOID HwDeviceExtension = (PVOID)deviceExtension;
        ULONG slotNumber = deviceExtension->slotNumber;
        ULONG SystemIoBusNumber = deviceExtension->SystemIoBusNumber;
        ULONG VendorID =  deviceExtension->DevID        & 0xffff;
        ULONG offs;
        ULONG p;

        switch(VendorID) {
        case ATA_INTEL_ID: {
            p = pm_port ? 1 : 0;
            if(deviceExtension->HwFlags & ICH5) {
                offs = 0x50+chan->lun[p]->SATA_lun_map*0x10;
                KdPrint3((PRINT_PREFIX "  ICH5 way, offs %#x\n", offs));
                switch(io_port_ndx) {
                case IDX_SATA_SStatus:
                    offs += 0;
                    break;
                case IDX_SATA_SError:
                    offs += 1*4;
                    break;
                case IDX_SATA_SControl:
                    offs += 2*4;
                    break;
                default:
                    return;
                }
                SetPciConfig4(0xa0, offs);
                SetPciConfig4(0xa4, data);
                return;
            } else {
                offs = ((deviceExtension->Channel+chan->lChannel)*2+p) * 0x100;
                KdPrint3((PRINT_PREFIX "  def way, offs %#x\n", offs));
                switch(io_port_ndx) {
                case IDX_SATA_SStatus:
                    offs += 0;
                    break;
                case IDX_SATA_SControl:
                    offs += 1;
                    break;
                case IDX_SATA_SError:
                    offs += 2;
                    break;
                default:
                    return;
                }
                AtapiWritePort4(chan, IDX_INDEXED_ADDR, offs);
                AtapiWritePort4(chan, IDX_INDEXED_DATA, data);
            }
        } // ATA_INTEL_ID
        } // end switch(VendorID)
        return;
    }
    AtapiWritePort4(chan, io_port_ndx, data);
} // end UniataSataWritePort4()

BOOLEAN
NTAPI
UniataSataReadPM(
    IN PHW_CHANNEL chan,
    IN ULONG DeviceNumber,
    IN ULONG Reg,
   OUT PULONG result
    )
{
    if(chan->DeviceExtension->HwFlags & UNIATA_AHCI) {
        return UniataAhciReadPM(chan, DeviceNumber, Reg, result);
    }
    return FALSE;
} // end UniataSataReadPM()

UCHAR
NTAPI
UniataSataWritePM(
    IN PHW_CHANNEL chan,
    IN ULONG DeviceNumber,
    IN ULONG Reg,
    IN ULONG value
    )
{
    if(chan->DeviceExtension->HwFlags & UNIATA_AHCI) {
        return UniataAhciWritePM(chan, DeviceNumber, Reg, value);
    }
    return 0xff;
} // end UniataSataWritePM()

ULONG
NTAPI
UniataSataSoftReset(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
    IN ULONG DeviceNumber
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;

    if(deviceExtension->HwFlags & UNIATA_AHCI) {
        return UniataAhciSoftReset(HwDeviceExtension, lChannel, DeviceNumber);
    }
    return 0xffffffff;
} // end UniataSataSoftReset()

VOID
UniataSataIdentifyPM(
    IN PHW_CHANNEL chan
    )
{
    ULONG PM_DeviceId;
    ULONG PM_RevId;
    ULONG PM_Ports;
    UCHAR i;
    ULONG signature;
    PHW_LU_EXTENSION     LunExt;

    KdPrint((PRINT_PREFIX "UniataSataIdentifyPM:\n"));

    chan->PmLunMap = 0;

    /* get PM vendor & product data */
    if(!UniataSataReadPM(chan, AHCI_DEV_SEL_PM, 0, &PM_DeviceId)) {
        KdPrint2((PRINT_PREFIX "  error getting PM vendor data\n"));
	return;
    }
    /* get PM revision data */
    if(!UniataSataReadPM(chan, AHCI_DEV_SEL_PM, 1, &PM_RevId)) {
        KdPrint2((PRINT_PREFIX "  error getting PM revison data\n"));
	return;
    }
    /* get number of HW ports on the PM */
    if(!UniataSataReadPM(chan, AHCI_DEV_SEL_PM, 2, &PM_Ports)) {
        KdPrint2((PRINT_PREFIX "  error getting PM port info\n"));
	return;
    }

    PM_Ports &= 0x0000000f;

    switch(PM_DeviceId) {
    case 0x37261095:
        /* This PM declares 6 ports, while only 5 of them are real.
         * Port 5 is enclosure management bridge port, which has implementation
         * problems, causing probe faults. Hide it for now. */
        KdPrint2((PRINT_PREFIX "  SiI 3726 (rev=%#x) Port Multiplier with %d (5) ports\n",
            PM_RevId, PM_Ports));
        PM_Ports = 5;
        break;
    case 0x47261095:
        /* This PM declares 7 ports, while only 5 of them are real.
         * Port 5 is some fake "Config  Disk" with 640 sectors size,
         * port 6 is enclosure management bridge port.
         * Both fake ports has implementation problems, causing
         * probe faults. Hide them for now. */
        KdPrint2((PRINT_PREFIX "  SiI 4726 (rev=%#x) Port Multiplier with %d (5) ports\n",
            PM_RevId, PM_Ports));
        PM_Ports = 5;
        break;
    default:
        KdPrint2((PRINT_PREFIX "  Port Multiplier (id=%08x rev=%#x) with %d ports\n",
            PM_DeviceId, PM_RevId, PM_Ports));
        break;
    }

    // reset
    for(i=0; i<PM_Ports; i++) {

        LunExt = chan->lun[i];

        KdPrint2((PRINT_PREFIX "    Port %d\n", i));
        if(UniataSataPhyEnable(chan->DeviceExtension, chan->lChannel, i, UNIATA_SATA_RESET_ENABLE) != IDE_STATUS_IDLE) {
            LunExt->DeviceFlags &= ~DFLAGS_DEVICE_PRESENT;
            continue;
        }
	/*
	 * XXX: I have no idea how to properly wait for PMP port hardreset
	 * completion. Without this delay soft reset does not completes
	 * successfully.
	 */
        AtapiStallExecution(1000000);

        signature = UniataSataSoftReset(chan->DeviceExtension, chan->lChannel, i);
        KdPrint2((PRINT_PREFIX "  signature %#x\n", signature));

        LunExt->DeviceFlags |= DFLAGS_DEVICE_PRESENT;
        chan->PmLunMap |= (1 << i);
	/* figure out whats there */
	switch (signature >> 16) {
	case 0x0000:
            LunExt->DeviceFlags &= ~DFLAGS_ATAPI_DEVICE;
	    continue;
	case 0xeb14:
            LunExt->DeviceFlags |= DFLAGS_ATAPI_DEVICE;
	    continue;
	}

    }

} // end UniataSataIdentifyPM()

#ifdef DBG
VOID
NTAPI
UniataDumpAhciRegs(
    IN PHW_DEVICE_EXTENSION deviceExtension
    )
{
    ULONG                j;
    ULONG                xReg;

    KdPrint2((PRINT_PREFIX 
               "  AHCI Base: %#x MemIo %d Proc %d\n",
               deviceExtension->BaseIoAHCI_0.Addr,
               deviceExtension->BaseIoAHCI_0.MemIo,
               deviceExtension->BaseIoAHCI_0.Proc));

    for(j=0; j<=IDX_AHCI_VS; j+=sizeof(ULONG)) {
        xReg = AtapiReadPortEx4(NULL, (ULONGIO_PTR)&deviceExtension->BaseIoAHCI_0, j);
        KdPrint2((PRINT_PREFIX 
                   "  AHCI_%#x (%#x) = %#x\n",
                   j,
                   (deviceExtension->BaseIoAHCI_0.Addr+j),
                   xReg));
    }
    return;
} // end UniataDumpAhciRegs()


VOID
NTAPI
UniataDumpAhciPortRegs(
    IN PHW_CHANNEL chan
    )
{
    ULONG                j;
    ULONG                xReg;

    KdPrint2((PRINT_PREFIX 
               "  AHCI port %d Base: %#x MemIo %d Proc %d\n",
               chan->lChannel,
               chan->BaseIoAHCI_Port.Addr,
               chan->BaseIoAHCI_Port.MemIo,
               chan->BaseIoAHCI_Port.Proc));

    for(j=0; j<=IDX_AHCI_P_SNTF; j+=sizeof(ULONG)) {
        xReg = AtapiReadPortEx4(NULL, (ULONGIO_PTR)&chan->BaseIoAHCI_Port, j);
        KdPrint2((PRINT_PREFIX 
                   "  AHCI%d_%#x (%#x) = %#x\n",
                   chan->lChannel,
                   j,
                   (chan->BaseIoAHCI_Port.Addr+j),
                   xReg));
    }
    return;
} // end UniataDumpAhciPortRegs()
#endif //DBG


BOOLEAN
NTAPI
UniataAhciInit(
    IN PVOID HwDeviceExtension
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    ULONG c, i;
    PHW_CHANNEL chan;
    ULONG offs;
    ULONG BaseMemAddress;
    ULONG PI;
    ULONG CAP;
    ULONG GHC;
    BOOLEAN MemIo = FALSE;

    KdPrint2((PRINT_PREFIX "  UniataAhciInit:\n"));

#ifdef DBG
    UniataDumpAhciRegs(deviceExtension);
#endif //DBG

    /* reset AHCI controller */
    GHC = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC);
    KdPrint2((PRINT_PREFIX "  reset AHCI controller, GHC %#x\n", GHC));
    UniataAhciWriteHostPort4(deviceExtension, IDX_AHCI_GHC,
        GHC | AHCI_GHC_HR);

    for(i=0; i<1000; i++) {
        AtapiStallExecution(1000);
        GHC = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC);
        KdPrint2((PRINT_PREFIX "  AHCI GHC %#x\n", GHC));
        if(!(GHC & AHCI_GHC_HR)) {
            break;
        }
    }
    if(GHC & AHCI_GHC_HR) {
        KdPrint2((PRINT_PREFIX "  AHCI reset failed\n"));
        return FALSE;
    }

    /* enable AHCI mode */
    GHC = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC);
    KdPrint2((PRINT_PREFIX "  enable AHCI mode, GHC %#x\n", GHC));
    UniataAhciWriteHostPort4(deviceExtension, IDX_AHCI_GHC,
        GHC | AHCI_GHC_AE);
    GHC = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC);
    KdPrint2((PRINT_PREFIX "  AHCI GHC %#x\n", GHC));

    deviceExtension->AHCI_CAP =
      CAP = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_CAP);
    PI = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_PI);
    KdPrint2((PRINT_PREFIX "  AHCI CAP %#x\n", CAP));
    if(CAP & AHCI_CAP_S64A) {
        KdPrint2((PRINT_PREFIX "  AHCI 64bit\n"));
        deviceExtension->Host64 = TRUE;
    }
    KdPrint2((PRINT_PREFIX "  AHCI %d CMD slots\n", (CAP & AHCI_CAP_NCS_MASK) >> 8 ));
    if(CAP & AHCI_CAP_PMD) {
        KdPrint2((PRINT_PREFIX "  AHCI multi-block PIO\n"));
    }
    if(CAP & AHCI_CAP_SAM) {
        KdPrint2((PRINT_PREFIX "  AHCI legasy SATA\n"));
    }
    /* get the number of HW channels */
    PI = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_PI);
    KdPrint2((PRINT_PREFIX "  AHCI PI %#x\n", PI));

    /* clear interrupts */
    UniataAhciWriteHostPort4(deviceExtension, IDX_AHCI_IS,
        UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_IS));

    /* enable AHCI interrupts */
    UniataAhciWriteHostPort4(deviceExtension, IDX_AHCI_GHC,
        UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC) | AHCI_GHC_IE);

    BaseMemAddress = deviceExtension->BaseIoAHCI_0.Addr;
    MemIo          = deviceExtension->BaseIoAHCI_0.MemIo;

    for(c=0; c<deviceExtension->NumberChannels; c++) {
        chan = &deviceExtension->chan[c];
        offs = sizeof(IDE_AHCI_REGISTERS) + c*sizeof(IDE_AHCI_PORT_REGISTERS);

        KdPrint2((PRINT_PREFIX "  chan %d, offs %#x\n", c, offs));

        AtapiSetupLunPtrs(chan, deviceExtension, c);

        chan->BaseIoAHCI_Port = deviceExtension->BaseIoAHCI_0;
        chan->BaseIoAHCI_Port.Addr = BaseMemAddress + offs;

        chan->RegTranslation[IDX_IO1_i_Status      ].Addr       = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, TFD.STS);
        chan->RegTranslation[IDX_IO1_i_Status      ].MemIo      = MemIo;
        chan->RegTranslation[IDX_IO2_AltStatus] = chan->RegTranslation[IDX_IO1_i_Status];
        chan->RegTranslation[IDX_IO1_i_Error       ].Addr       = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, TFD.ERR);
        chan->RegTranslation[IDX_IO1_i_Error       ].MemIo      = MemIo;
        chan->RegTranslation[IDX_IO1_i_CylinderLow ].Addr       = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SIG.LbaLow);
        chan->RegTranslation[IDX_IO1_i_CylinderLow ].MemIo      = MemIo;
        chan->RegTranslation[IDX_IO1_i_CylinderHigh].Addr       = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SIG.LbaHigh);
        chan->RegTranslation[IDX_IO1_i_CylinderHigh].MemIo      = MemIo;
        chan->RegTranslation[IDX_IO1_i_BlockCount  ].Addr       = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SIG.SectorCount);
        chan->RegTranslation[IDX_IO1_i_BlockCount  ].MemIo      = MemIo;

        UniataInitSyncBaseIO(chan);

        chan->RegTranslation[IDX_SATA_SStatus].Addr   = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SSTS);
        chan->RegTranslation[IDX_SATA_SStatus].MemIo  = MemIo;
        chan->RegTranslation[IDX_SATA_SError].Addr    = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SERR);
        chan->RegTranslation[IDX_SATA_SError].MemIo   = MemIo;
        chan->RegTranslation[IDX_SATA_SControl].Addr  = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SCTL);
        chan->RegTranslation[IDX_SATA_SControl].MemIo = MemIo;
        chan->RegTranslation[IDX_SATA_SActive].Addr   = BaseMemAddress + offs + FIELD_OFFSET(IDE_AHCI_PORT_REGISTERS, SACT);
        chan->RegTranslation[IDX_SATA_SActive].MemIo  = MemIo;

        AtapiDmaAlloc(HwDeviceExtension, NULL, c);

        UniataAhciResume(chan);

        chan->ChannelCtrlFlags |= CTRFLAGS_NO_SLAVE;
    }

    return TRUE;
} // end UniataAhciInit()

BOOLEAN
NTAPI
UniataAhciDetect(
    IN PVOID HwDeviceExtension,
    IN PPCI_COMMON_CONFIG pciData, // optional
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    //ULONG slotNumber = deviceExtension->slotNumber;
    ULONG SystemIoBusNumber = deviceExtension->SystemIoBusNumber;
    ULONG version;
    ULONG i, n;
    ULONG PI;
    ULONG CAP;
    ULONG GHC;
    ULONG NumberChannels;
    ULONG v_Mn, v_Mj;
    ULONG BaseMemAddress;
    BOOLEAN MemIo;

    KdPrint2((PRINT_PREFIX "  UniataAhciDetect:\n"));

    if(AtapiRegCheckDevValue(deviceExtension, CHAN_NOT_SPECIFIED, DEVNUM_NOT_SPECIFIED, L"IgnoreAhci", 1 /* DEBUG */)) {
        KdPrint(("  AHCI excluded\n"));
        return FALSE;
    }
    BaseMemAddress = AtapiGetIoRange(HwDeviceExtension, ConfigInfo, pciData, SystemIoBusNumber,
                            5, 0, 0x10);
    if(!BaseMemAddress) {
        KdPrint2((PRINT_PREFIX "  AHCI init failed - no IoRange\n"));
        return FALSE;
    }
    if(BaseMemAddress && (*ConfigInfo->AccessRanges)[5].RangeInMemory) {
        KdPrint2((PRINT_PREFIX "MemIo\n"));
        MemIo = TRUE;
    }
    deviceExtension->BaseIoAHCI_0.Addr  = BaseMemAddress;
    deviceExtension->BaseIoAHCI_0.MemIo = MemIo;

#ifdef DBG
    UniataDumpAhciRegs(deviceExtension);
#endif //DBG

    GHC = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC);
    if(GHC & AHCI_GHC_HR) {
        KdPrint2((PRINT_PREFIX "  AHCI in reset state\n"));
        return FALSE;
    }

    /* enable AHCI mode */
    GHC = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_GHC);
    KdPrint2((PRINT_PREFIX "  check AHCI mode, GHC %#x\n", GHC));
    if(!(GHC & AHCI_GHC_AE)) {
        KdPrint2((PRINT_PREFIX "  Non-AHCI GHC (!AE)\n"));
        return FALSE;
    }

    CAP = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_CAP);
    KdPrint2((PRINT_PREFIX "  AHCI CAP %#x\n", CAP));
    if(CAP & AHCI_CAP_S64A) {
        KdPrint2((PRINT_PREFIX "  AHCI 64bit\n"));
        //deviceExtension->Host64 = TRUE;
    }

    /* get the number of HW channels */
    PI = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_PI);
    KdPrint2((PRINT_PREFIX "  AHCI PI %#x\n", PI));
    for(i=PI, n=0; i; n++, i=i>>1);
    NumberChannels =
        max((CAP & AHCI_CAP_NOP_MASK)+1, n);

    KdPrint2((PRINT_PREFIX "  Channels %d\n", n));

    switch(deviceExtension->DevID) {
    case ATA_M88SX6111:
        NumberChannels = 1;
        break;
    case ATA_M88SX6121:
        NumberChannels = 2;
        break;
    case ATA_M88SX6141:
    case ATA_M88SX6145:
        NumberChannels = 4;
        break;
    } // switch()

    if(!NumberChannels) {
        KdPrint2((PRINT_PREFIX "  Non-AHCI - NumberChannels=0\n"));
        return FALSE;
    }

    version = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_VS);
    v_Mj = ((version >> 20) & 0xf0) + ((version >> 16) & 0x0f);
    v_Mn = ((version >> 4) & 0xf0) + (version & 0x0f);

    KdPrint2((PRINT_PREFIX "  AHCI version %#x.%02x controller with %d ports (mask %#x) detected\n",
		  v_Mj, v_Mn,
		  NumberChannels, PI));

    if(CAP & AHCI_CAP_SPM) {
        KdPrint2((PRINT_PREFIX "  PM supported\n"));
        if(AtapiRegCheckDevValue(deviceExtension, CHAN_NOT_SPECIFIED, DEVNUM_NOT_SPECIFIED, L"IgnoreAhciPM", 1 /* DEBUG */)) {
            KdPrint2((PRINT_PREFIX "SATA/AHCI w/o PM, max luns 1\n"));
            deviceExtension->NumberLuns = 2;
            //chan->ChannelCtrlFlags |= CTRFLAGS_NO_SLAVE;
        } else {
            KdPrint2((PRINT_PREFIX "SATA/AHCI -> possible PM, max luns %d\n", SATA_MAX_PM_UNITS));
            deviceExtension->NumberLuns = SATA_MAX_PM_UNITS;
            //deviceExtension->NumberLuns = 1;
        }
    } else {
        KdPrint2((PRINT_PREFIX "  PM not supported -> 1 lun/chan\n"));
        deviceExtension->NumberLuns = 1;
    }

    if((v_Mj != 0x01) || (v_Mn > 0x20)) {
        KdPrint2((PRINT_PREFIX "  Unknown AHCI revision\n"));
        if(AtapiRegCheckDevValue(deviceExtension, CHAN_NOT_SPECIFIED, DEVNUM_NOT_SPECIFIED, L"CheckAhciRevision", 1)) {
            KdPrint(("  AHCI revision excluded\n"));
            return FALSE;
        }
    }

    deviceExtension->HwFlags |= UNIATA_SATA;
    deviceExtension->HwFlags |= UNIATA_AHCI;
    if(deviceExtension->NumberChannels < NumberChannels) {
        deviceExtension->NumberChannels = NumberChannels;
    }

    return TRUE;
} // end UniataAhciDetect()

UCHAR
NTAPI
UniataAhciStatus(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
    IN ULONG DeviceNumber
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    ULONG Channel = deviceExtension->Channel + lChannel;
    ULONG            hIS;
    ULONG            CI;
    AHCI_IS_REG      IS;
    SATA_SSTATUS_REG SStatus;
    SATA_SERROR_REG  SError;
    //ULONG offs = sizeof(IDE_AHCI_REGISTERS) + Channel*sizeof(IDE_AHCI_PORT_REGISTERS);
    ULONG tag=0;

    KdPrint(("UniataAhciStatus:\n"));

    hIS = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_IS);
    KdPrint((" hIS %#x\n", hIS));
    hIS &= (1 << Channel);
    if(!hIS) {
        return INTERRUPT_REASON_IGNORE;
    }
    IS.Reg      = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_IS);
    CI          = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CI);
    SStatus.Reg = AtapiReadPort4(chan, IDX_SATA_SStatus);
    SError.Reg  = AtapiReadPort4(chan, IDX_SATA_SError); 

    /* clear interrupt(s) */
    UniataAhciWriteHostPort4(deviceExtension, IDX_AHCI_IS, hIS);
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IS, IS.Reg);
    AtapiWritePort4(chan, IDX_SATA_SError, SError.Reg);

    KdPrint((" AHCI: status=%08x sstatus=%08x error=%08x CI=%08x\n",
	   IS.Reg, SStatus.Reg, SError.Reg, CI));

    /* do we have cold connect surprise */
    if(IS.CPDS) {
    }

    /* check for and handle connect events */
    if(IS.PCS) {
        UniataSataEvent(HwDeviceExtension, lChannel, UNIATA_SATA_EVENT_ATTACH);
    }
    if(IS.PRCS) {
        UniataSataEvent(HwDeviceExtension, lChannel, UNIATA_SATA_EVENT_DETACH);
    }
    if(CI & (1 << tag)) {
        return INTERRUPT_REASON_OUR;
    }
    KdPrint((" AHCI: unexpected\n"));
    return INTERRUPT_REASON_UNEXPECTED;

} // end UniataAhciStatus()

ULONG
NTAPI
UniataAhciSetupFIS_H2D(
    IN PHW_DEVICE_EXTENSION deviceExtension,
    IN ULONG DeviceNumber,
    IN ULONG lChannel,
   OUT PUCHAR fis,
    IN UCHAR command,
    IN ULONGLONG lba,
    IN USHORT count,
    IN USHORT feature,
    IN ULONG flags
    )
{
    ULONG i;
    PUCHAR plba;
    BOOLEAN need48;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];

    KdPrint2((PRINT_PREFIX "  AHCI setup FIS ch %d, dev %d\n", lChannel, DeviceNumber));
    i = 0;
    plba = (PUCHAR)&lba;

    fis[0] = AHCI_FIS_TYPE_ATA_H2D;  /* host to device */
    fis[1] = 0x80 | ((UCHAR)DeviceNumber & 0x0f);  /* command FIS (note PM goes here) */
    fis[7] = IDE_USE_LBA;
    fis[15] = IDE_DC_A_4BIT;

    if(chan->lun[DeviceNumber]->DeviceFlags & DFLAGS_ATAPI_DEVICE) {
        fis[2] = IDE_COMMAND_ATAPI_PACKET;
        if(feature & ATA_F_DMA) {
            fis[3] = (UCHAR)(feature & 0xff);
        } else {
            fis[5] = (UCHAR)(count & 0xff);
            fis[6] = (UCHAR)(count>>8) & 0xff;
        }
    } else {

        if((AtaCommandFlags[command] & ATA_CMD_FLAG_LBAIOsupp) &&
           CheckIfBadBlock(chan->lun[DeviceNumber], lba, count)) {
            KdPrint3((PRINT_PREFIX ": artificial bad block, lba %#I64x count %#x\n", lba, count));
            //return IDE_STATUS_ERROR;
            //return SRB_STATUS_ERROR;
            return 0;
        }

        need48 = UniAta_need_lba48(command, lba, count,
            chan->lun[DeviceNumber]->IdentifyData.FeaturesSupport.Address48);

        /* translate command into 48bit version */
        if(need48) {
            if(AtaCommandFlags[command] & ATA_CMD_FLAG_48supp) {
                command = AtaCommands48[command];
            } else {
                KdPrint2((PRINT_PREFIX "  unhandled LBA48 command\n"));
                return 0;
            }
        }

        fis[2] = command;
        fis[3] = (UCHAR)feature;

        fis[4] = plba[0];
        fis[5] = plba[1];
        fis[6] = plba[2];
        if(need48) {
            i++;
        } else {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4333) // right shift by too large amount, data loss
#endif
            fis[7] |= IDE_DRIVE_1 | ((plba[3] >> 24) & 0x0f);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        }

        fis[8] = plba[3];
        fis[9] = plba[4];
        fis[10] = plba[5]; 
        fis[11] = (UCHAR)(feature>>8) & 0xff;

        fis[12] = (UCHAR)count & 0xff;
        fis[13] = (UCHAR)(count>>8) & 0xff;
        //fis[14] = 0x00;

    }

    KdDump(fis, 20);

    return 20;
} // end UniataAhciSetupFIS_H2D()

UCHAR
NTAPI
UniataAhciSendCommand(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
    IN ULONG DeviceNumber,
    IN ULONG flags,
    IN ULONG timeout
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG            hIS;
    ULONG            CI = 0;
    AHCI_IS_REG      IS;
    ULONG            SError;
    //SATA_SSTATUS_REG SStatus;
    //SATA_SERROR_REG  SError;
    //ULONG offs = sizeof(IDE_AHCI_REGISTERS) + Channel*sizeof(IDE_AHCI_PORT_REGISTERS);
    //ULONGIO_PTR base;
    ULONG tag=0;
    ULONG i;

    PIDE_AHCI_CMD_LIST AHCI_CL = &(chan->AhciCtlBlock->cmd_list[tag]);

    KdPrint(("UniataAhciSendCommand: lChan %d\n", chan->lChannel));

    AHCI_CL->prd_length = 0;
    AHCI_CL->cmd_flags = (20 / sizeof(ULONG)) | flags | (DeviceNumber << 12);
    AHCI_CL->bytecount = 0;
    AHCI_CL->cmd_table_phys = chan->AHCI_CTL_PhAddr + FIELD_OFFSET(IDE_AHCI_CHANNEL_CTL_BLOCK, cmd);
    if(AHCI_CL->cmd_table_phys & AHCI_CMD_ALIGNEMENT_MASK) {
        KdPrint2((PRINT_PREFIX "  AHCI CMD address is not aligned (mask %#x)\n", (ULONG)AHCI_CMD_ALIGNEMENT_MASK));
    }

    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CI, ATA_AHCI_P_CMD_ST);

    for (i=0; i<timeout; i++) {
        AtapiStallExecution(1000);
        CI = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CI);
        if (!(CI & ATA_AHCI_P_CMD_ST)) {
            break;
        }
    }
    KdPrint(("  CI %#x\n", CI));

    //SStatus.Reg = AtapiReadPort4(chan, IDX_SATA_SStatus);
    //SError.Reg  = AtapiReadPort4(chan, IDX_SATA_SError); 

    /* clear interrupt(s) */
    IS.Reg      = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_IS);
    KdPrint(("  IS %#x\n", IS.Reg));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IS, IS.Reg);

    if (timeout && (i >= timeout)) {
        SError = AtapiReadPort4(chan, IDX_SATA_SError);
        KdPrint((" AHCI: timeout, SError %#x\n", SError));
        return 0xff;
    }

    return IDE_STATUS_IDLE;

} // end UniataAhciSendCommand()

ULONG
NTAPI
UniataAhciSoftReset(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
    IN ULONG DeviceNumber
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG            hIS;
    //ULONG            CI;
    //AHCI_IS_REG      IS;
    //ULONG tag=0;

    KdPrint(("UniataAhciSoftReset: lChan %d\n", chan->lChannel));

    PIDE_AHCI_CMD  AHCI_CMD = &(chan->AhciCtlBlock->cmd);
    PUCHAR         RCV_FIS = &(chan->AhciCtlBlock->rcv_fis.rfis[0]);

#ifdef DBG
    UniataDumpAhciPortRegs(chan);
#endif // DBG

    /* kick controller into sane state */
    UniataAhciStop(chan);
    UniataAhciCLO(chan);
    UniataAhciStart(chan);

#ifdef DBG
    UniataDumpAhciPortRegs(chan);
#endif // DBG

    /* pull reset active */
    RtlZeroMemory(AHCI_CMD->cfis, sizeof(AHCI_CMD->cfis));
    AHCI_CMD->cfis[0] = AHCI_FIS_TYPE_ATA_H2D;
    AHCI_CMD->cfis[1] = (UCHAR)DeviceNumber & 0x0f;
    //AHCI_CMD->cfis[7] = IDE_USE_LBA | IDE_DRIVE_SELECT;
    AHCI_CMD->cfis[15] = (IDE_DC_A_4BIT | IDE_DC_RESET_CONTROLLER);

    if(UniataAhciSendCommand(HwDeviceExtension, lChannel, DeviceNumber, ATA_AHCI_CMD_RESET | ATA_AHCI_CMD_CLR_BUSY, 100) == 0xff) {
        KdPrint2(("  timeout\n"));
        return (ULONG)(-1);
    }
    AtapiStallExecution(50);

    /* pull reset inactive */
    RtlZeroMemory(AHCI_CMD->cfis, sizeof(AHCI_CMD->cfis));
    AHCI_CMD->cfis[0] = AHCI_FIS_TYPE_ATA_H2D;
    AHCI_CMD->cfis[1] = (UCHAR)DeviceNumber & 0x0f;
    //AHCI_CMD->cfis[7] = IDE_USE_LBA | IDE_DRIVE_SELECT;
    AHCI_CMD->cfis[15] = (IDE_DC_A_4BIT);
    if(UniataAhciSendCommand(HwDeviceExtension, lChannel, DeviceNumber, 0, 3000) == 0xff) {
        KdPrint2(("  timeout (2)\n"));
        return (ULONG)(-1);
    }

    UniataAhciWaitReady(chan, 1);

    KdDump(RCV_FIS, sizeof(chan->AhciCtlBlock->rcv_fis.rfis));

    return UniataAhciUlongFromRFIS(RCV_FIS);

} // end UniataAhciSoftReset()

ULONG
NTAPI
UniataAhciWaitReady(
    IN PHW_CHANNEL chan,
    IN ULONG timeout
    )
{
    ULONG TFD;
    ULONG i;

    KdPrint2(("UniataAhciWaitReady: lChan %d\n", chan->lChannel));

    //base = (ULONGIO_PTR)(&deviceExtension->BaseIoAHCI_0 + offs);

    TFD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_TFD);
    for(i=0; i<timeout && (TFD &
              (IDE_STATUS_DRQ | IDE_STATUS_BUSY)); i++) {
        AtapiStallExecution(1000);
        TFD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_TFD);
    }

    KdPrint2(("  TFD %#x\n", TFD));

    return TFD;

} // end UniataAhciWaitReady()

ULONG
NTAPI
UniataAhciHardReset(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
   OUT PULONG signature
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG Channel = deviceExtension->Channel + lChannel;
    ULONG            TFD;


    KdPrint(("UniataAhciHardReset: lChan %d\n", chan->lChannel));

    (*signature) = 0xffffffff;

    UniataAhciStop(chan);
    if(UniataSataPhyEnable(HwDeviceExtension, lChannel, 0/* dev0*/, UNIATA_SATA_RESET_ENABLE) == 0xff) {
        KdPrint(("  no PHY\n"));
        return 0xff;
    }

    /* Wait for clearing busy status. */
    TFD = UniataAhciWaitReady(chan, 15000);
    if(TFD & (IDE_STATUS_DRQ | IDE_STATUS_BUSY)) {
        KdPrint(("  busy: TFD %#x\n", TFD));
        return TFD;
    }
    KdPrint(("  TFD %#x\n", TFD));

#ifdef DBG
    UniataDumpAhciPortRegs(chan);
#endif // DBG

    (*signature) = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_SIG);
    KdPrint(("  sig: %#x\n", *signature));

    UniataAhciStart(chan);

    return 0;

} // end UniataAhciHardReset()

VOID
NTAPI
UniataAhciReset(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG offs = sizeof(IDE_AHCI_REGISTERS) + Channel*sizeof(IDE_AHCI_PORT_REGISTERS);
    ULONG CAP;
    //ULONGIO_PTR base;
    ULONG signature;
    ULONG i;
    ULONG VendorID =  deviceExtension->DevID & 0xffff;

    KdPrint(("UniataAhciReset: lChan %d\n", chan->lChannel));

    //base = (ULONGIO_PTR)(&deviceExtension->BaseIoAHCI_0 + offs);

    /* Disable port interrupts */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IE, 0);

    if(UniataAhciHardReset(HwDeviceExtension, lChannel, &signature)) {

        KdPrint(("  No devices in all LUNs\n"));
        for (i=0; i<deviceExtension->NumberLuns; i++) {
            // Zero device fields to ensure that if earlier devices were found,
            // but not claimed, the fields are cleared.
            UniataForgetDevice(chan->lun[i]);
        }

	/* enable wanted port interrupts */
        UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IE,
            ATA_AHCI_P_IX_CPD | ATA_AHCI_P_IX_PRC | ATA_AHCI_P_IX_PC);
        return;
    }

    /* enable wanted port interrupts */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IE,
        (ATA_AHCI_P_IX_CPD | ATA_AHCI_P_IX_TFE | ATA_AHCI_P_IX_HBF |
         ATA_AHCI_P_IX_HBD | ATA_AHCI_P_IX_IF | ATA_AHCI_P_IX_OF |
         ((/*ch->pm_level == */0) ? (ATA_AHCI_P_IX_PRC | ATA_AHCI_P_IX_PC) : 0) |
         ATA_AHCI_P_IX_DP | ATA_AHCI_P_IX_UF | ATA_AHCI_P_IX_SDB |
         ATA_AHCI_P_IX_DS | ATA_AHCI_P_IX_PS | ATA_AHCI_P_IX_DHR) );

    /*
     * Only probe for PortMultiplier if HW has support.
     * Ignore Marvell, which is not working,
     */
    CAP = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_CAP);
    if ((CAP & AHCI_CAP_SPM) &&
	    (VendorID != ATA_MARVELL_ID)) {
        KdPrint(("  check PM\n"));
	signature = UniataAhciSoftReset(HwDeviceExtension, lChannel, AHCI_DEV_SEL_PM);
	/* Workaround for some ATI chips, failing to soft-reset
	 * when port multiplicator supported, but absent.
	 * XXX: We can also check PxIS.IPMS==1 here to be sure. */
	if (signature == 0xffffffff) {
            KdPrint(("  re-check PM\n"));
	    signature = UniataAhciSoftReset(HwDeviceExtension, lChannel, 0);
	}
    } else {
        signature = UniataAhciSoftReset(HwDeviceExtension, lChannel, 0);
    }

    KdPrint(("  signature %#x\n", signature));
    chan->lun[0]->DeviceFlags &= ~(DFLAGS_ATAPI_DEVICE | DFLAGS_DEVICE_PRESENT | CTRFLAGS_AHCI_PM);
    switch (signature >> 16) {
    case 0x0000:
        KdPrint(("  ATA dev\n"));
        chan->lun[0]->DeviceFlags |= DFLAGS_DEVICE_PRESENT;
	chan->PmLunMap = 0;
	break;
    case 0x9669:
        KdPrint(("  PM\n"));
        if(deviceExtension->NumberLuns > 1) {
	    chan->ChannelCtrlFlags |= CTRFLAGS_AHCI_PM;
            UniataSataIdentifyPM(chan);
	} else {
            KdPrint(("  no PM supported (1 lun/chan)\n"));
	}
	break;
    case 0xeb14:
        KdPrint(("  ATAPI dev\n"));
        chan->lun[0]->DeviceFlags |= (DFLAGS_ATAPI_DEVICE | DFLAGS_DEVICE_PRESENT);
	chan->PmLunMap = 0;
	break;
    default: /* SOS XXX */
        KdPrint(("  default to ATA ???\n"));
        chan->lun[0]->DeviceFlags |= DFLAGS_DEVICE_PRESENT;
	chan->PmLunMap = 0;
    }

    return;

} // end UniataAhciReset()

VOID
NTAPI
UniataAhciStartFR(
    IN PHW_CHANNEL chan
    )
{
    ULONG CMD;

    KdPrint2(("UniataAhciStartFR: lChan %d\n", chan->lChannel));

    CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
    KdPrint2(("  CMD %#x\n", CMD));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, CMD | ATA_AHCI_P_CMD_FRE);

    return;
} // end UniataAhciStartFR()

VOID
NTAPI
UniataAhciStopFR(
    IN PHW_CHANNEL chan
    )
{
    ULONG CMD;
    ULONG i;

    KdPrint2(("UniataAhciStopFR: lChan %d\n", chan->lChannel));

    CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
    KdPrint2(("  CMD %#x\n", CMD));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, CMD & ~ATA_AHCI_P_CMD_FRE);

    for(i=0; i<1000; i++) {
        CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
        if(!(CMD & ATA_AHCI_P_CMD_FR)) {
            KdPrint2(("  final CMD %#x\n", CMD));
            return;
        }
        AtapiStallExecution(1000);
    }
    KdPrint2(("  CMD %#x\n", CMD));
    KdPrint(("   SError %#x\n", AtapiReadPort4(chan, IDX_SATA_SError)));
    KdPrint2(("UniataAhciStopFR: timeout\n"));
    return;
} // end UniataAhciStopFR()

VOID
NTAPI
UniataAhciStart(
    IN PHW_CHANNEL chan
    )
{
    ULONG IS, CMD;
    SATA_SERROR_REG  SError;

    KdPrint2(("UniataAhciStart: lChan %d\n", chan->lChannel));

    /* clear SATA error register */
    SError.Reg  = AtapiReadPort4(chan, IDX_SATA_SError); 

    /* clear any interrupts pending on this channel */
    IS = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_IS);
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IS, IS);

    KdPrint2(("    SError %#x, IS %#x\n", SError.Reg, IS));

    CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
    KdPrint2(("  CMD %#x\n", CMD));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD,
        CMD |
        ATA_AHCI_P_CMD_ST |
        ((chan->ChannelCtrlFlags & CTRFLAGS_AHCI_PM) ? ATA_AHCI_P_CMD_PMA : 0));

    return;
} // end UniataAhciStart()

VOID
NTAPI
UniataAhciCLO(
    IN PHW_CHANNEL chan
    )
{
    //PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    //PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    ULONG CAP, CMD;
    //SATA_SERROR_REG  SError;
    ULONG i;

    KdPrint2(("UniataAhciCLO: lChan %d\n", chan->lChannel));

    /* issue Command List Override if supported */ 
    //CAP = UniataAhciReadHostPort4(deviceExtension, IDX_AHCI_CAP);
    CAP = chan->DeviceExtension->AHCI_CAP; 
    if(!(CAP & AHCI_CAP_SCLO)) {
        return;
    }
    KdPrint2(("  send CLO\n"));
    CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
    CMD |= ATA_AHCI_P_CMD_CLO;
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, CMD);

    for(i=0; i<1000; i++) {
        CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
        if(!(CMD & ATA_AHCI_P_CMD_CLO)) {
            KdPrint2(("  final CMD %#x\n", CMD));
            return;
        }
        AtapiStallExecution(1000);
    }
    KdPrint2(("  CMD %#x\n", CMD));
    KdPrint2(("UniataAhciCLO: timeout\n"));
    return;
} // end UniataAhciCLO()

VOID
NTAPI
UniataAhciStop(
    IN PHW_CHANNEL chan
    )
{
    ULONG CMD;
    //SATA_SERROR_REG  SError;
    ULONG i;

    KdPrint2(("UniataAhciStop: lChan %d\n", chan->lChannel));

    /* issue Command List Override if supported */ 
    CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
    CMD &= ~ATA_AHCI_P_CMD_ST;
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, CMD);

    for(i=0; i<1000; i++) {
        CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
        if(!(CMD & ATA_AHCI_P_CMD_CR)) {
            KdPrint2(("  final CMD %#x\n", CMD));
            return;
        }
        AtapiStallExecution(1000);
    }
    KdPrint2(("  CMD %#x\n", CMD));
    KdPrint(("   SError %#x\n", AtapiReadPort4(chan, IDX_SATA_SError)));
    KdPrint2(("UniataAhciStop: timeout\n"));
    return;
} // end UniataAhciStop()

UCHAR
NTAPI
UniataAhciBeginTransaction(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
    IN ULONG DeviceNumber,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG            hIS;
    ULONG            CMD;
    //AHCI_IS_REG      IS;
    PATA_REQ AtaReq = (PATA_REQ)(Srb->SrbExtension);
    //SATA_SSTATUS_REG SStatus;
    //SATA_SERROR_REG  SError;
    //ULONG offs = sizeof(IDE_AHCI_REGISTERS) + Channel*sizeof(IDE_AHCI_PORT_REGISTERS);
    //ULONGIO_PTR base;
    ULONG tag=0;
    //ULONG i;

    PIDE_AHCI_CMD_LIST AHCI_CL = &(chan->AhciCtlBlock->cmd_list[tag]);

    KdPrint2(("UniataAhciBeginTransaction: lChan %d\n", chan->lChannel));

    if(AtaReq->dma_entries > (USHORT)0xffff) {
        KdPrint2(("UniataAhciBeginTransaction too long DMA tab\n"));
        return 0;
    }

    AHCI_CL->prd_length = (USHORT)AtaReq->dma_entries;
    AHCI_CL->cmd_flags  = AtaReq->ahci.io_cmd_flags;
    AHCI_CL->bytecount = 0;
    AHCI_CL->cmd_table_phys = AtaReq->ahci.ahci_base64;
    if(AHCI_CL->cmd_table_phys & AHCI_CMD_ALIGNEMENT_MASK) {
        KdPrint2((PRINT_PREFIX "  AHCI CMD address is not aligned (mask %#x)\n", (ULONG)AHCI_CMD_ALIGNEMENT_MASK));
    }

    CMD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_CMD);
    KdPrint2(("  CMD %#x\n", CMD));
    CMD &= ~ATA_AHCI_P_CMD_ATAPI;
    KdPrint2(("  send CMD %#x\n", CMD));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, CMD);

    /* issue command to controller */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CI, ATA_AHCI_P_CMD_ST);

    if(!(chan->lun[DeviceNumber]->DeviceFlags & DFLAGS_ATAPI_DEVICE)) {
        // TODO: check if we send ATA_RESET and wait for ready of so.
        if(AtaReq->ahci.ahci_cmd_ptr->cfis[2] == IDE_COMMAND_ATAPI_RESET) {
            ULONG  TFD;
            ULONG  i;

            for(i=0; i<1000000; i++) {
                TFD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_TFD);
                if(!(TFD & IDE_STATUS_BUSY)) {
                    break;
                }
            }
            if(TFD & IDE_STATUS_BUSY) {
                KdPrint2(("  timeout\n"));
            }
            if(TFD & IDE_STATUS_ERROR) {
                KdPrint2(("  ERROR %#x\n", (UCHAR)(TFD >> 8)));
            }
            AtaReq->ahci.in_status = TFD;

            return 0x00;
        }
    }

    return IDE_STATUS_IDLE;

} // end UniataAhciBeginTransaction()

UCHAR
NTAPI
UniataAhciEndTransaction(
    IN PVOID HwDeviceExtension,
    IN ULONG lChannel,
    IN ULONG DeviceNumber,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    PHW_DEVICE_EXTENSION deviceExtension = (PHW_DEVICE_EXTENSION)HwDeviceExtension;
    PHW_CHANNEL chan = &deviceExtension->chan[lChannel];
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG            hIS;
    PATA_REQ AtaReq = (PATA_REQ)(Srb->SrbExtension);
    ULONG            TFD;
    PUCHAR         RCV_FIS = &(chan->AhciCtlBlock->rcv_fis.rfis[0]);

    KdPrint2(("UniataAhciEndTransaction: lChan %d\n", chan->lChannel));

    TFD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_TFD);
    KdPrint2(("  TFD %#x\n", TFD));

    if(TFD & IDE_STATUS_ERROR) {
        KdPrint2(("  ERROR %#x\n", (UCHAR)(TFD >> 8)));
    }
    AtaReq->ahci.in_status = TFD;

    //if (request->flags & ATA_R_CONTROL) {

    AtaReq->ahci.in_bcount = (ULONG)(RCV_FIS[12]) | ((ULONG)(RCV_FIS[13]) << 8);
    AtaReq->ahci.in_lba = (ULONG)(RCV_FIS[4]) | ((ULONGLONG)(RCV_FIS[5]) << 8) |
			     ((ULONGLONG)(RCV_FIS[6]) << 16);
    if(chan->ChannelCtrlFlags & CTRFLAGS_LBA48) {
        AtaReq->ahci.in_lba |= ((ULONGLONG)(RCV_FIS[8]) << 24) |
                                ((ULONGLONG)(RCV_FIS[9]) << 32) |
                                ((ULONGLONG)(RCV_FIS[10]) << 40);
    } else {
        AtaReq->ahci.in_lba |= ((ULONGLONG)(RCV_FIS[8]) << 24) |
                                ((ULONGLONG)(RCV_FIS[9]) << 32) |
                                ((ULONGLONG)(RCV_FIS[7] & 0x0f) << 24);
    }

    //}

    return 0;

} // end UniataAhciEndTransaction()

VOID
NTAPI
UniataAhciResume(
    IN PHW_CHANNEL chan
    )
{
    ULONGLONG base;

    KdPrint2(("UniataAhciResume: lChan %d\n", chan->lChannel));

#ifdef DBG
    UniataDumpAhciPortRegs(chan);
#endif // DBG

    /* Disable port interrupts */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IE, 0);

    /* setup work areas */
    base = chan->AHCI_CTL_PhAddr;
    if(!base) {
        KdPrint2((PRINT_PREFIX "  AHCI buffer allocation failed\n"));
        return;
    }
    KdPrint2((PRINT_PREFIX "  AHCI CLB setup\n"));
    if(base & AHCI_CLB_ALIGNEMENT_MASK) {
        KdPrint2((PRINT_PREFIX "  AHCI CLB address is not aligned (mask %#x)\n", (ULONG)AHCI_FIS_ALIGNEMENT_MASK));
    }
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CLB,
        (ULONG)(base & 0xffffffff));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CLB + 4,
        (ULONG)((base >> 32) & 0xffffffff));

    KdPrint2((PRINT_PREFIX "  AHCI RCV FIS setup\n"));
    base = chan->AHCI_CTL_PhAddr + FIELD_OFFSET(IDE_AHCI_CHANNEL_CTL_BLOCK, rcv_fis);
    if(base & AHCI_FIS_ALIGNEMENT_MASK) {
        KdPrint2((PRINT_PREFIX "  AHCI FIS address is not aligned (mask %#x)\n", (ULONG)AHCI_FIS_ALIGNEMENT_MASK));
    }
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_FB,
        (ULONG)(base & 0xffffffff));
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_FB + 4,
        (ULONG)((base >> 32) & 0xffffffff));

    /* activate the channel and power/spin up device */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD,
        (ATA_AHCI_P_CMD_ACTIVE | ATA_AHCI_P_CMD_POD | ATA_AHCI_P_CMD_SUD |
	     (((chan->ChannelCtrlFlags & CTRFLAGS_AHCI_PM)) ? ATA_AHCI_P_CMD_ALPE : 0) |
	     (((chan->ChannelCtrlFlags & CTRFLAGS_AHCI_PM2)) ? ATA_AHCI_P_CMD_ASP : 0 ))
	     );

#ifdef DBG
    UniataDumpAhciPortRegs(chan);
#endif // DBG

    UniataAhciStartFR(chan);
    UniataAhciStart(chan);

#ifdef DBG
    UniataDumpAhciPortRegs(chan);
#endif // DBG

    return;
} // end UniataAhciResume()

#if 0
VOID
NTAPI
UniataAhciSuspend(
    IN PHW_CHANNEL chan
    )
{
    ULONGLONG base;
    SATA_SCONTROL_REG SControl;

    KdPrint2(("UniataAhciSuspend:\n"));

    /* Disable port interrupts */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_IE, 0);

    /* Reset command register. */
    UniataAhciStop(chan);
    UniataAhciStopFR(chan);
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, 0);

    /* Allow everything including partial and slumber modes. */
    UniataSataWritePort4(chan, IDX_SATA_SControl, 0, 0);

    /* Request slumber mode transition and give some time to get there. */
    UniataAhciWriteChannelPort4(chan, IDX_AHCI_P_CMD, ATA_AHCI_P_CMD_SLUMBER);
    AtapiStallExecution(100);

    /* Disable PHY. */
    SControl.Reg = 0;
    SControl.DET = SStatus_DET_Offline;
    UniataSataWritePort4(chan, IDX_SATA_SControl, SControl.Reg, 0);

    return;
} // end UniataAhciSuspend()
#endif

BOOLEAN
NTAPI
UniataAhciReadPM(
    IN PHW_CHANNEL chan,
    IN ULONG DeviceNumber,
    IN ULONG Reg,
   OUT PULONG result
    )
{
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG            hIS;
    //ULONG            CI;
    //AHCI_IS_REG      IS;
    //ULONG tag=0;
    PIDE_AHCI_CMD  AHCI_CMD = &(chan->AhciCtlBlock->cmd);
    PUCHAR         RCV_FIS = &(chan->AhciCtlBlock->rcv_fis.rfis[0]);

    KdPrint(("UniataAhciReadPM: lChan %d [%#x]\n", chan->lChannel, DeviceNumber));

    if(DeviceNumber == DEVNUM_NOT_SPECIFIED) {
        (*result) = UniataSataReadPort4(chan, Reg, 0);
        return TRUE;
    }
    if(DeviceNumber < AHCI_DEV_SEL_PM) {
        switch(Reg) {
        case IDX_SATA_SStatus:
            Reg = 0; break;
        case IDX_SATA_SError:
            Reg = 1; break;
        case IDX_SATA_SControl:
            Reg = 2; break;
        default:
            return FALSE;
        }
    }

    RtlZeroMemory(AHCI_CMD->cfis, sizeof(AHCI_CMD->cfis));
    AHCI_CMD->cfis[0] = AHCI_FIS_TYPE_ATA_H2D;
    AHCI_CMD->cfis[1] = AHCI_FIS_COMM_PM;
    AHCI_CMD->cfis[2] = IDE_COMMAND_READ_PM;
    AHCI_CMD->cfis[3] = (UCHAR)Reg;
    AHCI_CMD->cfis[7] = (UCHAR)(IDE_USE_LBA | DeviceNumber);
    AHCI_CMD->cfis[15] = IDE_DC_A_4BIT;

    if(UniataAhciSendCommand(chan->DeviceExtension, chan->lChannel, DeviceNumber, 0, 10) == 0xff) {
        KdPrint2(("  PM read failed\n"));
        return FALSE;
    }

    KdDump(RCV_FIS, sizeof(chan->AhciCtlBlock->rcv_fis.rfis));

    (*result) = UniataAhciUlongFromRFIS(RCV_FIS);
    return TRUE;

} // end UniataAhciReadPM()

UCHAR
NTAPI
UniataAhciWritePM(
    IN PHW_CHANNEL chan,
    IN ULONG DeviceNumber,
    IN ULONG Reg,
    IN ULONG value
    )
{
    //ULONG Channel = deviceExtension->Channel + lChannel;
    //ULONG            hIS;
    //ULONG            CI;
    //AHCI_IS_REG      IS;
    //ULONG tag=0;
    ULONG          TFD;
    PIDE_AHCI_CMD  AHCI_CMD = &(chan->AhciCtlBlock->cmd);
    //PUCHAR         RCV_FIS = &(chan->AhciCtlBlock->rcv_fis.rfis[0]);

    KdPrint(("UniataAhciWritePM: lChan %d [%#x] %#x\n", chan->lChannel, DeviceNumber, value));

    if(DeviceNumber == DEVNUM_NOT_SPECIFIED) {
        UniataSataWritePort4(chan, Reg, value, 0);
        return 0;
    }
    if(DeviceNumber < AHCI_DEV_SEL_PM) {
        switch(Reg) {
        case IDX_SATA_SStatus:
            Reg = 0; break;
        case IDX_SATA_SError:
            Reg = 1; break;
        case IDX_SATA_SControl:
            Reg = 2; break;
        default:
            return 0xff;
        }
    }

    RtlZeroMemory(AHCI_CMD->cfis, sizeof(AHCI_CMD->cfis));
    AHCI_CMD->cfis[0] = AHCI_FIS_TYPE_ATA_H2D;
    AHCI_CMD->cfis[1] = AHCI_FIS_COMM_PM;
    AHCI_CMD->cfis[2] = IDE_COMMAND_WRITE_PM;
    AHCI_CMD->cfis[3] = (UCHAR)Reg;
    AHCI_CMD->cfis[7] = (UCHAR)(IDE_USE_LBA | DeviceNumber);

    AHCI_CMD->cfis[12] = (UCHAR)(value & 0xff);
    AHCI_CMD->cfis[4]  = (UCHAR)((value >> 8) & 0xff);
    AHCI_CMD->cfis[5]  = (UCHAR)((value >> 16) & 0xff);
    AHCI_CMD->cfis[6]  = (UCHAR)((value >> 24) & 0xff);

    AHCI_CMD->cfis[15] = IDE_DC_A_4BIT;

    if(UniataAhciSendCommand(chan->DeviceExtension, chan->lChannel, DeviceNumber, 0, 100) == 0xff) {
        KdPrint2(("  PM write failed\n"));
        return 0xff;
    }

    TFD = UniataAhciReadChannelPort4(chan, IDX_AHCI_P_TFD);

    if(TFD & IDE_STATUS_ERROR) {
        KdPrint2(("  ERROR %#x\n", (UCHAR)(TFD >> 8)));
    }
    return (UCHAR)(TFD >> 8);

} // end UniataAhciWritePM()

VOID
UniataAhciSetupCmdPtr(
IN OUT PATA_REQ AtaReq
    )
{
    union {
        PUCHAR prd_base;
        ULONGLONG prd_base64;
    };
    union {
        PUCHAR prd_base0;
        ULONGLONG prd_base64_0;
    };
    ULONG d;

    prd_base64_0 = prd_base64 = 0;
    prd_base = (PUCHAR)(&AtaReq->ahci_cmd0);
    prd_base0 = prd_base;

    prd_base64 = (prd_base64 + max(FIELD_OFFSET(ATA_REQ, ahci_cmd0), AHCI_CMD_ALIGNEMENT_MASK)) & ~AHCI_CMD_ALIGNEMENT_MASK;

    d = (ULONG)(prd_base64 - prd_base64_0);
    KdPrint2((PRINT_PREFIX "  aligned %I64x, d=%x\n", prd_base64, d));

    AtaReq->ahci.ahci_cmd_ptr = (PIDE_AHCI_CMD)prd_base64;
    KdPrint2((PRINT_PREFIX "  ahci_cmd_ptr %#x\n", AtaReq->ahci.ahci_cmd_ptr));
} // end UniataAhciSetupCmdPtr()
