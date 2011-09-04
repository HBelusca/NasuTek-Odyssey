/******************************************************************************
 *
 * File-based ILockBytes implementation
 *
 * Copyright 1999 Thuy Nguyen
 * Copyright 2010 Vincent Povirk for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define COBJMACROS
#define NONAMELESSUNION
#define NONAMELESSSTRUCT

#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "winerror.h"
#include "objbase.h"
#include "ole2.h"

#include "storage32.h"

#include "wine/debug.h"
#include "wine/unicode.h"

WINE_DEFAULT_DEBUG_CHANNEL(storage);

typedef struct FileLockBytesImpl
{
    const ILockBytesVtbl *lpVtbl;
    LONG ref;
    ULARGE_INTEGER filesize;
    HANDLE hfile;
    DWORD flProtect;
    LPWSTR pwcsName;
} FileLockBytesImpl;

static const ILockBytesVtbl FileLockBytesImpl_Vtbl;

/***********************************************************
 * Prototypes for private methods
 */

/* Note that this evaluates a and b multiple times, so don't
 * pass expressions with side effects. */
#define ROUND_UP(a, b) ((((a) + (b) - 1)/(b))*(b))

/****************************************************************************
 *      GetProtectMode
 *
 * This function will return a protection mode flag for a file-mapping object
 * from the open flags of a file.
 */
static DWORD GetProtectMode(DWORD openFlags)
{
    switch(STGM_ACCESS_MODE(openFlags))
    {
    case STGM_WRITE:
    case STGM_READWRITE:
        return PAGE_READWRITE;
    }
    return PAGE_READONLY;
}

/******************************************************************************
 *      FileLockBytesImpl_Construct
 *
 * Initialize a big block object supported by a file.
 */
HRESULT FileLockBytesImpl_Construct(HANDLE hFile, DWORD openFlags, LPCWSTR pwcsName, ILockBytes **pLockBytes)
{
  FileLockBytesImpl *This;
  WCHAR fullpath[MAX_PATH];

  if (hFile == INVALID_HANDLE_VALUE)
    return E_FAIL;

  This = HeapAlloc(GetProcessHeap(), 0, sizeof(FileLockBytesImpl));

  if (!This)
    return E_OUTOFMEMORY;

  This->lpVtbl = &FileLockBytesImpl_Vtbl;
  This->ref = 1;
  This->hfile = hFile;
  This->filesize.u.LowPart = GetFileSize(This->hfile,
					 &This->filesize.u.HighPart);
  This->flProtect = GetProtectMode(openFlags);

  if(pwcsName) {
    if (!GetFullPathNameW(pwcsName, MAX_PATH, fullpath, NULL))
    {
      lstrcpynW(fullpath, pwcsName, MAX_PATH);
    }
    This->pwcsName = HeapAlloc(GetProcessHeap(), 0,
                              (lstrlenW(fullpath)+1)*sizeof(WCHAR));
    if (!This->pwcsName)
    {
       HeapFree(GetProcessHeap(), 0, This);
       return E_OUTOFMEMORY;
    }
    strcpyW(This->pwcsName, fullpath);
  }
  else
    This->pwcsName = NULL;

  TRACE("file len %u\n", This->filesize.u.LowPart);

  *pLockBytes = (ILockBytes*)This;

  return S_OK;
}

/* ILockByte Interfaces */

static HRESULT WINAPI FileLockBytesImpl_QueryInterface(ILockBytes *iface, REFIID riid,
    void **ppvObject)
{
    if (IsEqualIID(riid, &IID_ILockBytes) || IsEqualIID(riid, &IID_ILockBytes))
        *ppvObject = iface;
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    IUnknown_AddRef((IUnknown*)*ppvObject);

    return S_OK;
}

static ULONG WINAPI FileLockBytesImpl_AddRef(ILockBytes *iface)
{
    FileLockBytesImpl* This = (FileLockBytesImpl*)iface;
    return InterlockedIncrement(&This->ref);
}

static ULONG WINAPI FileLockBytesImpl_Release(ILockBytes *iface)
{
    FileLockBytesImpl* This = (FileLockBytesImpl*)iface;
    ULONG ref;

    ref = InterlockedDecrement(&This->ref);

    if (ref == 0)
    {
        CloseHandle(This->hfile);
        HeapFree(GetProcessHeap(), 0, This->pwcsName);
        HeapFree(GetProcessHeap(), 0, This);
    }

    return ref;
}

/******************************************************************************
 * This method is part of the ILockBytes interface.
 *
 * It reads a block of information from the byte array at the specified
 * offset.
 *
 * See the documentation of ILockBytes for more info.
 */
static HRESULT WINAPI FileLockBytesImpl_ReadAt(
      ILockBytes* iface,
      ULARGE_INTEGER ulOffset,  /* [in] */
      void*          pv,        /* [length_is][size_is][out] */
      ULONG          cb,        /* [in] */
      ULONG*         pcbRead)   /* [out] */
{
    FileLockBytesImpl* This = (FileLockBytesImpl*)iface;
    ULONG bytes_left = cb;
    LPBYTE readPtr = pv;
    BOOL ret;
    LARGE_INTEGER offset;
    ULONG cbRead;

    TRACE("(%p)-> %i %p %i %p\n",This, ulOffset.u.LowPart, pv, cb, pcbRead);

    /* verify a sane environment */
    if (!This) return E_FAIL;

    if (pcbRead)
        *pcbRead = 0;

    offset.QuadPart = ulOffset.QuadPart;

    ret = SetFilePointerEx(This->hfile, offset, NULL, FILE_BEGIN);

    if (!ret)
        return STG_E_READFAULT;

    while (bytes_left)
    {
        ret = ReadFile(This->hfile, readPtr, bytes_left, &cbRead, NULL);

        if (!ret || cbRead == 0)
            return STG_E_READFAULT;

        if (pcbRead)
            *pcbRead += cbRead;

        bytes_left -= cbRead;
        readPtr += cbRead;
    }

    TRACE("finished\n");
    return S_OK;
}

/******************************************************************************
 * This method is part of the ILockBytes interface.
 *
 * It writes the specified bytes at the specified offset.
 * position. If the file is too small, it will be resized.
 *
 * See the documentation of ILockBytes for more info.
 */
static HRESULT WINAPI FileLockBytesImpl_WriteAt(
      ILockBytes* iface,
      ULARGE_INTEGER ulOffset,    /* [in] */
      const void*    pv,          /* [size_is][in] */
      ULONG          cb,          /* [in] */
      ULONG*         pcbWritten)  /* [out] */
{
    FileLockBytesImpl* This = (FileLockBytesImpl*)iface;
    ULONG size_needed = ulOffset.u.LowPart + cb;
    ULONG bytes_left = cb;
    const BYTE *writePtr = pv;
    BOOL ret;
    LARGE_INTEGER offset;
    ULONG cbWritten;

    TRACE("(%p)-> %i %p %i %p\n",This, ulOffset.u.LowPart, pv, cb, pcbWritten);

    /* verify a sane environment */
    if (!This) return E_FAIL;

    if (This->flProtect != PAGE_READWRITE)
        return STG_E_ACCESSDENIED;

    if (pcbWritten)
        *pcbWritten = 0;

    if (size_needed > This->filesize.u.LowPart)
    {
        ULARGE_INTEGER newSize;
        newSize.u.HighPart = 0;
        newSize.u.LowPart = size_needed;
        ILockBytes_SetSize(iface, newSize);
    }

    offset.QuadPart = ulOffset.QuadPart;

    ret = SetFilePointerEx(This->hfile, offset, NULL, FILE_BEGIN);

    if (!ret)
        return STG_E_READFAULT;

    while (bytes_left)
    {
        ret = WriteFile(This->hfile, writePtr, bytes_left, &cbWritten, NULL);

        if (!ret)
            return STG_E_READFAULT;

        if (pcbWritten)
            *pcbWritten += cbWritten;

        bytes_left -= cbWritten;
        writePtr += cbWritten;
    }

    TRACE("finished\n");
    return S_OK;
}

static HRESULT WINAPI FileLockBytesImpl_Flush(ILockBytes* iface)
{
    return S_OK;
}

/******************************************************************************
 *      ILockBytes_SetSize
 *
 * Sets the size of the file.
 *
 */
static HRESULT WINAPI FileLockBytesImpl_SetSize(ILockBytes* iface, ULARGE_INTEGER newSize)
{
    FileLockBytesImpl* This = (FileLockBytesImpl*)iface;
    HRESULT hr = S_OK;
    LARGE_INTEGER newpos;

    if (This->filesize.u.LowPart == newSize.u.LowPart)
        return hr;

    TRACE("from %u to %u\n", This->filesize.u.LowPart, newSize.u.LowPart);

    newpos.QuadPart = newSize.QuadPart;
    if (SetFilePointerEx(This->hfile, newpos, NULL, FILE_BEGIN))
    {
        SetEndOfFile(This->hfile);
    }

    This->filesize = newSize;
    return hr;
}

static HRESULT WINAPI FileLockBytesImpl_LockRegion(ILockBytes* iface,
    ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    FIXME("stub\n");
    return E_NOTIMPL;
}

static HRESULT WINAPI FileLockBytesImpl_UnlockRegion(ILockBytes* iface,
    ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    FIXME("stub\n");
    return E_NOTIMPL;
}

static HRESULT WINAPI FileLockBytesImpl_Stat(ILockBytes* iface,
    STATSTG *pstatstg, DWORD grfStatFlag)
{
    FileLockBytesImpl* This = (FileLockBytesImpl*)iface;

    if (!(STATFLAG_NONAME & grfStatFlag) && This->pwcsName)
    {
        pstatstg->pwcsName =
          CoTaskMemAlloc((lstrlenW(This->pwcsName)+1)*sizeof(WCHAR));

        strcpyW(pstatstg->pwcsName, This->pwcsName);
    }
    else
        pstatstg->pwcsName = NULL;

    pstatstg->type = STGTY_LOCKBYTES;
    pstatstg->cbSize = This->filesize;
    /* FIXME: If the implementation is exported, we'll need to set other fields. */

    return S_OK;
}

static const ILockBytesVtbl FileLockBytesImpl_Vtbl = {
    FileLockBytesImpl_QueryInterface,
    FileLockBytesImpl_AddRef,
    FileLockBytesImpl_Release,
    FileLockBytesImpl_ReadAt,
    FileLockBytesImpl_WriteAt,
    FileLockBytesImpl_Flush,
    FileLockBytesImpl_SetSize,
    FileLockBytesImpl_LockRegion,
    FileLockBytesImpl_UnlockRegion,
    FileLockBytesImpl_Stat
};
