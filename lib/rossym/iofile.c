/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            lib/rossym/zwfile.c
 * PURPOSE:         File I/O using native functions
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@odyssey.com)
 */

#define NTOSAPI
#include <ntddk.h>
#include <odyssey/rossym.h>
#include "rossympriv.h"

#define NDEBUG
#include <debug.h>

NTSTATUS RosSymStatus;

BOOLEAN
RosSymIoReadFile(PVOID FileContext, PVOID Buffer, ULONG Size)
{
    PROSSYM_OWN_FILECONTEXT OwnContext = (PROSSYM_OWN_FILECONTEXT)FileContext;
    return OwnContext->ReadFileProc(FileContext, Buffer, Size);
}

BOOLEAN
RosSymIoSeekFile(PVOID FileContext, ULONG_PTR Position)
{
    PROSSYM_OWN_FILECONTEXT OwnContext = (PROSSYM_OWN_FILECONTEXT)FileContext;
    return OwnContext->SeekFileProc(FileContext, Position);
}

/* EOF */
