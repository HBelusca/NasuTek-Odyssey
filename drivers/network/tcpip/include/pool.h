/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey TCP/IP protocol driver
 * FILE:        include/pool.h
 * PURPOSE:     Prototypes for memory pooling
 */

#pragma once

NDIS_STATUS PrependPacket( PNDIS_PACKET Packet, PCHAR Data, UINT Len,
			   BOOLEAN Copy );

/* EOF */
