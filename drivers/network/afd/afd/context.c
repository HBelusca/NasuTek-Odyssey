/* $Id: context.c 52929 2011-07-27 19:29:34Z akhaldi $
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * FILE:             drivers/net/afd/afd/context.c
 * PURPOSE:          Ancillary functions driver
 * PROGRAMMER:       Art Yerkes (ayerkes@speakeasy.net)
 * UPDATE HISTORY:
 * 20040708 Created
 */
#include "afd.h"

NTSTATUS NTAPI
AfdGetContext( PDEVICE_OBJECT DeviceObject, PIRP Irp,
	       PIO_STACK_LOCATION IrpSp ) {
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    UINT ContextSize = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if( !SocketAcquireStateLock( FCB ) ) return LostSocket( Irp );

    if( FCB->ContextSize < ContextSize ) ContextSize = FCB->ContextSize;

    if( FCB->Context ) {
	RtlCopyMemory( Irp->UserBuffer,
		       FCB->Context,
		       ContextSize );
	Status = STATUS_SUCCESS;
    }

    AFD_DbgPrint(MID_TRACE,("Returning %x\n", Status));

    return UnlockAndMaybeComplete( FCB, Status, Irp, ContextSize );
}

NTSTATUS NTAPI
AfdGetContextSize( PDEVICE_OBJECT DeviceObject, PIRP Irp,
	           PIO_STACK_LOCATION IrpSp )
{
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;

    if( !SocketAcquireStateLock( FCB ) ) return LostSocket( Irp );

    if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(ULONG))
    {
        AFD_DbgPrint(MIN_TRACE,("Buffer too small\n"));
        return UnlockAndMaybeComplete(FCB, STATUS_BUFFER_TOO_SMALL, Irp, sizeof(ULONG));
    }

    RtlCopyMemory(Irp->UserBuffer,
                  &FCB->ContextSize,
                  sizeof(ULONG));

    return UnlockAndMaybeComplete(FCB, STATUS_SUCCESS, Irp, sizeof(ULONG));
}

NTSTATUS NTAPI
AfdSetContext( PDEVICE_OBJECT DeviceObject, PIRP Irp,
	       PIO_STACK_LOCATION IrpSp ) {
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PAFD_FCB FCB = FileObject->FsContext;
    PVOID Context = LockRequest(Irp, IrpSp);

    if( !SocketAcquireStateLock( FCB ) ) return LostSocket( Irp );
    
    if (!Context)
        return UnlockAndMaybeComplete(FCB, STATUS_NO_MEMORY, Irp, 0);

    if( FCB->Context ) {
	ExFreePool( FCB->Context );
	FCB->ContextSize = 0;
    }

    FCB->Context = ExAllocatePool( PagedPool,
	                           IrpSp->Parameters.DeviceIoControl.InputBufferLength );

    if( !FCB->Context ) return UnlockAndMaybeComplete( FCB, STATUS_NO_MEMORY, Irp, 0 );

    FCB->ContextSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

    RtlCopyMemory( FCB->Context,
		   Context,
		   FCB->ContextSize );

    return UnlockAndMaybeComplete( FCB, STATUS_SUCCESS, Irp, 0 );
}
