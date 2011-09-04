/*
 *  ITfDocumentMgr implementation
 *
 *  Copyright 2009 Aric Stewart, CodeWeavers
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

#include "config.h"

#include <stdarg.h>

#define COBJMACROS

#include "wine/debug.h"
#include "windef.h"
#include "winbase.h"
#include "winreg.h"
#include "winuser.h"
#include "shlwapi.h"
#include "winerror.h"
#include "objbase.h"

#include "wine/unicode.h"

#include "msctf.h"
#include "msctf_internal.h"

WINE_DEFAULT_DEBUG_CHANNEL(msctf);

typedef struct tagDocumentMgr {
    const ITfDocumentMgrVtbl *DocumentMgrVtbl;
    const ITfSourceVtbl *SourceVtbl;
    LONG refCount;

    /* Aggregation */
    ITfCompartmentMgr  *CompartmentMgr;

    ITfContext*  contextStack[2]; /* limit of 2 contexts */
    ITfThreadMgrEventSink* ThreadMgrSink;
} DocumentMgr;

typedef struct tagEnumTfContext {
    const IEnumTfContextsVtbl *Vtbl;
    LONG refCount;

    DWORD   index;
    DocumentMgr *docmgr;
} EnumTfContext;

static HRESULT EnumTfContext_Constructor(DocumentMgr* mgr, IEnumTfContexts **ppOut);

static inline DocumentMgr *impl_from_ITfSourceVtbl(ITfSource *iface)
{
    return (DocumentMgr *)((char *)iface - FIELD_OFFSET(DocumentMgr,SourceVtbl));
}

static void DocumentMgr_Destructor(DocumentMgr *This)
{
    ITfThreadMgr *tm;
    TRACE("destroying %p\n", This);

    TF_GetThreadMgr(&tm);
    ThreadMgr_OnDocumentMgrDestruction(tm, (ITfDocumentMgr*)This);

    if (This->contextStack[0])
        ITfContext_Release(This->contextStack[0]);
    if (This->contextStack[1])
        ITfContext_Release(This->contextStack[1]);
    CompartmentMgr_Destructor(This->CompartmentMgr);
    HeapFree(GetProcessHeap(),0,This);
}

static HRESULT WINAPI DocumentMgr_QueryInterface(ITfDocumentMgr *iface, REFIID iid, LPVOID *ppvOut)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    *ppvOut = NULL;

    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_ITfDocumentMgr))
    {
        *ppvOut = This;
    }
    else if (IsEqualIID(iid, &IID_ITfSource))
    {
        *ppvOut = &This->SourceVtbl;
    }
    else if (IsEqualIID(iid, &IID_ITfCompartmentMgr))
    {
        *ppvOut = This->CompartmentMgr;
    }

    if (*ppvOut)
    {
        IUnknown_AddRef(iface);
        return S_OK;
    }

    WARN("unsupported interface: %s\n", debugstr_guid(iid));
    return E_NOINTERFACE;
}

static ULONG WINAPI DocumentMgr_AddRef(ITfDocumentMgr *iface)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    return InterlockedIncrement(&This->refCount);
}

static ULONG WINAPI DocumentMgr_Release(ITfDocumentMgr *iface)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    ULONG ret;

    ret = InterlockedDecrement(&This->refCount);
    if (ret == 0)
        DocumentMgr_Destructor(This);
    return ret;
}

/*****************************************************
 * ITfDocumentMgr functions
 *****************************************************/
static HRESULT WINAPI DocumentMgr_CreateContext(ITfDocumentMgr *iface,
        TfClientId tidOwner,
        DWORD dwFlags, IUnknown *punk, ITfContext **ppic,
        TfEditCookie *pecTextStore)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    TRACE("(%p) 0x%x 0x%x %p %p %p\n",This,tidOwner,dwFlags,punk,ppic,pecTextStore);
    return Context_Constructor(tidOwner, punk, iface, ppic, pecTextStore);
}

static HRESULT WINAPI DocumentMgr_Push(ITfDocumentMgr *iface, ITfContext *pic)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    ITfContext *check;

    TRACE("(%p) %p\n",This,pic);

    if (This->contextStack[1])  /* FUll */
        return TF_E_STACKFULL;

    if (!pic || FAILED(IUnknown_QueryInterface(pic,&IID_ITfContext,(LPVOID*) &check)))
        return E_INVALIDARG;

    if (This->contextStack[0] == NULL)
        ITfThreadMgrEventSink_OnInitDocumentMgr(This->ThreadMgrSink,iface);

    This->contextStack[1] = This->contextStack[0];
    This->contextStack[0] = check;

    Context_Initialize(check, iface);
    ITfThreadMgrEventSink_OnPushContext(This->ThreadMgrSink,check);

    return S_OK;
}

static HRESULT WINAPI DocumentMgr_Pop(ITfDocumentMgr *iface, DWORD dwFlags)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    TRACE("(%p) 0x%x\n",This,dwFlags);

    if (dwFlags == TF_POPF_ALL)
    {
        if (This->contextStack[0])
        {
            ITfThreadMgrEventSink_OnPopContext(This->ThreadMgrSink,This->contextStack[0]);
            ITfContext_Release(This->contextStack[0]);
            Context_Uninitialize(This->contextStack[0]);
        }
        if (This->contextStack[1])
        {
            ITfThreadMgrEventSink_OnPopContext(This->ThreadMgrSink,This->contextStack[1]);
            ITfContext_Release(This->contextStack[1]);
            Context_Uninitialize(This->contextStack[1]);
        }
        This->contextStack[0] = This->contextStack[1] = NULL;
        ITfThreadMgrEventSink_OnUninitDocumentMgr(This->ThreadMgrSink, iface);
        return S_OK;
    }

    if (dwFlags)
        return E_INVALIDARG;

    if (This->contextStack[1] == NULL) /* Cannot pop last context */
        return E_FAIL;

    ITfThreadMgrEventSink_OnPopContext(This->ThreadMgrSink,This->contextStack[0]);
    ITfContext_Release(This->contextStack[0]);
    Context_Uninitialize(This->contextStack[0]);
    This->contextStack[0] = This->contextStack[1];
    This->contextStack[1] = NULL;

    if (This->contextStack[0] == NULL)
        ITfThreadMgrEventSink_OnUninitDocumentMgr(This->ThreadMgrSink, iface);

    return S_OK;
}

static HRESULT WINAPI DocumentMgr_GetTop(ITfDocumentMgr *iface, ITfContext **ppic)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    TRACE("(%p)\n",This);
    if (!ppic)
        return E_INVALIDARG;

    if (This->contextStack[0])
        ITfContext_AddRef(This->contextStack[0]);

    *ppic = This->contextStack[0];

    return S_OK;
}

static HRESULT WINAPI DocumentMgr_GetBase(ITfDocumentMgr *iface, ITfContext **ppic)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    ITfContext *tgt;

    TRACE("(%p)\n",This);
    if (!ppic)
        return E_INVALIDARG;

    if (This->contextStack[1])
        tgt = This->contextStack[1];
    else
        tgt = This->contextStack[0];

    if (tgt)
        ITfContext_AddRef(tgt);

    *ppic = tgt;

    return S_OK;
}

static HRESULT WINAPI DocumentMgr_EnumContexts(ITfDocumentMgr *iface, IEnumTfContexts **ppEnum)
{
    DocumentMgr *This = (DocumentMgr *)iface;
    TRACE("(%p) %p\n",This,ppEnum);
    return EnumTfContext_Constructor(This, ppEnum);
}

static const ITfDocumentMgrVtbl DocumentMgr_DocumentMgrVtbl =
{
    DocumentMgr_QueryInterface,
    DocumentMgr_AddRef,
    DocumentMgr_Release,

    DocumentMgr_CreateContext,
    DocumentMgr_Push,
    DocumentMgr_Pop,
    DocumentMgr_GetTop,
    DocumentMgr_GetBase,
    DocumentMgr_EnumContexts
};


static HRESULT WINAPI Source_QueryInterface(ITfSource *iface, REFIID iid, LPVOID *ppvOut)
{
    DocumentMgr *This = impl_from_ITfSourceVtbl(iface);
    return DocumentMgr_QueryInterface((ITfDocumentMgr*)This, iid, *ppvOut);
}

static ULONG WINAPI Source_AddRef(ITfSource *iface)
{
    DocumentMgr *This = impl_from_ITfSourceVtbl(iface);
    return DocumentMgr_AddRef((ITfDocumentMgr*)This);
}

static ULONG WINAPI Source_Release(ITfSource *iface)
{
    DocumentMgr *This = impl_from_ITfSourceVtbl(iface);
    return DocumentMgr_Release((ITfDocumentMgr*)This);
}

/*****************************************************
 * ITfSource functions
 *****************************************************/
static HRESULT WINAPI DocumentMgrSource_AdviseSink(ITfSource *iface,
        REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    DocumentMgr *This = impl_from_ITfSourceVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI DocumentMgrSource_UnadviseSink(ITfSource *iface, DWORD pdwCookie)
{
    DocumentMgr *This = impl_from_ITfSourceVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static const ITfSourceVtbl DocumentMgr_SourceVtbl =
{
    Source_QueryInterface,
    Source_AddRef,
    Source_Release,

    DocumentMgrSource_AdviseSink,
    DocumentMgrSource_UnadviseSink,
};

HRESULT DocumentMgr_Constructor(ITfThreadMgrEventSink *ThreadMgrSink, ITfDocumentMgr **ppOut)
{
    DocumentMgr *This;

    This = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(DocumentMgr));
    if (This == NULL)
        return E_OUTOFMEMORY;

    This->DocumentMgrVtbl= &DocumentMgr_DocumentMgrVtbl;
    This->SourceVtbl = &DocumentMgr_SourceVtbl;
    This->refCount = 1;
    This->ThreadMgrSink = ThreadMgrSink;

    CompartmentMgr_Constructor((IUnknown*)This, &IID_IUnknown, (IUnknown**)&This->CompartmentMgr);

    TRACE("returning %p\n", This);
    *ppOut = (ITfDocumentMgr*)This;
    return S_OK;
}

/**************************************************
 * IEnumTfContexts implementaion
 **************************************************/
static void EnumTfContext_Destructor(EnumTfContext *This)
{
    TRACE("destroying %p\n", This);
    HeapFree(GetProcessHeap(),0,This);
}

static HRESULT WINAPI EnumTfContext_QueryInterface(IEnumTfContexts *iface, REFIID iid, LPVOID *ppvOut)
{
    EnumTfContext *This = (EnumTfContext *)iface;
    *ppvOut = NULL;

    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_IEnumTfContexts))
    {
        *ppvOut = This;
    }

    if (*ppvOut)
    {
        IUnknown_AddRef(iface);
        return S_OK;
    }

    WARN("unsupported interface: %s\n", debugstr_guid(iid));
    return E_NOINTERFACE;
}

static ULONG WINAPI EnumTfContext_AddRef(IEnumTfContexts *iface)
{
    EnumTfContext *This = (EnumTfContext*)iface;
    return InterlockedIncrement(&This->refCount);
}

static ULONG WINAPI EnumTfContext_Release(IEnumTfContexts *iface)
{
    EnumTfContext *This = (EnumTfContext *)iface;
    ULONG ret;

    ret = InterlockedDecrement(&This->refCount);
    if (ret == 0)
        EnumTfContext_Destructor(This);
    return ret;
}

static HRESULT WINAPI EnumTfContext_Next(IEnumTfContexts *iface,
    ULONG ulCount, ITfContext **rgContext, ULONG *pcFetched)
{
    EnumTfContext *This = (EnumTfContext *)iface;
    ULONG fetched = 0;

    TRACE("(%p)\n",This);

    if (rgContext == NULL) return E_POINTER;

    while (fetched < ulCount)
    {
        if (This->index > 1)
            break;

        if (!This->docmgr->contextStack[This->index])
            break;

        *rgContext = This->docmgr->contextStack[This->index];
        ITfContext_AddRef(*rgContext);

        ++This->index;
        ++fetched;
        ++rgContext;
    }

    if (pcFetched) *pcFetched = fetched;
    return fetched == ulCount ? S_OK : S_FALSE;
}

static HRESULT WINAPI EnumTfContext_Skip( IEnumTfContexts* iface, ULONG celt)
{
    EnumTfContext *This = (EnumTfContext *)iface;
    TRACE("(%p)\n",This);
    This->index += celt;
    return S_OK;
}

static HRESULT WINAPI EnumTfContext_Reset( IEnumTfContexts* iface)
{
    EnumTfContext *This = (EnumTfContext *)iface;
    TRACE("(%p)\n",This);
    This->index = 0;
    return S_OK;
}

static HRESULT WINAPI EnumTfContext_Clone( IEnumTfContexts *iface,
    IEnumTfContexts **ppenum)
{
    EnumTfContext *This = (EnumTfContext *)iface;
    HRESULT res;

    TRACE("(%p)\n",This);

    if (ppenum == NULL) return E_POINTER;

    res = EnumTfContext_Constructor(This->docmgr, ppenum);
    if (SUCCEEDED(res))
    {
        EnumTfContext *new_This = (EnumTfContext *)*ppenum;
        new_This->index = This->index;
    }
    return res;
}

static const IEnumTfContextsVtbl IEnumTfContexts_Vtbl ={
    EnumTfContext_QueryInterface,
    EnumTfContext_AddRef,
    EnumTfContext_Release,

    EnumTfContext_Clone,
    EnumTfContext_Next,
    EnumTfContext_Reset,
    EnumTfContext_Skip
};

static HRESULT EnumTfContext_Constructor(DocumentMgr *mgr, IEnumTfContexts **ppOut)
{
    EnumTfContext *This;

    This = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(EnumTfContext));
    if (This == NULL)
        return E_OUTOFMEMORY;

    This->Vtbl= &IEnumTfContexts_Vtbl;
    This->refCount = 1;
    This->docmgr = mgr;

    TRACE("returning %p\n", This);
    *ppOut = (IEnumTfContexts*)This;
    return S_OK;
}
