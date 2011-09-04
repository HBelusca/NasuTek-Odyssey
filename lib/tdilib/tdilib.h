/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TDI interface
 * FILE:        tdilib.h
 * PURPOSE:     Shared TDI library header
 */

#pragma once

NTSTATUS openTcpFile(PHANDLE tcpFile);
VOID closeTcpFile(HANDLE tcpFile);
NTSTATUS tdiGetEntityIDSet( HANDLE tcpFile, TDIEntityID **entitySet,
			    PDWORD numEntities );
NTSTATUS tdiGetSetOfThings( HANDLE tcpFile, DWORD toiClass, DWORD toiType,
			    DWORD toiId, DWORD teiEntity, DWORD teiInstance,
			    DWORD fixedPart,
			    DWORD entrySize, PVOID *tdiEntitySet,
			    PDWORD numEntries );
VOID tdiFreeThingSet( PVOID things );
