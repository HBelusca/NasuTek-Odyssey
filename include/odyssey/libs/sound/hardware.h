/*
    Odyssey Sound System
    Legacy support
    Hardware interaction helper

    Author:
        Andrew Greenwood (silverblade@odyssey.org)

    History:
        2 July 2008 - Created
*/

#ifndef ROS_AUDIOLEG_HARDWARE_H
#define ROS_AUDIOLEG_HARDWARE_H

NTSTATUS
LegacyAttachInterrupt(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  UCHAR Irq,
    IN  PKSERVICE_ROUTINE ServiceRoutine,
    OUT PKINTERRUPT* InterruptObject);

#endif
