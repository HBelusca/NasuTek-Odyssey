/*
 *  ITfThreadMgr implementation
 *
 *  Copyright 2008 Aric Stewart, CodeWeavers
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

typedef struct tagThreadMgrSink {
    struct list         entry;
    union {
        /* ThreadMgr Sinks */
        IUnknown            *pIUnknown;
        /* ITfActiveLanguageProfileNotifySink *pITfActiveLanguageProfileNotifySink; */
        /* ITfDisplayAttributeNotifySink *pITfDisplayAttributeNotifySink; */
        /* ITfKeyTraceEventSink *pITfKeyTraceEventSink; */
        /* ITfPreservedKeyNotifySink *pITfPreservedKeyNotifySink; */
        /* ITfThreadFocusSink *pITfThreadFocusSink; */
        ITfThreadMgrEventSink *pITfThreadMgrEventSink;
    } interfaces;
} ThreadMgrSink;

typedef struct tagPreservedKey
{
    struct list     entry;
    GUID            guid;
    TF_PRESERVEDKEY prekey;
    LPWSTR          description;
    TfClientId      tid;
} PreservedKey;

typedef struct tagDocumentMgrs
{
    struct list     entry;
    ITfDocumentMgr  *docmgr;
} DocumentMgrEntry;

typedef struct tagAssociatedWindow
{
    struct list     entry;
    HWND            hwnd;
    ITfDocumentMgr  *docmgr;
} AssociatedWindow;

typedef struct tagACLMulti {
    const ITfThreadMgrVtbl *ThreadMgrVtbl;
    const ITfSourceVtbl *SourceVtbl;
    const ITfKeystrokeMgrVtbl *KeystrokeMgrVtbl;
    const ITfMessagePumpVtbl *MessagePumpVtbl;
    const ITfClientIdVtbl *ClientIdVtbl;
    /* const ITfThreadMgrExVtbl *ThreadMgrExVtbl; */
    /* const ITfConfigureSystemKeystrokeFeedVtbl *ConfigureSystemKeystrokeFeedVtbl; */
    /* const ITfLangBarItemMgrVtbl *LangBarItemMgrVtbl; */
    /* const ITfUIElementMgrVtbl *UIElementMgrVtbl; */
    const ITfSourceSingleVtbl *SourceSingleVtbl;
    LONG refCount;

    /* Aggregation */
    ITfCompartmentMgr  *CompartmentMgr;

    const ITfThreadMgrEventSinkVtbl *ThreadMgrEventSinkVtbl; /* internal */

    ITfDocumentMgr *focus;
    LONG activationCount;

    ITfKeyEventSink *forgroundKeyEventSink;
    CLSID forgroundTextService;

    struct list CurrentPreservedKeys;
    struct list CreatedDocumentMgrs;

    struct list AssociatedFocusWindows;
    HHOOK  focusHook;

    /* kept as separate lists to reduce unnecessary iterations */
    struct list     ActiveLanguageProfileNotifySink;
    struct list     DisplayAttributeNotifySink;
    struct list     KeyTraceEventSink;
    struct list     PreservedKeyNotifySink;
    struct list     ThreadFocusSink;
    struct list     ThreadMgrEventSink;
} ThreadMgr;

typedef struct tagEnumTfDocumentMgr {
    const IEnumTfDocumentMgrsVtbl *Vtbl;
    LONG refCount;

    struct list *index;
    struct list *head;
} EnumTfDocumentMgr;

static HRESULT EnumTfDocumentMgr_Constructor(struct list* head, IEnumTfDocumentMgrs **ppOut);

static inline ThreadMgr *impl_from_ITfSourceVtbl(ITfSource *iface)
{
    return (ThreadMgr *)((char *)iface - FIELD_OFFSET(ThreadMgr,SourceVtbl));
}

static inline ThreadMgr *impl_from_ITfKeystrokeMgrVtbl(ITfKeystrokeMgr *iface)
{
    return (ThreadMgr *)((char *)iface - FIELD_OFFSET(ThreadMgr,KeystrokeMgrVtbl));
}

static inline ThreadMgr *impl_from_ITfMessagePumpVtbl(ITfMessagePump *iface)
{
    return (ThreadMgr *)((char *)iface - FIELD_OFFSET(ThreadMgr,MessagePumpVtbl));
}

static inline ThreadMgr *impl_from_ITfClientIdVtbl(ITfClientId *iface)
{
    return (ThreadMgr *)((char *)iface - FIELD_OFFSET(ThreadMgr,ClientIdVtbl));
}

static inline ThreadMgr *impl_from_ITfThreadMgrEventSink(ITfThreadMgrEventSink *iface)
{
    return (ThreadMgr *)((char *)iface - FIELD_OFFSET(ThreadMgr,ThreadMgrEventSinkVtbl));
}

static inline ThreadMgr *impl_from_ITfSourceSingleVtbl(ITfSourceSingle* iface)

{
    return (ThreadMgr *)((char *)iface - FIELD_OFFSET(ThreadMgr,SourceSingleVtbl));
}

static void free_sink(ThreadMgrSink *sink)
{
        IUnknown_Release(sink->interfaces.pIUnknown);
        HeapFree(GetProcessHeap(),0,sink);
}

static void ThreadMgr_Destructor(ThreadMgr *This)
{
    struct list *cursor, *cursor2;

    /* unhook right away */
    if (This->focusHook)
        UnhookWindowsHookEx(This->focusHook);

    TlsSetValue(tlsIndex,NULL);
    TRACE("destroying %p\n", This);
    if (This->focus)
        ITfDocumentMgr_Release(This->focus);

    /* free sinks */
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->ActiveLanguageProfileNotifySink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->DisplayAttributeNotifySink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->KeyTraceEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->PreservedKeyNotifySink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->ThreadFocusSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }
    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->ThreadMgrEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        list_remove(cursor);
        free_sink(sink);
    }

    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->CurrentPreservedKeys)
    {
        PreservedKey* key = LIST_ENTRY(cursor,PreservedKey,entry);
        list_remove(cursor);
        HeapFree(GetProcessHeap(),0,key->description);
        HeapFree(GetProcessHeap(),0,key);
    }

    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->CreatedDocumentMgrs)
    {
        DocumentMgrEntry *mgr = LIST_ENTRY(cursor,DocumentMgrEntry,entry);
        list_remove(cursor);
        FIXME("Left Over ITfDocumentMgr.  Should we do something with it?\n");
        HeapFree(GetProcessHeap(),0,mgr);
    }

    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->AssociatedFocusWindows)
    {
        AssociatedWindow *wnd = LIST_ENTRY(cursor,AssociatedWindow,entry);
        list_remove(cursor);
        HeapFree(GetProcessHeap(),0,wnd);
    }

    CompartmentMgr_Destructor(This->CompartmentMgr);

    HeapFree(GetProcessHeap(),0,This);
}

static HRESULT WINAPI ThreadMgr_QueryInterface(ITfThreadMgr *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    *ppvOut = NULL;

    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_ITfThreadMgr))
    {
        *ppvOut = This;
    }
    else if (IsEqualIID(iid, &IID_ITfSource))
    {
        *ppvOut = &This->SourceVtbl;
    }
    else if (IsEqualIID(iid, &IID_ITfKeystrokeMgr))
    {
        *ppvOut = &This->KeystrokeMgrVtbl;
    }
    else if (IsEqualIID(iid, &IID_ITfMessagePump))
    {
        *ppvOut = &This->MessagePumpVtbl;
    }
    else if (IsEqualIID(iid, &IID_ITfClientId))
    {
        *ppvOut = &This->ClientIdVtbl;
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

static ULONG WINAPI ThreadMgr_AddRef(ITfThreadMgr *iface)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    return InterlockedIncrement(&This->refCount);
}

static ULONG WINAPI ThreadMgr_Release(ITfThreadMgr *iface)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    ULONG ret;

    ret = InterlockedDecrement(&This->refCount);
    if (ret == 0)
        ThreadMgr_Destructor(This);
    return ret;
}

/*****************************************************
 * ITfThreadMgr functions
 *****************************************************/

static HRESULT WINAPI ThreadMgr_fnActivate( ITfThreadMgr* iface, TfClientId *ptid)
{
    ThreadMgr *This = (ThreadMgr *)iface;

    TRACE("(%p) %p\n",This, ptid);

    if (!ptid)
        return E_INVALIDARG;

    if (!processId)
    {
        GUID guid;
        CoCreateGuid(&guid);
        ITfClientId_GetClientId((ITfClientId*)&This->ClientIdVtbl,&guid,&processId);
    }

    activate_textservices(iface);
    This->activationCount++;
    *ptid = processId;
    return S_OK;
}

static HRESULT WINAPI ThreadMgr_fnDeactivate( ITfThreadMgr* iface)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    TRACE("(%p)\n",This);

    if (This->activationCount == 0)
        return E_UNEXPECTED;

    This->activationCount --;

    if (This->activationCount == 0)
    {
        if (This->focus)
        {
            ITfThreadMgrEventSink_OnSetFocus((ITfThreadMgrEventSink*)&This->ThreadMgrEventSinkVtbl, 0, This->focus);
            ITfDocumentMgr_Release(This->focus);
            This->focus = 0;
        }
    }

    deactivate_textservices();

    return S_OK;
}

static HRESULT WINAPI ThreadMgr_CreateDocumentMgr( ITfThreadMgr* iface, ITfDocumentMgr
**ppdim)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    DocumentMgrEntry *mgrentry;
    HRESULT hr;

    TRACE("(%p)\n",iface);
    mgrentry = HeapAlloc(GetProcessHeap(),0,sizeof(DocumentMgrEntry));
    if (mgrentry == NULL)
        return E_OUTOFMEMORY;

    hr = DocumentMgr_Constructor((ITfThreadMgrEventSink*)&This->ThreadMgrEventSinkVtbl, ppdim);

    if (SUCCEEDED(hr))
    {
        mgrentry->docmgr = *ppdim;
        list_add_head(&This->CreatedDocumentMgrs,&mgrentry->entry);
    }
    else
        HeapFree(GetProcessHeap(),0,mgrentry);

    return hr;
}

static HRESULT WINAPI ThreadMgr_EnumDocumentMgrs( ITfThreadMgr* iface, IEnumTfDocumentMgrs
**ppEnum)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    TRACE("(%p) %p\n",This,ppEnum);

    if (!ppEnum)
        return E_INVALIDARG;

    return EnumTfDocumentMgr_Constructor(&This->CreatedDocumentMgrs, ppEnum);
}

static HRESULT WINAPI ThreadMgr_GetFocus( ITfThreadMgr* iface, ITfDocumentMgr
**ppdimFocus)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    TRACE("(%p)\n",This);

    if (!ppdimFocus)
        return E_INVALIDARG;

    *ppdimFocus = This->focus;

    TRACE("->%p\n",This->focus);

    if (This->focus == NULL)
        return S_FALSE;

    ITfDocumentMgr_AddRef(This->focus);

    return S_OK;
}

static HRESULT WINAPI ThreadMgr_SetFocus( ITfThreadMgr* iface, ITfDocumentMgr *pdimFocus)
{
    ITfDocumentMgr *check;
    ThreadMgr *This = (ThreadMgr *)iface;

    TRACE("(%p) %p\n",This,pdimFocus);

    if (!pdimFocus)
        check = NULL;
    else if (FAILED(IUnknown_QueryInterface(pdimFocus,&IID_ITfDocumentMgr,(LPVOID*) &check)))
        return E_INVALIDARG;

    ITfThreadMgrEventSink_OnSetFocus((ITfThreadMgrEventSink*)&This->ThreadMgrEventSinkVtbl, check, This->focus);

    if (This->focus)
        ITfDocumentMgr_Release(This->focus);

    This->focus = check;
    return S_OK;
}

static LRESULT CALLBACK ThreadFocusHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    ThreadMgr *This;

    This = TlsGetValue(tlsIndex);
    if (!This)
    {
        ERR("Hook proc but no ThreadMgr for this thread. Serious Error\n");
        return 0;
    }
    if (!This->focusHook)
    {
        ERR("Hook proc but no ThreadMgr focus Hook. Serious Error\n");
        return 0;
    }

    if (nCode == HCBT_SETFOCUS) /* focus change within our thread */
    {
        struct list *cursor;

        LIST_FOR_EACH(cursor, &This->AssociatedFocusWindows)
        {
            AssociatedWindow *wnd = LIST_ENTRY(cursor,AssociatedWindow,entry);
            if (wnd->hwnd == (HWND)wParam)
            {
                TRACE("Triggering Associated window focus\n");
                if (This->focus != wnd->docmgr)
                    ThreadMgr_SetFocus((ITfThreadMgr*)This, wnd->docmgr);
                break;
            }
        }
    }

    return CallNextHookEx(This->focusHook, nCode, wParam, lParam);
}

static HRESULT SetupWindowsHook(ThreadMgr *This)
{
    if (!This->focusHook)
    {
        This->focusHook = SetWindowsHookExW(WH_CBT, ThreadFocusHookProc, 0,
                             GetCurrentThreadId());
        if (!This->focusHook)
        {
            ERR("Unable to set focus hook\n");
            return E_FAIL;
        }
        return S_OK;
    }
    return S_FALSE;
}

static HRESULT WINAPI ThreadMgr_AssociateFocus( ITfThreadMgr* iface, HWND hwnd,
ITfDocumentMgr *pdimNew, ITfDocumentMgr **ppdimPrev)
{
    struct list *cursor, *cursor2;
    ThreadMgr *This = (ThreadMgr *)iface;
    AssociatedWindow *wnd;

    TRACE("(%p) %p %p %p\n",This,hwnd,pdimNew,ppdimPrev);

    if (!ppdimPrev)
        return E_INVALIDARG;

    *ppdimPrev = NULL;

    LIST_FOR_EACH_SAFE(cursor, cursor2, &This->AssociatedFocusWindows)
    {
        wnd = LIST_ENTRY(cursor,AssociatedWindow,entry);
        if (wnd->hwnd == hwnd)
        {
            if (wnd->docmgr)
                ITfDocumentMgr_AddRef(wnd->docmgr);
            *ppdimPrev = wnd->docmgr;
            wnd->docmgr = pdimNew;
            if (GetFocus() == hwnd)
                ThreadMgr_SetFocus(iface,pdimNew);
            return S_OK;
        }
    }

    wnd = HeapAlloc(GetProcessHeap(),0,sizeof(AssociatedWindow));
    wnd->hwnd = hwnd;
    wnd->docmgr = pdimNew;
    list_add_head(&This->AssociatedFocusWindows,&wnd->entry);

    if (GetFocus() == hwnd)
        ThreadMgr_SetFocus(iface,pdimNew);

    SetupWindowsHook(This);

    return S_OK;
}

static HRESULT WINAPI ThreadMgr_IsThreadFocus( ITfThreadMgr* iface, BOOL *pfThreadFocus)
{
    HWND focus;
    ThreadMgr *This = (ThreadMgr *)iface;
    TRACE("(%p) %p\n",This,pfThreadFocus);
    focus = GetFocus();
    *pfThreadFocus = (focus == NULL);
    return S_OK;
}

static HRESULT WINAPI ThreadMgr_GetFunctionProvider( ITfThreadMgr* iface, REFCLSID clsid,
ITfFunctionProvider **ppFuncProv)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI ThreadMgr_EnumFunctionProviders( ITfThreadMgr* iface,
IEnumTfFunctionProviders **ppEnum)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI ThreadMgr_GetGlobalCompartment( ITfThreadMgr* iface,
ITfCompartmentMgr **ppCompMgr)
{
    ThreadMgr *This = (ThreadMgr *)iface;
    HRESULT hr;
    TRACE("(%p) %p\n",This, ppCompMgr);

    if (!ppCompMgr)
        return E_INVALIDARG;

    if (!globalCompartmentMgr)
    {
        hr = CompartmentMgr_Constructor(NULL,&IID_ITfCompartmentMgr,(IUnknown**)&globalCompartmentMgr);
        if (FAILED(hr))
            return hr;
    }

    ITfCompartmentMgr_AddRef(globalCompartmentMgr);
    *ppCompMgr = globalCompartmentMgr;
    return S_OK;
}

static const ITfThreadMgrVtbl ThreadMgr_ThreadMgrVtbl =
{
    ThreadMgr_QueryInterface,
    ThreadMgr_AddRef,
    ThreadMgr_Release,

    ThreadMgr_fnActivate,
    ThreadMgr_fnDeactivate,
    ThreadMgr_CreateDocumentMgr,
    ThreadMgr_EnumDocumentMgrs,
    ThreadMgr_GetFocus,
    ThreadMgr_SetFocus,
    ThreadMgr_AssociateFocus,
    ThreadMgr_IsThreadFocus,
    ThreadMgr_GetFunctionProvider,
    ThreadMgr_EnumFunctionProviders,
    ThreadMgr_GetGlobalCompartment
};


static HRESULT WINAPI Source_QueryInterface(ITfSource *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = impl_from_ITfSourceVtbl(iface);
    return ThreadMgr_QueryInterface((ITfThreadMgr *)This, iid, *ppvOut);
}

static ULONG WINAPI Source_AddRef(ITfSource *iface)
{
    ThreadMgr *This = impl_from_ITfSourceVtbl(iface);
    return ThreadMgr_AddRef((ITfThreadMgr*)This);
}

static ULONG WINAPI Source_Release(ITfSource *iface)
{
    ThreadMgr *This = impl_from_ITfSourceVtbl(iface);
    return ThreadMgr_Release((ITfThreadMgr *)This);
}

/*****************************************************
 * ITfSource functions
 *****************************************************/
static HRESULT WINAPI ThreadMgrSource_AdviseSink(ITfSource *iface,
        REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    ThreadMgrSink *tms;
    ThreadMgr *This = impl_from_ITfSourceVtbl(iface);

    TRACE("(%p) %s %p %p\n",This,debugstr_guid(riid),punk,pdwCookie);

    if (!riid || !punk || !pdwCookie)
        return E_INVALIDARG;

    if (IsEqualIID(riid, &IID_ITfThreadMgrEventSink))
    {
        tms = HeapAlloc(GetProcessHeap(),0,sizeof(ThreadMgrSink));
        if (!tms)
            return E_OUTOFMEMORY;
        if (FAILED(IUnknown_QueryInterface(punk, riid, (LPVOID *)&tms->interfaces.pITfThreadMgrEventSink)))
        {
            HeapFree(GetProcessHeap(),0,tms);
            return CONNECT_E_CANNOTCONNECT;
        }
        list_add_head(&This->ThreadMgrEventSink,&tms->entry);
        *pdwCookie = generate_Cookie(COOKIE_MAGIC_TMSINK, tms);
    }
    else
    {
        FIXME("(%p) Unhandled Sink: %s\n",This,debugstr_guid(riid));
        return E_NOTIMPL;
    }

    TRACE("cookie %x\n",*pdwCookie);

    return S_OK;
}

static HRESULT WINAPI ThreadMgrSource_UnadviseSink(ITfSource *iface, DWORD pdwCookie)
{
    ThreadMgrSink *sink;
    ThreadMgr *This = impl_from_ITfSourceVtbl(iface);

    TRACE("(%p) %x\n",This,pdwCookie);

    if (get_Cookie_magic(pdwCookie)!=COOKIE_MAGIC_TMSINK)
        return E_INVALIDARG;

    sink = (ThreadMgrSink*)remove_Cookie(pdwCookie);
    if (!sink)
        return CONNECT_E_NOCONNECTION;

    list_remove(&sink->entry);
    free_sink(sink);

    return S_OK;
}

static const ITfSourceVtbl ThreadMgr_SourceVtbl =
{
    Source_QueryInterface,
    Source_AddRef,
    Source_Release,

    ThreadMgrSource_AdviseSink,
    ThreadMgrSource_UnadviseSink,
};

/*****************************************************
 * ITfKeystrokeMgr functions
 *****************************************************/

static HRESULT WINAPI KeystrokeMgr_QueryInterface(ITfKeystrokeMgr *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    return ThreadMgr_QueryInterface((ITfThreadMgr *)This, iid, *ppvOut);
}

static ULONG WINAPI KeystrokeMgr_AddRef(ITfKeystrokeMgr *iface)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    return ThreadMgr_AddRef((ITfThreadMgr*)This);
}

static ULONG WINAPI KeystrokeMgr_Release(ITfKeystrokeMgr *iface)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    return ThreadMgr_Release((ITfThreadMgr *)This);
}

static HRESULT WINAPI KeystrokeMgr_AdviseKeyEventSink(ITfKeystrokeMgr *iface,
        TfClientId tid, ITfKeyEventSink *pSink, BOOL fForeground)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    CLSID textservice;
    ITfKeyEventSink *check = NULL;

    TRACE("(%p) %x %p %i\n",This,tid,pSink,fForeground);

    if (!tid || !pSink)
        return E_INVALIDARG;

    textservice = get_textservice_clsid(tid);
    if (IsEqualCLSID(&GUID_NULL,&textservice))
        return E_INVALIDARG;

    get_textservice_sink(tid, &IID_ITfKeyEventSink, (IUnknown**)&check);
    if (check != NULL)
        return CONNECT_E_ADVISELIMIT;

    if (FAILED(IUnknown_QueryInterface(pSink,&IID_ITfKeyEventSink,(LPVOID*) &check)))
        return E_INVALIDARG;

    set_textservice_sink(tid, &IID_ITfKeyEventSink, (IUnknown*)check);

    if (fForeground)
    {
        if (This->forgroundKeyEventSink)
        {
            ITfKeyEventSink_OnSetFocus(This->forgroundKeyEventSink, FALSE);
            ITfKeyEventSink_Release(This->forgroundKeyEventSink);
        }
        ITfKeyEventSink_AddRef(check);
        ITfKeyEventSink_OnSetFocus(check, TRUE);
        This->forgroundKeyEventSink = check;
        This->forgroundTextService = textservice;
    }
    return S_OK;
}

static HRESULT WINAPI KeystrokeMgr_UnadviseKeyEventSink(ITfKeystrokeMgr *iface,
        TfClientId tid)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    CLSID textservice;
    ITfKeyEventSink *check = NULL;
    TRACE("(%p) %x\n",This,tid);

    if (!tid)
        return E_INVALIDARG;

    textservice = get_textservice_clsid(tid);
    if (IsEqualCLSID(&GUID_NULL,&textservice))
        return E_INVALIDARG;

    get_textservice_sink(tid, &IID_ITfKeyEventSink, (IUnknown**)&check);

    if (!check)
        return CONNECT_E_NOCONNECTION;

    set_textservice_sink(tid, &IID_ITfKeyEventSink, NULL);
    ITfKeyEventSink_Release(check);

    if (This->forgroundKeyEventSink == check)
    {
        ITfKeyEventSink_Release(This->forgroundKeyEventSink);
        This->forgroundKeyEventSink = NULL;
        This->forgroundTextService = GUID_NULL;
    }
    return S_OK;
}

static HRESULT WINAPI KeystrokeMgr_GetForeground(ITfKeystrokeMgr *iface,
        CLSID *pclsid)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    TRACE("(%p) %p\n",This,pclsid);
    if (!pclsid)
        return E_INVALIDARG;

    if (IsEqualCLSID(&This->forgroundTextService,&GUID_NULL))
        return S_FALSE;

    *pclsid = This->forgroundTextService;
    return S_OK;
}

static HRESULT WINAPI KeystrokeMgr_TestKeyDown(ITfKeystrokeMgr *iface,
        WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_TestKeyUp(ITfKeystrokeMgr *iface,
        WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_KeyDown(ITfKeystrokeMgr *iface,
        WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_KeyUp(ITfKeystrokeMgr *iface,
        WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_GetPreservedKey(ITfKeystrokeMgr *iface,
        ITfContext *pic, const TF_PRESERVEDKEY *pprekey, GUID *pguid)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_IsPreservedKey(ITfKeystrokeMgr *iface,
        REFGUID rguid, const TF_PRESERVEDKEY *pprekey, BOOL *pfRegistered)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    struct list *cursor;

    TRACE("(%p) %s (%x %x) %p\n",This,debugstr_guid(rguid), (pprekey)?pprekey->uVKey:0, (pprekey)?pprekey->uModifiers:0, pfRegistered);

    if (!rguid || !pprekey || !pfRegistered)
        return E_INVALIDARG;

    LIST_FOR_EACH(cursor, &This->CurrentPreservedKeys)
    {
        PreservedKey* key = LIST_ENTRY(cursor,PreservedKey,entry);
        if (IsEqualGUID(rguid,&key->guid) && pprekey->uVKey == key->prekey.uVKey && pprekey->uModifiers == key->prekey.uModifiers)
        {
            *pfRegistered = TRUE;
            return S_OK;
        }
    }

    *pfRegistered = FALSE;
    return S_FALSE;
}

static HRESULT WINAPI KeystrokeMgr_PreserveKey(ITfKeystrokeMgr *iface,
        TfClientId tid, REFGUID rguid, const TF_PRESERVEDKEY *prekey,
        const WCHAR *pchDesc, ULONG cchDesc)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    struct list *cursor;
    PreservedKey *newkey;

    TRACE("(%p) %x %s (%x,%x) %s\n",This,tid, debugstr_guid(rguid),(prekey)?prekey->uVKey:0,(prekey)?prekey->uModifiers:0,debugstr_wn(pchDesc,cchDesc));

    if (!tid || ! rguid || !prekey || (cchDesc && !pchDesc))
        return E_INVALIDARG;

    LIST_FOR_EACH(cursor, &This->CurrentPreservedKeys)
    {
        PreservedKey* key = LIST_ENTRY(cursor,PreservedKey,entry);
        if (IsEqualGUID(rguid,&key->guid) && prekey->uVKey == key->prekey.uVKey && prekey->uModifiers == key->prekey.uModifiers)
            return TF_E_ALREADY_EXISTS;
    }

    newkey = HeapAlloc(GetProcessHeap(),0,sizeof(PreservedKey));
    if (!newkey)
        return E_OUTOFMEMORY;

    newkey->guid  = *rguid;
    newkey->prekey = *prekey;
    newkey->tid = tid;
    newkey->description = NULL;
    if (cchDesc)
    {
        newkey->description = HeapAlloc(GetProcessHeap(),0,cchDesc * sizeof(WCHAR));
        if (!newkey->description)
        {
            HeapFree(GetProcessHeap(),0,newkey);
            return E_OUTOFMEMORY;
        }
        memcpy(newkey->description, pchDesc, cchDesc*sizeof(WCHAR));
    }

    list_add_head(&This->CurrentPreservedKeys,&newkey->entry);

    return S_OK;
}

static HRESULT WINAPI KeystrokeMgr_UnpreserveKey(ITfKeystrokeMgr *iface,
        REFGUID rguid, const TF_PRESERVEDKEY *pprekey)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    PreservedKey* key = NULL;
    struct list *cursor;
    TRACE("(%p) %s (%x %x)\n",This,debugstr_guid(rguid),(pprekey)?pprekey->uVKey:0, (pprekey)?pprekey->uModifiers:0);

    if (!pprekey || !rguid)
        return E_INVALIDARG;

    LIST_FOR_EACH(cursor, &This->CurrentPreservedKeys)
    {
        key = LIST_ENTRY(cursor,PreservedKey,entry);
        if (IsEqualGUID(rguid,&key->guid) && pprekey->uVKey == key->prekey.uVKey && pprekey->uModifiers == key->prekey.uModifiers)
            break;
        key = NULL;
    }

    if (!key)
        return CONNECT_E_NOCONNECTION;

    list_remove(&key->entry);
    HeapFree(GetProcessHeap(),0,key->description);
    HeapFree(GetProcessHeap(),0,key);

    return S_OK;
}

static HRESULT WINAPI KeystrokeMgr_SetPreservedKeyDescription(ITfKeystrokeMgr *iface,
        REFGUID rguid, const WCHAR *pchDesc, ULONG cchDesc)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_GetPreservedKeyDescription(ITfKeystrokeMgr *iface,
        REFGUID rguid, BSTR *pbstrDesc)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static HRESULT WINAPI KeystrokeMgr_SimulatePreservedKey(ITfKeystrokeMgr *iface,
        ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
    ThreadMgr *This = impl_from_ITfKeystrokeMgrVtbl(iface);
    FIXME("STUB:(%p)\n",This);
    return E_NOTIMPL;
}

static const ITfKeystrokeMgrVtbl ThreadMgr_KeystrokeMgrVtbl =
{
    KeystrokeMgr_QueryInterface,
    KeystrokeMgr_AddRef,
    KeystrokeMgr_Release,

    KeystrokeMgr_AdviseKeyEventSink,
    KeystrokeMgr_UnadviseKeyEventSink,
    KeystrokeMgr_GetForeground,
    KeystrokeMgr_TestKeyDown,
    KeystrokeMgr_TestKeyUp,
    KeystrokeMgr_KeyDown,
    KeystrokeMgr_KeyUp,
    KeystrokeMgr_GetPreservedKey,
    KeystrokeMgr_IsPreservedKey,
    KeystrokeMgr_PreserveKey,
    KeystrokeMgr_UnpreserveKey,
    KeystrokeMgr_SetPreservedKeyDescription,
    KeystrokeMgr_GetPreservedKeyDescription,
    KeystrokeMgr_SimulatePreservedKey
};

/*****************************************************
 * ITfMessagePump functions
 *****************************************************/

static HRESULT WINAPI MessagePump_QueryInterface(ITfMessagePump *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = impl_from_ITfMessagePumpVtbl(iface);
    return ThreadMgr_QueryInterface((ITfThreadMgr *)This, iid, *ppvOut);
}

static ULONG WINAPI MessagePump_AddRef(ITfMessagePump *iface)
{
    ThreadMgr *This = impl_from_ITfMessagePumpVtbl(iface);
    return ThreadMgr_AddRef((ITfThreadMgr*)This);
}

static ULONG WINAPI MessagePump_Release(ITfMessagePump *iface)
{
    ThreadMgr *This = impl_from_ITfMessagePumpVtbl(iface);
    return ThreadMgr_Release((ITfThreadMgr *)This);
}

static HRESULT WINAPI MessagePump_PeekMessageA(ITfMessagePump *iface,
        LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
        UINT wRemoveMsg, BOOL *pfResult)
{
    if (!pfResult)
        return E_INVALIDARG;
    *pfResult = PeekMessageA(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return S_OK;
}

static HRESULT WINAPI MessagePump_GetMessageA(ITfMessagePump *iface,
        LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
        BOOL *pfResult)
{
    if (!pfResult)
        return E_INVALIDARG;
    *pfResult = GetMessageA(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax);
    return S_OK;
}

static HRESULT WINAPI MessagePump_PeekMessageW(ITfMessagePump *iface,
        LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
        UINT wRemoveMsg, BOOL *pfResult)
{
    if (!pfResult)
        return E_INVALIDARG;
    *pfResult = PeekMessageW(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
    return S_OK;
}

static HRESULT WINAPI MessagePump_GetMessageW(ITfMessagePump *iface,
        LPMSG pMsg, HWND hwnd, UINT wMsgFilterMin, UINT wMsgFilterMax,
        BOOL *pfResult)
{
    if (!pfResult)
        return E_INVALIDARG;
    *pfResult = GetMessageW(pMsg, hwnd, wMsgFilterMin, wMsgFilterMax);
    return S_OK;
}

static const ITfMessagePumpVtbl ThreadMgr_MessagePumpVtbl =
{
    MessagePump_QueryInterface,
    MessagePump_AddRef,
    MessagePump_Release,

    MessagePump_PeekMessageA,
    MessagePump_GetMessageA,
    MessagePump_PeekMessageW,
    MessagePump_GetMessageW
};

/*****************************************************
 * ITfClientId functions
 *****************************************************/

static HRESULT WINAPI ClientId_QueryInterface(ITfClientId *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = impl_from_ITfClientIdVtbl(iface);
    return ThreadMgr_QueryInterface((ITfThreadMgr *)This, iid, *ppvOut);
}

static ULONG WINAPI ClientId_AddRef(ITfClientId *iface)
{
    ThreadMgr *This = impl_from_ITfClientIdVtbl(iface);
    return ThreadMgr_AddRef((ITfThreadMgr*)This);
}

static ULONG WINAPI ClientId_Release(ITfClientId *iface)
{
    ThreadMgr *This = impl_from_ITfClientIdVtbl(iface);
    return ThreadMgr_Release((ITfThreadMgr *)This);
}

static HRESULT WINAPI ClientId_GetClientId(ITfClientId *iface,
    REFCLSID rclsid, TfClientId *ptid)

{
    HRESULT hr;
    ITfCategoryMgr *catmgr;
    ThreadMgr *This = impl_from_ITfClientIdVtbl(iface);

    TRACE("(%p) %s\n",This,debugstr_guid(rclsid));

    CategoryMgr_Constructor(NULL,(IUnknown**)&catmgr);
    hr = ITfCategoryMgr_RegisterGUID(catmgr,rclsid,ptid);
    ITfCategoryMgr_Release(catmgr);

    return hr;
}

static const ITfClientIdVtbl ThreadMgr_ClientIdVtbl =
{
    ClientId_QueryInterface,
    ClientId_AddRef,
    ClientId_Release,

    ClientId_GetClientId
};

/*****************************************************
 * ITfThreadMgrEventSink functions  (internal)
 *****************************************************/
static HRESULT WINAPI ThreadMgrEventSink_QueryInterface(ITfThreadMgrEventSink *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);
    return ThreadMgr_QueryInterface((ITfThreadMgr *)This, iid, *ppvOut);
}

static ULONG WINAPI ThreadMgrEventSink_AddRef(ITfThreadMgrEventSink *iface)
{
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);
    return ThreadMgr_AddRef((ITfThreadMgr*)This);
}

static ULONG WINAPI ThreadMgrEventSink_Release(ITfThreadMgrEventSink *iface)
{
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);
    return ThreadMgr_Release((ITfThreadMgr *)This);
}


static HRESULT WINAPI ThreadMgrEventSink_OnInitDocumentMgr(
        ITfThreadMgrEventSink *iface,ITfDocumentMgr *pdim)
{
    struct list *cursor;
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);

    TRACE("(%p) %p\n",This,pdim);

    LIST_FOR_EACH(cursor, &This->ThreadMgrEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        ITfThreadMgrEventSink_OnInitDocumentMgr(sink->interfaces.pITfThreadMgrEventSink,pdim);
    }

    return S_OK;
}

static HRESULT WINAPI ThreadMgrEventSink_OnUninitDocumentMgr(
        ITfThreadMgrEventSink *iface, ITfDocumentMgr *pdim)
{
    struct list *cursor;
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);

    TRACE("(%p) %p\n",This,pdim);

    LIST_FOR_EACH(cursor, &This->ThreadMgrEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        ITfThreadMgrEventSink_OnUninitDocumentMgr(sink->interfaces.pITfThreadMgrEventSink,pdim);
    }

    return S_OK;
}

static HRESULT WINAPI ThreadMgrEventSink_OnSetFocus(
        ITfThreadMgrEventSink *iface, ITfDocumentMgr *pdimFocus,
        ITfDocumentMgr *pdimPrevFocus)
{
    struct list *cursor;
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);

    TRACE("(%p) %p %p\n",This,pdimFocus, pdimPrevFocus);

    LIST_FOR_EACH(cursor, &This->ThreadMgrEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        ITfThreadMgrEventSink_OnSetFocus(sink->interfaces.pITfThreadMgrEventSink, pdimFocus, pdimPrevFocus);
    }

    return S_OK;
}

static HRESULT WINAPI ThreadMgrEventSink_OnPushContext(
        ITfThreadMgrEventSink *iface, ITfContext *pic)
{
    struct list *cursor;
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);

    TRACE("(%p) %p\n",This,pic);

    LIST_FOR_EACH(cursor, &This->ThreadMgrEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        ITfThreadMgrEventSink_OnPushContext(sink->interfaces.pITfThreadMgrEventSink,pic);
    }

    return S_OK;
}

static HRESULT WINAPI ThreadMgrEventSink_OnPopContext(
        ITfThreadMgrEventSink *iface, ITfContext *pic)
{
    struct list *cursor;
    ThreadMgr *This = impl_from_ITfThreadMgrEventSink(iface);

    TRACE("(%p) %p\n",This,pic);

    LIST_FOR_EACH(cursor, &This->ThreadMgrEventSink)
    {
        ThreadMgrSink* sink = LIST_ENTRY(cursor,ThreadMgrSink,entry);
        ITfThreadMgrEventSink_OnPopContext(sink->interfaces.pITfThreadMgrEventSink,pic);
    }

    return S_OK;
}

static const ITfThreadMgrEventSinkVtbl ThreadMgr_ThreadMgrEventSinkVtbl =
{
    ThreadMgrEventSink_QueryInterface,
    ThreadMgrEventSink_AddRef,
    ThreadMgrEventSink_Release,

    ThreadMgrEventSink_OnInitDocumentMgr,
    ThreadMgrEventSink_OnUninitDocumentMgr,
    ThreadMgrEventSink_OnSetFocus,
    ThreadMgrEventSink_OnPushContext,
    ThreadMgrEventSink_OnPopContext
};

/*****************************************************
 * ITfSourceSingle functions
 *****************************************************/
static HRESULT WINAPI ThreadMgrSourceSingle_QueryInterface(ITfSourceSingle *iface, REFIID iid, LPVOID *ppvOut)
{
    ThreadMgr *This = impl_from_ITfSourceSingleVtbl(iface);
    return ThreadMgr_QueryInterface((ITfThreadMgr *)This, iid, *ppvOut);
}

static ULONG WINAPI ThreadMgrSourceSingle_AddRef(ITfSourceSingle *iface)
{
    ThreadMgr *This = impl_from_ITfSourceSingleVtbl(iface);
    return ThreadMgr_AddRef((ITfThreadMgr *)This);
}

static ULONG WINAPI ThreadMgrSourceSingle_Release(ITfSourceSingle *iface)
{
    ThreadMgr *This = impl_from_ITfSourceSingleVtbl(iface);
    return ThreadMgr_Release((ITfThreadMgr *)This);
}

static HRESULT WINAPI ThreadMgrSourceSingle_AdviseSingleSink( ITfSourceSingle *iface,
    TfClientId tid, REFIID riid, IUnknown *punk)
{
    ThreadMgr *This = impl_from_ITfSourceSingleVtbl(iface);
    FIXME("STUB:(%p) %i %s %p\n",This, tid, debugstr_guid(riid),punk);
    return E_NOTIMPL;
}

static HRESULT WINAPI ThreadMgrSourceSingle_UnadviseSingleSink( ITfSourceSingle *iface,
    TfClientId tid, REFIID riid)
{
    ThreadMgr *This = impl_from_ITfSourceSingleVtbl(iface);
    FIXME("STUB:(%p) %i %s\n",This, tid, debugstr_guid(riid));
    return E_NOTIMPL;
}

static const ITfSourceSingleVtbl ThreadMgr_SourceSingleVtbl =
{
    ThreadMgrSourceSingle_QueryInterface,
    ThreadMgrSourceSingle_AddRef,
    ThreadMgrSourceSingle_Release,

    ThreadMgrSourceSingle_AdviseSingleSink,
    ThreadMgrSourceSingle_UnadviseSingleSink,
};

HRESULT ThreadMgr_Constructor(IUnknown *pUnkOuter, IUnknown **ppOut)
{
    ThreadMgr *This;
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    /* Only 1 ThreadMgr is created per thread */
    This = TlsGetValue(tlsIndex);
    if (This)
    {
        ThreadMgr_AddRef((ITfThreadMgr*)This);
        *ppOut = (IUnknown*)This;
        return S_OK;
    }

    This = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(ThreadMgr));
    if (This == NULL)
        return E_OUTOFMEMORY;

    This->ThreadMgrVtbl= &ThreadMgr_ThreadMgrVtbl;
    This->SourceVtbl = &ThreadMgr_SourceVtbl;
    This->KeystrokeMgrVtbl= &ThreadMgr_KeystrokeMgrVtbl;
    This->MessagePumpVtbl= &ThreadMgr_MessagePumpVtbl;
    This->ClientIdVtbl = &ThreadMgr_ClientIdVtbl;
    This->ThreadMgrEventSinkVtbl = &ThreadMgr_ThreadMgrEventSinkVtbl;
    This->SourceSingleVtbl = &ThreadMgr_SourceSingleVtbl;
    This->refCount = 1;
    TlsSetValue(tlsIndex,This);

    CompartmentMgr_Constructor((IUnknown*)This, &IID_IUnknown, (IUnknown**)&This->CompartmentMgr);

    list_init(&This->CurrentPreservedKeys);
    list_init(&This->CreatedDocumentMgrs);
    list_init(&This->AssociatedFocusWindows);

    list_init(&This->ActiveLanguageProfileNotifySink);
    list_init(&This->DisplayAttributeNotifySink);
    list_init(&This->KeyTraceEventSink);
    list_init(&This->PreservedKeyNotifySink);
    list_init(&This->ThreadFocusSink);
    list_init(&This->ThreadMgrEventSink);

    TRACE("returning %p\n", This);
    *ppOut = (IUnknown *)This;
    return S_OK;
}

/**************************************************
 * IEnumTfDocumentMgrs implementaion
 **************************************************/
static void EnumTfDocumentMgr_Destructor(EnumTfDocumentMgr *This)
{
    TRACE("destroying %p\n", This);
    HeapFree(GetProcessHeap(),0,This);
}

static HRESULT WINAPI EnumTfDocumentMgr_QueryInterface(IEnumTfDocumentMgrs *iface, REFIID iid, LPVOID *ppvOut)
{
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr *)iface;
    *ppvOut = NULL;

    if (IsEqualIID(iid, &IID_IUnknown) || IsEqualIID(iid, &IID_IEnumTfDocumentMgrs))
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

static ULONG WINAPI EnumTfDocumentMgr_AddRef(IEnumTfDocumentMgrs *iface)
{
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr*)iface;
    return InterlockedIncrement(&This->refCount);
}

static ULONG WINAPI EnumTfDocumentMgr_Release(IEnumTfDocumentMgrs *iface)
{
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr *)iface;
    ULONG ret;

    ret = InterlockedDecrement(&This->refCount);
    if (ret == 0)
        EnumTfDocumentMgr_Destructor(This);
    return ret;
}

static HRESULT WINAPI EnumTfDocumentMgr_Next(IEnumTfDocumentMgrs *iface,
    ULONG ulCount, ITfDocumentMgr **rgDocumentMgr, ULONG *pcFetched)
{
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr *)iface;
    ULONG fetched = 0;

    TRACE("(%p)\n",This);

    if (rgDocumentMgr == NULL) return E_POINTER;

    while (fetched < ulCount)
    {
        DocumentMgrEntry *mgrentry;
        if (This->index == NULL)
            break;

        mgrentry = LIST_ENTRY(This->index,DocumentMgrEntry,entry);
        if (mgrentry == NULL)
            break;

        *rgDocumentMgr = mgrentry->docmgr;
        ITfDocumentMgr_AddRef(*rgDocumentMgr);

        This->index = list_next(This->head, This->index);
        ++fetched;
        ++rgDocumentMgr;
    }

    if (pcFetched) *pcFetched = fetched;
    return fetched == ulCount ? S_OK : S_FALSE;
}

static HRESULT WINAPI EnumTfDocumentMgr_Skip( IEnumTfDocumentMgrs* iface, ULONG celt)
{
    INT i;
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr *)iface;
    TRACE("(%p)\n",This);
    for(i = 0; i < celt && This->index != NULL; i++)
        This->index = list_next(This->head, This->index);
    return S_OK;
}

static HRESULT WINAPI EnumTfDocumentMgr_Reset( IEnumTfDocumentMgrs* iface)
{
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr *)iface;
    TRACE("(%p)\n",This);
    This->index = list_head(This->head);
    return S_OK;
}

static HRESULT WINAPI EnumTfDocumentMgr_Clone( IEnumTfDocumentMgrs *iface,
    IEnumTfDocumentMgrs **ppenum)
{
    EnumTfDocumentMgr *This = (EnumTfDocumentMgr *)iface;
    HRESULT res;

    TRACE("(%p)\n",This);

    if (ppenum == NULL) return E_POINTER;

    res = EnumTfDocumentMgr_Constructor(This->head, ppenum);
    if (SUCCEEDED(res))
    {
        EnumTfDocumentMgr *new_This = (EnumTfDocumentMgr *)*ppenum;
        new_This->index = This->index;
    }
    return res;
}

static const IEnumTfDocumentMgrsVtbl IEnumTfDocumentMgrs_Vtbl ={
    EnumTfDocumentMgr_QueryInterface,
    EnumTfDocumentMgr_AddRef,
    EnumTfDocumentMgr_Release,

    EnumTfDocumentMgr_Clone,
    EnumTfDocumentMgr_Next,
    EnumTfDocumentMgr_Reset,
    EnumTfDocumentMgr_Skip
};

static HRESULT EnumTfDocumentMgr_Constructor(struct list* head, IEnumTfDocumentMgrs **ppOut)
{
    EnumTfDocumentMgr *This;

    This = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(EnumTfDocumentMgr));
    if (This == NULL)
        return E_OUTOFMEMORY;

    This->Vtbl= &IEnumTfDocumentMgrs_Vtbl;
    This->refCount = 1;
    This->head = head;
    This->index = list_head(This->head);

    TRACE("returning %p\n", This);
    *ppOut = (IEnumTfDocumentMgrs*)This;
    return S_OK;
}

void ThreadMgr_OnDocumentMgrDestruction(ITfThreadMgr *tm, ITfDocumentMgr *mgr)
{
    ThreadMgr *This = (ThreadMgr *)tm;
    struct list *cursor;
    LIST_FOR_EACH(cursor, &This->CreatedDocumentMgrs)
    {
        DocumentMgrEntry *mgrentry = LIST_ENTRY(cursor,DocumentMgrEntry,entry);
        if (mgrentry->docmgr == mgr)
        {
            list_remove(cursor);
            HeapFree(GetProcessHeap(),0,mgrentry);
            return;
        }
    }
    FIXME("ITfDocumenMgr %p not found in this thread\n",mgr);
}
