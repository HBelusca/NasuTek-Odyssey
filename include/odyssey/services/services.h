/*
 * COPYRIGHT:   See COPYING in the top level directory
 * PROJECT:     Odyssey kernel
 * FILE:        include/services/services.h
 * PURPOSE:     Private interface between SERVICES.EXE and ADVAPI32.DLL
 * PROGRAMMER:  Eric Kohl
 */

#ifndef __SERVICES_SERVICES_H__
#define __SERVICES_SERVICES_H__

#define SERVICE_CONTROL_START 0

typedef struct _SCM_CONTROL_PACKET
{
    DWORD dwSize;
    DWORD dwControl;
    SERVICE_STATUS_HANDLE hServiceStatus;
    DWORD dwServiceNameOffset;
    DWORD dwArgumentsCount;
    DWORD dwArgumentsOffset;
} SCM_CONTROL_PACKET, *PSCM_CONTROL_PACKET;

typedef struct _SCM_REPLY_PACKET
{
    DWORD dwError;
} SCM_REPLY_PACKET, *PSCM_REPLY_PACKET;

#endif /* __SERVICES_SERVICES_H__ */

/* EOF */
