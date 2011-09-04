/*
 * VideoPort driver
 *
 * Copyright (C) 2002, 2003, 2004 ReactOS Team; (C) 2011 NasuTek Enterprises
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "videoprt.h"

/* PRIVATE FUNCTIONS **********************************************************/

VOID NTAPI
IntVideoPortTimerRoutine(
   IN PDEVICE_OBJECT DeviceObject,
   IN PVOID ServiceContext)
{
   PVIDEO_PORT_DEVICE_EXTENSION DeviceExtension = ServiceContext;

   ASSERT(DeviceExtension->DriverExtension->InitializationData.HwTimer != NULL);

   DeviceExtension->DriverExtension->InitializationData.HwTimer(
      &DeviceExtension->MiniPortDeviceExtension);
}

BOOLEAN NTAPI
IntVideoPortSetupTimer(
   IN PDEVICE_OBJECT DeviceObject,
   IN PVIDEO_PORT_DRIVER_EXTENSION DriverExtension)
{
   NTSTATUS Status;
   PVIDEO_PORT_DEVICE_EXTENSION DeviceExtension;

   DeviceExtension = (PVIDEO_PORT_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

   if (DriverExtension->InitializationData.HwTimer != NULL)
   {
      INFO_(VIDEOPRT, "Initializing timer\n");

      Status = IoInitializeTimer(
         DeviceObject,
         IntVideoPortTimerRoutine,
         DeviceExtension);

      if (!NT_SUCCESS(Status))
      {
         WARN_(VIDEOPRT, "IoInitializeTimer failed with status 0x%08x\n", Status);
         return FALSE;
      }
   }

   return TRUE;
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * @implemented
 */

VOID NTAPI
VideoPortStartTimer(IN PVOID HwDeviceExtension)
{
   TRACE_(VIDEOPRT, "VideoPortStartTimer\n");
   IoStartTimer(VIDEO_PORT_GET_DEVICE_EXTENSION(HwDeviceExtension)->FunctionalDeviceObject);
}

/*
 * @implemented
 */

VOID NTAPI
VideoPortStopTimer(IN PVOID HwDeviceExtension)
{
   TRACE_(VIDEOPRT, "VideoPortStopTimer\n");
   IoStopTimer(VIDEO_PORT_GET_DEVICE_EXTENSION(HwDeviceExtension)->FunctionalDeviceObject);
}
