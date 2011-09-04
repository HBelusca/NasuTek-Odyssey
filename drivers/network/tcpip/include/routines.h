/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TCP/IP protocol driver
 * FILE:        include/routines.h
 * PURPOSE:     Common routine prototypes
 */

#pragma once

UINT Random(VOID);

UINT CopyBufferToBufferChain(
    PNDIS_BUFFER DstBuffer,
    UINT DstOffset,
    PCHAR SrcData,
    UINT Length);

UINT CopyBufferChainToBuffer(
    PCHAR DstData,
    PNDIS_BUFFER SrcBuffer,
    UINT SrcOffset,
    UINT Length);

UINT CopyPacketToBuffer(
    PCHAR DstData,
    PNDIS_PACKET SrcPacket,
    UINT SrcOffset,
    UINT Length);

UINT CopyPacketToBufferChain(
    PNDIS_BUFFER DstBuffer,
    UINT DstOffset,
    PNDIS_PACKET SrcPacket,
    UINT SrcOffset,
    UINT Length);

VOID FreeNdisPacketX(
    PNDIS_PACKET Packet,
    PCHAR File,
    UINT Line);

PVOID AdjustPacket(
    PNDIS_PACKET Packet,
    UINT Available,
    UINT Needed);

UINT ResizePacket(
    PNDIS_PACKET Packet,
    UINT Size);

NDIS_STATUS AllocatePacketWithBuffer( PNDIS_PACKET *NdisPacket,
				       PCHAR Data, UINT Len );

VOID FreeNdisPacket( PNDIS_PACKET Packet );

void GetDataPtr( PNDIS_PACKET Packet,
		 UINT Offset,
		 PCHAR *DataOut,
		 PUINT Size );

#if DBG
VOID DisplayIPPacket(
    PIP_PACKET IPPacket);
#define DISPLAY_IP_PACKET(x) DisplayIPPacket(x)
VOID DisplayTCPPacket(
    PIP_PACKET IPPacket);
#define DISPLAY_TCP_PACKET(x) DisplayTCPPacket(x)
#else
#define DISPLAY_IP_PACKET(x)
#define DISPLAY_TCP_PACKET(x)
#endif /* DBG */

/* EOF */
