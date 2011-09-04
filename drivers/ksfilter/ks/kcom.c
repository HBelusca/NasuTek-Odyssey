/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey Kernel Streaming
 * FILE:            drivers/ksfilter/ks/allocators.c
 * PURPOSE:         KS Allocator functions
 * PROGRAMMER:      Johannes Anderwald
                    Andrew Greenwood
 */

#include "priv.h"

const GUID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x46}};

/* http://msdn2.microsoft.com/en-us/library/ms809781.aspx */
COMDDKAPI NTSTATUS NTAPI
KoCreateInstance(
    IN  REFCLSID ClassId,
    IN  IUnknown* UnkOuter OPTIONAL,
    IN  ULONG ClsContext,
    IN  REFIID InterfaceId,
    OUT PVOID* Interface)
{
    /* If UnkOuter isn't NULL, it must be IUnknown - TODO: CHECK THIS PARAM */
    /* TODO: Check IRQL? */

    DPRINT("KoCreateInstance called\n");

    if ( ClsContext != CLSCTX_KERNEL_SERVER )
    {
        DPRINT("KoCreateInstance: ClsContext must be CLSCTX_KERNEL_SERVER\n");
        return STATUS_INVALID_PARAMETER_3;
    }

    if (IsEqualGUIDAligned(InterfaceId, &IID_IUnknown))
    {
        DPRINT("KoCreateInstance: InterfaceId cannot be IID_IUnknown\n");
        return STATUS_INVALID_PARAMETER_4;
    }


    /*
        Find the desired interface and create an instance.

        But we also need to supply a
        pointer which will be set to a list of available interfaces, to
        IoGetDeviceInterfaces.

        We can then create a file based on this information and thus talk
        to the appropriate device.

        Useful references:
            http://www.freelists.org/archives/wdmaudiodev/01-2003/msg00023.html

        TODO
    */

    DPRINT("** FAKING SUCCESS **\n");

    return STATUS_SUCCESS;
}
