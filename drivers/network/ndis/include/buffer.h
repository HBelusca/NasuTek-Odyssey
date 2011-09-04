/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey NDIS library
 * FILE:        include/buffer.h
 * PURPOSE:     Buffer management routine definitions
 */

#pragma once

#include "ndissys.h"


/* FIXME: Possibly move this to ntddk.h */
typedef struct _NETWORK_HEADER
{
    MDL Mdl;                                /* Memory Descriptor List */
    struct _NETWORK_HEADER *Next;           /* Link to next NDIS buffer in pool */
    struct _NDIS_BUFFER_POOL *BufferPool;   /* Link to NDIS buffer pool */
} NETWORK_HEADER, *PNETWORK_HEADER;

typedef struct _NDIS_BUFFER_POOL
{
    KSPIN_LOCK SpinLock;
    PNETWORK_HEADER FreeList;
    NETWORK_HEADER Buffers[0];
} NDIS_BUFFER_POOL, *PNDIS_BUFFER_POOL;

typedef struct _NDISI_PACKET_POOL {
  NDIS_SPIN_LOCK  SpinLock;
  struct _NDIS_PACKET *FreeList;
  UINT  PacketLength;
  UCHAR  Buffer[1];
} NDISI_PACKET_POOL, * PNDISI_PACKET_POOL;

UINT CopyBufferToBufferChain(
    PNDIS_BUFFER DstBuffer,
    UINT DstOffset,
    PUCHAR SrcData,
    UINT Length);

UINT CopyBufferChainToBuffer(
    PUCHAR DstData,
    PNDIS_BUFFER SrcBuffer,
    UINT SrcOffset,
    UINT Length);

UINT CopyPacketToBuffer(
    PUCHAR DstData,
    PNDIS_PACKET SrcPacket,
    UINT SrcOffset,
    UINT Length);

UINT CopyPacketToBufferChain(
    PNDIS_BUFFER DstBuffer,
    UINT DstOffset,
    PNDIS_PACKET SrcPacket,
    UINT SrcOffset,
    UINT Length);

/* EOF */
