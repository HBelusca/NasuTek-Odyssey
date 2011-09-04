/* $Id: csr.h 45685 2010-02-26 11:43:19Z gedmurphy $
 *
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          Odyssey kernel
 * PURPOSE:          Interface to csrss
 * FILE:             subsys/win32k/include/csr.h
 * PROGRAMER:        Ge van Geldorp (ge@gse.nl)
 */

#pragma once

extern PEPROCESS CsrProcess;

NTSTATUS FASTCALL CsrInit(void);
NTSTATUS FASTCALL co_CsrNotify(PCSR_API_MESSAGE Request);
NTSTATUS FASTCALL CsrCloseHandle(HANDLE Handle);
NTSTATUS WINAPI CsrInsertObject(HANDLE ObjectHandle,
                                 ACCESS_MASK DesiredAccess,
                                 PHANDLE Handle);

/* EOF */
