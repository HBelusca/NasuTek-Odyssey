/* $Id: malloc.c 37763 2008-11-30 11:42:05Z sginsberg $
 */
/*
 * COPYRIGHT:   None
 * LICENSE:     Public domain
 * PROJECT:     Odyssey system libraries
 * FILE:        odyssey/lib/psapi/misc/malloc.c
 * PURPOSE:     Memory allocator for PSAPI
 * PROGRAMMER:  KJK::Hyperion <noog@libero.it>
 * UPDATE HISTORY:
 *              10/06/2002: Created
 *              12/02/2003: malloc and free renamed to PsaiMalloc and PsaiFree,
 *                          for better reusability
 */

#include "precomp.h"

#define NDEBUG
#include <debug.h>

PVOID
WINAPI
MemAlloc(IN HANDLE Heap,
         IN PVOID Ptr,
         IN ULONG Size)
{
  PVOID pBuf = NULL;

  if(Size == 0 && Ptr == NULL)
  {
    return NULL;
  }

  if(Heap == NULL)
  {
    Heap = NtCurrentPeb()->ProcessHeap;
  }

  if(Size > 0)
  {
    if(Ptr == NULL)
      /* malloc */
      pBuf = RtlAllocateHeap(Heap, 0, Size);
    else
      /* realloc */
      pBuf = RtlReAllocateHeap(Heap, 0, Ptr, Size);
  }
  else
    /* free */
    RtlFreeHeap(Heap, 0, Ptr);

  return pBuf;
}

void *PsaiMalloc(SIZE_T size)
{
 return MemAlloc(NULL, NULL, size);
}

void *PsaiRealloc(void *ptr, SIZE_T size)
{
 return MemAlloc(NULL, ptr, size);
}

void PsaiFree(void *ptr)
{
 MemAlloc(NULL, ptr, 0);
}

/* EOF */

