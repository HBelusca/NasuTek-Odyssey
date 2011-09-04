/*
 * PROJECT:         Odyssey USB Drivers
 * COPYRIGHT:       GPL - See COPYING in the top level directory
 * FILE:            mouse.c
 * PURPOSE:         Generic USB mouse driver
 * PROGRAMMERS:     Aleksey Bragin (aleksey@odyssey.org)
 */

#include "usbdriver.h"

BOOLEAN mouse_connect(PDEV_CONNECT_DATA dev_mgr, DEV_HANDLE dev_handle);
BOOLEAN mouse_disconnect(PUSB_DEV_MANAGER dev_mgr, DEV_HANDLE dev_handle);
BOOLEAN mouse_stop(PUSB_DEV_MANAGER dev_mgr, DEV_HANDLE dev_handle);
VOID mouse_irq(PURB purb, PVOID pcontext);
static NTSTATUS MouseCreateDevice(IN PDRIVER_OBJECT DriverObject, IN PMOUSE_DRVR_EXTENSION DriverExtension);

BOOLEAN
mouse_driver_init(PUSB_DEV_MANAGER dev_mgr, PUSB_DRIVER pdriver)
{
    PMOUSE_DRVR_EXTENSION pdrvr_ext;

    if (dev_mgr == NULL || pdriver == NULL)
        return FALSE;

    //init driver structure, no PNP table functions
    pdriver->driver_desc.flags = USB_DRIVER_FLAG_IF_CAPABLE;
    pdriver->driver_desc.vendor_id = 0xffff;    // USB Vendor ID
    pdriver->driver_desc.product_id = 0xffff;   // USB Product ID.
    pdriver->driver_desc.release_num = 0x100;   // Release Number of Device

    pdriver->driver_desc.config_val = 1;            // Configuration Value
    pdriver->driver_desc.if_num = 1;                // Interface Number
    pdriver->driver_desc.if_class = USB_CLASS_HID;  // Interface Class
    pdriver->driver_desc.if_sub_class = 1;          // Interface SubClass
    pdriver->driver_desc.if_protocol = 2;           // Interface Protocol

    pdriver->driver_desc.driver_name = "USB Mouse driver"; // Driver name for Name Registry
    pdriver->driver_desc.dev_class = USB_CLASS_HID;
    pdriver->driver_desc.dev_sub_class = 1;         // Device Subclass
    pdriver->driver_desc.dev_protocol = 2;          // Protocol Info.

    pdriver->driver_ext = usb_alloc_mem(NonPagedPool, sizeof(MOUSE_DRVR_EXTENSION));
    if (!pdriver->driver_ext) return FALSE;

    pdriver->driver_ext_size = sizeof(MOUSE_DRVR_EXTENSION);

    RtlZeroMemory(pdriver->driver_ext, sizeof(MOUSE_DRVR_EXTENSION));
    pdrvr_ext = (PMOUSE_DRVR_EXTENSION) pdriver->driver_ext;
    pdrvr_ext->dev_mgr = dev_mgr;

    pdriver->disp_tbl.version = 1;
    pdriver->disp_tbl.dev_connect = mouse_connect;
    pdriver->disp_tbl.dev_disconnect = mouse_disconnect;
    pdriver->disp_tbl.dev_stop = mouse_stop;
    pdriver->disp_tbl.dev_reserved = NULL;

    // Create the device
    MouseCreateDevice(dev_mgr->usb_driver_obj, pdrvr_ext);

    usb_dbg_print(DBGLVL_MAXIMUM, ("mouse_driver_init(): mouse driver is initialized\n"));
    return TRUE;
}

BOOLEAN
mouse_driver_destroy(PUSB_DEV_MANAGER dev_mgr, PUSB_DRIVER pdriver)
{
    //PMOUSE_DRVR_EXTENSION pdrvr_ext;
    if (dev_mgr == NULL || pdriver == NULL)
        return FALSE;

    //pdrvr_ext = (PUMSS_DRVR_EXTENSION) pdriver->driver_ext;
    //umss_delete_port_device(pdrvr_ext->port_dev_obj);
    //pdrvr_ext->port_dev_obj = NULL;

    //ASSERT(IsListEmpty(&pdrvr_ext->dev_list) == TRUE);
    usb_free_mem(pdriver->driver_ext);
    pdriver->driver_ext = NULL;
    pdriver->driver_ext_size = 0;
    usb_dbg_print(DBGLVL_MAXIMUM, ("mouse_driver_destroy(): mouse driver is destroyed\n"));
    return TRUE;
}

BOOLEAN
mouse_connect(PDEV_CONNECT_DATA param, DEV_HANDLE dev_handle)
{
    PURB purb;
    NTSTATUS status;
    PUSB_DEV_MANAGER dev_mgr;
    PUSB_DRIVER pdrvr;
    PUSB_DEV pdev;
    PMOUSE_DRVR_EXTENSION pdev_ext;
//    LONG i;
    PUSB_ENDPOINT_DESC pendp_desc = NULL;
    ULONG MaxPacketSize;

    usb_dbg_print(DBGLVL_MAXIMUM, ("mouse_connect(): entering...\n"));

    dev_mgr = param->dev_mgr;
    pdrvr = param->pdriver;
    pdev_ext = (PMOUSE_DRVR_EXTENSION)pdrvr->driver_ext;

    // Lock USB Device
    status = usb_query_and_lock_dev(dev_mgr, dev_handle, &pdev);
    if (status != STATUS_SUCCESS)
    {
        //usb_free_mem(desc_buf);
        usb_dbg_print(DBGLVL_MEDIUM, ("mouse_connect(): unable to query&lock device, status=0x%x\n", status));
        return FALSE;
    }

    // Get pointer to the endpoint descriptor
    pendp_desc = pdev->usb_config->interf[0].endp[0].pusb_endp_desc;

    // Store max packet size
    MaxPacketSize = pendp_desc->wMaxPacketSize;
    if (MaxPacketSize > 8)
        MaxPacketSize = 8;

    // Unlock USB Device
    usb_unlock_dev(pdev);

    // Send URB
    purb = usb_alloc_mem(NonPagedPool, sizeof(URB));
    if (purb == NULL)
        return FALSE;
    RtlZeroMemory(purb, sizeof(URB));

    // Build a URB for our interrupt transfer
    UsbBuildInterruptOrBulkTransferRequest(purb,
                                           usb_make_handle((dev_handle >> 16), 0, 0),
                                           (PUCHAR)&pdev_ext->mouse_data,
                                           MaxPacketSize, //use max packet size
                                           mouse_irq,
                                           pdev_ext->device_ext,
                                           0);

    // Call USB driver stack
    status = usb_submit_urb(pdev_ext->dev_mgr, purb);
    if (status != STATUS_PENDING)
    {
        usb_free_mem(purb);
        purb = NULL;
    }

    return TRUE;
}

VOID
mouse_irq(PURB purb, PVOID pcontext)
{
    MOUSE_INPUT_DATA MouseInputData;
    ULONG InputDataConsumed;
    NTSTATUS status;
    PMOUSE_DRVR_EXTENSION pdev_ext;
    PMOUSE_DEVICE_EXTENSION DeviceExtension = (PMOUSE_DEVICE_EXTENSION)pcontext;
    signed char *data;
    usb_dbg_print(DBGLVL_MAXIMUM, ("mouse_irq(): called\n"));

    ASSERT(purb);

    if (purb->status != STATUS_SUCCESS)
    {
        usb_dbg_print(DBGLVL_MAXIMUM, ("mouse_irq(): purb->status 0x%08X\n", purb->status));
        return;
    }

    pdev_ext = DeviceExtension->DriverExtension;
    data = pdev_ext->mouse_data;

    usb_dbg_print(DBGLVL_MAXIMUM, ("Mouse input: x %d, y %d, w %d, btn: 0x%02x\n", data[1], data[2], data[3], data[0]));

    // Fill mouse input data structure
    MouseInputData.Flags = MOUSE_MOVE_RELATIVE;
    MouseInputData.LastX = data[1];
    MouseInputData.LastY = data[2];

    MouseInputData.ButtonFlags = 0;
    MouseInputData.ButtonData = 0;

    if ((data[0] & 0x01) && ((pdev_ext->btn_old & 0x01) != (data[0] & 0x01)))
        MouseInputData.ButtonFlags |= MOUSE_LEFT_BUTTON_DOWN;
    else if (!(data[0] & 0x01) && ((pdev_ext->btn_old & 0x01) != (data[0] & 0x01)))
        MouseInputData.ButtonFlags |= MOUSE_LEFT_BUTTON_UP;

    if ((data[0] & 0x02) && ((pdev_ext->btn_old & 0x02) != (data[0] & 0x02)))
        MouseInputData.ButtonFlags |= MOUSE_RIGHT_BUTTON_DOWN;
    else if (!(data[0] & 0x02) && ((pdev_ext->btn_old & 0x02) != (data[0] & 0x02)))
        MouseInputData.ButtonFlags |= MOUSE_RIGHT_BUTTON_UP;

    if ((data[0] & 0x04) && ((pdev_ext->btn_old & 0x04) != (data[0] & 0x04)))
        MouseInputData.ButtonFlags |= MOUSE_MIDDLE_BUTTON_DOWN;
    else if (!(data[0] & 0x04) && ((pdev_ext->btn_old & 0x04) != (data[0] & 0x04)))
        MouseInputData.ButtonFlags |= MOUSE_MIDDLE_BUTTON_UP;

    if ((data[0] & 0x08) && ((pdev_ext->btn_old & 0x08) != (data[0] & 0x08)))
        MouseInputData.ButtonFlags |= MOUSE_BUTTON_4_DOWN;
    else if (!(data[0] & 0x08) && ((pdev_ext->btn_old & 0x08) != (data[0] & 0x08)))
        MouseInputData.ButtonFlags |= MOUSE_BUTTON_4_UP;

    if ((data[0] & 0x10) && ((pdev_ext->btn_old & 0x10) != (data[0] & 0x10)))
        MouseInputData.ButtonFlags |= MOUSE_BUTTON_5_DOWN;
    else if (!(data[0] & 0x10) && ((pdev_ext->btn_old & 0x10) != (data[0] & 0x10)))
        MouseInputData.ButtonFlags |= MOUSE_BUTTON_5_UP;

    if (data[3])
    {
        MouseInputData.ButtonFlags |= MOUSE_WHEEL;
        MouseInputData.ButtonData = data[3];
    }

    // Commit the input data somewhere...
    if (DeviceExtension->ConnectData.ClassService)
    {
        KIRQL OldIrql;

        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
        (*(PSERVICE_CALLBACK_ROUTINE)DeviceExtension->ConnectData.ClassService)(
            DeviceExtension->ConnectData.ClassDeviceObject,
            &MouseInputData,
            (&MouseInputData)+1,
            &InputDataConsumed);
        KeLowerIrql(OldIrql);
    }

    // Save old button data
    pdev_ext->btn_old = data[0];

    // resubmit the urb
    status = usb_submit_urb(pdev_ext->dev_mgr, purb);
}

BOOLEAN
mouse_stop(PUSB_DEV_MANAGER dev_mgr, DEV_HANDLE dev_handle)
{
    UNREFERENCED_PARAMETER(dev_handle);
    UNREFERENCED_PARAMETER(dev_mgr);
    return TRUE;
}

BOOLEAN
mouse_disconnect(PUSB_DEV_MANAGER dev_mgr, DEV_HANDLE dev_handle)
{
    PDEVICE_OBJECT dev_obj;
    NTSTATUS status;
    PUSB_DEV pdev;
    PUSB_DRIVER pdrvr;

    if (dev_mgr == NULL || dev_handle == 0)
        return FALSE;

    pdev = NULL;
    //special use of the lock dev, simply use this routine to get the dev
    status = usb_query_and_lock_dev(dev_mgr, dev_handle, &pdev);
    if (pdev == NULL)
    {
        return FALSE;
    }
    if (status == STATUS_SUCCESS)
    {
        // must be a bug
        TRAP();
        usb_unlock_dev(pdev);
    }
    pdrvr = pdev->dev_driver;
    dev_obj = pdev->dev_obj;
    pdev = NULL;

    return TRUE;//umss_delete_device(dev_mgr, pdrvr, dev_obj, FALSE);
}

// Dispatch routine for our IRPs
NTSTATUS
MouseDispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
    PMOUSE_DEVICE_EXTENSION DeviceExtension;

    DeviceExtension = DeviceObject->DeviceExtension;

    usb_dbg_print(DBGLVL_MAXIMUM, ("MouseDispatch(DO %p, code 0x%lx) called\n",
        DeviceObject,
        IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.IoControlCode));

    if (DeviceObject == DeviceExtension->Fdo)
    {
        // it's mouse's IOCTL
        PIO_STACK_LOCATION Stk;

        Irp->IoStatus.Information = 0;
        Stk = IoGetCurrentIrpStackLocation(Irp);

        switch (Stk->Parameters.DeviceIoControl.IoControlCode)
        {
        case IOCTL_INTERNAL_MOUSE_CONNECT:
            usb_dbg_print(DBGLVL_MAXIMUM, ("IOCTL_INTERNAL_MOUSE_CONNECT\n"));
            if (Stk->Parameters.DeviceIoControl.InputBufferLength <	sizeof(CONNECT_DATA)) {
                usb_dbg_print(DBGLVL_MINIMUM, ("IOCTL_INTERNAL_MOUSE_CONNECT: "
                    "invalid buffer size\n"));
                Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
                goto intcontfailure2;
            }

            RtlCopyMemory(&DeviceExtension->ConnectData,
                Stk->Parameters.DeviceIoControl.Type3InputBuffer,
                sizeof(CONNECT_DATA));

            Irp->IoStatus.Status = STATUS_SUCCESS;
            break;

        default:
            Irp->IoStatus.Status = STATUS_SUCCESS;//STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
intcontfailure2:
        Status = Irp->IoStatus.Status;
    }

    if (Status == STATUS_INVALID_DEVICE_REQUEST)
    {
        usb_dbg_print(DBGLVL_MINIMUM, ("Invalid internal device request!\n"));
    }

    if (Status != STATUS_PENDING)
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return Status;
}

NTSTATUS
AddRegistryEntry(
                 IN PCWSTR PortTypeName,
                 IN PUNICODE_STRING DeviceName,
                 IN PCWSTR RegistryPath)
{
    UNICODE_STRING PathU = RTL_CONSTANT_STRING(L"\\REGISTRY\\MACHINE\\HARDWARE\\DEVICEMAP");
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hDeviceMapKey = (HANDLE)-1;
    HANDLE hPortKey = (HANDLE)-1;
    UNICODE_STRING PortTypeNameU;
    NTSTATUS Status;

    InitializeObjectAttributes(&ObjectAttributes, &PathU, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    Status = ZwOpenKey(&hDeviceMapKey, 0, &ObjectAttributes);
    if (!NT_SUCCESS(Status))
    {
        usb_dbg_print(DBGLVL_MINIMUM, ("ZwOpenKey() failed with status 0x%08lx\n", Status));
        goto cleanup;
    }

    RtlInitUnicodeString(&PortTypeNameU, PortTypeName);
    InitializeObjectAttributes(&ObjectAttributes, &PortTypeNameU, OBJ_KERNEL_HANDLE, hDeviceMapKey, NULL);
    Status = ZwCreateKey(&hPortKey, KEY_SET_VALUE, &ObjectAttributes, 0, NULL, REG_OPTION_VOLATILE, NULL);
    if (!NT_SUCCESS(Status))
    {
        usb_dbg_print(DBGLVL_MINIMUM, ("ZwCreateKey() failed with status 0x%08lx\n", Status));
        goto cleanup;
    }

    Status = ZwSetValueKey(hPortKey, DeviceName, 0, REG_SZ, (PVOID)RegistryPath, (ULONG)(wcslen(RegistryPath) * sizeof(WCHAR) + sizeof(UNICODE_NULL)));
    if (!NT_SUCCESS(Status))
    {
        usb_dbg_print(DBGLVL_MINIMUM, ("ZwSetValueKey() failed with status 0x%08lx\n", Status));
        goto cleanup;
    }

    Status = STATUS_SUCCESS;

cleanup:
    if (hDeviceMapKey != (HANDLE)-1)
        ZwClose(hDeviceMapKey);
    if (hPortKey != (HANDLE)-1)
        ZwClose(hPortKey);
    return Status;
}

static NTSTATUS
MouseCreateDevice(IN PDRIVER_OBJECT DriverObject, IN PMOUSE_DRVR_EXTENSION DriverExtension)
{
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\PointerPortUSB");
    PMOUSE_DEVICE_EXTENSION DeviceExtension;
    PDEVICE_OBJECT Fdo;
    NTSTATUS Status;

    Status = AddRegistryEntry(L"PointerPort", &DeviceName, L"REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\usbdriver");
    if (!NT_SUCCESS(Status))
    {
        usb_dbg_print(DBGLVL_MINIMUM, ("AddRegistryEntry() for usb mouse driver failed with status 0x%08lx\n", Status));
        return Status;
    }

    Status = IoCreateDevice(DriverObject,
        sizeof(MOUSE_DEVICE_EXTENSION),
        &DeviceName,
        FILE_DEVICE_MOUSE,
        FILE_DEVICE_SECURE_OPEN,
        TRUE,
        &Fdo);

    if (!NT_SUCCESS(Status))
    {
        usb_dbg_print(DBGLVL_MINIMUM, ("IoCreateDevice() for usb mouse driver failed with status 0x%08lx\n", Status));
        return Status;
    }
    DeviceExtension = (PMOUSE_DEVICE_EXTENSION)Fdo->DeviceExtension;
    RtlZeroMemory(DeviceExtension, sizeof(MOUSE_DEVICE_EXTENSION));

    DeviceExtension->hdr.dispatch = MouseDispatch;
    DeviceExtension->DriverExtension = DriverExtension;
    DriverExtension->device_ext = DeviceExtension;

    DeviceExtension->Fdo = Fdo;
    Fdo->Flags &= ~DO_DEVICE_INITIALIZING;
    usb_dbg_print(DBGLVL_MEDIUM, ("Created mouse Fdo: %p\n", Fdo));

    return STATUS_SUCCESS;
}

