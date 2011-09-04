/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         Odyssey kernel
 * FILE:            lib/rossym/rossympriv.h
 * PURPOSE:         Private header for rossym
 *
 * PROGRAMMERS:     Ge van Geldorp (gvg@odyssey.com)
 */

#pragma once

#define HIGHBIT 0x80000000

extern ROSSYM_CALLBACKS RosSymCallbacks;

#define RosSymAllocMem(Size) (*RosSymCallbacks.AllocMemProc)(Size)
#define RosSymFreeMem(Area) (*RosSymCallbacks.FreeMemProc)(Area)
#define RosSymReadFile(FileContext, Buffer, Size) (*RosSymCallbacks.ReadFileProc)((FileContext), (Buffer), (Size))
#define RosSymSeekFile(FileContext, Position) (*RosSymCallbacks.SeekFileProc)((FileContext), (Position))
#define RosSymGetMem(TargetAddress, Address, Size) (*RosSymCallbacks.MemGetProc)((TargetAddress), (Address), (Size))

extern BOOLEAN RosSymZwReadFile(PVOID FileContext, PVOID Buffer, ULONG Size);
extern BOOLEAN RosSymZwSeekFile(PVOID FileContext, ULONG_PTR Position);

#define ROSSYM_IS_VALID_DOS_HEADER(DosHeader) (IMAGE_DOS_SIGNATURE == (DosHeader)->e_magic \
                                               && 0L != (DosHeader)->e_lfanew)
#define ROSSYM_IS_VALID_NT_HEADERS(NtHeaders) (IMAGE_NT_SIGNATURE == (NtHeaders)->Signature \
                                               && IMAGE_NT_OPTIONAL_HDR_MAGIC == (NtHeaders)->OptionalHeader.Magic)

/* EOF */
