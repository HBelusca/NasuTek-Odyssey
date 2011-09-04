/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            include/odyssey/chew/chew.h
 * PURPOSE:         Common Highlevel Executive Worker
 *
 * PROGRAMMERS:     arty (ayerkes@speakeasy.net)
 */

#ifndef _ODYSSEY_CHEW_H
#define _ODYSSEY_CHEW_H

/**
 * Initialize CHEW, given a device object (since IoAllocateWorkItem relies on
 * it).
 */
VOID ChewInit(PDEVICE_OBJECT DeviceObject);

/**
 * Shutdown CHEW, waits for remaining work items.
 */
VOID ChewShutdown(VOID);

/**
 * Creates and queues a work item.
 */
BOOLEAN ChewCreate(VOID (*Worker)(PVOID), PVOID WorkerContext);

#endif/*_ODYSSEY_CHEW_H*/
