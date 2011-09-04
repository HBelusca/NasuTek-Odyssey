#pragma once

#include <ntddk.h>
#include <usb.h>

/* USB Command Register */
#define EHCI_USBCMD         0x00
#define EHCI_USBSTS         0x04
#define EHCI_USBINTR            0x08
#define EHCI_FRINDEX            0x0C
#define EHCI_CTRLDSSEGMENT      0x10
#define EHCI_PERIODICLISTBASE       0x14
#define EHCI_ASYNCLISTBASE      0x18
#define EHCI_CONFIGFLAG         0x40
#define EHCI_PORTSC         0x44

/* USB Interrupt Register Flags 32 Bits */
#define EHCI_USBINTR_INTE       0x01
#define EHCI_USBINTR_ERR        0x02
#define EHCI_USBINTR_PC         0x04
#define EHCI_USBINTR_FLROVR     0x08
#define EHCI_USBINTR_HSERR      0x10
#define EHCI_USBINTR_ASYNC      0x20
/* Bits 6:31 Reserved */

/* Status Register Flags 32 Bits */
#define EHCI_STS_INT            0x01
#define EHCI_STS_ERR            0x02
#define EHCI_STS_PCD            0x04
#define EHCI_STS_FLR            0x08
#define EHCI_STS_FATAL          0x10
#define EHCI_STS_IAA            0x20
/* Bits 11:6 Reserved */
#define EHCI_STS_HALT           0x1000
#define EHCI_STS_RECL           0x2000
#define EHCI_STS_PSS            0x4000
#define EHCI_STS_ASS            0x8000
#define EHCI_ERROR_INT ( EHCI_STS_FATAL | EHCI_STS_ERR )


/* Last bit in QUEUE ELEMENT TRANSFER DESCRIPTOR Next Pointer */
/* Used for Queue Element Transfer Descriptor Pointers
   and Queue Head Horizontal Link Pointers */
#define TERMINATE_POINTER       0x01

/* QUEUE ELEMENT TRANSFER DESCRIPTOR, Token defines and structs */

/* PIDCodes for QETD_TOKEN
OR with QUEUE_TRANSFER_DESCRIPTOR Token.PIDCode*/
#define PID_CODE_OUT_TOKEN      0x00
#define PID_CODE_IN_TOKEN       0x01
#define PID_CODE_SETUP_TOKEN    0x02

/* Split Transaction States
OR with QUEUE_TRANSFER_DESCRIPTOR Token.SplitTransactionState */
#define DO_START_SPLIT          0x00
#define DO_COMPLETE_SPLIT       0x01

/* Ping States, OR with QUEUE_TRANSFER_DESCRIPTOR Token. */
#define PING_STATE_DO_OUT       0x00
#define PING_STATE_DO_PING      0x01

typedef struct _PERIODICFRAMELIST
{
    PULONG VirtualAddr;
    PHYSICAL_ADDRESS PhysicalAddr;
    ULONG Size;
} PERIODICFRAMELIST, *PPERIODICFRAMELIST;


/* QUEUE ELEMENT TRANSFER DESCRIPTOR TOKEN */
typedef struct _QETD_TOKEN_BITS
{
    ULONG PingState:1;
    ULONG SplitTransactionState:1;
    ULONG MissedMicroFrame:1;
    ULONG TransactionError:1;
    ULONG BabbleDetected:1;
    ULONG DataBufferError:1;
    ULONG Halted:1;
    ULONG Active:1;
    ULONG PIDCode:2;
    ULONG ErrorCounter:2;
    ULONG CurrentPage:3;
    ULONG InterruptOnComplete:1;
    ULONG TotalBytesToTransfer:15;
    ULONG DataToggle:1;
} QETD_TOKEN_BITS, *PQETD_TOKEN_BITS;

/* QUEUE ELEMENT TRANSFER DESCRIPTOR */
typedef struct _QUEUE_TRANSFER_DESCRIPTOR
{
    //Hardware
    ULONG NextPointer;
    ULONG AlternateNextPointer;
    union
    {
        QETD_TOKEN_BITS Bits;
        ULONG DWord;
    } Token;
    ULONG BufferPointer[5];

    //Software
    ULONG BufferPointerVA[5];
    ULONG PhysicalAddr;
    struct _QUEUE_TRANSFER_DESCRIPTOR *PreviousDescriptor;
    struct _QUEUE_TRANSFER_DESCRIPTOR *NextDescriptor;
} QUEUE_TRANSFER_DESCRIPTOR, *PQUEUE_TRANSFER_DESCRIPTOR;

/* EndPointSpeeds of END_POINT_CHARACTERISTICS */
#define QH_ENDPOINT_FULLSPEED       0x00
#define QH_ENDPOINT_LOWSPEED        0x01
#define QH_ENDPOINT_HIGHSPEED       0x02

typedef struct _END_POINT_CHARACTERISTICS
{
    ULONG DeviceAddress:7;
    ULONG InactiveOnNextTransaction:1;
    ULONG EndPointNumber:4;
    ULONG EndPointSpeed:2;
    ULONG QEDTDataToggleControl:1;
    ULONG HeadOfReclamation:1;
    ULONG MaximumPacketLength:11;
    ULONG ControlEndPointFlag:1;
    ULONG NakCountReload:4;
} END_POINT_CHARACTERISTICS, *PEND_POINT_CHARACTERISTICS;

typedef struct _END_POINT_CAPABILITIES
{
    ULONG InterruptScheduleMask:8;
    ULONG SplitCompletionMask:8;
    ULONG HubAddr:6;
    ULONG PortNumber:6;
    /* Multi */
    ULONG NumberOfTransactionPerFrame:2;
} END_POINT_CAPABILITIES, *PEND_POINT_CAPABILITIES;


/* QUEUE HEAD defines and structs */

/* QUEUE HEAD Select Types, OR with QUEUE_HEAD HorizontalLinkPointer */
#define QH_TYPE_IDT         0x00
#define QH_TYPE_QH          0x02
#define QH_TYPE_SITD            0x04
#define QH_TYPE_FSTN            0x06

/* QUEUE HEAD */
typedef struct _QUEUE_HEAD
{
    //Hardware
    ULONG HorizontalLinkPointer;
    END_POINT_CHARACTERISTICS EndPointCharacteristics;
    END_POINT_CAPABILITIES EndPointCapabilities;
    /* TERMINATE_POINTER not valid for this member */
    ULONG CurrentLinkPointer;
    /* TERMINATE_POINTER valid */
    ULONG NextPointer;
    /* TERMINATE_POINTER valid, bits 1:4 is NAK_COUNTER */
    ULONG AlternateNextPointer;
    /* Only DataToggle, InterruptOnComplete, ErrorCounter, PingState valid */
    union
    {
        QETD_TOKEN_BITS Bits;
        ULONG DWord;
    } Token;
    ULONG BufferPointer[5];

    //Software
    ULONG PhysicalAddr;
    struct _QUEUE_HEAD *PreviousQueueHead;
    struct _QUEUE_HEAD *NextQueueHead;
    ULONG NumberOfTransferDescriptors;
    PQUEUE_TRANSFER_DESCRIPTOR FirstTransferDescriptor;
    PQUEUE_TRANSFER_DESCRIPTOR DeadDescriptor;
    PIRP IrpToComplete;
    PKEVENT Event;
    PMDL Mdl;
    BOOLEAN FreeMdl;    
} QUEUE_HEAD, *PQUEUE_HEAD;

/* USBCMD register 32 bits */
typedef struct _EHCI_USBCMD_CONTENT
{
    ULONG Run : 1;
    ULONG HCReset : 1;
    ULONG FrameListSize : 2;
    ULONG PeriodicEnable : 1;
    ULONG AsyncEnable : 1;
    ULONG DoorBell : 1;
    ULONG LightReset : 1;
    ULONG AsyncParkCount : 2;
    ULONG Reserved : 1;
    ULONG AsyncParkEnable : 1;
    ULONG Reserved1 : 4;
    ULONG IntThreshold : 8;
    ULONG Reserved2 : 8;

} EHCI_USBCMD_CONTENT, *PEHCI_USBCMD_CONTENT;

typedef struct _EHCI_USBSTS_CONTENT
{
    ULONG USBInterrupt:1;
    ULONG ErrorInterrupt:1;
    ULONG DetectChangeInterrupt:1;
    ULONG FrameListRolloverInterrupt:1;
    ULONG HostSystemErrorInterrupt:1;
    ULONG AsyncAdvanceInterrupt:1;
    ULONG Reserved:6;
    ULONG HCHalted:1;
    ULONG Reclamation:1;
    ULONG PeriodicScheduleStatus:1;
    ULONG AsynchronousScheduleStatus:1;
} EHCI_USBSTS_CONTEXT, *PEHCI_USBSTS_CONTEXT;

typedef struct _EHCI_USBPORTSC_CONTENT
{
    ULONG CurrentConnectStatus:1;
    ULONG ConnectStatusChange:1;
    ULONG PortEnabled:1;
    ULONG PortEnableChanged:1;
    ULONG OverCurrentActive:1;
    ULONG OverCurrentChange:1;
    ULONG ForcePortResume:1;
    ULONG Suspend:1;
    ULONG PortReset:1;
    ULONG Reserved:1;
    ULONG LineStatus:2;
    ULONG PortPower:1;
    ULONG PortOwner:1;
} EHCI_USBPORTSC_CONTENT, *PEHCI_USBPORTSC_CONTENT;

typedef struct _EHCI_HCS_CONTENT
{
    ULONG PortCount : 4;
    ULONG PortPowerControl: 1;
    ULONG Reserved : 2;
    ULONG PortRouteRules : 1;
    ULONG PortPerCHC : 4;
    ULONG CHCCount : 4;
    ULONG PortIndicator : 1;
    ULONG Reserved2 : 3;
    ULONG DbgPortNum : 4;
    ULONG Reserved3 : 8;

} EHCI_HCS_CONTENT, *PEHCI_HCS_CONTENT;

typedef struct _EHCI_HCC_CONTENT
{
    ULONG CurAddrBits : 1;
    ULONG VarFrameList : 1;
    ULONG ParkMode : 1;
    ULONG Reserved : 1;
    ULONG IsoSchedThreshold : 4;
    ULONG EECPCapable : 8;
    ULONG Reserved2 : 16;

} EHCI_HCC_CONTENT, *PEHCI_HCC_CONTENT;

typedef struct _EHCI_CAPS {
    UCHAR Length;
    UCHAR Reserved;
    USHORT HCIVersion;
    union
    {
        EHCI_HCS_CONTENT HCSParams;
        ULONG HCSParamsLong;
    };
    ULONG HCCParams;
    UCHAR PortRoute [8];
} EHCI_CAPS, *PEHCI_CAPS;

typedef struct _EHCIPORTS
{
    ULONG PortNumber;
    ULONG PortType;
    USHORT PortStatus;
    USHORT PortChange;
} EHCIPORTS, *PEHCIPORTS;

typedef struct _EHCI_HOST_CONTROLLER
{
    PDMA_ADAPTER pDmaAdapter;
    ULONG MapRegisters;
    ULONG OpRegisters;
    EHCI_CAPS ECHICaps;
    ULONG NumberOfPorts;
    EHCIPORTS Ports[127];
    PVOID CommonBufferVA[16];
    PHYSICAL_ADDRESS CommonBufferPA[16];
    ULONG CommonBufferSize;
    PQUEUE_HEAD AsyncListQueue;
    PQUEUE_HEAD CompletedListQueue;
    KSPIN_LOCK Lock;
} EHCI_HOST_CONTROLLER, *PEHCI_HOST_CONTROLLER;

ULONG
ReadControllerStatus(PEHCI_HOST_CONTROLLER hcd);

VOID
ClearControllerStatus(PEHCI_HOST_CONTROLLER hcd, ULONG Status);

VOID
GetCapabilities(PEHCI_CAPS PCap, ULONG CapRegister);

VOID
ResetPort(PEHCI_HOST_CONTROLLER hcd, UCHAR Port);

VOID
StartEhci(PEHCI_HOST_CONTROLLER hcd);

VOID
StopEhci(PEHCI_HOST_CONTROLLER hcd);

VOID
SetAsyncListQueueRegister(PEHCI_HOST_CONTROLLER hcd, ULONG PhysicalAddr);

ULONG
GetAsyncListQueueRegister(PEHCI_HOST_CONTROLLER hcd);

VOID
SetPeriodicFrameListRegister(PEHCI_HOST_CONTROLLER hcd, ULONG PhysicalAddr);

ULONG
GetPeriodicFrameListRegister(PEHCI_HOST_CONTROLLER hcd);

BOOLEAN
EnumControllerPorts(PEHCI_HOST_CONTROLLER hcd);
