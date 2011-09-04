/*
 * Copyright 2007 Jacek Caban for CodeWeavers
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

#include <stdio.h>

#include "urlmon_main.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(urlmon);

static WCHAR BSCBHolder[] = { '_','B','S','C','B','_','H','o','l','d','e','r','_',0 };

extern IID IID_IBindStatusCallbackHolder;

typedef struct {
    IBindStatusCallbackEx IBindStatusCallbackEx_iface;
    IServiceProvider      IServiceProvider_iface;
    IHttpNegotiate2       IHttpNegotiate2_iface;
    IAuthenticate         IAuthenticate_iface;

    LONG ref;

    IBindStatusCallback *callback;
    IServiceProvider *serv_prov;
} BindStatusCallback;

static void *get_callback_iface(BindStatusCallback *This, REFIID riid)
{
    void *ret;
    HRESULT hres;

    hres = IBindStatusCallback_QueryInterface(This->callback, riid, (void**)&ret);
    if(FAILED(hres) && This->serv_prov)
        hres = IServiceProvider_QueryService(This->serv_prov, riid, riid, &ret);

    return SUCCEEDED(hres) ? ret : NULL;
}

static inline BindStatusCallback *impl_from_IBindStatusCallbackEx(IBindStatusCallbackEx *iface)
{
    return CONTAINING_RECORD(iface, BindStatusCallback, IBindStatusCallbackEx_iface);
}

static HRESULT WINAPI BindStatusCallback_QueryInterface(IBindStatusCallbackEx *iface,
        REFIID riid, void **ppv)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    *ppv = NULL;

    if(IsEqualGUID(&IID_IUnknown, riid)) {
        TRACE("(%p)->(IID_IUnknown, %p)\n", This, ppv);
        *ppv = &This->IBindStatusCallbackEx_iface;
    }else if(IsEqualGUID(&IID_IBindStatusCallback, riid)) {
        TRACE("(%p)->(IID_IBindStatusCallback, %p)\n", This, ppv);
        *ppv = &This->IBindStatusCallbackEx_iface;
    }else if(IsEqualGUID(&IID_IBindStatusCallbackEx, riid)) {
        TRACE("(%p)->(IID_IBindStatusCallback, %p)\n", This, ppv);
        *ppv = &This->IBindStatusCallbackEx_iface;
    }else if(IsEqualGUID(&IID_IBindStatusCallbackHolder, riid)) {
        TRACE("(%p)->(IID_IBindStatusCallbackHolder, %p)\n", This, ppv);
        *ppv = This;
    }else if(IsEqualGUID(&IID_IServiceProvider, riid)) {
        TRACE("(%p)->(IID_IServiceProvider, %p)\n", This, ppv);
        *ppv = &This->IServiceProvider_iface;
    }else if(IsEqualGUID(&IID_IHttpNegotiate, riid)) {
        TRACE("(%p)->(IID_IHttpNegotiate, %p)\n", This, ppv);
        *ppv = &This->IHttpNegotiate2_iface;
    }else if(IsEqualGUID(&IID_IHttpNegotiate2, riid)) {
        TRACE("(%p)->(IID_IHttpNegotiate2, %p)\n", This, ppv);
        *ppv = &This->IHttpNegotiate2_iface;
    }else if(IsEqualGUID(&IID_IAuthenticate, riid)) {
        TRACE("(%p)->(IID_IAuthenticate, %p)\n", This, ppv);
        *ppv = &This->IAuthenticate_iface;
    }

    if(*ppv) {
        IBindStatusCallback_AddRef((IUnknown*)*ppv);
        return S_OK;
    }

    TRACE("Unsupported riid = %s\n", debugstr_guid(riid));
    return E_NOINTERFACE;
}

static ULONG WINAPI BindStatusCallback_AddRef(IBindStatusCallbackEx *iface)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);
    LONG ref = InterlockedIncrement(&This->ref);

    TRACE("(%p) ref = %d\n", This, ref);

    return ref;
}

static ULONG WINAPI BindStatusCallback_Release(IBindStatusCallbackEx *iface)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);
    LONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p) ref = %d\n", This, ref);

    if(!ref) {
        if(This->serv_prov)
            IServiceProvider_Release(This->serv_prov);
        IBindStatusCallback_Release(This->callback);
        heap_free(This);
    }

    return ref;
}

static HRESULT WINAPI BindStatusCallback_OnStartBinding(IBindStatusCallbackEx *iface,
        DWORD dwReserved, IBinding *pbind)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("(%p)->(%d %p)\n", This, dwReserved, pbind);

    return IBindStatusCallback_OnStartBinding(This->callback, 0xff, pbind);
}

static HRESULT WINAPI BindStatusCallback_GetPriority(IBindStatusCallbackEx *iface, LONG *pnPriority)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("(%p)->(%p)\n", This, pnPriority);

    return IBindStatusCallback_GetPriority(This->callback, pnPriority);
}

static HRESULT WINAPI BindStatusCallback_OnLowResource(IBindStatusCallbackEx *iface, DWORD reserved)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("(%p)->(%d)\n", This, reserved);

    return IBindStatusCallback_OnLowResource(This->callback, reserved);
}

static HRESULT WINAPI BindStatusCallback_OnProgress(IBindStatusCallbackEx *iface, ULONG ulProgress,
        ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("%p)->(%u %u %u %s)\n", This, ulProgress, ulProgressMax, ulStatusCode,
            debugstr_w(szStatusText));

    return IBindStatusCallback_OnProgress(This->callback, ulProgress,
            ulProgressMax, ulStatusCode, szStatusText);
}

static HRESULT WINAPI BindStatusCallback_OnStopBinding(IBindStatusCallbackEx *iface,
        HRESULT hresult, LPCWSTR szError)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("(%p)->(%08x %s)\n", This, hresult, debugstr_w(szError));

    return IBindStatusCallback_OnStopBinding(This->callback, hresult, szError);
}

static HRESULT WINAPI BindStatusCallback_GetBindInfo(IBindStatusCallbackEx *iface,
        DWORD *grfBINDF, BINDINFO *pbindinfo)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);
    IBindStatusCallbackEx *bscex;
    HRESULT hres;

    TRACE("(%p)->(%p %p)\n", This, grfBINDF, pbindinfo);

    hres = IBindStatusCallback_QueryInterface(This->callback, &IID_IBindStatusCallbackEx, (void**)&bscex);
    if(SUCCEEDED(hres)) {
        DWORD bindf2 = 0, reserv = 0;

        hres = IBindStatusCallbackEx_GetBindInfoEx(bscex, grfBINDF, pbindinfo, &bindf2, &reserv);
        IBindStatusCallbackEx_Release(bscex);
    }else {
        hres = IBindStatusCallback_GetBindInfo(This->callback, grfBINDF, pbindinfo);
    }

    return hres;
}

static HRESULT WINAPI BindStatusCallback_OnDataAvailable(IBindStatusCallbackEx *iface,
        DWORD grfBSCF, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("(%p)->(%08x %d %p %p)\n", This, grfBSCF, dwSize, pformatetc, pstgmed);

    return IBindStatusCallback_OnDataAvailable(This->callback, grfBSCF, dwSize, pformatetc, pstgmed);
}

static HRESULT WINAPI BindStatusCallback_OnObjectAvailable(IBindStatusCallbackEx *iface,
        REFIID riid, IUnknown *punk)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);

    TRACE("(%p)->(%s %p)\n", This, debugstr_guid(riid), punk);

    return IBindStatusCallback_OnObjectAvailable(This->callback, riid, punk);
}

static HRESULT WINAPI BindStatusCallback_GetBindInfoEx(IBindStatusCallbackEx *iface, DWORD *grfBINDF,
        BINDINFO *pbindinfo, DWORD *grfBINDF2, DWORD *pdwReserved)
{
    BindStatusCallback *This = impl_from_IBindStatusCallbackEx(iface);
    IBindStatusCallbackEx *bscex;
    HRESULT hres;

    TRACE("(%p)->(%p %p %p %p)\n", This, grfBINDF, pbindinfo, grfBINDF2, pdwReserved);

    hres = IBindStatusCallback_QueryInterface(This->callback, &IID_IBindStatusCallbackEx, (void**)&bscex);
    if(SUCCEEDED(hres)) {
        hres = IBindStatusCallbackEx_GetBindInfoEx(bscex, grfBINDF, pbindinfo, grfBINDF2, pdwReserved);
        IBindStatusCallbackEx_Release(bscex);
    }else {
        hres = IBindStatusCallback_GetBindInfo(This->callback, grfBINDF, pbindinfo);
    }

    return hres;
}

static const IBindStatusCallbackExVtbl BindStatusCallbackExVtbl = {
    BindStatusCallback_QueryInterface,
    BindStatusCallback_AddRef,
    BindStatusCallback_Release,
    BindStatusCallback_OnStartBinding,
    BindStatusCallback_GetPriority,
    BindStatusCallback_OnLowResource,
    BindStatusCallback_OnProgress,
    BindStatusCallback_OnStopBinding,
    BindStatusCallback_GetBindInfo,
    BindStatusCallback_OnDataAvailable,
    BindStatusCallback_OnObjectAvailable,
    BindStatusCallback_GetBindInfoEx
};

static inline BindStatusCallback *impl_from_IServiceProvider(IServiceProvider *iface)
{
    return CONTAINING_RECORD(iface, BindStatusCallback, IServiceProvider_iface);
}

static HRESULT WINAPI BSCServiceProvider_QueryInterface(IServiceProvider *iface,
        REFIID riid, void **ppv)
{
    BindStatusCallback *This = impl_from_IServiceProvider(iface);
    return IBindStatusCallback_QueryInterface(&This->IBindStatusCallbackEx_iface, riid, ppv);
}

static ULONG WINAPI BSCServiceProvider_AddRef(IServiceProvider *iface)
{
    BindStatusCallback *This = impl_from_IServiceProvider(iface);
    return IBindStatusCallback_AddRef(&This->IBindStatusCallbackEx_iface);
}

static ULONG WINAPI BSCServiceProvider_Release(IServiceProvider *iface)
{
    BindStatusCallback *This = impl_from_IServiceProvider(iface);
    return IBindStatusCallback_Release(&This->IBindStatusCallbackEx_iface);
}

static HRESULT WINAPI BSCServiceProvider_QueryService(IServiceProvider *iface,
        REFGUID guidService, REFIID riid, void **ppv)
{
    BindStatusCallback *This = impl_from_IServiceProvider(iface);
    HRESULT hres;

    if(IsEqualGUID(&IID_IHttpNegotiate, guidService)) {
        TRACE("(%p)->(IID_IHttpNegotiate %s %p)\n", This, debugstr_guid(riid), ppv);
        return IBindStatusCallback_QueryInterface(&This->IBindStatusCallbackEx_iface, riid, ppv);
    }

    if(IsEqualGUID(&IID_IHttpNegotiate2, guidService)) {
        TRACE("(%p)->(IID_IHttpNegotiate2 %s %p)\n", This, debugstr_guid(riid), ppv);
        return IBindStatusCallback_QueryInterface(&This->IBindStatusCallbackEx_iface, riid, ppv);
    }

    if(IsEqualGUID(&IID_IAuthenticate, guidService)) {
        TRACE("(%p)->(IID_IAuthenticate %s %p)\n", This, debugstr_guid(riid), ppv);
        return IBindStatusCallback_QueryInterface(&This->IBindStatusCallbackEx_iface, riid, ppv);
    }

    TRACE("(%p)->(%s %s %p)\n", This, debugstr_guid(guidService), debugstr_guid(riid), ppv);

    hres = IBindStatusCallback_QueryInterface(This->callback, riid, ppv);
    if(SUCCEEDED(hres))
        return S_OK;

    if(This->serv_prov) {
        hres = IServiceProvider_QueryService(This->serv_prov, guidService, riid, ppv);
        if(SUCCEEDED(hres))
            return S_OK;
    }

    return E_NOINTERFACE;
}

static const IServiceProviderVtbl BSCServiceProviderVtbl = {
    BSCServiceProvider_QueryInterface,
    BSCServiceProvider_AddRef,
    BSCServiceProvider_Release,
    BSCServiceProvider_QueryService
};

static inline BindStatusCallback *impl_from_IHttpNegotiate2(IHttpNegotiate2 *iface)
{
    return CONTAINING_RECORD(iface, BindStatusCallback, IHttpNegotiate2_iface);
}

static HRESULT WINAPI BSCHttpNegotiate_QueryInterface(IHttpNegotiate2 *iface,
        REFIID riid, void **ppv)
{
    BindStatusCallback *This = impl_from_IHttpNegotiate2(iface);
    return IBindStatusCallback_QueryInterface(&This->IBindStatusCallbackEx_iface, riid, ppv);
}

static ULONG WINAPI BSCHttpNegotiate_AddRef(IHttpNegotiate2 *iface)
{
    BindStatusCallback *This = impl_from_IHttpNegotiate2(iface);
    return IBindStatusCallback_AddRef(&This->IBindStatusCallbackEx_iface);
}

static ULONG WINAPI BSCHttpNegotiate_Release(IHttpNegotiate2 *iface)
{
    BindStatusCallback *This = impl_from_IHttpNegotiate2(iface);
    return IBindStatusCallback_Release(&This->IBindStatusCallbackEx_iface);
}

static HRESULT WINAPI BSCHttpNegotiate_BeginningTransaction(IHttpNegotiate2 *iface,
        LPCWSTR szURL, LPCWSTR szHeaders, DWORD dwReserved, LPWSTR *pszAdditionalHeaders)
{
    BindStatusCallback *This = impl_from_IHttpNegotiate2(iface);
    IHttpNegotiate *http_negotiate;
    HRESULT hres = S_OK;

    TRACE("(%p)->(%s %s %d %p)\n", This, debugstr_w(szURL), debugstr_w(szHeaders), dwReserved,
          pszAdditionalHeaders);

    *pszAdditionalHeaders = NULL;

    http_negotiate = get_callback_iface(This, &IID_IHttpNegotiate);
    if(http_negotiate) {
        hres = IHttpNegotiate_BeginningTransaction(http_negotiate, szURL, szHeaders,
                dwReserved, pszAdditionalHeaders);
        IHttpNegotiate_Release(http_negotiate);
    }

    return hres;
}

static HRESULT WINAPI BSCHttpNegotiate_OnResponse(IHttpNegotiate2 *iface, DWORD dwResponseCode,
        LPCWSTR szResponseHeaders, LPCWSTR szRequestHeaders,
        LPWSTR *pszAdditionalRequestHeaders)
{
    BindStatusCallback *This = impl_from_IHttpNegotiate2(iface);
    LPWSTR additional_headers = NULL;
    IHttpNegotiate *http_negotiate;
    HRESULT hres = S_OK;

    TRACE("(%p)->(%d %s %s %p)\n", This, dwResponseCode, debugstr_w(szResponseHeaders),
          debugstr_w(szRequestHeaders), pszAdditionalRequestHeaders);

    http_negotiate = get_callback_iface(This, &IID_IHttpNegotiate);
    if(http_negotiate) {
        hres = IHttpNegotiate_OnResponse(http_negotiate, dwResponseCode, szResponseHeaders,
                szRequestHeaders, &additional_headers);
        IHttpNegotiate_Release(http_negotiate);
    }

    if(pszAdditionalRequestHeaders)
        *pszAdditionalRequestHeaders = additional_headers;
    else if(additional_headers)
        CoTaskMemFree(additional_headers);

    return hres;
}

static HRESULT WINAPI BSCHttpNegotiate_GetRootSecurityId(IHttpNegotiate2 *iface,
        BYTE *pbSecurityId, DWORD *pcbSecurityId, DWORD_PTR dwReserved)
{
    BindStatusCallback *This = impl_from_IHttpNegotiate2(iface);
    IHttpNegotiate2 *http_negotiate2;
    HRESULT hres = E_FAIL;

    TRACE("(%p)->(%p %p %ld)\n", This, pbSecurityId, pcbSecurityId, dwReserved);

    http_negotiate2 = get_callback_iface(This, &IID_IHttpNegotiate2);
    if(http_negotiate2) {
        hres = IHttpNegotiate2_GetRootSecurityId(http_negotiate2, pbSecurityId,
                pcbSecurityId, dwReserved);
        IHttpNegotiate2_Release(http_negotiate2);
    }

    return hres;
}

static const IHttpNegotiate2Vtbl BSCHttpNegotiateVtbl = {
    BSCHttpNegotiate_QueryInterface,
    BSCHttpNegotiate_AddRef,
    BSCHttpNegotiate_Release,
    BSCHttpNegotiate_BeginningTransaction,
    BSCHttpNegotiate_OnResponse,
    BSCHttpNegotiate_GetRootSecurityId
};

static inline BindStatusCallback *impl_from_IAuthenticate(IAuthenticate *iface)
{
    return CONTAINING_RECORD(iface, BindStatusCallback, IAuthenticate_iface);
}

static HRESULT WINAPI BSCAuthenticate_QueryInterface(IAuthenticate *iface, REFIID riid, void **ppv)
{
    BindStatusCallback *This = impl_from_IAuthenticate(iface);
    return IBindStatusCallback_QueryInterface(&This->IAuthenticate_iface, riid, ppv);
}

static ULONG WINAPI BSCAuthenticate_AddRef(IAuthenticate *iface)
{
    BindStatusCallback *This = impl_from_IAuthenticate(iface);
    return IBindStatusCallback_AddRef(&This->IBindStatusCallbackEx_iface);
}

static ULONG WINAPI BSCAuthenticate_Release(IAuthenticate *iface)
{
    BindStatusCallback *This = impl_from_IAuthenticate(iface);
    return IBindStatusCallback_Release(&This->IBindStatusCallbackEx_iface);
}

static HRESULT WINAPI BSCAuthenticate_Authenticate(IAuthenticate *iface,
        HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword)
{
    BindStatusCallback *This = impl_from_IAuthenticate(iface);
    FIXME("(%p)->(%p %p %p)\n", This, phwnd, pszUsername, pszPassword);
    return E_NOTIMPL;
}

static const IAuthenticateVtbl BSCAuthenticateVtbl = {
    BSCAuthenticate_QueryInterface,
    BSCAuthenticate_AddRef,
    BSCAuthenticate_Release,
    BSCAuthenticate_Authenticate
};

static void set_callback(BindStatusCallback *This, IBindStatusCallback *bsc)
{
    IServiceProvider *serv_prov;
    HRESULT hres;

    if(This->callback)
        IBindStatusCallback_Release(This->callback);
    if(This->serv_prov)
        IServiceProvider_Release(This->serv_prov);

    IBindStatusCallback_AddRef(bsc);
    This->callback = bsc;

    hres = IBindStatusCallback_QueryInterface(bsc, &IID_IServiceProvider, (void**)&serv_prov);
    This->serv_prov = hres == S_OK ? serv_prov : NULL;
}

HRESULT wrap_callback(IBindStatusCallback *bsc, IBindStatusCallback **ret_iface)
{
    BindStatusCallback *ret;

    ret = heap_alloc_zero(sizeof(BindStatusCallback));
    if(!ret)
        return E_OUTOFMEMORY;

    ret->IBindStatusCallbackEx_iface.lpVtbl = &BindStatusCallbackExVtbl;
    ret->IServiceProvider_iface.lpVtbl = &BSCServiceProviderVtbl;
    ret->IHttpNegotiate2_iface.lpVtbl = &BSCHttpNegotiateVtbl;
    ret->IAuthenticate_iface.lpVtbl = &BSCAuthenticateVtbl;

    ret->ref = 1;
    set_callback(ret, bsc);

    *ret_iface = (IBindStatusCallback*)&ret->IBindStatusCallbackEx_iface;
    return S_OK;
}

/***********************************************************************
 *           RegisterBindStatusCallback (urlmon.@)
 *
 * Register a bind status callback.
 *
 * PARAMS
 *  pbc           [I] Binding context
 *  pbsc          [I] Callback to register
 *  ppbscPrevious [O] Destination for previous callback
 *  dwReserved    [I] Reserved, must be 0.
 *
 * RETURNS
 *    Success: S_OK.
 *    Failure: E_INVALIDARG, if any argument is invalid, or
 *             E_OUTOFMEMORY if memory allocation fails.
 */
HRESULT WINAPI RegisterBindStatusCallback(IBindCtx *pbc, IBindStatusCallback *pbsc,
        IBindStatusCallback **ppbscPrevious, DWORD dwReserved)
{
    BindStatusCallback *holder;
    IBindStatusCallback *bsc, *prev = NULL;
    IUnknown *unk;
    HRESULT hres;

    TRACE("(%p %p %p %x)\n", pbc, pbsc, ppbscPrevious, dwReserved);

    if (!pbc || !pbsc)
        return E_INVALIDARG;

    hres = IBindCtx_GetObjectParam(pbc, BSCBHolder, &unk);
    if(SUCCEEDED(hres)) {
        hres = IUnknown_QueryInterface(unk, &IID_IBindStatusCallback, (void**)&bsc);
        IUnknown_Release(unk);
        if(SUCCEEDED(hres)) {
            hres = IBindStatusCallback_QueryInterface(bsc, &IID_IBindStatusCallbackHolder, (void**)&holder);
            if(SUCCEEDED(hres)) {
                if(ppbscPrevious) {
                    IBindStatusCallback_AddRef(holder->callback);
                    *ppbscPrevious = holder->callback;
                }

                set_callback(holder, pbsc);

                IBindStatusCallback_Release(bsc);
                IBindStatusCallback_Release(&holder->IBindStatusCallbackEx_iface);
                return S_OK;
            }else {
                prev = bsc;
            }
        }

        IBindCtx_RevokeObjectParam(pbc, BSCBHolder);
    }

    hres = wrap_callback(pbsc, &bsc);
    if(SUCCEEDED(hres)) {
        hres = IBindCtx_RegisterObjectParam(pbc, BSCBHolder, (IUnknown*)bsc);
        IBindStatusCallback_Release(bsc);
    }
    if(FAILED(hres)) {
        if(prev)
            IBindStatusCallback_Release(prev);
        return hres;
    }

    if(ppbscPrevious)
        *ppbscPrevious = prev;
    return S_OK;
}

/***********************************************************************
 *           RevokeBindStatusCallback (URLMON.@)
 *
 * Unregister a bind status callback.
 *
 *  pbc           [I] Binding context
 *  pbsc          [I] Callback to unregister
 *
 * RETURNS
 *    Success: S_OK.
 *    Failure: E_INVALIDARG, if any argument is invalid
 */
HRESULT WINAPI RevokeBindStatusCallback(IBindCtx *pbc, IBindStatusCallback *pbsc)
{
    BindStatusCallback *holder;
    IBindStatusCallback *callback;
    IUnknown *unk;
    BOOL dorevoke = FALSE;
    HRESULT hres;

    TRACE("(%p %p)\n", pbc, pbsc);

    if (!pbc || !pbsc)
        return E_INVALIDARG;

    hres = IBindCtx_GetObjectParam(pbc, BSCBHolder, &unk);
    if(FAILED(hres))
        return S_OK;

    hres = IUnknown_QueryInterface(unk, &IID_IBindStatusCallback, (void**)&callback);
    IUnknown_Release(unk);
    if(FAILED(hres))
        return S_OK;

    hres = IBindStatusCallback_QueryInterface(callback, &IID_IBindStatusCallbackHolder, (void**)&holder);
    if(SUCCEEDED(hres)) {
        if(pbsc == holder->callback)
            dorevoke = TRUE;
        IBindStatusCallback_Release(&holder->IBindStatusCallbackEx_iface);
    }else if(pbsc == callback) {
        dorevoke = TRUE;
    }
    IBindStatusCallback_Release(callback);

    if(dorevoke)
        IBindCtx_RevokeObjectParam(pbc, BSCBHolder);

    return S_OK;
}

typedef struct {
    IBindCtx IBindCtx_iface;

    LONG ref;

    IBindCtx *bindctx;
} AsyncBindCtx;

static inline AsyncBindCtx *impl_from_IBindCtx(IBindCtx *iface)
{
    return CONTAINING_RECORD(iface, AsyncBindCtx, IBindCtx_iface);
}

static HRESULT WINAPI AsyncBindCtx_QueryInterface(IBindCtx *iface, REFIID riid, void **ppv)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    *ppv = NULL;

    if(IsEqualGUID(riid, &IID_IUnknown)) {
        TRACE("(%p)->(IID_IUnknown %p)\n", This, ppv);
        *ppv = &This->IBindCtx_iface;
    }else if(IsEqualGUID(riid, &IID_IBindCtx)) {
        TRACE("(%p)->(IID_IBindCtx %p)\n", This, ppv);
        *ppv = &This->IBindCtx_iface;
    }else if(IsEqualGUID(riid, &IID_IAsyncBindCtx)) {
        TRACE("(%p)->(IID_IAsyncBindCtx %p)\n", This, ppv);
        *ppv = &This->IBindCtx_iface;
    }

    if(*ppv) {
        IUnknown_AddRef((IUnknown*)*ppv);
        return S_OK;
    }

    FIXME("(%p)->(%s %p)\n", This, debugstr_guid(riid), ppv);
    return E_NOINTERFACE;
}

static ULONG WINAPI AsyncBindCtx_AddRef(IBindCtx *iface)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);
    LONG ref = InterlockedIncrement(&This->ref);

    TRACE("(%p) ref=%d\n", This, ref);

    return ref;
}

static ULONG WINAPI AsyncBindCtx_Release(IBindCtx *iface)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);
    LONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p) ref=%d\n", This, ref);

    if(!ref) {
        IBindCtx_Release(This->bindctx);
        heap_free(This);
    }

    return ref;
}

static HRESULT WINAPI AsyncBindCtx_RegisterObjectBound(IBindCtx *iface, IUnknown *punk)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%p)\n", This, punk);

    return IBindCtx_RegisterObjectBound(This->bindctx, punk);
}

static HRESULT WINAPI AsyncBindCtx_RevokeObjectBound(IBindCtx *iface, IUnknown *punk)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p %p)\n", This, punk);

    return IBindCtx_RevokeObjectBound(This->bindctx, punk);
}

static HRESULT WINAPI AsyncBindCtx_ReleaseBoundObjects(IBindCtx *iface)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)\n", This);

    return IBindCtx_ReleaseBoundObjects(This->bindctx);
}

static HRESULT WINAPI AsyncBindCtx_SetBindOptions(IBindCtx *iface, BIND_OPTS *pbindopts)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%p)\n", This, pbindopts);

    return IBindCtx_SetBindOptions(This->bindctx, pbindopts);
}

static HRESULT WINAPI AsyncBindCtx_GetBindOptions(IBindCtx *iface, BIND_OPTS *pbindopts)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%p)\n", This, pbindopts);

    return IBindCtx_GetBindOptions(This->bindctx, pbindopts);
}

static HRESULT WINAPI AsyncBindCtx_GetRunningObjectTable(IBindCtx *iface, IRunningObjectTable **pprot)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%p)\n", This, pprot);

    return IBindCtx_GetRunningObjectTable(This->bindctx, pprot);
}

static HRESULT WINAPI AsyncBindCtx_RegisterObjectParam(IBindCtx *iface, LPOLESTR pszkey, IUnknown *punk)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%s %p)\n", This, debugstr_w(pszkey), punk);

    return IBindCtx_RegisterObjectParam(This->bindctx, pszkey, punk);
}

static HRESULT WINAPI AsyncBindCtx_GetObjectParam(IBindCtx* iface, LPOLESTR pszkey, IUnknown **punk)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%s %p)\n", This, debugstr_w(pszkey), punk);

    return IBindCtx_GetObjectParam(This->bindctx, pszkey, punk);
}

static HRESULT WINAPI AsyncBindCtx_RevokeObjectParam(IBindCtx *iface, LPOLESTR pszkey)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%s)\n", This, debugstr_w(pszkey));

    return IBindCtx_RevokeObjectParam(This->bindctx, pszkey);
}

static HRESULT WINAPI AsyncBindCtx_EnumObjectParam(IBindCtx *iface, IEnumString **pszkey)
{
    AsyncBindCtx *This = impl_from_IBindCtx(iface);

    TRACE("(%p)->(%p)\n", This, pszkey);

    return IBindCtx_EnumObjectParam(This->bindctx, pszkey);
}

static const IBindCtxVtbl AsyncBindCtxVtbl =
{
    AsyncBindCtx_QueryInterface,
    AsyncBindCtx_AddRef,
    AsyncBindCtx_Release,
    AsyncBindCtx_RegisterObjectBound,
    AsyncBindCtx_RevokeObjectBound,
    AsyncBindCtx_ReleaseBoundObjects,
    AsyncBindCtx_SetBindOptions,
    AsyncBindCtx_GetBindOptions,
    AsyncBindCtx_GetRunningObjectTable,
    AsyncBindCtx_RegisterObjectParam,
    AsyncBindCtx_GetObjectParam,
    AsyncBindCtx_EnumObjectParam,
    AsyncBindCtx_RevokeObjectParam
};

static HRESULT init_bindctx(IBindCtx *bindctx, DWORD options,
       IBindStatusCallback *callback, IEnumFORMATETC *format)
{
    BIND_OPTS bindopts;
    HRESULT hres;

    if(options)
        FIXME("not supported options %08x\n", options);
    if(format)
        FIXME("format is not supported\n");

    bindopts.cbStruct = sizeof(BIND_OPTS);
    bindopts.grfFlags = BIND_MAYBOTHERUSER;
    bindopts.grfMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
    bindopts.dwTickCountDeadline = 0;

    hres = IBindCtx_SetBindOptions(bindctx, &bindopts);
    if(FAILED(hres))
       return hres;

    if(callback) {
        hres = RegisterBindStatusCallback(bindctx, callback, NULL, 0);
        if(FAILED(hres))
            return hres;
    }

    return S_OK;
}

/***********************************************************************
 *           CreateAsyncBindCtx (urlmon.@)
 */
HRESULT WINAPI CreateAsyncBindCtx(DWORD reserved, IBindStatusCallback *callback,
        IEnumFORMATETC *format, IBindCtx **pbind)
{
    IBindCtx *bindctx;
    HRESULT hres;

    TRACE("(%08x %p %p %p)\n", reserved, callback, format, pbind);

    if(!pbind || !callback)
        return E_INVALIDARG;

    hres = CreateBindCtx(0, &bindctx);
    if(FAILED(hres))
        return hres;

    hres = init_bindctx(bindctx, 0, callback, format);
    if(FAILED(hres)) {
        IBindCtx_Release(bindctx);
        return hres;
    }

    *pbind = bindctx;
    return S_OK;
}

/***********************************************************************
 *           CreateAsyncBindCtxEx (urlmon.@)
 *
 * Create an asynchronous bind context.
 */
HRESULT WINAPI CreateAsyncBindCtxEx(IBindCtx *ibind, DWORD options,
        IBindStatusCallback *callback, IEnumFORMATETC *format, IBindCtx** pbind,
        DWORD reserved)
{
    AsyncBindCtx *ret;
    IBindCtx *bindctx;
    HRESULT hres;

    TRACE("(%p %08x %p %p %p %d)\n", ibind, options, callback, format, pbind, reserved);

    if(!pbind)
        return E_INVALIDARG;

    if(reserved)
        WARN("reserved=%d\n", reserved);

    if(ibind) {
        IBindCtx_AddRef(ibind);
        bindctx = ibind;
    }else {
        hres = CreateBindCtx(0, &bindctx);
        if(FAILED(hres))
            return hres;
    }

    ret = heap_alloc(sizeof(AsyncBindCtx));

    ret->IBindCtx_iface.lpVtbl = &AsyncBindCtxVtbl;
    ret->ref = 1;
    ret->bindctx = bindctx;

    hres = init_bindctx(&ret->IBindCtx_iface, options, callback, format);
    if(FAILED(hres)) {
        IBindCtx_Release(&ret->IBindCtx_iface);
        return hres;
    }

    *pbind = &ret->IBindCtx_iface;
    return S_OK;
}
