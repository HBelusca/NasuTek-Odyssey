/*
 *  ITfContext implementation
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
#include "olectl.h"

#include "wine/unicode.h"
#include "wine/list.h"

#include "msctf.h"
#include "msctf_internal.h"

WINE_DEFAULT_DEBUG_CHANNEL(msctf);

typedef struct tagContextSink {
    struct list         entry;
    union {
        /* Context Sinks */
        IUnknown            *pIUnknown;
        /* ITfContextKeyEventSink  *pITfContextKeyEventSink; */
        /* ITfEditTransactionSink  *pITfEditTransactionSink; */
        /* ITfStatusSink           *pITfStatusSink; */
        ITfTextEditSink     *pITfTextEditSink;
        /* ITfTextLayoutSink       *pITfTextLayoutSink; */
    } interfaces;
} ContextSink;

typedef struct tagContext {
    const ITfContextVtbl *ContextVtbl;
    const ITfSourceVtbl *SourceVtbl;
    /* const ITfContextCompositionVtbl *ContextCompositionVtbl; */
    /* const ITfContextOwnerCompositionServicesVtbl *ContextOwnerCompositionServicesVtbl; */
    /* const ITfContextOwnerServicesVtbl *ContextOwnerServicesVtbl; */
    const ITfInsertAtSelectionVtbl *InsertAtSelectionVtbl;
    /* const ITfMouseTrackerVtbl *MouseTrackerVtbl; */
    /* const ITfQueryEmbeddedVtbl *QueryEmbeddedVtbl; */
    const ITfSourceSingleVtbl *SourceSingleVtbl;
    LONG refCount;
    BOOL connected;

    /* Aggregation */
    ITfCompartmentMgr  *CompartmentMgr;

    TfClientId tidOwner;
    TfEditCookie defaultCookie;
    TS_STATUS documentStatus;
    ITfDocumentMgr *manager;

    ITextStoreACP   *pITextStoreACP;
    ITfContextOwnerCompositionSink *pITfContextOwnerCompositionSink;

    ITextStoreACPSink *pITextStoreACPSink;
    ITfEditSession* currentEditSession;

    /* kept as separate lists to reduce unnecessary iterations */
    struct list     pContextKeyEventSink;
    struct list     pEditTransactionSink;
    struct list     pStatusSink;
    struct list     pTextEditSink;
    struct list     pTextLayoutSink;

} Context;

typedef struct tagEditCookie {
    DWORD lockType;
    Context *pOwningContext;
} EditCookie;

typedef struct tagTextStoreACPSink {
    const ITextStoreACPSinkVtbl *TextStoreACPSinkVtbl;
    /* const ITextStoreACPServicesVtbl *TextStoreACPServicesVtbl; */
    LONG refCount;

    Context *pContext;
} TextStoreACPSink;


static HRESULT TextStoreACPSink_Constructor(ITextStoreACPSink **ppOut, Context *pContext);

static inline Context *impl_from_ITfSourceVtbl(ITfSource *iface)
{
    return (Context *)((char *)iface - FIELD_OFFSET(Context,SourceVtbl));
}

static inline Context *impl_from_ITfInsertAtSelectionVtbl(ITfInsertAtSelection*iface)
{
    return (Context *)((char *)iface - FIELD_OFFSET(Context,InsertAtSelectionVtbl));
}

static inline Context *impl_from_ITfSourceSingleVtbl(ITfSourceSingle* iface)
{
    return (Context *)((char *)iface - FIELD_OFFSET(Context,SourceSingleVtbl));
}

static void free_sink(ContextSink *sink)
{
        IUnknown_Release(sink->interfaces.pIUnknown);
        HeapFree(GetProcessHeap(),0,sink);
}

static void Context_Destructor(Context *This)
{
    struct list *cursor, *cursor2;
    EditCookie *cookie;
    TRACE("destroying %p\n", This);

    if (This->pITextStoreACPSink)
    {
        ITextStoreACP_UnadviseSink(This->pITextStoreACP, (IUnknown*)This->pITextStoreACPSink);
        ITextStoreACPSink_Release(This->pITextStoreACPSink);
    }

    if (This->pITextStoreACP)
        ITextStoreACPSink_Release(This->pITextStoreACP);

    if (This->pITfContextOwnerCompositionSink)
        ITextStoreACPSink_Release(This->pITfContextOwnerCompositionSink);

    if (This->defaultCookie)
    {
        cookie = remove_Cookie(This->defaultCookie);
        HeapFree(GetProcessHeap(),0,cookie);
        This->defaultCookie = 0;
    }

    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->pContextKeyEventSink)
    {
        ContextSink* sink = LIST_ENTRY(cursor,ContextSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->pEditTransactionSink)
    {
        ContextSink* sink = LIST_ENTRY(cursor,ContextSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->pStatusSink)
    {
        ContextSink* sink = LIST_ENTRY(cursor,ContextSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->pTextEditSink)
    {
        ContextSink* sink = LIST_ENTRY(cursor,ContextSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->pTextLayoutSink)
    {
        ContextSink* sink = LIST_ENTRY(cursor,ContextSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }

    CompartmentMgr_Destructor(This->CompartmentMgr);
    HeapFree(GetProcessHeap(),0,This);
}

static HRESULT WINAPI Context_QueryInterface(ITfContext *iface, REFIID iid, LPVOID *ppvOut)
{
    Context *This = (Context *)iface;
    *ppvOut = NULL;

    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_ITfContext))
    {
        *ppvOut = This;
    }
    else if (IsEqualIID(iid, &IID_ITfSource))
    {
        *ppvOut = &This->SourceVtbl;
    }
    else if (IsEqualIID(iid, &IID_ITfInsertAtSelection))
    {
        *ppvOut = &This->InsertAtSelectionVtbl;
    }
    else if (IsEqualIID(iid, &IID_ITfCompartmentMgr))
    {
        *ppvOut = This->CompartmentMgr;
    }
    else if (IsEqualIID(iid, &IID_ITfSourceSingle))
    {
        *ppvOut = &This->SourceSingleVtbl;
    }

    if (*ppvOut)
    {
        IUnknown_AddRef(iface);
        return S_OK;
    }

    WARN("unsupported interface: %s\n", debugstr_guid(iid));
    return E_NOINTERFACE;
}

static ULONG WINAPI Context_AddRef(ITfContext *iface)
{
    Context *This = (Context *)iface;
    return InterlockedIncrement(&This->refCount);
}

static ULONG WINAPI Context_Release(ITfContext *iface)
{
    Context *This = (Context *)iface;
    ULONG ret;

    ret = InterlockedDecrement(&This->refCount);
    if (ret == 0)
        Context_Destructor(This);
    return ret;
}

/*****************************************************
 * ITfContext functions
 *****************************************************/
static HRESULT WINAPI Context_RequestEditSession (ITfContext *iface,
        TfClientId tid, ITfEditSession *pes, DWORD dwFlags,
        HRESULT *phrSession)
{
    HRESULT hr;
    Context *This = (Context *)iface;
    DWORD  dwLockFlags = 0x0;

    TRACE("(%p) %i %p %x %p\n",This, tid, pes, dwFlags, phrSession);

    if (!(dwFlags & TF_ES_READ) && !(dwFlags & TF_ES_READWRITE))
    {
        *phrSession = E_FAIL;
        return E_INVALIDARG;
    }

    if (!This->pITextStoreACP)
    {
        FIXME("No ITextStoreACP avaliable\n");
        *phrSession = E_FAIL;
        return E_FAIL;
    }

    if (!(dwFlags & TF_ES_ASYNC))
        dwLockFlags |= TS_LF_SYNC;

    if ((dwFlags & TF_ES_READWRITE) == TF_ES_READWRITE)
        dwLockFlags |= TS_LF_READWRITE;
    else if (dwFlags & TF_ES_READ)
        dwLockFlags |= TS_LF_READ;

    if (!This->documentStatus.dwDynamicFlags)
        ITextStoreACP_GetStatus(This->pITextStoreACP, &This->documentStatus);

    if (((dwFlags & TF_ES_READWRITE) == TF_ES_READWRITE) && (This->documentStatus.dwDynamicFlags & TS_SD_READONLY))
    {
        *phrSession = TS_E_READONLY;
        return S_OK;
    }

    if (FAILED (ITfEditSession_QueryInterface(pes, &IID_ITfEditSession, (LPVOID*)&This->currentEditSession)))
    {
        *phrSession = E_FAIL;
        return E_INVALIDARG;
    }

    hr = ITextStoreACP_RequestLock(This->pITextStoreACP, dwLockFlags, phrSession);

    return hr;
}

static HRESULT WINAPI Context_InWriteSession (ITfContext *iface,
         TfClientId tid,
         BOOL *pfWriteSession)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_GetSelection (ITfContext *iface,
        TfEditCookie ec, ULONG ulIndex, ULONG ulCount,
        TF_SELECTION *pSelection, ULONG *pcFetched)
{
    Context *This = (Context *)iface;
    EditCookie *cookie;
    ULONG count, i;
    ULONG totalFetched = 0;
    HRESULT hr = S_OK;

    if (!pSelection || !pcFetched)
        return E_INVALIDARG;

    *pcFetched = 0;

    if (!This->connected)
        return TF_E_DISCONNECTED;

    if (get_Cookie_magic(ec)!=COOKIE_MAGIC_EDITCOOKIE)
        return TF_E_NOLOCK;

    if (!This->pITextStoreACP)
    {
        FIXME("Context does not have a ITextStoreACP\n");
        return E_NOTIMPL;
    }

    cookie = get_Cookie_data(ec);

    if (ulIndex == TF_DEFAULT_SELECTION)
        count = 1;
    else
        count = ulCount;

    for (i = 0; i < count; i++)
    {
        DWORD fetched;
        TS_SELECTION_ACP acps;

        hr = ITextStoreACP_GetSelection(This->pITextStoreACP, ulIndex + i,
                1, &acps, &fetched);

        if (hr == TS_E_NOLOCK)
            return TF_E_NOLOCK;
        else if (SUCCEEDED(hr))
        {
            pSelection[totalFetched].style.ase = acps.style.ase;
            pSelection[totalFetched].style.fInterimChar = acps.style.fInterimChar;
            Range_Constructor(iface, This->pITextStoreACP, cookie->lockType, acps.acpStart, acps.acpEnd, &pSelection[totalFetched].range);
            totalFetched ++;
        }
        else
            break;
    }

    *pcFetched = totalFetched;

    return hr;
}

static HRESULT WINAPI Context_SetSelection (ITfContext *iface,
        TfEditCookie ec, ULONG ulCount, const TF_SELECTION *pSelection)
{
    TS_SELECTION_ACP *acp;
    Context *This = (Context *)iface;
    INT i;
    HRESULT hr;

    TRACE("(%p) %i %i %p\n",This,ec,ulCount,pSelection);

    if (!This->pITextStoreACP)
    {
        FIXME("Context does not have a ITextStoreACP\n");
        return E_NOTIMPL;
    }

    if (get_Cookie_magic(ec)!=COOKIE_MAGIC_EDITCOOKIE)
        return TF_E_NOLOCK;

    acp = HeapAlloc(GetProcessHeap(), 0, sizeof(TS_SELECTION_ACP) * ulCount);
    if (!acp)
        return E_OUTOFMEMORY;

    for (i = 0; i < ulCount; i++)
        if (FAILED(TF_SELECTION_to_TS_SELECTION_ACP(&pSelection[i], &acp[i])))
        {
            TRACE("Selection Conversion Failed\n");
            HeapFree(GetProcessHeap(), 0 , acp);
            return E_FAIL;
        }

    hr = ITextStoreACP_SetSelection(This->pITextStoreACP, ulCount, acp);

    HeapFree(GetProcessHeap(), 0, acp);

    return hr;
}

static HRESULT WINAPI Context_GetStart (ITfContext *iface,
        TfEditCookie ec, ITfRange **ppStart)
{
    Context *This = (Context *)iface;
    EditCookie *cookie;
    TRACE("(%p) %i %p\n",This,ec,ppStart);

    if (!ppStart)
        return E_INVALIDARG;

    *ppStart = NULL;

    if (!This->connected)
        return TF_E_DISCONNECTED;

    if (get_Cookie_magic(ec)!=COOKIE_MAGIC_EDITCOOKIE)
        return TF_E_NOLOCK;

    cookie = get_Cookie_data(ec);
    return Range_Constructor(iface, This->pITextStoreACP, cookie->lockType, 0, 0, ppStart);
}

static HRESULT WINAPI Context_GetEnd (ITfContext *iface,
        TfEditCookie ec, ITfRange **ppEnd)
{
    Context *This = (Context *)iface;
    EditCookie *cookie;
    LONG end;
    TRACE("(%p) %i %p\n",This,ec,ppEnd);

    if (!ppEnd)
        return E_INVALIDARG;

    *ppEnd = NULL;

    if (!This->connected)
        return TF_E_DISCONNECTED;

    if (get_Cookie_magic(ec)!=COOKIE_MAGIC_EDITCOOKIE)
        return TF_E_NOLOCK;

    if (!This->pITextStoreACP)
    {
        FIXME("Context does not have a ITextStoreACP\n");
        return E_NOTIMPL;
    }

    cookie = get_Cookie_data(ec);
    ITextStoreACP_GetEndACP(This->pITextStoreACP,&end);

    return Range_Constructor(iface, This->pITextStoreACP, cookie->lockType, end, end, ppEnd);
}

static HRESULT WINAPI Context_GetActiveView (ITfContext *iface,
  ITfContextView **ppView)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_EnumViews (ITfContext *iface,
        IEnumTfContextViews **ppEnum)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_GetStatus (ITfContext *iface,
        TF_STATUS *pdcs)
{
    Context *This = (Context *)iface;
    TRACE("(%p) %p\n",This,pdcs);

    if (!This->connected)
        return TF_E_DISCONNECTED;

    if (!pdcs)
        return E_INVALIDARG;

    if (!This->pITextStoreACP)
    {
        FIXME("Context does not have a ITextStoreACP\n");
        return E_NOTIMPL;
    }

    ITextStoreACP_GetStatus(This->pITextStoreACP, &This->documentStatus);

    *pdcs = This->documentStatus;

    return S_OK;
}

static HRESULT WINAPI Context_GetProperty (ITfContext *iface,
        REFGUID guidProp, ITfProperty **ppProp)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_GetAppProperty (ITfContext *iface,
        REFGUID guidProp, ITfReadOnlyProperty **ppProp)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_TrackProperties (ITfContext *iface,
        const GUID **prgProp, ULONG cProp, const GUID **prgAppProp,
        ULONG cAppProp, ITfReadOnlyProperty **ppProperty)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_EnumProperties (ITfContext *iface,
        IEnumTfProperties **ppEnum)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI Context_GetDocumentMgr (ITfContext *iface,
        ITfDocumentMgr **ppDm)
{
    Context *This = (Context *)iface;
    TRACE("(%p) %p\n",This,ppDm);

    if (!ppDm)
        return E_INVALIDARG;

    *ppDm = This->manager;
    if (!This->manager)
        return S_FALSE;

    ITfDocumentMgr_AddRef(This->manager);

    return S_OK;
}

static HRESULT WINAPI Context_CreateRangeBackup (ITfContext *iface,
        TfEditCookie ec, ITfRange *pRange, ITfRangeBackup **ppBackup)
{
    Context *This = (Context *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static const ITfContextVtbl Context_ContextVtbl =
{
    Context_QueryInterface,
    Context_AddRef,
    Context_Release,

    Context_RequestEditSession,
    Context_InWriteSession,
    Context_GetSelection,
    Context_SetSelection,
    Context_GetStart,
    Context_GetEnd,
    Context_GetActiveView,
    Context_EnumViews,
    Context_GetStatus,
    Context_GetProperty,
    Context_GetAppProperty,
    Context_TrackProperties,
    Context_EnumProperties,
    Context_GetDocumentMgr,
    Context_CreateRangeBackup
};

static HRESULT WINAPI Source_QueryInterface(ITfSource *iface, REFIID iid, LPVOID *ppvOut)
{
    Context *This = impl_from_ITfSourceVtbl(iface);
    return Context_QueryInterface((ITfContext *)This, iid, *ppvOut);
}

static ULONG WINAPI Source_AddRef(ITfSource *iface)
{
    Context *This = impl_from_ITfSourceVtbl(iface);
    return Context_AddRef((ITfContext *)This);
}

static ULONG WINAPI Source_Release(ITfSource *iface)
{
    Context *This = impl_from_ITfSourceVtbl(iface);
    return Context_Release((ITfContext *)This);
}

/*****************************************************
 * ITfSource functions
 *****************************************************/
static HRESULT WINAPI ContextSource_AdviseSink(ITfSource *iface,
        REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    ContextSink *es;
    Context *This = impl_from_ITfSourceVtbl(iface);
    TRACE("(%p) %s %p %p\n",This,debugstr_guid(riid),punk,pdwCookie);

    if (!riid || !punk || !pdwCookie)
        return E_INVALIDARG;

    if (IsEqualIID(riid, &IID_ITfTextEditSink))
    {
        es = HeapAlloc(GetProcessHeap(),0,sizeof(ContextSink));
        if (!es)
            return E_OUTOFMEMORY;
        if (FAILED(IUnknown_QueryInterface(punk, riid, (LPVOID *)&es->interfaces.pITfTextEditSink)))
        {
            HeapFree(GetProcessHeap(),0,es);
            return CONNECT_E_CANNOTCONNECT;
        }
        list_add_head(&This->pTextEditSink ,&es->entry);
        *pdwCookie = generate_Cookie(COOKIE_MAGIC_CONTEXTSINK, es);
    }
    else
    {
        FIXME("(%p) Unhandled Sink: %s\n",This,debugstr_guid(riid));
        return E_NOTIMPL;
    }

    TRACE("cookie %x\n",*pdwCookie);
    return S_OK;
}

static HRESULT WINAPI ContextSource_UnadviseSink(ITfSource *iface, DWORD pdwCookie)
{
    ContextSink *sink;
    Context *This = impl_from_ITfSourceVtbl(iface);

    TRACE("(%p) %x\n",This,pdwCookie);

    if (get_Cookie_magic(pdwCookie)!=COOKIE_MAGIC_CONTEXTSINK)
        return E_INVALIDARG;

    sink = (ContextSink*)remove_Cookie(pdwCookie);
    if (!sink)
        return CONNECT_E_NOCONNECTION;

    list_remove(&sink->entry);
    free_sink(sink);

    return S_OK;
}

static const ITfSourceVtbl Context_SourceVtbl =
{
    Source_QueryInterface,
    Source_AddRef,
    Source_Release,

    ContextSource_AdviseSink,
    ContextSource_UnadviseSink,
};

/*****************************************************
 * ITfInsertAtSelection functions
 *****************************************************/
static HRESULT WINAPI InsertAtSelection_QueryInterface(ITfInsertAtSelection *iface, REFIID iid, LPVOID *ppvOut)
{
    Context *This = impl_from_ITfInsertAtSelectionVtbl(iface);
    return Context_QueryInterface((ITfContext *)This, iid, *ppvOut);
}

static ULONG WINAPI InsertAtSelection_AddRef(ITfInsertAtSelection *iface)
{
    Context *This = impl_from_ITfInsertAtSelectionVtbl(iface);
    return Context_AddRef((ITfContext *)This);
}

static ULONG WINAPI InsertAtSelection_Release(ITfInsertAtSelection *iface)
{
    Context *This = impl_from_ITfInsertAtSelectionVtbl(iface);
    return Context_Release((ITfContext *)This);
}

static HRESULT WINAPI InsertAtSelection_InsertTextAtSelection(
        ITfInsertAtSelection *iface, TfEditCookie ec, DWORD dwFlags,
        const WCHAR *pchText, LONG cch, ITfRange **ppRange)
{
    Context *This = impl_from_ITfInsertAtSelectionVtbl(iface);
    EditCookie *cookie;
    LONG acpStart, acpEnd;
    TS_TEXTCHANGE change;
    HRESULT hr;

    TRACE("(%p) %i %x %s %p\n",This, ec, dwFlags, debugstr_wn(pchText,cch), ppRange);

    if (!This->connected)
        return TF_E_DISCONNECTED;

    if (get_Cookie_magic(ec)!=COOKIE_MAGIC_EDITCOOKIE)
        return TF_E_NOLOCK;

    cookie = get_Cookie_data(ec);

    if ((cookie->lockType & TS_LF_READWRITE) != TS_LF_READWRITE )
        return TS_E_READONLY;

    if (!This->pITextStoreACP)
    {
        FIXME("Context does not have a ITextStoreACP\n");
        return E_NOTIMPL;
    }

    hr = ITextStoreACP_InsertTextAtSelection(This->pITextStoreACP, dwFlags, pchText, cch, &acpStart, &acpEnd, &change);
    if (SUCCEEDED(hr))
        Range_Constructor((ITfContext*)This, This->pITextStoreACP, cookie->lockType, change.acpStart, change.acpNewEnd, ppRange);

    return hr;
}

static HRESULT WINAPI InsertAtSelection_InsertEmbeddedAtSelection(
        ITfInsertAtSelection *iface, TfEditCookie ec, DWORD dwFlags,
        IDataObject *pDataObject, ITfRange **ppRange)
{
    Context *This = impl_from_ITfInsertAtSelectionVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static const ITfInsertAtSelectionVtbl Context_InsertAtSelectionVtbl =
{
    InsertAtSelection_QueryInterface,
    InsertAtSelection_AddRef,
    InsertAtSelection_Release,

    InsertAtSelection_InsertTextAtSelection,
    InsertAtSelection_InsertEmbeddedAtSelection,
};

/*****************************************************
 * ITfSourceSingle functions
 *****************************************************/
static HRESULT WINAPI SourceSingle_QueryInterface(ITfSourceSingle *iface, REFIID iid, LPVOID *ppvOut)
{
    Context *This = impl_from_ITfSourceSingleVtbl(iface);
    return Context_QueryInterface((ITfContext *)This, iid, *ppvOut);
}

static ULONG WINAPI SourceSingle_AddRef(ITfSourceSingle *iface)
{
    Context *This = impl_from_ITfSourceSingleVtbl(iface);
    return Context_AddRef((ITfContext *)This);
}

static ULONG WINAPI SourceSingle_Release(ITfSourceSingle *iface)
{
    Context *This = impl_from_ITfSourceSingleVtbl(iface);
    return Context_Release((ITfContext *)This);
}

static HRESULT WINAPI SourceSingle_AdviseSingleSink( ITfSourceSingle *iface,
    TfClientId tid, REFIID riid, IUnknown *punk)
{
    Context *This = impl_from_ITfSourceSingleVtbl(iface);
    FIXME("STUB:(%p) %i %s %p\n",This, tid, debugstr_guid(riid),punk);
    return E_NOTIMPL;
}

static HRESULT WINAPI SourceSingle_UnadviseSingleSink( ITfSourceSingle *iface,
    TfClientId tid, REFIID riid)
{
    Context *This = impl_from_ITfSourceSingleVtbl(iface);
    FIXME("STUB:(%p) %i %s\n",This, tid, debugstr_guid(riid));
    return E_NOTIMPL;
}

static const ITfSourceSingleVtbl Context_SourceSingleVtbl =
{
    SourceSingle_QueryInterface,
    SourceSingle_AddRef,
    SourceSingle_Release,

    SourceSingle_AdviseSingleSink,
    SourceSingle_UnadviseSingleSink,
};

HRESULT Context_Constructor(TfClientId tidOwner, IUnknown *punk, ITfDocumentMgr *mgr, ITfContext **ppOut, TfEditCookie *pecTextStore)
{
    Context *This;
    EditCookie *cookie;

    This = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(Context));
    if (This == NULL)
        return E_OUTOFMEMORY;

    cookie = HeapAlloc(GetProcessHeap(),0,sizeof(EditCookie));
    if (cookie == NULL)
    {
        HeapFree(GetProcessHeap(),0,This);
        return E_OUTOFMEMORY;
    }

    TRACE("(%p) %x %p %p %p\n",This, tidOwner, punk, ppOut, pecTextStore);

    This->ContextVtbl= &Context_ContextVtbl;
    This->SourceVtbl = &Context_SourceVtbl;
    This->InsertAtSelectionVtbl = &Context_InsertAtSelectionVtbl;
    This->SourceSingleVtbl = &Context_SourceSingleVtbl;
    This->refCount = 1;
    This->tidOwner = tidOwner;
    This->connected = FALSE;
    This->manager = mgr;

    CompartmentMgr_Constructor((IUnknown*)This, &IID_IUnknown, (IUnknown**)&This->CompartmentMgr);

    cookie->lockType = TF_ES_READ;
    cookie->pOwningContext = This;

    if (punk)
    {
        IUnknown_QueryInterface(punk, &IID_ITextStoreACP,
                          (LPVOID*)&This->pITextStoreACP);

        IUnknown_QueryInterface(punk, &IID_ITfContextOwnerCompositionSink,
                                (LPVOID*)&This->pITfContextOwnerCompositionSink);

        if (!This->pITextStoreACP && !This->pITfContextOwnerCompositionSink)
            FIXME("Unhandled pUnk\n");
    }

    This->defaultCookie = generate_Cookie(COOKIE_MAGIC_EDITCOOKIE,cookie);
    *pecTextStore = This->defaultCookie;

    list_init(&This->pContextKeyEventSink);
    list_init(&This->pEditTransactionSink);
    list_init(&This->pStatusSink);
    list_init(&This->pTextEditSink);
    list_init(&This->pTextLayoutSink);

    *ppOut = (ITfContext*)This;
    TRACE("returning %p\n", This);

    return S_OK;
}

HRESULT Context_Initialize(ITfContext *iface, ITfDocumentMgr *manager)
{
    Context *This = (Context *)iface;

    if (This->pITextStoreACP)
    {
        if (SUCCEEDED(TextStoreACPSink_Constructor(&This->pITextStoreACPSink, This)))
            ITextStoreACP_AdviseSink(This->pITextStoreACP, &IID_ITextStoreACPSink,
                            (IUnknown*)This->pITextStoreACPSink, TS_AS_ALL_SINKS);
    }
    This->connected = TRUE;
    This->manager = manager;
    return S_OK;
}

HRESULT Context_Uninitialize(ITfContext *iface)
{
    Context *This = (Context *)iface;

    if (This->pITextStoreACPSink)
    {
        ITextStoreACP_UnadviseSink(This->pITextStoreACP, (IUnknown*)This->pITextStoreACPSink);
        if (ITextStoreACPSink_Release(This->pITextStoreACPSink) == 0)
            This->pITextStoreACPSink = NULL;
    }
    This->connected = FALSE;
    This->manager = NULL;
    return S_OK;
}

/**************************************************************************
 *  ITextStoreACPSink
 **************************************************************************/

static void TextStoreACPSink_Destructor(TextStoreACPSink *This)
{
    TRACE("destroying %p\n", This);
    HeapFree(GetProcessHeap(),0,This);
}

static HRESULT WINAPI TextStoreACPSink_QueryInterface(ITextStoreACPSink *iface, REFIID iid, LPVOID *ppvOut)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    *ppvOut = NULL;

    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_ITextStoreACPSink))
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

static ULONG WINAPI TextStoreACPSink_AddRef(ITextStoreACPSink *iface)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    return InterlockedIncrement(&This->refCount);
}

static ULONG WINAPI TextStoreACPSink_Release(ITextStoreACPSink *iface)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    ULONG ret;

    ret = InterlockedDecrement(&This->refCount);
    if (ret == 0)
        TextStoreACPSink_Destructor(This);
    return ret;
}

/*****************************************************
 * ITextStoreACPSink functions
 *****************************************************/

static HRESULT WINAPI TextStoreACPSink_OnTextChange(ITextStoreACPSink *iface,
        DWORD dwFlags, const TS_TEXTCHANGE *pChange)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI TextStoreACPSink_OnSelectionChange(ITextStoreACPSink *iface)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI TextStoreACPSink_OnLayoutChange(ITextStoreACPSink *iface,
    TsLayoutCode lcode, TsViewCookie vcView)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI TextStoreACPSink_OnStatusChange(ITextStoreACPSink *iface,
        DWORD dwFlags)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    HRESULT hr, hrSession;

    TRACE("(%p) %x\n",This, dwFlags);

    if (!This->pContext)
    {
        ERR("No context?\n");
        return E_FAIL;
    }

    if (!This->pContext->pITextStoreACP)
    {
        FIXME("Context does not have a ITextStoreACP\n");
        return E_NOTIMPL;
    }

    hr = ITextStoreACP_RequestLock(This->pContext->pITextStoreACP, TS_LF_READ, &hrSession);

    if(SUCCEEDED(hr) && SUCCEEDED(hrSession))
        This->pContext->documentStatus.dwDynamicFlags = dwFlags;

    return S_OK;
}

static HRESULT WINAPI TextStoreACPSink_OnAttrsChange(ITextStoreACPSink *iface,
        LONG acpStart, LONG acpEnd, ULONG cAttrs, const TS_ATTRID *paAttrs)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI TextStoreACPSink_OnLockGranted(ITextStoreACPSink *iface,
        DWORD dwLockFlags)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    HRESULT hr;
    EditCookie *cookie,*sinkcookie;
    TfEditCookie ec;
    struct list *cursor;

    TRACE("(%p) %x\n",This, dwLockFlags);

    if (!This->pContext)
    {
        ERR("OnLockGranted called without a context\n");
        return E_FAIL;
    }

    if (!This->pContext->currentEditSession)
    {
        FIXME("OnLockGranted called for something other than an EditSession\n");
        return S_OK;
    }

    cookie = HeapAlloc(GetProcessHeap(),0,sizeof(EditCookie));
    if (!cookie)
        return E_OUTOFMEMORY;

    sinkcookie = HeapAlloc(GetProcessHeap(),0,sizeof(EditCookie));
    if (!sinkcookie)
        return E_OUTOFMEMORY;

    cookie->lockType = dwLockFlags;
    cookie->pOwningContext = This->pContext;
    ec = generate_Cookie(COOKIE_MAGIC_EDITCOOKIE, cookie);

    hr = ITfEditSession_DoEditSession(This->pContext->currentEditSession, ec);

    if ((dwLockFlags&TS_LF_READWRITE) == TS_LF_READWRITE)
    {
        TfEditCookie sc;

        sinkcookie->lockType = TS_LF_READ;
        sinkcookie->pOwningContext = This->pContext;
        sc = generate_Cookie(COOKIE_MAGIC_EDITCOOKIE, sinkcookie);

        /*TODO: implement ITfEditRecord */
        LIST_FOR_EACH(cursor, &This->pContext->pTextEditSink)
        {
            ContextSink* sink = LIST_ENTRY(cursor,ContextSink,entry);
            ITfTextEditSink_OnEndEdit(sink->interfaces.pITfTextEditSink,
                                      (ITfContext*) &This->pContext, sc, NULL);
        }
        sinkcookie = remove_Cookie(sc);
    }
    HeapFree(GetProcessHeap(),0,sinkcookie);

    ITfEditSession_Release(This->pContext->currentEditSession);
    This->pContext->currentEditSession = NULL;

    /* Edit Cookie is only valid during the edit session */
    cookie = remove_Cookie(ec);
    HeapFree(GetProcessHeap(),0,cookie);

    return hr;
}

static HRESULT WINAPI TextStoreACPSink_OnStartEditTransaction(ITextStoreACPSink *iface)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI TextStoreACPSink_OnEndEditTransaction(ITextStoreACPSink *iface)
{
    TextStoreACPSink *This = (TextStoreACPSink *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static const ITextStoreACPSinkVtbl TextStoreACPSink_TextStoreACPSinkVtbl =
{
    TextStoreACPSink_QueryInterface,
    TextStoreACPSink_AddRef,
    TextStoreACPSink_Release,

    TextStoreACPSink_OnTextChange,
    TextStoreACPSink_OnSelectionChange,
    TextStoreACPSink_OnLayoutChange,
    TextStoreACPSink_OnStatusChange,
    TextStoreACPSink_OnAttrsChange,
    TextStoreACPSink_OnLockGranted,
    TextStoreACPSink_OnStartEditTransaction,
    TextStoreACPSink_OnEndEditTransaction
};

static HRESULT TextStoreACPSink_Constructor(ITextStoreACPSink **ppOut, Context *pContext)
{
    TextStoreACPSink *This;

    This = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(TextStoreACPSink));
    if (This == NULL)
        return E_OUTOFMEMORY;

    This->TextStoreACPSinkVtbl= &TextStoreACPSink_TextStoreACPSinkVtbl;
    This->refCount = 1;

    This->pContext = pContext;

    TRACE("returning %p\n", This);
    *ppOut = (ITextStoreACPSink*)This;
    return S_OK;
}
