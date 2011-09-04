/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TDI interface
 * FILE:        enum.c
 * PURPOSE:     TDI entity enumeration
 */

#include "iphlpapi_private.h"
#include "tdilib.h"

/* A generic thing-getting function which interacts in the right way with
 * TDI.  This may seem oblique, but I'm using it to reduce code and hopefully
 * make this thing easier to debug.
 *
 * The things returned can be any of:
 *   TDIEntityID
 *   TDIObjectID
 *   IFEntry
 *   IPSNMPInfo
 *   IPAddrEntry
 *   IPInterfaceInfo
 */
NTSTATUS tdiGetSetOfThings( HANDLE tcpFile,
                            DWORD toiClass,
                            DWORD toiType,
                            DWORD toiId,
                            DWORD teiEntity,
			    DWORD teiInstance,
                            DWORD fixedPart,
                            DWORD entrySize,
                            PVOID *tdiEntitySet,
                            PDWORD numEntries ) {
    TCP_REQUEST_QUERY_INFORMATION_EX req = TCP_REQUEST_QUERY_INFORMATION_INIT;
    PVOID entitySet = 0;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD allocationSizeForEntityArray = entrySize * MAX_TDI_ENTITIES,
        arraySize = entrySize * MAX_TDI_ENTITIES;

    req.ID.toi_class                = toiClass;
    req.ID.toi_type                 = toiType;
    req.ID.toi_id                   = toiId;
    req.ID.toi_entity.tei_entity    = teiEntity;
    req.ID.toi_entity.tei_instance  = teiInstance;

    /* There's a subtle problem here...
     * If an interface is added at this exact instant, (as if by a PCMCIA
     * card insertion), the array will still not have enough entries after
     * have allocated it after the first DeviceIoControl call.
     *
     * We'll get around this by repeating until the number of interfaces
     * stabilizes.
     */
    do {
        status = DeviceIoControl( tcpFile,
                                  IOCTL_TCP_QUERY_INFORMATION_EX,
                                  &req,
                                  sizeof(req),
                                  0,
                                  0,
                                  &allocationSizeForEntityArray,
                                  NULL );

        if(!status)
        {
            return STATUS_UNSUCCESSFUL;
        }

        arraySize = allocationSizeForEntityArray;
        entitySet = HeapAlloc( GetProcessHeap(), 0, arraySize );

        if( !entitySet ) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            return status;
        }

        status = DeviceIoControl( tcpFile,
                                  IOCTL_TCP_QUERY_INFORMATION_EX,
                                  &req,
                                  sizeof(req),
                                  entitySet,
                                  arraySize,
                                  &allocationSizeForEntityArray,
                                  NULL );

        /* This is why we have the loop -- we might have added an adapter */
        if( arraySize == allocationSizeForEntityArray )
            break;

        HeapFree( GetProcessHeap(), 0, entitySet );
        entitySet = 0;

        if(!status)
            return STATUS_UNSUCCESSFUL;
    } while( TRUE ); /* We break if the array we received was the size we
                      * expected.  Therefore, we got here because it wasn't */

    *numEntries = (arraySize - fixedPart) / entrySize;
    *tdiEntitySet = entitySet;

    return STATUS_SUCCESS;
}

VOID tdiFreeThingSet( PVOID things ) {
    HeapFree( GetProcessHeap(), 0, things );
}

NTSTATUS tdiGetEntityIDSet( HANDLE tcpFile,
                            TDIEntityID **entitySet,
                            PDWORD numEntities ) {
    NTSTATUS status = tdiGetSetOfThings( tcpFile,
                                         INFO_CLASS_GENERIC,
                                         INFO_TYPE_PROVIDER,
                                         ENTITY_LIST_ID,
                                         GENERIC_ENTITY,
					 0,
                                         0,
                                         sizeof(TDIEntityID),
                                         (PVOID *)entitySet,
                                         numEntities );

    return status;
}

