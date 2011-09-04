/*
 *
 * COPYRIGHT:            See COPYING in the top level directory
 * PROJECT:              Odyssey kernel
 * FILE:                 services/dd/mpu401/mpu401.c
 * PURPOSE:              MPU-401 MIDI device driver
 * PROGRAMMER:           Andrew Greenwood
 * UPDATE HISTORY:
 *                       Sept 26, 2003: Copied from beep.c as template
 *                       Sept 27, 2003: Implemented MPU-401 init & playback
 */

/* INCLUDES ****************************************************************/

#include <ntddk.h>
//#include <ntddbeep.h>

//#define NDEBUG
#include <debug.h>

#include "mpu401.h"


/* INTERNAL VARIABLES ******************************************************/

ULONG DeviceCount = 0;


/* FUNCTIONS ***************************************************************/

static NTSTATUS InitDevice(
    IN PUNICODE_STRING RegistryPath,
    IN PVOID Context)
{
//    PDEVICE_INSTANCE Instance = Context;
    PDEVICE_OBJECT DeviceObject; // = Context;
    PDEVICE_EXTENSION Parameters; // = DeviceObject->DeviceExtension;
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\MidiOut0");
    UNICODE_STRING SymlinkName = RTL_CONSTANT_STRING(L"\\??\\MidiOut0");
//    CONFIG Config;
    RTL_QUERY_REGISTRY_TABLE Table[2];
    NTSTATUS s;

    // This is TEMPORARY, to ensure that we don't process more than 1 device.
    // I'll remove this limitation in the future.
    if (DeviceCount > 0)
    {
        DPRINT("Sorry - only 1 device supported by MPU401 driver at present :(\n");
        return STATUS_NOT_IMPLEMENTED;
    }

    DPRINT("Creating IO device\n");

    s = IoCreateDevice(Context, // driverobject
			  sizeof(DEVICE_EXTENSION),
			  &DeviceName,
			  FILE_DEVICE_SOUND, // Correct?
			  0,
			  FALSE,
			  &DeviceObject);

    if (!NT_SUCCESS(s))
        return s;

    DPRINT("Device Extension at 0x%x\n", DeviceObject->DeviceExtension);
    Parameters = DeviceObject->DeviceExtension;

    DPRINT("Creating DOS link\n");

    /* Create the dos device link */
    IoCreateSymbolicLink(&SymlinkName,
		       &DeviceName);

    DPRINT("Initializing device\n");

//    DPRINT("Allocating memory for parameters structure\n");
    // Bodged:
//    Parameters = (PDEVICE_EXTENSION)ExAllocatePool(NonPagedPool, sizeof(DEVICE_EXTENSION));
//    DeviceObject->DeviceExtension = Parameters;
//    Parameters = Instance->DriverObject->DriverExtension;

    DPRINT("DeviceObject at 0x%x, DeviceExtension at 0x%x\n", DeviceObject, Parameters);

    if (! Parameters)
    {
        DPRINT("NULL POINTER!\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

//    Instance->DriverObject->DriverExtension = Parameters;

    DPRINT("Setting reg path\n");
    Parameters->RegistryPath = RegistryPath;
//    Parameters->DriverObject = Instance->DriverObject;

    DPRINT("Zeroing table memory and setting query routine\n");
    RtlZeroMemory(Table, sizeof(Table));
    Table[0].QueryRoutine = LoadSettings;

    DPRINT("Setting port and IRQ defaults\n");
    Parameters->Port = DEFAULT_PORT;
    Parameters->IRQ = DEFAULT_IRQ;

// Only to be enabled once we can get support for multiple cards working :)
/*
    DPRINT("Loading settings from: %S\n", RegistryPath);

    s = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, RegistryPath, Table,
                                &Parameters, NULL);
*/

    if (! NT_SUCCESS(s))
        return s;

    DPRINT("Port 0x%x  IRQ %d\n", Parameters->Port, Parameters->IRQ);

//    Instance->P

    // Enter UART mode (should be done in init phase
    if (! InitUARTMode(Parameters->Port))
    {
        DPRINT("UART mode initialization FAILED!\n");
        // Set state indication somehow
        // Failure - what error code do we give?!
        // return STATUS_????
    }

    DeviceCount ++;

    return STATUS_SUCCESS;
}


static NTSTATUS NTAPI
MPU401Create(PDEVICE_OBJECT DeviceObject,
	   PIRP Irp)
/*
 * FUNCTION: Handles user mode requests
 * ARGUMENTS:
 *                       DeviceObject = Device for request
 *                       Irp = I/O request packet describing request
 * RETURNS: Success or failure
 */
{
    DPRINT("MPU401Create() called!\n");

    // Initialize the MPU-401?
    // ... do stuff ...


    // Play a note to say we're alive:
    WaitToSend(MPU401_PORT);
    MPU401_WRITE_DATA(MPU401_PORT, 0x90);
    WaitToSend(MPU401_PORT);
    MPU401_WRITE_DATA(MPU401_PORT, 0x50);
    WaitToSend(MPU401_PORT);
    MPU401_WRITE_DATA(MPU401_PORT, 0x7f);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp,
		    IO_NO_INCREMENT);

    return(STATUS_SUCCESS);
}


static NTSTATUS NTAPI
MPU401Close(PDEVICE_OBJECT DeviceObject,
	  PIRP Irp)
/*
 * FUNCTION: Handles user mode requests
 * ARGUMENTS:
 *                       DeviceObject = Device for request
 *                       Irp = I/O request packet describing request
 * RETURNS: Success or failure
 */
{
  PDEVICE_EXTENSION DeviceExtension;
  NTSTATUS Status;

  DPRINT("MPU401Close() called!\n");

  DeviceExtension = DeviceObject->DeviceExtension;

  Status = STATUS_SUCCESS;

  Irp->IoStatus.Status = Status;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp,
		    IO_NO_INCREMENT);

  return(Status);
}


static NTSTATUS NTAPI
MPU401Cleanup(PDEVICE_OBJECT DeviceObject,
	    PIRP Irp)
/*
 * FUNCTION: Handles user mode requests
 * ARGUMENTS:
 *                       DeviceObject = Device for request
 *                       Irp = I/O request packet describing request
 * RETURNS: Success or failure
 */
{
  ULONG Channel;
  DPRINT("MPU401Cleanup() called!\n");

    // Reset the device (should we do this?)
    for (Channel = 0; Channel <= 15; Channel ++)
    {
        // All notes off
//        MPU401_WRITE_MESSAGE(MPU401_PORT, 0xb0 + Channel, 123, 0);
        // All controllers off
//        MPU401_WRITE_MESSAGE(MPU401_PORT, 0xb0 + Channel, 121, 0);
    }


  Irp->IoStatus.Status = STATUS_SUCCESS;
  Irp->IoStatus.Information = 0;
  IoCompleteRequest(Irp,
		    IO_NO_INCREMENT);

  return(STATUS_SUCCESS);
}


static NTSTATUS NTAPI
MPU401DeviceControl(PDEVICE_OBJECT DeviceObject,
		  PIRP Irp)
/*
 * FUNCTION: Handles user mode requests
 * ARGUMENTS:
 *                       DeviceObject = Device for request
 *                       Irp = I/O request packet describing request
 * RETURNS: Success or failure
 */
{
    PIO_STACK_LOCATION Stack;
    PDEVICE_EXTENSION DeviceExtension;
    ULONG ByteCount;
    PUCHAR Data;

    DPRINT("MPU401DeviceControl() called!\n");

    DeviceExtension = DeviceObject->DeviceExtension;
    Stack = IoGetCurrentIrpStackLocation(Irp);

    DPRINT("Control code %d [0x%x]\n", Stack->Parameters.DeviceIoControl.IoControlCode,
                Stack->Parameters.DeviceIoControl.IoControlCode);

    switch(Stack->Parameters.DeviceIoControl.IoControlCode)
    {
        case IOCTL_MIDI_PLAY :
        {
            DPRINT("Received IOCTL_MIDI_PLAY\n");
            Data = (PUCHAR) Irp->AssociatedIrp.SystemBuffer;

            DPRINT("Sending %d bytes of MIDI data to 0x%d:\n", Stack->Parameters.DeviceIoControl.InputBufferLength, DeviceExtension->Port);

            for (ByteCount = 0; ByteCount < Stack->Parameters.DeviceIoControl.InputBufferLength; ByteCount ++)
            {
                DPRINT("0x%x ", Data[ByteCount]);

                MPU401_WRITE_BYTE(DeviceExtension->Port, Data[ByteCount]);
//                if (WaitToSend(MPU401_PORT))
//                    MPU401_WRITE_DATA(MPU401_PORT, Data[ByteCount]);
            }

            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);

            return(STATUS_SUCCESS);
        }
    }

    return(STATUS_SUCCESS);

/*
  DeviceExtension = DeviceObject->DeviceExtension;
  Stack = IoGetCurrentIrpStackLocation(Irp);
  BeepParam = (PBEEP_SET_PARAMETERS)Irp->AssociatedIrp.SystemBuffer;

  Irp->IoStatus.Information = 0;

  if (Stack->Parameters.DeviceIoControl.IoControlCode != IOCTL_BEEP_SET)
    {
      Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
      IoCompleteRequest(Irp,
			IO_NO_INCREMENT);
      return(STATUS_NOT_IMPLEMENTED);
    }

  if ((Stack->Parameters.DeviceIoControl.InputBufferLength != sizeof(BEEP_SET_PARAMETERS))
      || (BeepParam->Frequency < BEEP_FREQUENCY_MINIMUM)
      || (BeepParam->Frequency > BEEP_FREQUENCY_MAXIMUM))
    {
      Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
      IoCompleteRequest(Irp,
			IO_NO_INCREMENT);
      return(STATUS_INVALID_PARAMETER);
    }

  DueTime.QuadPart = 0;
*/
  /* do the beep!! */
/*  DPRINT("Beep:\n  Freq: %lu Hz\n  Dur: %lu ms\n",
	 pbsp->Frequency,
	 pbsp->Duration);

  if (BeepParam->Duration >= 0)
    {
      DueTime.QuadPart = (LONGLONG)BeepParam->Duration * -10000;

      KeSetTimer(&DeviceExtension->Timer,
		 DueTime,
		 &DeviceExtension->Dpc);

      HalMakeBeep(BeepParam->Frequency);
      DeviceExtension->BeepOn = TRUE;
      KeWaitForSingleObject(&DeviceExtension->Event,
			    Executive,
			    KernelMode,
			    FALSE,
			    NULL);
    }
  else if (BeepParam->Duration == (DWORD)-1)
    {
      if (DeviceExtension->BeepOn == TRUE)
	{
	  HalMakeBeep(0);
	  DeviceExtension->BeepOn = FALSE;
	}
      else
	{
	  HalMakeBeep(BeepParam->Frequency);
	  DeviceExtension->BeepOn = TRUE;
	}
    }

  DPRINT("Did the beep!\n");

  Irp->IoStatus.Status = STATUS_SUCCESS;
  IoCompleteRequest(Irp,
		    IO_NO_INCREMENT);
  return(STATUS_SUCCESS);
*/
}


static VOID NTAPI
MPU401Unload(PDRIVER_OBJECT DriverObject)
{
  DPRINT("MPU401Unload() called!\n");
}


NTSTATUS NTAPI
DriverEntry(PDRIVER_OBJECT DriverObject,
	    PUNICODE_STRING RegistryPath)
/*
 * FUNCTION:  Called by the system to initalize the driver
 * ARGUMENTS:
 *            DriverObject = object describing this driver
 *            RegistryPath = path to our configuration entries
 * RETURNS:   Success or failure
 */
{
//  PDEVICE_EXTENSION DeviceExtension;
//  PDEVICE_OBJECT DeviceObject;
//  DEVICE_INSTANCE Instance;
  // Doesn't support multiple instances (yet ...)
  NTSTATUS Status;

  DPRINT("MPU401 Device Driver 0.0.1\n");

    // Is this really necessary? Yes! (Talking to myself again...)
//    Instance.DriverObject = DriverObject;
    // previous instance = NULL...

//    DeviceExtension->RegistryPath = RegistryPath;

  DriverObject->Flags = 0;
  DriverObject->MajorFunction[IRP_MJ_CREATE] = MPU401Create;
  DriverObject->MajorFunction[IRP_MJ_CLOSE] = MPU401Close;
  DriverObject->MajorFunction[IRP_MJ_CLEANUP] = MPU401Cleanup;
  DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MPU401DeviceControl;
  DriverObject->DriverUnload = MPU401Unload;

    // Major hack to just get this damn thing working:
    Status = InitDevice(RegistryPath, DriverObject);    // ????

//    DPRINT("Enumerating devices at %wZ\n", RegistryPath);

//    Status = EnumDeviceKeys(RegistryPath, PARMS_SUBKEY, InitDevice, (PVOID)&DeviceObject); // &Instance;

    // check error

  /* set up device extension */
//  DeviceExtension = DeviceObject->DeviceExtension;
//  DeviceExtension->BeepOn = FALSE;

  return(STATUS_SUCCESS);
}

/* EOF */
